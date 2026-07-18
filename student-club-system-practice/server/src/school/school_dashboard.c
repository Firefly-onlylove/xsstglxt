/**
 * school_dashboard.c — 学校端 仪表盘 + 全校统计
 * 学生社团管理系统 SCMS（Web 架构 · 学校端）
 *
 * ┌──────────────────────────────────────────────────────────────────┐
 * │  ★ 备用参考实现。默认整份被 #if 0 关闭，不参与编译（走弱符号占位）。 │
 * │  ★ 启用方法：把下面这行的  #if 0  改成  #if 1  ，重新 make 即可。   │
 * │  ★ 启用后本文件的强符号会自动覆盖 handlers_stub.c 的弱符号。        │
 * └──────────────────────────────────────────────────────────────────┘
 *
 * 覆盖路由：
 *   GET /api/school/dashboard    → sch_dashboard        仪表盘数据卡片
 *   GET /api/school/stats        → sch_stats            分类/学院统计
 *   GET /api/school/stats/export → sch_stats_export     导出 CSV
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

/* GET /api/school/dashboard
 * 返回四张数据卡片 + 最近日志 + 各学院社团分布。 */
void sch_dashboard(ApiContext *ctx) {
    if (!api_require_school_admin(ctx)) return;

    int total_clubs = db_query_int(
        "SELECT COUNT(*) FROM clubs WHERE status='approved'");
    int total_members = db_query_int(
        "SELECT COUNT(*) FROM members WHERE left_at IS NULL AND join_status='approved'");
    int total_activities = db_query_int(
        "SELECT COUNT(*) FROM activities WHERE status IN ('published','ongoing','completed')");
    int pending_clubs = db_query_int(
        "SELECT COUNT(*) FROM clubs WHERE status='pending'");

    JsonBuilder jb;
    json_init(&jb);
    json_add_int(&jb, "total_clubs", total_clubs);
    json_add_int(&jb, "total_members", total_members);
    json_add_int(&jb, "total_activities", total_activities);
    json_add_int(&jb, "pending_clubs", pending_clubs);
    api_ok_data(ctx, json_finish(&jb));
    json_free(&jb);
}

/* GET /api/school/stats
 * 分类统计 + 级别统计 + 各学院社团分布，一次性返回。 */
void sch_stats(ApiContext *ctx) {
    if (!api_require_school_admin(ctx)) return;

    /* 直接把三段查询结果拼成一个大对象返回 */
    /* 1) 各学院社团分布 */
    MYSQL_RES *res = db_query(
        "SELECT c.college_name, COUNT(cl.club_id) AS club_count, "
        "COALESCE(SUM(cl.member_count),0) AS member_count "
        "FROM colleges c "
        "LEFT JOIN clubs cl ON c.college_id=cl.college_id AND cl.status='approved' "
        "WHERE c.status=1 GROUP BY c.college_id ORDER BY club_count DESC");
    /* 用 data 直接回传数组即可，前端拿去画图 */
    api_send_result_data(ctx, res);
}

/* GET /api/school/stats/export
 * 导出全校社团统计为 CSV 到 exports/ 目录，返回文件名。 */
void sch_stats_export(ApiContext *ctx) {
    if (!api_require_school_admin(ctx)) return;

    char fname[128];
    char ts[20];
    utils_now_datetime(ts);
    /* 把时间里的冒号/空格换成下划线，做文件名 */
    for (char *p = ts; *p; ++p) if (*p == ':' || *p == ' ' || *p == '-') *p = '_';
    snprintf(fname, sizeof(fname), "storage/exports/club_stats_%s.csv", ts);

    FILE *fp = fopen(fname, "w");
    if (!fp) { api_error(ctx, ERR_SYSTEM, "无法创建导出文件"); return; }

    /* UTF-8 BOM，方便 Excel 正确识别中文 */
    fputc(0xEF, fp); fputc(0xBB, fp); fputc(0xBF, fp);
    fprintf(fp, "社团名称,级别,所属学院,成员数,活动数,总收入,总支出,结余\n");

    MYSQL_RES *res = db_query(
        "SELECT c.club_name, c.level, COALESCE(col.college_name,'-'), c.member_count, "
        "COUNT(DISTINCT a.activity_id) AS act_cnt, "
        "COALESCE(SUM(CASE WHEN f.type='income' THEN f.amount ELSE 0 END),0), "
        "COALESCE(SUM(CASE WHEN f.type='expense' THEN f.amount ELSE 0 END),0) "
        "FROM clubs c "
        "LEFT JOIN colleges col ON c.college_id=col.college_id "
        "LEFT JOIN activities a ON c.club_id=a.club_id "
        "LEFT JOIN finance f ON c.club_id=f.club_id "
        "WHERE c.status='approved' GROUP BY c.club_id ORDER BY c.level, c.club_id");
    if (res) {
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(res)) != NULL) {
            double inc = atof(row[5]), exp = atof(row[6]);
            fprintf(fp, "%s,%s,%s,%s,%s,%.2f,%.2f,%.2f\n",
                    row[0], strcmp(row[1], "school") == 0 ? "校级" : "院级",
                    row[2], row[3], row[4], inc, exp, inc - exp);
        }
        mysql_free_result(res);
    }
    fclose(fp);

    JsonBuilder jb;
    json_init(&jb);
    json_add_str(&jb, "file", fname);
    api_ok_data(ctx, json_finish(&jb));
    json_free(&jb);
}

#endif /* 备用代码结束 */
