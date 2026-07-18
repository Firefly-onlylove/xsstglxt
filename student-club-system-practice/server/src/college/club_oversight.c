/**
 * club_oversight.c — 学院端 本院社团监督（只读查看）
 * 学生社团管理系统 SCMS（练习文件 — 代码空白，由你自行实现）
 *
 * 需要实现的处理函数：
 *
 *   void col_club_list(ApiContext *ctx)
 *     路由：GET /api/college/clubs
 *     参数：keyword（可选搜索）
 *     功能：返回本院社团列表
 *     返回字段：club_id, name, club_type, president_name, member_count, activity_count
 *
 *   void col_club_detail(ApiContext *ctx)
 *     路由：GET /api/college/clubs/{id}
 *     功能：返回社团详情 + 成员列表
 *
 * 权限：api_require_college_admin(ctx)
 * 核心 SQL 过滤：WHERE college_id = ctx->user->college_id
 * 注意：学院端只能【查看】，无权审批/解散社团
 *
 * 参考完整实现：student-club-system/src/college/club_oversight.c
 */

#include "../../include/api.h"
#include "../../include/db.h"
#include "../../include/common.h"
#include "../../include/routes.h"
#include "../../include/utils.h"

#include <stdio.h>

void col_club_list(ApiContext *ctx) {
    /* TODO: 本院社团列表 */
}

void col_club_detail(ApiContext *ctx) {
    /* TODO: 社团详情 */
}
