/**
 * utils.h — 工具函数接口
 * 学生社团管理系统 SCMS
 *
 * 包含 SHA256 密码哈希、输入校验、字符串处理、日期时间处理、
 * 随机码生成等功能。这些函数被各业务模块和公共模块复用。
 */
#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

/* ── 密码哈希（SHA256） ── */

/** 将明文密码哈希为 64 位十六进制字符串
 *  @param input   输入的明文字符串
 *  @param out_hex 输出缓冲区（至少 SHA256_HEX_LENGTH 即 65 字节） */
void utils_sha256(const char *input, char *out_hex);

/** 校验明文密码与已存储的哈希值是否匹配
 *  @param plain 用户输入的明文密码
 *  @param hash  数据库中存储的 SHA256 十六进制哈希
 *  @return 匹配返回 1，不匹配返回 0 */
int  utils_password_verify(const char *plain, const char *hash);

/* ── 输入校验 ── */

/** 校验学号格式（10 位数字）
 *  @return 合法返回 1，非法返回 0 */
int utils_validate_student_no(const char *s);

/** 校验身份证号格式（18 位，末位可为 X）
 *  @return 合法返回 1，非法返回 0 */
int utils_validate_id_card(const char *s);

/** 校验手机号格式（11 位，1 开头）
 *  @return 合法返回 1，非法返回 0 */
int utils_validate_phone(const char *s);

/** 校验用户名格式（4-50 位，仅字母数字下划线）
 *  @return 合法返回 1，非法返回 0 */
int utils_validate_username(const char *s);

/** 判断字符串是否为空（NULL 或全空白字符）
 *  @return 为空返回 1，非空返回 0 */
int utils_is_empty(const char *s);

/* ── 字符串处理 ── */

/** 原地去除字符串首尾空白字符
 *  @param s 待处理的字符串（原地修改） */
void utils_trim(char *s);

/** 安全字符串拷贝（保证目标缓冲区以 '\0' 结尾）
 *  @param dst  目标缓冲区
 *  @param src  源字符串
 *  @param size 目标缓冲区大小 */
void utils_strlcpy(char *dst, const char *src, size_t size);

/** 安全比较两个字符串（含 NULL 判断）
 *  @return 相等返回 1，不等或任一为 NULL 返回 0 */
int  utils_str_equal(const char *a, const char *b);

/* ── 日期时间 ── */

/** 获取当前日期时间字符串
 *  格式：YYYY-MM-DD HH:MM:SS
 *  @param out 输出缓冲区（至少 20 字节） */
void utils_now_datetime(char *out);

/** 获取当前日期字符串
 *  格式：YYYY-MM-DD
 *  @param out 输出缓冲区（至少 11 字节） */
void utils_now_date(char *out);

/** 日期加减指定天数
 *  @param date_in  输入日期（YYYY-MM-DD 格式）
 *  @param days     要加减的天数（正数为加，负数为减）
 *  @param date_out 输出缓冲区，写入计算后的日期（YYYY-MM-DD 格式） */
void utils_add_days(const char *date_in, int days, char *date_out);

/** 比较两个日期时间字符串的大小
 *  @param a 第一个日期时间（YYYY-MM-DD HH:MM:SS 格式）
 *  @param b 第二个日期时间（YYYY-MM-DD HH:MM:SS 格式）
 *  @return a<b 返回负数，a>b 返回正数，相等返回 0 */
int  utils_datetime_compare(const char *a, const char *b);

/* ── 随机生成 ── */

/** 生成 6 位数字签到码
 *  @param out 输出缓冲区（至少 7 字节，含结束符） */
void utils_gen_signin_code(char *out);

/** 生成指定长度的随机字母数字串
 *  @param out 输出缓冲区
 *  @param len 要生成的字符串长度（不含结束符） */
void utils_gen_random_string(char *out, int len);

/* ── 安全输出 ── */

/** 安全获取字符串指针（NULL 时返回空串 "")
 *  用于 printf / JSON 拼接等场景，避免打印 (null)。
 *  @param s 字符串指针，可为 NULL
 *  @return s 不为 NULL 时返回 s，否则返回 "" */
const char *utils_safe(const char *s);

#endif /* UTILS_H */
