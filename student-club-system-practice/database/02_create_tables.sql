-- =============================================================
-- 02_create_tables.sql — 建表脚本（21 张表）
-- 学生社团管理系统 SCMS
-- 执行前请确保已 USE scms_db;
-- =============================================================

USE scms_db;

-- 按依赖关系顺序建表，避免外键报错。
-- 先禁用外键检查，建完再启用（防止顺序问题）。
SET FOREIGN_KEY_CHECKS = 0;

-- -------------------------------------------------------------
-- 1. 学院表
-- -------------------------------------------------------------
CREATE TABLE colleges (
    college_id    INT AUTO_INCREMENT PRIMARY KEY,
    college_name  VARCHAR(100) NOT NULL UNIQUE,
    college_code  VARCHAR(20)  NOT NULL UNIQUE,
    description   VARCHAR(500),
    status        TINYINT NOT NULL DEFAULT 1,          -- 1=正常 0=停用
    created_at    DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at    DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- -------------------------------------------------------------
-- 2. 专业表
-- -------------------------------------------------------------
CREATE TABLE majors (
    major_id      INT AUTO_INCREMENT PRIMARY KEY,
    college_id    INT NOT NULL,
    major_name    VARCHAR(100) NOT NULL,
    major_code    VARCHAR(20),
    status        TINYINT NOT NULL DEFAULT 1,
    created_at    DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (college_id) REFERENCES colleges(college_id) ON DELETE CASCADE,
    UNIQUE KEY uk_college_major (college_id, major_name)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- -------------------------------------------------------------
-- 3. 班级表
-- -------------------------------------------------------------
CREATE TABLE classes (
    class_id      INT AUTO_INCREMENT PRIMARY KEY,
    major_id      INT NOT NULL,
    college_id    INT NOT NULL,
    class_name    VARCHAR(100) NOT NULL,
    grade         VARCHAR(10),
    status        TINYINT NOT NULL DEFAULT 1,
    created_at    DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (major_id) REFERENCES majors(major_id) ON DELETE CASCADE,
    FOREIGN KEY (college_id) REFERENCES colleges(college_id) ON DELETE CASCADE,
    UNIQUE KEY uk_major_class (major_id, class_name)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- -------------------------------------------------------------
-- 4. 用户表
-- -------------------------------------------------------------
CREATE TABLE users (
    user_id       INT AUTO_INCREMENT PRIMARY KEY,
    username      VARCHAR(50)  NOT NULL UNIQUE,
    password      VARCHAR(64)  NOT NULL,                -- SHA256 哈希（64位十六进制）
    real_name     VARCHAR(50)  NOT NULL,
    role          ENUM('school_admin','college_admin','club_admin','club_member','general_student')
                           NOT NULL DEFAULT 'general_student',
    college_id    INT          DEFAULT NULL,
    major_id      INT          DEFAULT NULL,
    class_id      INT          DEFAULT NULL,
    student_no    VARCHAR(10)  DEFAULT NULL,
    id_card       VARCHAR(18)  DEFAULT NULL,
    phone         VARCHAR(11)  DEFAULT NULL,
    email         VARCHAR(100) DEFAULT NULL,
    status        TINYINT NOT NULL DEFAULT 1,           -- 1=正常 0=禁用
    last_login    DATETIME DEFAULT NULL,
    created_at    DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at    DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (college_id) REFERENCES colleges(college_id) ON DELETE SET NULL,
    FOREIGN KEY (major_id)   REFERENCES majors(major_id) ON DELETE SET NULL,
    FOREIGN KEY (class_id)   REFERENCES classes(class_id) ON DELETE SET NULL,
    INDEX idx_role (role),
    INDEX idx_college (college_id),
    INDEX idx_student_no (student_no)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- -------------------------------------------------------------
-- 5. 社团表
-- -------------------------------------------------------------
CREATE TABLE clubs (
    club_id       INT AUTO_INCREMENT PRIMARY KEY,
    club_name     VARCHAR(100) NOT NULL UNIQUE,
    description   TEXT,
    category      VARCHAR(50),
    logo_path     VARCHAR(255),
    level         ENUM('school','college') NOT NULL DEFAULT 'college',
    college_id    INT DEFAULT NULL,
    president_id  INT DEFAULT NULL,
    vice_president_count INT DEFAULT 1,
    advisor       VARCHAR(50),
    member_count  INT DEFAULT 0,
    join_permission ENUM('all','college_only') NOT NULL DEFAULT 'all',
    status        ENUM('pending','approved','rejected','archived','dissolved')
                           NOT NULL DEFAULT 'pending',
    reject_reason VARCHAR(255),
    created_by    INT,
    created_at    DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at    DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (college_id)   REFERENCES colleges(college_id) ON DELETE SET NULL,
    FOREIGN KEY (president_id) REFERENCES users(user_id) ON DELETE SET NULL,
    FOREIGN KEY (created_by)   REFERENCES users(user_id) ON DELETE SET NULL,
    INDEX idx_level (level),
    INDEX idx_status (status),
    INDEX idx_category (category),
    INDEX idx_college (college_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- -------------------------------------------------------------
-- 6. 社团成员表
-- -------------------------------------------------------------
CREATE TABLE members (
    member_id     INT AUTO_INCREMENT PRIMARY KEY,
    club_id       INT NOT NULL,
    user_id       INT NOT NULL,
    role          ENUM('president','vice_president','secretary','treasurer','member')
                           NOT NULL DEFAULT 'member',
    join_status   ENUM('pending','approved','rejected') NOT NULL DEFAULT 'pending',
    joined_at     DATETIME DEFAULT CURRENT_TIMESTAMP,
    left_at       DATETIME DEFAULT NULL,
    FOREIGN KEY (club_id) REFERENCES clubs(club_id) ON DELETE CASCADE,
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE,
    UNIQUE KEY uk_club_user (club_id, user_id),
    INDEX idx_user (user_id),
    INDEX idx_club (club_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- -------------------------------------------------------------
-- 7. 活动表
-- -------------------------------------------------------------
CREATE TABLE activities (
    activity_id   INT AUTO_INCREMENT PRIMARY KEY,
    club_id       INT NOT NULL,
    title         VARCHAR(200) NOT NULL,
    description   TEXT,
    location      VARCHAR(200),
    start_time    DATETIME NOT NULL,
    end_time      DATETIME NOT NULL,
    signup_deadline DATETIME,
    max_participants INT DEFAULT 0,                     -- 0=不限
    current_count   INT DEFAULT 0,
    enable_signin   TINYINT NOT NULL DEFAULT 0,
    signin_code     VARCHAR(6) DEFAULT NULL,
    require_summary TINYINT NOT NULL DEFAULT 0,
    summary         TEXT DEFAULT NULL,
    status        ENUM('draft','published','ongoing','completed','cancelled','archived')
                           NOT NULL DEFAULT 'draft',
    cancel_reason VARCHAR(255),
    created_at    DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at    DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (club_id) REFERENCES clubs(club_id) ON DELETE CASCADE,
    INDEX idx_club (club_id),
    INDEX idx_status (status),
    INDEX idx_start (start_time)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- -------------------------------------------------------------
-- 8. 活动报名表
-- -------------------------------------------------------------
CREATE TABLE registrations (
    registration_id INT AUTO_INCREMENT PRIMARY KEY,
    activity_id     INT NOT NULL,
    user_id         INT NOT NULL,
    status          ENUM('registered','cancelled','attended') NOT NULL DEFAULT 'registered',
    registered_at   DATETIME DEFAULT CURRENT_TIMESTAMP,
    cancelled_at    DATETIME DEFAULT NULL,
    FOREIGN KEY (activity_id) REFERENCES activities(activity_id) ON DELETE CASCADE,
    FOREIGN KEY (user_id)     REFERENCES users(user_id) ON DELETE CASCADE,
    UNIQUE KEY uk_act_user (activity_id, user_id),
    INDEX idx_user (user_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- -------------------------------------------------------------
-- 9. 活动签到表
-- -------------------------------------------------------------
CREATE TABLE activity_signins (
    signin_id      INT AUTO_INCREMENT PRIMARY KEY,
    activity_id    INT NOT NULL,
    user_id        INT NOT NULL,
    signin_method  ENUM('code','manual') NOT NULL,
    signin_time    DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (activity_id) REFERENCES activities(activity_id) ON DELETE CASCADE,
    FOREIGN KEY (user_id)     REFERENCES users(user_id) ON DELETE CASCADE,
    UNIQUE KEY uk_signin (activity_id, user_id),
    INDEX idx_activity (activity_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- -------------------------------------------------------------
-- 10. 财务记录表
-- -------------------------------------------------------------
CREATE TABLE finance (
    finance_id    INT AUTO_INCREMENT PRIMARY KEY,
    club_id       INT NOT NULL,
    type          ENUM('income','expense') NOT NULL,
    amount        DECIMAL(12,2) NOT NULL,
    source        VARCHAR(50),
    description   VARCHAR(500),
    operator_id   INT,
    record_time   DATETIME DEFAULT CURRENT_TIMESTAMP,
    receipt_path  VARCHAR(255),
    FOREIGN KEY (club_id) REFERENCES clubs(club_id) ON DELETE CASCADE,
    FOREIGN KEY (operator_id) REFERENCES users(user_id) ON DELETE SET NULL,
    INDEX idx_club (club_id),
    INDEX idx_type (type),
    INDEX idx_time (record_time)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- -------------------------------------------------------------
-- 11. 报销申请表
-- -------------------------------------------------------------
CREATE TABLE reimbursement (
    reimbursement_id INT AUTO_INCREMENT PRIMARY KEY,
    club_id          INT NOT NULL,
    applicant_id     INT NOT NULL,
    amount           DECIMAL(12,2) NOT NULL,
    description      VARCHAR(500) NOT NULL,
    receipt_path     VARCHAR(255),
    status           ENUM('pending','approved','rejected') NOT NULL DEFAULT 'pending',
    reviewer_id      INT DEFAULT NULL,
    review_comment   VARCHAR(500),
    submitted_at     DATETIME DEFAULT CURRENT_TIMESTAMP,
    reviewed_at      DATETIME DEFAULT NULL,
    FOREIGN KEY (club_id)      REFERENCES clubs(club_id) ON DELETE CASCADE,
    FOREIGN KEY (applicant_id) REFERENCES users(user_id) ON DELETE CASCADE,
    FOREIGN KEY (reviewer_id)  REFERENCES users(user_id) ON DELETE SET NULL,
    INDEX idx_club (club_id),
    INDEX idx_status (status)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- -------------------------------------------------------------
-- 12. 报销额度配置表
-- -------------------------------------------------------------
CREATE TABLE reimbursement_limits (
    limit_id      INT AUTO_INCREMENT PRIMARY KEY,
    club_id       INT NOT NULL,
    single_limit  DECIMAL(12,2) NOT NULL DEFAULT 0.00,
    period_type   ENUM('monthly','semesterly') DEFAULT 'monthly',
    period_limit  DECIMAL(12,2) NOT NULL DEFAULT 0.00,
    current_period_used DECIMAL(12,2) NOT NULL DEFAULT 0.00,
    period_start  DATE,
    set_by        INT,
    updated_at    DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (club_id) REFERENCES clubs(club_id) ON DELETE CASCADE,
    FOREIGN KEY (set_by)  REFERENCES users(user_id) ON DELETE SET NULL,
    UNIQUE KEY uk_club (club_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- -------------------------------------------------------------
-- 13. 换届选举表
-- -------------------------------------------------------------
CREATE TABLE elections (
    election_id   INT AUTO_INCREMENT PRIMARY KEY,
    club_id       INT NOT NULL,
    initiated_by  INT NOT NULL,
    term_start    DATE NOT NULL,
    term_end      DATE NOT NULL,
    signup_start  DATETIME NOT NULL,
    signup_end    DATETIME NOT NULL,
    vote_start    DATETIME NOT NULL,
    vote_end      DATETIME NOT NULL,
    president_count INT NOT NULL DEFAULT 1,
    vice_president_count INT NOT NULL DEFAULT 1,
    status        ENUM('signup','voting','completed','cancelled') NOT NULL DEFAULT 'signup',
    result_summary TEXT,
    created_at    DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (club_id)      REFERENCES clubs(club_id) ON DELETE CASCADE,
    FOREIGN KEY (initiated_by) REFERENCES users(user_id) ON DELETE CASCADE,
    INDEX idx_club (club_id),
    INDEX idx_status (status)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- -------------------------------------------------------------
-- 14. 选举候选人表
-- -------------------------------------------------------------
CREATE TABLE election_candidates (
    candidate_id  INT AUTO_INCREMENT PRIMARY KEY,
    election_id   INT NOT NULL,
    user_id       INT NOT NULL,
    position      ENUM('president','vice_president') NOT NULL,
    signup_time   DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (election_id) REFERENCES elections(election_id) ON DELETE CASCADE,
    FOREIGN KEY (user_id)     REFERENCES users(user_id) ON DELETE CASCADE,
    UNIQUE KEY uk_election_user_pos (election_id, user_id, position),
    INDEX idx_election (election_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- -------------------------------------------------------------
-- 15. 投票记录表
-- -------------------------------------------------------------
CREATE TABLE election_votes (
    vote_id       INT AUTO_INCREMENT PRIMARY KEY,
    election_id   INT NOT NULL,
    candidate_id  INT NOT NULL,
    voter_id      INT NOT NULL,
    position      ENUM('president','vice_president') NOT NULL,
    voted_at      DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (election_id)  REFERENCES elections(election_id) ON DELETE CASCADE,
    FOREIGN KEY (candidate_id) REFERENCES election_candidates(candidate_id) ON DELETE CASCADE,
    FOREIGN KEY (voter_id)     REFERENCES users(user_id) ON DELETE CASCADE,
    UNIQUE KEY uk_election_voter_pos (election_id, voter_id, position),
    INDEX idx_election (election_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- -------------------------------------------------------------
-- 16. 消息通知表
-- -------------------------------------------------------------
CREATE TABLE notifications (
    notification_id INT AUTO_INCREMENT PRIMARY KEY,
    user_id         INT NOT NULL,
    title           VARCHAR(200) NOT NULL,
    content         TEXT NOT NULL,
    type            VARCHAR(50),
    related_id      INT DEFAULT NULL,
    is_read         TINYINT NOT NULL DEFAULT 0,
    created_at      DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE,
    INDEX idx_user_read (user_id, is_read),
    INDEX idx_time (created_at)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- -------------------------------------------------------------
-- 17. 公告表
-- -------------------------------------------------------------
CREATE TABLE announcements (
    announcement_id INT AUTO_INCREMENT PRIMARY KEY,
    title           VARCHAR(200) NOT NULL,
    content         TEXT NOT NULL,
    scope           ENUM('school','college','club') NOT NULL,
    college_id      INT DEFAULT NULL,
    club_id         INT DEFAULT NULL,
    publisher_id    INT NOT NULL,
    created_at      DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (college_id)   REFERENCES colleges(college_id) ON DELETE CASCADE,
    FOREIGN KEY (club_id)      REFERENCES clubs(club_id) ON DELETE CASCADE,
    FOREIGN KEY (publisher_id) REFERENCES users(user_id) ON DELETE CASCADE,
    INDEX idx_scope (scope),
    INDEX idx_time (created_at)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- -------------------------------------------------------------
-- 18. 操作日志表
-- -------------------------------------------------------------
CREATE TABLE logs (
    log_id        INT AUTO_INCREMENT PRIMARY KEY,
    user_id       INT,
    action        VARCHAR(100) NOT NULL,
    target_type   VARCHAR(50),
    target_id     INT,
    detail        TEXT,
    created_at    DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE SET NULL,
    INDEX idx_user (user_id),
    INDEX idx_action (action),
    INDEX idx_time (created_at)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- -------------------------------------------------------------
-- 19. 备份记录表
-- -------------------------------------------------------------
CREATE TABLE backup_records (
    backup_id     INT AUTO_INCREMENT PRIMARY KEY,
    backup_type   ENUM('auto','manual') NOT NULL,
    file_path     VARCHAR(500) NOT NULL,
    file_size     BIGINT,
    operator_id   INT,
    status        ENUM('success','failed') NOT NULL DEFAULT 'success',
    created_at    DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (operator_id) REFERENCES users(user_id) ON DELETE SET NULL,
    INDEX idx_time (created_at)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- -------------------------------------------------------------
-- 20. 系统配置表
-- -------------------------------------------------------------
CREATE TABLE system_config (
    config_id     INT AUTO_INCREMENT PRIMARY KEY,
    config_key    VARCHAR(100) NOT NULL UNIQUE,
    config_value  TEXT NOT NULL,
    description   VARCHAR(255),
    updated_at    DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- -------------------------------------------------------------
-- 21. 用户限制表
-- -------------------------------------------------------------
CREATE TABLE user_restrictions (
    restriction_id INT AUTO_INCREMENT PRIMARY KEY,
    user_id        INT NOT NULL,
    restriction_type ENUM('ban_join','ban_activity') NOT NULL,
    restricted_by  INT NOT NULL,
    start_time     DATETIME NOT NULL,
    end_time       DATETIME DEFAULT NULL,               -- NULL=永久
    reason         VARCHAR(500),
    is_active      TINYINT NOT NULL DEFAULT 1,
    created_at     DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id)       REFERENCES users(user_id) ON DELETE CASCADE,
    FOREIGN KEY (restricted_by) REFERENCES users(user_id) ON DELETE CASCADE,
    INDEX idx_user (user_id),
    INDEX idx_active (is_active)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

SET FOREIGN_KEY_CHECKS = 1;

-- 建表完成。共 21 张表。
