/**
 * auth.h — 认证与会话管理接口
 * 学生社团管理系统 SCMS（Web 架构）
 *
 * Web 无状态，用 session token 维持登录态：
 *   登录成功 → 生成随机 token，服务器内存保存 token→user 映射，
 *   通过 Set-Cookie 下发；后续请求带 Cookie，服务器据此还原当前用户。
 *
 * 业务模块通常不直接调用本层：main.c 已在每次请求前
 * 调用 auth_resolve_session 把当前用户放进 ctx->user。
 */
#ifndef AUTH_H
#define AUTH_H

#include "models.h"

#define SESSION_TOKEN_LEN 32          /* token 十六进制长度 */

/* ── 登录 / 登出 ──
 * auth_login：校验用户名+密码，成功返回 OK 并填充 out_user、生成 out_token；
 *             账号禁用返回 ERR_STATUS，密码错返回 ERR_AUTH。
 */
int  auth_login(const char *username, const char *password,
                User *out_user, char *out_token);
void auth_logout(const char *token);       /* 销毁会话 */

/* ── 会话解析 ──
 * 根据 token 查找已登录用户，命中返回 1 并填充 out_user，未命中返回 0。
 * main.c 每次请求调用，用于填充 ctx->user。
 */
int  auth_resolve_session(const char *token, User *out_user);

/* ── 用户加载 ──
 * 按 user_id 从数据库加载完整用户信息（含 JOIN 的学院/专业/班级名）。
 * 命中返回 OK，未找到返回 ERR_NOT_FOUND。
 */
int  auth_load_user(int user_id, User *out_user);

/* ── 会话池生命周期 ── */
void auth_init(void);                       /* 程序启动时调用 */
void auth_cleanup(void);                     /* 程序退出时调用 */

#endif /* AUTH_H */
