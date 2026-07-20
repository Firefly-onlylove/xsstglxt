/**
 * activity_mgmt.c — 社团管理端：活动全生命周期管理 + 签到管理
 * 学生社团管理系统 SCMS（社团管理端 club_admin）
 *
 * 覆盖处理函数（函数名/路由严格对齐 routes.h）：
 *   club_activity_list      GET    /api/club/{id}/activities
 *   club_activity_create    POST   /api/club/{id}/activities
 *   club_activity_update    PUT    /api/club/{id}/activities/{aid}
 *   club_activity_publish   POST   /api/club/{id}/activities/{aid}/publish
 *   club_activity_start     POST   /api/club/{id}/activities/{aid}/start
 *   club_activity_finish    POST   /api/club/{id}/activities/{aid}/finish
 *   club_activity_cancel    POST   /api/club/{id}/activities/{aid}/cancel
 *   club_activity_summary   POST   /api/club/{id}/activities/{aid}/summary
 *   club_activity_delete    DELETE /api/club/{id}/activities/{aid}
 *   club_signin_list        GET    /api/club/{id}/activities/{aid}/signins
 *   club_signin_manual      POST   /api/club/{id}/activities/{aid}/manual-signin
 *
 * 启用方法：把下面的 #if 0 改成 #if 1。
 *
 * 活动状态机：draft → published → ongoing → completed；任意非完成态可 cancelled。
 * 签到码：发布/开始活动时由社长设定 checkin_code；学生端 signin 时匹配。
 */
#if 1

#include "../../../include/api.h"
#include "../../../include/db.h"
#include "../../../include/common.h"
#include "../../../include/utils.h"
#include "../../../include/routes.h"
#include "../../../include/notification.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 校验活动属于该社团，返回 1/0 */
static int activity_belongs(int aid, int club_id) {
    return db_query_int("SELECT COUNT(*) FROM activities "
                        "WHERE activity_id=%d AND club_id=%d", aid, club_id) > 0;
}

/* GET /api/club/{id}/activities — 本社全部活动（含草稿） */
void club_activity_list(ApiContext *ctx) {
    int club_id = api_get_path_int(ctx, 1);
    if (club_id <= 0) { api_error(ctx, ERR_INPUT, "社团ID非法"); return; }
    if (!api_require_club_admin(ctx, club_id)) return;

    MYSQL_RES *res = db_query(
        "SELECT activity_id, title, location, start_time, end_time, "
        "max_participants, current_count, status, created_at, "
        "checkin_code, join_permission, enable_signin, description "
        "FROM activities WHERE club_id=%d ORDER BY created_at DESC", club_id);
    api_send_result(ctx, res);
}

/* POST /api/club/{id}/activities — 新建活动（草稿）
 * body: title, description, location, start_time, end_time, signup_deadline, max_participants,
 *       join_permission(all/members_only), enable_signin(0/1) */
void club_activity_create(ApiContext *ctx) {
    int club_id = api_get_path_int(ctx, 1);
    if (club_id <= 0) { api_error(ctx, ERR_INPUT, "社团ID非法"); return; }
    if (!api_require_club_admin(ctx, club_id)) return;

    char title[128] = "", description[2000] = "", location[128] = "";
    char start_time[32] = "", end_time[32] = "", signup_deadline[32] = "";
    char join_perm[16] = "all";
    api_get_json_str(ctx, "title",           title,           sizeof(title));
    api_get_json_str(ctx, "description",     description,     sizeof(description));
    api_get_json_str(ctx, "location",        location,        sizeof(location));
    api_get_json_str(ctx, "start_time",      start_time,      sizeof(start_time));
    api_get_json_str(ctx, "end_time",        end_time,        sizeof(end_time));
    api_get_json_str(ctx, "signup_deadline", signup_deadline, sizeof(signup_deadline));
    api_get_json_str(ctx, "join_permission", join_perm,       sizeof(join_perm));
    int max_p = api_get_json_int(ctx, "max_participants", 0);
    if (max_p < 1) max_p = 50;
    int enable_signin = api_get_json_int(ctx, "enable_signin", 0);

    if (utils_is_empty(title))      { api_error(ctx, ERR_VALIDATION, "请填写活动标题"); return; }
    if (utils_is_empty(start_time)) { api_error(ctx, ERR_VALIDATION, "请填写开始时间"); return; }
    if (!utils_str_equal(join_perm, "members_only"))
        utils_strlcpy(join_perm, "all", sizeof(join_perm));

    char *e_title = db_escape(title);
    char *e_desc  = db_escape(description);
    char *e_loc   = db_escape(location);

    int rc = db_execute(
        "INSERT INTO activities "
        "(club_id, title, description, location, start_time, end_time, "
        " signup_deadline, max_participants, join_permission, enable_signin, "
        " current_count, status, created_at) "
        "VALUES (%d,'%s','%s','%s','%s','%s','%s',%d,'%s',%d, 0,'draft',NOW())",
        club_id, e_title, e_desc, e_loc, start_time,
        utils_is_empty(end_time) ? "1970-01-01 00:00:00" : end_time,
        utils_is_empty(signup_deadline) ? "1970-01-01 00:00:00" : signup_deadline,
        max_p, join_perm, enable_signin);
    free(e_title); free(e_desc); free(e_loc);

    if (rc < 0) { api_error(ctx, ERR_DB, "创建失败"); return; }
    api_ok_msg(ctx, "活动已创建（草稿）");
}

