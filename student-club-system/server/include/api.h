/**
 * api.h — HTTP/JSON 业务接口辅助层
 * 学生社团管理系统 SCMS（Web 架构）
 *
 * ┌─────────────────────────────────────────────────────────────┐
 * │  业务模块开发者只需和本文件打交道，无需接触 Mongoose 底层。   │
 * │                                                               │
 * │  每个业务 API 函数签名统一为：                                 │
 * │      void handler(ApiContext *ctx);                           │
 * │                                                               │
 * │  在函数内部：                                                  │
 * │   1. 用 api_get_param / api_get_int 读取前端传来的参数         │
 * │   2. 用 ctx->user 获取当前登录用户信息（已鉴权）               │
 * │   3. 拼接 SQL，调用 db_query / db_execute 执行                │
 * │   4. 用 api_json_* 系列函数拼装 JSON 并返回                    │
 * └─────────────────────────────────────────────────────────────┘
 */
#ifndef API_H
#define API_H

#include "models.h"
#include "db.h"
#include "mongoose.h"

/* ── 请求上下文 ──
 * 每个 API 处理函数会收到一个 ApiContext 指针，
 * 里面封装了当前连接、HTTP 请求消息和已登录用户信息。
 */

/** API 请求上下文 */
typedef struct {
    struct mg_connection *conn;   // Mongoose 连接对象（内部使用，一般无需直接操作）
    struct mg_http_message *hm;   // HTTP 请求消息对象（内部使用）
    User *user;                    // 当前登录用户；未登录接口中为 NULL
    char method[8];                // HTTP 请求方法："GET"/"POST"/"PUT"/"DELETE"
} ApiContext;

/* ═══════════════════════ 一、读取请求参数 ═══════════════════════ */

/** 从查询字符串或 POST 表单中读取字符串参数
 *  @param ctx  API 请求上下文
 *  @param key  参数名
 *  @param out  输出缓冲区
 *  @param out_size 输出缓冲区大小（最多写入 out_size-1 字节）
 *  @return 找到返回 1（值写入 out），未找到返回 0（out 置为空串）
 *  @example api_get_param(ctx, "name", buf, sizeof(buf)); */
int api_get_param(ApiContext *ctx, const char *key, char *out, size_t out_size);

/** 从查询字符串或 POST 表单中读取整数参数
 *  @param ctx  API 请求上下文
 *  @param key  参数名
 *  @param def_val 参数不存在或非法时的默认值
 *  @return 解析后的整数值 */
int api_get_int(ApiContext *ctx, const char *key, int def_val);

/** 从查询字符串或 POST 表单中读取浮点参数（金额等）
 *  @param ctx  API 请求上下文
 *  @param key  参数名
 *  @param def_val 参数不存在或非法时的默认值
 *  @return 解析后的浮点值 */
double api_get_double(ApiContext *ctx, const char *key, double def_val);

/** 从 JSON 请求体中读取字符串字段（前端以 application/json 提交时使用）
 *  @param ctx  API 请求上下文
 *  @param key  JSON 字段名
 *  @param out  输出缓冲区
 *  @param out_size 输出缓冲区大小
 *  @return 找到返回 1，未找到返回 0（out 置为空串） */
int api_get_json_str(ApiContext *ctx, const char *key, char *out, size_t out_size);

/** 从 JSON 请求体中读取整数字段
 *  @param ctx  API 请求上下文
 *  @param key  JSON 字段名
 *  @param def_val 字段不存在或非法时的默认值
 *  @return 解析后的整数值 */
int api_get_json_int(ApiContext *ctx, const char *key, int def_val);

/** 从 JSON 请求体中读取浮点字段
 *  @param ctx  API 请求上下文
 *  @param key  JSON 字段名
 *  @param def_val 字段不存在或非法时的默认值
 *  @return 解析后的浮点值 */
double api_get_json_double(ApiContext *ctx, const char *key, double def_val);

