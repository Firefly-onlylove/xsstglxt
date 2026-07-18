/**
 * routes.c -- HTTP route table definition
 * Student Club Management System SCMS (Web arch)
 *
 * Maps each (HTTP method + URI pattern) to a handler declared in routes.h.
 * main.c's dispatch_api() walks this table and calls the first match.
 *
 * URI pattern rules (see uri_match in main.c):
 *   '*' matches a single path segment (not crossing '/').
 *   E.g.: /api/school/clubs/star          matches /api/school/clubs/12
 *         /api/school/clubs/star/approve  matches /api/school/clubs/12/approve
 *
 * WARNING: route ordering matters! More specific routes MUST come before
 * wildcard ones. E.g. /api/school/clubs/pending must precede
 * /api/school/clubs/* , otherwise "pending" will be swallowed by the wildcard.
 *
 * need_login: 1 = login required (main.c checks ctx->user before dispatching).
 * Finer-grained role checks happen inside the handler (api_require_school_admin etc).
 *
 * Path params: placeholders like {id} are written as '*' in patterns.
 * Handlers use api_get_path_int(ctx, n) to read the n-th segment after /api/.
 */
#include "../../include/routes.h"

/* ─────────────────────────────────────────────────────────────────────
 * 路由表。
 * 说明：路径里带 {id} 的，实际写成 '*'，处理函数内用 api_get_path_int(ctx, n)
 *      按段序号取值（n 从 0 开始，指 /api/ 之后的第几段）。
 * ───────────────────────────────────────────────────────────────────── */
