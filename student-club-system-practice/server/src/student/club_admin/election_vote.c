/**
 * election_vote.c — 社团管理端 匿名投票 + 结果结算
 * 学生社团管理系统 SCMS（练习文件 — 代码空白，由你自行实现）
 *
 * 注意：投票逻辑已并入 election.c 的 club_election_vote()
 * 本文件负责：投票状态查询 + 结果汇总 + 平票处理
 *
 * 需要实现的处理函数：
 *
 *   void club_vote_status(ApiContext *ctx)
 *     路由：GET /api/club-admin/elections/{id}/vote-status
 *     功能：返回当前选举阶段、各候选人票数、当前用户是否已投票
 *
 *   void club_vote_result(ApiContext *ctx)
 *     路由：GET /api/club-admin/elections/{id}/result
 *     功能：返回选举最终结果（候选人排名 + 票数）
 *     注意：仅 status='finished' 的选举才能查看结果
 *
 * 投票数据来源：
 *   election_votes 表：election_id, candidate_id
 *   SELECT candidate_id, COUNT(*) AS vote_count FROM election_votes
 *   WHERE election_id=? GROUP BY candidate_id ORDER BY vote_count DESC
 *
 * 参考完整实现：student-club-system/src/student/club_admin/election_vote.c
 */

#include "../../../include/api.h"
#include "../../../include/db.h"
#include "../../../include/common.h"
#include "../../../include/routes.h"
#include "../../../include/utils.h"

#include <stdio.h>

void club_vote_status(ApiContext *ctx) {
    /* TODO: 投票状态 */
}

void club_vote_result(ApiContext *ctx) {
    /* TODO: 选举结果 */
}
