/**
 * utils.c — 工具函数实现
 * 学生社团管理系统 SCMS
 *
 * 包含：
 *   - SHA256 密码哈希（自行实现，无需 OpenSSL 依赖，参考 FIPS 180-4）
 *   - 输入校验（学号/身份证号/手机号/用户名/空值判断）
 *   - 字符串处理（去除空白/安全拷贝/相等比较/安全取值）
 *   - 日期时间处理（当前时间/日期计算/时间比较）
 *   - 随机码生成（签到码/随机字符串）
 */
#include "../../include/utils.h"
#include "../../include/common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdint.h>

/* ═══════════════════════════════════════════════════════════════
 * 一、SHA256 实现（参考 FIPS 180-4，公有领域算法）
 * ═══════════════════════════════════════════════════════════════ */

typedef struct {
    uint32_t state[8];   // 哈希状态（8 个 32 位字）
    uint64_t bitlen;     // 已处理的总比特数
    uint8_t  data[64];   // 当前 512 位数据块缓冲区
    uint32_t datalen;    // 数据缓冲区中已写入的字节数
} SHA256_CTX;

/* SHA256 核心运算宏 */
#define ROTR(x,n)  (((x) >> (n)) | ((x) << (32 - (n))))
#define CH(x,y,z)  (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x)     (ROTR(x,2) ^ ROTR(x,13) ^ ROTR(x,22))
#define EP1(x)     (ROTR(x,6) ^ ROTR(x,11) ^ ROTR(x,25))
#define SIG0(x)    (ROTR(x,7) ^ ROTR(x,18) ^ ((x) >> 3))
#define SIG1(x)    (ROTR(x,17) ^ ROTR(x,19) ^ ((x) >> 10))

