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

/* 确保上传目录存在（不存在则创建）。仅处理单层目录名，够用即可。 */
static void utils_ensure_upload_dir(const char *dir) {
    if (!dir || !dir[0]) return;
    struct stat st;
    if (stat(dir, &st) == 0) return;   /* 已存在 */
    scms_mkdir(dir);
}

/* ═══════════════════════ 一、读取请求参数 ═══════════════════════ */

int api_get_param(ApiContext *ctx, const char *key, char *out, size_t out_size) {
    if (out_size == 0) return 0;
    out[0] = '\0';

    /* 先查 query string（GET） */
    struct mg_str *q = &ctx->hm->query;
    if (q->len > 0) {
        int n = mg_http_get_var(q, key, out, out_size);
        if (n > 0) return 1;
    }
    /* 再查 body（POST 表单 application/x-www-form-urlencoded） */
    if (ctx->hm->body.len > 0) {
        int n = mg_http_get_var(&ctx->hm->body, key, out, out_size);
        if (n > 0) return 1;
    }
    out[0] = '\0';
    return 0;
}

int api_get_int(ApiContext *ctx, const char *key, int def_val) {
    char buf[32];
    if (api_get_param(ctx, key, buf, sizeof(buf)) && buf[0])
        return atoi(buf);
    return def_val;
}

double api_get_double(ApiContext *ctx, const char *key, double def_val) {
    char buf[64];
    if (api_get_param(ctx, key, buf, sizeof(buf)) && buf[0])
        return atof(buf);
    return def_val;
}

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

int api_get_json_int(ApiContext *ctx, const char *key, int def_val) {
    if (ctx->hm->body.len == 0) return def_val;
    char path[128];
    snprintf(path, sizeof(path), "$.%s", key);
    double d;
    if (mg_json_get_num(ctx->hm->body, path, &d))
        return (int)d;
    return def_val;
}

double api_get_json_double(ApiContext *ctx, const char *key, double def_val) {
    if (ctx->hm->body.len == 0) return def_val;
    char path[128];
    snprintf(path, sizeof(path), "$.%s", key);
    double d;
    if (mg_json_get_num(ctx->hm->body, path, &d))
        return d;
    return def_val;
}

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
 * 前端用 <form enctype="multipart/form-data"> 或 FormData 提交时，
 * 普通字段和文件都在同一个 body 里，需用 multipart 解析，不能用 mg_http_get_var。
 */

/* 读取 multipart 表单里的普通文本字段到 out。找到返回 1，否则返回 0。 */
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

/* 读取 multipart 表单里的数值字段，未找到返回 def_val。 */
double api_get_form_double(ApiContext *ctx, const char *key, double def_val) {
    char buf[64];
    if (api_get_form_str(ctx, key, buf, sizeof(buf)) && buf[0])
        return atof(buf);
    return def_val;
}

/* 保存 multipart 表单里的上传文件到本地。
 *   field    ：文件字段名（如 "receipt"）
 *   save_dir ：目标目录（如 "receipts"），不存在会尝试创建
 *   out_path ：写回最终保存的相对路径（供入库）
 * 成功返回 1，无文件或写盘失败返回 0。文件名做随机化，避免覆盖与路径穿越。 */
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

        /* 取原文件扩展名（只留字母数字，防注入） */
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

void api_send_json(ApiContext *ctx, const char *json) {
    mg_http_reply(ctx->conn, 200,
                  "Content-Type: application/json; charset=utf-8\r\n"
                  "Access-Control-Allow-Origin: *\r\n",
                  "%s", json);
}

void api_ok(ApiContext *ctx) {
    api_send_json(ctx, "{\"code\":0,\"msg\":\"ok\"}");
}

void api_ok_msg(ApiContext *ctx, const char *msg) {
    /* 转义 msg 中的引号 */
    mg_http_reply(ctx->conn, 200,
                  "Content-Type: application/json; charset=utf-8\r\n"
                  "Access-Control-Allow-Origin: *\r\n",
                  "{\"code\":0,\"msg\":%m}", MG_ESC(msg));
}

void api_ok_data(ApiContext *ctx, const char *data_json) {
    mg_http_reply(ctx->conn, 200,
                  "Content-Type: application/json; charset=utf-8\r\n"
                  "Access-Control-Allow-Origin: *\r\n",
                  "{\"code\":0,\"msg\":\"ok\",\"data\":%s}", data_json);
}

void api_error(ApiContext *ctx, int code, const char *msg) {
    mg_http_reply(ctx->conn, 200,
                  "Content-Type: application/json; charset=utf-8\r\n"
                  "Access-Control-Allow-Origin: *\r\n",
                  "{\"code\":%d,\"msg\":%m}", code, MG_ESC(msg));
}

/* ═══════════════════════ 三、结果集转 JSON ═══════════════════════ */

