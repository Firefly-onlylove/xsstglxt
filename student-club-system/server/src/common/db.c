/**
 * db.c — 数据库访问层实现
 * 学生社团管理系统 SCMS
 *
 * 封装 MySQL C API：
 *   - 简易连接池（互斥锁保护，多线程安全）
 *   - db_query / db_execute 支持 printf 风格格式化
 *   - 事务控制、字符串转义、错误处理
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
    MYSQL *conn;     // MySQL 连接句柄
    int    in_use;   // 是否正在被使用（1=占用 0=空闲）
} PoolSlot;

static struct {
    DBConfig  cfg;          // 数据库配置
    PoolSlot  slots[MAX_POOL]; // 连接槽位数组
    int       size;         // 池中连接数
    db_mutex_t lock;        // 线程互斥锁
    int       initialized;  // 是否已初始化
    char      last_error[512]; // 最近一次错误信息
} g_db = {0};

/**
 * create_connection — 建立单条 MySQL 连接
 * 返回: 成功返回连接句柄，失败返回 NULL
 *
 * 配置客户端字符集为 utf8mb4，防止中文乱码。
 */
static MYSQL *create_connection(void) {
    MYSQL *conn = mysql_init(NULL);
    if (!conn) return NULL;

    /* 设置客户端字符集，防止中文乱码 */
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

/**
 * db_init — 初始化数据库连接池
 * @cfg: 数据库配置（主机、端口、用户名、密码、库名、池大小）
 * 返回: OK(0) 成功, ERR_DB(-1) 失败
 *
 * 全有或全无策略：所有连接建立成功才返回 OK，
 * 若有一条连接失败则关闭已建立的连接并返回错误。
 */
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

/**
 * db_close — 关闭数据库连接池
 *
 * 释放所有连接，销毁互斥锁，调用 mysql_library_end() 清理。
 */
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

/**
 * db_get_conn — 从连接池获取一条可用连接
 * 返回: 成功返回连接句柄，池满则临时新建一条（用完直接关闭，不入池），失败返回 NULL
 */
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

/**
 * db_release_conn — 归还连接到连接池
 * @conn: 要归还的连接句柄
 *
 * 如果连接属于池中的槽位，则标记为空闲。
 * 如果是临时创建的连接（不在池中），则直接关闭。
 */
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

/**
 * format_sql — 将 printf 风格的格式化参数写入 SQL 缓冲区
 * @buf: 输出缓冲区
 * @size: 缓冲区大小
 * @fmt: SQL 格式化模板
 * @ap: 变长参数列表
 * 返回: 成功返回写入字节数，失败（SQL 过长）返回 -1
 */
static int format_sql(char *buf, size_t size, const char *fmt, va_list ap) {
    int n = vsnprintf(buf, size, fmt, ap);
    if (n < 0 || (size_t)n >= size) {
        snprintf(g_db.last_error, sizeof(g_db.last_error),
                 "SQL 语句过长（超过 %d 字节）", (int)size);
        return -1;
    }
    return n;
}

/**
 * db_execute — 执行一条无结果集的 SQL（INSERT/UPDATE/DELETE 等）
 * @fmt: printf 风格的 SQL 模板
 * @...: 格式化参数
 * 返回: 成功返回受影响行数，失败返回 -1
 */
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

/**
 * db_query — 执行一条有结果集的 SQL（SELECT 等）
 * @fmt: printf 风格的 SQL 模板
 * @...: 格式化参数
 * 返回: 成功返回结果集句柄（调用者负责 mysql_free_result），失败返回 NULL
 *
 * res 为 NULL 且无错误码表示该语句本身无结果集（正常情况）。
 */
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

/**
 * db_last_insert_id — 获取最近一次 INSERT 操作生成的自增 ID
 * 返回: 自增 ID 值，失败返回 0
 */
unsigned long db_last_insert_id(void) {
    MYSQL *conn = db_get_conn();
    if (!conn) return 0;
    unsigned long id = (unsigned long)mysql_insert_id(conn);
    db_release_conn(conn);
    return id;
}

/**
 * 事务控制
 *
 * 注意：事务需在同一连接上执行。这里用简单实现，
 * 直接对整个连接开关 autocommit。业务若需严格事务，
 * 建议在单次请求内串行完成全部操作。
 */

/**
 * db_begin — 开启事务
 * 返回: OK(0) 成功, ERR_DB(-1) 失败
 */
int db_begin(void) {
    return db_execute("START TRANSACTION") >= 0 ? OK : ERR_DB;
}

/**
 * db_commit — 提交事务
 * 返回: OK(0) 成功, ERR_DB(-1) 失败
 */
int db_commit(void) {
    return db_execute("COMMIT") >= 0 ? OK : ERR_DB;
}

/**
 * db_rollback — 回滚事务
 * 返回: OK(0) 成功, ERR_DB(-1) 失败
 */
int db_rollback(void) {
    return db_execute("ROLLBACK") >= 0 ? OK : ERR_DB;
}

/**
 * db_escape — 对字符串进行 SQL 转义，防止注入
 * @str: 原始字符串（可为 NULL）
 * 返回: 转义后的字符串（调用者负责 free），NULL 输入返回空字符串
 */
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

/**
 * db_error — 获取最近一次数据库操作的错误信息
 * 返回: 错误描述字符串
 */
const char *db_error(void) {
    return g_db.last_error[0] ? g_db.last_error : "无错误";
}

/**
 * db_ping — 检测数据库连接是否存活
 * 返回: OK(0) 连接正常, ERR_DB(-1) 连接断开
 */
int db_ping(void) {
    MYSQL *conn = db_get_conn();
    if (!conn) return ERR_DB;
    int r = mysql_ping(conn);
    db_release_conn(conn);
    return r == 0 ? OK : ERR_DB;
}

/**
 * db_get_config — 获取当前数据库配置
 * 返回: 配置结构体指针
 */
const DBConfig *db_get_config(void) {
    return &g_db.cfg;
}

/* ── 便捷取值 ── */

/**
 * db_query_int — 执行 SQL 并返回第一行第一列的整数值
 * @fmt: printf 风格的 SQL 模板
 * @...: 格式化参数
 * 返回: 查询结果整数值，失败或无结果返回 0
 */
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

/**
 * db_query_double — 执行 SQL 并返回第一行第一列的浮点数值
 * @fmt: printf 风格的 SQL 模板
 * @...: 格式化参数
 * 返回: 查询结果浮点数值，失败或无结果返回 0.0
 */
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

/**
 * db_query_str — 执行 SQL 并将第一行第一列的字符串写入 out
 * @fmt: printf 风格的 SQL 模板（注意：fmt 在首，变参在 out/out_size 之后）
 * @out: 输出缓冲区
 * @out_size: 输出缓冲区大小
 * @...: SQL 格式化参数
 *
 * 无结果或出错时 out 置空串。
 */
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
