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
#if 1

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

    /* ── 4 个汇总卡片 ── */
    int total_clubs = db_query_int(
        "SELECT COUNT(*) FROM clubs WHERE status='approved'");
    int total_members = db_query_int(
        "SELECT COUNT(*) FROM members WHERE left_at IS NULL AND join_status='approved'");
    int total_activities = db_query_int(
        "SELECT COUNT(*) FROM activities");
    int new_clubs_this_month = db_query_int(
        "SELECT COUNT(*) FROM clubs WHERE status='approved' "
        "AND created_at >= DATE_FORMAT(NOW(), '%%Y-%%m-01')");

    JsonBuilder jb; json_init(&jb);
    json_add_int(&jb, "total_clubs",           total_clubs);
    json_add_int(&jb, "total_members",         total_members);
    json_add_int(&jb, "total_activities",      total_activities);
    json_add_int(&jb, "new_clubs_this_month",  new_clubs_this_month);

    /* college_stats — 各学院社团数 / 成员数 / 活动数 */
    {
        MYSQL_RES *r = db_query(
            "SELECT col.college_name, "
            "COUNT(DISTINCT cl.club_id) AS club_count, "
            "COALESCE(SUM(cl.member_count),0) AS member_count, "
            "COUNT(DISTINCT a.activity_id) AS activity_count "
            "FROM colleges col "
            "LEFT JOIN clubs cl ON col.college_id=cl.college_id AND cl.status='approved' "
            "LEFT JOIN activities a ON cl.club_id=a.club_id "
            "WHERE col.status=1 GROUP BY col.college_id ORDER BY club_count DESC");
        char *arr = db_result_to_json_array(r);
        json_add_raw(&jb, "college_stats", arr ? arr : "[]");
        if (r) mysql_free_result(r);
        free(arr);
    }

    /* top_clubs — 社团成员规模排名 TOP 10 */
    {
        MYSQL_RES *r = db_query(
            "SELECT c.club_name AS name, "
            "COALESCE(col.college_name,'—') AS college_name, "
            "c.member_count "
            "FROM clubs c "
            "LEFT JOIN colleges col ON c.college_id=col.college_id "
            "WHERE c.status='approved' ORDER BY c.member_count DESC LIMIT 10");
        char *arr = db_result_to_json_array(r);
        json_add_raw(&jb, "top_clubs", arr ? arr : "[]");
        if (r) mysql_free_result(r);
        free(arr);
    }

    /* type_distribution — 社团类型分布（含百分比） */
    {
        MYSQL_RES *r = db_query(
            "SELECT category AS club_type, COUNT(*) AS count, "
            "CONCAT(ROUND(COUNT(*)*100.0/"
            "(SELECT COUNT(*) FROM clubs WHERE status='approved'),1),'%%') "
            "AS percentage "
            "FROM clubs WHERE status='approved' GROUP BY category ORDER BY count DESC");
        char *arr = db_result_to_json_array(r);
        json_add_raw(&jb, "type_distribution", arr ? arr : "[]");
        if (r) mysql_free_result(r);
        free(arr);
    }

    /* monthly_activities — 近 12 月活动趋势（含参与人次） */
    {
        MYSQL_RES *r = db_query(
            "SELECT DATE_FORMAT(a.start_time,'%%Y-%%m') AS month, "
            "COUNT(DISTINCT a.activity_id) AS activity_count, "
            "COUNT(r.registration_id) AS participant_count "
            "FROM activities a "
            "LEFT JOIN registrations r ON a.activity_id=r.activity_id "
            "WHERE a.start_time >= DATE_SUB(NOW(), INTERVAL 12 MONTH) "
            "GROUP BY month ORDER BY month DESC");
        char *arr = db_result_to_json_array(r);
        json_add_raw(&jb, "monthly_activities", arr ? arr : "[]");
        if (r) mysql_free_result(r);
        free(arr);
    }

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
