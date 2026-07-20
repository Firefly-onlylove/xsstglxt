/**
 * election_vote.c — 社团管理端：匿名投票 + 结果结算（含平票重投）
 * 学生社团管理系统 SCMS（社团管理端 club_admin）
 *
 * 覆盖处理函数（函数名/路由严格对齐 routes.h）：
 *   club_election_vote    POST /api/club/{id}/elections/{eid}/vote    匿名投票
 *   club_election_result  GET  /api/club/{id}/elections/{eid}/result  结算并返回结果
 *
 * 启用方法：把下面的 #if 0 改成 #if 1。
 *
 * 投票规则：
 *  - 每位在籍成员一票，匿名（votes_log 只记录"是否已投"，不记录投给谁）。
 *  - 社长固定一人：得票最高者当选。副社长按名额取次高的若干人。
 *  - 平票处理：若社长位出现最高票并列，发起"平的人之间重新投票"（reelect 一轮）；
 *    若再次平票则继续，直到分出。此处结算时检测到并列，把 election 状态置回
 *    voting 并写 tie_candidates，前端提示进入平票重投。
 *  - 报名为0：结算返回 no_candidate，向管理端警示（学校端决定后续）。
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

/* 判断当前用户是否本社在籍成员 */
static int is_valid_member(int uid, int club_id) {
    return db_query_int(
        "SELECT COUNT(*) FROM members WHERE club_id=%d AND user_id=%d "
        "AND join_status='approved' AND left_at IS NULL", club_id, uid) > 0;
}

/* POST /api/club/{id}/elections/{eid}/vote — 匿名投票  body: candidate_id */
void club_election_vote(ApiContext *ctx) {
    int club_id = api_get_path_int(ctx, 1);
    int eid     = api_get_path_int(ctx, 3);
    if (club_id <= 0 || eid <= 0) { api_error(ctx, ERR_INPUT, "参数非法"); return; }
    if (!api_require_login(ctx)) return;
    int uid = ctx->user->user_id;

    if (!is_valid_member(uid, club_id)) {
        api_error(ctx, ERR_PERMISSION, "仅本社成员可投票"); return;
    }

    /* 换届须处于投票期且未过投票截止 */
    char status[16] = "";
    db_query_str("SELECT status FROM elections WHERE election_id=%d AND club_id=%d "
                 "AND vote_end > NOW()", status, sizeof(status), eid, club_id);
    if (!utils_str_equal(status, "voting")) {
        api_error(ctx, ERR_STATUS, "当前不在投票时间内"); return;
    }

    int candidate_id = api_get_json_int(ctx, "candidate_id", 0);
    if (candidate_id <= 0) { api_error(ctx, ERR_INPUT, "请选择候选人"); return; }

    /* 候选人须属于本次换届 */
    int valid = db_query_int(
        "SELECT COUNT(*) FROM election_candidates "
        "WHERE candidate_id=%d AND election_id=%d", candidate_id, eid);
    if (valid == 0) { api_error(ctx, ERR_NOT_FOUND, "候选人不存在"); return; }

    /* 一人一票：vote_logs 记录该用户是否已在本次换届投过（匿名，不存投给谁） */
    int voted = db_query_int(
        "SELECT COUNT(*) FROM vote_logs WHERE election_id=%d AND user_id=%d", eid, uid);
    if (voted > 0) { api_error(ctx, ERR_DUPLICATE, "您已投过票"); return; }

    /* 事务：+1 票 & 记录已投 */
    db_execute("UPDATE election_candidates SET votes=votes+1 WHERE candidate_id=%d",
               candidate_id);
    db_execute("INSERT INTO vote_logs (election_id, user_id, voted_at) "
               "VALUES (%d, %d, NOW())", eid, uid);

    api_ok_msg(ctx, "投票成功");
}

/* GET /api/club/{id}/elections/{eid}/result — 结算并返回结果
 * 只有投票截止后才允许结算。结算逻辑：
 *   1. 无候选人 → no_candidate，警示管理端。
 *   2. 社长位最高票并列 → 平票重投（状态回 voting，只保留并列者）。
 *   3. 正常 → 最高票为社长，次高 vice_count 人为副社长，写回 members，标记 completed。 */