/* 把一个字符串按 JSON 规则转义并追加到动态缓冲区 */
static void append_json_escaped(char **buf, size_t *len, size_t *cap, const char *s) {
    if (!s) s = "";
    for (const char *p = s; *p; ++p) {
        /* 最坏情况一个字符扩成 6 字节(\u00XX)，预留 8 */
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

/* 判断某列是否为数值类型（决定 JSON 里是否加引号） */
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

/* 将结果集转为 JSON 数组字符串（malloc，调用者 free）。声明见 db.h。
 * 注意：只消耗游标，不释放 res，由调用方负责 mysql_free_result。 */
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
                /* key */
                buf[len++] = '"';
                append_json_escaped(&buf, &len, &cap, fields[i].name);
                buf[len++] = '"';
                buf[len++] = ':';
                /* value */
                if (row[i] == NULL) {
                    memcpy(buf + len, "null", 4); len += 4;
                } else if (is_numeric_field(fields[i].type)) {
                    /* 数值直接输出（数据库保证格式合法） */
                    size_t vl = strlen(row[i]);
                    if (cap - len < vl + 2) { cap = cap * 2 + vl + 64; buf = realloc(buf, cap); }
                    memcpy(buf + len, row[i], vl); len += vl;
                } else {
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

/* ═══════════════════════ 四、JsonBuilder ═══════════════════════ */

void json_init(JsonBuilder *jb) {
    jb->cap = 256;
    jb->buf = malloc(jb->cap);
    jb->len = 0;
    jb->first = 1;
    jb->buf[jb->len++] = '{';
}

static void jb_ensure(JsonBuilder *jb, size_t extra) {
    if (jb->len + extra >= jb->cap) {
        while (jb->len + extra >= jb->cap) jb->cap = jb->cap * 2 + 64;
        jb->buf = realloc(jb->buf, jb->cap);
    }
}

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

void json_add_str(JsonBuilder *jb, const char *key, const char *val) {
    jb_key(jb, key);
    jb_ensure(jb, 2);
    jb->buf[jb->len++] = '"';
    append_json_escaped(&jb->buf, &jb->len, &jb->cap, val ? val : "");
    jb_ensure(jb, 1);
    jb->buf[jb->len++] = '"';
}

void json_add_int(JsonBuilder *jb, const char *key, long val) {
    jb_key(jb, key);
    jb_ensure(jb, 32);
    jb->len += snprintf(jb->buf + jb->len, jb->cap - jb->len, "%ld", val);
}

void json_add_double(JsonBuilder *jb, const char *key, double val) {
    jb_key(jb, key);
    jb_ensure(jb, 48);
    jb->len += snprintf(jb->buf + jb->len, jb->cap - jb->len, "%.2f", val);
}

void json_add_bool(JsonBuilder *jb, const char *key, int val) {
    jb_key(jb, key);
    const char *s = val ? "true" : "false";
    size_t sl = strlen(s);
    jb_ensure(jb, sl);
    memcpy(jb->buf + jb->len, s, sl); jb->len += sl;
}

void json_add_raw(JsonBuilder *jb, const char *key, const char *raw) {
    jb_key(jb, key);
    size_t rl = strlen(raw ? raw : "null");
    jb_ensure(jb, rl);
    memcpy(jb->buf + jb->len, raw ? raw : "null", rl); jb->len += rl;
}

const char *json_finish(JsonBuilder *jb) {
    jb_ensure(jb, 2);
    jb->buf[jb->len++] = '}';
    jb->buf[jb->len] = '\0';
    return jb->buf;
}

void json_free(JsonBuilder *jb) {
    if (jb->buf) { free(jb->buf); jb->buf = NULL; }
    jb->len = jb->cap = 0;
}

/* ═══════════════════════ 五、权限检查 ═══════════════════════ */

int api_require_login(ApiContext *ctx) {
    if (!ctx->user) {
        api_error(ctx, ERR_AUTH, "请先登录");
        return 0;
    }
    return 1;
}

int api_require_role(ApiContext *ctx, const char *role) {
    if (!api_require_login(ctx)) return 0;
    if (!utils_str_equal(ctx->user->role, role)) {
        api_error(ctx, ERR_PERMISSION, "无权限执行此操作");
        return 0;
    }
    return 1;
}

int api_require_school_admin(ApiContext *ctx) {
    return api_require_role(ctx, "school_admin");
}

int api_require_college_admin(ApiContext *ctx) {
    if (!api_require_login(ctx)) return 0;
    /* 学校管理员拥有学院管理员的一切权限 */
    if (utils_str_equal(ctx->user->role, "school_admin") ||
        utils_str_equal(ctx->user->role, "college_admin"))
        return 1;
    api_error(ctx, ERR_PERMISSION, "需要学院管理员权限");
    return 0;
}

int api_require_club_admin(ApiContext *ctx, int club_id) {
    if (!api_require_login(ctx)) return 0;
    /* 学校管理员直接放行 */
    if (utils_str_equal(ctx->user->role, "school_admin")) return 1;

    /* 校验当前用户是否为该社团的社长/副社长且在籍 */
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

int api_save_upload(ApiContext *ctx, const char *field_name,
                    char *out_path, size_t out_size) {
    struct mg_http_part part;
    size_t ofs = 0;
    int found = 0;

    while ((ofs = mg_http_next_multipart(ctx->hm->body, ofs, &part)) > 0) {
        if (part.name.len == strlen(field_name) &&
            strncmp(part.name.buf, field_name, part.name.len) == 0) {
            found = 1;

            /* 取扩展名 */
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

            /* 生成随机文件名，避免覆盖 */
            char rnd[17];
            utils_gen_random_string(rnd, 16);

            char rel[256];
            snprintf(rel, sizeof(rel), "storage/receipts/%s%s", rnd, ext);

            char full[512];
            snprintf(full, sizeof(full), "%s", rel);

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
