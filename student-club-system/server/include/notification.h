/**
 * notification.h — 消息通知服务接口
 * 学生社团管理系统 SCMS
 *
 * 按"通知精准推送规则"（设计文档 5.8）向指定范围的用户投递站内消息。
 * 业务模块只需调用这里的函数，不用自己拼 INSERT。
 *
 * 通知类型（type 字段取值，见设计文档附录A）：
 *   club_apply_result / join_result / member_removed / role_change /
 *   term_downgrade / activity_cancelled / activity_signin_remind /
 *   election_start / election_result / election_warning /
 *   reimbursement_result / restriction_applied / restriction_lifted / announcement
 */
#ifndef NOTIFICATION_H
#define NOTIFICATION_H

/* 给单个用户发通知。related_id 为关联业务ID（无则传 0）。 */
void notification_send(int user_id, const char *title, const char *content,
                       const char *type, int related_id);

/* 广播给全校所有用户（scope=school 公告用）。 */
void notification_broadcast_school(const char *title, const char *content,
                                   int publisher_id);

/* 广播给某学院的所有学生（按 users.college_id 匹配）。 */
void notification_broadcast_college(int college_id, const char *title,
                                    const char *content, int publisher_id);

/* 广播给某社团的所有在籍成员（members 表 join_status=approved 且未退社）。 */
void notification_broadcast_club(int club_id, const char *title,
                                 const char *content, const char *type,
                                 int related_id);

/* 给某社团社长发通知（报销提交、警示处理等定向场景）。
 * 找不到社长时静默返回。 */
void notification_send_president(int club_id, const char *title,
                                 const char *content, const char *type,
                                 int related_id);

/* 通知某活动的所有已报名用户（活动取消、签到提醒等）。 */
void notification_notify_registrants(int activity_id, const char *title,
                                     const char *content, const char *type);

/* 通知全体学校管理员（建社申请、换届警示等需学校处理的事项）。 */
void notification_notify_school_admins(const char *title, const char *content,
                                       const char *type, int related_id);

/* 通知指定学院的全体学院管理员（报销申请等院级审批事项）。 */
void notification_notify_college_admins(int college_id, const char *title,
                                        const char *content, const char *type,
                                        int related_id);

#endif /* NOTIFICATION_H */
