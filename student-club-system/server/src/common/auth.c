/**
 * auth.c — 认证与会话管理实现
 * 学生社团管理系统 SCMS（Web 架构）
 *
 * 会话采用内存哈希表（简单线性数组 + 互斥锁）保存 token→user_id 映射。
 * 单机部署、用户量不大，够用；如需分布式可换 Redis，接口不变。
 */
#include "../../include/auth.h"
#include "../../include/db.h"
#include "../../include/common.h"
#include "../../include/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
static CRITICAL_SECTION g_sess_lock;
#define LOCK_INIT()    InitializeCriticalSection(&g_sess_lock)
#define LOCK()         EnterCriticalSection(&g_sess_lock)
#define UNLOCK()       LeaveCriticalSection(&g_sess_lock)
#define LOCK_DESTROY() DeleteCriticalSection(&g_sess_lock)
#else
#include <pthread.h>
static pthread_mutex_t g_sess_lock = PTHREAD_MUTEX_INITIALIZER;
#define LOCK_INIT()    ((void)0)
#define LOCK()         pthread_mutex_lock(&g_sess_lock)
#define UNLOCK()       pthread_mutex_unlock(&g_sess_lock)
#define LOCK_DESTROY() ((void)0)
#endif

/* 会话表 */
#define MAX_SESSIONS 1024

typedef struct {
    int  in_use;
    char token[SESSION_TOKEN_LEN + 1];
    int  user_id;
    time_t last_access;
} Session;

static Session g_sessions[MAX_SESSIONS];

void auth_init(void) {
    LOCK_INIT();
    memset(g_sessions, 0, sizeof(g_sessions));
    srand((unsigned)time(NULL));
}

void auth_cleanup(void) {
    LOCK_DESTROY();
}

/* ── 从一行结果填充 User 结构 ──
 * 列顺序必须与下方 SELECT 完全一致。
 */
static void fill_user_from_row(User *u, MYSQL_ROW row) {
    memset(u, 0, sizeof(*u));
    u->user_id = row[0] ? atoi(row[0]) : 0;
    utils_strlcpy(u->username,  utils_safe(row[1]),  sizeof(u->username));
    utils_strlcpy(u->password,  utils_safe(row[2]),  sizeof(u->password));
    utils_strlcpy(u->real_name, utils_safe(row[3]),  sizeof(u->real_name));
    utils_strlcpy(u->role,      utils_safe(row[4]),  sizeof(u->role));
    u->college_id = row[5] ? atoi(row[5]) : 0;
    u->major_id   = row[6] ? atoi(row[6]) : 0;
    u->class_id   = row[7] ? atoi(row[7]) : 0;
    utils_strlcpy(u->student_no, utils_safe(row[8]),  sizeof(u->student_no));
    utils_strlcpy(u->id_card,    utils_safe(row[9]),  sizeof(u->id_card));
    utils_strlcpy(u->phone,      utils_safe(row[10]), sizeof(u->phone));
    utils_strlcpy(u->email,      utils_safe(row[11]), sizeof(u->email));
    u->status = row[12] ? atoi(row[12]) : 0;
    utils_strlcpy(u->last_login, utils_safe(row[13]), sizeof(u->last_login));
    utils_strlcpy(u->created_at, utils_safe(row[14]), sizeof(u->created_at));
    utils_strlcpy(u->updated_at, utils_safe(row[15]), sizeof(u->updated_at));
    /* JOIN 展示字段 */
    utils_strlcpy(u->college_name, utils_safe(row[16]), sizeof(u->college_name));
    utils_strlcpy(u->major_name,   utils_safe(row[17]), sizeof(u->major_name));
    utils_strlcpy(u->class_name,   utils_safe(row[18]), sizeof(u->class_name));
}

/* 统一的用户查询 SQL（列顺序固定，供 fill_user_from_row 使用） */
#define USER_SELECT_COLS \
    "u.user_id,u.username,u.password,u.real_name,u.role," \
    "u.college_id,u.major_id,u.class_id,u.student_no,u.id_card," \
    "u.phone,u.email,u.status,u.last_login,u.created_at,u.updated_at," \
    "co.college_name,m.major_name,c.class_name"

