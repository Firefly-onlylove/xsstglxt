/**
 * system_config.c — 学校端 系统配置（学期等全局参数）
 * 学生社团管理系统 SCMS（Web 架构 · 学校端）
 *
 * ┌──────────────────────────────────────────────────────────────────┐
 * │  ★ 备用参考实现。默认整份被 #if 0 关闭，不参与编译（走弱符号占位）。 │
 * │  ★ 启用方法：把下面这行的  #if 0  改成  #if 1  ，重新 make 即可。   │
 * └──────────────────────────────────────────────────────────────────┘
 *
 * 覆盖路由：
 *   GET  /api/school/config  → sch_config_get  读取全部键值
 *   POST /api/school/config  → sch_config_set  设置某个键值（存在则更新，不存在则插入）
 *
 * system_config 表结构：config_key(唯一) / config_value / description。
 * 常用键：current_semester（当前学期）、current_term_year（学年）、
 *        election_notice_days（任期到期提前提醒天数）等。
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

/* GET /api/school/config */
void sch_config_get(ApiContext *ctx) {
    if (!api_require_school_admin(ctx)) return;
    MYSQL_RES *res = db_query(
        "SELECT config_id, config_key, config_value, description, updated_at "
        "FROM system_config ORDER BY config_key");
    api_send_result_data(ctx, res);
}

/* POST /api/school/config  body: 整个配置对象 {system_name:"", max_members_school:60, ...} */
void sch_config_set(ApiContext *ctx) {
    if (!api_require_school_admin(ctx)) return;

    /* 前端字段名 → 数据库 config_key / description 映射 */
    struct {
        const char *field;      /* 前端 JSON 字段名 */
        const char *config_key; /* system_config.config_key */
        const char *desc;       /* 写入 description 列 */
    } map[] = {
        {"system_name",          "system_name",               "系统名称"},
        {"max_members_school",   "club_member_limit_school",  "校级社团人数上限"},
        {"max_members_college",  "club_member_limit_college", "院级社团人数上限"},
        {"max_members_per_club", "club_member_limit_global",  "社团成员上限（全局）"},
        {NULL, NULL, NULL}
    };

    char buf[1001];
    int any_change = 0;

    for (int i = 0; map[i].field; i++) {
        /* 尝试从 JSON body 读取该字段；未传或不存在则跳过 */
        if (!api_get_json_str(ctx, map[i].field, buf, sizeof(buf))) continue;
        if (!buf[0]) continue;  /* 跳过空值（如空的 system_name） */

        char *ek = db_escape(map[i].config_key);
        char *ev = db_escape(buf);
        char *ed = db_escape(map[i].desc);
        /* 存在即更新，不存在即插入（依赖 config_key 的 UNIQUE 约束） */
        int ok = db_execute(
            "INSERT INTO system_config (config_key, config_value, description) "
            "VALUES ('%s','%s','%s') "
            "ON DUPLICATE KEY UPDATE config_value=VALUES(config_value), "
            "description=VALUES(description)",
            ek, ev, ed);
        free(ek); free(ev); free(ed);
        if (ok >= 0) any_change = 1;
    }

    if (any_change) {
        db_execute("INSERT INTO logs (user_id, action, target_type, detail) "
                   "VALUES (%d, 'set_config', 'system_config', '修改系统配置')",
                   ctx->user->user_id);
    }
    api_ok_msg(ctx, "配置已保存");
}

#endif /* 备用代码结束 */
