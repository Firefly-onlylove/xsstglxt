/**
 * handlers_stub.c — 业务处理函数默认实现（桩函数）
 * 学生社团管理系统 SCMS（Web 架构）
 *
 * 用途：为 routes.h 中声明的所有 handler 函数提供默认实现。
 * 这些是普通（强）符号，因此链接器总能解析来自 routes.c 的引用。
 *
 * 当启用某个真实业务模块时（将 #if 0 改为 #if 1），
 * 必须删除或注释掉下方对应的 STUB_HANDLER(...) 行，
 * 以避免链接器报"重复符号"错误。
 *
 * 每个桩函数统一返回：{"code":-99,"msg":"功能尚未实现"}
 *
 * 注意：不要在此文件中编写业务逻辑。真实代码请放在各模块独立的 .c 文件中。
 */
#include "../../include/routes.h"
#include "../../include/api.h"
#include "../../include/common.h"

#include <stdio.h>

/**
 * stub_reply — 桩函数的统一响应
 * @ctx: 请求上下文
 *
 * 所有桩函数都调用此函数，返回"功能尚未实现"的错误响应
 */
static void stub_reply(ApiContext *ctx) {
    api_error(ctx, ERR_SYSTEM, "该功能尚未实现（业务模块待编写）");
}

/* STUB_HANDLER 宏：生成一个名为 name 的函数，函数体只调用 stub_reply */
#define STUB_HANDLER(name) \
    void name(ApiContext *ctx) { stub_reply(ctx); }

/* ===== 公共接口（无需登录） ===== */
STUB_HANDLER(pub_register)
STUB_HANDLER(pub_colleges_list)
STUB_HANDLER(pub_majors_list)
STUB_HANDLER(pub_classes_list)
STUB_HANDLER(pub_notif_list)
STUB_HANDLER(pub_notif_unread)
STUB_HANDLER(pub_notif_read)
STUB_HANDLER(pub_notif_read_all)

/* ===== 学校端 ===== */
STUB_HANDLER(sch_dashboard)
STUB_HANDLER(sch_stats)
STUB_HANDLER(sch_stats_export)
STUB_HANDLER(sch_club_pending)
STUB_HANDLER(sch_club_approve)
STUB_HANDLER(sch_club_reject)
STUB_HANDLER(sch_club_approved_history)
STUB_HANDLER(sch_club_list)
STUB_HANDLER(sch_club_detail)
STUB_HANDLER(sch_club_set_level)
STUB_HANDLER(sch_club_dissolve)
STUB_HANDLER(sch_club_rename)
STUB_HANDLER(sch_user_list)
STUB_HANDLER(sch_user_detail)
STUB_HANDLER(sch_user_create_college_admin)
STUB_HANDLER(sch_user_set_role)
STUB_HANDLER(sch_user_toggle)
STUB_HANDLER(sch_user_reset_pwd)
STUB_HANDLER(sch_user_restrict)
STUB_HANDLER(sch_user_lift_restrict)
STUB_HANDLER(sch_restriction_list)
STUB_HANDLER(sch_finance_overview)
STUB_HANDLER(sch_finance_club)
STUB_HANDLER(sch_reimb_pending)
STUB_HANDLER(sch_reimb_approve)
STUB_HANDLER(sch_reimb_reject)
STUB_HANDLER(sch_reimb_college_list)
STUB_HANDLER(sch_announce_list)
STUB_HANDLER(sch_announce_post)
STUB_HANDLER(sch_announce_delete)
STUB_HANDLER(sch_backup_create)
STUB_HANDLER(sch_backup_list)
STUB_HANDLER(sch_config_get)
STUB_HANDLER(sch_config_set)

/* ===== 学院端 ===== */
STUB_HANDLER(col_dashboard)
STUB_HANDLER(col_stats)
STUB_HANDLER(col_stats_export)
STUB_HANDLER(col_class_list)
STUB_HANDLER(col_class_create)
STUB_HANDLER(col_class_update)
STUB_HANDLER(col_class_delete)
STUB_HANDLER(col_club_list)
STUB_HANDLER(col_club_detail)
STUB_HANDLER(col_club_members)
STUB_HANDLER(col_club_activities)
STUB_HANDLER(col_club_rename)
STUB_HANDLER(col_club_dissolve)
STUB_HANDLER(col_reimb_pending)
STUB_HANDLER(col_reimb_approve)
STUB_HANDLER(col_reimb_reject)
STUB_HANDLER(col_limit_list)
STUB_HANDLER(col_limit_set)
STUB_HANDLER(col_student_list)
STUB_HANDLER(col_student_restrict)
STUB_HANDLER(col_student_lift)
STUB_HANDLER(col_announce_list)
STUB_HANDLER(col_announce_post)
STUB_HANDLER(col_announce_delete)

/* ===== 学生端 ===== */
STUB_HANDLER(stu_club_browse)
STUB_HANDLER(stu_club_detail)
STUB_HANDLER(stu_club_members)
STUB_HANDLER(stu_club_activities)
STUB_HANDLER(stu_club_apply)
STUB_HANDLER(stu_club_join)
STUB_HANDLER(stu_club_leave)
STUB_HANDLER(stu_activity_list)
STUB_HANDLER(stu_activity_detail)
STUB_HANDLER(stu_activity_registrations)
STUB_HANDLER(stu_activity_register)
STUB_HANDLER(stu_activity_cancel)
STUB_HANDLER(stu_activity_signin)
STUB_HANDLER(stu_profile_get)
STUB_HANDLER(stu_profile_update)
STUB_HANDLER(stu_change_password)
STUB_HANDLER(stu_my_clubs)
STUB_HANDLER(stu_my_applications)

/* ===== 社团管理端 ===== */
STUB_HANDLER(club_profile_get)
STUB_HANDLER(club_profile_update)
STUB_HANDLER(club_dashboard)
STUB_HANDLER(club_member_list)
STUB_HANDLER(club_join_pending)
STUB_HANDLER(club_join_approve)
STUB_HANDLER(club_join_reject)
STUB_HANDLER(club_member_remove)
STUB_HANDLER(club_member_appoint)
STUB_HANDLER(club_activity_list)
STUB_HANDLER(club_activity_create)
STUB_HANDLER(club_activity_update)
STUB_HANDLER(club_activity_publish)
STUB_HANDLER(club_activity_start)
STUB_HANDLER(club_activity_finish)
STUB_HANDLER(club_activity_cancel)
STUB_HANDLER(club_activity_summary)
STUB_HANDLER(club_activity_delete)
STUB_HANDLER(club_signin_list)
STUB_HANDLER(club_signin_manual)
STUB_HANDLER(club_finance_create)
STUB_HANDLER(club_finance_delete)
STUB_HANDLER(club_reimb_upload)
STUB_HANDLER(club_election_list)
STUB_HANDLER(club_election_create)
STUB_HANDLER(club_election_detail)
STUB_HANDLER(club_election_reelect)
STUB_HANDLER(club_election_signup)
STUB_HANDLER(club_election_candidates)
STUB_HANDLER(club_election_vote)
STUB_HANDLER(club_election_result)
STUB_HANDLER(club_announce_post)
