/**
 * finance.c — 社团管理端：财务台账 + 报销申请（含发票上传）
 * 学生社团管理系统 SCMS（社团管理端 club_admin）
 *
 * 覆盖处理函数（函数名/路由严格对齐 routes.h）：
 *   club_finance_list   GET  /api/club/{id}/finance          财务台账（收入+支出）
 *   club_reimb_list     GET  /api/club/{id}/reimbursements   报销记录及审核状态
 *   club_reimb_create   POST /api/club/{id}/reimbursements   提交报销（含发票图片）
 *
 * 启用方法：把下面的 #if 0 改成 #if 1。
 *
 * 报销流程：社长上传发票图片 + 金额 + 事由 → 记录 status='pending'
 *          → 学院管理员审核（在学院端 reimbursement.c）→ 通过即完成打款。
 * 额度校验：单次上限 + 周期(月)累计上限，取自 clubs 或学院配置。
 */
#if 1

#include "../../../include/api.h"
#include "../../../include/db.h"
#include "../../../include/common.h"
#include "../../../include/utils.h"
#include "../../../include/routes.h"
#include "../../../include/notification.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int club_require_manager(ApiContext *ctx, int club_id) {
    if (!api_require_login(ctx)) return 0;
    if (utils_str_equal(ctx->user->role, "school_admin")) return 1;
    int is_mgr = db_query_int(
        "SELECT COUNT(*) FROM members WHERE club_id=%d AND user_id=%d "
        "AND join_status='approved' AND left_at IS NULL "
        "AND role IN ('president','vice_president')", club_id, ctx->user->user_id);
    if (is_mgr == 0) { api_error(ctx, ERR_PERMISSION, "仅社长/副社长可操作"); return 0; }
    return 1;
}

/* GET /api/club/{id}/finance — 财务台账 + 余额汇总 */
void club_finance_list(ApiContext *ctx) {
    int club_id = api_get_path_int(ctx, 1);
    if (club_id <= 0) { api_error(ctx, ERR_INPUT, "社团ID非法"); return; }
    if (!club_require_manager(ctx, club_id)) return;

    /* 读取筛选参数 */
    char filter_type[32] = "";
    api_get_param(ctx, "type", filter_type, sizeof(filter_type));

    /* 构建 WHERE 条件 */
    char where[128] = "";
    if (!utils_is_empty(filter_type)) {
        char *e_type = db_escape(filter_type);
        snprintf(where, sizeof(where), "AND type='%s' ", e_type);
        free(e_type);
    }

    double income = db_query_double(
        "SELECT COALESCE(SUM(amount),0) FROM finance "
        "WHERE club_id=%d AND type='income'", club_id);
    double expense = db_query_double(
        "SELECT COALESCE(SUM(amount),0) FROM finance "
        "WHERE club_id=%d AND type='expense'", club_id);

    char sql[512];
    snprintf(sql, sizeof(sql),
        "SELECT f.finance_id, f.type, f.amount, f.description, f.source, "
        "DATE_FORMAT(f.record_time,'%%Y-%%m-%%d') AS date, "
        "COALESCE(u.real_name, '') AS operator_name "
        "FROM finance f LEFT JOIN users u ON f.operator_id=u.user_id "
        "WHERE f.club_id=%d %s ORDER BY f.record_time DESC",
        club_id, where);

    MYSQL_RES *res = db_query("%s", sql);

    JsonBuilder jb;
    json_init(&jb);
    json_add_double(&jb, "income", income);
    json_add_double(&jb, "expense", expense);
    json_add_double(&jb, "balance", income - expense);
    json_add_raw(&jb, "records", db_result_to_json_array(res));
    if (res) mysql_free_result(res);
    api_ok_data(ctx, json_finish(&jb));
    json_free(&jb);
}

/* POST /api/club/{id}/finance — 新增收支记录 */
void club_finance_create(ApiContext *ctx) {
    int club_id = api_get_path_int(ctx, 1);
    if (club_id <= 0) { api_error(ctx, ERR_INPUT, "社团ID非法"); return; }
    if (!club_require_manager(ctx, club_id)) return;

    char type[32] = "", description[256] = "";
    double amount = api_get_json_double(ctx, "amount", 0);
    api_get_json_str(ctx, "type", type, sizeof(type));
    api_get_json_str(ctx, "description", description, sizeof(description));

    if (amount <= 0) { api_error(ctx, ERR_VALIDATION, "金额必须大于0"); return; }
    if (utils_is_empty(type)) { api_error(ctx, ERR_VALIDATION, "请选择收支类型"); return; }
    if (strcmp(type, "income") != 0 && strcmp(type, "expense") != 0) {
        api_error(ctx, ERR_VALIDATION, "类型须为income或expense"); return;
    }

    char *e_desc = db_escape(description);
    int rc = db_execute(
        "INSERT INTO finance (club_id, type, amount, description, operator_id, record_time) "
        "VALUES (%d, '%s', %.2f, '%s', %d, NOW())",
        club_id, type, amount, e_desc,
        ctx->user ? ctx->user->user_id : 0);
    free(e_desc);

    if (rc < 0) { api_error(ctx, ERR_DB, "添加失败"); return; }
    api_ok_msg(ctx, "已添加");
}

