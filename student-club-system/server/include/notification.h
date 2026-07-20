/**
 * notification.h — 消息通知服务接口
 * 学生社团管理系统 SCMS
 *
 * 按照"通知精准推送规则"（设计文档第 5.8 节）向指定范围的用户投递站内消息。
 * 业务模块只需调用本文件的函数，无需自行拼接 INSERT 语句插入通知记录。
 *
 * 通知类型（type 字段取值，详见设计文档附录 A）：
 *   club_apply_result      — 社团申请结果通知
 *   join_result            — 入社申请结果通知
 *   member_removed         — 成员被移出社团通知
 *   role_change            — 社团内角色变更通知
 *   term_downgrade         — 任期降级通知
 *   activity_cancelled     — 活动取消通知
 *   activity_signin_remind — 活动签到提醒
 *   election_start         — 选举开始通知
 *   election_result        — 选举结果通知
 *   election_warning       — 选举警示通知
 *   reimbursement_result   — 报销审批结果通知
 *   restriction_applied    — 限制已生效通知
 *   restriction_lifted     — 限制已解除通知
 *   announcement           — 公告通知
 */
#ifndef NOTIFICATION_H
#define NOTIFICATION_H

/* ── 单用户通知 ── */

/** 给单个用户发送通知
 *  @param user_id    接收用户ID
 *  @param title      通知标题
 *  @param content    通知内容
 *  @param type       通知类型（如 "reimbursement_result"）
 *  @param related_id 关联业务ID（无关联时传 0） */
void notification_send(int user_id, const char *title, const char *content,
                       const char *type, int related_id);

/* ── 广播通知 ── */

/** 向全校所有用户广播通知（校级公告用）
 *  @param title        通知标题
 *  @param content      通知内容
 *  @param publisher_id 发布人用户ID */
void notification_broadcast_school(const char *title, const char *content,
                                   int publisher_id);

/** 向指定学院的所有学生广播通知
 *  @param college_id   目标学院ID
 *  @param title        通知标题
 *  @param content      通知内容
 *  @param publisher_id 发布人用户ID */
void notification_broadcast_college(int college_id, const char *title,
                                    const char *content, int publisher_id);

/** 向指定社团的所有在籍成员广播通知
 *  （仅限 join_status=approved 且未退社的成员）
 *  @param club_id     目标社团ID
 *  @param title       通知标题
 *  @param content     通知内容
 *  @param type        通知类型
 *  @param related_id  关联业务ID */
void notification_broadcast_club(int club_id, const char *title,
                                 const char *content, const char *type,
                                 int related_id);

/* ── 定向通知 ── */

/** 向指定社团的社长发通知（报销提交、警示处理等定向场景）
 *  找不到社长时静默返回，不报错。
 *  @param club_id     目标社团ID
 *  @param title       通知标题
 *  @param content     通知内容
 *  @param type        通知类型
 *  @param related_id  关联业务ID */
void notification_send_president(int club_id, const char *title,
                                 const char *content, const char *type,
                                 int related_id);

/** 向指定活动的所有已报名用户发通知（活动取消、签到提醒等场景）
 *  @param activity_id 目标活动ID
 *  @param title       通知标题
 *  @param content     通知内容
 *  @param type        通知类型 */
void notification_notify_registrants(int activity_id, const char *title,
                                     const char *content, const char *type);

/** 通知全体学校管理员（建社申请、换届警示等需学校处理的事项）
 *  @param title       通知标题
 *  @param content     通知内容
 *  @param type        通知类型
 *  @param related_id  关联业务ID */
void notification_notify_school_admins(const char *title, const char *content,
                                       const char *type, int related_id);

/** 通知指定学院的全体学院管理员（报销申请等院级审批事项）
 *  @param college_id  目标学院ID
 *  @param title       通知标题
 *  @param content     通知内容
 *  @param type        通知类型
 *  @param related_id  关联业务ID */
void notification_notify_college_admins(int college_id, const char *title,
                                        const char *content, const char *type,
                                        int related_id);

#endif /* NOTIFICATION_H */