static const Route ROUTES[] = {

    /* ═══════════════ 公共（无需登录） ═══════════════ */
    { "POST", "/api/register",       pub_register,       0 },
    { "GET",  "/api/colleges",       pub_colleges_list,  0 },
    { "GET",  "/api/majors",         pub_majors_list,    0 },
    { "GET",  "/api/classes",        pub_classes_list,   0 },

    /* ═══════════════ 通知中心（需登录） ═══════════════ */
    { "GET",  "/api/notifications/unread",   pub_notif_unread,   1 },
    { "POST", "/api/notifications/read-all", pub_notif_read_all, 1 },
    { "POST", "/api/notifications/read",     pub_notif_read,     1 },
    { "GET",  "/api/notifications",          pub_notif_list,     1 },

    /* ═══════════════ 学校端 sch_（需登录 + 内部校验 school_admin） ═══════════════ */
    /* 仪表盘/统计 */
    { "GET",  "/api/school/dashboard",     sch_dashboard,    1 },
    { "GET",  "/api/school/stats/export",  sch_stats_export, 1 },
    { "GET",  "/api/school/stats",         sch_stats,        1 },

    /* 社团审批（具体路径在前，通配在后） */
    { "GET",  "/api/school/clubs/pending", sch_club_pending,         1 },
    { "GET",  "/api/school/clubs/history", sch_club_approved_history,1 },
    { "POST", "/api/school/clubs/*/approve",  sch_club_approve,   1 },
    { "POST", "/api/school/clubs/*/reject",   sch_club_reject,    1 },
    { "POST", "/api/school/clubs/*/level",    sch_club_set_level, 1 },
    { "POST", "/api/school/clubs/*/dissolve", sch_club_dissolve,  1 },
    { "GET",  "/api/school/clubs/*",          sch_club_detail,    1 },
    { "GET",  "/api/school/clubs",            sch_club_list,      1 },

    /* 学院管理 */
    { "POST", "/api/school/colleges/*/toggle", sch_college_toggle, 1 },
    { "PUT",  "/api/school/colleges/*",        sch_college_update, 1 },
    { "POST", "/api/school/colleges",          sch_college_create, 1 },
    { "GET",  "/api/school/colleges",          sch_college_list,   1 },

    /* 专业管理 */
    { "PUT",    "/api/school/majors/*", sch_major_update, 1 },
    { "DELETE", "/api/school/majors/*", sch_major_delete, 1 },
    { "POST",   "/api/school/majors",   sch_major_create, 1 },
    { "GET",    "/api/school/majors",   sch_major_list,   1 },

    /* 用户管理 */
    { "POST", "/api/school/users/college-admin",   sch_user_create_college_admin, 1 },
    { "POST", "/api/school/users/*/toggle",         sch_user_toggle,        1 },
    { "POST", "/api/school/users/*/reset-password", sch_user_reset_pwd,     1 },
    { "POST", "/api/school/users/*/restrict",       sch_user_restrict,      1 },
    { "POST", "/api/school/users/*/lift-restriction",sch_user_lift_restrict,1 },
    { "GET",  "/api/school/users/*",                sch_user_detail,        1 },
    { "GET",  "/api/school/users",                  sch_user_list,          1 },
    { "GET",  "/api/school/restrictions",           sch_restriction_list,   1 },

    /* 财务监督 */
    { "GET",  "/api/school/finance/*",                sch_finance_club,   1 },
    { "GET",  "/api/school/finance",                  sch_finance_overview,1 },
    { "GET",  "/api/school/reimbursements/pending",   sch_reimb_pending,  1 },
    { "POST", "/api/school/reimbursements/*/approve", sch_reimb_approve,  1 },
    { "POST", "/api/school/reimbursements/*/reject",  sch_reimb_reject,   1 },

    /* 公告 */
    { "DELETE", "/api/school/announcements/*", sch_announce_delete, 1 },
    { "POST",   "/api/school/announcements",   sch_announce_post,   1 },
    { "GET",    "/api/school/announcements",   sch_announce_list,   1 },

    /* 系统与备份 */
    { "POST", "/api/school/backup",  sch_backup_create, 1 },
    { "GET",  "/api/school/backups", sch_backup_list,   1 },
    { "GET",  "/api/school/config",  sch_config_get,    1 },
    { "POST", "/api/school/config",  sch_config_set,    1 },

    /* ═══════════════ 学院端 col_（需登录 + 内部校验 college_admin） ═══════════════ */
    { "GET",  "/api/college/dashboard",    col_dashboard,    1 },
    { "GET",  "/api/college/stats/export", col_stats_export, 1 },
    { "GET",  "/api/college/stats",        col_stats,        1 },

    /* 班级管理 */
    { "PUT",    "/api/college/classes/*", col_class_update, 1 },
    { "DELETE", "/api/college/classes/*", col_class_delete, 1 },
    { "POST",   "/api/college/classes",   col_class_create, 1 },
    { "GET",    "/api/college/classes",   col_class_list,   1 },

    /* 本院社团监督 */
    { "GET",  "/api/college/clubs/*", col_club_detail, 1 },
    { "GET",  "/api/college/clubs",   col_club_list,   1 },

    /* 报销审批 + 额度 */
    { "GET",  "/api/college/reimbursements/pending",   col_reimb_pending, 1 },
    { "POST", "/api/college/reimbursements/*/approve", col_reimb_approve, 1 },
    { "POST", "/api/college/reimbursements/*/reject",  col_reimb_reject,  1 },
    { "GET",  "/api/college/limits",       col_limit_list, 1 },
    { "POST", "/api/college/limits/*",     col_limit_set,  1 },

    /* 本院学生管理 */
    { "POST", "/api/college/students/*/restrict",        col_student_restrict, 1 },
    { "POST", "/api/college/students/*/lift-restriction",col_student_lift,     1 },
    { "GET",  "/api/college/students",                   col_student_list,     1 },

    /* 学院公告 */
    { "POST", "/api/college/announcements", col_announce_post, 1 },
    { "GET",  "/api/college/announcements", col_announce_list, 1 },

    /* ═══════════════ 学生端 stu_（需登录） ═══════════════ */
    /* 社团广场（注意 apply 在通配前） */
    { "POST", "/api/clubs/apply",         stu_club_apply,      1 },
    { "POST", "/api/clubs/*/join",        stu_club_join,       1 },
    { "POST", "/api/clubs/*/leave",       stu_club_leave,      1 },
    { "GET",  "/api/clubs/*/members",     stu_club_members,    1 },
    { "GET",  "/api/clubs/*/activities",  stu_club_activities, 1 },
    { "GET",  "/api/clubs/*",             stu_club_detail,     1 },
    { "GET",  "/api/clubs",               stu_club_browse,     1 },

    /* 活动 */
    { "POST", "/api/activities/*/register", stu_activity_register, 1 },
    { "POST", "/api/activities/*/cancel",   stu_activity_cancel,   1 },
    { "POST", "/api/activities/*/signin",   stu_activity_signin,   1 },
    { "GET",  "/api/activities/*",          stu_activity_detail,   1 },
    { "GET",  "/api/activities",            stu_activity_list,     1 },

    /* 个人中心 */
    { "POST", "/api/profile/password", stu_change_password, 1 },
    { "PUT",  "/api/profile",          stu_profile_update,  1 },
    { "GET",  "/api/profile",          stu_profile_get,     1 },

    /* 我的社团 */
    { "GET",  "/api/my/clubs",        stu_my_clubs,        1 },
    { "GET",  "/api/my/applications", stu_my_applications, 1 },

    /* ═══════════════ 社团管理端 club_（需登录 + 内部校验社长/副社长） ═══════════════
     * 路径统一形如 /api/club/{id}/...，段序号：0=club 1={id} 2=... */
    { "GET",  "/api/club/*/dashboard",  club_dashboard,      1 },
    { "GET",  "/api/club/*/profile",    club_profile_get,    1 },
    { "PUT",  "/api/club/*/profile",    club_profile_update, 1 },

    /* 成员管理（具体在前） */
    { "GET",  "/api/club/*/join-requests",          club_join_pending,  1 },
    { "POST", "/api/club/*/join-requests/*/approve",club_join_approve,  1 },
    { "POST", "/api/club/*/join-requests/*/reject", club_join_reject,   1 },
    { "POST", "/api/club/*/members/*/remove",       club_member_remove, 1 },
    { "POST", "/api/club/*/members/*/appoint",      club_member_appoint,1 },
    { "GET",  "/api/club/*/members",                club_member_list,   1 },

    /* 活动管理（具体动作在前，通配详情在后） */
    { "GET",  "/api/club/*/activities/*/signins",       club_signin_list,     1 },
    { "POST", "/api/club/*/activities/*/manual-signin", club_signin_manual,   1 },
    { "POST", "/api/club/*/activities/*/publish",       club_activity_publish,1 },
    { "POST", "/api/club/*/activities/*/start",         club_activity_start,  1 },
    { "POST", "/api/club/*/activities/*/finish",        club_activity_finish, 1 },
    { "POST", "/api/club/*/activities/*/cancel",        club_activity_cancel, 1 },
    { "POST", "/api/club/*/activities/*/summary",       club_activity_summary,1 },
    { "PUT",  "/api/club/*/activities/*",               club_activity_update, 1 },
    { "DELETE","/api/club/*/activities/*",              club_activity_delete, 1 },
    { "POST", "/api/club/*/activities",                 club_activity_create, 1 },
    { "GET",  "/api/club/*/activities",                 club_activity_list,   1 },

    /* 财务 */
    { "GET",  "/api/club/*/finance",        club_finance_list, 1 },
    { "POST", "/api/club/*/reimbursements", club_reimb_create, 1 },
    { "GET",  "/api/club/*/reimbursements", club_reimb_list,   1 },

    /* 换届选举（具体在前） */
    { "POST", "/api/club/*/elections/reelect",       club_election_reelect,    1 },
    { "POST", "/api/club/*/elections/*/signup",      club_election_signup,     1 },
    { "GET",  "/api/club/*/elections/*/candidates",  club_election_candidates, 1 },
    { "POST", "/api/club/*/elections/*/vote",        club_election_vote,       1 },
    { "GET",  "/api/club/*/elections/*/result",      club_election_result,     1 },
    { "GET",  "/api/club/*/elections/*",             club_election_detail,     1 },
    { "POST", "/api/club/*/elections",               club_election_create,     1 },
    { "GET",  "/api/club/*/elections",               club_election_list,       1 },

    /* 社团通知 */
    { "POST", "/api/club/*/announcements", club_announce_post, 1 },
};

const Route *routes_table(int *out_count) {
    *out_count = (int)(sizeof(ROUTES) / sizeof(ROUTES[0]));
    return ROUTES;
}
