/**
 * api.c — HTTP/JSON 业务接口辅助层实现
 * 学生社团管理系统 SCMS（Web 架构）
 *
 * 实现 api.h 声明的全部辅助函数：
 *   - 请求参数读取（query / form / json / path）
 *   - JSON 响应封装
 *   - MySQL 结果集自动转 JSON
 *   - JsonBuilder 手动拼装
 *   - 角色权限检查
 *   - 发票文件上传保存
 *
 * 业务模块只调用本层，不直接接触 Mongoose 细节。
 */
#include "../../include/api.h"
#include "../../include/db.h"
#include "../../include/common.h"
#include "../../include/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#define scms_mkdir(p) _mkdir(p)
#else
#define scms_mkdir(p) mkdir((p), 0755)
#endif

/* 确保上传目录存在（不存在则创建），仅处理单层目录名 */
static void utils_ensure_upload_dir(const char *dir) {
    if (!dir || !dir[0]) return;
    struct stat st;
    if (stat(dir, &st) == 0) return;   /* 目录已存在，直接返回 */
    scms_mkdir(dir);
}

/* ═══════════════════════ 一、读取请求参数 ═══════════════════════ */

/**
 * api_get_param — 从 query string 或 POST 表单读取字符串参数
 * @ctx:      请求上下文
 * @key:      参数名
 * @out:      输出缓冲区（调用方提供）
 * @out_size: 缓冲区大小
 * 返回值：1 = 找到参数，0 = 未找到
 *
 * 查找顺序：先查 URL query string（GET 请求参数），再查 POST body（表单提交）
 */
int api_get_param(ApiContext *ctx, const char *key, char *out, size_t out_size) {
    if (out_size == 0) return 0;
    out[0] = '\0';

    /* 先查 query string（GET 请求） */
    struct mg_str *q = &ctx->hm->query;
    if (q->len > 0) {
        int n = mg_http_get_var(q, key, out, out_size);
        if (n > 0) return 1;
    }
    /* 再查 body（POST 表单，application/x-www-form-urlencoded） */
    if (ctx->hm->body.len > 0) {
        int n = mg_http_get_var(&ctx->hm->body, key, out, out_size);
        if (n > 0) return 1;
    }
    out[0] = '\0';
    return 0;
}

/**
 * api_get_int — 按整型读取请求参数
 * @ctx:     请求上下文
 * @key:     参数名
 * @def_val: 默认值（参数未找到时返回）
 * 返回值：参数值（整数）
 */
int api_get_int(ApiContext *ctx, const char *key, int def_val) {
    char buf[32];
    if (api_get_param(ctx, key, buf, sizeof(buf)) && buf[0])
        return atoi(buf);
    return def_val;
}

/**
 * api_get_double — 按浮点型读取请求参数
 * @ctx:     请求上下文
 * @key:     参数名
 * @def_val: 默认值（参数未找到时返回）
 * 返回值：参数值（双精度浮点数）
 */
double api_get_double(ApiContext *ctx, const char *key, double def_val) {
    char buf[64];
    if (api_get_param(ctx, key, buf, sizeof(buf)) && buf[0])
        return atof(buf);
    return def_val;
}

/**
 * api_get_json_str — 从 JSON body 中读取字符串字段
 * @ctx:      请求上下文
 * @key:      字段名
 * @out:      输出缓冲区
 * @out_size: 缓冲区大小
 * 返回值：1 = 找到，0 = 未找到或 body 为空
 *
 * 使用 Mongoose 内置的 mg_json_get_str 解析 JSON path"$.key"
 */
int api_get_json_str(ApiContext *ctx, const char *key, char *out, size_t out_size) {
    if (out_size == 0) return 0;
    out[0] = '\0';
    if (ctx->hm->body.len == 0) return 0;

    char path[128];
    snprintf(path, sizeof(path), "$.%s", key);
    char *val = mg_json_get_str(ctx->hm->body, path);
    if (val) {
        utils_strlcpy(out, val, out_size);
        free(val);
        return 1;
    }
    return 0;
}

