/**
 * user_mgmt.c — 学校端 用户管理
 * 学生社团管理系统 SCMS（Web 架构 · 学校端）
 *
 * ┌──────────────────────────────────────────────────────────────────┐
 * │  ★ 备用参考实现。默认整份被 #if 0 关闭，不参与编译（走弱符号占位）。 │
 * │  ★ 启用方法：把下面这行的  #if 0  改成  #if 1  ，重新 make 即可。   │
 * └──────────────────────────────────────────────────────────────────┘
 *
 * 覆盖路由：
 *   GET  /api/school/users                        → sch_user_list
 *   GET  /api/school/users/{id}                   → sch_user_detail
 *   POST /api/school/users/college-admin          → sch_user_create_college_admin
 *   POST /api/school/users/{id}/toggle            → sch_user_toggle（禁用/启用）
 *   POST /api/school/users/{id}/reset-password    → sch_user_reset_pwd
 *   POST /api/school/users/{id}/restrict          → sch_user_restrict
 *   POST /api/school/users/{id}/lift-restriction  → sch_user_lift_restrict
 *   GET  /api/school/restrictions                 → sch_restriction_list
 */
#if 1

#include "../../include/routes.h"
#include "../../include/api.h"
#include "../../include/db.h"
#include "../../include/common.h"
#include "../../include/utils.h"
#include "../../include/notification.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* GET /api/school/users?role=&keyword=&page=  用户列表（分页） */
void sch_user_list(ApiContext *ctx) {
    if (!api_require_school_admin(ctx)) return;

    char role[32] = "", kw[64] = "", status[32] = "";
    api_get_param(ctx, "role", role, sizeof(role));
    api_get_param(ctx, "keyword", kw, sizeof(kw));
    api_get_param(ctx, "status", status, sizeof(status));
    int page = api_get_int(ctx, "page", 1);
    if (page < 1) page = 1;
    int page_size = PAGE_SIZE_DEFAULT;
    int offset = (page - 1) * page_size;

    /* 组合 WHERE 条件 */
    char where[512] = "WHERE 1=1";
    if (role[0]) {
        char *er = db_escape(role);
        char frag[128];
        if (strcmp(role, "student") == 0 || strcmp(role, "general_student") == 0) {
            /* "student"/"general_student" 匹配普通学生（不含社团成员） */
            snprintf(frag, sizeof(frag), " AND u.role='general_student'");
        } else {
            snprintf(frag, sizeof(frag), " AND u.role='%s'", er);
        }
        strncat(where, frag, sizeof(where) - strlen(where) - 1);
        free(er);
    }
    if (kw[0]) {
        char *ek = db_escape(kw);
        char frag[256];
        snprintf(frag, sizeof(frag),
                 " AND (u.real_name LIKE '%%%s%%' OR u.student_no LIKE '%%%s%%')", ek, ek);
        strncat(where, frag, sizeof(where) - strlen(where) - 1);
        free(ek);
    }
    if (status[0]) {
        if (utils_str_equal(status, "1")) {
            strncat(where, " AND u.status=1", sizeof(where) - strlen(where) - 1);
        } else if (utils_str_equal(status, "0")) {
            strncat(where, " AND u.status=0", sizeof(where) - strlen(where) - 1);
        } else if (utils_str_equal(status, "restricted") || utils_str_equal(status, "2")) {
            strncat(where, " AND u.status=2", sizeof(where) - strlen(where) - 1);
        }
    }

    int total = db_query_int("SELECT COUNT(*) FROM users u %s", where);

    MYSQL_RES *res = db_query(
        "SELECT u.user_id, u.username, u.real_name, u.role, u.student_no, "
        "COALESCE(c.college_name,'—') AS college_name, u.phone, u.status, u.last_login "
        "FROM users u LEFT JOIN colleges c ON u.college_id=c.college_id "
        "%s ORDER BY u.user_id LIMIT %d OFFSET %d",
        where, page_size, offset);

    api_send_result_paged(ctx, res, page, page_size, total);
}

/* GET /api/school/users/{id}  用户详情 */
void sch_user_detail(ApiContext *ctx) {
    if (!api_require_school_admin(ctx)) return;
    int id = api_get_path_int(ctx, 2);
    if (id <= 0) { api_error(ctx, ERR_INPUT, "用户ID非法"); return; }

    MYSQL_RES *res = db_query(
        "SELECT u.user_id, u.username, u.real_name, u.role, u.student_no, u.id_card, "
        "u.phone, u.email, u.status, u.last_login, u.created_at, "
        "COALESCE(c.college_name,'—') AS college_name, "
        "COALESCE(m.major_name,'—') AS major_name, "
        "COALESCE(cl.class_name,'—') AS class_name "
        "FROM users u "
        "LEFT JOIN colleges c ON u.college_id=c.college_id "
        "LEFT JOIN majors m ON u.major_id=m.major_id "
        "LEFT JOIN classes cl ON u.class_id=cl.class_id "
        "WHERE u.user_id=%d", id);
    if (!res || mysql_num_rows(res) == 0) {
        if (res) mysql_free_result(res);
        api_error(ctx, ERR_NOT_FOUND, "用户不存在"); return;
    }
    api_send_result_data(ctx, res);
}

