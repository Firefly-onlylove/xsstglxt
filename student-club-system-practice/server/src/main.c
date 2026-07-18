/**
 * main.c — 程序入口：Mongoose HTTP 服务器 + 路由分发 + 会话
 * 学生社团管理系统 SCMS（Web 架构）
 *
 * 运行流程：
 *   1. 读取 db_config.ini
 *   2. 初始化数据库连接池、会话池
 *   3. 启动 HTTP 服务器，监听 :8000
 *   4. 每个请求：
 *        - 静态资源(.html/.css/.js等) → 从 web/ 目录返回
 *        - /api/login、/api/logout → 内建处理
 *        - 其他 /api/ 开头的请求 → 解析 Cookie 还原登录用户 → 匹配路由表 → 调用处理函数
 *        - /receipts/ 开头的请求 → 返回发票图片
 *
 * 编译产物：scms.exe —— 单一可执行文件，既是网页服务器又是 API 服务器。
 * 浏览器访问 http://<服务器IP>:8000 即可使用。
 */
#include "mongoose.h"
#include "../include/common.h"
#include "../include/db.h"
#include "../include/auth.h"
#include "../include/api.h"
#include "../include/routes.h"
#include "../include/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

/* ─────────────────────────── 全局配置 ─────────────────────────── */

#define HTTP_PORT          "8000"
#define WEB_ROOT           "public"        /* 前端静态文件目录 */
#define RECEIPTS_ROOT      "receipts"     /* 发票图片目录 */
#define CONFIG_FILE        "db_config.ini"
#define COOKIE_NAME        "scms_token"

static volatile int s_signo = 0;
static void signal_handler(int signo) { s_signo = signo; }

/* ─────────────────────────── 配置解析 ─────────────────────────── */

/* 去除行首尾空白（含 \r\n） */
static char *trim_line(char *s) {
    while (*s == ' ' || *s == '\t') s++;
    char *end = s + strlen(s);
    while (end > s && (end[-1] == '\n' || end[-1] == '\r' ||
                       end[-1] == ' '  || end[-1] == '\t')) *--end = '\0';
    return s;
}

/* 读取 db_config.ini 的 [database] 段。失败返回 -1（用默认值仍可继续）。 */
static int load_config(const char *path, DBConfig *cfg) {
    /* 默认值 */
    utils_strlcpy(cfg->host, "127.0.0.1", sizeof(cfg->host));
    cfg->port = 3306;
    utils_strlcpy(cfg->user, "root", sizeof(cfg->user));
    utils_strlcpy(cfg->password, "", sizeof(cfg->password));
    utils_strlcpy(cfg->database, "scms_db", sizeof(cfg->database));
    cfg->pool_size = 5;

    FILE *fp = fopen(path, "r");
    if (!fp) {
        fprintf(stderr, "[WARN] 无法打开 %s，使用默认数据库配置。\n", path);
        return -1;
    }

    char line[512];
    char section[64] = "";
    while (fgets(line, sizeof(line), fp)) {
        char *s = trim_line(line);
        if (s[0] == '\0' || s[0] == '#' || s[0] == ';') continue;
        if (s[0] == '[') {
            char *rb = strchr(s, ']');
            if (rb) { *rb = '\0'; utils_strlcpy(section, s + 1, sizeof(section)); }
            continue;
        }
        char *eq = strchr(s, '=');
        if (!eq) continue;
        *eq = '\0';
        char *key = trim_line(s);
        char *val = trim_line(eq + 1);

        if (strcmp(section, "database") == 0) {
            if      (strcmp(key, "host") == 0)      utils_strlcpy(cfg->host, val, sizeof(cfg->host));
            else if (strcmp(key, "port") == 0)      cfg->port = atoi(val);
            else if (strcmp(key, "user") == 0)      utils_strlcpy(cfg->user, val, sizeof(cfg->user));
            else if (strcmp(key, "password") == 0)  utils_strlcpy(cfg->password, val, sizeof(cfg->password));
            else if (strcmp(key, "database") == 0)  utils_strlcpy(cfg->database, val, sizeof(cfg->database));
            else if (strcmp(key, "pool_size") == 0) cfg->pool_size = atoi(val);
        }
    }
    fclose(fp);
    return 0;
}

/* ─────────────────────────── 会话 Cookie ─────────────────────────── */

/* 从请求 Cookie 中取出 token，写入 out（未找到置空）。 */
static void get_token_from_cookie(struct mg_http_message *hm, char *out, size_t out_size) {
    out[0] = '\0';
    struct mg_str *cookie = mg_http_get_header(hm, "Cookie");
    if (!cookie) return;
    struct mg_str v = mg_http_get_header_var(*cookie, mg_str(COOKIE_NAME));
    if (v.len > 0 && v.len < out_size) {
        memcpy(out, v.buf, v.len);
        out[v.len] = '\0';
    }
}

/* ─────────────────────────── 路由匹配 ─────────────────────────── */

