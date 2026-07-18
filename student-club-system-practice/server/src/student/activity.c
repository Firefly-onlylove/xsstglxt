/**
 * activity.c — 活动浏览 / 报名 / 取消报名 / 签到
 * 学生社团管理系统 SCMS（学生端）
 *
 * 覆盖处理函数（函数名/路由严格对齐 routes.h）：
 *   stu_activity_list      GET  /api/activities              全部可报名活动（分页）
 *   stu_activity_detail    GET  /api/activities/{id}         活动详情
 *   stu_activity_register  POST /api/activities/{id}/register 报名
 *   stu_activity_cancel    POST /api/activities/{id}/cancel   取消报名
 *   stu_activity_signin    POST /api/activities/{id}/signin   活动码签到
 *
 * 启用方法：把下面的 #if 0 改成 #if 1。
 *
 * 业务要点：
 *  - 报名前检查 ban_activity 限制、名额是否已满、是否重复报名。
 *  - 签到采用"活动码"方式：社长发布活动码，学生输入匹配则签到成功。
 *  - "我报名的活动"并入 /api/my/clubs 相关页面，此处不单列。
 */
#if 0

#include "../../include/api.h"
#include "../../include/db.h"
#include "../../include/common.h"
#include "../../include/utils.h"
#include "../../include/routes.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* GET /api/activities?page= — 已发布/进行中的活动 */
void stu_activity_list(ApiContext *ctx) {
    int page = api_get_int(ctx, "page", 1);
    if (page < 1) page = 1;
    int page_size = PAGE_SIZE_DEFAULT;
    int offset = (page - 1) * page_size;

    int total = db_query_int(
        "SELECT COUNT(*) FROM activities "
        "WHERE status IN ('published','ongoing')");

    MYSQL_RES *res = db_query(
        "SELECT a.activity_id, a.title, a.location, a.start_time, a.end_time, "
        "a.max_participants, a.current_count, a.status, "
        "c.club_name, c.club_id "
        "FROM activities a JOIN clubs c ON a.club_id=c.club_id "
        "WHERE a.status IN ('published','ongoing') "
        "ORDER BY a.start_time LIMIT %d OFFSET %d",
        page_size, offset);

    api_send_result_paged(ctx, res, page, page_size, total);
}

/* GET /api/activities/{id} — 活动详情 */
void stu_activity_detail(ApiContext *ctx) {
    int aid = api_get_path_int(ctx, 1);
    if (aid <= 0) { api_error(ctx, ERR_INPUT, "活动ID非法"); return; }

    MYSQL_RES *res = db_query(
        "SELECT a.activity_id, a.title, a.description, a.location, "
        "a.start_time, a.end_time, a.enroll_deadline, a.max_participants, "
        "a.current_count, a.status, c.club_name, c.club_id "
        "FROM activities a JOIN clubs c ON a.club_id=c.club_id "
        "WHERE a.activity_id=%d", aid);

    if (!res || mysql_num_rows(res) == 0) {
        if (res) mysql_free_result(res);
        api_error(ctx, ERR_NOT_FOUND, "活动不存在");
        return;
    }
    api_send_result_data(ctx, res);
}

