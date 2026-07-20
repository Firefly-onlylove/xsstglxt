/**
 * college_dashboard.c — 学院端 仪表盘 + 统计 + 导出
 * 学生社团管理系统 SCMS（Web 架构）
 *
 * 对应处理函数：col_dashboard / col_stats / col_stats_export
 * 对应前端页面：C1 学院管理员主页
 *
 * ┌──────────────────────────────────────────────────────────────┐
 * │  本文件当前被 #if 0 整体禁用（备用状态）。                     │
 * │  启用方法：把下面第一行的  #if 0  改成  #if 1  即可正常编译。   │
 * │  （无需逐行取消注释——本文件内部还有 / * * / 注释，用块注释    │
 * │    包裹会因嵌套失效，所以统一用 #if 0 整体开关。）             │
 * └──────────────────────────────────────────────────────────────┘
 *
 * 学院端权限要点：所有数据都限定在【本学院】范围内，
 * 用 ctx->user->college_id 作为过滤条件，不能看到别的学院的数据。
 */
#if 1

#include "../../include/api.h"
#include "../../include/db.h"
#include "../../include/common.h"
#include "../../include/routes.h"
#include "../../include/utils.h"

#include <stdio.h>

/* GET /api/college/dashboard —— 本院数据概览卡片 */
void col_dashboard(ApiContext *ctx) {
    if (!api_require_college_admin(ctx)) return;
    int cid = ctx->user->college_id;

    /* 本院社团数（院级 + 已通过） */
    int club_cnt = db_query_int(
        "SELECT COUNT(*) FROM clubs "
        "WHERE college_id=%d AND status='approved'", cid);

    /* 本院学生总数（按学院维度统计 users 表中的在册学生） */
    int student_cnt = db_query_int(
        "SELECT COUNT(*) FROM users "
        "WHERE college_id=%d AND is_active=1", cid);

    /* 本院社团活动总场次 */
    int act_cnt = db_query_int(
        "SELECT COUNT(*) FROM activities a "
        "JOIN clubs c ON a.club_id=c.club_id "
        "WHERE c.college_id=%d AND a.status IN "
        "('published','ongoing','completed')", cid);

    /* 待审批报销数（本院院级社团，待学院初审：status=pending AND college_reviewed=pending） */
    int reimb_cnt = db_query_int(
        "SELECT COUNT(*) FROM reimbursement r "
        "JOIN clubs c ON r.club_id=c.club_id "
        "WHERE c.college_id=%d AND r.status='pending' AND r.college_reviewed='pending'", cid);

    JsonBuilder jb;
    json_init(&jb);
    json_add_int(&jb, "club_count", club_cnt);
    json_add_int(&jb, "student_count", student_cnt);
    json_add_int(&jb, "activity_count", act_cnt);
    json_add_int(&jb, "pending_reimb", reimb_cnt);
    api_ok_data(ctx, json_finish(&jb));
    json_free(&jb);
}

/* GET /api/college/stats —— 本院分类统计（供图表用） */
void col_stats(ApiContext *ctx) {
    if (!api_require_college_admin(ctx)) return;
    int cid = ctx->user->college_id;

    /* 直接把"本院各社团的成员/活动数"作为数组返回，前端画柱状图 */
    MYSQL_RES *res = db_query(
        "SELECT c.club_id, c.club_name, c.category, c.member_count, "
        "(SELECT COUNT(*) FROM activities a WHERE a.club_id=c.club_id "
        " AND a.status IN ('published','ongoing','completed')) AS activity_count "
        "FROM clubs c "
        "WHERE c.college_id=%d AND c.status='approved' "
        "ORDER BY c.member_count DESC", cid);
    api_send_result_data(ctx, res);
}

/* GET /api/college/stats/export —— 导出本院统计为 CSV 文件 */
void col_stats_export(ApiContext *ctx) {
    if (!api_require_college_admin(ctx)) return;
    int cid = ctx->user->college_id;

    MYSQL_RES *res = db_query(
        "SELECT c.club_name, c.category, c.member_count, "
        "COALESCE((SELECT SUM(amount) FROM finance WHERE club_id=c.club_id AND type='income'),0), "
        "COALESCE((SELECT SUM(amount) FROM finance WHERE club_id=c.club_id AND type='expense'),0) "
        "FROM clubs c WHERE c.college_id=%d AND c.status='approved' "
        "ORDER BY c.club_id", cid);
    if (!res) { api_error(ctx, ERR_DB, "查询失败"); return; }

    /* 写 CSV 到 exports/ 目录 */
    char fname[128];
    char now[20];
    utils_now_datetime(now);
    /* 把时间里的空格和冒号换成下划线，作文件名 */
    for (char *p = now; *p; ++p) if (*p == ' ' || *p == ':') *p = '_';
    snprintf(fname, sizeof(fname), "storage/exports/college_%d_stats_%s.csv", cid, now);

    FILE *fp = fopen(fname, "w");
    if (!fp) { mysql_free_result(res); api_error(ctx, ERR_SYSTEM, "无法创建导出文件"); return; }

    /* UTF-8 BOM，方便 Excel 正确识别中文 */
    fputs("\xEF\xBB\xBF", fp);
    fputs("社团名称,类别,成员数,总收入,总支出\n", fp);

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res)) != NULL) {
        fprintf(fp, "%s,%s,%s,%s,%s\n",
                utils_safe(row[0]), utils_safe(row[1]), utils_safe(row[2]),
                utils_safe(row[3]), utils_safe(row[4]));
    }
    fclose(fp);
    mysql_free_result(res);

    JsonBuilder jb;
    json_init(&jb);
    json_add_str(&jb, "file", fname);
    api_ok_data(ctx, json_finish(&jb));
    json_free(&jb);
}

#endif
