/**
 * notification.c — 消息通知服务实现
 * 学生社团管理系统 SCMS
 *
 * 所有通知最终都落到 notifications 表（一条 = 一个接收者）。
 * 广播类函数用一条 INSERT ... SELECT 批量插入，避免循环单插。
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../../include/common.h"
#include "../../include/db.h"
#include "../../include/notification.h"

/* ── 单发 ── */
void notification_send(int user_id, const char *title, const char *content,
                       const char *type, int related_id) {
    if (user_id <= 0) return;

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

/* ── 学院广播：给该学院所有状态正常的用户各插一条 ── */
void notification_broadcast_college(int college_id, const char *title,
                                    const char *content, int publisher_id) {
    (void)publisher_id;
    if (college_id <= 0) return;

    char *t = db_escape(title ? title : "");
    char *c = db_escape(content ? content : "");

    db_execute(
        "INSERT INTO notifications (user_id, title, content, type) "
        "SELECT user_id, '%s', '%s', 'announcement' FROM users "
        "WHERE status=1 AND college_id=%d",
        t, c, college_id);

    free(t); free(c);
}

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