/**
 * api_get_json_int — 从 JSON body 中读取整型字段
 * @ctx:     请求上下文
 * @key:     字段名
 * @def_val: 默认值
 * 返回值：字段的整数值
 */
int api_get_json_int(ApiContext *ctx, const char *key, int def_val) {
    if (ctx->hm->body.len == 0) return def_val;
    char path[128];
    snprintf(path, sizeof(path), "$.%s", key);
    double d;
    if (mg_json_get_num(ctx->hm->body, path, &d))
        return (int)d;
    return def_val;
}

/**
 * api_get_json_double — 从 JSON body 中读取浮点字段
 * @ctx:     请求上下文
 * @key:     字段名
 * @def_val: 默认值
 * 返回值：字段的浮点数值
 */
double api_get_json_double(ApiContext *ctx, const char *key, double def_val) {
    if (ctx->hm->body.len == 0) return def_val;
    char path[128];
    snprintf(path, sizeof(path), "$.%s", key);
    double d;
    if (mg_json_get_num(ctx->hm->body, path, &d))
        return d;
    return def_val;
}

/**
 * api_get_path_int — 从 URL 路径中提取指定段序号对应的整数值
 * @ctx: 请求上下文
 * @idx: 段序号（从 0 开始，指 /api/ 之后的第几段）
 * 返回值：路径段对应的整数，-1 表示解析失败
 *
 * 例：URI 为 /api/clubs/12/members 时：
 *   idx=0 返回 "clubs" 转整数（失败），idx=1 返回 12，idx=2 返回 "members" 转整数（失败）
 */
int api_get_path_int(ApiContext *ctx, int idx) {
    /* uri 形如 /api/clubs/12/members，取 /api/ 之后第 idx 段 */
    struct mg_str uri = ctx->hm->uri;
    const char *p = uri.buf;
    const char *end = uri.buf + uri.len;

    /* 跳过开头的 "/api/" */
    const char *prefix = "/api/";
    size_t plen = strlen(prefix);
    if (uri.len < plen || strncmp(p, prefix, plen) != 0) return -1;
    p += plen;

    int seg = 0;
    while (p < end) {
        const char *seg_start = p;
        while (p < end && *p != '/') p++;
        if (seg == idx) {
            char buf[32];
            size_t len = (size_t)(p - seg_start);
            if (len == 0 || len >= sizeof(buf)) return -1;
            memcpy(buf, seg_start, len);
            buf[len] = '\0';
            return atoi(buf);
        }
        seg++;
        if (p < end) p++;   /* 跳过 '/' */
    }
    return -1;
}

/* ── multipart/form-data 支持（文件上传表单） ──
 * 前端用 FormData 或 <form enctype="multipart/form-data"> 提交时，
 * 普通字段和文件都在同一个 body 里，需用 multipart 解析，不能用 mg_http_get_var。
 */

/**
 * api_get_form_str — 从 multipart 表单中读取文本字段
 * @ctx:      请求上下文
 * @key:      字段名
 * @out:      输出缓冲区
 * @out_size: 缓冲区大小
 * 返回值：1 = 找到，0 = 未找到
 */
int api_get_form_str(ApiContext *ctx, const char *key, char *out, size_t out_size) {
    if (out_size == 0) return 0;
    out[0] = '\0';
    if (ctx->hm->body.len == 0) return 0;

    struct mg_http_part part;
    size_t ofs = 0;
    while ((ofs = mg_http_next_multipart(ctx->hm->body, ofs, &part)) > 0) {
        if (part.name.len == strlen(key) &&
            strncmp(part.name.buf, key, part.name.len) == 0) {
            utils_strlcpy(out, "", out_size);
            size_t n = part.body.len < out_size - 1 ? part.body.len : out_size - 1;
            memcpy(out, part.body.buf, n);
            out[n] = '\0';
            return 1;
        }
    }
    return 0;
}

