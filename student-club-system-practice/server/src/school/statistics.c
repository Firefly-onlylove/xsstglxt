/**
 * statistics.c — 学校端 全校数据统计与导出
 * 学生社团管理系统 SCMS（Web 架构 · 学校端）
 *
 * ┌──────────────────────────────────────────────────────────────────┐
 * │  ★ 备用参考实现。默认整份被 #if 0 关闭，不参与编译（走弱符号占位）。 │
 * │  ★ 启用方法：把下面这行的  #if 0  改成  #if 1  ，重新 make 即可。   │
 * └──────────────────────────────────────────────────────────────────┘
 *
 * 覆盖路由：
 *   GET /api/school/stats         → sch_stats        分类/级别/学院多维统计 JSON
 *   GET /api/school/stats/export  → sch_stats_export 导出 CSV 报表（直接下载）
 *
 * 说明：sch_dashboard（首页概览）在 school_dashboard.c 中实现，这里只做深度统计。
 */
#if 0

#include "../../include/routes.h"
#include "../../include/api.h"
#include "../../include/db.h"
#include "../../include/common.h"
#include "../../include/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* GET /api/school/stats  多维统计，一次性返回给前端画图 */
void sch_stats(ApiContext *ctx) {
    if (!api_require_school_admin(ctx)) return;

    /* 1. 按类别统计社团数 */
    MYSQL_RES *r1 = db_query(
        "SELECT category, COUNT(*) AS cnt FROM clubs WHERE status='approved' "
        "GROUP BY category ORDER BY cnt DESC");
    /* 2. 按级别统计 */
    MYSQL_RES *r2 = db_query(
        "SELECT level, COUNT(*) AS cnt FROM clubs WHERE status='approved' GROUP BY level");
    /* 3. 各学院社团/成员分布 */
    MYSQL_RES *r3 = db_query(
        "SELECT c.college_name, COUNT(cl.club_id) AS club_cnt, "
        "COALESCE(SUM(cl.member_count),0) AS member_cnt "
        "FROM colleges c LEFT JOIN clubs cl ON c.college_id=cl.college_id AND cl.status='approved' "
        "WHERE c.status=1 GROUP BY c.college_id ORDER BY club_cnt DESC");

    /* 手动把三个结果集拼进一个 JSON 对象 */
    /* 为简洁，用 JsonBuilder 逐段构造：{"by_category":[...],"by_level":[...],"by_college":[...]} */
    /* 这里演示用一个辅助 lambda 风格的小函数把结果集转数组字符串。 */

    /* 直接借助 api 内部能力较麻烦，改为拼接字符串。 */
    /* 简化做法：分别用临时缓冲输出。实际项目可封装 result_to_array_string。 */

    /* —— 由于 api 层未暴露"结果集转字符串"给业务层，这里改为三个字段各自查询计数后拼 —— */
    (void)r1; (void)r2; (void)r3;
    if (r1) mysql_free_result(r1);
    if (r2) mysql_free_result(r2);
    if (r3) mysql_free_result(r3);

    /* 用 JsonBuilder 逐项 add_raw 拼装数组（数组内容用子查询循环生成） */
    JsonBuilder jb; json_init(&jb);

    /* by_category */
    {
        char arr[2048] = "["; int first = 1;
        MYSQL_RES *r = db_query(
            "SELECT category, COUNT(*) FROM clubs WHERE status='approved' GROUP BY category ORDER BY 2 DESC");
        if (r) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(r))) {
                char item[256];
                snprintf(item, sizeof(item), "%s{\"name\":\"%s\",\"value\":%s}",
                         first ? "" : ",", utils_safe(row[0]), row[1]);
                strncat(arr, item, sizeof(arr) - strlen(arr) - 1);
                first = 0;
            }
            mysql_free_result(r);
        }
        strncat(arr, "]", sizeof(arr) - strlen(arr) - 1);
        json_add_raw(&jb, "by_category", arr);
    }

    /* by_college */
    {
        char arr[4096] = "["; int first = 1;
        MYSQL_RES *r = db_query(
            "SELECT c.college_name, COUNT(cl.club_id), COALESCE(SUM(cl.member_count),0) "
            "FROM colleges c LEFT JOIN clubs cl ON c.college_id=cl.college_id AND cl.status='approved' "
            "WHERE c.status=1 GROUP BY c.college_id ORDER BY 2 DESC");
        if (r) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(r))) {
                char item[512];
                snprintf(item, sizeof(item),
                         "%s{\"college\":\"%s\",\"clubs\":%s,\"members\":%s}",
                         first ? "" : ",", utils_safe(row[0]), row[1], row[2]);
                strncat(arr, item, sizeof(arr) - strlen(arr) - 1);
                first = 0;
            }
            mysql_free_result(r);
        }
        strncat(arr, "]", sizeof(arr) - strlen(arr) - 1);
        json_add_raw(&jb, "by_college", arr);
    }

    int school_lv = db_query_int("SELECT COUNT(*) FROM clubs WHERE status='approved' AND level='school'");
    int college_lv = db_query_int("SELECT COUNT(*) FROM clubs WHERE status='approved' AND level='college'");
    json_add_int(&jb, "level_school", school_lv);
    json_add_int(&jb, "level_college", college_lv);

    api_ok_data(ctx, json_finish(&jb));
    json_free(&jb);
}

