/**
 * club_oversight.c — 学院端 本院社团监督（查看、重命名、解散 + 成员/活动）
 * 学生社团管理系统 SCMS（Web 架构）
 *
 * 对应处理函数：col_club_list / col_club_detail / col_club_members /
 *   col_club_activities / col_club_rename / col_club_dissolve
 * 对应前端页面：C1 主页下方本院社团列表 / 社团监督
 *
 * 启用方法：把下面第一行的  #if 0  改成  #if 1。
 *
 * 权限要点：学院只能操作本院院级社团。
 */
#if 1

#include "../../include/api.h"
#include "../../include/db.h"
#include "../../include/common.h"
#include "../../include/routes.h"
#include "../../include/utils.h"
#include "../../include/notification.h"

#include <stdio.h>

/* GET /api/college/clubs —— 本院院级社团列表（也可看校级） */
void col_club_list(ApiContext *ctx) {
    if (!api_require_college_admin(ctx)) return;
    int cid = ctx->user->college_id;

    char search[101] = "";
    api_get_param(ctx, "search", search, sizeof(search));

    /* 本院院级社团 + 校级社团（校级 college_id 为 NULL，对所有人可见） */
    if (search[0]) {
        char *e = db_escape(search);
        MYSQL_RES *res = db_query(
            "SELECT c.club_id, c.club_name, c.category, c.level, "
            "c.member_count, c.status, u.real_name AS president_name, "
            "(SELECT COUNT(*) FROM activities WHERE activities.club_id=c.club_id AND status IN ('published','ongoing','completed')) AS activity_count, "
            "DATE_FORMAT(c.created_at,'%%Y-%%m-%%d') AS created_at "
            "FROM clubs c LEFT JOIN users u ON c.president_id=u.user_id "
            "WHERE (c.college_id=%d OR c.level='school') "
            "AND c.status='approved' AND c.club_name LIKE '%%%s%%' "
            "ORDER BY c.level, c.club_id", cid, e);
        free(e);
        api_send_result_data(ctx, res);
    } else {
        MYSQL_RES *res = db_query(
            "SELECT c.club_id, c.club_name, c.category, c.level, "
            "c.member_count, c.status, u.real_name AS president_name, "
            "(SELECT COUNT(*) FROM activities WHERE activities.club_id=c.club_id AND status IN ('published','ongoing','completed')) AS activity_count, "
            "DATE_FORMAT(c.created_at,'%%Y-%%m-%%d') AS created_at "
            "FROM clubs c LEFT JOIN users u ON c.president_id=u.user_id "
            "WHERE (c.college_id=%d OR c.level='school') "
            "AND c.status='approved' "
            "ORDER BY c.level, c.club_id", cid);
        api_send_result_data(ctx, res);
    }
}

/* GET /api/college/clubs/{id} —— 社团详情（基本信息 + 成员 + 近期活动） */
void col_club_detail(ApiContext *ctx) {
    if (!api_require_college_admin(ctx)) return;
    int cid = ctx->user->college_id;
    int club_id = api_get_path_int(ctx, 2);   /* /api/college/clubs/{id} */

    /* 只能看本院院级或校级社团 */
    MYSQL_RES *res = db_query(
        "SELECT c.club_id, c.club_name, c.description, c.category, c.level, "
        "c.member_count, c.advisor, c.status, u.real_name AS president_name, "
        "col.college_name "
        "FROM clubs c "
        "LEFT JOIN users u ON c.president_id=u.user_id "
        "LEFT JOIN colleges col ON c.college_id=col.college_id "
        "WHERE c.club_id=%d AND (c.college_id=%d OR c.level='school')",
        club_id, cid);
    if (!res || mysql_num_rows(res) == 0) {
        if (res) mysql_free_result(res);
        api_error(ctx, ERR_NOT_FOUND, "社团不存在或无权查看");
        return;
    }
    /* 直接把社团基本信息作为对象返回；成员/活动前端另调专门接口 */
    api_send_result_data(ctx, res);
}

#endif
