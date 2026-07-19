/**
 * school_dashboard.c — 学校端 仪表盘 + 全校统计
 * 学生社团管理系统 SCMS（Web 架构 · 学校端）
 *
 * ┌──────────────────────────────────────────────────────────────────┐
 * │  ★ 备用参考实现。默认整份被 #if 0 关闭，不参与编译（走弱符号占位）。 │
 * │  ★ 启用方法：把下面这行的  #if 0  改成  #if 1  ，重新 make 即可。   │
 * │  ★ 启用后本文件的强符号会自动覆盖 handlers_stub.c 的弱符号。        │
 * └──────────────────────────────────────────────────────────────────┘
 *
 * 覆盖路由：
 *   GET /api/school/dashboard    → sch_dashboard        仪表盘数据卡片
 *   GET /api/school/stats        → sch_stats            分类/学院统计
 *   GET /api/school/stats/export → sch_stats_export     导出 CSV
 */
#if 1

#include "../../include/routes.h"
#include "../../include/api.h"
#include "../../include/db.h"
#include "../../include/common.h"
#include "../../include/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* GET /api/school/dashboard
 * 返回四张数据卡片 + 最近日志 + 各学院社团分布。 */
void sch_dashboard(ApiContext *ctx) {
    if (!api_require_school_admin(ctx)) return;

    int active_clubs = db_query_int(
        "SELECT COUNT(*) FROM clubs WHERE status='approved'");
    int total_members = db_query_int(
        "SELECT COUNT(*) FROM members WHERE left_at IS NULL AND join_status='approved'");
    int monthly_activities = db_query_int(
        "SELECT COUNT(*) FROM activities WHERE status IN ('published','ongoing','completed')");
    int pending_approvals = db_query_int(
        "SELECT COUNT(*) FROM clubs WHERE status='pending'");

    /* 最近20条操作日志 */
    MYSQL_RES *log_res = db_query(
        "SELECT u.real_name AS username, l.action, l.created_at "
        "FROM logs l LEFT JOIN users u ON l.user_id=u.user_id "
        "ORDER BY l.created_at DESC LIMIT 20");
    char *logs_json = db_result_to_json_array(log_res);
    mysql_free_result(log_res);

    JsonBuilder jb;
    json_init(&jb);
    json_add_int(&jb, "active_clubs", active_clubs);
    json_add_int(&jb, "total_members", total_members);
    json_add_int(&jb, "monthly_activities", monthly_activities);
    json_add_int(&jb, "pending_approvals", pending_approvals);
    json_add_raw(&jb, "recent_logs", logs_json ? logs_json : "[]");
    api_ok_data(ctx, json_finish(&jb));
    json_free(&jb);
    free(logs_json);
}

#endif /* 备用代码结束 */
