/**
 * class_mgmt.c — 学院端 班级管理（本院专业下的班级增删改查）
 * 学生社团管理系统 SCMS（Web 架构）
 *
 * 对应处理函数：col_class_list / col_class_create / col_class_update / col_class_delete
 * 对应前端页面：C2 班级管理页
 *
 * 启用方法：把下面第一行的  #if 0  改成  #if 1。
 *
 * 权限要点：只能管理本院（ctx->user->college_id）的班级。
 */
#if 1

#include "../../include/api.h"
#include "../../include/db.h"
#include "../../include/common.h"
#include "../../include/routes.h"
#include "../../include/utils.h"

#include <stdio.h>
#include <stdlib.h>

/* GET /api/college/classes?major_id=  —— 班级列表（可按专业过滤） */
void col_class_list(ApiContext *ctx) {
    if (!api_require_college_admin(ctx)) return;
    int cid = ctx->user->college_id;
    int major_id = api_get_int(ctx, "major_id", 0);

    MYSQL_RES *res;
    if (major_id > 0) {
        res = db_query(
            "SELECT cl.class_id, cl.class_name, cl.grade, "
            "m.major_name, cl.major_id, "
            "(SELECT COUNT(*) FROM users WHERE class_id=cl.class_id) AS student_count "
            "FROM classes cl JOIN majors m ON cl.major_id=m.major_id "
            "WHERE cl.college_id=%d AND cl.major_id=%d AND cl.status=1 "
            "ORDER BY cl.grade DESC, cl.class_name", cid, major_id);
    } else {
        res = db_query(
            "SELECT cl.class_id, cl.class_name, cl.grade, "
            "m.major_name, cl.major_id, "
            "(SELECT COUNT(*) FROM users WHERE class_id=cl.class_id) AS student_count "
            "FROM classes cl JOIN majors m ON cl.major_id=m.major_id "
            "WHERE cl.college_id=%d AND cl.status=1 "
            "ORDER BY m.major_name, cl.grade DESC", cid);
    }
    api_send_result_data(ctx, res);
}

/* POST /api/college/classes —— 新建班级（专业+班级名+年级） */
void col_class_create(ApiContext *ctx) {
    if (!api_require_college_admin(ctx)) return;
    int cid = ctx->user->college_id;

    int major_id = api_get_json_int(ctx, "major_id", 0);
    char class_name[101] = "", grade[11] = "";
    api_get_json_str(ctx, "class_name", class_name, sizeof(class_name));
    api_get_json_str(ctx, "grade", grade, sizeof(grade));

    if (major_id <= 0 || utils_is_empty(class_name)) {
        api_error(ctx, ERR_INPUT, "请填写专业和班级名称");
        return;
    }

    /* 校验该专业确实属于本院，防止越权给别院加班级 */
    int owned = db_query_int(
        "SELECT COUNT(*) FROM majors WHERE major_id=%d AND college_id=%d",
        major_id, cid);
    if (owned == 0) { api_error(ctx, ERR_PERMISSION, "该专业不属于本学院"); return; }

    char *e_name = db_escape(class_name);
    char *e_grade = db_escape(grade);
    int rc = db_execute(
        "INSERT INTO classes (major_id, college_id, class_name, grade) "
        "VALUES (%d, %d, '%s', '%s')", major_id, cid, e_name, e_grade);
    free(e_name); free(e_grade);

    if (rc < 0) { api_error(ctx, ERR_DUPLICATE, "班级已存在或创建失败"); return; }
    api_ok_msg(ctx, "班级创建成功");
}

/* PUT /api/college/classes/{id} —— 修改班级 */
void col_class_update(ApiContext *ctx) {
    if (!api_require_college_admin(ctx)) return;
    int cid = ctx->user->college_id;
    int class_id = api_get_path_int(ctx, 2);   /* /api/college/classes/{id} */

    char class_name[101] = "", grade[11] = "";
    api_get_json_str(ctx, "class_name", class_name, sizeof(class_name));
    api_get_json_str(ctx, "grade", grade, sizeof(grade));

    /* 确认该班级属于本院 */
    int owned = db_query_int(
        "SELECT COUNT(*) FROM classes WHERE class_id=%d AND college_id=%d",
        class_id, cid);
    if (owned == 0) { api_error(ctx, ERR_PERMISSION, "无权修改该班级"); return; }

    char *e_name = db_escape(class_name);
    char *e_grade = db_escape(grade);
    db_execute("UPDATE classes SET class_name='%s', grade='%s' WHERE class_id=%d",
               e_name, e_grade, class_id);
    free(e_name); free(e_grade);
    api_ok_msg(ctx, "修改成功");
}

/* DELETE /api/college/classes/{id} —— 删除班级 */
void col_class_delete(ApiContext *ctx) {
    if (!api_require_college_admin(ctx)) return;
    int cid = ctx->user->college_id;
    int class_id = api_get_path_int(ctx, 2);

    int owned = db_query_int(
        "SELECT COUNT(*) FROM classes WHERE class_id=%d AND college_id=%d",
        class_id, cid);
    if (owned == 0) { api_error(ctx, ERR_PERMISSION, "无权删除该班级"); return; }

    /* 若已有学生绑定该班级，禁止删除（外键为 SET NULL，这里主动拦截更友好） */
    int used = db_query_int("SELECT COUNT(*) FROM users WHERE class_id=%d", class_id);
    if (used > 0) { api_error(ctx, ERR_STATUS, "该班级下已有学生，不能删除"); return; }

    db_execute("DELETE FROM classes WHERE class_id=%d", class_id);
    api_ok_msg(ctx, "已删除");
}

#endif