/* 模式匹配：pattern 支持 '*' 匹配单个路径段（不跨 '/'）。
 * 例：pattern="/api/school/clubs/star"  uri="/api/school/clubs/12" 匹配
 *     pattern="/api/school/clubs/star/approve" uri="/api/school/clubs/12/approve" 匹配 */
static int uri_match(const char *pattern, struct mg_str uri) {
    const char *p = pattern;
    const char *u = uri.buf;
    const char *uend = uri.buf + uri.len;

    while (*p) {
        if (*p == '*') {
            p++;                        /* 跳过 '*' */
            /* 通配当前段：吃掉 uri 里直到下一个 '/' 的内容 */
            while (u < uend && *u != '/') u++;
        } else {
            if (u >= uend || *p != *u) return 0;
            p++; u++;
        }
    }
    return (u == uend) ? 1 : 0;
}

/* 方法匹配：route->method 为 NULL 表示任意方法 */
static int method_match(const char *rm, struct mg_str method) {
    if (!rm) return 1;
    return (method.len == strlen(rm) &&
            strncmp(method.buf, rm, method.len) == 0);
}

/* ─────────────────────────── 内建接口 ─────────────────────────── */

/* POST /api/login  body: username, password（表单或 JSON 均可） */
static void handle_login(ApiContext *ctx) {
    char username[64] = "", password[128] = "";

    /* 优先 JSON，退化到表单 */
    if (!api_get_json_str(ctx, "username", username, sizeof(username)))
        api_get_param(ctx, "username", username, sizeof(username));
    if (!api_get_json_str(ctx, "password", password, sizeof(password)))
        api_get_param(ctx, "password", password, sizeof(password));

    if (username[0] == '\0' || password[0] == '\0') {
        api_error(ctx, ERR_INPUT, "请输入用户名和密码");
        return;
    }

    User u;
    char token[SESSION_TOKEN_LEN + 1];
    int rc = auth_login(username, password, &u, token);

    if (rc == ERR_AUTH) { api_error(ctx, ERR_AUTH, "用户名或密码错误"); return; }
    if (rc == ERR_STATUS) { api_error(ctx, ERR_STATUS, "账号已被禁用，请联系管理员"); return; }
    if (rc != OK) { api_error(ctx, ERR_DB, "登录失败，请稍后重试"); return; }

    /* 记录登录日志 */
    db_execute("INSERT INTO logs (user_id, action, target_type, target_id, detail) "
               "VALUES (%d, 'login', 'users', %d, '用户登录')", u.user_id, u.user_id);

    /* 下发 Cookie + 返回用户基本信息（供前端分流） */
    char cookie_hdr[160];
    snprintf(cookie_hdr, sizeof(cookie_hdr),
             "Content-Type: application/json; charset=utf-8\r\n"
             "Set-Cookie: %s=%s; Path=/; HttpOnly; SameSite=Lax\r\n",
             COOKIE_NAME, token);

    mg_http_reply(ctx->conn, 200, cookie_hdr,
        "{\"code\":0,\"msg\":\"ok\",\"data\":{"
        "\"user_id\":%d,\"username\":%m,\"real_name\":%m,\"role\":%m,"
        "\"college_id\":%d}}",
        u.user_id, MG_ESC(u.username), MG_ESC(u.real_name),
        MG_ESC(u.role), u.college_id);
}

/* POST /api/logout */
static void handle_logout(ApiContext *ctx) {
    char token[SESSION_TOKEN_LEN + 1];
    get_token_from_cookie(ctx->hm, token, sizeof(token));
    if (token[0]) auth_logout(token);
    mg_http_reply(ctx->conn, 200,
        "Set-Cookie: " COOKIE_NAME "=; Path=/; Max-Age=0\r\n"
        "Content-Type: application/json; charset=utf-8\r\n",
        "{\"code\":0,\"msg\":\"已登出\"}");
}

/* GET /api/me  返回当前登录用户（未登录返回错误码，前端据此跳登录页） */
static void handle_me(ApiContext *ctx) {
    if (!ctx->user) { api_error(ctx, ERR_AUTH, "未登录"); return; }
    User *u = ctx->user;
    mg_http_reply(ctx->conn, 200,
        "Content-Type: application/json; charset=utf-8\r\n",
        "{\"code\":0,\"msg\":\"ok\",\"data\":{"
        "\"user_id\":%d,\"username\":%m,\"real_name\":%m,\"role\":%m,"
        "\"college_id\":%d,\"college_name\":%m}}",
        u->user_id, MG_ESC(u->username), MG_ESC(u->real_name),
        MG_ESC(u->role), u->college_id, MG_ESC(u->college_name));
}

/* ─────────────────────────── 请求分发 ─────────────────────────── */

