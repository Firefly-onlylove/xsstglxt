/**
 * election.c — 社团管理端：换届选举（发起 / 报名参选 / 连任 / 候选人）
 * 学生社团管理系统 SCMS（社团管理端 club_admin）
 *
 * 覆盖处理函数（函数名/路由严格对齐 routes.h）：
 *   club_election_list        GET  /api/club/{id}/elections
 *   club_election_create      POST /api/club/{id}/elections            社长发起换届
 *   club_election_detail      GET  /api/club/{id}/elections/{eid}
 *   club_election_reelect     POST /api/club/{id}/elections/reelect    社长/副社长申请连任
 *   club_election_signup      POST /api/club/{id}/elections/{eid}/signup 成员报名参选
 *   club_election_candidates  GET  /api/club/{id}/elections/{eid}/candidates
 *
 * 投票与结果在 election_vote.c。
 *
 * 启用方法：把下面的 #if 0 改成 #if 1。
 *
 * 规则：任期一学年，到期自动触发换届。发起时设报名截止/投票截止时间。
 *      任何在籍成员均可报名参选社长。副社长人数由社长设定。
 *      报名为0时保持原样并向管理端警示（由学校端处理解散/重投/保持）。
 */
#if 1

#include "../../../include/api.h"
#include "../../../include/db.h"
#include "../../../include/common.h"
#include "../../../include/utils.h"
#include "../../../include/routes.h"
#include "../../../include/notification.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 校验当前用户是否该社团在籍成员（不含学校管理员特权，纯身份判断） */
static int is_valid_member(int uid, int club_id) {
    return db_query_int(
        "SELECT COUNT(*) FROM members WHERE club_id=%d AND user_id=%d "
        "AND join_status='approved' AND left_at IS NULL", club_id, uid) > 0;
}

/* GET /api/club/{id}/elections — 换届列表 */
void club_election_list(ApiContext *ctx) {
    int club_id = api_get_path_int(ctx, 1);
    if (club_id <= 0) { api_error(ctx, ERR_INPUT, "社团ID非法"); return; }
    if (!api_require_login(ctx)) return;
    if (!is_valid_member(ctx->user->user_id, club_id) &&
        !utils_str_equal(ctx->user->role, "school_admin")) {
        api_error(ctx, ERR_PERMISSION, "仅本社成员可查看"); return;
    }

    MYSQL_RES *res = db_query(
        "SELECT election_id, title, vice_president_count, signup_end, vote_end, "
        "status, created_at FROM elections WHERE club_id=%d ORDER BY created_at DESC",
        club_id);
    api_send_result(ctx, res);
}

/* POST /api/club/{id}/elections — 发起换届
 * body: title, vice_president_count(副社长名额), signup_deadline, vote_deadline */
void club_election_create(ApiContext *ctx) {
    int club_id = api_get_path_int(ctx, 1);
    if (club_id <= 0) { api_error(ctx, ERR_INPUT, "社团ID非法"); return; }
    if (!api_require_club_president(ctx, club_id)) return;

    /* 不允许存在进行中的换届 */
    int ongoing = db_query_int(
        "SELECT COUNT(*) FROM elections WHERE club_id=%d "
        "AND status IN ('signup','voting')", club_id);
    if (ongoing > 0) { api_error(ctx, ERR_DUPLICATE, "已有进行中的换届"); return; }

    char title[128] = "", signup_deadline[32] = "", vote_deadline[32] = "";
    api_get_json_str(ctx, "title",            title,            sizeof(title));
    api_get_json_str(ctx, "signup_deadline",  signup_deadline,  sizeof(signup_deadline));
    api_get_json_str(ctx, "vote_deadline",    vote_deadline,    sizeof(vote_deadline));
    int vice_count = api_get_json_int(ctx, "vice_president_count", 1);

    if (utils_is_empty(title)) utils_strlcpy(title, "社团换届选举", sizeof(title));
    if (utils_is_empty(signup_deadline) || utils_is_empty(vote_deadline)) {
        api_error(ctx, ERR_VALIDATION, "请设置报名截止与投票截止时间"); return;
    }
    if (vice_count < 0) vice_count = 0;

    char *e_title = db_escape(title);
    int rc = db_execute(
        "INSERT INTO elections "
        "(club_id, title, vice_president_count, signup_end, vote_end, status, created_at) "
        "VALUES (%d, '%s', %d, '%s', '%s', 'signup', NOW())",
        club_id, e_title, vice_count, signup_deadline, vote_deadline);
    free(e_title);
    if (rc < 0) { api_error(ctx, ERR_DB, "发起失败"); return; }

    /* 通知全体成员参与 */
    notification_broadcast_club(club_id, "社团换届开始",
        "本社团已发起换届选举，欢迎报名参选或投票。", "election", club_id);
    api_ok_msg(ctx, "换届已发起");
}

/* GET /api/club/{id}/elections/{eid} — 换届详情 */
void club_election_detail(ApiContext *ctx) {
    int club_id = api_get_path_int(ctx, 1);
    int eid     = api_get_path_int(ctx, 3);
    if (club_id <= 0 || eid <= 0) { api_error(ctx, ERR_INPUT, "参数非法"); return; }
    if (!api_require_login(ctx)) return;

    MYSQL_RES *res = db_query(
        "SELECT election_id, title, vice_president_count, signup_end, vote_end, "
        "status, created_at FROM elections WHERE election_id=%d AND club_id=%d",
        eid, club_id);
    if (!res || mysql_num_rows(res) == 0) {
        if (res) mysql_free_result(res);
        api_error(ctx, ERR_NOT_FOUND, "换届不存在");
        return;
    }
    api_send_result_data(ctx, res);
}

