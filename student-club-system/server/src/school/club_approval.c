/**
 * club_approval.c — 学校端 社团审批
 * 学生社团管理系统 SCMS（Web 架构 · 学校端）
 *
 * ┌──────────────────────────────────────────────────────────────────┐
 * │  ★ 备用参考实现。默认整份被 #if 0 关闭，不参与编译（走弱符号占位）。 │
 * │  ★ 启用方法：把下面这行的  #if 0  改成  #if 1  ，重新 make 即可。   │
 * └──────────────────────────────────────────────────────────────────┘
 *
 * 覆盖路由：
 *   GET  /api/school/clubs/pending         → sch_club_pending          待审批列表
 *   GET  /api/school/clubs/history         → sch_club_approved_history 已审批记录
 *   POST /api/school/clubs/{id}/approve    → sch_club_approve          通过（设级别）
 *   POST /api/school/clubs/{id}/reject     → sch_club_reject           驳回（填原因）
 *
 * 说明：审批通过时，申请人自动成为社长（写入 members，role=president）。
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

/* GET /api/school/clubs/pending  待审批社团列表 */
void sch_club_pending(ApiContext *ctx) {
    if (!api_require_school_admin(ctx)) return;

    char keyword[64] = "", club_type[32] = "";
    api_get_param(ctx, "keyword", keyword, sizeof(keyword));
    api_get_param(ctx, "club_type", club_type, sizeof(club_type));
    int page = api_get_int(ctx, "page", 1);
    int page_size = api_get_int(ctx, "page_size", 10);

    /* 构建 WHERE 条件 */
    char where[256] = "WHERE cl.status='pending'";
    if (!utils_is_empty(keyword)) {
        char *ek = db_escape(keyword);
        char buf[128];
        snprintf(buf, sizeof(buf), " AND cl.club_name LIKE '%%%s%%'", ek);
        strcat(where, buf);
        free(ek);
    }
    if (!utils_is_empty(club_type)) {
        char *et = db_escape(club_type);
        char buf[128];
        snprintf(buf, sizeof(buf), " AND cl.category='%s'", et);
        strcat(where, buf);
        free(et);
    }

    /* 总数 */
    char count_sql[512];
    snprintf(count_sql, sizeof(count_sql),
        "SELECT COUNT(*) FROM clubs cl %s", where);
    int total = db_query_int("%s", count_sql);

    /* 分页数据 */
    int offset = (page - 1) * page_size;
    if (offset < 0) offset = 0;

    char sql[1024];
    snprintf(sql, sizeof(sql),
        "SELECT cl.club_id, cl.club_name, cl.category, cl.level, "
        "COALESCE(col.college_name,'-') AS college_name, "
        "u.real_name AS creator, cl.description, cl.created_at, cl.status "
        "FROM clubs cl "
        "LEFT JOIN colleges col ON cl.college_id=col.college_id "
        "LEFT JOIN users u ON cl.created_by=u.user_id "
        "%s ORDER BY cl.created_at LIMIT %d OFFSET %d",
        where, page_size, offset);

    MYSQL_RES *res = db_query("%s", sql);
    api_send_result_paged(ctx, res, page, page_size, total);
}

/* GET /api/school/clubs/history  已审批（通过/驳回）记录 */
void sch_club_approved_history(ApiContext *ctx) {
    if (!api_require_school_admin(ctx)) return;

    char keyword[64] = "", club_type[32] = "";
    api_get_param(ctx, "keyword", keyword, sizeof(keyword));
    api_get_param(ctx, "club_type", club_type, sizeof(club_type));
    int page = api_get_int(ctx, "page", 1);
    int page_size = api_get_int(ctx, "page_size", 10);

    char where[256] = "WHERE cl.status IN ('approved','rejected')";
    if (!utils_is_empty(keyword)) {
        char *ek = db_escape(keyword);
        char buf[128];
        snprintf(buf, sizeof(buf), " AND cl.club_name LIKE '%%%s%%'", ek);
        strcat(where, buf);
        free(ek);
    }
    if (!utils_is_empty(club_type)) {
        char *et = db_escape(club_type);
        char buf[128];
        snprintf(buf, sizeof(buf), " AND cl.category='%s'", et);
        strcat(where, buf);
        free(et);
    }

    int total = db_query_int("SELECT COUNT(*) FROM clubs cl %s", where);
    int offset = (page - 1) * page_size;
    if (offset < 0) offset = 0;

    char sql[1024];
    snprintf(sql, sizeof(sql),
        "SELECT cl.club_id, cl.club_name, cl.category, cl.level, cl.status, "
        "cl.reject_reason, cl.description, cl.created_at, "
        "COALESCE(col.college_name,'-') AS college_name, "
        "u.real_name AS creator, cl.updated_at "
        "FROM clubs cl "
        "LEFT JOIN colleges col ON cl.college_id=col.college_id "
        "LEFT JOIN users u ON cl.created_by=u.user_id "
        "%s ORDER BY cl.updated_at DESC LIMIT %d OFFSET %d",
        where, page_size, offset);

    MYSQL_RES *res = db_query("%s", sql);
    api_send_result_paged(ctx, res, page, page_size, total);
}