/* DELETE /api/club/{id}/finance/{fid} — 删除收支记录 */
void club_finance_delete(ApiContext *ctx) {
    int club_id = api_get_path_int(ctx, 1);
    int fid = api_get_path_int(ctx, 3);
    if (club_id <= 0 || fid <= 0) { api_error(ctx, ERR_INPUT, "参数非法"); return; }
    if (!club_require_manager(ctx, club_id)) return;

    int rc = db_execute(
        "DELETE FROM finance WHERE finance_id=%d AND club_id=%d", fid, club_id);
    if (rc < 0) { api_error(ctx, ERR_DB, "删除失败"); return; }
    api_ok_msg(ctx, "已删除");
}

/* POST /api/club/{id}/upload-receipt — 上传发票图片 */
void club_reimb_upload(ApiContext *ctx) {
    int club_id = api_get_path_int(ctx, 1);
    if (club_id <= 0) { api_error(ctx, ERR_INPUT, "社团ID非法"); return; }
    if (!api_require_login(ctx)) return;

    char receipt_path[256] = "";
    int up = api_save_uploaded_file(ctx, "file", "receipts", receipt_path, sizeof(receipt_path));
    if (up < 0) { api_error(ctx, ERR_INPUT, "发票图片上传失败"); return; }

    JsonBuilder jb;
    json_init(&jb);
    json_add_str(&jb, "path", receipt_path);
    api_ok_data(ctx, json_finish(&jb));
    json_free(&jb);
}

/* GET /api/club/{id}/reimbursements — 报销记录 */
void club_reimb_list(ApiContext *ctx) {
    int club_id = api_get_path_int(ctx, 1);
    if (club_id <= 0) { api_error(ctx, ERR_INPUT, "社团ID非法"); return; }
    if (!club_require_manager(ctx, club_id)) return;

    MYSQL_RES *res = db_query(
        "SELECT reimbursement_id, amount, description, receipt_path, status, "
        "review_comment, submitted_at, reviewed_at "
        "FROM reimbursement WHERE club_id=%d ORDER BY submitted_at DESC", club_id);
    api_send_result(ctx, res);
}

/* POST /api/club/{id}/reimbursements — 提交报销
 * application/json: amount, description, receipt_path
 * 发票图片已通过 /upload-receipt 预先上传，receipt_path 即返回的路径。 */
void club_reimb_create(ApiContext *ctx) {
    int club_id = api_get_path_int(ctx, 1);
    if (club_id <= 0) { api_error(ctx, ERR_INPUT, "社团ID非法"); return; }
    if (!club_require_manager(ctx, club_id)) return;

    double amount = api_get_json_double(ctx, "amount", 0);
    char description[512] = "";
    api_get_json_str(ctx, "description", description, sizeof(description));
    char receipt_path[256] = "";
    api_get_json_str(ctx, "receipt_path", receipt_path, sizeof(receipt_path));

    if (amount <= 0) { api_error(ctx, ERR_VALIDATION, "报销金额必须大于0"); return; }
    if (utils_is_empty(description)) { api_error(ctx, ERR_VALIDATION, "请填写报销事由"); return; }
    if (utils_is_empty(receipt_path)) { api_error(ctx, ERR_VALIDATION, "请上传发票"); return; }

    /* 单次上限校验（取自社团所属学院的配置，默认 REIMB_SINGLE_LIMIT） */
    double single_limit = db_query_double(
        "SELECT COALESCE(col.reimb_single_limit, %d) FROM clubs c "
        "LEFT JOIN colleges col ON c.college_id=col.college_id WHERE c.club_id=%d",
        REIMB_SINGLE_LIMIT_DEFAULT, club_id);
    if (single_limit > 0 && amount > single_limit) {
        api_error(ctx, ERR_STATUS, "超过单次报销上限"); return;
    }

    /* 周期(本月)累计上限校验 */
    double period_limit = db_query_double(
        "SELECT COALESCE(col.reimb_period_limit, %d) FROM clubs c "
        "LEFT JOIN colleges col ON c.college_id=col.college_id WHERE c.club_id=%d",
        REIMB_PERIOD_LIMIT_DEFAULT, club_id);
    double month_used = db_query_double(
        "SELECT COALESCE(SUM(amount),0) FROM reimbursement "
        "WHERE club_id=%d AND status IN ('pending','approved') "
        "AND YEAR(submitted_at)=YEAR(NOW()) AND MONTH(submitted_at)=MONTH(NOW())", club_id);
    if (period_limit > 0 && month_used + amount > period_limit) {
        api_error(ctx, ERR_STATUS, "超过本月报销累计上限"); return;
    }

    char *e_desc = db_escape(description);
    char *e_path = db_escape(receipt_path);
    int rc = db_execute(
        "INSERT INTO reimbursement "
        "(club_id, applicant_id, amount, description, receipt_path, status, submitted_at) "
        "VALUES (%d, %d, %.2f, '%s', '%s', 'pending', NOW())",
        club_id, ctx->user->user_id, amount, e_desc, e_path);
    free(e_desc); free(e_path);

    if (rc < 0) { api_error(ctx, ERR_DB, "提交失败"); return; }

    /* 通知本社团所属学院的管理员审核 */
    int college_id = db_query_int("SELECT college_id FROM colleges c JOIN clubs cl ON c.college_id=cl.college_id WHERE cl.club_id=%d", club_id);
    if (college_id > 0)
        notification_notify_college_admins(college_id, "新的报销申请",
            "有社团提交了报销申请，请前往报销审批处理。", "reimb_apply", club_id);

    api_ok_msg(ctx, "报销申请已提交，等待学院审核");
}

#endif /* 备用文件：改 #if 0 为 #if 1 启用 */