/* POST /api/school/users/college-admin  创建学院管理员
 * body: username, real_name, password, college_id */
void sch_user_create_college_admin(ApiContext *ctx) {
    if (!api_require_school_admin(ctx)) return;

    char username[51] = "", real_name[51] = "", password[128] = "";
    api_get_json_str(ctx, "username", username, sizeof(username));
    api_get_json_str(ctx, "real_name", real_name, sizeof(real_name));
    api_get_json_str(ctx, "password", password, sizeof(password));
    int college_id = api_get_json_int(ctx, "college_id", 0);

    if (utils_is_empty(username) || utils_is_empty(real_name) ||
        utils_is_empty(password) || college_id <= 0) {
        api_error(ctx, ERR_INPUT, "用户名、姓名、密码、所属学院均必填"); return;
    }
    if (!utils_validate_username(username)) {
        api_error(ctx, ERR_VALIDATION, "用户名格式非法（4-50位字母数字下划线）"); return;
    }
    int dup = db_query_int("SELECT COUNT(*) FROM users WHERE username='%s'", username);
    if (dup > 0) { api_error(ctx, ERR_DUPLICATE, "用户名已存在"); return; }

    char hash[SHA256_HEX_LENGTH];
    utils_sha256(password, hash);

    char *eu = db_escape(username), *en = db_escape(real_name);
    int ok = db_execute(
        "INSERT INTO users (username, password, real_name, role, college_id) "
        "VALUES ('%s','%s','%s','college_admin',%d)", eu, hash, en, college_id);
    free(eu); free(en);
    if (ok < 0) { api_error(ctx, ERR_DB, "创建失败"); return; }

    db_execute("INSERT INTO logs (user_id, action, target_type, target_id, detail) "
               "VALUES (%d, 'create_college_admin', 'users', %lu, '创建学院管理员')",
               ctx->user->user_id, db_last_insert_id());
    api_ok_msg(ctx, "学院管理员已创建");
}

/* POST /api/school/users/{id}/toggle  切换 status：1 ↔ 0 */
void sch_user_toggle(ApiContext *ctx) {
    if (!api_require_school_admin(ctx)) return;
    int id = api_get_path_int(ctx, 2);
    if (id <= 0) { api_error(ctx, ERR_INPUT, "用户ID非法"); return; }
    if (id == ctx->user->user_id) { api_error(ctx, ERR_STATUS, "不能禁用自己"); return; }

    int cur_status = db_query_int("SELECT status FROM users WHERE user_id=%d", id);
    int new_status;
    if (cur_status == 0) {
        /* 启用时检查用户是否仍有活跃限制，有则设为限制态(2) */
        int has_restrict = db_query_int("SELECT COUNT(*) FROM user_restrictions WHERE user_id=%d AND is_active=1", id);
        new_status = (has_restrict > 0) ? 2 : 1;
    } else {
        /* 正常(1)或限制态(2) -> 禁用 */
        new_status = 0;
    }
    int ok = db_execute("UPDATE users SET status=%d WHERE user_id=%d", new_status, id);
    if (ok <= 0) { api_error(ctx, ERR_DB, "操作失败"); return; }

    db_execute("INSERT INTO logs (user_id, action, target_type, target_id, detail) "
               "VALUES (%d, 'toggle_user', 'users', %d, '切换用户状态')",
               ctx->user->user_id, id);
    if (new_status == 1)
        api_ok_msg(ctx, "已启用");
    else if (new_status == 2)
        api_ok_msg(ctx, "已启用（有限制）");
    else
        api_ok_msg(ctx, "已禁用");
}

/* POST /api/school/users/{id}/reset-password  body: new_password（缺省重置为 123456） */
void sch_user_reset_pwd(ApiContext *ctx) {
    if (!api_require_school_admin(ctx)) return;
    int id = api_get_path_int(ctx, 2);
    if (id <= 0) { api_error(ctx, ERR_INPUT, "用户ID非法"); return; }

    char newpwd[128] = "";
    api_get_json_str(ctx, "new_password", newpwd, sizeof(newpwd));
    if (utils_is_empty(newpwd)) utils_strlcpy(newpwd, "123456", sizeof(newpwd));

    char hash[SHA256_HEX_LENGTH];
    utils_sha256(newpwd, hash);
    int ok = db_execute("UPDATE users SET password='%s' WHERE user_id=%d", hash, id);
    if (ok <= 0) { api_error(ctx, ERR_NOT_FOUND, "用户不存在"); return; }

    db_execute("INSERT INTO logs (user_id, action, target_type, target_id, detail) "
               "VALUES (%d, 'reset_password', 'users', %d, '重置用户密码')",
               ctx->user->user_id, id);
    api_ok_msg(ctx, "密码已重置");
}

