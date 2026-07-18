/**
 * register.c — 用户注册 + 公共下拉数据（学院/专业/班级）
 * 学生社团管理系统 SCMS（学生端 / 公共接口）
 *
 * 覆盖处理函数：
 *   pub_register        POST /api/register          用户注册
 *   pub_colleges_list   GET  /api/colleges          学院下拉
 *   pub_majors_list     GET  /api/majors?college_id 专业下拉（按学院联动）
 *   pub_classes_list    GET  /api/classes?major_id  班级下拉（按专业联动）
 *
 * ┌──── 启用方法 ────────────────────────────────────────────────┐
 * │  把下面第一行的  #if 0  改成  #if 1  即可编译进项目。          │
 * │  （文件用 #if 0 包裹表示"备用/未启用"，改 1 即启用）           │
 * └──────────────────────────────────────────────────────────────┘
 */
#if 1

#include "../../include/api.h"
#include "../../include/db.h"
#include "../../include/common.h"
#include "../../include/utils.h"
#include "../../include/routes.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ─────────────── 公共下拉：学院 / 专业 / 班级 ─────────────── */

/* GET /api/colleges — 返回所有正常状态学院，供注册页下拉 */
void pub_colleges_list(ApiContext *ctx) {
    MYSQL_RES *res = db_query(
        "SELECT college_id, college_name, college_code "
        "FROM colleges WHERE status=1 ORDER BY college_id");
    api_send_result(ctx, res);
}

/* GET /api/majors?college_id=X — 某学院下的专业 */
void pub_majors_list(ApiContext *ctx) {
    int college_id = api_get_int(ctx, "college_id", 0);
    if (college_id <= 0) { api_error(ctx, ERR_INPUT, "缺少 college_id"); return; }

    MYSQL_RES *res = db_query(
        "SELECT major_id, major_name, major_code "
        "FROM majors WHERE college_id=%d AND status=1 ORDER BY major_id",
        college_id);
    api_send_result(ctx, res);
}

/* GET /api/classes?major_id=X — 某专业下的班级 */
void pub_classes_list(ApiContext *ctx) {
    int major_id = api_get_int(ctx, "major_id", 0);
    if (major_id <= 0) { api_error(ctx, ERR_INPUT, "缺少 major_id"); return; }

    MYSQL_RES *res = db_query(
        "SELECT class_id, class_name, grade "
        "FROM classes WHERE major_id=%d AND status=1 ORDER BY class_id",
        major_id);
    api_send_result(ctx, res);
}

/* ─────────────────────────── 注册 ─────────────────────────── */

/* POST /api/register
 * body(JSON): username, password, real_name, college_id, major_id,
 *             class_id, student_no, id_card, phone
 * 校验后写入 users 表，角色固定 general_student。 */
void pub_register(ApiContext *ctx) {
    char username[64] = "", password[128] = "", real_name[64] = "";
    char student_no[16] = "", id_card[24] = "", phone[16] = "";
    int college_id, major_id, class_id;

    api_get_json_str(ctx, "username",   username,   sizeof(username));
    api_get_json_str(ctx, "password",   password,   sizeof(password));
    api_get_json_str(ctx, "real_name",  real_name,  sizeof(real_name));
    api_get_json_str(ctx, "student_no", student_no, sizeof(student_no));
    api_get_json_str(ctx, "id_card",    id_card,    sizeof(id_card));
    api_get_json_str(ctx, "phone",      phone,      sizeof(phone));
    college_id = api_get_json_int(ctx, "college_id", 0);
    major_id   = api_get_json_int(ctx, "major_id", 0);
    class_id   = api_get_json_int(ctx, "class_id", 0);

    /* ── 输入校验 ── */
    if (!utils_validate_username(username)) {
        api_error(ctx, ERR_VALIDATION, "用户名不合法（4-50位字母数字下划线）"); return;
    }
    if (utils_is_empty(password) || strlen(password) < 6) {
        api_error(ctx, ERR_VALIDATION, "密码至少6位"); return;
    }
    if (utils_is_empty(real_name)) {
        api_error(ctx, ERR_VALIDATION, "请填写真实姓名"); return;
    }
    if (!utils_validate_student_no(student_no)) {
        api_error(ctx, ERR_VALIDATION, "学号必须为10位数字"); return;
    }
    if (!utils_validate_id_card(id_card)) {
        api_error(ctx, ERR_VALIDATION, "身份证号格式不正确（18位）"); return;
    }
    if (!utils_validate_phone(phone)) {
        api_error(ctx, ERR_VALIDATION, "手机号格式不正确（11位）"); return;
    }
    if (college_id <= 0 || major_id <= 0 || class_id <= 0) {
        api_error(ctx, ERR_VALIDATION, "请选择学院、专业、班级"); return;
    }

    /* ── 唯一性检查：用户名 / 学号 ── */
    if (db_query_int("SELECT COUNT(*) FROM users WHERE username='%s'", username) > 0) {
        api_error(ctx, ERR_DUPLICATE, "用户名已存在"); return;
    }
    if (db_query_int("SELECT COUNT(*) FROM users WHERE student_no='%s'", student_no) > 0) {
        api_error(ctx, ERR_DUPLICATE, "该学号已被注册"); return;
    }

    /* ── 密码哈希 ── */
    char hash[SHA256_HEX_LENGTH];
    utils_sha256(password, hash);

    /* ── 转义用户输入后写库（real_name 可能含特殊字符） ── */
    char *e_name = db_escape(real_name);

    int rc = db_execute(
        "INSERT INTO users "
        "(username, password, real_name, role, college_id, major_id, class_id, "
        " student_no, id_card, phone, status) "
        "VALUES ('%s','%s','%s','general_student',%d,%d,%d,'%s','%s','%s',1)",
        username, hash, e_name, college_id, major_id, class_id,
        student_no, id_card, phone);

    free(e_name);

    if (rc < 0) { api_error(ctx, ERR_DB, "注册失败，请稍后重试"); return; }

    api_ok_msg(ctx, "注册成功，请登录");
}

#endif /* 备用文件：改 #if 0 为 #if 1 启用 */