#define USER_SELECT_FROM \
    "FROM users u " \
    "LEFT JOIN colleges co ON u.college_id=co.college_id " \
    "LEFT JOIN majors   m  ON u.major_id=m.major_id " \
    "LEFT JOIN classes  c  ON u.class_id=c.class_id "

int auth_load_user(int user_id, User *out_user) {
    MYSQL_RES *res = db_query(
        "SELECT " USER_SELECT_COLS " " USER_SELECT_FROM
        "WHERE u.user_id=%d", user_id);
    if (!res) return ERR_DB;
    MYSQL_ROW row = mysql_fetch_row(res);
    if (!row) { mysql_free_result(res); return ERR_NOT_FOUND; }
    fill_user_from_row(out_user, row);
    mysql_free_result(res);
    return OK;
}

int auth_login(const char *username, const char *password,
               User *out_user, char *out_token) {
    if (utils_is_empty(username) || utils_is_empty(password))
        return ERR_INPUT;

    char *esc = db_escape(username);
    if (!esc) return ERR_SYSTEM;

    MYSQL_RES *res = db_query(
        "SELECT " USER_SELECT_COLS " " USER_SELECT_FROM
        "WHERE u.username='%s'", esc);
    free(esc);
    if (!res) return ERR_DB;

    MYSQL_ROW row = mysql_fetch_row(res);
    if (!row) { mysql_free_result(res); return ERR_AUTH; }

    User u;
    fill_user_from_row(&u, row);
    mysql_free_result(res);

    /* 校验密码 */
    if (!utils_password_verify(password, u.password))
        return ERR_AUTH;

    /* 账号禁用 */
    if (u.status != 1)
        return ERR_STATUS;

    /* 更新最后登录时间 */
    db_execute("UPDATE users SET last_login=NOW() WHERE user_id=%d", u.user_id);

    /* 记录登录日志 */
    db_execute("INSERT INTO logs (user_id, action, target_type, target_id, detail) "
               "VALUES (%d, 'login', 'users', %d, '用户登录')",
               u.user_id, u.user_id);

    /* 生成会话 token */
    char token[SESSION_TOKEN_LEN + 1];
    utils_gen_random_string(token, SESSION_TOKEN_LEN);
    token[SESSION_TOKEN_LEN] = '\0';

    LOCK();
    int slot = -1;
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (!g_sessions[i].in_use) { slot = i; break; }
    }
    if (slot < 0) {
        /* 会话池满，回收最久未访问的槽位 */
        time_t oldest = g_sessions[0].last_access;
        slot = 0;
        for (int i = 1; i < MAX_SESSIONS; i++) {
            if (g_sessions[i].last_access < oldest) {
                oldest = g_sessions[i].last_access;
                slot = i;
            }
        }
    }
    g_sessions[slot].in_use = 1;
    utils_strlcpy(g_sessions[slot].token, token, sizeof(g_sessions[slot].token));
    g_sessions[slot].user_id = u.user_id;
    g_sessions[slot].last_access = time(NULL);
    UNLOCK();

    *out_user = u;
    utils_strlcpy(out_token, token, SESSION_TOKEN_LEN + 1);
    return OK;
}

void auth_logout(const char *token) {
    if (!token || !*token) return;
    LOCK();
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (g_sessions[i].in_use &&
            strcmp(g_sessions[i].token, token) == 0) {
            g_sessions[i].in_use = 0;
            g_sessions[i].token[0] = '\0';
            g_sessions[i].user_id = 0;
            break;
        }
    }
    UNLOCK();
}

int auth_resolve_session(const char *token, User *out_user) {
    if (!token || !*token) return 0;

    int uid = -1;
    LOCK();
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (g_sessions[i].in_use &&
            strcmp(g_sessions[i].token, token) == 0) {
            uid = g_sessions[i].user_id;
            g_sessions[i].last_access = time(NULL);
            break;
        }
    }
    UNLOCK();

    if (uid < 0) return 0;
    /* 每次都从库里加载，保证角色/状态实时（如被禁用/角色变更立即生效） */
    if (auth_load_user(uid, out_user) != OK) return 0;
    if (out_user->status != 1) return 0;   /* 会话期间被禁用 */
    return 1;
}
