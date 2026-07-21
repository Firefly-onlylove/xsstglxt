/**
 * finance_approval.c — 学院端 院级社团报销审批 + 报销额度设定
 * 学生社团管理系统 SCMS（Web 架构）
 *
 * 对应处理函数：
 *   col_reimb_pending  GET  待审批报销列表
 *   col_reimb_approve  POST 通过报销（提交至学校终审）
 *   col_reimb_reject   POST 驳回报销
 *   col_limit_list     GET  本院社团报销额度列表
 *   col_limit_set      POST 设定某社团报销额度
 *
 * 启用方法：把下面的 #if 0 改成 #if 1。
 *
 * 权限要点：只能审批【本院院级社团】的报销；额度也只能给本院社团设。
 *
 * 二级审批流程：
 *   社团提交(status=pending, college_reviewed=pending)
 *   → 学院审批通过(status=approved) → 自动记finance支出 → 通知学校
 *   → 学院驳回(status=rejected)
 */
#if 1

#include "../../include/api.h"
#include "../../include/db.h"
#include "../../include/common.h"
#include "../../include/routes.h"
#include "../../include/utils.h"
#include "../../include/notification.h"

#include <stdio.h>

/* GET /api/college/reimbursements/pending —— 本院报销列表
 * 可选参数 status:
 *   pending = 社团刚提交的待审批（默认：college_reviewed='pending' AND status='pending'）
 *   approved = 学院已批过的（college_reviewed='approved'）
 *   rejected = 学院已驳回的（college_reviewed='rejected'）
 */
void col_reimb_pending(ApiContext *ctx) {
    if (!api_require_college_admin(ctx)) return;
    int cid = ctx->user->college_id;

    char status[32] = "";
    api_get_param(ctx, "status", status, sizeof(status));

    MYSQL_RES *res;
    if (strcmp(status, "approved") == 0) {
        /* 已通过——status='approved' */
        res = db_query(
            "SELECT r.reimbursement_id, r.club_id, c.club_name, r.amount, "
            "r.description, r.receipt_path, u.real_name AS applicant_name, "
            "r.submitted_at, r.status, r.college_reviewed, r.review_comment "
            "FROM reimbursement r "
            "JOIN clubs c ON r.club_id=c.club_id "
            "LEFT JOIN users u ON r.applicant_id=u.user_id "
            "WHERE c.college_id=%d AND c.level='college' "
            "AND r.status='approved' "
            "ORDER BY r.submitted_at DESC", cid);
    } else if (strcmp(status, "rejected") == 0) {
        /* 已驳回——status='rejected' */
        res = db_query(
            "SELECT r.reimbursement_id, r.club_id, c.club_name, r.amount, "
            "r.description, r.receipt_path, u.real_name AS applicant_name, "
            "r.submitted_at, r.status, r.college_reviewed, r.review_comment "
            "FROM reimbursement r "
            "JOIN clubs c ON r.club_id=c.club_id "
            "LEFT JOIN users u ON r.applicant_id=u.user_id "
            "WHERE c.college_id=%d AND c.level='college' "
            "AND r.status='rejected' "
            "ORDER BY r.submitted_at DESC", cid);
    } else if (strcmp(status, "history") == 0) {
        /* 已审批记录：所有学院已处理过的（college_reviewed=approved或rejected，或status=approved/rejected） */
        res = db_query(
            "SELECT r.reimbursement_id, r.club_id, c.club_name, r.amount, "
            "r.description, r.receipt_path, u.real_name AS applicant_name, "
            "r.submitted_at, r.status, r.college_reviewed, r.review_comment "
            "FROM reimbursement r "
            "JOIN clubs c ON r.club_id=c.club_id "
            "LEFT JOIN users u ON r.applicant_id=u.user_id "
            "WHERE c.college_id=%d AND c.level='college' "
            "AND (r.college_reviewed IN ('approved','rejected') OR r.status IN ('approved','rejected')) "
            "ORDER BY r.submitted_at DESC", cid);
    } else {
        /* 默认 pending：社团刚提交的，学院尚未处理 */
        res = db_query(
            "SELECT r.reimbursement_id, r.club_id, c.club_name, r.amount, "
            "r.description, r.receipt_path, u.real_name AS applicant_name, "
            "r.submitted_at, r.status, r.college_reviewed, r.review_comment "
            "FROM reimbursement r "
            "JOIN clubs c ON r.club_id=c.club_id "
            "LEFT JOIN users u ON r.applicant_id=u.user_id "
            "WHERE c.college_id=%d AND c.level='college' "
            "AND r.college_reviewed='pending' AND r.status='pending' "
            "ORDER BY r.submitted_at", cid);
    }
    api_send_result_data(ctx, res);
}

