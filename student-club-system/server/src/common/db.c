/**
 * db.c — 数据库访问层实现
 * 学生社团管理系统 SCMS
 *
 * 封装 MySQL C API：
 *   - 简单连接池（互斥保护，多线程安全）
 *   - db_query / db_execute 支持 printf 风格格式化
 *   - 事务、转义、错误处理
 *
 * 线程模型：Mongoose 单线程事件循环调用业务函数，
 * 但为稳健起见连接池仍加锁，便于将来扩展多线程。
 */
#include "../../include/db.h"
#include "../../include/common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#ifdef _WIN32
#include <windows.h>
typedef CRITICAL_SECTION db_mutex_t;
#define MUTEX_INIT(m)    InitializeCriticalSection(m)
#define MUTEX_LOCK(m)    EnterCriticalSection(m)
#define MUTEX_UNLOCK(m)  LeaveCriticalSection(m)
#define MUTEX_DESTROY(m) DeleteCriticalSection(m)
#else
#include <pthread.h>
typedef pthread_mutex_t db_mutex_t;
#define MUTEX_INIT(m)    pthread_mutex_init(m, NULL)
#define MUTEX_LOCK(m)    pthread_mutex_lock(m)
#define MUTEX_UNLOCK(m)  pthread_mutex_unlock(m)
#define MUTEX_DESTROY(m) pthread_mutex_destroy(m)
#endif

#define MAX_POOL 16
#define SQL_BUF_SIZE 8192

/* 连接池槽位 */
typedef struct {
    MYSQL *conn;
    int    in_use;
} PoolSlot;

static struct {
    DBConfig  cfg;
    PoolSlot  slots[MAX_POOL];
    int       size;
    db_mutex_t lock;
    int       initialized;
    char      last_error[512];
} g_db = {0};

/* ── 建立单条连接 ── */
static MYSQL *create_connection(void) {
    MYSQL *conn = mysql_init(NULL);
    if (!conn) return NULL;

    /* 客户端字符集，防止中文乱码 */
    mysql_options(conn, MYSQL_SET_CHARSET_NAME, "utf8mb4");

    if (!mysql_real_connect(conn, g_db.cfg.host, g_db.cfg.user,
                            g_db.cfg.password, g_db.cfg.database,
                            (unsigned int)g_db.cfg.port, NULL, 0)) {
        snprintf(g_db.last_error, sizeof(g_db.last_error),
                 "连接失败: %s", mysql_error(conn));
        mysql_close(conn);
        return NULL;
    }
    mysql_set_character_set(conn, "utf8mb4");
    return conn;
}

int db_init(const DBConfig *cfg) {
    if (g_db.initialized) return OK;

    memcpy(&g_db.cfg, cfg, sizeof(DBConfig));
    if (g_db.cfg.pool_size <= 0) g_db.cfg.pool_size = 5;
    if (g_db.cfg.pool_size > MAX_POOL) g_db.cfg.pool_size = MAX_POOL;
    g_db.size = g_db.cfg.pool_size;

    MUTEX_INIT(&g_db.lock);

    if (mysql_library_init(0, NULL, NULL) != 0) {
        snprintf(g_db.last_error, sizeof(g_db.last_error),
                 "mysql_library_init 失败");
        return ERR_DB;
    }

    for (int i = 0; i < g_db.size; ++i) {
        g_db.slots[i].conn = create_connection();
        g_db.slots[i].in_use = 0;
        if (!g_db.slots[i].conn) {
            /* 有一条连不上就判定初始化失败 */
            for (int j = 0; j < i; ++j) {
                mysql_close(g_db.slots[j].conn);
                g_db.slots[j].conn = NULL;
            }
            return ERR_DB;
        }
    }

    g_db.initialized = 1;
    return OK;
}

void db_close(void) {
    if (!g_db.initialized) return;
    MUTEX_LOCK(&g_db.lock);
    for (int i = 0; i < g_db.size; ++i) {
        if (g_db.slots[i].conn) {
            mysql_close(g_db.slots[i].conn);
            g_db.slots[i].conn = NULL;
        }
    }
    MUTEX_UNLOCK(&g_db.lock);
    MUTEX_DESTROY(&g_db.lock);
    mysql_library_end();
    g_db.initialized = 0;
}

MYSQL *db_get_conn(void) {
    MYSQL *conn = NULL;
    MUTEX_LOCK(&g_db.lock);
    for (int i = 0; i < g_db.size; ++i) {
        if (!g_db.slots[i].in_use && g_db.slots[i].conn) {
            g_db.slots[i].in_use = 1;
            conn = g_db.slots[i].conn;
            break;
        }
    }
    MUTEX_UNLOCK(&g_db.lock);

    /* 池满则临时新建一条（用完直接关闭，不入池） */
    if (!conn) conn = create_connection();
    return conn;
}

void db_release_conn(MYSQL *conn) {
    if (!conn) return;
    MUTEX_LOCK(&g_db.lock);
    for (int i = 0; i < g_db.size; ++i) {
        if (g_db.slots[i].conn == conn) {
            g_db.slots[i].in_use = 0;
            MUTEX_UNLOCK(&g_db.lock);
            return;
        }
    }
    MUTEX_UNLOCK(&g_db.lock);
    /* 不在池中，是临时连接，关闭它 */
    mysql_close(conn);
}

