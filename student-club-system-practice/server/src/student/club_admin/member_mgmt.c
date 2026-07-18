/**
 * member_mgmt.c — 社团管理端 成员管理 + 入社审批 + 角色任命
 * 学生社团管理系统 SCMS（练习文件 — 代码空白，由你自行实现）
 *
 * 需要实现的处理函数：
 *
 *   void club_member_list(ApiContext *ctx)
 *     路由：GET /api/club-admin/members
 *     功能：返回本社团在籍成员列表
 *     返回字段：user_id, real_name, student_id, class_name, role, join_date
 *
 *   void club_member_role(ApiContext *ctx)
 *     路由：PUT /api/club-admin/members/{id}/role
 *     JSON body：role（vice_president/member）
 *     功能：修改成员角色（社长可任命副社长）
 *
 *   void club_member_remove(ApiContext *ctx)
 *     路由：DELETE /api/club-admin/members/{id}
 *     功能：将成员移出社团（社长不能被移出）
 *
 *   void club_join_list(ApiContext *ctx)
 *     路由：GET /api/club-admin/join-requests
 *     功能：返回待审批入社申请列表
 *
 *   void club_join_approve(ApiContext *ctx)
 *     路由：POST /api/club-admin/join-requests/{id}/approve
 *     JSON body：approved(bool), reason(拒绝时必填)
 *     功能：通过或拒绝入社申请
 *
 * 权限：需验证当前用户是该社团 president 或 vice_president
 *
 * 参考完整实现：student-club-system/src/student/club_admin/member_mgmt.c
 */

#include "../../../include/api.h"
#include "../../../include/db.h"
#include "../../../include/common.h"
#include "../../../include/routes.h"
#include "../../../include/utils.h"

#include <stdio.h>
#include <string.h>

void club_member_list(ApiContext *ctx) {
    /* TODO: 成员列表 */
}

void club_member_role(ApiContext *ctx) {
    /* TODO: 修改角色 */
}

void club_member_remove(ApiContext *ctx) {
    /* TODO: 移出成员 */
}

void club_join_list(ApiContext *ctx) {
    /* TODO: 入社申请列表 */
}

void club_join_approve(ApiContext *ctx) {
    /* TODO: 审批入社 */
}
