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

/* POST /api/club/{id}/announcements — 发布公告  body: title, content */
void club_announce_post(ApiContext *ctx) {
    int club_id = api_get_path_int(ctx, 1);
    if (club_id <= 0) { api_error(ctx, ERR_INPUT, "社团ID非法"); return; }
    if (!api_require_club_admin(ctx, club_id)) return;

    char title[128] = "", content[2000] = "";
    api_get_json_str(ctx, "title",   title,   sizeof(title));
    api_get_json_str(ctx, "content", content, sizeof(content));
    if (utils_is_empty(title))   { api_error(ctx, ERR_VALIDATION, "请填写公告标题"); return; }
    if (utils_is_empty(content)) { api_error(ctx, ERR_VALIDATION, "请填写公告内容"); return; }

    /* 群发给本社全体在籍成员（使用批量广播，单条INSERT SELECT） */
    notification_broadcast_club(club_id, title, content, "club_announce", club_id);

    api_ok_msg(ctx, "公告已发布");
}

#endif /* 备用文件：改 #if 0 为 #if 1 启用 */
