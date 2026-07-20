/**
 * finance_oversight.c — 学校端 全校财务监督 + 校级报销终审
 * 学生社团管理系统 SCMS（Web 架构 · 学校端）
 *
 * 覆盖路由：
 *   GET  /api/school/finance                       → sch_finance_overview 全校财务概况
 *   GET  /api/school/finance/{club_id}             → sch_finance_club      某社团明细
 *   GET  /api/school/reimbursements/pending        → sch_reimb_pending     待学校审批的报销
 *   GET  /api/school/reimbursements/college        → sch_reimb_college_list  学院报销记录汇总
 *   POST /api/school/reimbursements/{id}/approve   → sch_reimb_approve     学校终审通过
 *   POST /api/school/reimbursements/{id}/reject    → sch_reimb_reject      学校驳回
 *
 * 二级审批流程：
 *   社团提交(status=pending, college_reviewed=pending)
 *   → 学院审批通过(college_reviewed='approved', 状态不变)
 *   → 学校终审通过(status='approved') → 自动记 finance(type='expense')
 *
 * 启用方法：把下面的 #if 0 改成 #if 1。
 */
#if 1

#include "../../include/routes.h"
#include "../../include/api.h"
#include "../../include/db.h"
#include "../../include/common.h"
#include "../../include/utils.h"
#include "../../include/notification.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* GET /api/school/finance  全校财务概况（卡片 + 各学院 + 各社团）
 * 返回: total_income(本学期收入), total_expense(总支出), pending_reimb, approved_reimb,
 *       colleges(学院财务汇总), clubs(社团财务明细) */
void sch_finance_overview(ApiContext *ctx) {
    if (!api_require_school_admin(ctx)) return;

    /* 本学期开始日期（上半年：1月，下半年：9月）*/
    int year, month;
    char semester_start[11] = "";
    MYSQL_RES *sr = db_query("SELECT config_value FROM system_config WHERE config_key='semester_start'");
    if (sr && mysql_num_rows(sr) > 0) {
        MYSQL_ROW r = mysql_fetch_row(sr);
        if (r && r[0]) { utils_strlcpy(semester_start, r[0], sizeof(semester_start)); }
    }
    if (sr) mysql_free_result(sr);
    /* 如果未配学期开始日，则根据当前月份计算 */
    if (utils_is_empty(semester_start)) {
        MYSQL_RES *dr = db_query("SELECT NOW()");
        if (dr) {
            MYSQL_ROW r = mysql_fetch_row(dr);
            if (r && r[0]) {
                sscanf(r[0], "%d-%d", &year, &month);
                if (month >= 9) snprintf(semester_start, 11, "%d-09-01", year);
                else snprintf(semester_start, 11, "%d-01-01", year);
            }
            mysql_free_result(dr);
        }
    }
    /* 确保有兜底值 */
    if (utils_is_empty(semester_start)) utils_strlcpy(semester_start, "2026-01-01", 11);

    /* 本学期全校社团总收入（finance 表中 type='income' 且在本学期）*/
    double total_income = db_query_double(
        "SELECT COALESCE(SUM(amount),0) FROM finance "
        "WHERE type='income' AND record_time >= '%s 00:00:00'",
        semester_start);

    /* 全校总支出（finance 表中所有 type='expense'）*/
    double total_expense = db_query_double(
        "SELECT COALESCE(SUM(amount),0) FROM finance WHERE type='expense'");

    /* 待学校终审数量（学院已批，等待学校审批）*/
    int pending_reimb = db_query_int(
        "SELECT COUNT(*) FROM reimbursement r "
        "JOIN clubs c ON r.club_id=c.club_id "
        "WHERE c.level='college' AND r.college_reviewed='approved' AND r.status='pending'");

    /* 学校已终审通过数量 */
    int approved_reimb = db_query_int(
        "SELECT COUNT(*) FROM reimbursement WHERE status='approved'");

    JsonBuilder jb; json_init(&jb);
    json_add_str(&jb, "semester_start", semester_start);
    json_add_double(&jb, "total_income", total_income);
    json_add_double(&jb, "total_expense", total_expense);
    json_add_int(&jb, "pending_reimb", pending_reimb);
    json_add_int(&jb, "approved_reimb", approved_reimb);

    /* 各学院财务汇总 */
    {
        MYSQL_RES *r = db_query(
            "SELECT col.college_name, "
            "COALESCE(SUM(CASE WHEN f.type='income' THEN f.amount ELSE 0 END),0) AS total_income, "
            "COALESCE(SUM(CASE WHEN f.type='expense' THEN f.amount ELSE 0 END),0) AS total_expense, "
            "COALESCE(SUM(CASE WHEN f.type='income' THEN f.amount ELSE 0 END),0) "
            "- COALESCE(SUM(CASE WHEN f.type='expense' THEN f.amount ELSE 0 END),0) AS balance, "
            "COUNT(DISTINCT cl.club_id) AS club_count "
            "FROM colleges col "
            "LEFT JOIN clubs cl ON col.college_id=cl.college_id AND cl.status='approved' "
            "LEFT JOIN finance f ON cl.club_id=f.club_id "
            "WHERE col.status=1 GROUP BY col.college_id ORDER BY total_income DESC");
        char *arr = db_result_to_json_array(r);
        json_add_raw(&jb, "colleges", arr ? arr : "[]");
        if (r) mysql_free_result(r);
        free(arr);
    }

    /* 各社团财务明细 */
    {
        MYSQL_RES *r = db_query(
            "SELECT c.club_id, c.club_name, "
            "COALESCE(col.college_name,'—') AS college_name, "
            "COALESCE((SELECT SUM(amount) FROM finance WHERE club_id=c.club_id AND type='income'),0) AS total_income, "
            "COALESCE((SELECT SUM(amount) FROM finance WHERE club_id=c.club_id AND type='expense'),0) AS total_expense, "
            "COALESCE((SELECT SUM(amount) FROM finance WHERE club_id=c.club_id AND type='income'),0) "
            "- COALESCE((SELECT SUM(amount) FROM finance WHERE club_id=c.club_id AND type='expense'),0) AS balance "
            "FROM clubs c "
            "LEFT JOIN colleges col ON c.college_id=col.college_id "
            "WHERE c.status='approved' ORDER BY c.level, c.club_id");
        char *arr = db_result_to_json_array(r);
        json_add_raw(&jb, "clubs", arr ? arr : "[]");
        if (r) mysql_free_result(r);
        free(arr);
    }

    api_ok_data(ctx, json_finish(&jb));
    json_free(&jb);
}

