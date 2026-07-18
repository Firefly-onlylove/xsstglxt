/**
 * common.h — 全局通用定义
 * 学生社团管理系统 SCMS
 *
 * 包含：返回码、通用宏、系统级常量。
 * 所有 .c 文件都应最先包含本头文件。
 */
#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============ 返回码 ============ */
#define OK                0    /* 成功 */
#define ERR_DB           -1    /* 数据库错误 */
#define ERR_NOT_FOUND    -2    /* 记录不存在 */
#define ERR_DUPLICATE    -3    /* 记录重复（唯一约束冲突） */
#define ERR_PERMISSION   -4    /* 权限不足 */
#define ERR_VALIDATION   -5    /* 数据校验失败 */
#define ERR_FULL         -6    /* 名额已满 */
#define ERR_STATUS       -7    /* 状态不允许该操作 */
#define ERR_AUTH         -8    /* 认证失败（未登录/token无效） */
#define ERR_INPUT        -9    /* 输入参数错误 */
#define ERR_QUOTA_EXCEED -10   /* 超出额度 */
#define ERR_RESTRICTED   -11   /* 用户被限制 */
#define ERR_ELECTION     -12   /* 选举流程错误 */
#define ERR_VOTE_TIED    -13   /* 平票 */
#define ERR_SYSTEM       -99   /* 系统级错误 */

/* ============ 分页 ============ */
#define PAGE_SIZE_DEFAULT  15
#define PAGE_SIZE_MAX      100

/* ============ 报销额度默认值（学院未单独配置时的兜底，单位：元） ============ */
#define REIMB_SINGLE_LIMIT_DEFAULT   2000    /* 单次报销上限 */
#define REIMB_PERIOD_LIMIT_DEFAULT   5000    /* 每月累计报销上限 */

/* ============ 密码哈希 ============ */
#define SHA256_DIGEST_LENGTH 32
#define SHA256_HEX_LENGTH    65   /* 64 hex + '\0' */

/* ============ 会话 ============ */
/* SESSION_TOKEN_LEN 定义在 auth.h 中，如需在 common.h 引用请包含 auth.h */
/* ============ 字符串长度上限 ============ */
#define MAX_SQL_LEN       4096
#define MAX_NAME_LEN      101
#define MAX_TEXT_LEN      2001

/* ============ 角色字符串 ============ */
#define ROLE_SCHOOL   "school_admin"
#define ROLE_COLLEGE  "college_admin"
#define ROLE_CLUB     "club_admin"
#define ROLE_MEMBER   "club_member"
#define ROLE_STUDENT  "general_student"

/* ============ 通用宏 ============ */
#define SAFE_STR(s)   ((s) ? (s) : "")
#define ARRAY_LEN(a)  (sizeof(a) / sizeof((a)[0]))

#endif /* COMMON_H */