/**
 * api_get_form_double — 从 multipart 表单中读取浮点数字段
 * @ctx:     请求上下文
 * @key:     字段名
 * @def_val: 默认值
 * 返回值：字段的浮点数值
 */
double api_get_form_double(ApiContext *ctx, const char *key, double def_val) {
    char buf[64];
    if (api_get_form_str(ctx, key, buf, sizeof(buf)) && buf[0])
        return atof(buf);
    return def_val;
}

/**
 * api_save_uploaded_file — 保存 multipart 表单中的上传文件到本地
 * @ctx:      请求上下文
 * @field:    文件字段名（如 "receipt"）
 * @save_dir: 目标目录（如 "receipts"），不存在则自动创建
 * @out_path: 输出缓冲区，写入最终保存的相对路径（供入库使用）
 * @out_size: 输出缓冲区大小
 * 返回值：1 = 保存成功，0 = 无文件或写入失败
 *
 * 文件名做随机化处理（时间戳 + 随机码），避免覆盖与路径穿越。
 */
int api_save_uploaded_file(ApiContext *ctx, const char *field,
                           const char *save_dir, char *out_path, size_t out_size) {
    if (out_size > 0) out_path[0] = '\0';
    if (ctx->hm->body.len == 0) return 0;

    struct mg_http_part part;
    size_t ofs = 0;
    while ((ofs = mg_http_next_multipart(ctx->hm->body, ofs, &part)) > 0) {
        if (part.filename.len == 0) continue;
        if (!(part.name.len == strlen(field) &&
              strncmp(part.name.buf, field, part.name.len) == 0))
            continue;

        /* 取原文件扩展名（只保留字母和数字，防止路径注入） */
        char ext[16] = "";
        for (size_t i = part.filename.len; i > 0; --i) {
            if (part.filename.buf[i - 1] == '.') {
                size_t elen = part.filename.len - i;
                if (elen > 0 && elen < sizeof(ext)) {
                    size_t k = 0;
                    for (size_t j = i; j < part.filename.len && k < sizeof(ext) - 1; ++j) {
                        char c = part.filename.buf[j];
                        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                            (c >= '0' && c <= '9'))
                            ext[k++] = c;
                    }
                    ext[k] = '\0';
                }
                break;
            }
        }

        /* 确保目标目录存在 */
        utils_ensure_upload_dir(save_dir);

        /* 随机文件名：时间戳 + 随机码，杜绝覆盖与目录穿越 */
        char code[7];
        utils_gen_signin_code(code);
        char rel[256];
        if (ext[0])
            snprintf(rel, sizeof(rel), "%s/%ld_%s.%s",
                     save_dir, (long)time(NULL), code, ext);
        else
            snprintf(rel, sizeof(rel), "%s/%ld_%s",
                     save_dir, (long)time(NULL), code);

        /* 写入文件 */
        FILE *fp = fopen(rel, "wb");
        if (!fp) return 0;
        size_t w = fwrite(part.body.buf, 1, part.body.len, fp);
        fclose(fp);
        if (w != part.body.len) { remove(rel); return 0; }

        utils_strlcpy(out_path, rel, out_size);
        return 1;
    }
    return 0;
}

/* ═══════════════════════ 二、JSON 响应封装 ═══════════════════════ */

/**
 * api_send_json — 直接发送已拼好的 JSON 字符串
 * @ctx:  请求上下文
 * @json: 完整的 JSON 字符串
 *
 * 内部调用 mg_http_reply，附加 CORS 头和 UTF-8 编码声明
 */
void api_send_json(ApiContext *ctx, const char *json) {
    mg_http_reply(ctx->conn, 200,
                  "Content-Type: application/json; charset=utf-8\r\n"
                  "Access-Control-Allow-Origin: *\r\n",
                  "%s", json);
}

/**
 * api_ok — 返回标准成功响应 {"code":0,"msg":"ok"}
 * @ctx: 请求上下文
 */
