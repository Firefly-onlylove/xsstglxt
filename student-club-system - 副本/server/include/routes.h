/**
 * routes.h — 业务 API 处理函数原型声明与路由分发
 * 学生社团管理系统 SCMS（Web 架构）
 *
 * ┌────────────────────────────────────────────────────────────────┐
 * │  这里声明了全部业务 API 的处理函数原型。                          │
 * │  每个函数由业务模块开发者在对应的 .c 文件中实现，签名统一为：     │
 * │      void 处理函数名(ApiContext *ctx);                           │
 * │                                                                  │
 * │  main.c 中的路由表把 HTTP 请求（方法+路径）映射到这些函数。        │
 * │                                                                  │
 * │  当业务模块尚未编写时，由 handlers_stub.c 提供弱符号占位实现，     │
 * │  返回"功能尚未实现"。真实模块解除注释并编译后，强符号自动覆盖。    │
 * └────────────────────────────────────────────────────────────────┘
 *
 * 命名规则：<端>_<模块>_<动作>
 *   sch_  = 学校端（school）
 *   col_  = 学院端（college）
 *   stu_  = 学生端（student）
 *   club_ = 社团管理端（club_admin）
 *   pub_  = 公共接口（无需登录或所有角色可用）
 */
#ifndef ROUTES_H
#define ROUTES_H

#include "api.h"

/** API 处理函数指针类型 */
typedef void (*ApiHandler)(ApiContext *ctx);

/** 路由项：HTTP 方法 + URI 模式（支持 * 通配单个路径段） + 处理函数 + 是否需要登录 */
typedef struct {
    const char *method;     // HTTP 方法："GET"/"POST"/"PUT"/"DELETE"，NULL 表示匹配任意方法
    const char *pattern;    // URI 模式，如 "/api/school/clubs/star"，star 匹配单个路径段
    ApiHandler  handler;    // 对应的处理函数指针
    int         need_login; // 是否需要登录：1 为需要（main 会先校验 ctx->user），0 为不需要
} Route;

/** 获取路由表首地址及条目数
 *  @param out_count 输出参数，返回路由表条目总数
 *  @return 路由表数组的首地址（定义在 main.c 中） */
const Route *routes_table(int *out_count);

/* ═══════════════════════ 公共接口（pub_） ═══════════════════════ */

void pub_register(ApiContext *ctx);          // POST /api/register —— 用户注册
void pub_colleges_list(ApiContext *ctx);     // GET  /api/colleges —— 学院下拉列表
void pub_majors_list(ApiContext *ctx);       // GET  /api/majors?college_id= —— 专业下拉列表
void pub_classes_list(ApiContext *ctx);      // GET  /api/classes?major_id= —— 班级下拉列表

/* 通知中心（所有已登录角色） */
void pub_notif_list(ApiContext *ctx);        // GET  /api/notifications —— 通知列表
void pub_notif_unread(ApiContext *ctx);      // GET  /api/notifications/unread —— 未读通知数
void pub_notif_read(ApiContext *ctx);        // POST /api/notifications/read —— 标记单条已读
void pub_notif_read_all(ApiContext *ctx);    // POST /api/notifications/read-all —— 全部标记已读

/* ═══════════════════════ 学校端（sch_） ═══════════════════════ */

/* 仪表盘与统计 */
void sch_dashboard(ApiContext *ctx);         // GET  /api/school/dashboard —— 仪表盘数据
void sch_stats(ApiContext *ctx);             // GET  /api/school/stats —— 统计数据
void sch_stats_export(ApiContext *ctx);      // GET  /api/school/stats/export —— 导出统计

/* 社团审批 */
void sch_club_pending(ApiContext *ctx);      // GET  /api/school/clubs/pending —— 待审批社团列表
void sch_club_approve(ApiContext *ctx);      // POST /api/school/clubs/{id}/approve —— 批准社团
void sch_club_reject(ApiContext *ctx);       // POST /api/school/clubs/{id}/reject —— 驳回社团
void sch_club_approved_history(ApiContext *ctx); // GET /api/school/clubs/history —— 审批历史