/* 确认某报销单属于本院院级社团且未处理，返回申请人 id（0=无权/不存在） */
static int check_reimb_owned(int reimbursement_id, int college_id, double *out_amount) {
    MYSQL_RES *res = db_query(
        "SELECT r.applicant_id, r.amount FROM reimbursement r "
        "JOIN clubs c ON r.club_id=c.club_id "
        "WHERE r.reimbursement_id=%d AND c.college_id=%d "
        "AND c.level='college' AND r.status='pending' AND r.college_reviewed='pending'",
        reimbursement_id, college_id);
    if (!res || mysql_num_rows(res) == 0) {
        if (res) mysql_free_result(res);
        return 0;
    }
    MYSQL_ROW row = mysql_fetch_row(res);
    int applicant = atoi(row[0]);
    if (out_amount) *out_amount = atof(row[1]);
    mysql_free_result(res);
    return applicant;
}

/* POST /api/college/reimbursements/{id}/approve —— 学院审批通过
 * 直接设为 status='approved'，无需学校端二次审批 */
void col_reimb_approve(ApiContext *ctx) {
    if (!api_require_college_admin(ctx)) return;
    int cid = ctx->user->college_id;
    int rid = api_get_path_int(ctx, 2);

    double amount = 0;
    int applicant = check_reimb_owned(rid, cid, &amount);
    if (applicant == 0) { api_error(ctx, ERR_PERMISSION, "无权审批或该报销已处理"); return; }

    /* 学院审批通过：直接完结，设 status='approved' */
    db_execute("UPDATE reimbursement SET status='approved', college_reviewed='approved', "
               "reviewer_id=%d, reviewed_at=NOW() WHERE reimbursement_id=%d",
               ctx->user->user_id, rid);
    db_execute("INSERT INTO logs (user_id, action, target_type, target_id, detail) "
               "VALUES (%d, 'approve_reimbursement', 'reimbursement', %d, '学院审批通过报销')",
               ctx->user->user_id, rid);

    /* 记录支出（finance 表记一笔 expense） */
    db_execute("INSERT INTO finance (club_id, type, amount, source, description, operator_id, record_time) "
               "SELECT r.club_id, 'expense', r.amount, 'reimbursement', r.description, %d, NOW() "
               "FROM reimbursement r WHERE r.reimbursement_id=%d",
               ctx->user->user_id, rid);

    /* 通知学校管理员 */
    notification_notify_school_admins("报销已审批",
        "一笔报销申请已由学院审批通过并记账。",
        "reimb_notify", rid);

    notification_send(applicant, "报销申请已通过",
        "您的报销申请已通过学院审批，款项已记录。",
        "reimbursement_result", rid);

    api_ok_msg(ctx, "已通过");
}

