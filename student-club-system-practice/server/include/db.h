/**
 * db.h — 数据库访问层接口
 * 学生社团管理系统 SCMS
 *
 * 封装 MySQL C API，业务模块只需调用 db_query / db_execute，
 * 无需直接接触 libmysqlclient 的连接管理细节。
 *
 * 用法速查：
 *   查询：MYSQL_RES *r = db_query("SELECT ... WHERE id=%d", id);
 *         遍历 mysql_fetch_row(r)，用完 mysql_free_result(r);
 *   写入：db_execute("UPDATE ... SET x=%d WHERE id=%d", x, id);
 *         返回受影响行数，失败返回 -1。
 *   转义：拼接用户输入的字符串前，先 char *e = db_escape(s);
 *         用完 free(e);
 */
#ifndef DB_H
#define DB_H

#include <mysql.h>

/* 数据库连接配置（从 db_config.ini 读取） */
typedef struct {
    char host[256];         /* 默认 127.0.0.1 */
    int  port;              /* 默认 3306 */
    char user[51];
    char password[51];
    char database[51];      /* 默认 scms_db */
    int  pool_size;         /* 连接池大小，默认 5 */
} DBConfig;

/* ── 生命周期 ── */
int  db_init(const DBConfig *cfg);     /* 初始化连接池，成功返回 OK */
void db_close(void);                    /* 关闭全部连接，程序退出时调用 */

/* ── 连接池（一般无需直接调用，db_query/db_execute 内部使用） ── */
MYSQL *db_get_conn(void);
void   db_release_conn(MYSQL *conn);

/* ── CRUD ──
 * 这两个函数支持 printf 风格的格式化占位符。
 * 注意：%s 会原样插入，若插入用户输入，务必先 db_escape 转义！
 */
int        db_execute(const char *fmt, ...);   /* INSERT/UPDATE/DELETE，返回影响行数，失败 -1 */
MYSQL_RES *db_query(const char *fmt, ...);      /* SELECT，返回结果集（需 mysql_free_result），失败 NULL */

unsigned long db_last_insert_id(void);          /* 最近一次 INSERT 的自增 ID */

/* ── 事务 ── */
int db_begin(void);
int db_commit(void);
int db_rollback(void);

/* ── 工具 ── */
char       *db_escape(const char *str);         /* 返回 malloc 的转义串，需 free；str 为 NULL 返回空串 */
const char *db_error(void);                     /* 最近一次错误描述 */
int         db_ping(void);                       /* 连接保活检测 */

/* ── 便捷取值（结果集第一行第一列） ── */
int    db_query_int(const char *fmt, ...);       /* 常用于 COUNT(*)，无结果返回 0 */
double db_query_double(const char *fmt, ...);    /* 常用于 SUM()，无结果返回 0.0 */

/* 取第一行第一列的字符串到 out（截断保证 '\0'）。无结果时 out 置空串。
 * 变参在 fmt 之后、out/out_size 之前无法放——故 out/out_size 在前，fmt 在最后。 */
void   db_query_str(const char *fmt, char *out, size_t out_size, ...);

/* 把结果集转成 JSON 数组字符串（malloc，需 free）。res 为 NULL 返回 "[]"。
 * 每行是一个对象，键为列名。消耗游标但不释放 res（由调用方释放）。 */
char  *db_result_to_json_array(MYSQL_RES *res);

#endif /* DB_H */
