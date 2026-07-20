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
#define OK                0    // 操作成功
#define ERR_DB           -1    // 数据库错误
#define ERR_NOT_FOUND    -2    // 记录不存在
#define ERR_DUPLICATE    -3    // 记录重复（唯一约束冲突）
#define ERR_PERMISSION   -4    // 权限不足
#define ERR_VALIDATION   -5    // 数据校验失败
#define ERR_FULL         -6    // 名额已满
#define ERR_STATUS       -7    // 状态不允许该操作
#define ERR_AUTH         -8    // 认证失败（未登录或令牌无效）
#define ERR_INPUT        -9    // 输入参数错误
#define ERR_QUOTA_EXCEED -10   // 超出额度
#define ERR_RESTRICTED   -11   // 用户被限制
#define ERR_ELECTION     -12   // 选举流程错误
#define ERR_VOTE_TIED    -13   // 平票
#define ERR_SYSTEM       -99   // 系统级错误

/* ============ 分页 ============ */
#define PAGE_SIZE_DEFAULT  15    // 默认每页条数
#define PAGE_SIZE_MAX      100   // 每页最大条数

/* ============ 报销额度默认值（学院未单独配置时的兜底值，单位：元） ============ */
#define REIMB_SINGLE_LIMIT_DEFAULT   2000    // 单次报销上限
#define REIMB_PERIOD_LIMIT_DEFAULT   5000    // 每月累计报销上限

/* ============ 密码哈希 ============ */
#define SHA256_DIGEST_LENGTH 32    // SHA256 摘要长度（字节数）
#define SHA256_HEX_LENGTH    65    // 十六进制字符串长度（64字符+结束符）

/* ============ 字符串长度上限 ============ */
#define MAX_SQL_LEN       4096      // SQL 语句最大长度
#define MAX_NAME_LEN      101       // 名称字段最大长度
#define MAX_TEXT_LEN      2001      // 文本字段最大长度

/* ============ 角色字符串 ============ */
#define ROLE_SCHOOL   "school_admin"       // 学校管理员
#define ROLE_COLLEGE  "college_admin"      // 学院管理员
#define ROLE_CLUB     "club_admin"         // 社团社长/管理员
#define ROLE_MEMBER   "club_member"        // 社团成员
#define ROLE_STUDENT  "general_student"    // 普通学生

/* ============ 通用宏 ============ */
#define SAFE_STR(s)   ((s) ? (s) : "")           // 安全取字符串，NULL 时返回空串
#define ARRAY_LEN(a)  (sizeof(a) / sizeof((a)[0])) // 计算数组元素个数

#endif /* COMMON_H */
