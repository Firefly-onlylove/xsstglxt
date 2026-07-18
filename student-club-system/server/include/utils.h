/**
 * utils.h — 工具函数接口
 * 学生社团管理系统 SCMS
 *
 * 含 SHA256 密码哈希、输入校验、字符串/日期处理、随机码生成等。
 * 这些函数被各业务模块和公共模块复用。
 */
#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

/* ── 密码哈希（SHA256） ──
 * 将明文密码哈希为 64 位十六进制字符串。
 * out 缓冲区至少 SHA256_HEX_LENGTH(65) 字节。
 */
void utils_sha256(const char *input, char *out_hex);

/* 校验明文密码与已存哈希是否匹配，匹配返回 1 */
int  utils_password_verify(const char *plain, const char *hash);

/* ── 输入校验 ── */
int utils_validate_student_no(const char *s);   /* 10 位数字，合法返回 1 */
int utils_validate_id_card(const char *s);        /* 18 位身份证（末位可为X），合法返回 1 */
int utils_validate_phone(const char *s);           /* 11 位手机号，1 开头，合法返回 1 */
int utils_validate_username(const char *s);        /* 4-50 位，字母数字下划线，合法返回 1 */
int utils_is_empty(const char *s);                 /* NULL 或全空白返回 1 */

/* ── 字符串处理 ── */
void utils_trim(char *s);                          /* 原地去除首尾空白 */
void utils_strlcpy(char *dst, const char *src, size_t size); /* 安全拷贝，保证 '\0' */
int  utils_str_equal(const char *a, const char *b);/* 安全比较（含 NULL 判断），相等返回 1 */
/* ── 日期时间 ── */
void utils_now_datetime(char *out);                /* "YYYY-MM-DD HH:MM:SS"，out 至少 20 字节 */
void utils_now_date(char *out);                    /* "YYYY-MM-DD"，out 至少 11 字节 */
void utils_add_days(const char *date_in, int days, char *date_out); /* 日期加减天数 */
int  utils_datetime_compare(const char *a, const char *b); /* a<b 返回负，a>b 返回正，相等 0 */

/* ── 随机 ── */
void utils_gen_signin_code(char *out);             /* 生成 6 位数字签到码，out 至少 7 字节 */
void utils_gen_random_string(char *out, int len);  /* 生成指定长度随机字母数字串 */

/* ── 安全 ── */
const char *utils_safe(const char *s);             /* s 为 NULL 时返回 ""，用于 printf/JSON */

#endif /* UTILS_H */
