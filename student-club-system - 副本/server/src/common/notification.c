/**
 * notification.c — 消息通知服务实现
 * 学生社团管理系统 SCMS
 *
 * 所有通知最终都写入 notifications 表（一条记录对应一个接收者）。
 * 广播类函数使用一条 INSERT ... SELECT 批量插入，避免逐条循环插入。
 *
 * 提供的函数：
 *   - notification_send                 单发通知
 *   - notification_broadcast_school     全校广播
 *   - notification_broadcast_college    学院广播
 *   - notification_broadcast_club       社团广播
 *   - notification_send_president       定向社长
 *   - notification_notify_registrants   活动报名者批量通知
 *   - notification_notify_school_admins 通知全体学校管理员
 *   - notification_notify_college_admins 通知指定学院的学院管理员
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../../include/common.h"
#include "../../include/db.h"
#include "../../include/notification.h"

/**
 * notification_send — 向单个用户发送通知
 * @user_id:    接收者用户 ID
 * @title:      通知标题
 * @content:    通知正文
 * @type:       通知类型（如 "announcement"、"approval" 等）
 * @related_id: 关联的业务对象 ID（如社团 ID、活动 ID），<=0 表示无关联
 *
 * user_id <= 0 时直接返回，不插入
 */
/* ── 单发 ── */
void notification_send(int user_id, const char *title, const char *content,
                       const char *type, int related_id) {
    if (user_id <= 0) return;

    /* 对传入的字符串做 SQL 转义，防止注入 */
    char *t = db_escape(title ? title : "");
    char *c = db_escape(content ? content : "");
    char *ty = db_escape(type ? type : "");

    if (related_id > 0) {
        db_execute(
            "INSERT INTO notifications (user_id, title, content, type, related_id) "
            "VALUES (%d, '%s', '%s', '%s', %d)",
            user_id, t, c, ty, related_id);
    } else {
        db_execute(
            "INSERT INTO notifications (user_id, title, content, type) "
            "VALUES (%d, '%s', '%s', '%s')",
            user_id, t, c, ty);
    }

    free(t); free(c); free(ty);
}

/**
 * notification_broadcast_school — 向全校所有状态正常的用户发送通知
 * @title:        通知标题
 * @content:      通知正文
 * @publisher_id: 发布者用户 ID（当前未使用，保留供扩展）
 *
 * 通过 INSERT ... SELECT 从 users 表批量插入，一次 SQL 完成
 */
/* ── 全校广播：给 users 表中所有状态正常的用户各插一条 ── */
void notification_broadcast_school(const char *title, const char *content,
                                   int publisher_id) {
    (void)publisher_id;
    char *t = db_escape(title ? title : "");
    char *c = db_escape(content ? content : "");

    db_execute(
        "INSERT INTO notifications (user_id, title, content, type) "
        "SELECT user_id, '%s', '%s', 'announcement' FROM users WHERE status=1",
        t, c);

    free(t); free(c);
}

/**
 * notification_broadcast_college — 向指定学院及以下级别用户发送通知
 * @college_id:   目标学院 ID
 * @title:        通知标题
 * @content:      通知正文
 * @publisher_id: 发布者用户 ID（当前未使用，保留供扩展）
 *
 * 接收范围：该学院内 role <= college_admin 的用户
 * （即 college_admin、club_admin、club_member、general_student）
 * 学校管理员不接收学院级公告
 *
 * college_id <= 0 时直接返回
 */
/* ── 学院广播：给该学院及以下级别用户各插一条 ── */
void notification_broadcast_college(int college_id, const char *title,
                                    const char *content, int publisher_id) {
    (void)publisher_id;
    if (college_id <= 0) return;

    char *t = db_escape(title ? title : "");
    char *c = db_escape(content ? content : "");

    db_execute(
        "INSERT INTO notifications (user_id, title, content, type) "
        "SELECT user_id, '%s', '%s', 'announcement' FROM users "
        "WHERE status=1 AND college_id=%d "
        "AND role IN ('college_admin','club_admin','club_member','general_student')",
        t, c, college_id);

    free(t); free(c);
}

/**
 * notification_broadcast_club — 向指定社团所有在籍成员发送通知
 * @club_id:    目标社团 ID
 * @title:      通知标题
 * @content:    通知正文
 * @type:       通知类型，默认 "announcement"
 * @related_id: 关联的业务对象 ID，<=0 表示无关联
 *
 * club_id <= 0 时直接返回。
 * 通过 INSERT ... SELECT 从 members 表批量选取在籍成员。
 */
