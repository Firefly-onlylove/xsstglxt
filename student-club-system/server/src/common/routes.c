/**
 * routes.c — HTTP 路由表定义
 * 学生社团管理系统 SCMS（Web 架构）
 *
 * 将每个（HTTP 方法 + URI 模式）映射到 routes.h 中声明的处理函数。
 * main.c 的 dispatch_api() 遍历此表，找到第一个匹配的条目后调用对应处理函数。
 *
 * URI 模式规则（参见 main.c 的 uri_match 函数）：
 *   '*' 匹配单个路径段（不跨 '/'）
 *   例：/api/school/clubs/*          可匹配 /api/school/clubs/12
 *       /api/school/clubs/*/approve  可匹配 /api/school/clubs/12/approve
 *
 * 重要：路由顺序有先后！更具体的路径必须放在通配路径之前。
 * 例：/api/school/clubs/pending 必须在 /api/school/clubs/* 之前，
 * 否则 "pending" 会被通配符吞掉，导致永远匹配不到。
 *
 * need_login 字段：1 = 需要登录（main.c 在分发前检查 ctx->user）。
 * 更细粒度的角色检查在 handler 内部完成（api_require_school_admin 等函数）。
 *
 * 路径参数：模式中的 {id} 类占位符在实际路径中写为 '*'。
 * handler 内部通过 api_get_path_int(ctx, n) 读取 /api/ 之后的第 n 段（n 从 0 开始）。
 */
#include "../../include/routes.h"

/* ─────────────────────────────────────────────────────────────────────
 * 路由表（静态数组）。
 * 说明：路径里带 {id} 的，实际写成 '*'，处理函数内用 api_get_path_int(ctx, n)
 *      按段序号取值（n 从 0 开始，指 /api/ 之后的第几段）。
 *      need_login=1 表示仅登录用户可访问。
 * ───────────────────────────────────────────────────────────────────── */