void api_ok(ApiContext *ctx) {
    api_send_json(ctx, "{\"code\":0,\"msg\":\"ok\"}");
}

/**
 * api_ok_msg — 返回带自定义消息的成功响应
 * @ctx: 请求上下文
 * @msg: 自定义消息文本
 *
 * MG_ESC 宏自动转义 msg 中的引号和特殊字符
 */
void api_ok_msg(ApiContext *ctx, const char *msg) {
    mg_http_reply(ctx->conn, 200,
                  "Content-Type: application/json; charset=utf-8\r\n"
                  "Access-Control-Allow-Origin: *\r\n",
                  "{\"code\":0,\"msg\":%m}", MG_ESC(msg));
}

/**
 * api_ok_data — 返回带 data 字段的成功响应
 * @ctx:       请求上下文
 * @data_json: 已拼好的 data 对象 JSON 字符串（不含外层大括号）
 *
 * 最终响应格式：{"code":0,"msg":"ok","data":<data_json>}
 */
void api_ok_data(ApiContext *ctx, const char *data_json) {
    mg_http_reply(ctx->conn, 200,
                  "Content-Type: application/json; charset=utf-8\r\n"
                  "Access-Control-Allow-Origin: *\r\n",
                  "{\"code\":0,\"msg\":\"ok\",\"data\":%s}", data_json);
}

/**
 * api_error — 返回错误响应
 * @ctx:  请求上下文
 * @code: 业务错误码（非 0）
 * @msg:  错误描述文本
 *
 * HTTP 状态码统一返回 200，前端根据 JSON 中的 code 字段判断成功与否
 */
void api_error(ApiContext *ctx, int code, const char *msg) {
    mg_http_reply(ctx->conn, 200,
                  "Content-Type: application/json; charset=utf-8\r\n"
                  "Access-Control-Allow-Origin: *\r\n",
                  "{\"code\":%d,\"msg\":%m}", code, MG_ESC(msg));
}

/* ═══════════════════════ 三、结果集转 JSON ═══════════════════════ */

/**
 * append_json_escaped — 将字符串按 JSON 转义规则追加到动态缓冲区
 * @buf: 缓冲区指针的指针（可能因 realloc 改变）
 * @len: 当前已用长度指针
 * @cap: 当前容量指针
 * @s:   待转义的源字符串
 *
 * 处理双引号、反斜杠、换行、回车、制表符等 JSON 特殊字符
 */
static void append_json_escaped(char **buf, size_t *len, size_t *cap, const char *s) {
    if (!s) s = "";
    for (const char *p = s; *p; ++p) {
        /* 最坏情况一个字符扩成 6 字节（\u00XX），预留 8 */
        if (*len + 8 >= *cap) {
            *cap = (*cap) * 2 + 64;
            *buf = realloc(*buf, *cap);
        }
        unsigned char c = (unsigned char)*p;
        switch (c) {
            case '"':  (*buf)[(*len)++] = '\\'; (*buf)[(*len)++] = '"';  break;
            case '\\': (*buf)[(*len)++] = '\\'; (*buf)[(*len)++] = '\\'; break;
            case '\n': (*buf)[(*len)++] = '\\'; (*buf)[(*len)++] = 'n';  break;
            case '\r': (*buf)[(*len)++] = '\\'; (*buf)[(*len)++] = 'r';  break;
            case '\t': (*buf)[(*len)++] = '\\'; (*buf)[(*len)++] = 't';  break;
            default:
                if (c < 0x20) {
                    *len += snprintf(*buf + *len, *cap - *len, "\\u%04x", c);
                } else {
                    (*buf)[(*len)++] = (char)c;
                }
        }
    }
}

/**
 * is_numeric_field — 判断 MySQL 字段是否为数值类型
 * @t: MySQL 字段类型枚举值
 * 返回值：1 = 数值类型（JSON 中不加引号），0 = 字符串类型
 */
