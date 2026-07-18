/**
 * college_dashboard.c — 学院端 仪表盘 + 统计
 * 学生社团管理系统 SCMS（练习文件 — 代码空白，由你自行实现）
 *
 * 需要实现的处理函数（须与 routes.c 中路由表一致）：
 *
 *   void col_dashboard(ApiContext *ctx)
 *     路由：GET /api/college/dashboard
 *     功能：返回本院仪表盘统计数据
 *     权限：api_require_college_admin(ctx)
 *     返回字段：
 *       club_count     本院社团总数（status='approved'）
 *       student_count  本院学生总数
 *       activity_count 本院活动总数
 *       pending_reimb  待审批报销数（status='college_pending'）
 *       clubs[]        本院社团简列（club_id, name, president_name, member_count）
 *
 * 核心 SQL 过滤：WHERE college_id = ctx->user->college_id
 *
 * 可用 API 函数（api.h）：
 *   api_require_college_admin(ctx)       — 权限校验
 *   db_query_int("SELECT COUNT(*) ...")  — 查询单个整数
 *   db_query("SELECT ...", ...)          — 查询结果集
 *   api_send_data(ctx, json)             — 发送数据响应
 *   api_error(ctx, code, msg)            — 发送错误响应
 *
 * 参考完整实现：student-club-system/src/college/college_dashboard.c
 */

#include "../../include/api.h"
#include "../../include/db.h"
#include "../../include/common.h"
#include "../../include/routes.h"
#include "../../include/utils.h"

#include <stdio.h>
#include <string.h>

void col_dashboard(ApiContext *ctx) {
    /* TODO: 实现学院仪表盘 */
}
