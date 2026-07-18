/**
 * major_mgmt.c — 学校端 专业管理（增删改）
 * 学生社团管理系统 SCMS（Web 架构 · 学校端）
 *
 * ┌──────────────────────────────────────────────────────────────────┐
 * │  ★ 备用参考实现。默认整份被 #if 0 关闭，不参与编译（走弱符号占位）。 │
 * │  ★ 启用方法：把下面这行的  #if 0  改成  #if 1  ，重新 make 即可。   │
 * └──────────────────────────────────────────────────────────────────┘
 *
 * 覆盖路由：
 *   GET    /api/school/majors?college_id=  → sch_major_list    专业列表
 *   POST   /api/school/majors              → sch_major_create  新增专业
 *   PUT    /api/school/majors/{id}         → sch_major_update  修改专业
 *   DELETE /api/school/majors/{id}         → sch_major_delete  删除专业
 */
#if 0

#include "../../include/routes.h"
#include "../../include/api.h"
#include "../../include/db.h"
#include "../../include/common.h"
#include "../../include/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* GET /api/school/majors?college_id=  某学院下专业（不传则全部） */
void sch_major_list(ApiContext *ctx) {
    if (!api_require_school_admin(ctx)) return;
    int college_id = api_get_int(ctx, "college_id", 0);

    MYSQL_RES *res;
    if (college_id > 0) {
        res = db_query(
            "SELECT m.major_id, m.major_name, m.major_code, m.college_id, "
            "c.college_name, m.status "
            "FROM majors m JOIN colleges c ON m.college_id=c.college_id "
            "WHERE m.college_id=%d ORDER BY m.major_id", college_id);
    } else {
        res = db_query(
            "SELECT m.major_id, m.major_name, m.major_code, m.college_id, "
            "c.college_name, m.status "
            "FROM majors m JOIN colleges c ON m.college_id=c.college_id "
            "ORDER BY m.college_id, m.major_id");
    }
    api_send_result_data(ctx, res);
}

/* POST /api/school/majors  body: college_id, name, code */
void sch_major_create(ApiContext *ctx) {
    if (!api_require_school_admin(ctx)) return;

    int college_id = api_get_json_int(ctx, "college_id", 0);
    if (college_id == 0) college_id = api_get_int(ctx, "college_id", 0);
    char name[101] = "", code[21] = "";
    api_get_json_str(ctx, "name", name, sizeof(name));
    api_get_json_str(ctx, "code", code, sizeof(code));
    if (!name[0]) api_get_param(ctx, "name", name, sizeof(name));

    if (college_id <= 0 || utils_is_empty(name)) {
        api_error(ctx, ERR_INPUT, "所属学院和专业名称必填"); return;
    }
    int dup = db_query_int(
        "SELECT COUNT(*) FROM majors WHERE college_id=%d AND major_name='%s'",
        college_id, name);
    if (dup > 0) { api_error(ctx, ERR_DUPLICATE, "该学院下已有同名专业"); return; }

    char *en = db_escape(name), *ec = db_escape(code);
    int ok = db_execute(
        "INSERT INTO majors (college_id, major_name, major_code) VALUES (%d,'%s','%s')",
        college_id, en, ec);
    free(en); free(ec);
    if (ok < 0) { api_error(ctx, ERR_DB, "创建失败"); return; }

    db_execute("INSERT INTO logs (user_id, action, target_type, target_id, detail) "
               "VALUES (%d, 'create_major', 'majors', %lu, '新增专业')",
               ctx->user->user_id, db_last_insert_id());
    api_ok_msg(ctx, "专业已创建");
}

/* PUT /api/school/majors/{id}  body: name, code */
void sch_major_update(ApiContext *ctx) {
    if (!api_require_school_admin(ctx)) return;
    int id = api_get_path_int(ctx, 2);
    if (id <= 0) { api_error(ctx, ERR_INPUT, "专业ID非法"); return; }

    char name[101] = "", code[21] = "";
    api_get_json_str(ctx, "name", name, sizeof(name));
    api_get_json_str(ctx, "code", code, sizeof(code));
    if (utils_is_empty(name)) { api_error(ctx, ERR_INPUT, "专业名称必填"); return; }

    char *en = db_escape(name), *ec = db_escape(code);
    int ok = db_execute(
        "UPDATE majors SET major_name='%s', major_code='%s' WHERE major_id=%d",
        en, ec, id);
    free(en); free(ec);
    if (ok < 0) { api_error(ctx, ERR_DB, "修改失败"); return; }

    db_execute("INSERT INTO logs (user_id, action, target_type, target_id, detail) "
               "VALUES (%d, 'update_major', 'majors', %d, '修改专业')",
               ctx->user->user_id, id);
    api_ok_msg(ctx, "已保存");
}

/* DELETE /api/school/majors/{id} */
void sch_major_delete(ApiContext *ctx) {
    if (!api_require_school_admin(ctx)) return;
    int id = api_get_path_int(ctx, 2);
    if (id <= 0) { api_error(ctx, ERR_INPUT, "专业ID非法"); return; }

    /* 有班级挂靠时禁止删除 */
    int used = db_query_int("SELECT COUNT(*) FROM classes WHERE major_id=%d", id);
    if (used > 0) { api_error(ctx, ERR_STATUS, "该专业下仍有班级，无法删除"); return; }

    int ok = db_execute("DELETE FROM majors WHERE major_id=%d", id);
    if (ok <= 0) { api_error(ctx, ERR_NOT_FOUND, "专业不存在"); return; }

    db_execute("INSERT INTO logs (user_id, action, target_type, target_id, detail) "
               "VALUES (%d, 'delete_major', 'majors', %d, '删除专业')",
               ctx->user->user_id, id);
    api_ok_msg(ctx, "已删除");
}

#endif /* 备用代码结束 */