/* GET /api/school/finance/{club_id}  某社团财务明细 */
void sch_finance_club(ApiContext *ctx) {
    if (!api_require_school_admin(ctx)) return;
    int club_id = api_get_path_int(ctx, 2);
    if (club_id <= 0) { api_error(ctx, ERR_INPUT, "社团ID非法"); return; }

    MYSQL_RES *res = db_query(
        "SELECT f.finance_id, f.type, f.amount, f.source, f.description, "
        "COALESCE(u.real_name,'—') AS operator_name, f.record_time "
        "FROM finance f LEFT JOIN users u ON f.operator_id=u.user_id "
        "WHERE f.club_id=%d ORDER BY f.record_time DESC LIMIT 100", club_id);
    api_send_result_data(ctx, res);
}

/* GET /api/school/reimbursements/pending
 * 显示已通过学院审批、等待学校终审的报销（college_reviewed='approved' AND status='pending'）
 * 可选 status 参数：
 *   pending（默认）= 学院已批，待学校终审
 *   approved = 学校已终审通过
 *   rejected = 学校已驳回
 */
void sch_reimb_pending(ApiContext *ctx) {
    if (!api_require_school_admin(ctx)) return;
    char status[32] = "";
    api_get_param(ctx, "status", status, sizeof(status));
    MYSQL_RES *res;
    if (strcmp(status, "approved") == 0) {
        /* 学校已终审通过 */
        res = db_query(
            "SELECT r.reimbursement_id, r.club_id, c.club_name, r.amount, r.description, "
            "r.receipt_path, COALESCE(u.real_name,'—') AS applicant_name, "
            "r.submitted_at, r.status, r.college_reviewed "
            "FROM reimbursement r "
            "JOIN clubs c ON r.club_id=c.club_id "
            "LEFT JOIN users u ON r.applicant_id=u.user_id "
            "WHERE c.level='college' AND r.status='approved' ORDER BY r.submitted_at DESC");
    } else if (strcmp(status, "rejected") == 0) {
        /* 学校已驳回 */
        res = db_query(
            "SELECT r.reimbursement_id, r.club_id, c.club_name, r.amount, r.description, "
            "r.receipt_path, COALESCE(u.real_name,'—') AS applicant_name, "
            "r.submitted_at, r.status, r.college_reviewed "
            "FROM reimbursement r "
            "JOIN clubs c ON r.club_id=c.club_id "
            "LEFT JOIN users u ON r.applicant_id=u.user_id "
            "WHERE c.level='college' AND r.status='rejected' ORDER BY r.submitted_at DESC");
    } else {
        /* 默认：待学校终审 = 学院已批但学校尚未处理的院级社团报销 */
        res = db_query(
            "SELECT r.reimbursement_id, r.club_id, c.club_name, r.amount, r.description, "
            "r.receipt_path, COALESCE(u.real_name,'—') AS applicant_name, "
            "r.submitted_at, r.status, r.college_reviewed "
            "FROM reimbursement r "
            "JOIN clubs c ON r.club_id=c.club_id "
            "LEFT JOIN users u ON r.applicant_id=u.user_id "
            "WHERE c.level='college' AND r.college_reviewed='approved' AND r.status='pending' "
            "ORDER BY r.submitted_at DESC");
    }
    api_send_result_data(ctx, res);
}

