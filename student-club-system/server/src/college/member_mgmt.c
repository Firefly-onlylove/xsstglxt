/**
 * member_mgmt.c — 学院端 本院学生管理（限制 / 解除限制）
 * 学生社团管理系统 SCMS（Web 架构）
 *
 * 对应处理函数：
 *   col_student_list      GET  本院学生列表（含当前限制状态）
 *   col_student_restrict  POST 对本院学生施加限制
 *   col_student_lift      POST 解除本院学生的限制
 * 对应前端页面：C4 本院学生管理页
 *
 * 启用方法：把下面第一行的  #if 0  改成  #if 1。
 *
 * 权限要点：学院只能限制【本院】学生（按 users.college_id 匹配），
 * 限制类型两级：ban_join（禁止加入社团）/ ban_activity（禁止参加活动）。
 */
#if 1

#include "../../include/api.h"
#include "../../include/db.h"
#include "../../include/common.h"
#include "../../include/routes.h"
#include "../../include/utils.h"
#include "../../include/notification.h"

#include <stdio.h>

/* GET /api/college/students?major_id=&class_id=&keyword=&search=&status=&page=&page_size= */
void col_student_list(ApiContext *ctx) {
    if (!api_require_college_admin(ctx)) return;
    int cid = ctx->user->college_id;

    int major_id = api_get_int(ctx, "major_id", 0);
    int class_id = api_get_int(ctx, "class_id", 0);

    /* 优先读 keyword，兼容旧参数名 search */
    char keyword[101] = "";
    api_get_param(ctx, "keyword", keyword, sizeof(keyword));
    if (!keyword[0]) {
        api_get_param(ctx, "search", keyword, sizeof(keyword));
    }

    /* 分页 */
    int page = api_get_int(ctx, "page", 1);
    if (page < 1) page = 1;
    int page_size = api_get_int(ctx, "page_size", PAGE_SIZE_DEFAULT);
    if (page_size < 1) page_size = PAGE_SIZE_DEFAULT;
    int offset = (page - 1) * page_size;

    /* 状态筛选 */
    char status[20] = "";
    api_get_param(ctx, "status", status, sizeof(status));

    /* 动态拼可选过滤条件。用户输入 keyword 需转义。 */
    char where[512] = "WHERE 1=1";
    int off = 0;
    off = snprintf(where, sizeof(where),
                   "WHERE (u.college_id=%d OR u.college_id IS NULL) AND u.role IN ('general_student','club_member','club_admin')",
                   cid);
    if (major_id > 0)
        off += snprintf(where + off, sizeof(where) - off, " AND u.major_id=%d", major_id);
    if (class_id > 0)
        off += snprintf(where + off, sizeof(where) - off, " AND u.class_id=%d", class_id);
    if (keyword[0]) {
        char *e = db_escape(keyword);
        snprintf(where + off, sizeof(where) - off,
                 " AND (u.real_name LIKE '%%%s%%' OR u.student_no LIKE '%%%s%%')", e, e);
        free(e);
    }
    if (status[0]) {
        if (utils_str_equal(status, "active")) {
            int len = strlen(where);
            snprintf(where + len, sizeof(where) - len,
                     " AND NOT EXISTS (SELECT 1 FROM user_restrictions ur WHERE ur.user_id=u.user_id AND ur.is_active=1)");
        } else if (utils_str_equal(status, "restricted")) {
            int len = strlen(where);
            snprintf(where + len, sizeof(where) - len,
                     " AND EXISTS (SELECT 1 FROM user_restrictions ur WHERE ur.user_id=u.user_id AND ur.is_active=1)");
        }
    }

    /* 查总数 */
    int total = db_query_int("SELECT COUNT(*) FROM users u %s", where);

    /* 关联 user_restrictions 取当前生效限制（子查询）。 */
    MYSQL_RES *res = db_query(
        "SELECT u.user_id, u.username, u.real_name, u.student_no, u.phone, u.status, u.last_login, "
        "m.major_name, cl.class_name, "
        "(SELECT GROUP_CONCAT(restriction_type) FROM user_restrictions ur "
        " WHERE ur.user_id=u.user_id AND ur.is_active=1) AS restrictions "
        "FROM users u "
        "LEFT JOIN majors m ON u.major_id=m.major_id "
        "LEFT JOIN classes cl ON u.class_id=cl.class_id "
        "%s ORDER BY u.student_no LIMIT %d OFFSET %d",
        where, page_size, offset);
    api_send_result_paged(ctx, res, page, page_size, total);
}