/* ── 内部：格式化 SQL 到缓冲区 ── */
static int format_sql(char *buf, size_t size, const char *fmt, va_list ap) {
    int n = vsnprintf(buf, size, fmt, ap);
    if (n < 0 || (size_t)n >= size) {
        snprintf(g_db.last_error, sizeof(g_db.last_error),
                 "SQL 语句过长（超过 %d 字节）", (int)size);
        return -1;
    }
    return n;
}

int db_execute(const char *fmt, ...) {
    char sql[SQL_BUF_SIZE];
    va_list ap;
    va_start(ap, fmt);
    int n = format_sql(sql, sizeof(sql), fmt, ap);
    va_end(ap);
    if (n < 0) return -1;

    MYSQL *conn = db_get_conn();
    if (!conn) return -1;

    int affected = -1;
    if (mysql_query(conn, sql) == 0) {
        affected = (int)mysql_affected_rows(conn);
    } else {
        snprintf(g_db.last_error, sizeof(g_db.last_error),
                 "%s", mysql_error(conn));
    }
    db_release_conn(conn);
    return affected;
}

MYSQL_RES *db_query(const char *fmt, ...) {
    char sql[SQL_BUF_SIZE];
    va_list ap;
    va_start(ap, fmt);
    int n = format_sql(sql, sizeof(sql), fmt, ap);
    va_end(ap);
    if (n < 0) return NULL;

    MYSQL *conn = db_get_conn();
    if (!conn) return NULL;

    MYSQL_RES *res = NULL;
    if (mysql_query(conn, sql) == 0) {
        res = mysql_store_result(conn);
        if (!res && mysql_errno(conn) != 0) {
            snprintf(g_db.last_error, sizeof(g_db.last_error),
                     "%s", mysql_error(conn));
        }
        /* res 为 NULL 且无错误码 = 该语句无结果集（正常） */
    } else {
        snprintf(g_db.last_error, sizeof(g_db.last_error),
                 "%s", mysql_error(conn));
    }
    db_release_conn(conn);
    return res;
}

unsigned long db_last_insert_id(void) {
    MYSQL *conn = db_get_conn();
    if (!conn) return 0;
    unsigned long id = (unsigned long)mysql_insert_id(conn);
    db_release_conn(conn);
    return id;
}

/* ── 事务 ──
 * 注意：事务需在同一连接上执行。这里用简单实现，
 * 直接对整个库开关 autocommit。业务若需严格事务，
 * 建议在单次请求内串行完成。
 */
int db_begin(void) {
    return db_execute("START TRANSACTION") >= 0 ? OK : ERR_DB;
}

int db_commit(void) {
    return db_execute("COMMIT") >= 0 ? OK : ERR_DB;
}

int db_rollback(void) {
    return db_execute("ROLLBACK") >= 0 ? OK : ERR_DB;
}

char *db_escape(const char *str) {
    if (!str) {
        char *empty = malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }
    size_t len = strlen(str);
    char *out = malloc(len * 2 + 1);
    if (!out) return NULL;

    MYSQL *conn = db_get_conn();
    if (conn) {
        mysql_real_escape_string(conn, out, str, (unsigned long)len);
        db_release_conn(conn);
    } else {
        /* 无连接时退化为原样拷贝（不应发生） */
        memcpy(out, str, len + 1);
    }
    return out;
}

const char *db_error(void) {
    return g_db.last_error[0] ? g_db.last_error : "无错误";
}

int db_ping(void) {
    MYSQL *conn = db_get_conn();
    if (!conn) return ERR_DB;
    int r = mysql_ping(conn);
    db_release_conn(conn);
    return r == 0 ? OK : ERR_DB;
}

/* ── 便捷取值 ── */
int db_query_int(const char *fmt, ...) {
    char sql[SQL_BUF_SIZE];
    va_list ap;
    va_start(ap, fmt);
    int n = format_sql(sql, sizeof(sql), fmt, ap);
    va_end(ap);
    if (n < 0) return 0;

    MYSQL_RES *res = db_query("%s", sql);
    if (!res) return 0;
    int val = 0;
    MYSQL_ROW row = mysql_fetch_row(res);
    if (row && row[0]) val = atoi(row[0]);
    mysql_free_result(res);
    return val;
}

double db_query_double(const char *fmt, ...) {
    char sql[SQL_BUF_SIZE];
    va_list ap;
    va_start(ap, fmt);
    int n = format_sql(sql, sizeof(sql), fmt, ap);
    va_end(ap);
    if (n < 0) return 0.0;

    MYSQL_RES *res = db_query("%s", sql);
    if (!res) return 0.0;
    double val = 0.0;
    MYSQL_ROW row = mysql_fetch_row(res);
    if (row && row[0]) val = atof(row[0]);
    mysql_free_result(res);
    return val;
}

/* 取第一行第一列字符串到 out。签名：fmt 在首，变参在尾（out/out_size 之后）。
 * 无结果或出错时 out 置空串。 */
void db_query_str(const char *fmt, char *out, size_t out_size, ...) {
    if (out && out_size > 0) out[0] = '\0';
    if (!out || out_size == 0) return;

    char sql[SQL_BUF_SIZE];
    va_list ap;
    va_start(ap, out_size);
    int n = format_sql(sql, sizeof(sql), fmt, ap);
    va_end(ap);
    if (n < 0) return;

    MYSQL_RES *res = db_query("%s", sql);
    if (!res) return;
    MYSQL_ROW row = mysql_fetch_row(res);
    if (row && row[0]) {
        strncpy(out, row[0], out_size - 1);
        out[out_size - 1] = '\0';
    }
    mysql_free_result(res);
}
