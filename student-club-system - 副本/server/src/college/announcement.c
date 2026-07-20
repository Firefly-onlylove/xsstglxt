/**
 * announcement.c — 学院端 本院公告发布与查看
 * 学生社团管理系统 SCMS（Web 架构）
 *
 * 对应处理函数：col_announce_list / col_announce_post
 * 对应前端页面：C1 主页本院公告区
 *
 * 启用方法：把下面第一行的  #if 0  改成  #if 1。
 *
 * 权限要点：学院公告 scope='college'，college_id 固定为本院；
 * 发布后通知本院全体学生（notification_broadcast_college）。
 */
#if 1

#include "../../include/api.h"
#include "../../include/db.h"
#include "../../include/common.h"
#include "../../include/routes.h"
#include "../../include/utils.h"
#include "../../include/notification.h"

#include <stdio.h>

/* GET /api/college/announcements —— 本院公告历史 */
void col_announce_list(ApiContext *ctx) {
    if (!api_require_college_admin(ctx)) return;
    int cid = ctx->user->college_id;

    MYSQL_RES *res = db_query(
        "SELECT a.announcement_id, a.title, a.content, u.real_name AS publisher_name, "
        "a.created_at "
        "FROM announcements a LEFT JOIN users u ON a.publisher_id=u.user_id "
        "WHERE a.scope='college' AND a.college_id=%d "
        "ORDER BY a.created_at DESC LIMIT 50", cid);
    api_send_result_data(ctx, res);
}

/* POST /api/college/announcements —— 发布本院公告 body: title, content */
void col_announce_post(ApiContext *ctx) {
    if (!api_require_college_admin(ctx)) return;
    int cid = ctx->user->college_id;

    char title[201] = "", content[2001] = "";
    api_get_json_str(ctx, "title", title, sizeof(title));
    api_get_json_str(ctx, "content", content, sizeof(content));
    if (utils_is_empty(title) || utils_is_empty(content)) {
        api_error(ctx, ERR_INPUT, "请填写公告标题和内容");
        return;
    }

    char *e_title = db_escape(title);
    char *e_content = db_escape(content);
    db_execute("INSERT INTO announcements (title, content, scope, college_id, publisher_id) "
               "VALUES ('%s', '%s', 'college', %d, %d)",
               e_title, e_content, cid, ctx->user->user_id);
    free(e_title); free(e_content);

    db_execute("INSERT INTO logs (user_id, action, target_type, target_id, detail) "
               "VALUES (%d, 'post_announcement', 'announcements', LAST_INSERT_ID(), '发布学院公告')",
               ctx->user->user_id);

    /* 通知本院全体学生 */
    notification_broadcast_college(cid, title, content, ctx->user->user_id);
    api_ok_msg(ctx, "公告已发布，本院学生将收到通知");
}

/* DELETE /api/college/announcements/{id} */
void col_announce_delete(ApiContext *ctx) {
    if (!api_require_college_admin(ctx)) return;
    int cid = ctx->user->college_id;
    int id = api_get_path_int(ctx, 2);
    if (id <= 0) { api_error(ctx, ERR_INPUT, "公告ID非法"); return; }

    int ok = db_execute("DELETE FROM announcements WHERE announcement_id=%d AND scope='college' AND college_id=%d", id, cid);
    if (ok <= 0) { api_error(ctx, ERR_NOT_FOUND, "公告不存在或无权删除"); return; }

    db_execute("INSERT INTO logs (user_id, action, target_type, target_id, detail) "
               "VALUES (%d, 'delete_announcement', 'announcements', %d, '删除学院公告')",
               ctx->user->user_id, id);
    api_ok_msg(ctx, "已删除");
}

#endif
