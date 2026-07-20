/**
 * announcement.c — 社团管理端：社团公告发布
 * 学生社团管理系统 SCMS（社团管理端 club_admin）
 *
 * 覆盖处理函数（函数名/路由严格对齐 routes.h）：
 *   club_announce_post   POST /api/club/{id}/announcements   发布社团公告
 *
 * 启用方法：把下面的 #if 0 改成 #if 1。
 *
 * 说明：社长/副社长向本社全体在籍成员群发通知（不越界，仅本社成员收到）。
 */
#if 1

#include "../../../include/api.h"
#include "../../../include/db.h"
#include "../../../include/common.h"
#include "../../../include/utils.h"
#include "../../../include/routes.h"
#include "../../../include/notification.h"

#include <stdio.h>
#include <string.h>

static int club_require_manager(ApiContext *ctx, int club_id) {
    if (!api_require_login(ctx)) return 0;
    if (utils_str_equal(ctx->user->role, "school_admin")) return 1;
    int is_mgr = db_query_int(
        "SELECT COUNT(*) FROM members WHERE club_id=%d AND user_id=%d "
        "AND join_status='approved' AND left_at IS NULL "
        "AND role IN ('president','vice_president')", club_id, ctx->user->user_id);
    if (is_mgr == 0) { api_error(ctx, ERR_PERMISSION, "仅社长/副社长可操作"); return 0; }
    return 1;
}

/* POST /api/club/{id}/announcements — 发布公告  body: title, content */
void club_announce_post(ApiContext *ctx) {
    int club_id = api_get_path_int(ctx, 1);
    if (club_id <= 0) { api_error(ctx, ERR_INPUT, "社团ID非法"); return; }
    if (!club_require_manager(ctx, club_id)) return;

    char title[128] = "", content[2000] = "";
    api_get_json_str(ctx, "title",   title,   sizeof(title));
    api_get_json_str(ctx, "content", content, sizeof(content));
    if (utils_is_empty(title))   { api_error(ctx, ERR_VALIDATION, "请填写公告标题"); return; }
    if (utils_is_empty(content)) { api_error(ctx, ERR_VALIDATION, "请填写公告内容"); return; }

    /* 群发给本社全体在籍成员 */
    MYSQL_RES *res = db_query(
        "SELECT user_id FROM members WHERE club_id=%d "
        "AND join_status='approved' AND left_at IS NULL", club_id);
    int sent = 0;
    if (res) {
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(res)) != NULL) {
            if (row[0]) {
                notification_send(atoi(row[0]), title, content, "club_announce", club_id);
                sent++;
            }
        }
        mysql_free_result(res);
    }

    char msg[64];
    snprintf(msg, sizeof(msg), "公告已发送给 %d 名成员", sent);
    api_ok_msg(ctx, msg);
}

#endif /* 备用文件：改 #if 0 为 #if 1 启用 */
