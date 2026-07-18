/**
 * api.h — HTTP/JSON 业务接口辅助层
 * 学生社团管理系统 SCMS（Web 架构）
 *
 * ┌─────────────────────────────────────────────────────────────┐
 * │  你（业务模块作者）只需要和本文件打交道，不用碰 Mongoose 底层。 │
 * │                                                               │
 * │  每个业务 API 函数签名统一为：                                 │
 * │      void handler(ApiContext *ctx);                           │
 * │                                                               │
 * │  在函数内部：                                                  │
 * │   1. 用 api_get_param / api_get_int 读取前端传来的参数         │
 * │   2. 用 ctx->user 拿到当前登录用户（已鉴权）                   │
 * │   3. 拼 SQL，调 db_query / db_execute                         │
 * │   4. 用 api_json_* 系列函数拼 JSON 并返回                      │
 * └─────────────────────────────────────────────────────────────┘
 */
#ifndef API_H
#define API_H

#include "models.h"
#include "db.h"
#include "mongoose.h"

/* ── 请求上下文 ──
 * 每个 API 处理函数收到一个 ApiContext 指针，
 * 里面封装了当前连接、HTTP 消息、已登录用户。
 */
typedef struct {
    struct mg_connection *conn;   /* Mongoose 连接（内部使用，一般不用直接碰） */
    struct mg_http_message *hm;   /* HTTP 请求消息（内部使用） */
    User *user;                    /* 当前登录用户；未登录接口为 NULL */
    char method[8];                /* "GET" / "POST" / "PUT" / "DELETE" */
} ApiContext;

/* ═══════════════════════ 一、读取请求参数 ═══════════════════════ */

/* 从 query string 或 POST 表单读取字符串参数。
 * 找到写入 out（最多 out_size-1 字节）并返回 1，未找到返回 0（out 置空串）。
 * 例：api_get_param(ctx, "name", buf, sizeof(buf)); */
int api_get_param(ApiContext *ctx, const char *key, char *out, size_t out_size);

/* 读取整数参数，未找到或非法时返回 def_val */
int api_get_int(ApiContext *ctx, const char *key, int def_val);

/* 读取浮点参数（金额等），未找到返回 def_val */
double api_get_double(ApiContext *ctx, const char *key, double def_val);

/* 从 JSON 请求体读取字符串字段（前端以 application/json 提交时使用） */
int api_get_json_str(ApiContext *ctx, const char *key, char *out, size_t out_size);

/* 从 JSON 请求体读取整数字段 */
int api_get_json_int(ApiContext *ctx, const char *key, int def_val);

/* 从 JSON 请求体读取浮点字段 */
double api_get_json_double(ApiContext *ctx, const char *key, double def_val);

/* 读取 URL 路径参数：例如路由 /api/clubs/:id ，取 :id
 * idx 为第几段（从 0 开始，仅 /api/ 之后的段计数）。 */
int api_get_path_int(ApiContext *ctx, int idx);

/* ── multipart/form-data 表单（含文件上传，如报销发票） ── */

/* 读取 multipart 表单里的文本字段到 out，未找到置空串，返回是否找到(1/0)。 */
int api_get_form_str(ApiContext *ctx, const char *key, char *out, size_t out_size);

/* 读取 multipart 表单里的浮点字段（金额等），未找到返回 def_val。 */
double api_get_form_double(ApiContext *ctx, const char *key, double def_val);

/* 保存 multipart 表单里名为 field 的上传文件到 subdir 目录（相对项目根，如 "receipts"）。
 * 文件名自动加时间戳+随机串防冲突，保留原扩展名。
 * 成功把相对路径（如 "receipts/20260717_ab12.jpg"）写入 out_path，返回 0；
 * 无该文件或写入失败返回 -1。 */
int api_save_uploaded_file(ApiContext *ctx, const char *field,
                           const char *subdir, char *out_path, size_t out_size);

/* ═══════════════════════ 二、返回 JSON 响应 ═══════════════════════ */

