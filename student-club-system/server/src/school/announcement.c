/**
 * announcement.c — 学校端 全校公告发布与管理
 * 学生社团管理系统 SCMS（Web 架构 · 学校端）
 *
 * ┌──────────────────────────────────────────────────────────────────┐
 * │  ★ 备用参考实现。默认整份被 #if 0 关闭，不参与编译（走弱符号占位）。 │
 * │  ★ 启用方法：把下面这行的  #if 0  改成  #if 1  ，重新 make 即可。   │
 * └──────────────────────────────────────────────────────────────────┘
 *
 * 覆盖路由：
 *   GET    /api/school/announcements       → sch_announce_list
 *   POST   /api/school/announcements       → sch_announce_post   （发布并全校推送通知）
 *   DELETE /api/school/announcements/{id}  → sch_announce_delete
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

/* GET /api/school/announcements  最近全校公告 */
void sch_announce_list(ApiContext *ctx) {
    if (!api_require_school_admin(ctx)) return;
    MYSQL_RES *res = db_query(
        "SELECT a.announcement_id, a.title, a.content, "
        "COALESCE(u.real_name,'—') AS publisher_name, a.created_at "
        "FROM announcements a LEFT JOIN users u ON a.publisher_id=u.user_id "
        "WHERE a.scope='school' ORDER BY a.created_at DESC LIMIT 50");
    api_send_result_data(ctx, res);
}

/* POST /api/school/announcements  body: title, content */
void sch_announce_post(ApiContext *ctx) {
    if (!api_require_school_admin(ctx)) return;

    char title[201] = "", content[2001] = "";
    api_get_json_str(ctx, "title", title, sizeof(title));
    api_get_json_str(ctx, "content", content, sizeof(content));
    if (utils_is_empty(title) || utils_is_empty(content)) {
        api_error(ctx, ERR_INPUT, "标题和内容均必填"); return;
    }

    char *et = db_escape(title), *ec = db_escape(content);
    int ok = db_execute(
        "INSERT INTO announcements (title, content, scope, publisher_id) "
        "VALUES ('%s','%s','school',%d)", et, ec, ctx->user->user_id);
    free(et); free(ec);
    if (ok < 0) { api_error(ctx, ERR_DB, "发布失败"); return; }

    db_execute("INSERT INTO logs (user_id, action, target_type, target_id, detail) "
               "VALUES (%d, 'post_announcement', 'announcements', %lu, '发布全校公告')",
               ctx->user->user_id, db_last_insert_id());

    /* 全校推送 */
    notification_broadcast_school(title, content, ctx->user->user_id);
    api_ok_msg(ctx, "公告已发布，全校用户将收到通知");
}

/* DELETE /api/school/announcements/{id} */
void sch_announce_delete(ApiContext *ctx) {
    if (!api_require_school_admin(ctx)) return;
    int id = api_get_path_int(ctx, 2);
    if (id <= 0) { api_error(ctx, ERR_INPUT, "公告ID非法"); return; }

    int ok = db_execute("DELETE FROM announcements WHERE announcement_id=%d AND scope='school'", id);
    if (ok <= 0) { api_error(ctx, ERR_NOT_FOUND, "公告不存在"); return; }

    db_execute("INSERT INTO logs (user_id, action, target_type, target_id, detail) "
               "VALUES (%d, 'delete_announcement', 'announcements', %d, '删除公告')",
               ctx->user->user_id, id);
    api_ok_msg(ctx, "已删除");
}

#endif /* 备用代码结束 */