/* SHA256 64 轮常量 K（前 64 个质数的小数部分） */
static const uint32_t sha256_k[64] = {
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

/**
 * sha256_transform — 对 512 位（64 字节）数据块执行 64 轮压缩变换
 * @ctx: SHA256 上下文
 * @data: 512 位数据块
 */
static void sha256_transform(SHA256_CTX *ctx, const uint8_t data[]) {
    uint32_t a,b,c,d,e,f,g,h,i,j,t1,t2,m[64];

    /* 前 16 个字直接取自输入数据块（大端序） */
    for (i = 0, j = 0; i < 16; ++i, j += 4)
        m[i] = ((uint32_t)data[j] << 24) | ((uint32_t)data[j+1] << 16) |
               ((uint32_t)data[j+2] << 8) | ((uint32_t)data[j+3]);
    /* 后 48 个字由递推公式生成 */
    for (; i < 64; ++i)
        m[i] = SIG1(m[i-2]) + m[i-7] + SIG0(m[i-15]) + m[i-16];

    /* 读取当前状态 */
    a = ctx->state[0]; b = ctx->state[1]; c = ctx->state[2]; d = ctx->state[3];
    e = ctx->state[4]; f = ctx->state[5]; g = ctx->state[6]; h = ctx->state[7];

    /* 64 轮主循环 */
    for (i = 0; i < 64; ++i) {
        t1 = h + EP1(e) + CH(e,f,g) + sha256_k[i] + m[i];
        t2 = EP0(a) + MAJ(a,b,c);
        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }

    /* 中间状态累加回原状态 */
    ctx->state[0] += a; ctx->state[1] += b; ctx->state[2] += c; ctx->state[3] += d;
    ctx->state[4] += e; ctx->state[5] += f; ctx->state[6] += g; ctx->state[7] += h;
}

/**
 * sha256_init — 初始化 SHA256 上下文
 * @ctx: SHA256 上下文指针
 *
 * 设置初始哈希值（前 8 个质数的平方根小数部分前 32 位）。
 */
static void sha256_init(SHA256_CTX *ctx) {
    ctx->datalen = 0;
    ctx->bitlen  = 0;
    ctx->state[0] = 0x6a09e667; ctx->state[1] = 0xbb67ae85;
    ctx->state[2] = 0x3c6ef372; ctx->state[3] = 0xa54ff53a;
    ctx->state[4] = 0x510e527f; ctx->state[5] = 0x9b05688c;
    ctx->state[6] = 0x1f83d9ab; ctx->state[7] = 0x5be0cd19;
}

/**
 * sha256_update — 追加数据到 SHA256 计算中
 * @ctx: SHA256 上下文指针
 * @data: 要追加的数据
 * @len: 数据长度（字节）
 */
static void sha256_update(SHA256_CTX *ctx, const uint8_t data[], size_t len) {
    for (size_t i = 0; i < len; ++i) {
        ctx->data[ctx->datalen++] = data[i];
        if (ctx->datalen == 64) {
            sha256_transform(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

/**
 * sha256_final — 完成 SHA256 计算，输出 32 字节哈希值
 * @ctx: SHA256 上下文指针
 * @hash: 输出的 32 字节哈希值缓冲区
 */
static void sha256_final(SHA256_CTX *ctx, uint8_t hash[]) {
    uint32_t i = ctx->datalen;

    /* 填充：先加 0x80，再补 0x00 直到长度对 64 取模余 56 */
    if (ctx->datalen < 56) {
        ctx->data[i++] = 0x80;
        while (i < 56) ctx->data[i++] = 0x00;
    } else {
        ctx->data[i++] = 0x80;
        while (i < 64) ctx->data[i++] = 0x00;
        sha256_transform(ctx, ctx->data);
        memset(ctx->data, 0, 56);
    }

    /* 末尾 8 字节写入原始消息总比特数（大端序） */
    ctx->bitlen += (uint64_t)ctx->datalen * 8;
    ctx->data[63] = (uint8_t)(ctx->bitlen);
    ctx->data[62] = (uint8_t)(ctx->bitlen >> 8);
    ctx->data[61] = (uint8_t)(ctx->bitlen >> 16);
    ctx->data[60] = (uint8_t)(ctx->bitlen >> 24);
    ctx->data[59] = (uint8_t)(ctx->bitlen >> 32);
    ctx->data[58] = (uint8_t)(ctx->bitlen >> 40);
    ctx->data[57] = (uint8_t)(ctx->bitlen >> 48);
    ctx->data[56] = (uint8_t)(ctx->bitlen >> 56);
    sha256_transform(ctx, ctx->data);

    /* 将 8 个 32 位状态字展开为 32 字节哈希（大端序） */
    for (i = 0; i < 4; ++i) {
        hash[i]      = (uint8_t)((ctx->state[0] >> (24 - i * 8)) & 0xff);
        hash[i + 4]  = (uint8_t)((ctx->state[1] >> (24 - i * 8)) & 0xff);
        hash[i + 8]  = (uint8_t)((ctx->state[2] >> (24 - i * 8)) & 0xff);
        hash[i + 12] = (uint8_t)((ctx->state[3] >> (24 - i * 8)) & 0xff);
        hash[i + 16] = (uint8_t)((ctx->state[4] >> (24 - i * 8)) & 0xff);
        hash[i + 20] = (uint8_t)((ctx->state[5] >> (24 - i * 8)) & 0xff);
        hash[i + 24] = (uint8_t)((ctx->state[6] >> (24 - i * 8)) & 0xff);
        hash[i + 28] = (uint8_t)((ctx->state[7] >> (24 - i * 8)) & 0xff);
    }
}

/**
 * utils_sha256 — 计算字符串的 SHA256 哈希值，输出为小写十六进制字符串
 * @input: 输入字符串
 * @out_hex: 输出的 65 字节十六进制字符串缓冲区（64 字符 + 结尾 '\0'）
 */
void utils_sha256(const char *input, char *out_hex) {
    SHA256_CTX ctx;
    uint8_t hash[32];
    static const char hexchars[] = "0123456789abcdef";

    sha256_init(&ctx);
    sha256_update(&ctx, (const uint8_t *)input, strlen(input));
    sha256_final(&ctx, hash);

    for (int i = 0; i < 32; ++i) {
        out_hex[i * 2]     = hexchars[(hash[i] >> 4) & 0xf];
        out_hex[i * 2 + 1] = hexchars[hash[i] & 0xf];
    }
    out_hex[64] = '\0';
}

/**
 * utils_password_verify — 验证明文密码是否与哈希值匹配
 * @plain: 明文密码
 * @hash: 存储的 SHA256 十六进制哈希值
 * 返回: 1 匹配成功, 0 不匹配
 */
int utils_password_verify(const char *plain, const char *hash) {
    char computed[SHA256_HEX_LENGTH];
    utils_sha256(plain, computed);
    return strcmp(computed, hash) == 0 ? 1 : 0;
}

/* ═══════════════════════════════════════════════════════════════
 * 二、输入校验
 * ═══════════════════════════════════════════════════════════════ */

/**
 * utils_validate_student_no — 校验学号格式
 * @s: 学号字符串
 * 返回: 1 合法（10 位纯数字）, 0 不合法
 */
int utils_validate_student_no(const char *s) {
    if (!s) return 0;
    if (strlen(s) != 10) return 0;
    for (int i = 0; i < 10; ++i)
        if (!isdigit((unsigned char)s[i])) return 0;
    return 1;
}

/**
 * utils_validate_id_card — 校验身份证号格式
 * @s: 身份证号字符串
 * 返回: 1 合法或为空（选填字段空值放行）, 0 不合法
 *
 * 18 位：前 17 位数字，末位可为数字或 X/x。
 */
int utils_validate_id_card(const char *s) {
    if (!s) return 1; /* 选填字段，空值放行 */
    if (strlen(s) == 0) return 1;
    if (strlen(s) != 18) return 0;
    for (int i = 0; i < 17; ++i)
        if (!isdigit((unsigned char)s[i])) return 0;
    char last = s[17];
    if (!isdigit((unsigned char)last) && last != 'X' && last != 'x') return 0;
    return 1;
}

/**
 * utils_validate_phone — 校验手机号格式
 * @s: 手机号字符串
 * 返回: 1 合法（11 位纯数字且首位为 1）, 0 不合法
 */
int utils_validate_phone(const char *s) {
    if (!s) return 0;
    if (strlen(s) != 11) return 0;
    if (s[0] != '1') return 0;
    for (int i = 0; i < 11; ++i)
        if (!isdigit((unsigned char)s[i])) return 0;
    return 1;
}

/**
 * utils_validate_username — 校验用户名格式
 * @s: 用户名字符串
 * 返回: 1 合法（4-50 位字母数字下划线）, 0 不合法
 */
int utils_validate_username(const char *s) {
    if (!s) return 0;
    size_t len = strlen(s);
    if (len < 4 || len > 50) return 0;
    for (size_t i = 0; i < len; ++i) {
        char c = s[i];
        if (!isalnum((unsigned char)c) && c != '_') return 0;
    }
    return 1;
}

/**
 * utils_is_empty — 判断字符串是否为空（NULL 或仅含空白字符）
 * @s: 待判断的字符串
 * 返回: 1 为空, 0 非空
 */
int utils_is_empty(const char *s) {
    if (!s) return 1;
    while (*s) {
        if (!isspace((unsigned char)*s)) return 0;
        s++;
    }
    return 1;
}

/* ═══════════════════════════════════════════════════════════════
 * 三、字符串处理
 * ═══════════════════════════════════════════════════════════════ */

/**
 * utils_trim — 去除字符串首尾空白字符（原地修改）
 * @s: 待处理的字符串
 */
void utils_trim(char *s) {
    if (!s) return;

    /* 跳过前导空白 */
    char *start = s;
    while (*start && isspace((unsigned char)*start)) start++;

    /* 找到末尾非空白字符 */
    char *end = start + strlen(start);
    while (end > start && isspace((unsigned char)*(end - 1))) end--;

    /* 将修剪后的内容移到开头 */
    size_t len = (size_t)(end - start);
    memmove(s, start, len);
    s[len] = '\0';
}

/**
 * utils_strlcpy — 安全字符串拷贝（始终以 '\0' 结尾）
 * @dst: 目标缓冲区
 * @src: 源字符串（可为 NULL）
 * @size: 目标缓冲区大小（字节）
 */
void utils_strlcpy(char *dst, const char *src, size_t size) {
    if (size == 0) return;
    if (!src) { dst[0] = '\0'; return; }
    size_t i = 0;
    for (; i < size - 1 && src[i]; ++i)
        dst[i] = src[i];
    dst[i] = '\0';
}

/**
 * utils_str_equal — 判断两个字符串是否相等
 * @a: 字符串 A（可为 NULL）
 * @b: 字符串 B（可为 NULL）
 * 返回: 1 相等, 0 不等
 *
 * 处理 NULL 指针：同为 NULL 视为相等，一为 NULL 一不为 NULL 视为不等。
 */
int utils_str_equal(const char *a, const char *b) {
    if (a == b) return 1;
    if (!a || !b) return 0;
    return strcmp(a, b) == 0 ? 1 : 0;
}

/* ═══════════════════════════════════════════════════════════════
 * 四、日期时间处理
 * ═══════════════════════════════════════════════════════════════ */

/**
 * utils_now_datetime — 获取当前日期时间字符串
 * @out: 输出缓冲区（至少 20 字节），格式 "YYYY-MM-DD HH:MM:SS"
 */
void utils_now_datetime(char *out) {
    time_t t = time(NULL);
    struct tm *lt = localtime(&t);
    strftime(out, 20, "%Y-%m-%d %H:%M:%S", lt);
}

/**
 * utils_now_date — 获取当前日期字符串
 * @out: 输出缓冲区（至少 11 字节），格式 "YYYY-MM-DD"
 */
void utils_now_date(char *out) {
    time_t t = time(NULL);
    struct tm *lt = localtime(&t);
    strftime(out, 11, "%Y-%m-%d", lt);
}

/**
 * utils_add_days — 在给定日期上加减天数
 * @date_in: 输入日期字符串（格式 "YYYY-MM-DD"）
 * @days: 要增加的天数（可为负数）
 * @date_out: 输出日期字符串缓冲区（至少 11 字节）
 *
 * 设置时间为正午 12:00 以避免夏令时边界问题。
 */
void utils_add_days(const char *date_in, int days, char *date_out) {
    struct tm tm_date;
    memset(&tm_date, 0, sizeof(tm_date));
    int y = 0, m = 0, d = 0;
    if (sscanf(date_in, "%d-%d-%d", &y, &m, &d) != 3) {
        utils_strlcpy(date_out, date_in, 11);
        return;
    }
    tm_date.tm_year = y - 1900;
    tm_date.tm_mon  = m - 1;
    tm_date.tm_mday = d + days;
    tm_date.tm_hour = 12;   /* 正午，避免夏令时边界问题 */
    time_t t = mktime(&tm_date);
    struct tm *lt = localtime(&t);
    strftime(date_out, 11, "%Y-%m-%d", lt);
}

/**
 * utils_datetime_compare — 比较两个日期时间字符串
 * @a: 日期时间字符串 A（格式 "YYYY-MM-DD HH:MM:SS"）
 * @b: 日期时间字符串 B（格式 "YYYY-MM-DD HH:MM:SS"）
 * 返回: 负数 a 早于 b, 0 相等, 正数 a 晚于 b
 *
 * 利用 "YYYY-MM-DD HH:MM:SS" 格式的字典序即时间序的特性，直接 strcmp。
 */
int utils_datetime_compare(const char *a, const char *b) {
    if (!a || !b) return 0;
    return strcmp(a, b);   /* "YYYY-MM-DD HH:MM:SS" 字典序即时间序 */
}

/* ═══════════════════════════════════════════════════════════════
 * 五、随机数生成
 * ═══════════════════════════════════════════════════════════════ */

static int g_rand_seeded = 0;

/**
 * ensure_seed — 确保随机数种子已设置
 *
 * 仅首次调用时设置种子，混合 time() 和 clock() 增加随机性。
 */
static void ensure_seed(void) {
    if (!g_rand_seeded) {
        srand((unsigned int)(time(NULL) ^ (unsigned int)clock()));
        g_rand_seeded = 1;
    }
}

/**
 * utils_gen_signin_code — 生成 6 位数字签到码
 * @out: 输出缓冲区（至少 7 字节，含结尾 '\0'）
 */
void utils_gen_signin_code(char *out) {
    ensure_seed();
    for (int i = 0; i < 6; ++i)
        out[i] = (char)('0' + rand() % 10);
    out[6] = '\0';
}

/**
 * utils_gen_random_string — 生成指定长度的随机字符串
 * @out: 输出缓冲区（至少 len + 1 字节）
 * @len: 随机字符串长度
 *
 * 字符集包含大小写字母和数字，共 62 个字符。
 */
void utils_gen_random_string(char *out, int len) {
    static const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    ensure_seed();
    for (int i = 0; i < len; ++i)
        out[i] = charset[rand() % (int)(sizeof(charset) - 1)];
    out[len] = '\0';
}

/**
 * utils_safe — 将 NULL 指针转换为空字符串
 * @s: 输入字符串（可为 NULL）
 * 返回: 输入字符串本身，若为 NULL 则返回 ""
 */
const char *utils_safe(const char *s) {
    return s ? s : "";
}