static int is_numeric_field(enum enum_field_types t) {
    switch (t) {
        case MYSQL_TYPE_TINY:
        case MYSQL_TYPE_SHORT:
        case MYSQL_TYPE_LONG:
        case MYSQL_TYPE_LONGLONG:
        case MYSQL_TYPE_INT24:
        case MYSQL_TYPE_FLOAT:
        case MYSQL_TYPE_DOUBLE:
        case MYSQL_TYPE_DECIMAL:
        case MYSQL_TYPE_NEWDECIMAL:
            return 1;
        default:
            return 0;
    }
}

/**
 * db_result_to_json_array — 将 MySQL 结果集转为 JSON 数组字符串
 * @res: MySQL 结果集指针
 * 返回值：malloc 分配的 JSON 字符串，调用方负责 free
 *
 * 仅消耗游标遍历数据，不释放 res，由调用方负责 mysql_free_result。
 * 数值字段不加引号，NULL 值输出为 null。
 */
char *db_result_to_json_array(MYSQL_RES *res) {
    size_t cap = 256, len = 0;
    char *buf = malloc(cap);
    if (!buf) return NULL;
    buf[len++] = '[';

    if (res) {
        unsigned int ncol = mysql_num_fields(res);
        MYSQL_FIELD *fields = mysql_fetch_fields(res);
        MYSQL_ROW row;
        int first_row = 1;

        while ((row = mysql_fetch_row(res)) != NULL) {
            unsigned long *lens = mysql_fetch_lengths(res);
            (void)lens;
            if (cap - len < 4) { cap = cap * 2 + 64; buf = realloc(buf, cap); }
            if (!first_row) buf[len++] = ',';
            first_row = 0;
            buf[len++] = '{';

            for (unsigned int i = 0; i < ncol; ++i) {
                if (cap - len < 64) { cap = cap * 2 + 128; buf = realloc(buf, cap); }
                if (i > 0) buf[len++] = ',';
                /* 输出字段名（key） */
                buf[len++] = '"';
                append_json_escaped(&buf, &len, &cap, fields[i].name);
                buf[len++] = '"';
                buf[len++] = ':';
                /* 输出字段值（value） */
                if (row[i] == NULL) {
                    memcpy(buf + len, "null", 4); len += 4;
                } else if (is_numeric_field(fields[i].type)) {
                    /* 数值字段直接输出（数据库保证格式合法） */
                    size_t vl = strlen(row[i]);
                    if (cap - len < vl + 2) { cap = cap * 2 + vl + 64; buf = realloc(buf, cap); }
                    memcpy(buf + len, row[i], vl); len += vl;
                } else {
                    /* 字符串字段需要 JSON 转义并加引号 */
                    buf[len++] = '"';
                    append_json_escaped(&buf, &len, &cap, row[i]);
                    buf[len++] = '"';
                }
            }
            if (cap - len < 2) { cap = cap * 2 + 16; buf = realloc(buf, cap); }
            buf[len++] = '}';
        }
    }
    buf[len++] = ']';
    buf[len] = '\0';
    return buf;
}

/**
 * api_send_result — 执行 SQL 查询并直接返回结果集 JSON 数组
 * @ctx: 请求上下文
 * @res: MySQL 查询结果集
 *
 * 内部自动释放结果集，失败时返回错误响应
 */
void api_send_result(ApiContext *ctx, MYSQL_RES *res) {
    if (!res) {
        api_error(ctx, ERR_DB, "查询失败");
        return;
    }
    char *json = db_result_to_json_array(res);
    mysql_free_result(res);
    if (!json) { api_error(ctx, ERR_SYSTEM, "内存不足"); return; }
    api_ok_data(ctx, json);
    free(json);
}

/**
 * api_send_result_data — 同 api_send_result，发送结果集 JSON 数据
 * @ctx: 请求上下文
 * @res: MySQL 查询结果集
 */