/* POST /api/college/students/{id}/restrict
 * body: restriction_type(ban_join/ban_activity), end_time(可空=永久), reason */
void col_student_restrict(ApiContext *ctx) {
    if (!api_require_college_admin(ctx)) return;
    int cid = ctx->user->college_id;
    int target = api_get_path_int(ctx, 2);   /* /api/college/students/{id}/restrict */

    char rtype[20] = "", end_time[20] = "", reason[501] = "";
    api_get_json_str(ctx, "restriction_type", rtype, sizeof(rtype));
    api_get_json_str(ctx, "end_time", end_time, sizeof(end_time));
    api_get_json_str(ctx, "reason", reason, sizeof(reason));

    if (!utils_str_equal(rtype, "ban_join") && !utils_str_equal(rtype, "ban_activity") &&
        !utils_str_equal(rtype, "no_join") && !utils_str_equal(rtype, "no_activity") &&
        !utils_str_equal(rtype, "no_apply")) {
        api_error(ctx, ERR_INPUT, "限制类型不合法");
        return;
    }

    /* 确认目标学生属于本院 */
    int owned = db_query_int(
        "SELECT COUNT(*) FROM users WHERE user_id=%d AND college_id=%d", target, cid);
    if (owned == 0) { api_error(ctx, ERR_PERMISSION, "该学生不属于本院"); return; }

    char now[20];
    utils_now_datetime(now);
    char *e_reason = db_escape(reason);

    if (utils_is_empty(end_time)) {
        /* 永久限制，end_time 存 NULL */
        db_execute("INSERT INTO user_restrictions "
                   "(user_id, restriction_type, restricted_by, start_time, end_time, reason, is_active) "
                   "VALUES (%d, '%s', %d, '%s', NULL, '%s', 1)",
                   target, rtype, ctx->user->user_id, now, e_reason);
    } else {
        char *e_end = db_escape(end_time);
        db_execute("INSERT INTO user_restrictions "
                   "(user_id, restriction_type, restricted_by, start_time, end_time, reason, is_active) "
                   "VALUES (%d, '%s', %d, '%s', '%s', '%s', 1)",
                   target, rtype, ctx->user->user_id, now, e_end, e_reason);
        free(e_end);
    }
    free(e_reason);

    db_execute("INSERT INTO logs (user_id, action, target_type, target_id, detail) "
               "VALUES (%d, 'restrict_user', 'user_restrictions', %d, '限制本院学生')",
               ctx->user->user_id, target);

    const char *type_cn = utils_str_equal(rtype, "ban_join") ? "禁止加入社团" : "禁止参加活动";
    char msg[600];
    snprintf(msg, sizeof(msg), "您已被学院限制：%s。原因：%s", type_cn,
             reason[0] ? reason : "（未填写）");
    notification_send(target, "账号被限制", msg, "restriction_applied", target);
    api_ok_msg(ctx, "限制已生效");
}

/* POST /api/college/students/{id}/lift-restriction
 * body: restriction_type（要解除的类型） */
void col_student_lift(ApiContext *ctx) {
    if (!api_require_college_admin(ctx)) return;
    int cid = ctx->user->college_id;
    int target = api_get_path_int(ctx, 2);

    char rtype[20] = "";
    api_get_json_str(ctx, "restriction_type", rtype, sizeof(rtype));

    int owned = db_query_int(
        "SELECT COUNT(*) FROM users WHERE user_id=%d AND college_id=%d", target, cid);
    if (owned == 0) { api_error(ctx, ERR_PERMISSION, "该学生不属于本院"); return; }

    if (rtype[0]) {
        db_execute("UPDATE user_restrictions SET is_active=0 "
                   "WHERE user_id=%d AND restriction_type='%s' AND is_active=1",
                   target, rtype);
    } else {
        /* 未指定类型则解除该学生的全部限制 */
        db_execute("UPDATE user_restrictions SET is_active=0 "
                   "WHERE user_id=%d AND is_active=1", target);
    }
    db_execute("INSERT INTO logs (user_id, action, target_type, target_id, detail) "
               "VALUES (%d, 'lift_restriction', 'user_restrictions', %d, '解除本院学生限制')",
               ctx->user->user_id, target);

    notification_send(target, "限制已解除", "您的账号限制已被学院解除。",
                      "restriction_lifted", target);
    api_ok_msg(ctx, "已解除");
}

#endif
