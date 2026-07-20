-- =============================================================
-- 01_create_database.sql — 建库脚本
-- 学生社团管理系统 SCMS
-- 字符集 utf8mb4 / 引擎 InnoDB
-- =============================================================

-- 删除已存在的数据库（重装时使用，谨慎）
DROP DATABASE IF EXISTS scms_db;

-- 创建数据库
CREATE DATABASE scms_db
    DEFAULT CHARACTER SET utf8mb4
    DEFAULT COLLATE utf8mb4_general_ci;

-- 创建专用账号（可选，若已有账号可跳过）
-- 注意：MySQL 8.0 默认使用 caching_sha2_password，
-- 若 C 客户端连接报 "Authentication plugin ... cannot be loaded"，
-- 改用 mysql_native_password（见下方注释）。

-- 方式一（MySQL 8.0 推荐，若 C API 支持 caching_sha2）：
CREATE USER IF NOT EXISTS 'scms_user'@'%' IDENTIFIED BY 'scms_pass_2026';

-- 方式二（若 C API 连接失败，改用传统认证插件，取消下一行注释并注释掉上一行）：
-- CREATE USER IF NOT EXISTS 'scms_user'@'%' IDENTIFIED WITH mysql_native_password BY 'scms_pass_2026';

-- 授权
GRANT ALL PRIVILEGES ON scms_db.* TO 'scms_user'@'%';
FLUSH PRIVILEGES;

-- 切换到目标库
USE scms_db;
