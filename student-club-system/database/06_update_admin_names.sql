-- ============================================================
-- 06_update_admin_names.sql -- 更新管理员姓名为真实中文名称
-- 学生社团管理系统 SCMS
-- 用途：将学院管理员的 real_name 从通用描述改为个人化中文名称
-- ============================================================

USE scms_db;

UPDATE users SET real_name = '张明远老师' WHERE username = 'cs_admin' AND role = 'college_admin';
UPDATE users SET real_name = '李文学老师' WHERE username = 'lit_admin' AND role = 'college_admin';
UPDATE users SET real_name = '王理正老师' WHERE username = 'sci_admin' AND role = 'college_admin';
UPDATE users SET real_name = '陈工明老师' WHERE username = 'eng_admin' AND role = 'college_admin';
UPDATE users SET real_name = '赵经纶老师' WHERE username = 'biz_admin' AND role = 'college_admin';
UPDATE users SET real_name = '刘济世老师' WHERE username = 'med_admin' AND role = 'college_admin';
UPDATE users SET real_name = '系统管理员'           WHERE username = 'admin'     AND role = 'school_admin';

SELECT '管理员姓名已更新' AS message;
SELECT user_id, username, real_name, role FROM users WHERE role IN ('school_admin','college_admin');
