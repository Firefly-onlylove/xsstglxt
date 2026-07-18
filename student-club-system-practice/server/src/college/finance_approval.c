/**
 * finance_approval.c — 学院端 院级报销审批 + 报销额度设定
 * 学生社团管理系统 SCMS（练习文件 — 代码空白，由你自行实现）
 *
 * 需要实现的处理函数：
 *
 *   void col_reimb_list(ApiContext *ctx)
 *     路由：GET /api/college/reimbursements
 *     参数：status（college_pending/approved/rejected）
 *     功能：返回本院待审批/历史报销列表
 *
 *   void col_reimb_approve(ApiContext *ctx)
 *     路由：POST /api/college/reimbursements/{id}/approve
 *     JSON body：approved(bool), reason(驳回时必填)
 *     功能：通过或驳回报销申请
 *     操作：UPDATE reimbursements SET status=... WHERE reimb_id=? AND college_id=?
 *
 *   void col_reimb_limit(ApiContext *ctx)
 *     路由：GET /api/college/reimb-limit
 *     功能：返回本院报销额度（剩余可审批金额）
 *
 * 权限：api_require_college_admin(ctx)
 * 核心 SQL 过滤：通过 JOIN clubs WHERE clubs.college_id = ctx->user->college_id
 *
 * 参考完整实现：student-club-system/src/college/finance_approval.c
 */

#include "../../include/api.h"
#include "../../include/db.h"
#include "../../include/common.h"
#include "../../include/routes.h"
#include "../../include/utils.h"

#include <stdio.h>
#include <string.h>

void col_reimb_list(ApiContext *ctx) {
    /* TODO: 报销列表 */
}

void col_reimb_approve(ApiContext *ctx) {
    /* TODO: 审批报销 */
}

void col_reimb_limit(ApiContext *ctx) {
    /* TODO: 返回报销额度 */
}