/* 社团管理 */
void sch_club_list(ApiContext *ctx);         // GET  /api/school/clubs —— 社团列表
void sch_club_detail(ApiContext *ctx);       // GET  /api/school/clubs/{id} —— 社团详情
void sch_club_set_level(ApiContext *ctx);    // POST /api/school/clubs/{id}/level —— 设置社团级别
void sch_club_dissolve(ApiContext *ctx);     // POST /api/school/clubs/{id}/dissolve —— 解散社团
void sch_club_rename(ApiContext *ctx);       // POST /api/school/clubs/{id}/rename —— 社团更名

/* 用户管理 */
void sch_user_list(ApiContext *ctx);         // GET  /api/school/users —— 用户列表
void sch_user_detail(ApiContext *ctx);       // GET  /api/school/users/{id} —— 用户详情
void sch_user_create_college_admin(ApiContext *ctx); // POST /api/school/users/college-admin —— 创建学院管理员
void sch_user_toggle(ApiContext *ctx);       // POST /api/school/users/{id}/toggle —— 禁用/启用用户
void sch_user_set_role(ApiContext *ctx);     // POST /api/school/users/{id}/set-role —— 修改角色
void sch_user_reset_pwd(ApiContext *ctx);    // POST /api/school/users/{id}/reset-password —— 重置密码
void sch_user_restrict(ApiContext *ctx);     // POST /api/school/users/{id}/restrict —— 限制用户
void sch_user_lift_restrict(ApiContext *ctx);// POST /api/school/users/{id}/lift-restriction —— 解除限制
void sch_restriction_list(ApiContext *ctx);  // GET  /api/school/restrictions —— 限制记录列表

/* 财务监督 */
void sch_finance_overview(ApiContext *ctx);  // GET  /api/school/finance —— 全校财务概览
void sch_finance_club(ApiContext *ctx);      // GET  /api/school/finance/{club_id} —— 社团财务明细
void sch_finance_school_clubs(ApiContext *ctx); // GET /api/school/finance/school-clubs —— 校级社团经费明细
void sch_reimb_pending(ApiContext *ctx);     // GET  /api/school/reimbursements/pending —— 待审批报销列表
void sch_reimb_approve(ApiContext *ctx);     // POST /api/school/reimbursements/{id}/approve —— 批准报销
void sch_reimb_reject(ApiContext *ctx);      // POST /api/school/reimbursements/{id}/reject —— 驳回报销
void sch_reimb_college_list(ApiContext *ctx);// GET  /api/school/reimbursements/college —— 学院报销记录

/* 公告 */
void sch_announce_list(ApiContext *ctx);     // GET    /api/school/announcements —— 公告列表
void sch_announce_post(ApiContext *ctx);     // POST   /api/school/announcements —— 发布公告
void sch_announce_delete(ApiContext *ctx);   // DELETE /api/school/announcements/{id} —— 删除公告

/* 系统与备份 */
void sch_backup_create(ApiContext *ctx);     // POST /api/school/backup —— 创建数据库备份
void sch_backup_list(ApiContext *ctx);       // GET  /api/school/backups —— 备份列表
void sch_config_get(ApiContext *ctx);        // GET  /api/school/config —— 获取系统配置
void sch_config_set(ApiContext *ctx);        // POST /api/school/config —— 修改系统配置

/* ═══════════════════════ 学院端（col_） ═══════════════════════ */

void col_dashboard(ApiContext *ctx);         // GET  /api/college/dashboard —— 学院仪表盘
void col_stats(ApiContext *ctx);             // GET  /api/college/stats —— 学院统计
void col_stats_export(ApiContext *ctx);      // GET  /api/college/stats/export —— 导出学院统计

/* 班级管理 */
void col_class_list(ApiContext *ctx);        // GET    /api/college/classes?major_id= —— 班级列表
void col_class_create(ApiContext *ctx);      // POST   /api/college/classes —— 创建班级
void col_class_update(ApiContext *ctx);      // PUT    /api/college/classes/{id} —— 修改班级
void col_class_delete(ApiContext *ctx);      // DELETE /api/college/classes/{id} —— 删除班级