/* POST /api/school/users/{id}/restrict
 * body: type(ban_join|ban_activity), start_time, end_time(可空=永久), reason */
void sch_user_restrict(ApiContext *ctx) {
    if (!api_require_school_admin(ctx)) return;
    int id = api_get_path_int(ctx, 2);
    if (id <= 0) { api_error(ctx, ERR_INPUT, "用户ID非法"); return; }

    char type[20] = "", start[20] = "", end[20] = "", reason[501] = "";
    api_get_json_str(ctx, "restriction_type", type, sizeof(type));
    api_get_json_str(ctx, "start_time", start, sizeof(start));
    api_get_json_str(ctx, "end_time", end, sizeof(end));
    api_get_json_str(ctx, "reason", reason, sizeof(reason));

    if (!utils_str_equal(type, "ban_join") && !utils_str_equal(type, "ban_activity")) {
        api_error(ctx, ERR_INPUT, "限制类型非法"); return;
    }
    if (utils_is_empty(start)) utils_now_datetime(start);

    char *er = db_escape(reason);
    int ok;
    if (utils_is_empty(end)) {
        ok = db_execute(
            "INSERT INTO user_restrictions (user_id, restriction_type, restricted_by, "
            "start_time, end_time, reason) VALUES (%d,'%s',%d,'%s',NULL,'%s')",
            id, type, ctx->user->user_id, start, er);
    } else {
        ok = db_execute(
            "INSERT INTO user_restrictions (user_id, restriction_type, restricted_by, "
            "start_time, end_time, reason) VALUES (%d,'%s',%d,'%s','%s','%s')",
            id, type, ctx->user->user_id, start, end, er);
    }
    free(er);
    /* 同步更新用户状态为限制态 */
    db_execute("UPDATE users SET status=2 WHERE user_id=%d", id);
    if (ok < 0) { api_error(ctx, ERR_DB, "施加限制失败"); return; }

    db_execute("INSERT INTO logs (user_id, action, target_type, target_id, detail) "
               "VALUES (%d, 'restrict_user', 'user_restrictions', %d, '限制用户')",
               ctx->user->user_id, id);

    const char *tn = utils_str_equal(type, "ban_join") ? "禁止加入社团" : "禁止参加活动";
    char msg[600];
    snprintf(msg, sizeof(msg), "您的账号被施加限制：%s。原因：%s",
             tn, reason[0] ? reason : "（未填写）");
    notification_send(id, "账号被限制", msg, "restriction_applied", 0);
    api_ok_msg(ctx, "已施加限制");
}

/* POST /api/school/users/{id}/lift-restriction  body: restriction_id */
void sch_user_lift_restrict(ApiContext *ctx) {
    if (!api_require_school_admin(ctx)) return;
    int uid = api_get_path_int(ctx, 2);
    int rid = api_get_json_int(ctx, "restriction_id", 0);
    if (uid <= 0 || rid <= 0) { api_error(ctx, ERR_INPUT, "参数非法"); return; }

    int ok = db_execute(
        "UPDATE user_restrictions SET is_active=0 "
        "WHERE restriction_id=%d AND user_id=%d AND is_active=1", rid, uid);
    if (ok <= 0) { api_error(ctx, ERR_NOT_FOUND, "限制不存在或已解除"); return; }
    /* 检查是否已无活跃限制，无则恢复为正常状态 */
    int remain = db_query_int("SELECT COUNT(*) FROM user_restrictions WHERE user_id=%d AND is_active=1", uid);
    if (remain == 0)
        db_execute("UPDATE users SET status=1 WHERE user_id=%d AND status=2", uid);

    db_execute("INSERT INTO logs (user_id, action, target_type, target_id, detail) "
               "VALUES (%d, 'lift_restriction', 'user_restrictions', %d, '解除限制')",
               ctx->user->user_id, rid);
    notification_send(uid, "限制已解除", "您的账号限制已被解除。", "restriction_lifted", 0);
    api_ok_msg(ctx, "已解除限制");
}