void api_send_result_data(ApiContext *ctx, MYSQL_RES *res) {
    if (!res) {
        api_error(ctx, ERR_DB, "查询失败");
        return;
    }
    char *json = db_result_to_json_array(res);
    mysql_free_result(res);
    if (!json) { api_error(ctx, ERR_SYSTEM, "内存不足"); return; }
    api_ok_data(ctx, json);
    free(json);
}

/**
 * api_send_result_paged — 查询并返回带分页信息的结果集
 * @ctx:       请求上下文
 * @res:       MySQL 查询结果集（当前页数据）
 * @page:      当前页码
 * @page_size: 每页条数
 * @total:     总记录数
 *
 * 响应包含 data（数组）、page、page_size、total、total_pages 五个字段
 */
void api_send_result_paged(ApiContext *ctx, MYSQL_RES *res,
                           int page, int page_size, int total) {
    if (!res) {
        api_error(ctx, ERR_DB, "查询失败");
        return;
    }
    char *json = db_result_to_json_array(res);
    mysql_free_result(res);
    if (!json) { api_error(ctx, ERR_SYSTEM, "内存不足"); return; }

    int total_pages = (page_size > 0) ? (total + page_size - 1) / page_size : 0;
    mg_http_reply(ctx->conn, 200,
                  "Content-Type: application/json; charset=utf-8\r\n"
                  "Access-Control-Allow-Origin: *\r\n",
                  "{\"code\":0,\"msg\":\"ok\",\"data\":%s,"
                  "\"page\":%d,\"page_size\":%d,\"total\":%d,\"total_pages\":%d}",
                  json, page, page_size, total, total_pages);
    free(json);
}

/* ═══════════════════════ 四、JsonBuilder 手动拼装工具 ═══════════════════════ */

/**
 * json_init — 初始化 JsonBuilder
 * @jb: JsonBuilder 结构体指针
 *
 * 分配初始缓冲区，写入左花括号 '{'，first 标记置为 1
 */
void json_init(JsonBuilder *jb) {
    jb->cap = 256;
    jb->buf = malloc(jb->cap);
    jb->len = 0;
    jb->first = 1;
    jb->buf[jb->len++] = '{';
}

/**
 * jb_ensure — 确保 JsonBuilder 缓冲区有足够剩余空间
 * @jb:    JsonBuilder 结构体指针
 * @extra: 需要的额外字节数
 */
static void jb_ensure(JsonBuilder *jb, size_t extra) {
    if (jb->len + extra >= jb->cap) {
        while (jb->len + extra >= jb->cap) jb->cap = jb->cap * 2 + 64;
        jb->buf = realloc(jb->buf, jb->cap);
    }
}

/**
 * jb_key — 向缓冲区写入 JSON 键名（带引号和冒号）
 * @jb:  JsonBuilder 结构体指针
 * @key: 键名字符串
 *
 * 自动处理逗号分隔（首字段不加逗号）
 */
static void jb_key(JsonBuilder *jb, const char *key) {
    size_t klen = strlen(key);
    jb_ensure(jb, klen + 8);
    if (!jb->first) jb->buf[jb->len++] = ',';
    jb->first = 0;
    jb->buf[jb->len++] = '"';
    memcpy(jb->buf + jb->len, key, klen); jb->len += klen;
    jb->buf[jb->len++] = '"';
    jb->buf[jb->len++] = ':';
}

/**
 * json_add_str — 添加字符串字段
 * @jb:  JsonBuilder 指针
 * @key: 字段名
 * @val: 字段值（自动 JSON 转义）
 */
void json_add_str(JsonBuilder *jb, const char *key, const char *val) {
    jb_key(jb, key);
    jb_ensure(jb, 2);
    jb->buf[jb->len++] = '"';
    append_json_escaped(&jb->buf, &jb->len, &jb->cap, val ? val : "");
    jb_ensure(jb, 1);
    jb->buf[jb->len++] = '"';
}

/**
 * json_add_int — 添加整数字段
 * @jb:  JsonBuilder 指针
 * @key: 字段名
 * @val: 整数值
 */