/* PUT /api/club/{id}/activities/{aid} — 修改活动（仅草稿态可改） */
void club_activity_update(ApiContext *ctx) {
    int club_id = api_get_path_int(ctx, 1);
    int aid     = api_get_path_int(ctx, 3);
    if (club_id <= 0 || aid <= 0) { api_error(ctx, ERR_INPUT, "参数非法"); return; }
    if (!api_require_club_admin(ctx, club_id)) return;
    if (!activity_belongs(aid, club_id)) { api_error(ctx, ERR_NOT_FOUND, "活动不存在"); return; }

    char status[16];
    db_query_str("SELECT status FROM activities WHERE activity_id=%d",
                 status, sizeof(status), aid);
    if (!utils_str_equal(status, "draft")) {
        api_error(ctx, ERR_STATUS, "仅草稿状态可修改"); return;
    }

    char title[128] = "", description[2000] = "", location[128] = "";
    char start_time[32] = "", end_time[32] = "", signup_deadline[32] = "";
    char join_perm[16] = "";
    api_get_json_str(ctx, "title",           title,           sizeof(title));
    api_get_json_str(ctx, "description",     description,     sizeof(description));
    api_get_json_str(ctx, "location",        location,        sizeof(location));
    api_get_json_str(ctx, "start_time",      start_time,      sizeof(start_time));
    api_get_json_str(ctx, "end_time",        end_time,        sizeof(end_time));
    api_get_json_str(ctx, "signup_deadline", signup_deadline, sizeof(signup_deadline));
    api_get_json_str(ctx, "join_permission", join_perm,       sizeof(join_perm));
    int max_p = api_get_json_int(ctx, "max_participants", -1);
    int enable_signin = api_get_json_int(ctx, "enable_signin", -1);

    char *e_title = db_escape(title);
    char *e_desc  = db_escape(description);
    char *e_loc   = db_escape(location);

    char set[1024] = "";
    int sl = 0;
    if (!utils_is_empty(title))       sl += snprintf(set + sl, sizeof(set) - sl, "title='%s', ", e_title);
    if (!utils_is_empty(description)) sl += snprintf(set + sl, sizeof(set) - sl, "description='%s', ", e_desc);
    if (!utils_is_empty(location))    sl += snprintf(set + sl, sizeof(set) - sl, "location='%s', ", e_loc);
    if (!utils_is_empty(start_time))  sl += snprintf(set + sl, sizeof(set) - sl, "start_time='%s', ", start_time);
    if (!utils_is_empty(end_time))    sl += snprintf(set + sl, sizeof(set) - sl, "end_time='%s', ", end_time);
    if (!utils_is_empty(signup_deadline)) sl += snprintf(set + sl, sizeof(set) - sl, "signup_deadline='%s', ", signup_deadline);
    if (max_p >= 0)                   sl += snprintf(set + sl, sizeof(set) - sl, "max_participants=%d, ", max_p);
    if (!utils_is_empty(join_perm))   sl += snprintf(set + sl, sizeof(set) - sl, "join_permission='%s', ", join_perm);
    if (enable_signin >= 0)           sl += snprintf(set + sl, sizeof(set) - sl, "enable_signin=%d, ", enable_signin);

    free(e_title); free(e_desc); free(e_loc);

    if (sl == 0) { api_error(ctx, ERR_INPUT, "没有需要更新的字段"); return; }
    set[sl - 2] = '\0'; /* trim trailing ", " */

    db_execute("UPDATE activities SET %s WHERE activity_id=%d", set, aid);
    api_ok_msg(ctx, "活动已更新");
}