static const Route ROUTES[] = {

    /* ═══════════════ 公共接口（无需登录） ═══════════════ */
    { "POST", "/api/register",       pub_register,       0 },
    { "GET",  "/api/colleges",       pub_colleges_list,  0 },
    { "GET",  "/api/majors",         pub_majors_list,    0 },
    { "GET",  "/api/classes",        pub_classes_list,   0 },

    /* ═══════════════ 通知中心（需登录） ═══════════════ */
    { "GET",  "/api/notifications/unread",   pub_notif_unread,   1 },
    { "POST", "/api/notifications/read-all", pub_notif_read_all, 1 },
    { "POST", "/api/notifications/read",     pub_notif_read,     1 },
    { "GET",  "/api/notifications",          pub_notif_list,     1 },

    /* ═══════════════ 学校端（需登录 + 内部校验 school_admin 角色） ═══════════════ */
    /* 仪表盘与统计 */
    { "GET",  "/api/school/dashboard",     sch_dashboard,    1 },
    { "GET",  "/api/school/stats/export",  sch_stats_export, 1 },
    { "GET",  "/api/school/stats",         sch_stats,        1 },

    /* 社团审批（具体路径在前，通配 '*' 在后） */
    { "GET",  "/api/school/clubs/pending", sch_club_pending,         1 },
    { "GET",  "/api/school/clubs/history", sch_club_approved_history,1 },
    { "POST", "/api/school/clubs/*/approve",  sch_club_approve,   1 },
    { "POST", "/api/school/clubs/*/reject",   sch_club_reject,    1 },
    { "POST", "/api/school/clubs/*/level",    sch_club_set_level, 1 },
    { "POST", "/api/school/clubs/*/dissolve", sch_club_dissolve,  1 },
    { "POST", "/api/school/clubs/*/rename",   sch_club_rename,    1 },
    { "GET",  "/api/school/clubs/*",          sch_club_detail,    1 },
    { "GET",  "/api/school/clubs",            sch_club_list,      1 },

    /* 用户管理 */
    { "POST", "/api/school/users/college-admin",   sch_user_create_college_admin, 1 },
    { "POST", "/api/school/users/*/set-role",        sch_user_set_role,    1 },
    { "POST", "/api/school/users/*/toggle",         sch_user_toggle,        1 },
    { "POST", "/api/school/users/*/reset-password", sch_user_reset_pwd,     1 },
    { "POST", "/api/school/users/*/restrict",       sch_user_restrict,      1 },
    { "POST", "/api/school/users/*/lift-restriction",sch_user_lift_restrict,1 },
    { "GET",  "/api/school/users/*",                sch_user_detail,        1 },
    { "GET",  "/api/school/users",                  sch_user_list,          1 },
    { "GET",  "/api/school/restrictions",           sch_restriction_list,   1 },

    /* 财务监督 */
    { "GET",  "/api/school/finance/clubs",              sch_finance_overview,1 },
    { "GET",  "/api/school/finance/*",                sch_finance_club,   1 },
    { "GET",  "/api/school/finance",                  sch_finance_overview,1 },
    { "GET",  "/api/school/reimbursements/pending",   sch_reimb_pending,  1 },
    { "GET",  "/api/school/reimbursements/college",   sch_reimb_college_list, 1 },
    { "POST", "/api/school/reimbursements/*/approve", sch_reimb_approve,  1 },
    { "POST", "/api/school/reimbursements/*/reject",  sch_reimb_reject,   1 },

    /* 公告管理 */
    { "DELETE", "/api/school/announcements/*", sch_announce_delete, 1 },
    { "POST",   "/api/school/announcements",   sch_announce_post,   1 },
    { "GET",    "/api/school/announcements",   sch_announce_list,   1 },

    /* 系统与备份 */
    { "POST", "/api/school/backup",  sch_backup_create, 1 },
    { "GET",  "/api/school/backups", sch_backup_list,   1 },
    { "GET",  "/api/school/config",  sch_config_get,    1 },
    { "POST", "/api/school/config",  sch_config_set,    1 },

    /* ═══════════════ 学院端（需登录 + 内部校验 college_admin 角色） ═══════════════ */
    { "GET",  "/api/college/dashboard",    col_dashboard,    1 },
    { "GET",  "/api/college/stats/export", col_stats_export, 1 },
    { "GET",  "/api/college/stats",        col_stats,        1 },

    /* 班级管理 */
    { "PUT",    "/api/college/classes/*", col_class_update, 1 },
    { "DELETE", "/api/college/classes/*", col_class_delete, 1 },
    { "POST",   "/api/college/classes",   col_class_create, 1 },
    { "GET",    "/api/college/classes",   col_class_list,   1 },

    /* 本院社团监督 */
    { "POST", "/api/college/clubs/*/dissolve",   col_club_dissolve,  1 },
    { "POST", "/api/college/clubs/*/rename",     col_club_rename,    1 },
    { "GET",  "/api/college/clubs/*/members",    col_club_members,   1 },
    { "GET",  "/api/college/clubs/*/activities", col_club_activities,1 },
    { "GET",  "/api/college/clubs/*",            col_club_detail,    1 },
    { "GET",  "/api/college/clubs",              col_club_list,      1 },

    /* 报销审批 + 经费额度管理 */
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
    { "DELETE","/api/college/announcements/*", col_announce_delete, 1 },
    { "POST", "/api/college/announcements", col_announce_post, 1 },
    { "GET",  "/api/college/announcements", col_announce_list, 1 },

    /* ═══════════════ 学生端（需登录） ═══════════════ */
    /* 社团广场（注意 /api/clubs/apply 在通配 /api/clubs/* 之前） */
    { "POST", "/api/clubs/apply",         stu_club_apply,      1 },
    { "POST", "/api/clubs/*/join",        stu_club_join,       1 },
    { "POST", "/api/clubs/*/leave",       stu_club_leave,      1 },
    { "GET",  "/api/clubs/*/members",     stu_club_members,    1 },
    { "GET",  "/api/clubs/*/activities",  stu_club_activities, 1 },
    { "GET",  "/api/clubs/*",             stu_club_detail,     1 },
    { "GET",  "/api/clubs",               stu_club_browse,     1 },

    /* 活动报名与签到 */
    { "POST", "/api/activities/*/register", stu_activity_register, 1 },
    { "POST", "/api/activities/*/cancel",   stu_activity_cancel,   1 },
    { "POST", "/api/activities/*/signin",   stu_activity_signin,   1 },
    { "GET",  "/api/activities/*/registrations", stu_activity_registrations, 1 },
    { "GET",  "/api/activities/*",          stu_activity_detail,   1 },
    { "GET",  "/api/activities",            stu_activity_list,     1 },

    /* 个人中心 */
    { "POST", "/api/profile/password", stu_change_password, 1 },
    { "PUT",  "/api/profile",          stu_profile_update,  1 },
    { "GET",  "/api/profile",          stu_profile_get,     1 },

    /* 我的社团 */
    { "GET",  "/api/my/clubs",        stu_my_clubs,        1 },
    { "GET",  "/api/my/applications", stu_my_applications, 1 },

    /* ═══════════════ 社团管理端（需登录 + 内部校验社长/副社长） ═══════════════
     * 路径统一形如 /api/club/{id}/...，段序号：0=club 1={id} 2=... */
    { "GET",  "/api/club/*/dashboard",  club_dashboard,      1 },
    { "GET",  "/api/club/*/profile",    club_profile_get,    1 },
    { "PUT",  "/api/club/*/profile",    club_profile_update, 1 },

    /* 成员管理（具体路径在前，通配在后） */
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

    /* 财务管理 */
    { "GET",    "/api/club/*/finance",           club_finance_list,   1 },
    { "POST",   "/api/club/*/finance",           club_finance_create, 1 },
    { "DELETE", "/api/club/*/finance/*",         club_finance_delete, 1 },
    { "POST",   "/api/club/*/upload-receipt",    club_reimb_upload,   1 },
    { "POST",   "/api/club/*/reimbursements",    club_reimb_create,   1 },
    { "GET",    "/api/club/*/reimbursements",    club_reimb_list,     1 },

    /* 换届选举（具体路径在前） */
    { "POST", "/api/club/*/elections/reelect",       club_election_reelect,    1 },
    { "POST", "/api/club/*/elections/*/signup",      club_election_signup,     1 },
    { "GET",  "/api/club/*/elections/*/candidates",  club_election_candidates, 1 },
    { "POST", "/api/club/*/elections/*/vote",        club_election_vote,       1 },
    { "GET",  "/api/club/*/elections/*/result",      club_election_result,     1 },
    { "GET",  "/api/club/*/elections/*",             club_election_detail,     1 },
    { "POST", "/api/club/*/elections",               club_election_create,     1 },
    { "GET",  "/api/club/*/elections",               club_election_list,       1 },

    /* 社团公告 */
    { "POST", "/api/club/*/announcements", club_announce_post, 1 },
};

/**
 * routes_table — 返回路由表及条目数量
 * @out_count: 输出参数，写入路由表条目总数
 * 返回值：指向静态路由表数组的指针
 *
 * 路由表在编译期确定，存于只读数据段，运行时不可修改
 */
const Route *routes_table(int *out_count) {
    *out_count = (int)(sizeof(ROUTES) / sizeof(ROUTES[0]));
    return ROUTES;
}
