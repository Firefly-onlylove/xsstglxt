-- =============================================================
-- 03_triggers.sql — 触发器脚本（5 个）
-- 学生社团管理系统 SCMS
-- 自动维护冗余计数字段与报销周期额度，减少业务代码负担。
-- =============================================================

USE scms_db;

-- 若重复执行本脚本，先删除已存在的触发器，避免报错。
DROP TRIGGER IF EXISTS trg_member_after_insert;
DROP TRIGGER IF EXISTS trg_member_after_update;
DROP TRIGGER IF EXISTS trg_registration_after_insert;
DROP TRIGGER IF EXISTS trg_registration_after_update;
DROP TRIGGER IF EXISTS trg_reimbursement_after_update;

DELIMITER $$

-- -------------------------------------------------------------
-- 1. 成员加入（审批通过）时，社团人数 +1
-- -------------------------------------------------------------
CREATE TRIGGER trg_member_after_insert AFTER INSERT ON members
FOR EACH ROW
BEGIN
    IF NEW.join_status = 'approved' AND NEW.left_at IS NULL THEN
        UPDATE clubs SET member_count = member_count + 1 WHERE club_id = NEW.club_id;
    END IF;
END$$

-- -------------------------------------------------------------
-- 2. 成员状态变化时维护人数
--    - 由 pending/rejected 变为 approved 且在籍 → +1
--    - 设置了 left_at（退社/被移除） → -1
-- -------------------------------------------------------------
CREATE TRIGGER trg_member_after_update AFTER UPDATE ON members
FOR EACH ROW
BEGIN
    -- 审批通过（之前不是approved，现在是approved，且在籍）
    IF NEW.join_status = 'approved' AND OLD.join_status <> 'approved'
       AND NEW.left_at IS NULL THEN
        UPDATE clubs SET member_count = member_count + 1 WHERE club_id = NEW.club_id;
    END IF;
    -- 退出社团（原本在籍，现在 left_at 被赋值）
    IF NEW.left_at IS NOT NULL AND OLD.left_at IS NULL
       AND OLD.join_status = 'approved' THEN
        UPDATE clubs SET member_count = member_count - 1 WHERE club_id = NEW.club_id;
    END IF;
END$$

-- -------------------------------------------------------------
-- 3. 活动报名时，报名人数 +1
-- -------------------------------------------------------------
CREATE TRIGGER trg_registration_after_insert AFTER INSERT ON registrations
FOR EACH ROW
BEGIN
    IF NEW.status = 'registered' THEN
        UPDATE activities SET current_count = current_count + 1 WHERE activity_id = NEW.activity_id;
    END IF;
END$$

-- -------------------------------------------------------------
-- 4. 取消报名时，报名人数 -1
-- -------------------------------------------------------------
CREATE TRIGGER trg_registration_after_update AFTER UPDATE ON registrations
FOR EACH ROW
BEGIN
    IF NEW.status = 'cancelled' AND OLD.status = 'registered' THEN
        UPDATE activities SET current_count = current_count - 1 WHERE activity_id = NEW.activity_id;
    END IF;
END$$

-- -------------------------------------------------------------
-- 5. 报销审批通过时，累加当前周期已用额度
-- -------------------------------------------------------------
CREATE TRIGGER trg_reimbursement_after_update AFTER UPDATE ON reimbursement
FOR EACH ROW
BEGIN
    IF NEW.status = 'approved' AND OLD.status = 'pending' THEN
        UPDATE reimbursement_limits
        SET current_period_used = current_period_used + NEW.amount
        WHERE club_id = NEW.club_id;
    END IF;
END$$

DELIMITER ;

-- 触发器创建完成。共 5 个。