/** 读取 URL 路径中的参数值
 *  例如路由 /api/clubs/star/42，idx=1 返回 42
 *  @param ctx API 请求上下文
 *  @param idx 路径段索引（从 0 开始，仅对 /api/ 之后的段计数）
 *  @return 该路径段的整数值 */
int api_get_path_int(ApiContext *ctx, int idx);

/* ── multipart/form-data 表单（含文件上传，如报销发票） ── */

/** 从 multipart 表单中读取文本字段
 *  @param ctx  API 请求上下文
 *  @param key  表单字段名
 *  @param out  输出缓冲区
 *  @param out_size 输出缓冲区大小
 *  @return 找到返回 1，未找到返回 0（out 置为空串） */
int api_get_form_str(ApiContext *ctx, const char *key, char *out, size_t out_size);

/** 从 multipart 表单中读取浮点字段（金额等）
 *  @param ctx  API 请求上下文
 *  @param key  表单字段名
 *  @param def_val 字段不存在时的默认值
 *  @return 解析后的浮点值 */
double api_get_form_double(ApiContext *ctx, const char *key, double def_val);

/** 保存 multipart 表单中上传的文件到指定子目录
 *  文件名自动附加时间戳和随机串防止冲突，保留原始扩展名。
 *  @param ctx   API 请求上下文
 *  @param field 表单中文件字段的名称
 *  @param subdir 保存目录（相对于项目根目录，如 "receipts"）
 *  @param out_path 输出缓冲区，成功时写入相对路径（如 "receipts/20260717_ab12.jpg"）
 *  @param out_size 输出缓冲区大小
 *  @return 成功返回 0，无该文件或写入失败返回 -1 */
int api_save_uploaded_file(ApiContext *ctx, const char *field,
                           const char *subdir, char *out_path, size_t out_size);

/* ═══════════════════════ 二、返回 JSON 响应 ═══════════════════════ */

/** 直接发送一段 JSON 文本作为响应
 *  自动附带 HTTP 200 状态码与 Content-Type: application/json 头。
 *  @param ctx  API 请求上下文
 *  @param json 已拼装好的 JSON 字符串 */
void api_send_json(ApiContext *ctx, const char *json);

/** 返回成功但无业务数据：{"code":0,"msg":"ok"}
 *  @param ctx API 请求上下文 */
void api_ok(ApiContext *ctx);

/** 返回成功并附带提示消息：{"code":0,"msg":"<msg>"}
 *  @param ctx API 请求上下文
 *  @param msg 提示消息文本 */
void api_ok_msg(ApiContext *ctx, const char *msg);

/** 返回成功并附带业务数据对象/数组
 *  响应格式：{"code":0,"msg":"ok","data": <data>}
 *  @param ctx       API 请求上下文
 *  @param data_json 已拼装好的 JSON 片段（不含外层包装） */
void api_ok_data(ApiContext *ctx, const char *data_json);

/** 返回业务错误
 *  响应格式：{"code":<code>,"msg":"<msg>"}
 *  HTTP 状态码仍为 200，由前端根据 code 字段判断成功与否。
 *  @param ctx  API 请求上下文
 *  @param code 业务错误码（参见 common.h 中 ERR_* 系列宏）
 *  @param msg  错误描述文本 */
void api_error(ApiContext *ctx, int code, const char *msg);

/* ═══════════════════════ 三、将查询结果转为 JSON ═══════════════════════ */
/* 这组函数把 MySQL 结果集自动转为 JSON 数组并返回，省去手动拼接。 */

/** 将 SELECT 结果集转为 JSON 数组并直接响应
 *  格式：[ {"列名":"值",...}, ... ]，列名作键，NULL 值转为 null。
 *  调用后内部会释放结果集。
 *  @param ctx API 请求上下文
 *  @param res MySQL 结果集指针
 *  @example api_send_result(ctx, db_query("SELECT ...")); */
void api_send_result(ApiContext *ctx, MYSQL_RES *res);

/** 同上，但外层再包一层成功响应
 *  格式：{"code":0,"msg":"ok","data":[...]}
 *  @param ctx API 请求上下文
 *  @param res MySQL 结果集指针 */