/* POST /api/club/{id}/activities/{aid}/publish — 发布（draft→published，生成签到码） */
void club_activity_publish(ApiContext *ctx) {
    int club_id = api_get_path_int(ctx, 1);
    int aid     = api_get_path_int(ctx, 3);
    if (club_id <= 0 || aid <= 0) { api_error(ctx, ERR_INPUT, "参数非法"); return; }
    if (!api_require_club_admin(ctx, club_id)) return;
    if (!activity_belongs(aid, club_id)) { api_error(ctx, ERR_NOT_FOUND, "活动不存在"); return; }

    /* 检查是否启用签到 */
    int enable_signin = db_query_int(
        "SELECT enable_signin FROM activities WHERE activity_id=%d", aid);
    char code[8] = "";
    if (enable_signin) {
        utils_gen_signin_code(code);
        db_execute("UPDATE activities SET status='published', checkin_code='%s' "
                   "WHERE activity_id=%d AND status='draft'", code, aid);
    } else {
        db_execute("UPDATE activities SET status='published' "
                   "WHERE activity_id=%d AND status='draft'", aid);
    }

    JsonBuilder jb;
    json_init(&jb);
    if (enable_signin) json_add_str(&jb, "checkin_code", code);
    json_add_str(&jb, "message", enable_signin ? "活动已发布，签到已开启" : "活动已发布");
    api_ok_data(ctx, json_finish(&jb));
    json_free(&jb);
}

/* POST /api/club/{id}/activities/{aid}/start — 开始（published→ongoing） */
void club_activity_start(ApiContext *ctx) {
    int club_id = api_get_path_int(ctx, 1);
    int aid     = api_get_path_int(ctx, 3);
    if (club_id <= 0 || aid <= 0) { api_error(ctx, ERR_INPUT, "参数非法"); return; }
    if (!api_require_club_admin(ctx, club_id)) return;
    if (!activity_belongs(aid, club_id)) { api_error(ctx, ERR_NOT_FOUND, "活动不存在"); return; }

    db_execute("UPDATE activities SET status='ongoing' "
               "WHERE activity_id=%d AND status='published'", aid);
    api_ok_msg(ctx, "活动已开始");
}

/* POST /api/club/{id}/activities/{aid}/finish — 结束（ongoing→completed） */
void club_activity_finish(ApiContext *ctx) {
    int club_id = api_get_path_int(ctx, 1);
    int aid     = api_get_path_int(ctx, 3);
    if (club_id <= 0 || aid <= 0) { api_error(ctx, ERR_INPUT, "参数非法"); return; }
    if (!api_require_club_admin(ctx, club_id)) return;
    if (!activity_belongs(aid, club_id)) { api_error(ctx, ERR_NOT_FOUND, "活动不存在"); return; }

    db_execute("UPDATE activities SET status='completed' "
               "WHERE activity_id=%d AND status='ongoing'", aid);
    api_ok_msg(ctx, "活动已结束");
}

/* POST /api/club/{id}/activities/{aid}/cancel — 取消（通知已报名者） body: reason */
void club_activity_cancel(ApiContext *ctx) {
    int club_id = api_get_path_int(ctx, 1);
    int aid     = api_get_path_int(ctx, 3);
    if (club_id <= 0 || aid <= 0) { api_error(ctx, ERR_INPUT, "参数非法"); return; }
    if (!api_require_club_admin(ctx, club_id)) return;
    if (!activity_belongs(aid, club_id)) { api_error(ctx, ERR_NOT_FOUND, "活动不存在"); return; }

    char reason[256] = "";
    api_get_json_str(ctx, "reason", reason, sizeof(reason));

    db_execute("UPDATE activities SET status='cancelled' WHERE activity_id=%d "
               "AND status IN ('published','ongoing')", aid);

    /* 通知所有已报名且未取消的学生 */
    MYSQL_RES *res = db_query(
        "SELECT user_id FROM registrations "
        "WHERE activity_id=%d AND status!='cancelled'", aid);
    if (res) {
        char title[128];
        db_query_str("SELECT title FROM activities WHERE activity_id=%d",
                     title, sizeof(title), aid);
        char msg[400];
        snprintf(msg, sizeof(msg), "活动「%s」已取消。%s%s", title,
                 utils_is_empty(reason) ? "" : "原因：", reason);
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(res)) != NULL) {
            if (row[0]) notification_send(atoi(row[0]), "活动取消通知",
                                          msg, "activity_cancel", aid);
        }
        mysql_free_result(res);
    }
    api_ok_msg(ctx, "活动已取消，已通知报名者");
}

