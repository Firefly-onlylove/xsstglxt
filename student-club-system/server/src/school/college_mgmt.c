/**
 * college_mgmt.c — 学校端 学院管理（增删改停用）
 * 学生社团管理系统 SCMS（Web 架构 · 学校端）
 *
 * ┌──────────────────────────────────────────────────────────────────┐
 * │  ★ 备用参考实现。默认整份被 #if 0 关闭，不参与编译（走弱符号占位）。 │
 * │  ★ 启用方法：把下面这行的  #if 0  改成  #if 1  ，重新 make 即可。   │
 * └──────────────────────────────────────────────────────────────────┘
 *
 * 覆盖路由：
 *   GET    /api/school/colleges              → sch_college_list    学院列表
 *   POST   /api/school/colleges              → sch_college_create  新增学院
 *   PUT    /api/school/colleges/{id}         → sch_college_update  修改学院
 *   POST   /api/school/colleges/{id}/toggle  → sch_college_toggle  停用/启用
 *   DELETE /api/school/colleges/{id}         → sch_college_delete  删除学院
 */
#if 1

#include "../../include/routes.h"
#include "../../include/api.h"
#include "../../include/db.h"
#include "../../include/common.h"
#include "../../include/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* GET /api/school/colleges  全部学院 */
void sch_college_list(ApiContext *ctx) {
    if (!api_require_school_admin(ctx)) return;
    MYSQL_RES *res = db_query(
        "SELECT c.college_id, c.college_name, c.college_code, c.description, c.status, c.created_at, "
        "(SELECT COUNT(*) FROM majors WHERE college_id=c.college_id) AS major_count "
        "FROM colleges c ORDER BY c.college_id");
    api_send_result_data(ctx, res);
}

/* POST /api/school/colleges  body: name, code, description */
void sch_college_create(ApiContext *ctx) {
    if (!api_require_school_admin(ctx)) return;

    char name[101] = "", code[21] = "", desc[501] = "";
    api_get_json_str(ctx, "name", name, sizeof(name));
    api_get_json_str(ctx, "code", code, sizeof(code));
    api_get_json_str(ctx, "description", desc, sizeof(desc));
    if (!name[0]) api_get_param(ctx, "name", name, sizeof(name));
    if (!code[0]) api_get_param(ctx, "code", code, sizeof(code));

    if (utils_is_empty(name)) {
        api_error(ctx, ERR_INPUT, "学院名称必填"); return;
    }

    /* 查重 */
    char *en = db_escape(name);
    int dup;
    if (code[0]) {
        dup = db_query_int(
            "SELECT COUNT(*) FROM colleges WHERE college_name='%s' OR college_code='%s'",
            name, code);
    } else {
        dup = db_query_int(
            "SELECT COUNT(*) FROM colleges WHERE college_name='%s'",
            name);
    }
    free(en);
    if (dup > 0) { api_error(ctx, ERR_DUPLICATE, "学院名称或编码已存在"); return; }

    char *ec = db_escape(code), *ed = db_escape(desc);
    en = db_escape(name);
    int ok = db_execute(
        "INSERT INTO colleges (college_name, college_code, description) "
        "VALUES ('%s','%s','%s')", en, ec, ed);
    free(en); free(ec); free(ed);
    if (ok < 0) { api_error(ctx, ERR_DB, "创建失败"); return; }

    db_execute("INSERT INTO logs (user_id, action, target_type, target_id, detail) "
               "VALUES (%d, 'create_college', 'colleges', %lu, '新增学院')",
               ctx->user->user_id, db_last_insert_id());
    api_ok_msg(ctx, "学院已创建");
}

/* PUT /api/school/colleges/{id}  body: name, code, description */
void sch_college_update(ApiContext *ctx) {
    if (!api_require_school_admin(ctx)) return;
    int id = api_get_path_int(ctx, 2);
    if (id <= 0) { api_error(ctx, ERR_INPUT, "学院ID非法"); return; }

    char name[101] = "", code[21] = "", desc[501] = "";
    api_get_json_str(ctx, "name", name, sizeof(name));
    api_get_json_str(ctx, "code", code, sizeof(code));
    api_get_json_str(ctx, "description", desc, sizeof(desc));
    if (utils_is_empty(name) || utils_is_empty(code)) {
        api_error(ctx, ERR_INPUT, "学院名称和编码必填"); return;
    }

    char *en = db_escape(name), *ec = db_escape(code), *ed = db_escape(desc);
    int ok = db_execute(
        "UPDATE colleges SET college_name='%s', college_code='%s', description='%s' "
        "WHERE college_id=%d", en, ec, ed, id);
    free(en); free(ec); free(ed);
    if (ok < 0) { api_error(ctx, ERR_DB, "修改失败"); return; }

    db_execute("INSERT INTO logs (user_id, action, target_type, target_id, detail) "
               "VALUES (%d, 'update_college', 'colleges', %d, '修改学院信息')",
               ctx->user->user_id, id);
    api_ok_msg(ctx, "已保存");
}

/* POST /api/school/colleges/{id}/toggle  在 1/0 间切换 status */
void sch_college_toggle(ApiContext *ctx) {
    if (!api_require_school_admin(ctx)) return;
    int id = api_get_path_int(ctx, 2);
    if (id <= 0) { api_error(ctx, ERR_INPUT, "学院ID非法"); return; }

    int ok = db_execute(
        "UPDATE colleges SET status = 1 - status WHERE college_id=%d", id);
    if (ok <= 0) { api_error(ctx, ERR_NOT_FOUND, "学院不存在"); return; }

    int now = db_query_int("SELECT status FROM colleges WHERE college_id=%d", id);
    db_execute("INSERT INTO logs (user_id, action, target_type, target_id, detail) "
               "VALUES (%d, 'toggle_college', 'colleges', %d, '切换学院状态')",
               ctx->user->user_id, id);
    api_ok_msg(ctx, now ? "已启用" : "已停用");
}

/* DELETE /api/school/colleges/{id} */
void sch_college_delete(ApiContext *ctx) {
    if (!api_require_school_admin(ctx)) return;
    int id = api_get_path_int(ctx, 2);
    if (id <= 0) { api_error(ctx, ERR_INPUT, "学院ID非法"); return; }

    /* 检查该学院下是否有专业 */
    int used = db_query_int("SELECT COUNT(*) FROM majors WHERE college_id=%d", id);
    if (used > 0) { api_error(ctx, ERR_STATUS, "该学院下仍有专业，无法删除"); return; }

    /* 检查该学院下是否有社团 */
    int club_used = db_query_int("SELECT COUNT(*) FROM clubs WHERE college_id=%d", id);
    if (club_used > 0) { api_error(ctx, ERR_STATUS, "该学院下仍有社团，无法删除"); return; }

    int ok = db_execute("DELETE FROM colleges WHERE college_id=%d", id);
    if (ok <= 0) { api_error(ctx, ERR_NOT_FOUND, "学院不存在"); return; }

    db_execute("INSERT INTO logs (user_id, action, target_type, target_id, detail) "
               "VALUES (%d, 'delete_college', 'colleges', %d, '删除学院')",
               ctx->user->user_id, id);
    api_ok_msg(ctx, "已删除");
}

#endif /* 备用代码结束 */