/* ── 社团广播：给该社团所有在籍成员各插一条 ── */
void notification_broadcast_club(int club_id, const char *title,
                                 const char *content, const char *type,
                                 int related_id) {
    if (club_id <= 0) return;

    char *t = db_escape(title ? title : "");
    char *c = db_escape(content ? content : "");
    char *ty = db_escape(type ? type : "announcement");

    if (related_id > 0) {
        db_execute(
            "INSERT INTO notifications (user_id, title, content, type, related_id) "
            "SELECT user_id, '%s', '%s', '%s', %d FROM members "
            "WHERE club_id=%d AND join_status='approved' AND left_at IS NULL",
            t, c, ty, related_id, club_id);
    } else {
        db_execute(
            "INSERT INTO notifications (user_id, title, content, type) "
            "SELECT user_id, '%s', '%s', '%s' FROM members "
            "WHERE club_id=%d AND join_status='approved' AND left_at IS NULL",
            t, c, ty, club_id);
    }

    free(t); free(c); free(ty);
}

/**
 * notification_send_president — 向指定社团的社长发送通知
 * @club_id:    目标社团 ID
 * @title:      通知标题
 * @content:    通知正文
 * @type:       通知类型
 * @related_id: 关联业务对象 ID
 *
 * 先从 clubs 表查询 president_id，再调用 notification_send 单发。
 * club_id <= 0 或社团无社长时直接返回。
 */
/* ── 定向社长 ── */
void notification_send_president(int club_id, const char *title,
                                 const char *content, const char *type,
                                 int related_id) {
    if (club_id <= 0) return;

    int president_id = db_query_int(
        "SELECT president_id FROM clubs WHERE club_id=%d", club_id);
    if (president_id <= 0) return;

    notification_send(president_id, title, content, type, related_id);
}

/**
 * notification_notify_registrants — 向指定活动的所有报名者发送通知
 * @activity_id: 目标活动 ID
 * @title:       通知标题
 * @content:     通知正文
 * @type:        通知类型，默认 "activity_cancelled"
 *
 * 通过 INSERT ... SELECT 从 registrations 表批量选取已报名的用户。
 * activity_id <= 0 时直接返回。
 */
/* ── 活动报名者批量通知 ── */
void notification_notify_registrants(int activity_id, const char *title,
                                     const char *content, const char *type) {
    if (activity_id <= 0) return;

    char *t = db_escape(title ? title : "");
    char *c = db_escape(content ? content : "");
    char *ty = db_escape(type ? type : "activity_cancelled");

    db_execute(
        "INSERT INTO notifications (user_id, title, content, type, related_id) "
        "SELECT user_id, '%s', '%s', '%s', %d FROM registrations "
        "WHERE activity_id=%d AND status='registered'",
        t, c, ty, activity_id, activity_id);

    free(t); free(c); free(ty);
}

/**
 * notification_notify_school_admins — 向全体学校管理员发送通知
 * @title:      通知标题
 * @content:    通知正文
 * @type:       通知类型，默认 "system"
 * @related_id: 关联业务对象 ID
 *
 * 通过 INSERT ... SELECT 从 users 表选取 role='school_admin' 且状态正常的用户
 */
/* ── 通知全体学校管理员 ── */
void notification_notify_school_admins(const char *title, const char *content,
                                       const char *type, int related_id) {
    char *t = db_escape(title ? title : "");
    char *c = db_escape(content ? content : "");
    char *ty = db_escape(type ? type : "system");

    db_execute(
        "INSERT INTO notifications (user_id, title, content, type, related_id) "
        "SELECT user_id, '%s', '%s', '%s', %d FROM users "
        "WHERE status=1 AND role='" ROLE_SCHOOL "'",
        t, c, ty, related_id);

    free(t); free(c); free(ty);
}

/**
 * notification_notify_college_admins — 向指定学院的所有学院管理员发送通知
 * @college_id: 目标学院 ID
 * @title:      通知标题
 * @content:    通知正文
 * @type:       通知类型，默认 "system"
 * @related_id: 关联业务对象 ID
 *
 * college_id <= 0 时直接返回。
 * 通过 INSERT ... SELECT 从 users 表选取 role='college_admin' 且状态正常的用户
 */
/* ── 通知指定学院的全体学院管理员 ── */
void notification_notify_college_admins(int college_id, const char *title,
                                        const char *content, const char *type,
                                        int related_id) {
    if (college_id <= 0) return;

    char *t = db_escape(title ? title : "");
    char *c = db_escape(content ? content : "");
    char *ty = db_escape(type ? type : "system");

    db_execute(
        "INSERT INTO notifications (user_id, title, content, type, related_id) "
        "SELECT user_id, '%s', '%s', '%s', %d FROM users "
        "WHERE status=1 AND role='" ROLE_COLLEGE "' AND college_id=%d",
        t, c, ty, related_id, college_id);

    free(t); free(c); free(ty);
}