void api_send_result_data(ApiContext *ctx, MYSQL_RES *res);

/** 将结果集转为 JSON 数组并附带分页信息
 *  格式：{"code":0,"data":[...],"page":P,"page_size":S,"total":T,"total_pages":N}
 *  @param ctx       API 请求上下文
 *  @param res       MySQL 结果集指针
 *  @param page      当前页码
 *  @param page_size 每页条数
 *  @param total     总记录数 */
void api_send_result_paged(ApiContext *ctx, MYSQL_RES *res,
                           int page, int page_size, int total);

/* ═══════════════════════ 四、手动拼接 JSON 的辅助工具 ═══════════════════════ */
/* 如需精细控制 JSON 输出，可使用 JsonBuilder 逐字段追加。 */

/** JSON 构建器（顺序追加字段，自动处理逗号分隔） */
typedef struct {
    char  *buf;     // 内部缓冲区指针
    size_t len;     // 当前已写入长度
    size_t cap;     // 缓冲区总容量
    int    first;   // 内部标记：是否尚未写入任何字段（用于控制逗号）
} JsonBuilder;

/** 初始化 JSON 构建器，开始一个对象 { */
void json_init(JsonBuilder *jb);

/** 追加字符串字段："key":"val" */
void json_add_str(JsonBuilder *jb, const char *key, const char *val);

/** 追加整数字段："key":123 */
void json_add_int(JsonBuilder *jb, const char *key, long val);

/** 追加浮点字段："key":1.50 */
void json_add_double(JsonBuilder *jb, const char *key, double val);

/** 追加布尔字段："key":true 或 "key":false */
void json_add_bool(JsonBuilder *jb, const char *key, int val);

/** 追加原始 JSON 片段："key":<raw>（不转义，直接拼接） */
void json_add_raw(JsonBuilder *jb, const char *key, const char *raw);

/** 结束 JSON 对象 } 并返回内部字符串（只读，下次操作前有效） */
const char *json_finish(JsonBuilder *jb);

/** 释放 JsonBuilder 内部缓冲区 */
void json_free(JsonBuilder *jb);

/* ═══════════════════════ 五、权限检查（越权即自动返回错误） ═══════════════════════ */
/* 返回 1 表示有权限；返回 0 表示无权限，且已自动向前端发送错误 JSON 响应。
 * 调用方收到 0 后应直接 return，不要继续执行后续逻辑。
 * 用法：if (!api_require_role(ctx, "school_admin")) return; */

/** 要求用户已登录，未登录返回 403
 *  @return 已登录返回 1，未登录返回 0（已发送错误响应） */
int api_require_login(ApiContext *ctx);

/** 要求用户具备指定角色
 *  @param role 要求的角色字符串（如 "school_admin"）
 *  @return 角色匹配返回 1，不匹配返回 0（已发送错误响应） */
int api_require_role(ApiContext *ctx, const char *role);

/** 要求当前用户为学校管理员
 *  @return 是学校管理员返回 1，否则返回 0（已发送错误响应） */
int api_require_school_admin(ApiContext *ctx);

/** 要求当前用户为学院管理员
 *  @return 是学院管理员返回 1，否则返回 0（已发送错误响应） */
int api_require_college_admin(ApiContext *ctx);

/** 要求当前用户为指定社团的社长或副社长
 *  @param club_id 社团 ID
 *  @return 是该社团管理员返回 1，否则返回 0（已发送错误响应） */
int api_require_club_admin(ApiContext *ctx, int club_id);

/* ═══════════════════════ 六、文件上传（发票图片） ═══════════════════════ */

/** 保存 multipart 上传的文件到 receipts/ 目录
 *  @param ctx        API 请求上下文
 *  @param field_name 表单中文件字段的名称
 *  @param out_path   输出缓冲区，成功时写入相对路径（如 "receipts/xxxx.jpg"）
 *  @param out_size   输出缓冲区大小
 *  @return 成功返回 OK，失败返回错误码 */
int api_save_upload(ApiContext *ctx, const char *field_name,
                    char *out_path, size_t out_size);

#endif /* API_H */
