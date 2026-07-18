/**
 * club_dashboard.c — 社团管理端：仪表盘 + 社团资料维护
 * 学生社团管理系统 SCMS（社团管理端 club_admin，社长/副社长）
 *
 * 覆盖处理函数（函数名/路由严格对齐 routes.h）：
 *   club_dashboard       GET  /api/club/{id}/dashboard   社团概览数据
 *   club_profile_get     GET  /api/club/{id}/profile      社团资料
 *   club_profile_update  PUT  /api/club/{id}/profile      修改简介/公告/加入权限
 *
 * 启用方法：把下面的 #if 0 改成 #if 1。
 *
 * 权限：所有 /api/club/{id}/* 接口都要先 club_require_manager(ctx, club_id)
 *      校验当前用户是该社团的 president 或 vice_president。
 */
#if 1

#include "../../../include/api.h"
#include "../../../include/db.h"
#include "../../../include/common.h"
#include "../../../include/utils.h"
#include "../../../include/routes.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 内部辅助：校验当前登录用户是否为该社团的社长/副社长。
 * 通过返回 1；否则写好错误响应并返回 0。 */
static int club_require_manager(ApiContext *ctx, int club_id) {
    if (!api_require_login(ctx)) return 0;
    int uid = ctx->user->user_id;

    /* 学校管理员拥有全部社团管理权限 */
    if (utils_str_equal(ctx->user->role, "school_admin")) return 1;

    int is_mgr = db_query_int(
        "SELECT COUNT(*) FROM members "
        "WHERE club_id=%d AND user_id=%d AND join_status='approved' AND left_at IS NULL "
        "AND role IN ('president','vice_president')", club_id, uid);
    if (is_mgr == 0) {
        api_error(ctx, ERR_PERMISSION, "仅社长/副社长可操作");
        return 0;
    }
    return 1;
}

/* GET /api/club/{id}/dashboard — 概览：成员数、活动数、待审入社、经费余额 */
void club_dashboard(ApiContext *ctx) {
    int club_id = api_get_path_int(ctx, 1);   /* /api/club/{id}/dashboard */
    if (club_id <= 0) { api_error(ctx, ERR_INPUT, "社团ID非法"); return; }
    if (!club_require_manager(ctx, club_id)) return;

    int member_cnt = db_query_int(
        "SELECT COUNT(*) FROM members WHERE club_id=%d "
        "AND join_status='approved' AND left_at IS NULL", club_id);
    int pending_cnt = db_query_int(
        "SELECT COUNT(*) FROM members WHERE club_id=%d AND join_status='pending'", club_id);
    int activity_cnt = db_query_int(
        "SELECT COUNT(*) FROM activities WHERE club_id=%d "
        "AND status IN ('published','ongoing')", club_id);
    /* 经费余额：收入合计 - 已报销支出合计 */
    double income = db_query_double(
        "SELECT COALESCE(SUM(amount),0) FROM finance "
        "WHERE club_id=%d AND type='income'", club_id);
    double expense = db_query_double(
        "SELECT COALESCE(SUM(amount),0) FROM finance "
        "WHERE club_id=%d AND type='expense'", club_id);

    JsonBuilder jb;
    json_init(&jb);
    json_add_int(&jb, "member_count", member_cnt);
    json_add_int(&jb, "pending_join", pending_cnt);
    json_add_int(&jb, "activity_count", activity_cnt);
    json_add_double(&jb, "balance", income - expense);
    api_ok_data(ctx, json_finish(&jb));
    json_free(&jb);
}

/* GET /api/club/{id}/profile — 社团资料（管理视角，含待审信息） */
void club_profile_get(ApiContext *ctx) {
    int club_id = api_get_path_int(ctx, 1);
    if (club_id <= 0) { api_error(ctx, ERR_INPUT, "社团ID非法"); return; }
    if (!club_require_manager(ctx, club_id)) return;

    MYSQL_RES *res = db_query(
        "SELECT c.club_id, c.club_name, c.description, c.category, c.level, "
        "c.logo_path, c.advisor, c.join_permission, c.member_count, c.status, "
        "COALESCE(col.college_name,'') AS college_name, "
        "COALESCE(u.real_name,'') AS president_name "
        "FROM clubs c "
        "LEFT JOIN colleges col ON c.college_id=col.college_id "
        "LEFT JOIN users u ON c.president_id=u.user_id "
        "WHERE c.club_id=%d", club_id);

    if (!res || mysql_num_rows(res) == 0) {
        if (res) mysql_free_result(res);
        api_error(ctx, ERR_NOT_FOUND, "社团不存在");
        return;
    }
    api_send_result_data(ctx, res);
}

/* PUT /api/club/{id}/profile — 修改简介、指导老师、加入权限 */
void club_profile_update(ApiContext *ctx) {
    int club_id = api_get_path_int(ctx, 1);
    if (club_id <= 0) { api_error(ctx, ERR_INPUT, "社团ID非法"); return; }
    if (!club_require_manager(ctx, club_id)) return;

    char description[1000] = "", advisor[64] = "", join_perm[16] = "";
    api_get_json_str(ctx, "description",     description, sizeof(description));
    api_get_json_str(ctx, "advisor",         advisor,     sizeof(advisor));
    api_get_json_str(ctx, "join_permission", join_perm,   sizeof(join_perm));

    /* join_permission 仅允许 free / approval */
    if (!utils_is_empty(join_perm) &&
        !utils_str_equal(join_perm, "free") &&
        !utils_str_equal(join_perm, "approval")) {
        api_error(ctx, ERR_VALIDATION, "加入权限取值非法"); return;
    }
    if (utils_is_empty(join_perm)) utils_strlcpy(join_perm, "approval", sizeof(join_perm));

    char *e_desc = db_escape(description);
    char *e_adv  = db_escape(advisor);
    db_execute("UPDATE clubs SET description='%s', advisor='%s', join_permission='%s' "
               "WHERE club_id=%d", e_desc, e_adv, join_perm, club_id);
    free(e_desc); free(e_adv);

    api_ok_msg(ctx, "社团资料已更新");
}

#endif /* 备用文件：改 #if 0 为 #if 1 启用 */
