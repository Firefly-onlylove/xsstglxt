/**
 * club_mgmt.c — 学校端 社团管理（列表/详情/级别/解散）
 * 学生社团管理系统 SCMS（Web 架构 · 学校端）
 *
 * ┌──────────────────────────────────────────────────────────────────┐
 * │  ★ 备用参考实现。默认整份被 #if 0 关闭，不参与编译（走弱符号占位）。 │
 * │  ★ 启用方法：把下面这行的  #if 0  改成  #if 1  ，重新 make 即可。   │
 * └──────────────────────────────────────────────────────────────────┘
 *
 * 覆盖路由：
 *   GET  /api/school/clubs             → sch_club_list       全校社团列表（筛选）
 *   GET  /api/school/clubs/{id}        → sch_club_detail      社团详情+成员+活动
 *   POST /api/school/clubs/{id}/level  → sch_club_set_level   设级别
 *   POST /api/school/clubs/{id}/dissolve → sch_club_dissolve  解散社团
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

/* GET /api/school/clubs?level=&status=&college_id=&keyword=&page=
 * 全校社团列表，支持多条件筛选 + 分页。 */
void sch_club_list(ApiContext *ctx) {
    if (!api_require_school_admin(ctx)) return;

    char level[16] = "", status[16] = "", keyword[64] = "";
    api_get_param(ctx, "level", level, sizeof(level));
    api_get_param(ctx, "status", status, sizeof(status));
    api_get_param(ctx, "keyword", keyword, sizeof(keyword));
    int college_id = api_get_int(ctx, "college_id", 0);
    int page = api_get_int(ctx, "page", 1);
    if (page < 1) page = 1;
    int page_size = PAGE_SIZE_DEFAULT;
    int offset = (page - 1) * page_size;

    /* 动态拼 WHERE 子句 */
    char where[512] = "WHERE 1=1";
    if (level[0]) {
        char *e = db_escape(level);
        char frag[64]; snprintf(frag, sizeof(frag), " AND cl.level='%s'", e);
        strncat(where, frag, sizeof(where)-strlen(where)-1); free(e);
    }
    if (status[0]) {
        char *e = db_escape(status);
        char frag[64]; snprintf(frag, sizeof(frag), " AND cl.status='%s'", e);
        strncat(where, frag, sizeof(where)-strlen(where)-1); free(e);
    }
    if (college_id > 0) {
        char frag[64]; snprintf(frag, sizeof(frag), " AND cl.college_id=%d", college_id);
        strncat(where, frag, sizeof(where)-strlen(where)-1);
    }
    if (keyword[0]) {
        char *e = db_escape(keyword);
        char frag[128]; snprintf(frag, sizeof(frag), " AND cl.club_name LIKE '%%%s%%'", e);
        strncat(where, frag, sizeof(where)-strlen(where)-1); free(e);
    }

    int total = db_query_int("SELECT COUNT(*) FROM clubs cl %s", where);

    MYSQL_RES *res = db_query(
        "SELECT cl.club_id, cl.club_name, cl.category, cl.level, cl.status, "
        "COALESCE(col.college_name,'-') AS college_name, "
        "COALESCE(u.real_name,'-') AS president, cl.member_count "
        "FROM clubs cl "
        "LEFT JOIN colleges col ON cl.college_id=col.college_id "
        "LEFT JOIN users u ON cl.president_id=u.user_id "
        "%s ORDER BY cl.club_id DESC LIMIT %d OFFSET %d",
        where, page_size, offset);
    api_send_result_paged(ctx, res, page, page_size, total);
}

/* GET /api/school/clubs/{id}  社团详情 + 成员 + 最近活动 */
void sch_club_detail(ApiContext *ctx) {
    if (!api_require_school_admin(ctx)) return;
    int club_id = api_get_path_int(ctx, 2);
    if (club_id <= 0) { api_error(ctx, ERR_INPUT, "社团ID非法"); return; }

    MYSQL_RES *res = db_query(
        "SELECT cl.club_id, cl.club_name, cl.description, cl.category, cl.level, "
        "cl.status, cl.advisor, cl.member_count, cl.join_permission, "
        "COALESCE(col.college_name,'-') AS college_name, "
        "COALESCE(u.real_name,'-') AS president "
        "FROM clubs cl "
        "LEFT JOIN colleges col ON cl.college_id=col.college_id "
        "LEFT JOIN users u ON cl.president_id=u.user_id "
        "WHERE cl.club_id=%d", club_id);
    if (!res || mysql_num_rows(res) == 0) {
        if (res) mysql_free_result(res);
        api_error(ctx, ERR_NOT_FOUND, "社团不存在");
        return;
    }
    /* 直接把这一行返回即可；成员/活动前端可再拉 members/activities 接口 */
    api_send_result_data(ctx, res);
}