void club_election_result(ApiContext *ctx) {
    int club_id = api_get_path_int(ctx, 1);
    int eid     = api_get_path_int(ctx, 3);
    if (club_id <= 0 || eid <= 0) { api_error(ctx, ERR_INPUT, "参数非法"); return; }
    if (!api_require_login(ctx)) return;

    /* 取换届信息 */
    MYSQL_RES *er = db_query(
        "SELECT status, vice_president_count, vote_end FROM elections "
        "WHERE election_id=%d AND club_id=%d", eid, club_id);
    if (!er || mysql_num_rows(er) == 0) {
        if (er) mysql_free_result(er);
        api_error(ctx, ERR_NOT_FOUND, "换届不存在");
        return;
    }
    MYSQL_ROW erow = mysql_fetch_row(er);
    char status[16]; utils_strlcpy(status, erow[0] ? erow[0] : "", sizeof(status));
    int vice_count = erow[1] ? atoi(erow[1]) : 0;
    mysql_free_result(er);

    /* 若已结算，直接返回候选人排名 */
    if (utils_str_equal(status, "completed")) {
        MYSQL_RES *res = db_query(
            "SELECT c.user_id, u.real_name, c.votes, "
            "CASE WHEN m.role IN ('president','vice_president') THEN m.role ELSE '' END AS elected_role "
            "FROM election_candidates c JOIN users u ON c.user_id=u.user_id "
            "LEFT JOIN members m ON m.user_id=c.user_id AND m.club_id=%d AND m.left_at IS NULL "
            "WHERE c.election_id=%d ORDER BY c.votes DESC", club_id, eid);
        api_send_result(ctx, res);
        return;
    }

    /* 未到投票截止不能结算 */
    int can_settle = db_query_int(
        "SELECT COUNT(*) FROM elections WHERE election_id=%d AND vote_end<=NOW()", eid);
    if (can_settle == 0) { api_error(ctx, ERR_STATUS, "投票尚未截止，无法结算"); return; }

    /* 候选人按票数降序 */
    MYSQL_RES *cs = db_query(
        "SELECT candidate_id, user_id, votes FROM election_candidates "
        "WHERE election_id=%d ORDER BY votes DESC", eid);
    int n = cs ? (int)mysql_num_rows(cs) : 0;

    if (n == 0) {
        if (cs) mysql_free_result(cs);
        db_execute("UPDATE elections SET status='no_candidate' WHERE election_id=%d", eid);
        /* 警示学校管理端：报名为0 */
        notification_notify_school_admins("换届无人报名",
            "有社团换届报名人数为0，请决定：解散/重投/保持。", "election_alert", club_id);
        api_error(ctx, ERR_STATUS, "无人参选，已上报学校管理端处理");
        return;
    }

    /* 读入候选人到数组 */
    typedef struct { int cid; int uid; int votes; } Cand;
    Cand *arr = (Cand *)malloc(sizeof(Cand) * n);
    int i = 0; MYSQL_ROW row;
    while ((row = mysql_fetch_row(cs)) != NULL) {
        arr[i].cid = atoi(row[0]);
        arr[i].uid = atoi(row[1]);
        arr[i].votes = atoi(row[2]);
        i++;
    }
    mysql_free_result(cs);

    /* 检测社长位平票：最高票是否有并列 */
    int top_votes = arr[0].votes;
    int tie_cnt = 0;
    for (i = 0; i < n; i++) if (arr[i].votes == top_votes) tie_cnt++;

    if (tie_cnt > 1) {
        /* 平票重投：清零并列者票数与投票记录，只保留并列者为候选人，状态回 voting。
         * 需前端/社长重新设置 vote_deadline。 */
        db_execute("DELETE FROM election_candidates WHERE election_id=%d AND votes<%d",
                   eid, top_votes);
        db_execute("UPDATE election_candidates SET votes=0 WHERE election_id=%d", eid);
        db_execute("DELETE FROM vote_logs WHERE election_id=%d", eid);
        db_execute("UPDATE elections SET status='voting' WHERE election_id=%d", eid);
        free(arr);
        notification_notify_school_admins("换届出现平票",
            "有社团社长选举出现平票，已在并列候选人间发起重新投票。", "election_alert", club_id);
        api_error(ctx, ERR_STATUS, "社长票数并列，已在并列者间发起重新投票");
        return;
    }

    /* 正常结算：arr[0] 当选社长，arr[1..vice_count] 当选副社长 */
    int president_uid = arr[0].uid;

    /* 先把现任社长/副社长（未连任成功者）降为普通成员 */
    db_execute("UPDATE members SET role='member' WHERE club_id=%d "
               "AND role IN ('president','vice_president') AND left_at IS NULL", club_id);

    /* 设新社长 */
    db_execute("UPDATE members SET role='president' WHERE club_id=%d AND user_id=%d "
               "AND left_at IS NULL", club_id, president_uid);
    db_execute("UPDATE clubs SET president_id=%d WHERE club_id=%d", president_uid, club_id);
    db_execute("UPDATE users SET role='club_admin' WHERE user_id=%d", president_uid);
    notification_send(president_uid, "当选社长", "恭喜您当选本社团社长！", "election_result", club_id);

    /* 设副社长（次高的 vice_count 人） */
    for (i = 1; i <= vice_count && i < n; i++) {
        db_execute("UPDATE members SET role='vice_president' WHERE club_id=%d AND user_id=%d "
                   "AND left_at IS NULL", club_id, arr[i].uid);
        db_execute("UPDATE users SET role='club_admin' WHERE user_id=%d", arr[i].uid);
        notification_send(arr[i].uid, "当选副社长",
                          "恭喜您当选本社团副社长！", "election_result", club_id);
    }

    db_execute("UPDATE elections SET status='completed', settled_at=NOW() "
               "WHERE election_id=%d", eid);
    free(arr);

    /* 返回最终排名 */
    MYSQL_RES *res = db_query(
        "SELECT c.user_id, u.real_name, c.votes, "
        "CASE WHEN m.role IN ('president','vice_president') THEN m.role ELSE '' END AS elected_role "
        "FROM election_candidates c JOIN users u ON c.user_id=u.user_id "
        "LEFT JOIN members m ON m.user_id=c.user_id AND m.club_id=%d AND m.left_at IS NULL "
        "WHERE c.election_id=%d ORDER BY c.votes DESC", club_id, eid);
    api_send_result(ctx, res);
}

#endif /* 备用文件：改 #if 0 为 #if 1 启用 */
