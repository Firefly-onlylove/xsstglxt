/**
 * club_browse.c — 社团广场：浏览 / 搜索 / 筛选 / 详情
 * 学生社团管理系统 SCMS（学生端）
 *
 * 覆盖处理函数：
 *   stu_club_browse      GET /api/clubs                浏览+搜索+分类/级别筛选+分页
 *   stu_club_detail      GET /api/clubs/{id}           社团详情
 *   stu_club_members     GET /api/clubs/{id}/members   成员列表
 *   stu_club_activities  GET /api/clubs/{id}/activities 该社团活动列表
 *
 * 启用方法：把下面的 #if 0 改成 #if 1。
 */
#if 0

#include "../../include/api.h"
#include "../../include/db.h"
#include "../../include/common.h"
#include "../../include/utils.h"
#include "../../include/routes.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* GET /api/clubs?keyword=&category=&level=&page=
 * 只展示已审批（approved）社团。keyword 模糊匹配名称，category/level 精确筛选。 */
void stu_club_browse(ApiContext *ctx) {
    char keyword[64] = "", category[32] = "", level[16] = "";
    api_get_param(ctx, "keyword",  keyword,  sizeof(keyword));
    api_get_param(ctx, "category", category, sizeof(category));
    api_get_param(ctx, "level",    level,    sizeof(level));
    int page = api_get_int(ctx, "page", 1);
    if (page < 1) page = 1;
    int page_size = PAGE_SIZE_DEFAULT;
    int offset = (page - 1) * page_size;

    /* 动态拼 WHERE 条件。用户输入先转义。 */
    char where[512];
    int wl = snprintf(where, sizeof(where), "WHERE c.status='approved'");

    if (!utils_is_empty(keyword)) {
        char *e = db_escape(keyword);
        wl += snprintf(where + wl, sizeof(where) - wl,
                       " AND c.club_name LIKE '%%%s%%'", e);
        free(e);
    }
    if (!utils_is_empty(category)) {
        char *e = db_escape(category);
        wl += snprintf(where + wl, sizeof(where) - wl,
                       " AND c.category='%s'", e);
        free(e);
    }
    if (utils_str_equal(level, "school") || utils_str_equal(level, "college")) {
        wl += snprintf(where + wl, sizeof(where) - wl, " AND c.level='%s'", level);
    }

    /* 总数（用于分页） */
    int total = db_query_int("SELECT COUNT(*) FROM clubs c %s", where);

    MYSQL_RES *res = db_query(
        "SELECT c.club_id, c.club_name, c.category, c.level, c.description, "
        "c.member_count, c.logo_path, COALESCE(col.college_name,'') AS college_name, "
        "COALESCE(u.real_name,'') AS president_name "
        "FROM clubs c "
        "LEFT JOIN colleges col ON c.college_id=col.college_id "
        "LEFT JOIN users u ON c.president_id=u.user_id "
        "%s ORDER BY c.member_count DESC, c.club_id "
        "LIMIT %d OFFSET %d",
        where, page_size, offset);

    api_send_result_paged(ctx, res, page, page_size, total);
}

/* GET /api/clubs/{id} — 社团详情（段序号1为 id） */
void stu_club_detail(ApiContext *ctx) {
    int club_id = api_get_path_int(ctx, 1);
    if (club_id <= 0) { api_error(ctx, ERR_INPUT, "社团ID非法"); return; }

    MYSQL_RES *res = db_query(
        "SELECT c.club_id, c.club_name, c.description, c.category, c.level, "
        "c.logo_path, c.advisor, c.member_count, c.join_permission, c.status, "
        "COALESCE(col.college_name,'') AS college_name, "
        "COALESCE(u.real_name,'') AS president_name "
        "FROM clubs c "
        "LEFT JOIN colleges col ON c.college_id=col.college_id "
        "LEFT JOIN users u ON c.president_id=u.user_id "
        "WHERE c.club_id=%d", club_id);

    if (!res) { api_error(ctx, ERR_DB, "查询失败"); return; }
    if (mysql_num_rows(res) == 0) {
        mysql_free_result(res);
        api_error(ctx, ERR_NOT_FOUND, "社团不存在");
        return;
    }
    /* 单条对象：直接把结果集第一行转 JSON（复用数组接口，前端取 [0]） */
    api_send_result_data(ctx, res);
}

/* GET /api/clubs/{id}/members — 在籍成员列表 */
void stu_club_members(ApiContext *ctx) {
    int club_id = api_get_path_int(ctx, 1);
    if (club_id <= 0) { api_error(ctx, ERR_INPUT, "社团ID非法"); return; }

    MYSQL_RES *res = db_query(
        "SELECT m.member_id, u.real_name, m.role, m.joined_at "
        "FROM members m JOIN users u ON m.user_id=u.user_id "
        "WHERE m.club_id=%d AND m.join_status='approved' AND m.left_at IS NULL "
        "ORDER BY FIELD(m.role,'president','vice_president','secretary','treasurer','member'), "
        "m.joined_at", club_id);

    api_send_result(ctx, res);
}

/* GET /api/clubs/{id}/activities — 该社团已发布及以后的活动 */
void stu_club_activities(ApiContext *ctx) {
    int club_id = api_get_path_int(ctx, 1);
    if (club_id <= 0) { api_error(ctx, ERR_INPUT, "社团ID非法"); return; }

    MYSQL_RES *res = db_query(
        "SELECT activity_id, title, location, start_time, end_time, "
        "max_participants, current_count, status "
        "FROM activities "
        "WHERE club_id=%d AND status IN ('published','ongoing','completed') "
        "ORDER BY start_time DESC", club_id);

    api_send_result(ctx, res);
}

#endif /* 备用文件：改 #if 0 为 #if 1 启用 */