/* 直接返回一段自行拼好的 JSON 文本（自动带 200 状态与 Content-Type） */
void api_send_json(ApiContext *ctx, const char *json);

/* 返回成功但无数据：{"code":0,"msg":"ok"} */
void api_ok(ApiContext *ctx);

/* 返回成功 + 一句提示：{"code":0,"msg":"<msg>"} */
void api_ok_msg(ApiContext *ctx, const char *msg);

/* 返回成功 + 一段数据对象/数组（data 为已拼好的 JSON 片段）：
 * {"code":0,"msg":"ok","data": <data> } */
void api_ok_data(ApiContext *ctx, const char *data_json);

/* 返回错误：{"code":<code>,"msg":"<msg>"}，HTTP 状态仍 200，前端读 code 判断 */
void api_error(ApiContext *ctx, int code, const char *msg);

/* ═══════════════════════ 三、把查询结果转成 JSON ═══════════════════════ */
/* 这组函数把 MySQL 结果集自动转为 JSON 数组返回，省去手动拼接。 */

/* 将 SELECT 结果集直接转为 JSON 数组并返回：
 *   [ {"列名":"值",...}, ... ]
 * 用列名做 key。NULL 值转为 null。调用后内部会 free 结果集。
 * 用法：api_send_result(ctx, db_query("SELECT ...")); */
void api_send_result(ApiContext *ctx, MYSQL_RES *res);

/* 同上，但外面再包一层 {"code":0,"msg":"ok","data":[...]} */
void api_send_result_data(ApiContext *ctx, MYSQL_RES *res);

/* 带分页信息返回：
 * {"code":0,"data":[...],"page":P,"page_size":S,"total":T,"total_pages":N} */
void api_send_result_paged(ApiContext *ctx, MYSQL_RES *res,
                           int page, int page_size, int total);

/* ═══════════════════════ 四、手动拼 JSON 的辅助（可选） ═══════════════════════ */
/* 若要精细控制 JSON，可用 JsonBuilder 逐字段追加。 */

typedef struct {
    char  *buf;
    size_t len;
    size_t cap;
    int    first;   /* 内部标记：是否已写入第一个字段 */
} JsonBuilder;

void json_init(JsonBuilder *jb);                                  /* 初始化，开始一个对象 { */
void json_add_str(JsonBuilder *jb, const char *key, const char *val);   /* "key":"val" */
void json_add_int(JsonBuilder *jb, const char *key, long val);          /* "key":123 */
void json_add_double(JsonBuilder *jb, const char *key, double val);     /* "key":1.50 */
void json_add_bool(JsonBuilder *jb, const char *key, int val);          /* "key":true/false */
void json_add_raw(JsonBuilder *jb, const char *key, const char *raw);   /* "key":<raw JSON> */
const char *json_finish(JsonBuilder *jb);                          /* 收尾 }，返回内部字符串 */
void json_free(JsonBuilder *jb);                                   /* 释放缓冲 */

/* ═══════════════════════ 五、权限检查（越权即返回错误） ═══════════════════════ */
/* 返回 1 表示有权限；返回 0 表示无权限，且已自动向前端返回 403 错误 JSON。
 * 用法：if (!api_require_role(ctx, "school_admin")) return; */

int api_require_login(ApiContext *ctx);                       /* 需已登录 */
int api_require_role(ApiContext *ctx, const char *role);      /* 需指定角色 */
int api_require_school_admin(ApiContext *ctx);                /* 需学校管理员 */
int api_require_college_admin(ApiContext *ctx);               /* 需学院管理员 */
int api_require_club_admin(ApiContext *ctx, int club_id);     /* 需为该社团社长/副社长 */

/* ═══════════════════════ 六、文件上传（发票图片） ═══════════════════════ */

/* 保存 multipart 上传的文件到 receipts/ 目录，
 * 成功时把相对路径写入 out_path（形如 "receipts/xxxx.jpg"）并返回 OK。 */
int api_save_upload(ApiContext *ctx, const char *field_name,
                    char *out_path, size_t out_size);

#endif /* API_H */