void json_add_int(JsonBuilder *jb, const char *key, long val) {
    jb_key(jb, key);
    jb_ensure(jb, 32);
    jb->len += snprintf(jb->buf + jb->len, jb->cap - jb->len, "%ld", val);
}

/**
 * json_add_double — 添加浮点字段（保留两位小数）
 * @jb:  JsonBuilder 指针
 * @key: 字段名
 * @val: 浮点数值
 */
void json_add_double(JsonBuilder *jb, const char *key, double val) {
    jb_key(jb, key);
    jb_ensure(jb, 48);
    jb->len += snprintf(jb->buf + jb->len, jb->cap - jb->len, "%.2f", val);
}

/**
 * json_add_bool — 添加布尔字段
 * @jb:  JsonBuilder 指针
 * @key: 字段名
 * @val: 0 = false，非 0 = true
 */
void json_add_bool(JsonBuilder *jb, const char *key, int val) {
    jb_key(jb, key);
    const char *s = val ? "true" : "false";
    size_t sl = strlen(s);
    jb_ensure(jb, sl);
    memcpy(jb->buf + jb->len, s, sl); jb->len += sl;
}

/**
 * json_add_raw — 添加原始 JSON 值（不转义、不加引号）
 * @jb:  JsonBuilder 指针
 * @key: 字段名
 * @raw: 原始 JSON 字符串（如 "null"、"{}"、"{\"a\":1}" 等）
 *
 * 用于嵌入已拼好的 JSON 片段，如子对象或数组
 */
void json_add_raw(JsonBuilder *jb, const char *key, const char *raw) {
    jb_key(jb, key);
    size_t rl = strlen(raw ? raw : "null");
    jb_ensure(jb, rl);
    memcpy(jb->buf + jb->len, raw ? raw : "null", rl); jb->len += rl;
}

/**
 * json_finish — 完成 JSON 对象拼装，返回字符串指针
 * @jb: JsonBuilder 指针
 * 返回值：以 '\0' 结尾的完整 JSON 字符串
 *
 * 返回指针指向内部缓冲区，由 json_free 统一释放
 */
const char *json_finish(JsonBuilder *jb) {
    jb_ensure(jb, 2);
    jb->buf[jb->len++] = '}';
    jb->buf[jb->len] = '\0';
    return jb->buf;
}

/**
 * json_free — 释放 JsonBuilder 内部缓冲区
 * @jb: JsonBuilder 指针
 */
void json_free(JsonBuilder *jb) {
    if (jb->buf) { free(jb->buf); jb->buf = NULL; }
    jb->len = jb->cap = 0;
}

/* ═══════════════════════ 五、权限检查 ═══════════════════════ */

/**
 * api_require_login — 检查当前请求是否已登录
 * @ctx: 请求上下文
 * 返回值：1 = 已登录，0 = 未登录（已自动返回错误响应）
 */
int api_require_login(ApiContext *ctx) {
    if (!ctx->user) {
        api_error(ctx, ERR_AUTH, "请先登录");
        return 0;
    }
    return 1;
}

/**
 * api_require_role — 检查当前用户是否具有指定角色
 * @ctx:  请求上下文
 * @role: 要求的角色名
 * 返回值：1 = 角色匹配，0 = 不匹配或无权限
 */
int api_require_role(ApiContext *ctx, const char *role) {
    if (!api_require_login(ctx)) return 0;
    if (!utils_str_equal(ctx->user->role, role)) {
        api_error(ctx, ERR_PERMISSION, "无权限执行此操作");
        return 0;
    }
    return 1;
}

/**
 * api_require_school_admin — 检查当前用户是否为学校管理员
 * @ctx: 请求上下文
 * 返回值：1 = 是学校管理员，0 = 否
 */
int api_require_school_admin(ApiContext *ctx) {
    return api_require_role(ctx, "school_admin");
}