/* POST /api/club/{id}/elections/reelect — 现任社长/副社长申请连任
 * 若不申请连任，任期结束自动降为普通成员。此接口把当前用户登记为连任候选。 */
void club_election_reelect(ApiContext *ctx) {
    int club_id = api_get_path_int(ctx, 1);
    if (club_id <= 0) { api_error(ctx, ERR_INPUT, "社团ID非法"); return; }
    if (!api_require_login(ctx)) return;
    int uid = ctx->user->user_id;

    /* 必须是现任社长/副社长 */
    char role[16] = "";
    db_query_str("SELECT role FROM members WHERE club_id=%d AND user_id=%d "
                 "AND join_status='approved' AND left_at IS NULL",
                 role, sizeof(role), club_id, uid);
    if (!utils_str_equal(role, "president") && !utils_str_equal(role, "vice_president")) {
        api_error(ctx, ERR_PERMISSION, "仅现任社长/副社长可申请连任"); return;
    }

    /* 找到进行中(signup)的换届，登记为候选人（标记 is_reelect=1） */
    int eid = db_query_int(
        "SELECT election_id FROM elections WHERE club_id=%d AND status='signup'", club_id);
    if (eid <= 0) { api_error(ctx, ERR_STATUS, "当前无可报名的换届"); return; }

    int dup = db_query_int(
        "SELECT COUNT(*) FROM election_candidates WHERE election_id=%d AND user_id=%d",
        eid, uid);
    if (dup > 0) { api_error(ctx, ERR_DUPLICATE, "您已登记参选"); return; }

    db_execute("INSERT INTO election_candidates "
               "(election_id, user_id, is_reelect, votes, created_at) "
               "VALUES (%d, %d, 1, 0, NOW())", eid, uid);
    api_ok_msg(ctx, "已提交连任申请");
}

/* POST /api/club/{id}/elections/{eid}/signup — 成员报名参选 */
void club_election_signup(ApiContext *ctx) {
    int club_id = api_get_path_int(ctx, 1);
    int eid     = api_get_path_int(ctx, 3);
    if (club_id <= 0 || eid <= 0) { api_error(ctx, ERR_INPUT, "参数非法"); return; }
    if (!api_require_login(ctx)) return;
    int uid = ctx->user->user_id;

    if (!is_valid_member(uid, club_id)) {
        api_error(ctx, ERR_PERMISSION, "仅本社成员可参选"); return;
    }

    /* 换届须处于报名期且未过报名截止 */
    MYSQL_RES *res = db_query(
        "SELECT status FROM elections WHERE election_id=%d AND club_id=%d "
        "AND signup_end > NOW()", eid, club_id);
    int ok = (res && mysql_num_rows(res) > 0);
    char status[16] = "";
    if (ok) { MYSQL_ROW r = mysql_fetch_row(res); utils_strlcpy(status, r[0], sizeof(status)); }
    if (res) mysql_free_result(res);
    if (!ok || !utils_str_equal(status, "signup")) {
        api_error(ctx, ERR_STATUS, "报名已截止或换届状态不符"); return;
    }

    int dup = db_query_int(
        "SELECT COUNT(*) FROM election_candidates WHERE election_id=%d AND user_id=%d",
        eid, uid);
    if (dup > 0) { api_error(ctx, ERR_DUPLICATE, "您已报名参选"); return; }

    db_execute("INSERT INTO election_candidates "
               "(election_id, user_id, is_reelect, votes, created_at) "
               "VALUES (%d, %d, 0, 0, NOW())", eid, uid);
    api_ok_msg(ctx, "报名成功");
}

/* GET /api/club/{id}/elections/{eid}/candidates — 候选人列表 */
void club_election_candidates(ApiContext *ctx) {
    int club_id = api_get_path_int(ctx, 1);
    int eid     = api_get_path_int(ctx, 3);
    if (club_id <= 0 || eid <= 0) { api_error(ctx, ERR_INPUT, "参数非法"); return; }
    if (!api_require_login(ctx)) return;

    /* 投票期内不返回票数，避免影响投票；结束后才显示 */
    char status[16] = "";
    db_query_str("SELECT status FROM elections WHERE election_id=%d", status, sizeof(status), eid);
    int show_votes = utils_str_equal(status, "completed");

    MYSQL_RES *res;
    if (show_votes)
        res = db_query(
            "SELECT c.candidate_id, c.user_id, u.real_name, c.is_reelect, c.votes "
            "FROM election_candidates c JOIN users u ON c.user_id=u.user_id "
            "WHERE c.election_id=%d ORDER BY c.votes DESC", eid);
    else
        res = db_query(
            "SELECT c.candidate_id, c.user_id, u.real_name, c.is_reelect, 0 AS votes "
            "FROM election_candidates c JOIN users u ON c.user_id=u.user_id "
            "WHERE c.election_id=%d ORDER BY c.created_at", eid);
    api_send_result(ctx, res);
}

#endif /* 备用文件：改 #if 0 为 #if 1 启用 */