/* 本院社团监督 */
void col_club_list(ApiContext *ctx);         // GET  /api/college/clubs —— 本院社团列表
void col_club_detail(ApiContext *ctx);       // GET  /api/college/clubs/{id} —— 本院社团详情
void col_club_members(ApiContext *ctx);      // GET  /api/college/clubs/{id}/members —— 社团成员
void col_club_activities(ApiContext *ctx);   // GET  /api/college/clubs/{id}/activities —— 社团活动
void col_club_rename(ApiContext *ctx);       // POST /api/college/clubs/{id}/rename —— 重命名
void col_club_dissolve(ApiContext *ctx);     // POST /api/college/clubs/{id}/dissolve —— 解散

/* 报销审批与额度管理 */
void col_reimb_pending(ApiContext *ctx);     // GET  /api/college/reimbursements/pending —— 待审批报销
void col_reimb_approve(ApiContext *ctx);     // POST /api/college/reimbursements/{id}/approve —— 批准报销
void col_reimb_reject(ApiContext *ctx);      // POST /api/college/reimbursements/{id}/reject —— 驳回报销
void col_limit_list(ApiContext *ctx);        // GET  /api/college/limits —— 报销额度列表
void col_limit_set(ApiContext *ctx);         // POST /api/college/limits/{club_id} —— 设置报销额度

/* 本院学生管理 */
void col_student_list(ApiContext *ctx);      // GET  /api/college/students —— 学生列表
void col_student_restrict(ApiContext *ctx);  // POST /api/college/students/{id}/restrict —— 限制学生
void col_student_lift(ApiContext *ctx);      // POST /api/college/students/{id}/lift-restriction —— 解除限制

/* 学院公告 */
void col_announce_list(ApiContext *ctx);     // GET  /api/college/announcements —— 公告列表
void col_announce_post(ApiContext *ctx);     // POST /api/college/announcements —— 发布公告
void col_announce_delete(ApiContext *ctx);   // DELETE /api/college/announcements/{id} —— 删除公告

/* ═══════════════════════ 学生端（stu_） ═══════════════════════ */

/* 社团广场 */
void stu_club_browse(ApiContext *ctx);       // GET  /api/clubs —— 浏览/搜索/筛选社团
void stu_club_detail(ApiContext *ctx);       // GET  /api/clubs/{id} —— 社团详情
void stu_club_members(ApiContext *ctx);      // GET  /api/clubs/{id}/members —— 社团成员列表
void stu_club_activities(ApiContext *ctx);   // GET  /api/clubs/{id}/activities —— 社团活动列表

/* 申请创建/加入/退出社团 */
void stu_club_apply(ApiContext *ctx);        // POST /api/clubs/apply —— 申请创建社团
void stu_club_join(ApiContext *ctx);         // POST /api/clubs/{id}/join —— 申请加入社团
void stu_club_leave(ApiContext *ctx);        // POST /api/clubs/{id}/leave —— 退出社团

/* 活动 */
void stu_activity_list(ApiContext *ctx);     // GET  /api/activities —— 活动列表
void stu_activity_detail(ApiContext *ctx);   // GET  /api/activities/{id} —— 活动详情
void stu_activity_registrations(ApiContext *ctx); // GET /api/activities/{id}/registrations —— 活动报名列表
void stu_activity_register(ApiContext *ctx); // POST /api/activities/{id}/register —— 报名活动
void stu_activity_cancel(ApiContext *ctx);   // POST /api/activities/{id}/cancel —— 取消报名
void stu_activity_signin(ApiContext *ctx);   // POST /api/activities/{id}/signin —— 输入签到码签到

/* 个人中心 */
void stu_profile_get(ApiContext *ctx);       // GET  /api/profile —— 获取个人信息
void stu_profile_update(ApiContext *ctx);    // PUT  /api/profile —— 修改个人信息
void stu_change_password(ApiContext *ctx);   // POST /api/profile/password —— 修改密码

/* 我的社团 */
void stu_my_clubs(ApiContext *ctx);          // GET  /api/my/clubs —— 我加入的社团列表
void stu_my_applications(ApiContext *ctx);   // GET  /api/my/applications —— 我的入社和创建申请记录

/* ═══════════════════════ 社团管理端（club_） ═══════════════════════ */

/* 社团信息 */
void club_profile_get(ApiContext *ctx);      // GET  /api/club/{id}/profile —— 获取社团资料
void club_profile_update(ApiContext *ctx);   // PUT  /api/club/{id}/profile —— 修改社团资料
void club_dashboard(ApiContext *ctx);        // GET  /api/club/{id}/dashboard —— 社团仪表盘

