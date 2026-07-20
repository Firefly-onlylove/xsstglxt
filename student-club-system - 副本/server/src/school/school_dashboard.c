/**
 * school_dashboard.c — 学校端 仪表盘
 * 学生社团管理系统 SCMS（Web 架构 · 学校端）
 *
 * 覆盖路由：
 *   GET /api/school/dashboard    → sch_dashboard        仪表盘数据卡片
 *
 * 启用方法：把下面的 #if 0 改成 #if 1。
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

/* 计算本学期开始日期 */
static void get_semester_start(char *out, size_t out_size) {
    /* 先尝试从配置中读取 */
    MYSQL_RES *sr = db_query(
        "SELECT config_value FROM system_config WHERE config_key='semester_start'");
    if (sr && mysql_num_rows(sr) > 0) {
        MYSQL_ROW r = mysql_fetch_row(sr);
        if (r && r[0] && !utils_is_empty(r[0])) {
            utils_strlcpy(out, r[0], out_size);
            mysql_free_result(sr);
            return;
        }
    }
    if (sr) mysql_free_result(sr);

    /* 无配置则按月份判断：9月~次年8月 = 本学期从9/1开始 */
    MYSQL_RES *dr = db_query("SELECT YEAR(NOW()), MONTH(NOW())");
    if (dr) {
        MYSQL_ROW r = mysql_fetch_row(dr);
        if (r && r[0] && r[1]) {
            int year = atoi(r[0]), month = atoi(r[1]);
            if (month >= 9) snprintf(out, out_size, "%d-09-01", year);
            else snprintf(out, out_size, "%d-01-01", year);
            mysql_free_result(dr);
            return;
        }
        mysql_free_result(dr);
    }
    utils_strlcpy(out, "2026-01-01", out_size);
}

/* GET /api/school/dashboard
 * 返回数据卡片 + 最近日志 + 各学院分布 + 财务汇总。 */
void sch_dashboard(ApiContext *ctx) {
    if (!api_require_school_admin(ctx)) return;

    int active_clubs = db_query_int(
        "SELECT COUNT(*) FROM clubs WHERE status='approved'");
    int total_members = db_query_int(
        "SELECT COUNT(*) FROM members WHERE left_at IS NULL AND join_status='approved'");
    int monthly_activities = db_query_int(
        "SELECT COUNT(*) FROM activities WHERE status IN ('published','ongoing','completed')");
    int pending_approvals = db_query_int(
        "SELECT COUNT(*) FROM clubs WHERE status='pending'");

    /* 本学期开始日期 */
    char semester_start[11] = "";
    get_semester_start(semester_start, sizeof(semester_start));

    /* 本学期全校社团总收入 */
    double total_income = db_query_double(
        "SELECT COALESCE(SUM(amount),0) FROM finance "
        "WHERE type='income' AND record_time >= '%s 00:00:00'", semester_start);

    /* 全校总支出 */
    double total_expense = db_query_double(
        "SELECT COALESCE(SUM(amount),0) FROM finance WHERE type='expense'");

    /* 待学校终审批的报销数（学院已批）*/
    int pending_reimb = db_query_int(
        "SELECT COUNT(*) FROM reimbursement r "
        "JOIN clubs c ON r.club_id=c.club_id "
        "WHERE c.level='college' AND r.college_reviewed='approved' AND r.status='pending'");

    /* 已通过报销数 */
    int approved_reimb = db_query_int(
        "SELECT COUNT(*) FROM reimbursement WHERE status='approved'");

    /* 最近20条操作日志 */
    MYSQL_RES *log_res = db_query(
        "SELECT u.real_name AS username, l.action, l.created_at "
        "FROM logs l LEFT JOIN users u ON l.user_id=u.user_id "
        "ORDER BY l.created_at DESC LIMIT 20");
    char *logs_json = db_result_to_json_array(log_res);
    mysql_free_result(log_res);

    JsonBuilder jb;
    json_init(&jb);
    json_add_int(&jb, "active_clubs", active_clubs);
    json_add_int(&jb, "total_members", total_members);
    json_add_int(&jb, "monthly_activities", monthly_activities);
    json_add_int(&jb, "pending_approvals", pending_approvals);
    json_add_str(&jb, "semester_start", semester_start);
    json_add_double(&jb, "total_income", total_income);
    json_add_double(&jb, "total_expense", total_expense);
    json_add_int(&jb, "pending_reimb", pending_reimb);
    json_add_int(&jb, "approved_reimb", approved_reimb);
    json_add_raw(&jb, "recent_logs", logs_json ? logs_json : "[]");
    api_ok_data(ctx, json_finish(&jb));
    json_free(&jb);
    free(logs_json);
}

#endif /* 备用代码结束 */
