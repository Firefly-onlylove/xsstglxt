/**
 * activity_mgmt.c — 社团管理端 活动全生命周期管理 + 签到管理
 * 学生社团管理系统 SCMS（练习文件 — 代码空白，由你自行实现）
 *
 * 需要实现的处理函数：
 *
 *   void club_activity_list(ApiContext *ctx)
 *     路由：GET /api/club-admin/activities
 *     参数：status（可选过滤）, page, page_size
 *     功能：返回本社团所有活动列表
 *
 *   void club_activity_create(ApiContext *ctx)
 *     路由：POST /api/club-admin/activities
 *     JSON body：title, max_participants, start_time, end_time,
 *                signup_deadline, location, description
 *     功能：创建新活动（初始状态 draft）
 *
 *   void club_activity_update(ApiContext *ctx)
 *     路由：PUT /api/club-admin/activities/{id}
 *     功能：编辑活动（仅草稿和已发布状态可编辑）
 *
 *   void club_activity_status(ApiContext *ctx)
 *     路由：PUT /api/club-admin/activities/{id}/status
 *     JSON body：status（published/ongoing/finished/cancelled）, reason（取消时必填）
 *     功能：变更活动状态
 *     状态流转：draft → published → ongoing → finished
 *                                           ↘ cancelled
 *
 *   void club_activity_delete(ApiContext *ctx)
 *     路由：DELETE /api/club-admin/activities/{id}
 *     功能：删除草稿活动
 *
 *   void club_activity_registrations(ApiContext *ctx)
 *     路由：GET /api/club-admin/activities/{id}/registrations
 *     功能：获取报名/签到列表
 *
 *   void club_activity_manual_signin(ApiContext *ctx)
 *     路由：POST /api/club-admin/activities/{id}/manual-signin
 *     JSON body：keyword（姓名或学号）
 *     功能：手动签到
 *
 * 签到码：activities 表的 signin_code 字段（创建活动时自动生成6位随机码）
 * 权限：需验证当前用户是该社团 president 或 vice_president
 *
 * 参考完整实现：student-club-system/src/student/club_admin/activity_mgmt.c
 */

#include "../../../include/api.h"
#include "../../../include/db.h"
#include "../../../include/common.h"
#include "../../../include/routes.h"
#include "../../../include/utils.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void club_activity_list(ApiContext *ctx) {
    /* TODO: 活动列表 */
}

void club_activity_create(ApiContext *ctx) {
    /* TODO: 创建活动 */
}

void club_activity_update(ApiContext *ctx) {
    /* TODO: 编辑活动 */
}

void club_activity_status(ApiContext *ctx) {
    /* TODO: 变更活动状态 */
}

void club_activity_delete(ApiContext *ctx) {
    /* TODO: 删除草稿 */
}

void club_activity_registrations(ApiContext *ctx) {
    /* TODO: 报名/签到列表 */
}

void club_activity_manual_signin(ApiContext *ctx) {
    /* TODO: 手动签到 */
}
