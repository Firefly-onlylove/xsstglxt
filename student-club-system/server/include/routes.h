/**
 * routes.h — 业务 API 处理函数原型 + 路由分发声明
 * 学生社团管理系统 SCMS（Web 架构）
 *
 * ┌────────────────────────────────────────────────────────────────┐
 * │  这里声明了全部业务 API 处理函数。                                │
 * │  每个函数你（业务模块作者）在对应的 .c 文件里实现，签名统一为：    │
 * │      void 处理函数名(ApiContext *ctx);                           │
 * │                                                                  │
 * │  main.c 里的路由表把 HTTP 请求(方法+路径) 映射到这些函数。         │
 * │                                                                  │
 * │  这些函数在业务模块尚未编写时，由 handlers_stub.c 提供弱符号占位，  │
 * │  返回"功能尚未实现"。你把真实模块解注释并编译后，强符号自动覆盖。   │
 * └────────────────────────────────────────────────────────────────┘
 *
 * 命名规则： <端>_<模块>_<动作>
 *   sch_  = 学校端 (school)
 *   col_  = 学院端 (college)
 *   stu_  = 学生端 (student)
 *   club_ = 社团管理端 (club_admin)
 *   pub_  = 公共（无需登录或所有角色）
 */
#ifndef ROUTES_H
#define ROUTES_H

#include "api.h"

/* 处理函数指针类型 */
typedef void (*ApiHandler)(ApiContext *ctx);

/* 路由项：HTTP 方法 + URI 模式（支持 * 通配单段） + 处理函数 + 是否需要登录 */
typedef struct {
    const char *method;    /* "GET"/"POST"/"PUT"/"DELETE"，NULL 表示任意方法 */
    const char *pattern;   /* 如 "/api/school/clubs/star"，star 匹配单个路径段 */
    ApiHandler  handler;
    int         need_login;/* 1=需登录（main 会先校验 ctx->user） */
} Route;

/* 返回路由表首地址，并通过 out_count 返回条数（定义在 main.c） */
const Route *routes_table(int *out_count);

/* ═══════════════════════ 公共（pub_） ═══════════════════════ */
void pub_register(ApiContext *ctx);          /* POST /api/register 用户注册 */
void pub_colleges_list(ApiContext *ctx);     /* GET  /api/colleges 学院下拉 */
void pub_majors_list(ApiContext *ctx);       /* GET  /api/majors?college_id= 专业下拉 */
void pub_classes_list(ApiContext *ctx);      /* GET  /api/classes?major_id= 班级下拉 */

/* 通知中心（所有登录角色） */
void pub_notif_list(ApiContext *ctx);        /* GET  /api/notifications */
void pub_notif_unread(ApiContext *ctx);      /* GET  /api/notifications/unread */
void pub_notif_read(ApiContext *ctx);        /* POST /api/notifications/read */
void pub_notif_read_all(ApiContext *ctx);    /* POST /api/notifications/read-all */

/* ═══════════════════════ 学校端（sch_） ═══════════════════════ */
/* 仪表盘/统计 */
void sch_dashboard(ApiContext *ctx);         /* GET  /api/school/dashboard */
void sch_stats(ApiContext *ctx);             /* GET  /api/school/stats */
void sch_stats_export(ApiContext *ctx);      /* GET  /api/school/stats/export */

/* 社团审批 */
void sch_club_pending(ApiContext *ctx);      /* GET  /api/school/clubs/pending */
void sch_club_approve(ApiContext *ctx);      /* POST /api/school/clubs/{id}/approve */
void sch_club_reject(ApiContext *ctx);       /* POST /api/school/clubs/{id}/reject */
void sch_club_approved_history(ApiContext *ctx); /* GET /api/school/clubs/history */

/* 社团管理 */
void sch_club_list(ApiContext *ctx);         /* GET  /api/school/clubs */
void sch_club_detail(ApiContext *ctx);       /* GET  /api/school/clubs/{id} */
void sch_club_set_level(ApiContext *ctx);    /* POST /api/school/clubs/{id}/level */
void sch_club_dissolve(ApiContext *ctx);     /* POST /api/school/clubs/{id}/dissolve */
void sch_club_rename(ApiContext *ctx);       /* POST /api/school/clubs/{id}/rename */

/* 用户管理 */
void sch_user_list(ApiContext *ctx);         /* GET  /api/school/users */
void sch_user_detail(ApiContext *ctx);       /* GET  /api/school/users/{id} */
void sch_user_create_college_admin(ApiContext *ctx); /* POST /api/school/users/college-admin */
void sch_user_toggle(ApiContext *ctx);       /* POST /api/school/users/{id}/toggle 禁用/启用 */
void sch_user_reset_pwd(ApiContext *ctx);    /* POST /api/school/users/{id}/reset-password */
void sch_user_restrict(ApiContext *ctx);     /* POST /api/school/users/{id}/restrict */
void sch_user_lift_restrict(ApiContext *ctx);/* POST /api/school/users/{id}/lift-restriction */
void sch_restriction_list(ApiContext *ctx);  /* GET  /api/school/restrictions */

