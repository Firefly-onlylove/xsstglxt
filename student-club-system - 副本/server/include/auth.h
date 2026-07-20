/**
 * auth.h — 认证与会话管理接口
 * 学生社团管理系统 SCMS（Web 架构）
 *
 * Web 应用无状态，使用会话令牌维持登录态：
 *   登录成功 → 生成随机令牌，服务器内存中保存令牌到用户的映射，
 *   通过 Set-Cookie 下发给浏览器；后续请求携带 Cookie，服务器据此还原当前用户。
 *
 * 业务模块通常不直接调用本层：main.c 已在每次请求前
 * 调用 auth_resolve_session 把当前用户填入 ctx->user。
 */
#ifndef AUTH_H
#define AUTH_H

#include "models.h"

/** 会话令牌的十六进制字符串长度（不含结束符） */
#define SESSION_TOKEN_LEN 32

/* ── 登录 / 登出 ── */

/** 用户登录校验
 *  校验用户名和密码，成功则生成会话令牌。
 *  @param username 登录用户名
 *  @param password 明文密码
 *  @param out_user 输出参数，登录成功时填充用户完整信息
 *  @param out_token 输出参数，登录成功时填充生成的会话令牌（至少 SESSION_TOKEN_LEN+1 字节）
 *  @return 成功返回 OK，账号被禁用返回 ERR_STATUS，密码错误返回 ERR_AUTH */
int  auth_login(const char *username, const char *password,
                User *out_user, char *out_token);

/** 销毁会话（登出）
 *  @param token 要销毁的会话令牌 */
void auth_logout(const char *token);

/* ── 会话解析 ── */

/** 根据令牌查找已登录用户
 *  main.c 每次收到请求时调用，用于填充 ctx->user。
 *  @param token 客户端 Cookie 中携带的会话令牌
 *  @param out_user 输出参数，命中时填充用户信息
 *  @return 令牌有效返回 1，无效或过期返回 0 */
int  auth_resolve_session(const char *token, User *out_user);

/* ── 用户加载 ── */

/** 按用户 ID 从数据库加载完整用户信息
 *  包含 JOIN 查询的学院名、专业名、班级名。
 *  @param user_id 要加载的用户 ID
 *  @param out_user 输出参数，填充用户完整信息
 *  @return 成功返回 OK，用户不存在返回 ERR_NOT_FOUND */
int  auth_load_user(int user_id, User *out_user);

/* ── 会话池生命周期 ── */

/** 初始化会话池（哈希表），程序启动时调用 */
void auth_init(void);

/** 清理会话池，释放所有内存，程序退出时调用 */
void auth_cleanup(void);

#endif /* AUTH_H */