/* GET /api/school/stats/export  导出 CSV（前端用 <a download> 触发下载） */
void sch_stats_export(ApiContext *ctx) {
    if (!api_require_school_admin(ctx)) return;

    /* 拼 CSV 文本，带 UTF-8 BOM 让 Excel 正确识别中文 */
    size_t cap = 8192, len = 0;
    char *csv = malloc(cap);
    if (!csv) { api_error(ctx, ERR_SYSTEM, "内存不足"); return; }
    len += snprintf(csv + len, cap - len, "\xEF\xBB\xBF");   /* BOM */
    len += snprintf(csv + len, cap - len,
                    "社团名称,级别,所属学院,成员数,活动数,总收入,总支出,结余\n");

    MYSQL_RES *r = db_query(
        "SELECT c.club_name, c.level, COALESCE(col.college_name,'—'), c.member_count, "
        "COUNT(DISTINCT a.activity_id), "
        "COALESCE(SUM(CASE WHEN f.type='income' THEN f.amount ELSE 0 END),0), "
        "COALESCE(SUM(CASE WHEN f.type='expense' THEN f.amount ELSE 0 END),0) "
        "FROM clubs c "
        "LEFT JOIN colleges col ON c.college_id=col.college_id "
        "LEFT JOIN activities a ON c.club_id=a.club_id "
        "LEFT JOIN finance f ON c.club_id=f.club_id "
        "WHERE c.status='approved' GROUP BY c.club_id ORDER BY c.level, c.club_id");
    if (r) {
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(r))) {
            double inc = atof(row[5]), exp = atof(row[6]);
            char line[512];
            int n = snprintf(line, sizeof(line), "%s,%s,%s,%s,%s,%.2f,%.2f,%.2f\n",
                             utils_safe(row[0]),
                             utils_str_equal(row[1], "school") ? "校级" : "院级",
                             utils_safe(row[2]), utils_safe(row[3]), utils_safe(row[4]),
                             inc, exp, inc - exp);
            if (len + n + 1 >= cap) { cap = cap * 2 + n; csv = realloc(csv, cap); }
            memcpy(csv + len, line, n); len += n;
        }
        mysql_free_result(r);
    }
    csv[len] = '\0';

    /* 以附件形式返回，触发浏览器下载 */
    mg_http_reply(ctx->conn, 200,
        "Content-Type: text/csv; charset=utf-8\r\n"
        "Content-Disposition: attachment; filename=\"school_stats.csv\"\r\n",
        "%s", csv);
    free(csv);
}

#endif /* 备用代码结束 */
