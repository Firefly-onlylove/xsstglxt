/**
 * handlers_stub.c -- 业务处理函数默认实现
 * Student Club Management System SCMS (Web arch)
 *
 * Purpose: provides default implementations for ALL handler functions
 * declared in routes.h. These are regular (strong) symbols so the linker
 * can always resolve references from routes.c.
 *
 * When you uncomment a real business module (by changing #if 0 to #if 1),
 * you MUST delete / comment out the corresponding STUB_HANDLER(...) line
 * below to avoid "duplicate symbol" linker errors.
 *
 * Each stub returns: {"code":-99,"msg":"Not implemented yet"}
 *
 * DO NOT write business logic here. Put real code in the per-module .c files.
 */
#include "../../include/routes.h"
#include "../../include/api.h"
#include "../../include/common.h"

#include <stdio.h>

static void stub_reply(ApiContext *ctx) {
    api_error(ctx, ERR_SYSTEM, "该功能尚未实现（业务模块待编写）");
}

#define STUB_HANDLER(name) \
    void name(ApiContext *ctx) { stub_reply(ctx); }

/* ===== public ===== */
STUB_HANDLER(pub_register)
STUB_HANDLER(pub_colleges_list)
STUB_HANDLER(pub_majors_list)
STUB_HANDLER(pub_classes_list)
STUB_HANDLER(pub_notif_list)
STUB_HANDLER(pub_notif_unread)
STUB_HANDLER(pub_notif_read)
STUB_HANDLER(pub_notif_read_all)

/* ===== school ===== */
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
STUB_HANDLER(sch_announce_list)
STUB_HANDLER(sch_announce_post)
STUB_HANDLER(sch_announce_delete)
STUB_HANDLER(sch_backup_create)
STUB_HANDLER(sch_backup_list)
STUB_HANDLER(sch_config_get)
STUB_HANDLER(sch_config_set)

/* ===== college ===== */
STUB_HANDLER(col_dashboard)
STUB_HANDLER(col_stats)
STUB_HANDLER(col_stats_export)
STUB_HANDLER(col_class_list)
STUB_HANDLER(col_class_create)
STUB_HANDLER(col_class_update)
STUB_HANDLER(col_class_delete)
STUB_HANDLER(col_club_list)
STUB_HANDLER(col_club_detail)
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

/* ===== student ===== */
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

/* ===== club admin ===== */
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
