/**
 * finance_oversight.c — 学校端 全校财务监督 + 校级报销审批
 * 学生社团管理系统 SCMS（Web 架构 · 学校端）
 *
 * ┌──────────────────────────────────────────────────────────────────┐
 * │  ★ 备用参考实现。默认整份被 #if 0 关闭，不参与编译（走弱符号占位）。 │
 * │  ★ 启用方法：把下面这行的  #if 0  改成  #if 1  ，重新 make 即可。   │
 * └──────────────────────────────────────────────────────────────────┘
 *
 * 覆盖路由：
 *   GET  /api/school/finance                       → sch_finance_overview 全校财务概况
 *   GET  /api/school/finance/{club_id}             → sch_finance_club      某社团明细
 *   GET  /api/school/reimbursements/pending        → sch_reimb_pending     待审校级报销
 *   POST /api/school/reimbursements/{id}/approve    → sch_reimb_approve
 *   POST /api/school/reimbursements/{id}/reject     → sch_reimb_reject
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

/* GET /api/school/finance  全校财务概况（卡片 + 各学院 + 各社团） */
void sch_finance_overview(ApiContext *ctx) {
    if (!api_require_school_admin(ctx)) return;

    /* ── 4 个汇总卡片 ── */
    double total_income   = db_query_double(
        "SELECT COALESCE(SUM(amount),0) FROM finance WHERE type='income'");
    double total_expense  = db_query_double(
        "SELECT COALESCE(SUM(amount),0) FROM finance WHERE type='expense'");
    int pending_reimb     = db_query_int(
        "SELECT COUNT(*) FROM reimbursement WHERE status='pending'");
    int approved_reimb    = db_query_int(
        "SELECT COUNT(*) FROM reimbursement WHERE status='approved'");

    JsonBuilder jb; json_init(&jb);
    json_add_double(&jb, "total_income",   total_income);
    json_add_double(&jb, "total_expense",  total_expense);
    json_add_int(&jb,    "pending_reimb",  pending_reimb);
    json_add_int(&jb,    "approved_reimb", approved_reimb);

    /* colleges — 按学院汇总财务（含余额） */
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

    /* clubs — 各社团财务明细（保留原有逐社团查询） */
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

/* GET /api/school/reimbursements/pending  待审批的校级社团报销 */
void sch_reimb_pending(ApiContext *ctx) {
    if (!api_require_school_admin(ctx)) return;
    MYSQL_RES *res = db_query(
        "SELECT r.reimbursement_id, r.club_id, c.club_name, r.amount, r.description, "
        "r.receipt_path, COALESCE(u.real_name,'—') AS applicant_name, r.submitted_at "
        "FROM reimbursement r "
        "JOIN clubs c ON r.club_id=c.club_id "
        "LEFT JOIN users u ON r.applicant_id=u.user_id "
        "WHERE c.level='school' AND r.status='pending' ORDER BY r.submitted_at");
    api_send_result_data(ctx, res);
}

/* POST /api/school/reimbursements/{id}/approve
 * 审批通过：触发器会自动累加周期已用额度。 */
void sch_reimb_approve(ApiContext *ctx) {
    if (!api_require_school_admin(ctx)) return;
    int rid = api_get_path_int(ctx, 2);
    if (rid <= 0) { api_error(ctx, ERR_INPUT, "报销ID非法"); return; }

    /* 取申请人以便通知 */
    int applicant = db_query_int(
        "SELECT applicant_id FROM reimbursement WHERE reimbursement_id=%d AND status='pending'", rid);
    if (applicant == 0) { api_error(ctx, ERR_NOT_FOUND, "报销不存在或已处理"); return; }

    int ok = db_execute(
        "UPDATE reimbursement SET status='approved', reviewer_id=%d, reviewed_at=NOW() "
        "WHERE reimbursement_id=%d AND status='pending'", ctx->user->user_id, rid);
    if (ok <= 0) { api_error(ctx, ERR_STATUS, "审批失败，可能已被处理"); return; }

    db_execute("INSERT INTO logs (user_id, action, target_type, target_id, detail) "
               "VALUES (%d, 'approve_reimbursement', 'reimbursement', %d, '审批通过校级报销')",
               ctx->user->user_id, rid);
    notification_send(applicant, "报销申请已通过",
                      "您提交的报销申请已通过学校审核。", "reimbursement_result", rid);
    api_ok_msg(ctx, "已通过");
}

/* POST /api/school/reimbursements/{id}/reject  body: comment */
void sch_reimb_reject(ApiContext *ctx) {
    if (!api_require_school_admin(ctx)) return;
    int rid = api_get_path_int(ctx, 2);
    if (rid <= 0) { api_error(ctx, ERR_INPUT, "报销ID非法"); return; }

    char comment[501] = "";
    api_get_json_str(ctx, "comment", comment, sizeof(comment));
    if (utils_is_empty(comment)) { api_error(ctx, ERR_INPUT, "请填写驳回原因"); return; }

    int applicant = db_query_int(
        "SELECT applicant_id FROM reimbursement WHERE reimbursement_id=%d AND status='pending'", rid);
    if (applicant == 0) { api_error(ctx, ERR_NOT_FOUND, "报销不存在或已处理"); return; }

    char *ec = db_escape(comment);
    int ok = db_execute(
        "UPDATE reimbursement SET status='rejected', reviewer_id=%d, review_comment='%s', "
        "reviewed_at=NOW() WHERE reimbursement_id=%d AND status='pending'",
        ctx->user->user_id, ec, rid);
    free(ec);
    if (ok <= 0) { api_error(ctx, ERR_STATUS, "驳回失败，可能已被处理"); return; }

    db_execute("INSERT INTO logs (user_id, action, target_type, target_id, detail) "
               "VALUES (%d, 'reject_reimbursement', 'reimbursement', %d, '驳回校级报销')",
               ctx->user->user_id, rid);
    notification_send(applicant, "报销申请被驳回", comment, "reimbursement_result", rid);
    api_ok_msg(ctx, "已驳回");
}

#endif /* 备用代码结束 */
