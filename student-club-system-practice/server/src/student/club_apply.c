/**
 * club_apply.c — 申请创建新社团（普通学生发起，学校端审批）
 * 学生社团管理系统 SCMS（学生端）
 *
 * 覆盖处理函数：
 *   stu_club_apply        POST /api/clubs/apply       提交创建社团申请
 *   stu_my_applications   GET  /api/my/applications   我提交的建社申请及状态
 *
 * 启用方法：把下面的 #if 0 改成 #if 1。
 *
 * 业务要点：
 *  - 申请写入 clubs 表，status='pending'，created_by=当前用户。
 *  - 级别(level)申请时可填期望值，最终由学校端审批时确定。
 *  - 审批通过后（在学校端 club_approval.c 处理），创建者自动成为社长。
 */
#if 0

#include "../../include/api.h"
#include "../../include/db.h"
#include "../../include/common.h"
#include "../../include/utils.h"
#include "../../include/routes.h"
#include "../../include/notification.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* POST /api/clubs/apply
 * body: club_name, category, level(期望), description, advisor, college_id(院级时) */
void stu_club_apply(ApiContext *ctx) {
    if (!api_require_login(ctx)) return;
    int uid = ctx->user->user_id;

    char club_name[100] = "", category[32] = "", level[16] = "";
    char description[1000] = "", advisor[64] = "";
    api_get_json_str(ctx, "club_name",   club_name,   sizeof(club_name));
    api_get_json_str(ctx, "category",    category,    sizeof(category));
    api_get_json_str(ctx, "level",       level,       sizeof(level));
    api_get_json_str(ctx, "description", description, sizeof(description));
    api_get_json_str(ctx, "advisor",     advisor,     sizeof(advisor));
    int college_id = api_get_json_int(ctx, "college_id", 0);

    /* 校验 */
    if (utils_is_empty(club_name)) {
        api_error(ctx, ERR_VALIDATION, "请填写社团名称"); return;
    }
    if (utils_is_empty(description)) {
        api_error(ctx, ERR_VALIDATION, "请填写社团简介"); return;
    }
    /* 级别默认院级；院级需带 college_id（默认取本人所属学院） */
    if (utils_is_empty(level)) utils_strlcpy(level, "college", sizeof(level));
    if (utils_str_equal(level, "college") && college_id <= 0) {
        college_id = ctx->user->college_id;
    }

    /* 名称查重（含待审批的） */
    if (db_query_int("SELECT COUNT(*) FROM clubs WHERE club_name='%s' "
                     "AND status IN ('pending','approved')", club_name) > 0) {
        api_error(ctx, ERR_DUPLICATE, "已存在同名社团或待审申请"); return;
    }

    char *e_name = db_escape(club_name);
    char *e_desc = db_escape(description);
    char *e_adv  = db_escape(advisor);

    int rc;
    if (utils_str_equal(level, "school")) {
        /* 校级：college_id 置空 */
        rc = db_execute(
            "INSERT INTO clubs "
            "(club_name, description, category, level, college_id, advisor, "
            " created_by, status, member_count) "
            "VALUES ('%s','%s','%s','school',NULL,'%s',%d,'pending',0)",
            e_name, e_desc, category, e_adv, uid);
    } else {
        rc = db_execute(
            "INSERT INTO clubs "
            "(club_name, description, category, level, college_id, advisor, "
            " created_by, status, member_count) "
            "VALUES ('%s','%s','%s','college',%d,'%s',%d,'pending',0)",
            e_name, e_desc, category, college_id, e_adv, uid);
    }
    free(e_name); free(e_desc); free(e_adv);

    if (rc < 0) { api_error(ctx, ERR_DB, "提交失败"); return; }

    /* 通知全体学校管理员有新的建社申请 */
    notification_notify_school_admins(
        "新的社团创建申请",
        "有学生提交了社团创建申请，请前往社团审批处理。",
        "club_apply", (int)db_last_insert_id());

    api_ok_msg(ctx, "申请已提交，等待学校审批");
}

/* GET /api/my/applications — 我的建社申请记录 */
void stu_my_applications(ApiContext *ctx) {
    if (!api_require_login(ctx)) return;
    int uid = ctx->user->user_id;

    MYSQL_RES *res = db_query(
        "SELECT club_id, club_name, category, level, status, reject_reason, created_at "
        "FROM clubs WHERE created_by=%d ORDER BY created_at DESC", uid);

    api_send_result(ctx, res);
}

#endif /* 备用文件：改 #if 0 为 #if 1 启用 */
