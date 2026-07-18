/**
 * finance.c — 社团管理端 财务台账 + 报销申请
 * 学生社团管理系统 SCMS（练习文件 — 代码空白，由你自行实现）
 *
 * 需要实现的处理函数：
 *
 *   void club_finance_overview(ApiContext *ctx)
 *     路由：GET /api/club-admin/finance-overview
 *     功能：返回财务概览（total_income, total_expense, balance, pending_reimb）
 *
 *   void club_finance_list(ApiContext *ctx)
 *     路由：GET /api/club-admin/finance
 *     参数：type（income/expense）, page, page_size
 *     功能：返回收支记录列表
 *
 *   void club_finance_add(ApiContext *ctx)
 *     路由：POST /api/club-admin/finance
 *     JSON body：type(income/expense), amount, description, date
 *     功能：新增收支记录
 *
 *   void club_finance_delete(ApiContext *ctx)
 *     路由：DELETE /api/club-admin/finance/{id}
 *     功能：删除收支记录
 *
 *   void club_reimb_list(ApiContext *ctx)
 *     路由：GET /api/club-admin/reimbursements
 *     功能：返回报销申请列表
 *
 *   void club_reimb_create(ApiContext *ctx)
 *     路由：POST /api/club-admin/reimbursements
 *     JSON body：amount, purpose, receipt_path
 *     功能：提交报销申请
 *     操作：INSERT INTO reimbursements(club_id, amount, purpose, receipt_path, applicant_id)
 *     初始状态：status = 'college_pending'
 *
 *   void club_upload_receipt(ApiContext *ctx)
 *     路由：POST /api/club-admin/upload-receipt
 *     功能：上传发票图片到 receipts/ 目录，返回文件路径
 *
 * 权限：需验证当前用户是该社团 president 或 vice_president
 *
 * 参考完整实现：student-club-system/src/student/club_admin/finance.c
 */

#include "../../../include/api.h"
#include "../../../include/db.h"
#include "../../../include/common.h"
#include "../../../include/routes.h"
#include "../../../include/utils.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void club_finance_overview(ApiContext *ctx) {
    /* TODO: 财务概览 */
}

void club_finance_list(ApiContext *ctx) {
    /* TODO: 收支列表 */
}

void club_finance_add(ApiContext *ctx) {
    /* TODO: 新增收支 */
}

void club_finance_delete(ApiContext *ctx) {
    /* TODO: 删除收支 */
}

void club_reimb_list(ApiContext *ctx) {
    /* TODO: 报销申请列表 */
}

void club_reimb_create(ApiContext *ctx) {
    /* TODO: 提交报销申请 */
}

void club_upload_receipt(ApiContext *ctx) {
    /* TODO: 上传发票图片 */
}
