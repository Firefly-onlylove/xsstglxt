/**
 * profile.c — 个人中心（查看/修改资料、修改密码）
 * 学生社团管理系统 SCMS（学生端）
 *
 * 覆盖处理函数：
 *   stu_profile_get       GET  /api/my/profile          我的资料
 *   stu_profile_update    PUT  /api/my/profile          修改联系方式
 *   stu_change_password   POST /api/my/change-password  修改密码
 *
 * 启用方法：把下面的 #if 0 改成 #if 1。
 *
 * 业务要点：学号/身份证/学院专业班级等身份信息不允许自行修改，
 *          只能改手机号 phone 和邮箱 email。改密码需校验旧密码。
 */
#if 0

#include "../../include/api.h"
#include "../../include/db.h"
#include "../../include/common.h"
#include "../../include/utils.h"
#include "../../include/routes.h"

#include <stdio.h>
#include <string.h>

/* GET /api/my/profile — 当前登录用户资料 */
void stu_profile_get(ApiContext *ctx) {
    if (!api_require_login(ctx)) return;
    int uid = ctx->user->user_id;

    MYSQL_RES *res = db_query(
        "SELECT u.user_id, u.username, u.real_name, u.role, u.student_no, "
        "u.phone, u.email, u.id_card, "
        "COALESCE(col.college_name,'') AS college_name, "
        "COALESCE(m.major_name,'') AS major_name, "
        "COALESCE(cl.class_name,'') AS class_name, u.created_at, u.last_login "
        "FROM users u "
        "LEFT JOIN colleges col ON u.college_id=col.college_id "
        "LEFT JOIN majors m ON u.major_id=m.major_id "
        "LEFT JOIN classes cl ON u.class_id=cl.class_id "
        "WHERE u.user_id=%d", uid);

    if (!res || mysql_num_rows(res) == 0) {
        if (res) mysql_free_result(res);
        api_error(ctx, ERR_NOT_FOUND, "用户不存在");
        return;
    }
    api_send_result_data(ctx, res);
}

/* PUT /api/my/profile — 修改手机号 / 邮箱 */
void stu_profile_update(ApiContext *ctx) {
    if (!api_require_login(ctx)) return;
    int uid = ctx->user->user_id;

    char phone[16] = "", email[128] = "";
    api_get_json_str(ctx, "phone", phone, sizeof(phone));
    api_get_json_str(ctx, "email", email, sizeof(email));

    if (!utils_is_empty(phone) && !utils_validate_phone(phone)) {
        api_error(ctx, ERR_VALIDATION, "手机号格式不正确"); return;
    }

    char *e_email = db_escape(email);
    db_execute("UPDATE users SET phone='%s', email='%s' WHERE user_id=%d",
               phone, e_email, uid);
    free(e_email);

    api_ok_msg(ctx, "资料已更新");
}

/* POST /api/my/change-password — body: old_password, new_password */
void stu_change_password(ApiContext *ctx) {
    if (!api_require_login(ctx)) return;
    int uid = ctx->user->user_id;

    char old_pwd[128] = "", new_pwd[128] = "";
    api_get_json_str(ctx, "old_password", old_pwd, sizeof(old_pwd));
    api_get_json_str(ctx, "new_password", new_pwd, sizeof(new_pwd));

    if (utils_is_empty(new_pwd) || strlen(new_pwd) < 6) {
        api_error(ctx, ERR_VALIDATION, "新密码至少6位"); return;
    }

    /* 校验旧密码 */
    char old_hash[SHA256_HEX_LENGTH];
    utils_sha256(old_pwd, old_hash);
    int match = db_query_int(
        "SELECT COUNT(*) FROM users WHERE user_id=%d AND password='%s'",
        uid, old_hash);
    if (match == 0) { api_error(ctx, ERR_VALIDATION, "原密码不正确"); return; }

    /* 更新为新密码 */
    char new_hash[SHA256_HEX_LENGTH];
    utils_sha256(new_pwd, new_hash);
    db_execute("UPDATE users SET password='%s' WHERE user_id=%d", new_hash, uid);

    api_ok_msg(ctx, "密码已修改，请重新登录");
}

#endif /* 备用文件：改 #if 0 为 #if 1 启用 */