/**
 * api_require_college_admin — 检查当前用户是否为学院管理员
 * @ctx: 请求上下文
 * 返回值：1 = 是学院管理员（或学校管理员），0 = 否
 *
 * 学校管理员拥有学院管理员的一切权限
 */
int api_require_college_admin(ApiContext *ctx) {
    if (!api_require_login(ctx)) return 0;
    /* 学校管理员拥有学院管理员的一切权限 */
    if (utils_str_equal(ctx->user->role, "school_admin") ||
        utils_str_equal(ctx->user->role, "college_admin"))
        return 1;
    api_error(ctx, ERR_PERMISSION, "需要学院管理员权限");
    return 0;
}

/**
 * api_require_club_admin — 检查当前用户是否为指定社团的管理员（社长/副社长）
 * @ctx:     请求上下文
 * @club_id: 社团 ID
 * 返回值：1 = 是社团管理员或学校管理员，0 = 否
 *
 * 学校管理员对所有社团拥有管理员权限。
 * 普通用户需在该社团担任社长或副社长，且当前在籍。
 */
int api_require_club_admin(ApiContext *ctx, int club_id) {
    if (!api_require_login(ctx)) return 0;
    /* 学校管理员对所有社团拥有管理员权限 */
    if (utils_str_equal(ctx->user->role, "school_admin")) return 1;

    /* 校验当前用户是否为该社团的社长或副社长，且在籍 */
    int cnt = db_query_int(
        "SELECT COUNT(*) FROM members "
        "WHERE club_id=%d AND user_id=%d AND left_at IS NULL "
        "AND join_status='approved' AND role IN ('president','vice_president')",
        club_id, ctx->user->user_id);
    if (cnt > 0) return 1;

    api_error(ctx, ERR_PERMISSION, "您不是该社团的管理员");
    return 0;
}

/* ═══════════════════════ 六、文件上传 ═══════════════════════ */

/**
 * api_save_upload — 保存上传的文件到本地存储目录
 * @ctx:        请求上下文
 * @field_name: multipart 表单中的文件字段名
 * @out_path:   输出缓冲区，写入保存后的相对路径
 * @out_size:   输出缓冲区大小
 * 返回值：OK(0) = 成功，ERR_SYSTEM = 写文件失败，ERR_NOT_FOUND = 未找到文件字段
 *
 * 自动生成随机文件名防止覆盖，文件保存到 storage/receipts/ 目录
 */
int api_save_upload(ApiContext *ctx, const char *field_name,
                    char *out_path, size_t out_size) {
    struct mg_http_part part;
    size_t ofs = 0;
    int found = 0;

    while ((ofs = mg_http_next_multipart(ctx->hm->body, ofs, &part)) > 0) {
        if (part.name.len == strlen(field_name) &&
            strncmp(part.name.buf, field_name, part.name.len) == 0) {
            found = 1;

            /* 取原文件扩展名 */
            char ext[16] = ".dat";
            if (part.filename.len > 0) {
                const char *fn = part.filename.buf;
                for (size_t i = part.filename.len; i > 0; --i) {
                    if (fn[i-1] == '.') {
                        size_t elen = part.filename.len - (i - 1);
                        if (elen < sizeof(ext)) {
                            memcpy(ext, fn + i - 1, elen);
                            ext[elen] = '\0';
                        }
                        break;
                    }
                }
            }

            /* 生成 16 位随机文件名，避免覆盖 */
            char rnd[17];
            utils_gen_random_string(rnd, 16);

            char rel[256];
            snprintf(rel, sizeof(rel), "storage/receipts/%s%s", rnd, ext);

            char full[512];
            snprintf(full, sizeof(full), "%s", rel);

            /* 写文件到磁盘 */
            FILE *fp = fopen(full, "wb");
            if (!fp) return ERR_SYSTEM;
            fwrite(part.body.buf, 1, part.body.len, fp);
            fclose(fp);

            utils_strlcpy(out_path, rel, out_size);
            return OK;
        }
    }
    (void)found;
    return ERR_NOT_FOUND;
}
