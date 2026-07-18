/**
 * notification_center.c — 通知中心（消息列表 / 已读 / 未读数）
 * 学生社团管理系统 SCMS（公共 · 全角色共用，pub_ 前缀）
 *
 * 覆盖处理函数（函数名/路由严格对齐 routes.h）：
 *   pub_notif_list       GET  /api/notifications        我的消息（分页，可筛未读）
 *   pub_notif_unread     GET  /api/notifications/unread  未读数（顶部小红点）
 *   pub_notif_read       POST /api/notifications/read    标记单条已读（id 在 body）
 *   pub_notif_read_all   POST /api/notifications/read-all 全部标记已读
 *
 * 启用方法：把下面的 #if 0 改成 #if 1。
 *
 * 说明：所有角色都用，登录即可访问。消息只能看自己的（receiver_id=当前用户），
 *      保证不越界（例如报销消息只发给社长本人）。
 */
#if 1

#include "../../include/api.h"
#include "../../include/db.h"
#include "../../include/common.h"
#include "../../include/utils.h"
#include "../../include/routes.h"

#include <stdio.h>

/* GET /api/notifications?page=&type=&unread_only= — 我的消息列表 */
void pub_notif_list(ApiContext *ctx) {
    if (!api_require_login(ctx)) return;
    int uid = ctx->user->user_id;
    int page = api_get_int(ctx, "page", 1);
    if (page < 1) page = 1;
    int page_size = PAGE_SIZE_DEFAULT;
    int offset = (page - 1) * page_size;

    char type[32] = "";
    api_get_param(ctx, "type", type, sizeof(type));

    char filter[256];
    int fl = snprintf(filter, sizeof(filter), "user_id=%d", uid);

    if (utils_str_equal(type, "system")) {
        fl += snprintf(filter + fl, sizeof(filter) - fl,
                       " AND type IN ('system','announcement')");
    } else if (utils_str_equal(type, "club")) {
        fl += snprintf(filter + fl, sizeof(filter) - fl,
                       " AND type IN ('club','club_announce','join_request','join_result','role_change','member_removed')");
    } else if (utils_str_equal(type, "activity")) {
        fl += snprintf(filter + fl, sizeof(filter) - fl,
                       " AND type IN ('activity','activity_cancelled','activity_signin_remind','checkin')");
    }

    int total = db_query_int("SELECT COUNT(*) FROM notifications WHERE %s", filter);

    MYSQL_RES *res = db_query(
        "SELECT notification_id, title, content, type, related_id, is_read, created_at "
        "FROM notifications WHERE %s "
        "ORDER BY created_at DESC LIMIT %d OFFSET %d",
        filter, page_size, offset);

    api_send_result_paged(ctx, res, page, page_size, total);
}

/* GET /api/notifications/unread — 未读数 */
void pub_notif_unread(ApiContext *ctx) {
    if (!api_require_login(ctx)) return;
    int uid = ctx->user->user_id;

    int cnt = db_query_int(
        "SELECT COUNT(*) FROM notifications WHERE user_id=%d AND is_read=0", uid);

    JsonBuilder jb;
    json_init(&jb);
    json_add_int(&jb, "unread", cnt);
    api_ok_data(ctx, json_finish(&jb));
    json_free(&jb);
}

/* POST /api/notifications/read — 单条已读，body: notification_id */
void pub_notif_read(ApiContext *ctx) {
    if (!api_require_login(ctx)) return;
    int uid = ctx->user->user_id;
    int nid = api_get_json_int(ctx, "notification_id", 0);
    if (nid <= 0) { api_error(ctx, ERR_INPUT, "消息ID非法"); return; }

    /* 加 receiver_id 条件，确保只能操作自己的消息 */
    db_execute("UPDATE notifications SET is_read=1, read_at=NOW() "
               "WHERE notification_id=%d AND user_id=%d", nid, uid);
    api_ok_msg(ctx, "已读");
}

/* POST /api/notifications/read-all — 全部已读 */
void pub_notif_read_all(ApiContext *ctx) {
    if (!api_require_login(ctx)) return;
    int uid = ctx->user->user_id;

    db_execute("UPDATE notifications SET is_read=1, read_at=NOW() "
               "WHERE user_id=%d AND is_read=0", uid);
    api_ok_msg(ctx, "全部已读");
}

#endif /* 备用文件：改 #if 0 为 #if 1 启用 */