static void dispatch_api(struct mg_connection *c, struct mg_http_message *hm) {
    ApiContext ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.conn = c;
    ctx.hm = hm;
    ctx.user = NULL;
    size_t mlen = hm->method.len < sizeof(ctx.method) - 1 ? hm->method.len : sizeof(ctx.method) - 1;
    memcpy(ctx.method, hm->method.buf, mlen);
    ctx.method[mlen] = '\0';

    /* 解析会话，还原当前用户（除 login/register 外的接口都可能用到） */
    User current;
    memset(&current, 0, sizeof(current));
    char token[SESSION_TOKEN_LEN + 1];
    get_token_from_cookie(hm, token, sizeof(token));
    if (token[0] && auth_resolve_session(token, &current)) {
        ctx.user = &current;
    }

    /* ── 内建接口优先 ── */
    if (uri_match("/api/login", hm->uri) && method_match("POST", hm->method)) {
        handle_login(&ctx); return;
    }
    if (uri_match("/api/logout", hm->uri)) { handle_logout(&ctx); return; }
    if (uri_match("/api/me", hm->uri))     { handle_me(&ctx); return; }

    /* ── 业务路由表 ── */
    int n = 0;
    const Route *routes = routes_table(&n);
    for (int i = 0; i < n; ++i) {
        if (method_match(routes[i].method, hm->method) &&
            uri_match(routes[i].pattern, hm->uri)) {
            if (routes[i].need_login && !ctx.user) {
                api_error(&ctx, ERR_AUTH, "请先登录");
                return;
            }
            routes[i].handler(&ctx);
            return;
        }
    }

    /* 未匹配任何路由 */
    api_error(&ctx, ERR_NOT_FOUND, "接口不存在");
}

/* ─────────────────────────── Mongoose 事件回调 ─────────────────────────── */

static void ev_handler(struct mg_connection *c, int ev, void *ev_data) {
    if (ev != MG_EV_HTTP_MSG) return;
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;

    /* CORS 预检 */
    if (mg_strcmp(hm->method, mg_str("OPTIONS")) == 0) {
        mg_http_reply(c, 204,
            "Access-Control-Allow-Origin: *\r\n"
            "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n"
            "Access-Control-Allow-Headers: Content-Type\r\n", "");
        return;
    }

    /* /api/ 前缀 → 业务分发 */
    if (mg_match(hm->uri, mg_str("/api/#"), NULL)) {
        dispatch_api(c, hm);
        return;
    }

    /* /receipts/ 前缀 → 发票图片（登录用户可看，简化为直接返回文件） */
    if (mg_match(hm->uri, mg_str("/receipts/#"), NULL)) {
        struct mg_http_serve_opts opts = { .root_dir = "storage" };
        mg_http_serve_dir(c, hm, &opts);
        return;
    }

    /* 其它 → 前端静态资源 */
    struct mg_http_serve_opts opts;
    memset(&opts, 0, sizeof(opts));
    opts.root_dir = WEB_ROOT;
    mg_http_serve_dir(c, hm, &opts);
}

/* ─────────────────────────── main ─────────────────────────── */

int main(void) {
    printf("======================================================\n");
    printf("   学生社团管理系统 SCMS —— 服务端启动中...\n");
    printf("======================================================\n");

    /* 1. 读配置 */
    DBConfig cfg;
    load_config(CONFIG_FILE, &cfg);
    printf("[INFO] 数据库: %s@%s:%d/%s (pool=%d)\n",
           cfg.user, cfg.host, cfg.port, cfg.database, cfg.pool_size);

    /* 2. 初始化数据库连接池 */
    if (db_init(&cfg) != OK) {
        fprintf(stderr, "[FATAL] 数据库连接失败: %s\n", db_error());
        fprintf(stderr, "        请检查 MySQL 是否启动、db_config.ini 是否正确。\n");
        return 1;
    }
    printf("[INFO] 数据库连接成功。\n");

    /* 3. 初始化会话池 */
    auth_init();

    /* 4. 启动 HTTP 服务器 */
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    struct mg_mgr mgr;
    mg_mgr_init(&mgr);

    char url[64];
    snprintf(url, sizeof(url), "http://0.0.0.0:%s", HTTP_PORT);
    struct mg_connection *c = mg_http_listen(&mgr, url, ev_handler, NULL);
    if (c == NULL) {
        fprintf(stderr, "[FATAL] 无法监听端口 %s（可能被占用）。\n", HTTP_PORT);
        db_close();
        auth_cleanup();
        mg_mgr_free(&mgr);
        return 1;
    }

    printf("[INFO] 服务已启动：\n");
    printf("       本机访问   → http://127.0.0.1:%s\n", HTTP_PORT);
    printf("       局域网访问 → http://<本机IP>:%s\n", HTTP_PORT);
    printf("       (按 Ctrl+C 停止)\n");
    printf("======================================================\n");

    /* 5. 事件循环 */
    while (s_signo == 0) {
        mg_mgr_poll(&mgr, 200);
    }

    /* 6. 清理 */
    printf("\n[INFO] 收到停止信号，正在关闭...\n");
    mg_mgr_free(&mgr);
    auth_cleanup();
    db_close();
    printf("[INFO] 已安全退出。\n");
    return 0;
}