/* GET /api/school/reimbursements/college  学院报销记录汇总
 * 学校端查看所有院级社团报销的最终状态（仅 approved/rejected，不含 pending）*/
void sch_reimb_college_list(ApiContext *ctx) {
    if (!api_require_school_admin(ctx)) return;
    MYSQL_RES *res = db_query(
        "SELECT r.reimbursement_id, r.club_id, c.club_name, col.college_name, r.amount, "
        "r.description, COALESCE(u.real_name,'—') AS applicant_name, "
        "r.submitted_at, r.status, r.college_reviewed, r.review_comment "
        "FROM reimbursement r "
        "JOIN clubs c ON r.club_id=c.club_id "
        "JOIN colleges col ON c.college_id=col.college_id "
        "LEFT JOIN users u ON r.applicant_id=u.user_id "
        "WHERE c.level='college' AND r.status IN ('approved','rejected') "
        "ORDER BY r.submitted_at DESC");
    api_send_result_data(ctx, res);
}

/* POST /api/school/reimbursements/{id}/approve —— 学校终审通过
 * 将 status 设为 approved，同时将该笔金额记入 finance 表作为支出
 * 按之前讨论：只有学校最终审批后才算支出 */
void sch_reimb_approve(ApiContext *ctx) {
    if (!api_require_school_admin(ctx)) return;
    int rid = api_get_path_int(ctx, 2);
    if (rid <= 0) { api_error(ctx, ERR_INPUT, "报销ID非法"); return; }

    /* 获取报销详情（必须为学院已批、等待学校终审的状态）*/
    int applicant = 0;
    int club_id = 0;
    double amount = 0;
    char description[501] = "";
    MYSQL_RES *res = db_query(
        "SELECT applicant_id, club_id, amount, description FROM reimbursement "
        "WHERE reimbursement_id=%d AND college_reviewed='approved' AND status='pending'",
        rid);
    if (!res || mysql_num_rows(res) == 0) {
        if (res) mysql_free_result(res);
        api_error(ctx, ERR_NOT_FOUND, "报销不存在或尚未经过学院审批");
        return;
    }
    MYSQL_ROW row = mysql_fetch_row(res);
    applicant = row[0] ? atoi(row[0]) : 0;
    club_id = row[1] ? atoi(row[1]) : 0;
    amount = row[2] ? atof(row[2]) : 0;
    if (row[3]) utils_strlcpy(description, row[3], sizeof(description));
    mysql_free_result(res);

    /* 事务：更新状态 + 记入财务支出 */
    db_begin();

    int ok = db_execute(
        "UPDATE reimbursement SET status='approved', reviewer_id=%d, reviewed_at=NOW() "
        "WHERE reimbursement_id=%d AND college_reviewed='approved' AND status='pending'",
        ctx->user->user_id, rid);
    if (ok <= 0) { db_rollback(); api_error(ctx, ERR_STATUS, "审批失败，可能已被处理"); return; }

    /* 将报销金额记入 finance 表作为支出 */
    char *esc_desc = db_escape(description);
    db_execute(
        "INSERT INTO finance (club_id, type, amount, description, operator_id, record_time) "
        "VALUES (%d, 'expense', %.2f, '报销: %s', %d, NOW())",
        club_id, amount, esc_desc, ctx->user->user_id);
    free(esc_desc);

    db_commit();

    db_execute("INSERT INTO logs (user_id, action, target_type, target_id, detail) "
               "VALUES (%d, 'approve_reimbursement_final', 'reimbursement', %d, '学校终审通过报销，已记支出')",
               ctx->user->user_id, rid);
    notification_send(applicant, "报销申请已通过",
                      "您提交的报销申请已通过学校终审，金额已计入社团支出。",
                      "reimbursement_result", rid);
    api_ok_msg(ctx, "已通过（已记支出）");
}

