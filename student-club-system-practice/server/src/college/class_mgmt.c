/**
 * class_mgmt.c — 学院端 班级管理（本院专业下的班级增删改查）
 * 学生社团管理系统 SCMS（练习文件 — 代码空白，由你自行实现）
 *
 * 需要实现的处理函数：
 *
 *   void col_class_list(ApiContext *ctx)
 *     路由：GET /api/college/classes
 *     参数：major_id（可选，按专业过滤）
 *     功能：返回本院班级列表
 *
 *   void col_class_create(ApiContext *ctx)
 *     路由：POST /api/college/classes
 *     JSON body：major_id, class_name
 *     功能：新建班级
 *
 *   void col_class_update(ApiContext *ctx)
 *     路由：PUT /api/college/classes/{id}
 *     JSON body：class_name, major_id
 *     功能：修改班级信息
 *
 *   void col_class_delete(ApiContext *ctx)
 *     路由：DELETE /api/college/classes/{id}
 *     功能：删除班级（需校验该班级下无学生）
 *
 * 权限：api_require_college_admin(ctx)
 * 核心 SQL 过滤：WHERE college_id = ctx->user->college_id
 *
 * 读取 URL 路径参数：int id = api_get_path_int(ctx, 3);
 *   /api/college/classes/{id}  => 第3段（0-based）
 *
 * 参考完整实现：student-club-system/src/college/class_mgmt.c
 */

#include "../../include/api.h"
#include "../../include/db.h"
#include "../../include/common.h"
#include "../../include/routes.h"
#include "../../include/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void col_class_list(ApiContext *ctx) {
    /* TODO: 班级列表 */
}

void col_class_create(ApiContext *ctx) {
    /* TODO: 新建班级 */
}

void col_class_update(ApiContext *ctx) {
    /* TODO: 修改班级 */
}

void col_class_delete(ApiContext *ctx) {
    /* TODO: 删除班级 */
}