/* POST /api/college/reimbursements/{id}/reject —— 学院驳回（需原因） */
void col_reimb_reject(ApiContext *ctx) {
    if (!api_require_college_admin(ctx)) return;
    int cid = ctx->user->college_id;
    int rid = api_get_path_int(ctx, 2);

    char reason[501] = "";
    api_get_json_str(ctx, "reason", reason, sizeof(reason));
    if (utils_is_empty(reason)) { api_error(ctx, ERR_INPUT, "请填写驳回原因"); return; }

    int applicant = check_reimb_owned(rid, cid, NULL);
    if (applicant == 0) { api_error(ctx, ERR_PERMISSION, "无权审批或该报销已处理"); return; }

    char *e = db_escape(reason);
    db_execute("UPDATE reimbursement SET status='rejected', college_reviewed='rejected', "
               "reviewer_id=%d, review_comment='%s', reviewed_at=NOW() "
               "WHERE reimbursement_id=%d",
               ctx->user->user_id, e, rid);
    free(e);
    db_execute("INSERT INTO logs (user_id, action, target_type, target_id, detail) "
               "VALUES (%d, 'reject_reimbursement', 'reimbursement', %d, '学院驳回报销')",
               ctx->user->user_id, rid);

    notification_send(applicant, "报销申请被驳回", reason, "reimbursement_result", rid);
    api_ok_msg(ctx, "已驳回");
}

/* GET /api/college/limits —— 本院社团的报销额度配置列表 */
void col_limit_list(ApiContext *ctx) {
    if (!api_require_college_admin(ctx)) return;
    int cid = ctx->user->college_id;

    MYSQL_RES *res = db_query(
        "SELECT c.club_id, c.club_name, "
        "rl.single_limit, rl.period_type, rl.period_limit, rl.current_period_used "
        "FROM clubs c "
        "LEFT JOIN reimbursement_limits rl ON c.club_id=rl.club_id "
        "WHERE c.college_id=%d AND c.level='college' AND c.status='approved' "
        "ORDER BY c.club_id", cid);
    api_send_result_data(ctx, res);
}

/* POST /api/college/limits/{club_id} —— 设定/更新某社团额度
 * body: single_limit, period_type(monthly/semesterly), period_limit */
void col_limit_set(ApiContext *ctx) {
    if (!api_require_college_admin(ctx)) return;
    int cid = ctx->user->college_id;
    int club_id = api_get_path_int(ctx, 2);

    double single_limit = api_get_json_double(ctx, "single_limit", 0);
    double period_limit = api_get_json_double(ctx, "period_limit", 0);
    char period_type[15] = "monthly";
    api_get_json_str(ctx, "period_type", period_type, sizeof(period_type));
    if (!utils_str_equal(period_type, "monthly") &&
        !utils_str_equal(period_type, "semesterly")) {
        utils_strlcpy(period_type, "monthly", sizeof(period_type));
    }

    int owned = db_query_int(
        "SELECT COUNT(*) FROM clubs WHERE club_id=%d AND college_id=%d AND level='college'",
        club_id, cid);
    if (owned == 0) { api_error(ctx, ERR_PERMISSION, "该社团不属于本院"); return; }

    char today[11];
    utils_now_date(today);
    int rc = db_execute(
        "INSERT INTO reimbursement_limits "
        "(club_id, single_limit, period_type, period_limit, current_period_used, period_start, set_by) "
        "VALUES (%d, %.2f, '%s', %.2f, 0.00, '%s', %d) "
        "ON DUPLICATE KEY UPDATE "
        "single_limit=%.2f, period_type='%s', period_limit=%.2f, set_by=%d",
        club_id, single_limit, period_type, period_limit, today, ctx->user->user_id,
        single_limit, period_type, period_limit, ctx->user->user_id);
    if (rc < 0) { api_error(ctx, ERR_DB, "设定失败"); return; }

    db_execute("INSERT INTO logs (user_id, action, target_type, target_id, detail) "
               "VALUES (%d, 'set_reimbursement_limit', 'reimbursement_limits', %d, '设定报销额度')",
               ctx->user->user_id, club_id);
    api_ok_msg(ctx, "额度已保存");
}

#endif