/* 成员管理 */
void club_member_list(ApiContext *ctx);      // GET  /api/club/{id}/members —— 成员列表
void club_join_pending(ApiContext *ctx);     // GET  /api/club/{id}/join-requests —— 待审批入社申请
void club_join_approve(ApiContext *ctx);     // POST /api/club/{id}/join-requests/{mid}/approve —— 批准入社
void club_join_reject(ApiContext *ctx);      // POST /api/club/{id}/join-requests/{mid}/reject —— 拒绝入社
void club_member_remove(ApiContext *ctx);    // POST /api/club/{id}/members/{mid}/remove —— 移除成员
void club_member_appoint(ApiContext *ctx);   // POST /api/club/{id}/members/{mid}/appoint —— 任命角色

/* 活动管理 */
void club_activity_list(ApiContext *ctx);    // GET    /api/club/{id}/activities —— 活动列表
void club_activity_create(ApiContext *ctx);  // POST   /api/club/{id}/activities —— 创建活动
void club_activity_update(ApiContext *ctx);  // PUT    /api/club/{id}/activities/{aid} —— 修改活动
void club_activity_publish(ApiContext *ctx); // POST   /api/club/{id}/activities/{aid}/publish —— 发布活动
void club_activity_start(ApiContext *ctx);   // POST   /api/club/{id}/activities/{aid}/start —— 开始活动
void club_activity_finish(ApiContext *ctx);  // POST   /api/club/{id}/activities/{aid}/finish —— 结束活动
void club_activity_cancel(ApiContext *ctx);  // POST   /api/club/{id}/activities/{aid}/cancel —— 取消活动
void club_activity_summary(ApiContext *ctx); // POST   /api/club/{id}/activities/{aid}/summary —— 提交活动总结
void club_activity_delete(ApiContext *ctx);  // DELETE /api/club/{id}/activities/{aid} —— 删除活动

/* 签到管理 */
void club_signin_list(ApiContext *ctx);      // GET  /api/club/{id}/activities/{aid}/signins —— 签到记录列表
void club_signin_manual(ApiContext *ctx);    // POST /api/club/{id}/activities/{aid}/manual-signin —— 手动签到

/* 财务管理 */
void club_finance_list(ApiContext *ctx);     // GET  /api/club/{id}/finance —— 财务记录列表
void club_finance_create(ApiContext *ctx);   // POST /api/club/{id}/finance —— 添加财务记录
void club_finance_delete(ApiContext *ctx);   // DELETE /api/club/{id}/finance/{fid} —— 删除财务记录
void club_reimb_list(ApiContext *ctx);       // GET  /api/club/{id}/reimbursements —— 报销记录列表
void club_reimb_create(ApiContext *ctx);     // POST /api/club/{id}/reimbursements —— 提交报销申请（含发票上传）
void club_reimb_upload(ApiContext *ctx);     // POST /api/club/{id}/upload-receipt —— 上传发票图片

/* 换届选举 */
void club_election_list(ApiContext *ctx);    // GET  /api/club/{id}/elections —— 选举记录列表
void club_election_create(ApiContext *ctx);  // POST /api/club/{id}/elections —— 发起选举
void club_election_detail(ApiContext *ctx);  // GET  /api/club/{id}/elections/{eid} —— 选举详情
void club_election_reelect(ApiContext *ctx); // POST /api/club/{id}/elections/reelect —— 申请连任
void club_election_signup(ApiContext *ctx);  // POST /api/club/{id}/elections/{eid}/signup —— 报名参选
void club_election_candidates(ApiContext *ctx);// GET /api/club/{id}/elections/{eid}/candidates —— 候选人列表
void club_election_vote(ApiContext *ctx);    // POST /api/club/{id}/elections/{eid}/vote —— 匿名投票
void club_election_result(ApiContext *ctx);  // GET  /api/club/{id}/elections/{eid}/result —— 选举结果

/* 社团通知 */
void club_announce_post(ApiContext *ctx);    // POST /api/club/{id}/announcements —— 发布社团通知

#endif /* ROUTES_H */
