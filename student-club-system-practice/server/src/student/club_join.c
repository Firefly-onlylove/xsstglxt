/**
 * club_join.c — 加入 / 退出社团（含加入审批相关的申请方逻辑）
 * 学生社团管理系统 SCMS（学生端）
 *
 * 覆盖处理函数（函数名/路由严格对齐 routes.h）：
 *   stu_club_join    POST /api/clubs/{id}/join     申请加入社团
 *   stu_club_leave   POST /api/clubs/{id}/leave    退出社团
 *   stu_my_clubs     GET  /api/my/clubs            我加入的社团列表
 *
 * 启用方法：把下面的 #if 0 改成 #if 1。
 *
 * 业务要点：
 *  - 加入前检查：账号是否被限制 ban_join；是否已在社团中。
 *  - join_permission = free 直接通过（approved），否则挂 pending 待社长审批。
 *  - 社长(president)不能直接退出（需先换届），其余成员可退。
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

/* POST /api/clubs/{id}/join — 申请加入 */
void stu_club_join(ApiContext *ctx) {
    if (!api_require_login(ctx)) return;
    int uid = ctx->user->user_id;
    int club_id = api_get_path_int(ctx, 1);
    if (club_id <= 0) { api_error(ctx, ERR_INPUT, "社团ID非法"); return; }

    /* 1. 账号是否被禁止加入社团（生效中的 ban_join 限制） */
    int banned = db_query_int(
        "SELECT COUNT(*) FROM user_restrictions "
        "WHERE user_id=%d AND restriction_type='ban_join' AND is_active=1 "
        "AND (end_time IS NULL OR end_time > NOW())", uid);
    if (banned > 0) { api_error(ctx, ERR_PERMISSION, "您已被限制加入社团"); return; }

    /* 2. 社团是否存在且已审批，取加入权限设置 */
    MYSQL_RES *res = db_query(
        "SELECT join_permission, president_id FROM clubs "
        "WHERE club_id=%d AND status='approved'", club_id);
    if (!res || mysql_num_rows(res) == 0) {
        if (res) mysql_free_result(res);
        api_error(ctx, ERR_NOT_FOUND, "社团不存在或未开放");
        return;
    }
    MYSQL_ROW row = mysql_fetch_row(res);
    char join_perm[16];
    utils_strlcpy(join_perm, row[0] ? row[0] : "free", sizeof(join_perm));
    int president_id = row[1] ? atoi(row[1]) : 0;
    mysql_free_result(res);

    /* 3. 是否已是在籍成员或已有待审申请 */
    int existing = db_query_int(
        "SELECT COUNT(*) FROM members "
        "WHERE club_id=%d AND user_id=%d "
        "AND (left_at IS NULL) AND join_status IN ('pending','approved')",
        club_id, uid);
    if (existing > 0) { api_error(ctx, ERR_DUPLICATE, "您已加入或已提交申请"); return; }

    /* 4. 依据加入权限决定状态 */
    const char *status = utils_str_equal(join_perm, "free") ? "approved" : "pending";

    int rc = db_execute(
        "INSERT INTO members (club_id, user_id, role, join_status, joined_at) "
        "VALUES (%d, %d, 'member', '%s', NOW())",
        club_id, uid, status);
    if (rc < 0) { api_error(ctx, ERR_DB, "加入失败"); return; }

    if (utils_str_equal(status, "approved")) {
        /* 自由加入：成员数 +1，普通学生升级为 club_member（若当前是 general_student） */
        db_execute("UPDATE clubs SET member_count=member_count+1 WHERE club_id=%d", club_id);
        db_execute("UPDATE users SET role='club_member' "
                   "WHERE user_id=%d AND role='general_student'", uid);
        api_ok_msg(ctx, "已成功加入社团");
    } else {
        /* 需审批：通知社长 */
        if (president_id > 0) {
            char msg[256];
            snprintf(msg, sizeof(msg), "%s 申请加入您的社团，请前往成员管理审批。",
                     ctx->user->real_name);
            notification_send(president_id, "新的入社申请", msg,
                              "join_request", club_id);
        }
        api_ok_msg(ctx, "申请已提交，等待社长审批");
    }
}

/* POST /api/clubs/{id}/leave — 退出社团 */
void stu_club_leave(ApiContext *ctx) {
    if (!api_require_login(ctx)) return;
    int uid = ctx->user->user_id;
    int club_id = api_get_path_int(ctx, 1);
    if (club_id <= 0) { api_error(ctx, ERR_INPUT, "社团ID非法"); return; }

    /* 查询该成员当前角色 */
    MYSQL_RES *res = db_query(
        "SELECT member_id, role FROM members "
        "WHERE club_id=%d AND user_id=%d AND join_status='approved' AND left_at IS NULL",
        club_id, uid);
    if (!res || mysql_num_rows(res) == 0) {
        if (res) mysql_free_result(res);
        api_error(ctx, ERR_NOT_FOUND, "您不是该社团成员");
        return;
    }
    MYSQL_ROW row = mysql_fetch_row(res);
    char role[16];
    utils_strlcpy(role, row[1] ? row[1] : "member", sizeof(role));
    mysql_free_result(res);

    /* 社长不能直接退出 */
    if (utils_str_equal(role, "president")) {
        api_error(ctx, ERR_STATUS, "社长不能直接退出，请先完成换届");
        return;
    }

    /* 标记离开：left_at 置当前时间 */
    db_execute("UPDATE members SET left_at=NOW() "
               "WHERE club_id=%d AND user_id=%d AND left_at IS NULL", club_id, uid);
    db_execute("UPDATE clubs SET member_count=GREATEST(member_count-1,0) WHERE club_id=%d",
               club_id);

    /* 如果该用户不再属于任何社团，角色降回 general_student */
    int still_in = db_query_int(
        "SELECT COUNT(*) FROM members "
        "WHERE user_id=%d AND join_status='approved' AND left_at IS NULL", uid);
    if (still_in == 0) {
        db_execute("UPDATE users SET role='general_student' "
                   "WHERE user_id=%d AND role='club_member'", uid);
    }

    api_ok_msg(ctx, "已退出社团");
}

/* GET /api/my/clubs — 我加入的社团（含我的角色与状态） */
void stu_my_clubs(ApiContext *ctx) {
    if (!api_require_login(ctx)) return;
    int uid = ctx->user->user_id;

    MYSQL_RES *res = db_query(
        "SELECT c.club_id, c.club_name, c.category, c.level, c.logo_path, "
        "m.role, m.join_status, m.joined_at "
        "FROM members m JOIN clubs c ON m.club_id=c.club_id "
        "WHERE m.user_id=%d AND m.left_at IS NULL "
        "ORDER BY m.joined_at DESC", uid);

    api_send_result(ctx, res);
}

#endif /* 备用文件：改 #if 0 为 #if 1 启用 */