/* POST /api/activities/{id}/register — 报名 */
void stu_activity_register(ApiContext *ctx) {
    if (!api_require_login(ctx)) return;
    int uid = ctx->user->user_id;
    int aid = api_get_path_int(ctx, 1);
    if (aid <= 0) { api_error(ctx, ERR_INPUT, "活动ID非法"); return; }

    /* 1. 是否被限制参加活动 */
    int banned = db_query_int(
        "SELECT COUNT(*) FROM user_restrictions "
        "WHERE user_id=%d AND restriction_type='ban_activity' AND is_active=1 "
        "AND (end_time IS NULL OR end_time > NOW())", uid);
    if (banned > 0) { api_error(ctx, ERR_PERMISSION, "您已被限制参加活动"); return; }

    /* 2. 取活动信息：名额、状态、报名截止 */
    MYSQL_RES *res = db_query(
        "SELECT max_participants, current_count, status, enroll_deadline "
        "FROM activities WHERE activity_id=%d", aid);
    if (!res || mysql_num_rows(res) == 0) {
        if (res) mysql_free_result(res);
        api_error(ctx, ERR_NOT_FOUND, "活动不存在");
        return;
    }
    MYSQL_ROW row = mysql_fetch_row(res);
    int max_p = atoi(row[0]);
    int cur   = atoi(row[1]);
    char status[16]; utils_strlcpy(status, row[2] ? row[2] : "", sizeof(status));
    mysql_free_result(res);

    if (!utils_str_equal(status, "published")) {
        api_error(ctx, ERR_STATUS, "该活动当前不可报名"); return;
    }
    if (max_p > 0 && cur >= max_p) {
        api_error(ctx, ERR_STATUS, "报名名额已满"); return;
    }

    /* 3. 重复报名检查 */
    int dup = db_query_int(
        "SELECT COUNT(*) FROM activity_enrollments "
        "WHERE activity_id=%d AND user_id=%d AND status!='cancelled'", aid, uid);
    if (dup > 0) { api_error(ctx, ERR_DUPLICATE, "您已报名该活动"); return; }

    /* 4. 写报名记录 + 名额 +1 */
    db_execute("INSERT INTO activity_enrollments (activity_id, user_id, status, enrolled_at) "
               "VALUES (%d, %d, 'enrolled', NOW())", aid, uid);
    db_execute("UPDATE activities SET current_count=current_count+1 WHERE activity_id=%d", aid);

    api_ok_msg(ctx, "报名成功");
}

/* POST /api/activities/{id}/cancel — 取消报名 */
void stu_activity_cancel(ApiContext *ctx) {
    if (!api_require_login(ctx)) return;
    int uid = ctx->user->user_id;
    int aid = api_get_path_int(ctx, 1);
    if (aid <= 0) { api_error(ctx, ERR_INPUT, "活动ID非法"); return; }

    /* 只能取消未签到的报名 */
    int rc = db_execute(
        "UPDATE activity_enrollments SET status='cancelled' "
        "WHERE activity_id=%d AND user_id=%d AND status='enrolled'", aid, uid);
    if (rc <= 0) { api_error(ctx, ERR_STATUS, "无可取消的报名"); return; }

    db_execute("UPDATE activities SET current_count=GREATEST(current_count-1,0) "
               "WHERE activity_id=%d", aid);
    api_ok_msg(ctx, "已取消报名");
}

/* POST /api/activities/{id}/signin — 活动码签到  body: code */
void stu_activity_signin(ApiContext *ctx) {
    if (!api_require_login(ctx)) return;
    int uid = ctx->user->user_id;
    int aid = api_get_path_int(ctx, 1);
    if (aid <= 0) { api_error(ctx, ERR_INPUT, "活动ID非法"); return; }

    char code[32] = "";
    api_get_json_str(ctx, "code", code, sizeof(code));
    if (utils_is_empty(code)) { api_error(ctx, ERR_INPUT, "请输入活动码"); return; }

    /* 校验活动码 */
    MYSQL_RES *res = db_query(
        "SELECT checkin_code FROM activities WHERE activity_id=%d", aid);
    if (!res || mysql_num_rows(res) == 0) {
        if (res) mysql_free_result(res);
        api_error(ctx, ERR_NOT_FOUND, "活动不存在");
        return;
    }
    MYSQL_ROW row = mysql_fetch_row(res);
    char real_code[32]; utils_strlcpy(real_code, row[0] ? row[0] : "", sizeof(real_code));
    mysql_free_result(res);

    if (utils_is_empty(real_code) || !utils_str_equal(code, real_code)) {
        api_error(ctx, ERR_VALIDATION, "活动码不正确");
        return;
    }

    /* 必须已报名 */
    int rc = db_execute(
        "UPDATE activity_enrollments SET status='checked_in', checkin_time=NOW() "
        "WHERE activity_id=%d AND user_id=%d AND status='enrolled'", aid, uid);
    if (rc <= 0) { api_error(ctx, ERR_STATUS, "您尚未报名或已签到"); return; }

    api_ok_msg(ctx, "签到成功");
}

#endif /* 备用文件：改 #if 0 为 #if 1 启用 */