/* POST /api/school/clubs/{id}/approve
 * body: level=school|college, college_id（level=college 时必填）
 * 通过后：设级别、创建者升为社长、发通知。 */
void sch_club_approve(ApiContext *ctx) {
    if (!api_require_school_admin(ctx)) return;
    int club_id = api_get_path_int(ctx, 2);   /* /api/school/clubs/{id}/approve */
    if (club_id <= 0) { api_error(ctx, ERR_INPUT, "社团ID非法"); return; }

    char level[16] = "college";
    if (!api_get_json_str(ctx, "level", level, sizeof(level)))
        api_get_param(ctx, "level", level, sizeof(level));
    int college_id = api_get_json_int(ctx, "college_id", 0);
    if (college_id == 0) college_id = api_get_int(ctx, "college_id", 0);

    /* 校验社团存在且待审批 */
    MYSQL_RES *r = db_query(
        "SELECT created_by FROM clubs WHERE club_id=%d AND status='pending'", club_id);
    if (!r || mysql_num_rows(r) == 0) {
        if (r) mysql_free_result(r);
        api_error(ctx, ERR_NOT_FOUND, "社团不存在或已处理");
        return;
    }
    MYSQL_ROW row = mysql_fetch_row(r);
    int creator_id = row[0] ? atoi(row[0]) : 0;
    mysql_free_result(r);

    /* 校级社团 college_id 置空；院级必须带 college_id */
    int is_school = (strcmp(level, "school") == 0);
    if (!is_school && college_id <= 0) {
        api_error(ctx, ERR_INPUT, "院级社团必须指定所属学院");
        return;
    }

    db_begin();
    int ok;
    if (is_school) {
        ok = db_execute(
            "UPDATE clubs SET status='approved', level='school', college_id=NULL, "
            "president_id=%d WHERE club_id=%d", creator_id, club_id);
    } else {
        ok = db_execute(
            "UPDATE clubs SET status='approved', level='college', college_id=%d, "
            "president_id=%d WHERE club_id=%d", college_id, creator_id, club_id);
    }
    if (ok < 0) { db_rollback(); api_error(ctx, ERR_DB, "审批失败"); return; }

    /* 创建者成为社长（若已有成员记录则升级，否则插入） */
    if (creator_id > 0) {
        int exists = db_query_int(
            "SELECT COUNT(*) FROM members WHERE club_id=%d AND user_id=%d",
            club_id, creator_id);
        if (exists > 0) {
            db_execute("UPDATE members SET role='president', join_status='approved', "
                       "left_at=NULL WHERE club_id=%d AND user_id=%d", club_id, creator_id);
        } else {
            db_execute("INSERT INTO members (club_id, user_id, role, join_status) "
                       "VALUES (%d, %d, 'president', 'approved')", club_id, creator_id);
        }
        /* 创建者角色升为 club_admin */
        db_execute("UPDATE users SET role='club_admin' WHERE user_id=%d "
                   "AND role IN ('general_student','club_member')", creator_id);
    }

    db_execute("INSERT INTO logs (user_id, action, target_type, target_id, detail) "
               "VALUES (%d, 'approve_club', 'clubs', %d, '审批通过社团')",
               ctx->user->user_id, club_id);
    db_commit();

    if (creator_id > 0)
        notification_send(creator_id, "社团创建申请已通过",
                          "恭喜！您申请创建的社团已通过学校审批，您已成为社长。",
                          "club_apply_result", club_id);
    api_ok_msg(ctx, "已通过");
}

/* POST /api/school/clubs/{id}/reject  body: reason */
void sch_club_reject(ApiContext *ctx) {
    if (!api_require_school_admin(ctx)) return;
    int club_id = api_get_path_int(ctx, 2);
    if (club_id <= 0) { api_error(ctx, ERR_INPUT, "社团ID非法"); return; }

    char reason[256] = "";
    if (!api_get_json_str(ctx, "reason", reason, sizeof(reason)))
        api_get_param(ctx, "reason", reason, sizeof(reason));
    if (utils_is_empty(reason)) { api_error(ctx, ERR_INPUT, "请填写驳回原因"); return; }

    int creator_id = db_query_int(
        "SELECT created_by FROM clubs WHERE club_id=%d AND status='pending'", club_id);

    char *e = db_escape(reason);
    int ok = db_execute(
        "UPDATE clubs SET status='rejected', reject_reason='%s' "
        "WHERE club_id=%d AND status='pending'", e, club_id);
    free(e);
    if (ok <= 0) { api_error(ctx, ERR_NOT_FOUND, "社团不存在或已处理"); return; }

    db_execute("INSERT INTO logs (user_id, action, target_type, target_id, detail) "
               "VALUES (%d, 'reject_club', 'clubs', %d, '驳回社团申请')",
               ctx->user->user_id, club_id);
    if (creator_id > 0)
        notification_send(creator_id, "社团创建申请被驳回", reason,
                          "club_apply_result", club_id);
    api_ok_msg(ctx, "已驳回");
}

#endif /* 备用代码结束 */
