/**
 * member_mgmt.c — 学院端 本院学生管理（限制 / 解除限制）
 * 学生社团管理系统 SCMS（练习文件 — 代码空白，由你自行实现）
 *
 * 需要实现的处理函数：
 *
 *   void col_student_list(ApiContext *ctx)
 *     路由：GET /api/college/students
 *     参数：keyword, class_id, status
 *     功能：返回本院学生列表（带限制状态）
 *
 *   void col_student_restrict(ApiContext *ctx)
 *     路由：POST /api/college/students/{id}/restrict
 *     JSON body：restriction_type, reason
 *     restriction_type 可选：no_join / no_activity / no_apply
 *     功能：限制学生
 *     操作：INSERT INTO user_restrictions ...
 *
 *   void col_student_unrestrict(ApiContext *ctx)
 *     路由：POST /api/college/students/{id}/unrestrict
 *     功能：解除学生的限制
 *     操作：DELETE FROM user_restrictions WHERE user_id=?
 *
 *   void col_classes_all(ApiContext *ctx)
 *     路由：GET /api/college/classes-all
 *     功能：返回本院所有班级（供前端筛选下拉框使用）
 *
 * 权限：api_require_college_admin(ctx)
 * 核心 SQL 过滤：WHERE users.college_id = ctx->user->college_id
 *
 * 参考完整实现：student-club-system/src/college/member_mgmt.c
 */

#include "../../include/api.h"
#include "../../include/db.h"
#include "../../include/common.h"
#include "../../include/routes.h"
#include "../../include/utils.h"

#include <stdio.h>
#include <string.h>

void col_student_list(ApiContext *ctx) {
    /* TODO: 学生列表 */
}

void col_student_restrict(ApiContext *ctx) {
    /* TODO: 限制学生 */
}

void col_student_unrestrict(ApiContext *ctx) {
    /* TODO: 解除限制 */
}

void col_classes_all(ApiContext *ctx) {
    /* TODO: 本院全部班级（用于下拉） */
}