/* POST /api/school/reimbursements/{id}/reject —— 学校驳回
 * body: comment（驳回理由）*/
void sch_reimb_reject(ApiContext *ctx) {
    if (!api_require_school_admin(ctx)) return;
    int rid = api_get_path_int(ctx, 2);
    if (rid <= 0) { api_error(ctx, ERR_INPUT, "报销ID非法"); return; }

    char comment[501] = "";
    api_get_json_str(ctx, "comment", comment, sizeof(comment));
    if (utils_is_empty(comment)) { api_error(ctx, ERR_INPUT, "请填写驳回原因"); return; }

    int applicant = db_query_int(
        "SELECT applicant_id FROM reimbursement "
        "WHERE reimbursement_id=%d AND college_reviewed='approved' AND status='pending'", rid);
    if (applicant == 0) { api_error(ctx, ERR_NOT_FOUND, "报销不存在或尚未经过学院审批"); return; }

    char *ec = db_escape(comment);
    int ok = db_execute(
        "UPDATE reimbursement SET status='rejected', reviewer_id=%d, review_comment='%s', "
        "reviewed_at=NOW() WHERE reimbursement_id=%d AND college_reviewed='approved' AND status='pending'",
        ctx->user->user_id, ec, rid);
    free(ec);
    if (ok <= 0) { api_error(ctx, ERR_STATUS, "驳回失败，可能已被处理"); return; }

    db_execute("INSERT INTO logs (user_id, action, target_type, target_id, detail) "
               "VALUES (%d, 'reject_reimbursement_final', 'reimbursement', %d, '学校终审驳回')",
               ctx->user->user_id, rid);
    notification_send(applicant, "报销申请被驳回", comment, "reimbursement_result", rid);
    api_ok_msg(ctx, "已驳回");
}

/* GET /api/school/finance/school-clubs  校级社团经费明细 */
void sch_finance_school_clubs(ApiContext *ctx) {
    if (!api_require_school_admin(ctx)) return;

    /* 每个校级社团单独查询汇总 */
    MYSQL_RES *r = db_query(
        "SELECT c.club_id, c.club_name, c.level, "
        "COALESCE(col.college_name,'—') AS college_name, "
        "COALESCE((SELECT SUM(amount) FROM finance WHERE club_id=c.club_id AND type='income'),0) AS total_income, "
        "COALESCE((SELECT SUM(amount) FROM finance WHERE club_id=c.club_id AND type='expense'),0) AS total_expense, "
        "COALESCE((SELECT SUM(amount) FROM finance WHERE club_id=c.club_id AND type='income'),0) "
        "- COALESCE((SELECT SUM(amount) FROM finance WHERE club_id=c.club_id AND type='expense'),0) AS balance "
        "FROM clubs c "
        "LEFT JOIN colleges col ON c.college_id=col.college_id "
        "WHERE c.level='school' AND c.status='approved' "
        "ORDER BY c.club_id");
    api_send_result_data(ctx, r);
}

#endif /* 备用代码结束 */