/* 财务监督 */
void sch_finance_overview(ApiContext *ctx);  /* GET  /api/school/finance */
void sch_finance_club(ApiContext *ctx);      /* GET  /api/school/finance/{club_id} */
void sch_reimb_pending(ApiContext *ctx);     /* GET  /api/school/reimbursements/pending */
void sch_reimb_approve(ApiContext *ctx);     /* POST /api/school/reimbursements/{id}/approve */
void sch_reimb_reject(ApiContext *ctx);      /* POST /api/school/reimbursements/{id}/reject */
void sch_reimb_college_list(ApiContext *ctx);/* GET  /api/school/reimbursements/college */

/* 公告 */
void sch_announce_list(ApiContext *ctx);     /* GET    /api/school/announcements */
void sch_announce_post(ApiContext *ctx);     /* POST   /api/school/announcements */
void sch_announce_delete(ApiContext *ctx);   /* DELETE /api/school/announcements/{id} */

/* 系统与备份 */
void sch_backup_create(ApiContext *ctx);     /* POST /api/school/backup */
void sch_backup_list(ApiContext *ctx);       /* GET  /api/school/backups */
void sch_config_get(ApiContext *ctx);        /* GET  /api/school/config */
void sch_config_set(ApiContext *ctx);        /* POST /api/school/config */

/* ═══════════════════════ 学院端（col_） ═══════════════════════ */
void col_dashboard(ApiContext *ctx);         /* GET  /api/college/dashboard */
void col_stats(ApiContext *ctx);             /* GET  /api/college/stats */
void col_stats_export(ApiContext *ctx);      /* GET  /api/college/stats/export */

/* 班级管理 */
void col_class_list(ApiContext *ctx);        /* GET    /api/college/classes?major_id= */
void col_class_create(ApiContext *ctx);      /* POST   /api/college/classes */
void col_class_update(ApiContext *ctx);      /* PUT    /api/college/classes/{id} */
void col_class_delete(ApiContext *ctx);      /* DELETE /api/college/classes/{id} */

/* 本院社团监督 */
void col_club_list(ApiContext *ctx);         /* GET  /api/college/clubs */
void col_club_detail(ApiContext *ctx);       /* GET  /api/college/clubs/{id} */

/* 报销审批 + 额度 */
void col_reimb_pending(ApiContext *ctx);     /* GET  /api/college/reimbursements/pending */
void col_reimb_approve(ApiContext *ctx);     /* POST /api/college/reimbursements/{id}/approve */
void col_reimb_reject(ApiContext *ctx);      /* POST /api/college/reimbursements/{id}/reject */
void col_limit_list(ApiContext *ctx);        /* GET  /api/college/limits */
void col_limit_set(ApiContext *ctx);         /* POST /api/college/limits/{club_id} */

/* 本院学生管理 */
void col_student_list(ApiContext *ctx);      /* GET  /api/college/students */
void col_student_restrict(ApiContext *ctx);  /* POST /api/college/students/{id}/restrict */
void col_student_lift(ApiContext *ctx);      /* POST /api/college/students/{id}/lift-restriction */

/* 学院公告 */
void col_announce_list(ApiContext *ctx);     /* GET  /api/college/announcements */
void col_announce_post(ApiContext *ctx);     /* POST /api/college/announcements */
void col_announce_delete(ApiContext *ctx);   /* DELETE /api/college/announcements/{id} */

/* ═══════════════════════ 学生端（stu_） ═══════════════════════ */
/* 社团广场 */
void stu_club_browse(ApiContext *ctx);       /* GET  /api/clubs 浏览/搜索/筛选 */
void stu_club_detail(ApiContext *ctx);       /* GET  /api/clubs/{id} 社团详情 */
void stu_club_members(ApiContext *ctx);      /* GET  /api/clubs/{id}/members */
void stu_club_activities(ApiContext *ctx);   /* GET  /api/clubs/{id}/activities */

/* 申请创建/加入/退出社团 */
void stu_club_apply(ApiContext *ctx);        /* POST /api/clubs/apply 申请创建社团 */
void stu_club_join(ApiContext *ctx);         /* POST /api/clubs/{id}/join 申请加入 */
void stu_club_leave(ApiContext *ctx);        /* POST /api/clubs/{id}/leave 退出 */

/* 活动 */
void stu_activity_list(ApiContext *ctx);     /* GET  /api/activities */
void stu_activity_detail(ApiContext *ctx);   /* GET  /api/activities/{id} */
void stu_activity_registrations(ApiContext *ctx); /* GET /api/activities/{id}/registrations */
void stu_activity_register(ApiContext *ctx); /* POST /api/activities/{id}/register */
void stu_activity_cancel(ApiContext *ctx);   /* POST /api/activities/{id}/cancel */
void stu_activity_signin(ApiContext *ctx);   /* POST /api/activities/{id}/signin 输入签到码 */