/* POST /api/club/{id}/activities/{aid}/summary — 提交活动总结  body: summary */
void club_activity_summary(ApiContext *ctx) {
    int club_id = api_get_path_int(ctx, 1);
    int aid     = api_get_path_int(ctx, 3);
    if (club_id <= 0 || aid <= 0) { api_error(ctx, ERR_INPUT, "参数非法"); return; }
    if (!api_require_club_admin(ctx, club_id)) return;
    if (!activity_belongs(aid, club_id)) { api_error(ctx, ERR_NOT_FOUND, "活动不存在"); return; }

    char summary[4000] = "";
    api_get_json_str(ctx, "summary", summary, sizeof(summary));
    char *e = db_escape(summary);
    db_execute("UPDATE activities SET summary='%s' WHERE activity_id=%d", e, aid);
    free(e);
    api_ok_msg(ctx, "总结已保存");
}

/* DELETE /api/club/{id}/activities/{aid} — 删除（仅草稿/已取消可删） */
void club_activity_delete(ApiContext *ctx) {
    int club_id = api_get_path_int(ctx, 1);
    int aid     = api_get_path_int(ctx, 3);
    if (club_id <= 0 || aid <= 0) { api_error(ctx, ERR_INPUT, "参数非法"); return; }
    if (!api_require_club_admin(ctx, club_id)) return;
    if (!activity_belongs(aid, club_id)) { api_error(ctx, ERR_NOT_FOUND, "活动不存在"); return; }

    char status[16];
    db_query_str("SELECT status FROM activities WHERE activity_id=%d",
                 status, sizeof(status), aid);
    if (!utils_str_equal(status, "draft") && !utils_str_equal(status, "cancelled")) {
        api_error(ctx, ERR_STATUS, "仅草稿或已取消的活动可删除"); return;
    }
    db_execute("DELETE FROM registrations WHERE activity_id=%d", aid);
    db_execute("DELETE FROM activities WHERE activity_id=%d", aid);
    api_ok_msg(ctx, "活动已删除");
}

/* GET /api/club/{id}/activities/{aid}/signins — 签到名单 */
void club_signin_list(ApiContext *ctx) {
    int club_id = api_get_path_int(ctx, 1);
    int aid     = api_get_path_int(ctx, 3);
    if (club_id <= 0 || aid <= 0) { api_error(ctx, ERR_INPUT, "参数非法"); return; }
    if (!api_require_club_admin(ctx, club_id)) return;
    if (!activity_belongs(aid, club_id)) { api_error(ctx, ERR_NOT_FOUND, "活动不存在"); return; }

    MYSQL_RES *res = db_query(
        "SELECT e.registration_id, u.real_name, u.student_no, e.status, e.checkin_time "
        "FROM registrations e JOIN users u ON e.user_id=u.user_id "
        "WHERE e.activity_id=%d AND e.status!='cancelled' "
        "ORDER BY e.status DESC, e.checkin_time", aid);
    api_send_result(ctx, res);
}

/* POST /api/club/{id}/activities/{aid}/manual-signin — 手动补签  body: user_id */
void club_signin_manual(ApiContext *ctx) {
    int club_id = api_get_path_int(ctx, 1);
    int aid     = api_get_path_int(ctx, 3);
    if (club_id <= 0 || aid <= 0) { api_error(ctx, ERR_INPUT, "参数非法"); return; }
    if (!api_require_club_admin(ctx, club_id)) return;
    if (!activity_belongs(aid, club_id)) { api_error(ctx, ERR_NOT_FOUND, "活动不存在"); return; }

    int target = api_get_json_int(ctx, "user_id", 0);
    if (target <= 0) { api_error(ctx, ERR_INPUT, "请指定成员"); return; }

    int rc = db_execute(
        "UPDATE registrations SET status='attended', checkin_time=NOW() "
        "WHERE activity_id=%d AND user_id=%d AND status='registered'", aid, target);
    if (rc <= 0) { api_error(ctx, ERR_STATUS, "该成员未报名或已签到"); return; }
    api_ok_msg(ctx, "补签成功");
}

#endif /* 备用文件：改 #if 0 为 #if 1 启用 */
