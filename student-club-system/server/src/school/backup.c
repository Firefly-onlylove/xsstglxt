/**
 * backup.c — 学校端 数据库备份与恢复
 * 学生社团管理系统 SCMS（Web 架构 · 学校端）
 *
 * ┌──────────────────────────────────────────────────────────────────┐
 * │  ★ 备用参考实现。默认整份被 #if 0 关闭，不参与编译（走弱符号占位）。 │
 * │  ★ 启用方法：把下面这行的  #if 0  改成  #if 1  ，重新 make 即可。   │
 * └──────────────────────────────────────────────────────────────────┘
 *
 * 覆盖路由：
 *   POST /api/school/backup   → sch_backup_create  调用 mysqldump 生成备份文件
 *   GET  /api/school/backups  → sch_backup_list    历史备份记录
 *
 * ⚠ 依赖系统已安装 mysqldump（随 MySQL 一起安装）。备份文件存 backups/ 目录。
 *   恢复操作风险高（会覆盖现有数据），不做成一键 API，仅在此说明手动方式：
 *      mysql -u root -p scms_db < backups/xxx.sql
 */
#if 1

#include "../../include/routes.h"
#include "../../include/api.h"
#include "../../include/db.h"
#include "../../include/common.h"
#include "../../include/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* POST /api/school/backup  触发一次手动备份 */
void sch_backup_create(ApiContext *ctx) {
    if (!api_require_school_admin(ctx)) return;

    /* 用时间戳命名备份文件 */
    char ts[20]; utils_now_datetime(ts);
    /* 把 "2026-07-17 15:30:00" 里的空格和冒号换成下划线，做成合法文件名 */
    char safe_ts[20];
    for (int i = 0; ts[i] && i < 19; ++i)
        safe_ts[i] = (ts[i] == ' ' || ts[i] == ':' || ts[i] == '-') ? '_' : ts[i];
    safe_ts[19] = '\0';

    char filepath[256];
    snprintf(filepath, sizeof(filepath), "storage/backups/scms_%s.sql", safe_ts);

    /* 组装 mysqldump 命令。
     * 注意：生产环境不要把密码明文放命令行，应使用 --defaults-extra-file。
     * 这里为教学演示保持简单，实际密码建议从 db_config.ini 读取后写入临时选项文件。 */
    char cmd[600];

    /* 确保 backup 目录存在 */
    #ifdef _WIN32
        system("if not exist storage\\backups mkdir storage\\backups");
    #else
        system("mkdir -p storage/backups");
    #endif

    const DBConfig *dbcfg = db_get_config();
    snprintf(cmd, sizeof(cmd),
             "mysqldump -h %s -u %s -p%s %s > \"%s\" 2>&1",
             dbcfg->host, dbcfg->user, dbcfg->password, dbcfg->database, filepath);

    int rc = system(cmd);

    /* 计算文件大小 */
    long fsize = 0;
    FILE *fp = fopen(filepath, "rb");
    if (fp) { fseek(fp, 0, SEEK_END); fsize = ftell(fp); fclose(fp); }

    const char *status = (rc == 0 && fsize > 0) ? "success" : "failed";

    char *ef = db_escape(filepath);
    db_execute("INSERT INTO backup_records (backup_type, file_path, file_size, operator_id, status) "
               "VALUES ('manual','%s',%ld,%d,'%s')",
               ef, fsize, ctx->user->user_id, status);
    free(ef);

    db_execute("INSERT INTO logs (user_id, action, target_type, target_id, detail) "
               "VALUES (%d, 'db_backup', 'backup_records', %lu, '手动数据库备份')",
               ctx->user->user_id, db_last_insert_id());

    if (rc == 0 && fsize > 0)
        api_ok_msg(ctx, "备份成功");
    else
        api_error(ctx, ERR_SYSTEM, "备份失败：请确认 mysqldump 在 PATH 中且账号密码正确");
}

/* GET /api/school/backups  历史备份列表 */
void sch_backup_list(ApiContext *ctx) {
    if (!api_require_school_admin(ctx)) return;
    MYSQL_RES *res = db_query(
        "SELECT b.backup_id, b.backup_type, b.file_path, b.file_size, "
        "COALESCE(u.real_name,'—') AS operator, b.status, b.created_at "
        "FROM backup_records b LEFT JOIN users u ON b.operator_id=u.user_id "
        "ORDER BY b.created_at DESC LIMIT 50");
    api_send_result_data(ctx, res);
}

#endif /* 备用代码结束 */