/* GET /api/school/restrictions?user_id=  当前生效中的限制列表 */
void sch_restriction_list(ApiContext *ctx) {
    if (!api_require_school_admin(ctx)) return;
    int uid = api_get_int(ctx, "user_id", 0);

    MYSQL_RES *res;
    if (uid > 0) {
        res = db_query(
            "SELECT r.restriction_id, r.user_id, u.real_name, r.restriction_type, "
            "r.start_time, r.end_time, r.reason, b.real_name AS by_name "
            "FROM user_restrictions r "
            "JOIN users u ON r.user_id=u.user_id "
            "LEFT JOIN users b ON r.restricted_by=b.user_id "
            "WHERE r.is_active=1 AND r.user_id=%d ORDER BY r.created_at DESC", uid);
    } else {
        res = db_query(
            "SELECT r.restriction_id, r.user_id, u.real_name, r.restriction_type, "
            "r.start_time, r.end_time, r.reason, b.real_name AS by_name "
            "FROM user_restrictions r "
            "JOIN users u ON r.user_id=u.user_id "
            "LEFT JOIN users b ON r.restricted_by=b.user_id "
            "WHERE r.is_active=1 ORDER BY r.created_at DESC LIMIT 100");
    }
    api_send_result_data(ctx, res);
}

/* POST /api/school/users/{id}/set-role  修改用户角色
 * body: role(college_admin|club_admin), college_id(role=college_admin时), club_id(role=club_admin时) */
void sch_user_set_role(ApiContext *ctx) {
    if (!api_require_school_admin(ctx)) return;
    int id = api_get_path_int(ctx, 2);
    if (id <= 0) { api_error(ctx, ERR_INPUT, "用户ID非法"); return; }

    char role[32] = "";
    api_get_json_str(ctx, "role", role, sizeof(role));
    int college_id = api_get_json_int(ctx, "college_id", 0);
    int club_id = api_get_json_int(ctx, "club_id", 0);

    if (!utils_str_equal(role, "college_admin") && !utils_str_equal(role, "club_admin") &&
        !utils_str_equal(role, "club_member") && !utils_str_equal(role, "general_student")) {
        api_error(ctx, ERR_INPUT, "角色类型非法（支持 college_admin / club_admin / club_member / general_student）"); return;
    }

    if (utils_str_equal(role, "college_admin") && college_id <= 0) {
        api_error(ctx, ERR_INPUT, "请指定所属学院"); return;
    }
    if ((utils_str_equal(role, "club_admin") || utils_str_equal(role, "club_member")) && club_id <= 0) {
        api_error(ctx, ERR_INPUT, "请指定所属社团"); return;
    }

    /* 检查目标用户是否存在 */
    int exist = db_query_int("SELECT COUNT(*) FROM users WHERE user_id=%d", id);
    if (exist == 0) { api_error(ctx, ERR_NOT_FOUND, "用户不存在"); return; }

    db_begin();
    if (utils_str_equal(role, "college_admin")) {
        db_execute("UPDATE users SET role='college_admin', college_id=%d WHERE user_id=%d",
                   college_id, id);
    } else if (utils_str_equal(role, "club_admin")) {
        int club_college_id = db_query_int("SELECT college_id FROM clubs WHERE club_id=%d", club_id);
        if (club_college_id <= 0) {
            db_rollback();
            api_error(ctx, ERR_NOT_FOUND, "社团不存在"); return;
        }
        db_execute("UPDATE users SET role='club_admin', college_id=%d WHERE user_id=%d",
                   club_college_id, id);
        db_execute("INSERT IGNORE INTO club_admins (user_id, club_id) VALUES (%d, %d)", id, club_id);
    } else if (utils_str_equal(role, "club_member")) {
        int club_college_id = db_query_int("SELECT college_id FROM clubs WHERE club_id=%d", club_id);
        if (club_college_id <= 0) {
            db_rollback();
            api_error(ctx, ERR_NOT_FOUND, "社团不存在"); return;
        }
        db_execute("UPDATE users SET role='club_member', college_id=%d WHERE user_id=%d",
                   club_college_id, id);
        /* 如果不是该社团成员则插入成员记录 */
        db_execute("INSERT IGNORE INTO members (club_id, user_id, role, join_status) "
                   "VALUES (%d, %d, 'member', 'approved')", club_id, id);
    } else {
        /* general_student: 清除特殊角色，去除 club_admins 关联和成员记录 */
        db_execute("DELETE FROM club_admins WHERE user_id=%d", id);
        db_execute("UPDATE users SET role='general_student' WHERE user_id=%d", id);
    }
    db_commit();

    db_execute("INSERT INTO logs (user_id, action, target_type, target_id, detail) "
               "VALUES (%d, 'set_role', 'users', %d, '修改用户角色为%s')",
               ctx->user->user_id, id, role);
    api_ok_msg(ctx, "角色已更新");
}

#endif /* 备用代码结束 */