/* 个人中心 */
void stu_profile_get(ApiContext *ctx);       /* GET  /api/profile */
void stu_profile_update(ApiContext *ctx);    /* PUT  /api/profile */
void stu_change_password(ApiContext *ctx);   /* POST /api/profile/password */

/* 我的社团 */
void stu_my_clubs(ApiContext *ctx);          /* GET  /api/my/clubs */
void stu_my_applications(ApiContext *ctx);   /* GET  /api/my/applications 入社+创建申请记录 */

/* ═══════════════════════ 社团管理端（club_） ═══════════════════════ */
/* 社团信息 */
void club_profile_get(ApiContext *ctx);      /* GET  /api/club/{id}/profile */
void club_profile_update(ApiContext *ctx);   /* PUT  /api/club/{id}/profile */
void club_dashboard(ApiContext *ctx);        /* GET  /api/club/{id}/dashboard */

/* 成员管理 */
void club_member_list(ApiContext *ctx);      /* GET  /api/club/{id}/members */
void club_join_pending(ApiContext *ctx);     /* GET  /api/club/{id}/join-requests */
void club_join_approve(ApiContext *ctx);     /* POST /api/club/{id}/join-requests/{mid}/approve */
void club_join_reject(ApiContext *ctx);      /* POST /api/club/{id}/join-requests/{mid}/reject */
void club_member_remove(ApiContext *ctx);    /* POST /api/club/{id}/members/{mid}/remove */
void club_member_appoint(ApiContext *ctx);   /* POST /api/club/{id}/members/{mid}/appoint 任命角色 */

/* 活动管理 */
void club_activity_list(ApiContext *ctx);    /* GET    /api/club/{id}/activities */
void club_activity_create(ApiContext *ctx);  /* POST   /api/club/{id}/activities */
void club_activity_update(ApiContext *ctx);  /* PUT    /api/club/{id}/activities/{aid} */
void club_activity_publish(ApiContext *ctx); /* POST   /api/club/{id}/activities/{aid}/publish */
void club_activity_start(ApiContext *ctx);   /* POST   /api/club/{id}/activities/{aid}/start */
void club_activity_finish(ApiContext *ctx);  /* POST   /api/club/{id}/activities/{aid}/finish */
void club_activity_cancel(ApiContext *ctx);  /* POST   /api/club/{id}/activities/{aid}/cancel */
void club_activity_summary(ApiContext *ctx); /* POST   /api/club/{id}/activities/{aid}/summary */
void club_activity_delete(ApiContext *ctx);  /* DELETE /api/club/{id}/activities/{aid} */

/* 签到管理 */
void club_signin_list(ApiContext *ctx);      /* GET  /api/club/{id}/activities/{aid}/signins */
void club_signin_manual(ApiContext *ctx);    /* POST /api/club/{id}/activities/{aid}/manual-signin */

/* 财务 */
void club_finance_list(ApiContext *ctx);     /* GET  /api/club/{id}/finance */
void club_finance_create(ApiContext *ctx);   /* POST /api/club/{id}/finance */
void club_finance_delete(ApiContext *ctx);   /* DELETE /api/club/{id}/finance/{fid} */
void club_reimb_list(ApiContext *ctx);       /* GET  /api/club/{id}/reimbursements */
void club_reimb_create(ApiContext *ctx);     /* POST /api/club/{id}/reimbursements（含发票上传） */
void club_reimb_upload(ApiContext *ctx);     /* POST /api/club/{id}/upload-receipt（发票上传） */

/* 换届选举 */
void club_election_list(ApiContext *ctx);    /* GET  /api/club/{id}/elections */
void club_election_create(ApiContext *ctx);  /* POST /api/club/{id}/elections 发起选举 */
void club_election_detail(ApiContext *ctx);  /* GET  /api/club/{id}/elections/{eid} */
void club_election_reelect(ApiContext *ctx); /* POST /api/club/{id}/elections/reelect 申请连任 */
void club_election_signup(ApiContext *ctx);  /* POST /api/club/{id}/elections/{eid}/signup 报名参选 */
void club_election_candidates(ApiContext *ctx);/* GET /api/club/{id}/elections/{eid}/candidates */
void club_election_vote(ApiContext *ctx);    /* POST /api/club/{id}/elections/{eid}/vote 匿名投票 */
void club_election_result(ApiContext *ctx);  /* GET  /api/club/{id}/elections/{eid}/result */

/* 社团通知 */
void club_announce_post(ApiContext *ctx);    /* POST /api/club/{id}/announcements */

#endif /* ROUTES_H */