/* POST /api/school/clubs/{id}/level  body: level=school|college, college_id */
void sch_club_set_level(ApiContext *ctx) {
    if (!api_require_school_admin(ctx)) return;
    int club_id = api_get_path_int(ctx, 2);
    if (club_id <= 0) { api_error(ctx, ERR_INPUT, "社团ID非法"); return; }

    char level[16] = "";
    if (!api_get_json_str(ctx, "level", level, sizeof(level)))
        api_get_param(ctx, "level", level, sizeof(level));
    int college_id = api_get_json_int(ctx, "college_id", 0);
    if (college_id == 0) college_id = api_get_int(ctx, "college_id", 0);

    int ok;
    if (strcmp(level, "school") == 0) {
        ok = db_execute("UPDATE clubs SET level='school', college_id=NULL WHERE club_id=%d", club_id);
    } else if (strcmp(level, "college") == 0) {
        if (college_id <= 0) { api_error(ctx, ERR_INPUT, "院级须指定学院"); return; }
        ok = db_execute("UPDATE clubs SET level='college', college_id=%d WHERE club_id=%d",
                        college_id, club_id);
    } else {
        api_error(ctx, ERR_INPUT, "级别取值非法"); return;
    }
    if (ok < 0) { api_error(ctx, ERR_DB, "设置失败"); return; }

    db_execute("INSERT INTO logs (user_id, action, target_type, target_id, detail) "
               "VALUES (%d, 'set_club_level', 'clubs', %d, '设定社团级别')",
               ctx->user->user_id, club_id);
    api_ok_msg(ctx, "级别已更新");
}

/* POST /api/school/clubs/{id}/dissolve  解散社团（通知全体成员） */
void sch_club_dissolve(ApiContext *ctx) {
    if (!api_require_school_admin(ctx)) return;
    int club_id = api_get_path_int(ctx, 2);
    if (club_id <= 0) { api_error(ctx, ERR_INPUT, "社团ID非法"); return; }

    int ok = db_execute("UPDATE clubs SET status='dissolved' WHERE club_id=%d", club_id);
    if (ok <= 0) { api_error(ctx, ERR_NOT_FOUND, "社团不存在"); return; }

    /* 通知全体在籍成员 */
    notification_broadcast_club(club_id, "社团已解散",
        "很遗憾，该社团已被学校管理员解散。", "club_dissolved", club_id);

    db_execute("INSERT INTO logs (user_id, action, target_type, target_id, detail) "
               "VALUES (%d, 'dissolve_club', 'clubs', %d, '解散社团')",
               ctx->user->user_id, club_id);
    api_ok_msg(ctx, "社团已解散");
}

/* POST /api/school/clubs/{id}/rename  body: name */
void sch_club_rename(ApiContext *ctx) {
    if (!api_require_school_admin(ctx)) return;
    int club_id = api_get_path_int(ctx, 2);
    if (club_id <= 0) { api_error(ctx, ERR_INPUT, "社团ID非法"); return; }

    char name[128] = "";
    if (!api_get_json_str(ctx, "name", name, sizeof(name)))
        api_get_param(ctx, "name", name, sizeof(name));
    if (utils_is_empty(name)) { api_error(ctx, ERR_INPUT, "请输入新名称"); return; }

    char *en = db_escape(name);
    int ok = db_execute("UPDATE clubs SET club_name='%s' WHERE club_id=%d", en, club_id);
    free(en);
    if (ok <= 0) { api_error(ctx, ERR_NOT_FOUND, "社团不存在"); return; }

    db_execute("INSERT INTO logs (user_id, action, target_type, target_id, detail) "
               "VALUES (%d, 'rename_club', 'clubs', %d, '重命名社团')",
               ctx->user->user_id, club_id);
    api_ok_msg(ctx, "社团已重命名");
}

#endif /* 备用代码结束 */
