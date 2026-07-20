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

/** 数据库连接配置（从 db_config.ini 读取） */
typedef struct {
    char host[256];         // 数据库主机地址，默认 127.0.0.1
    int  port;              // 端口号，默认 3306
    char user[51];          // 数据库用户名
    char password[51];      // 数据库密码
    char database[51];      // 数据库名，默认 scms_db
    int  pool_size;         // 连接池大小，默认 5，最大 16
} DBConfig;

/* ── 生命周期 ── */

/** 初始化数据库连接池
 *  @param cfg 指向已填充的数据库连接配置结构体
 *  @return 成功返回 OK，失败返回错误码 */
int  db_init(const DBConfig *cfg);

/** 关闭全部数据库连接，程序退出时调用 */
void db_close(void);

/* ── 连接池（一般无需直接调用，db_query/db_execute 内部已封装） ── */

/** 从连接池获取一个可用连接
 *  @return 成功返回 MySQL 连接句柄，池满则等待 */
MYSQL *db_get_conn(void);

/** 将连接归还连接池
 *  @param conn 要归还的 MySQL 连接句柄 */
void   db_release_conn(MYSQL *conn);

/* ── 增删改查（CRUD） ──
 * 这两个函数支持 printf 风格的格式化占位符。
 * 注意：%s 会原样插入，若插入用户输入，务必先 db_escape 转义！
 */

/** 执行 INSERT/UPDATE/DELETE 语句
 *  @param fmt printf 风格的 SQL 模板字符串
 *  @param ... 可变参数，依次替换模板中的占位符
 *  @return 受影响行数，失败返回 -1 */
int        db_execute(const char *fmt, ...);

/** 执行 SELECT 查询语句
 *  @param fmt printf 风格的 SQL 模板字符串
 *  @param ... 可变参数，依次替换模板中的占位符
 *  @return 结果集指针（用后需 mysql_free_result），失败返回 NULL */
MYSQL_RES *db_query(const char *fmt, ...);

/** 获取最近一次 INSERT 操作的自增主键 ID
 *  @return 自增 ID 值 */
unsigned long db_last_insert_id(void);

/* ── 事务 ── */

/** 开启事务
 *  @return 成功返回 OK，失败返回错误码 */
int db_begin(void);

/** 提交事务
 *  @return 成功返回 OK，失败返回错误码 */
int db_commit(void);

/** 回滚事务
 *  @return 成功返回 OK，失败返回错误码 */
int db_rollback(void);

/* ── 工具函数 ── */

/** 对字符串进行 SQL 转义，防止注入
 *  @param str 待转义的原始字符串，可为 NULL
 *  @return malloc 分配的转义后字符串（用后需 free），str 为 NULL 时返回空串 */
char       *db_escape(const char *str);

/** 获取最近一次数据库操作的错误描述文本
 *  @return 错误描述字符串（静态缓冲区，无需释放） */
const char *db_error(void);

/** 检测数据库连接是否存活
 *  @return 连接正常返回 0，断开返回非 0 */
int         db_ping(void);

/** 获取当前数据库连接配置的只读指针
 *  @return 指向 DBConfig 的常量指针（供备份等模块读取密码等配置） */
const DBConfig *db_get_config(void);

/* ── 便捷取值（结果集第一行第一列） ── */

/** 查询并返回整数值（常用于 COUNT(*)）
 *  @param fmt printf 风格的 SQL 模板字符串
 *  @param ... 可变参数
 *  @return 查询结果的第一行第一列的整数值，无结果返回 0 */
int    db_query_int(const char *fmt, ...);

/** 查询并返回浮点值（常用于 SUM()）
 *  @param fmt printf 风格的 SQL 模板字符串
 *  @param ... 可变参数
 *  @return 查询结果的第一行第一列的浮点值，无结果返回 0.0 */
double db_query_double(const char *fmt, ...);

/** 查询并返回字符串值（第一行第一列），存入指定缓冲区
 *  无结果时 out 置为空串。
 *  @param fmt printf 风格的 SQL 模板字符串
 *  @param out 输出缓冲区
 *  @param out_size 输出缓冲区大小（自动截断保证结束符）
 *  @param ... 可变参数，替换模板中的占位符 */
void   db_query_str(const char *fmt, char *out, size_t out_size, ...);

/** 将 MySQL 结果集转换为 JSON 数组字符串
 *  每行对应一个 JSON 对象，键为列名。消耗游标但不释放结果集。
 *  @param res MySQL 结果集指针，为 NULL 时返回 "[]"
 *  @return malloc 分配的 JSON 字符串（用后需 free） */
char  *db_result_to_json_array(MYSQL_RES *res);

#endif /* DB_H */
