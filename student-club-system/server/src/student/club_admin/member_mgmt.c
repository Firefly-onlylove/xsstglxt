/**
 * member_mgmt.c — 社团管理端：成员管理 + 入社审批 + 任命角色
 * 学生社团管理系统 SCMS（社团管理端 club_admin）
 *
 * 覆盖处理函数（函数名/路由严格对齐 routes.h）：
 *   club_member_list     GET  /api/club/{id}/members
 *   club_join_pending    GET  /api/club/{id}/join-requests
 *   club_join_approve    POST /api/club/{id}/join-requests/{mid}/approve
 *   club_join_reject     POST /api/club/{id}/join-requests/{mid}/reject
 *   club_member_remove   POST /api/club/{id}/members/{mid}/remove
 *   club_member_appoint  POST /api/club/{id}/members/{mid}/appoint  任命副社长/干事
 *
 * 启用方法：把下面的 #if 0 改成 #if 1。
 *
 * 权限：社长/副社长可审批入社、移除成员；仅社长可任命角色。
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

/* club_require_manager / club_require_president 已提取到 api.c 公共函数 */

/* GET /api/club/{id}/members — 在籍成员列表 */
void club_member_list(ApiContext *ctx) {
    int club_id = api_get_path_int(ctx, 1);
    if (club_id <= 0) { api_error(ctx, ERR_INPUT, "社团ID非法"); return; }
    if (!api_require_club_admin(ctx, club_id)) return;

    MYSQL_RES *res = db_query(
        "SELECT m.member_id, m.user_id, u.real_name, u.student_no, u.phone, "
        "m.role, DATE_FORMAT(m.joined_at,'%%Y-%%m-%%d') AS join_date, "
        "COALESCE(col.college_name,'') AS college_name, m.attachment "
        "FROM members m JOIN users u ON m.user_id=u.user_id "
        "LEFT JOIN colleges col ON u.college_id=col.college_id "
        "WHERE m.club_id=%d AND m.join_status='approved' AND m.left_at IS NULL "
        "ORDER BY FIELD(m.role,'president','vice_president','secretary','treasurer','member'), "
        "m.joined_at", club_id);

    api_send_result(ctx, res);
}

/* GET /api/club/{id}/join-requests — 待审入社申请 */
void club_join_pending(ApiContext *ctx) {
    int club_id = api_get_path_int(ctx, 1);
    if (club_id <= 0) { api_error(ctx, ERR_INPUT, "社团ID非法"); return; }
    if (!api_require_club_admin(ctx, club_id)) return;

    MYSQL_RES *res = db_query(
        "SELECT m.member_id, m.user_id, u.real_name, u.student_no, u.phone, "
        "DATE_FORMAT(m.joined_at,'%%Y-%%m-%%d %%H:%%i') AS applied_at, "
        "COALESCE(col.college_name,'') AS college_name, m.attachment "
        "FROM members m JOIN users u ON m.user_id=u.user_id "
        "LEFT JOIN colleges col ON u.college_id=col.college_id "
        "WHERE m.club_id=%d AND m.join_status='pending' "
        "ORDER BY m.joined_at", club_id);

    api_send_result(ctx, res);
}

/* POST /api/club/{id}/join-requests/{mid}/approve — 通过入社 */
void club_join_approve(ApiContext *ctx) {
    int club_id = api_get_path_int(ctx, 1);
    int mid     = api_get_path_int(ctx, 3);   /* .../join-requests/{mid}/approve */
    if (club_id <= 0 || mid <= 0) { api_error(ctx, ERR_INPUT, "参数非法"); return; }
    if (!api_require_club_admin(ctx, club_id)) return;

    /* 取申请人 user_id 校验归属 */
    int applicant = db_query_int(
        "SELECT user_id FROM members WHERE member_id=%d AND club_id=%d "
        "AND join_status='pending'", mid, club_id);
    if (applicant <= 0) { api_error(ctx, ERR_NOT_FOUND, "申请不存在或已处理"); return; }

    db_execute("UPDATE members SET join_status='approved' WHERE member_id=%d", mid);
    db_execute("UPDATE clubs SET member_count=member_count+1 WHERE club_id=%d", club_id);
    db_execute("UPDATE users SET role='club_member' "
               "WHERE user_id=%d AND role='general_student'", applicant);

    notification_send(applicant, "入社申请已通过",
                      "您的入社申请已被社长通过，欢迎加入！", "join_result", club_id);
    api_ok_msg(ctx, "已通过");
}

/* POST /api/club/{id}/join-requests/{mid}/reject — 拒绝入社  body: reason */
void club_join_reject(ApiContext *ctx) {
    int club_id = api_get_path_int(ctx, 1);
    int mid     = api_get_path_int(ctx, 3);
    if (club_id <= 0 || mid <= 0) { api_error(ctx, ERR_INPUT, "参数非法"); return; }
    if (!api_require_club_admin(ctx, club_id)) return;

    int applicant = db_query_int(
        "SELECT user_id FROM members WHERE member_id=%d AND club_id=%d "
        "AND join_status='pending'", mid, club_id);
    if (applicant <= 0) { api_error(ctx, ERR_NOT_FOUND, "申请不存在或已处理"); return; }

    char reason[256] = "";
    api_get_json_str(ctx, "reason", reason, sizeof(reason));

    /* 拒绝：直接删除该申请记录（成员表无 rejected 状态时用软标记亦可） */
    db_execute("UPDATE members SET join_status='rejected', left_at=NOW() "
               "WHERE member_id=%d", mid);

    char msg[300];
    snprintf(msg, sizeof(msg), "您的入社申请未通过。%s%s",
             utils_is_empty(reason) ? "" : "原因：", reason);
    notification_send(applicant, "入社申请未通过", msg, "join_result", club_id);
    api_ok_msg(ctx, "已拒绝");
}

/* POST /api/club/{id}/members/{mid}/remove — 移除成员 */
void club_member_remove(ApiContext *ctx) {
    int club_id = api_get_path_int(ctx, 1);
    int mid     = api_get_path_int(ctx, 3);   /* .../members/{mid}/remove */
    if (club_id <= 0 || mid <= 0) { api_error(ctx, ERR_INPUT, "参数非法"); return; }
    if (!api_require_club_admin(ctx, club_id)) return;

    /* 取目标成员角色与 user_id */
    MYSQL_RES *res = db_query(
        "SELECT user_id, role FROM members WHERE member_id=%d AND club_id=%d "
        "AND join_status='approved' AND left_at IS NULL", mid, club_id);
    if (!res || mysql_num_rows(res) == 0) {
        if (res) mysql_free_result(res);
        api_error(ctx, ERR_NOT_FOUND, "成员不存在");
        return;
    }
    MYSQL_ROW row = mysql_fetch_row(res);
    int target_uid = atoi(row[0]);
    char role[16]; utils_strlcpy(role, row[1] ? row[1] : "member", sizeof(role));
    mysql_free_result(res);

    /* 不能移除社长 */
    if (utils_str_equal(role, "president")) {
        api_error(ctx, ERR_STATUS, "不能移除社长"); return;
    }

    db_execute("UPDATE members SET left_at=NOW() WHERE member_id=%d", mid);
    db_execute("UPDATE clubs SET member_count=GREATEST(member_count-1,0) WHERE club_id=%d",
               club_id);

    /* 如果不再属于任何社团，降回普通学生 */
    int still_in = db_query_int(
        "SELECT COUNT(*) FROM members WHERE user_id=%d "
        "AND join_status='approved' AND left_at IS NULL", target_uid);
    if (still_in == 0)
        db_execute("UPDATE users SET role='general_student' "
                   "WHERE user_id=%d AND role='club_member'", target_uid);

    notification_send(target_uid, "社团成员变动",
                      "您已被移出社团。", "member_change", club_id);
    api_ok_msg(ctx, "已移除该成员");
}

/* POST /api/club/{id}/members/{mid}/appoint — 任命角色  body: role
 * role ∈ vice_president / secretary / treasurer / member（取消职务）
 * 仅社长可操作；社长职位只能通过换届变更，不在此处。 */
void club_member_appoint(ApiContext *ctx) {
    int club_id = api_get_path_int(ctx, 1);
    int mid     = api_get_path_int(ctx, 3);
    if (club_id <= 0 || mid <= 0) { api_error(ctx, ERR_INPUT, "参数非法"); return; }
    if (!api_require_club_president(ctx, club_id)) return;

    char role[16] = "";
    api_get_json_str(ctx, "role", role, sizeof(role));
    if (!utils_str_equal(role, "vice_president") &&
        !utils_str_equal(role, "secretary") &&
        !utils_str_equal(role, "treasurer") &&
        !utils_str_equal(role, "member")) {
        api_error(ctx, ERR_VALIDATION, "角色取值非法"); return;
    }

    int target_uid = db_query_int(
        "SELECT user_id FROM members WHERE member_id=%d AND club_id=%d "
        "AND join_status='approved' AND left_at IS NULL AND role!='president'",
        mid, club_id);
    if (target_uid <= 0) { api_error(ctx, ERR_NOT_FOUND, "成员不存在或为社长"); return; }

    db_execute("UPDATE members SET role='%s' WHERE member_id=%d", role, mid);

    char msg[128];
    snprintf(msg, sizeof(msg), "您在社团中的职务已调整为：%s", role);
    notification_send(target_uid, "职务调整", msg, "member_change", club_id);
    api_ok_msg(ctx, "任命成功");
}

#endif /* 备用文件：改 #if 0 为 #if 1 启用 */
