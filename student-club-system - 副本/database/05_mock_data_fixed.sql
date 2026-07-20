-- ============================================================
-- 05_mock_data.sql — 完整模拟数据
-- 学生社团管理系统 SCMS
-- 用途：前端开发所需的全覆盖模拟数据
-- 依赖：先执行 01/02/03/04 脚本
-- 密码：所有学生密码均为 admin123
--        SHA256(admin123) = 240be518fabd2724ddb6f04eeb1da5967448d7e831c08c8fa822809f74c720a9
-- ============================================================

USE scms_db;
SET FOREIGN_KEY_CHECKS = 0;
SET @pwd = '240be518fabd2724ddb6f04eeb1da5967448d7e831c08c8fa822809f74c720a9';

-- ================================================================
-- 1. 普通学生账号（74 人，按学院分配，user_id 201-274）
--    覆盖所有班级，含 general_student 和 club_member 两种角色
-- ================================================================

-- 信息科学与技术学院 (college_id=1): 20 人
INSERT INTO users (user_id, username, password, real_name, role, college_id, major_id, class_id, student_no, phone, status) VALUES
(201, 'stu_cs01', @pwd, '刘明辉', 'general_student', 1, 1, 1, '2023010101', '13810000201', 1),
(202, 'stu_cs02', @pwd, '陈晓琳', 'general_student', 1, 1, 1, '2023010102', '13810000202', 1),
(203, 'stu_cs03', @pwd, '黄志远', 'general_student', 1, 1, 2, '2024010103', '13810000203', 1),
(204, 'stu_cs04', @pwd, '林雨婷', 'general_student', 1, 1, 2, '2024010104', '13810000204', 1),
(205, 'stu_cs05', @pwd, '张伟杰', 'general_student', 1, 2, 3, '2023020105', '13810000205', 1),
(206, 'stu_cs06', @pwd, '吴思琪', 'general_student', 1, 2, 3, '2023020106', '13810000206', 1),
(207, 'stu_cs07', @pwd, '郑浩然', 'general_student', 1, 2, 4, '2024020107', '13810000207', 1),
(208, 'stu_cs08', @pwd, '何美玲', 'general_student', 1, 3, 5, '2023030108', '13810000208', 1),
(209, 'stu_cs09', @pwd, '赵子轩', 'general_student', 1, 3, 5, '2023030109', '13810000209', 1),
(210, 'stu_cs10', @pwd, '孙晓萌', 'general_student', 1, 3, 6, '2024030110', '13810000210', 1),
(211, 'stu_cs11', @pwd, '周逸飞', 'general_student', 1, 4, 7, '2023040111', '13810000211', 1),
(212, 'stu_cs12', @pwd, '马思远', 'general_student', 1, 4, 7, '2023040112', '13810000212', 1),
(213, 'stu_cs13', @pwd, '朱文博', 'general_student', 1, 4, 8, '2024040113', '13810000213', 1),
(214, 'stu_cs14', @pwd, '胡雪晴', 'general_student', 1, 1, 1, '2023010114', '13810000214', 1),
(215, 'stu_cs15', @pwd, '沈嘉豪', 'general_student', 1, 2, 4, '2024020115', '13810000215', 1),
(216, 'stu_cs16', @pwd, '曹雅婷', 'general_student', 1, 3, 6, '2024030116', '13810000216', 1),
(217, 'stu_cs17', @pwd, '唐志鹏', 'general_student', 1, 4, 8, '2024040117', '13810000217', 1),
(218, 'stu_cs18', @pwd, '梁晓燕', 'general_student', 1, 1, 2, '2024010118', '13810000218', 1),
(219, 'stu_cs19', @pwd, '许俊杰', 'general_student', 1, 2, 3, '2023020119', '13810000219', 1),
(220, 'stu_cs20', @pwd, '高思源', 'general_student', 1, 3, 5, '2023030120', '13810000220', 1);

-- 文学院 (college_id=2): 12 人
INSERT INTO users (user_id, username, password, real_name, role, college_id, major_id, class_id, student_no, phone, status) VALUES
(221, 'stu_lit01', @pwd, '罗晓雯', 'general_student', 2, 5, 9, '2023050201', '13810000221', 1),
(222, 'stu_lit02', @pwd, '谢雨晴', 'general_student', 2, 5, 9, '2023050202', '13810000222', 1),
(223, 'stu_lit03', @pwd, '韩书瑶', 'general_student', 2, 5, 10, '2024050203', '13810000223', 1),
(224, 'stu_lit04', @pwd, '唐文博', 'general_student', 2, 6, 11, '2023060204', '13810000224', 1),
(225, 'stu_lit05', @pwd, '冯雅琪', 'general_student', 2, 6, 11, '2023060205', '13810000225', 1),
(226, 'stu_lit06', @pwd, '董思雨', 'general_student', 2, 7, 12, '2023070206', '13810000226', 1),
(227, 'stu_lit07', @pwd, '程佳慧', 'general_student', 2, 7, 12, '2023070207', '13810000227', 1),
(228, 'stu_lit08', @pwd, '蔡明哲', 'general_student', 2, 5, 10, '2024050208', '13810000228', 1),
(229, 'stu_lit09', @pwd, '潘雨萱', 'general_student', 2, 6, 11, '2023060209', '13810000229', 1),
(230, 'stu_lit10', @pwd, '彭学斌', 'general_student', 2, 7, 12, '2023070210', '13810000230', 1),
(231, 'stu_lit11', @pwd, '江美玲', 'general_student', 2, 5, 9, '2023050211', '13810000231', 1),
(232, 'stu_lit12', @pwd, '田志强', 'general_student', 2, 6, 11, '2023060212', '13810000232', 1);

-- 理学院 (college_id=3): 10 人
INSERT INTO users (user_id, username, password, real_name, role, college_id, major_id, class_id, student_no, phone, status) VALUES
(233, 'stu_sci01', @pwd, '任博文', 'general_student', 3, 8, 13, '2023080301', '13810000233', 1),
(234, 'stu_sci02', @pwd, '姜晓萌', 'general_student', 3, 8, 13, '2023080302', '13810000234', 1),
(235, 'stu_sci03', @pwd, '苏浩然', 'general_student', 3, 9, 14, '2023090303', '13810000235', 1),
(236, 'stu_sci04', @pwd, '卢雅雯', 'general_student', 3, 9, 14, '2023090304', '13810000236', 1),
(237, 'stu_sci05', @pwd, '蒋志伟', 'general_student', 3, 10, 15, '2023100305', '13810000237', 1),
(238, 'stu_sci06', @pwd, '沈佳怡', 'general_student', 3, 10, 15, '2023100306', '13810000238', 1),
(239, 'stu_sci07', @pwd, '邓子豪', 'general_student', 3, 8, 13, '2023080307', '13810000239', 1),
(240, 'stu_sci08', @pwd, '范思琪', 'general_student', 3, 9, 14, '2023090308', '13810000240', 1),
(241, 'stu_sci09', @pwd, '方文婧', 'general_student', 3, 10, 15, '2023100309', '13810000241', 1),
(242, 'stu_sci10', @pwd, '石明哲', 'general_student', 3, 8, 13, '2023080310', '13810000242', 1);

-- 工学院 (college_id=4): 10 人
INSERT INTO users (user_id, username, password, real_name, role, college_id, major_id, class_id, student_no, phone, status) VALUES
(243, 'stu_eng01', @pwd, '崔志强', 'general_student', 4, 11, 16, '2023110401', '13810000243', 1),
(244, 'stu_eng02', @pwd, '康晓燕', 'general_student', 4, 11, 16, '2023110402', '13810000244', 1),
(245, 'stu_eng03', @pwd, '牛浩然', 'general_student', 4, 12, 17, '2023120403', '13810000245', 1),
(246, 'stu_eng04', @pwd, '廖雨萌', 'general_student', 4, 12, 17, '2023120404', '13810000246', 1),
(247, 'stu_eng05', @pwd, '邱志远', 'general_student', 4, 13, 18, '2023130405', '13810000247', 1),
(248, 'stu_eng06', @pwd, '秦雪琳', 'general_student', 4, 13, 18, '2023130406', '13810000248', 1),
(249, 'stu_eng07', @pwd, '江一帆', 'general_student', 4, 11, 16, '2023110407', '13810000249', 1),
(250, 'stu_eng08', @pwd, '史嘉琪', 'general_student', 4, 12, 17, '2023120408', '13810000250', 1),
(251, 'stu_eng09', @pwd, '陶明达', 'general_student', 4, 13, 18, '2023130409', '13810000251', 1),
(252, 'stu_eng10', @pwd, '顾思源', 'general_student', 4, 11, 16, '2023110410', '13810000252', 1);

-- 经济与管理学院 (college_id=5): 12 人
INSERT INTO users (user_id, username, password, real_name, role, college_id, major_id, class_id, student_no, phone, status) VALUES
(253, 'stu_biz01', @pwd, '侯志明', 'general_student', 5, 14, 19, '2023140501', '13810000253', 1),
(254, 'stu_biz02', @pwd, '邵婷婷', 'general_student', 5, 14, 19, '2023140502', '13810000254', 1),
(255, 'stu_biz03', @pwd, '孟祥瑞', 'general_student', 5, 15, 20, '2023150503', '13810000255', 1),
(256, 'stu_biz04', @pwd, '白雅楠', 'general_student', 5, 15, 20, '2023150504', '13810000256', 1),
(257, 'stu_biz05', @pwd, '闫志豪', 'general_student', 5, 16, 21, '2023160505', '13810000257', 1),
(258, 'stu_biz06', @pwd, '段思琪', 'general_student', 5, 16, 21, '2023160506', '13810000258', 1),
(259, 'stu_biz07', @pwd, '雷雨辰', 'general_student', 5, 14, 19, '2023140507', '13810000259', 1),
(260, 'stu_biz08', @pwd, '龙晓雯', 'general_student', 5, 15, 20, '2023150508', '13810000260', 1),
(261, 'stu_biz09', @pwd, '万子豪', 'general_student', 5, 16, 21, '2023160509', '13810000261', 1),
(262, 'stu_biz10', @pwd, '段嘉豪', 'general_student', 5, 14, 19, '2023140510', '13810000262', 1),
(263, 'stu_biz11', @pwd, '龚雪莹', 'general_student', 5, 15, 20, '2023150511', '13810000263', 1),
(264, 'stu_biz12', @pwd, '易明哲', 'general_student', 5, 16, 21, '2023160512', '13810000264', 1);

-- 医学院 (college_id=6): 10 人
INSERT INTO users (user_id, username, password, real_name, role, college_id, major_id, class_id, student_no, phone, status) VALUES
(265, 'stu_med01', @pwd, '常思远', 'general_student', 6, 17, 22, '2023170601', '13810000265', 1),
(266, 'stu_med02', @pwd, '孔雨欣', 'general_student', 6, 17, 22, '2023170602', '13810000266', 1),
(267, 'stu_med03', @pwd, '宋志伟', 'general_student', 6, 18, 23, '2023180603', '13810000267', 1),
(268, 'stu_med04', @pwd, '梅雅琪', 'general_student', 6, 18, 23, '2023180604', '13810000268', 1),
(269, 'stu_med05', @pwd, '郭俊杰', 'general_student', 6, 19, 24, '2023190605', '13810000269', 1),
(270, 'stu_med06', @pwd, '谭晓燕', 'general_student', 6, 19, 24, '2023190606', '13810000270', 1),
(271, 'stu_med07', @pwd, '贺文博', 'general_student', 6, 17, 22, '2023170607', '13810000271', 1),
(272, 'stu_med08', @pwd, '赖佳琳', 'general_student', 6, 18, 23, '2023180608', '13810000272', 1),
(273, 'stu_med09', @pwd, '付昊天', 'general_student', 6, 19, 24, '2023190609', '13810000273', 1),
(274, 'stu_med10', @pwd, '陆思源', 'general_student', 6, 17, 22, '2023170610', '13810000274', 1);

-- 额外：3个禁用/受限状态学生（测试用户管理页状态筛选）
INSERT INTO users (user_id, username, password, real_name, role, college_id, major_id, class_id, student_no, phone, status) VALUES
(275, 'stu_lock01', @pwd, '熊凯文', 'general_student', 1, 1, 1, '2023010275', '13810000275', 0),
(276, 'stu_lock02', @pwd, '袁思雨', 'general_student', 3, 8, 13, '2023080276', '13810000276', 0),
(277, 'stu_test01', @pwd, '测试学生A', 'general_student', 2, 5, 9, '2023050277', '13810000277', 1);

-- ================================================================
-- 2. 社团成员（给每个社团填充 6-15 名成员，覆盖多种角色）
--    先将现有社长 member_count 重置，由触发器重新计数
-- ================================================================
UPDATE clubs SET member_count = 0;

-- ACM编程协会 (club_id=1): 社长(101 张三) + 副社长 + 12 名普通成员
INSERT INTO members (club_id, user_id, role, join_status) VALUES
(1, 202, 'vice_president', 'approved'),
(1, 203, 'member', 'approved'),
(1, 205, 'member', 'approved'),
(1, 208, 'member', 'approved'),
(1, 211, 'member', 'approved'),
(1, 214, 'member', 'approved'),
(1, 216, 'member', 'approved'),
(1, 219, 'member', 'approved'),
(1, 233, 'member', 'approved'),
(1, 235, 'member', 'approved'),
(1, 239, 'member', 'approved'),
(1, 243, 'member', 'approved'),
(1, 255, 'member', 'approved');

-- 志愿者协会 (club_id=2): 社长(102 李四) + 副社长 + 10 成员
INSERT INTO members (club_id, user_id, role, join_status) VALUES
(2, 206, 'vice_president', 'approved'),
(2, 209, 'member', 'approved'),
(2, 212, 'member', 'approved'),
(2, 221, 'member', 'approved'),
(2, 224, 'member', 'approved'),
(2, 228, 'member', 'approved'),
(2, 234, 'member', 'approved'),
(2, 244, 'member', 'approved'),
(2, 253, 'member', 'approved'),
(2, 265, 'member', 'approved'),
(2, 270, 'member', 'approved');

-- 星空天文社 (club_id=3): 社长(106 孙八) + 副社长 + 8 成员
INSERT INTO members (club_id, user_id, role, join_status) VALUES
(3, 207, 'vice_president', 'approved'),
(3, 210, 'member', 'approved'),
(3, 215, 'member', 'approved'),
(3, 225, 'member', 'approved'),
(3, 237, 'member', 'approved'),
(3, 248, 'member', 'approved'),
(3, 258, 'member', 'approved'),
(3, 266, 'member', 'approved'),
(3, 272, 'member', 'approved');

-- 绿茵足球社 (club_id=4): 社长(108 吴十) + 副社长 + 9 成员
INSERT INTO members (club_id, user_id, role, join_status) VALUES
(4, 204, 'vice_president', 'approved'),
(4, 213, 'member', 'approved'),
(4, 218, 'member', 'approved'),
(4, 229, 'member', 'approved'),
(4, 240, 'member', 'approved'),
(4, 246, 'member', 'approved'),
(4, 252, 'member', 'approved'),
(4, 257, 'member', 'approved'),
(4, 269, 'member', 'approved'),
(4, 201, 'member', 'approved');

-- 信科算法社 (club_id=5, 院级-信科): 社长(107 周九) + 副社长 + 6 成员（全是信科院）
INSERT INTO members (club_id, user_id, role, join_status) VALUES
(5, 205, 'vice_president', 'approved'),
(5, 208, 'member', 'approved'),
(5, 211, 'member', 'approved'),
(5, 214, 'member', 'approved'),
(5, 217, 'member', 'approved'),
(5, 220, 'member', 'approved');

-- 文学创作社 (club_id=6, 院级-文学): 社长(103 王五) + 副社长 + 6 成员（全是文学院）
INSERT INTO members (club_id, user_id, role, join_status) VALUES
(6, 222, 'vice_president', 'approved'),
(6, 223, 'member', 'approved'),
(6, 226, 'member', 'approved'),
(6, 227, 'member', 'approved'),
(6, 231, 'member', 'approved'),
(6, 232, 'member', 'approved');

-- 化学实验社 (club_id=7, 院级-理学): 社长(104 赵六) + 副社长 + 5 成员（全是理学院）
INSERT INTO members (club_id, user_id, role, join_status) VALUES
(7, 234, 'vice_president', 'approved'),
(7, 236, 'member', 'approved'),
(7, 238, 'member', 'approved'),
(7, 241, 'member', 'approved'),
(7, 242, 'member', 'approved');

-- 创业实践社 (club_id=8, 院级-经管): 社长(105 钱七) + 副社长 + 5 成员（全是经管院）
INSERT INTO members (club_id, user_id, role, join_status) VALUES
(8, 254, 'vice_president', 'approved'),
(8, 256, 'member', 'approved'),
(8, 259, 'member', 'approved'),
(8, 260, 'member', 'approved'),
(8, 263, 'member', 'approved');

-- ================================================================
-- 3. 待审批入社申请（用于 M2 入社审批页测试）
-- ================================================================
INSERT INTO members (club_id, user_id, role, join_status) VALUES
(1, 277, 'member', 'pending'),     -- 测试学生A 申请 ACM
(1, 230, 'member', 'pending'),     -- 彭学斌 申请 ACM
(2, 262, 'member', 'pending'),     -- 段嘉豪 申请 志愿者协会
(3, 245, 'member', 'pending'),     -- 牛浩然 申请 天文社
(4, 251, 'member', 'pending'),     -- 陶明达 申请 足球社
(5, 243, 'member', 'pending'),     -- 崔志强 申请 信科算法社
(6, 264, 'member', 'pending'),     -- 易明哲 申请 文学创作社
(8, 261, 'member', 'pending');     -- 万子豪 申请 创业实践社

-- ================================================================
-- 4. 待审批社团创建申请（用于 S2 社团审批页测试）
-- ================================================================
INSERT INTO clubs (club_id, club_name, description, category, level, college_id, president_id, vice_president_count, advisor, member_count, join_permission, status, created_by) VALUES
(9,  '无人机航拍社',   '探索无人机技术与航拍艺术，记录校园美丽瞬间。',          '实践', 'school', NULL, 215, 1, '杨老师', 0, 'all', 'pending', 215),
(10, '英语演讲辩论社', '培养英语口语表达与思辨能力，参加各类英语演讲比赛。',     '学术', 'school', NULL, 228, 1, '赵老师', 0, 'all', 'pending', 228),
(11, '数据科学研习社', '信息科学学院内部数据科学学习与实践社团。',              '学术', 'college', 1, 220, 1, '周老师', 0, 'college_only', 'pending', 220),
(12, '电影评论社',     '文学院电影爱好者社团，欣赏经典电影并撰写影评。',          '文艺', 'college', 2, 227, 1, '钱老师', 0, 'college_only', 'pending', 227);

-- 待审核社团的申请人成员记录
INSERT INTO members (club_id, user_id, role, join_status) VALUES
(9,  215, 'president', 'approved'),
(10, 228, 'president', 'approved'),
(11, 220, 'president', 'approved'),
(12, 227, 'president', 'approved');

-- ================================================================
-- 5. 活动数据（16 场活动，覆盖草稿/报名中/进行中/已结束/已取消）
-- ================================================================

-- ACM编程协会 活动 (club_id=1)
INSERT INTO activities (activity_id, club_id, title, description, type, start_time, end_time, signup_deadline, location, max_participants, status, signin_code, created_by) VALUES
(1, 1, 'ACM新生算法入门赛', '面向大一新生的算法竞赛入门训练赛，涵盖基础数据结构和经典算法题目，帮助新生快速适应ACM竞赛节奏。',
    '竞赛', '2026-07-20 14:00:00', '2026-07-20 18:00:00', '2026-07-19 23:59:59', '信科院实验楼A301', 50, 'published', 'A1B2C3', 101),
(2, 1, '动态规划专题训练', '深度讲解动态规划经典题型，从记忆化搜索到状态压缩DP，含实战训练环节。',
    '培训', '2026-04-10 09:00:00', '2026-04-10 12:00:00', '2026-04-09 18:00:00', '信科院实验楼A201', 40, 'completed', 'D4E5F6', 101),
(3, 1, '暑期算法集训营', '为期3天的暑期算法集训，包含高级数据结构与图论算法，备战省赛。',
    '培训', '2026-08-15 09:00:00', '2026-08-17 18:00:00', '2026-08-10 23:59:59', '线上+线下', 60, 'draft', NULL, 101);

-- 志愿者协会 活动 (club_id=2)
INSERT INTO activities (activity_id, club_id, title, description, type, start_time, end_time, signup_deadline, location, max_participants, status, signin_code, created_by) VALUES
(4, 2, '校园环保志愿活动', '组织志愿者在校园内进行垃圾分类宣传和环境清理活动，共建绿色校园。',
    '公益', '2026-07-25 08:00:00', '2026-07-25 12:00:00', '2026-07-24 20:00:00', '校园中心广场', 30, 'published', 'C7D8E9', 102),
(5, 2, '敬老院慰问活动', '前往市养老院进行慰问演出和陪伴活动，传递社会温暖与关爱。',
    '公益', '2026-05-15 08:00:00', '2026-05-15 16:00:00', '2026-05-14 18:00:00', '市养老院', 25, 'completed', 'F0A1B2', 102),
(6, 2, '暑期支教志愿者招募', '招募暑期赴山区支教的志愿者，面试通过后统一安排培训和出行。',
    '招募', '2026-08-01 00:00:00', '2026-08-15 00:00:00', '2026-07-28 23:59:59', '线上报名', 50, 'published', NULL, 102);

-- 星空天文社 活动 (club_id=3)
INSERT INTO activities (activity_id, club_id, title, description, type, start_time, end_time, signup_deadline, location, max_participants, status, signin_code, created_by) VALUES
(7, 3, '夏季星空观测之夜', '前往郊区天文观测点，使用天文望远镜观测夏季大三角和银河，学习星座知识。',
    '实践', '2026-07-28 20:00:00', '2026-07-29 02:00:00', '2026-07-27 18:00:00', '郊外天文观测基地', 20, 'published', 'G3H4I5', 106),
(8, 3, '天文知识科普讲座', '邀请物理系教授讲解黑洞、引力波等前沿天文知识，面向全校师生开放。',
    '讲座', '2026-06-20 14:00:00', '2026-06-20 17:00:00', '2026-06-19 18:00:00', '理学院报告厅', 80, 'completed', 'J6K7L8', 106);

-- 绿茵足球社 活动 (club_id=4)
INSERT INTO activities (activity_id, club_id, title, description, type, start_time, end_time, signup_deadline, location, max_participants, status, signin_code, created_by) VALUES
(9, 4, '院际足球联赛第一轮', '本学期院际足球联赛开幕战，信息科学学院 VS 工学院，欢迎观赛助威。',
    '比赛', '2026-07-22 14:00:00', '2026-07-22 17:00:00', '2026-07-22 12:00:00', '北校区足球场', 100, 'published', 'M9N0O1', 108),
(10, 4, '足球基础技术训练', '针对足球初学者的基础技术训练课程，含传球、射门、防守基本功教学。',
    '训练', '2026-07-18 16:00:00', '2026-07-18 18:00:00', '2026-07-18 14:00:00', '北校区足球场', 30, 'ongoing', 'P2Q3R4', 108);

-- 信科算法社 活动 (club_id=5)
INSERT INTO activities (activity_id, club_id, title, description, type, start_time, end_time, signup_deadline, location, max_participants, status, signin_code, created_by) VALUES
(11, 5, 'LeetCode周赛实战', '组织社团成员统一参加LeetCode周赛，赛后讲解排名和解题思路，提升算法能力。',
    '竞赛', '2026-07-19 10:00:00', '2026-07-19 12:00:00', '2026-07-19 09:30:00', '信科院实验室B102', 30, 'cancelled', NULL, 107),
(12, 5, '数据结构系列讲座——图论篇', '系统讲解图的存储、遍历、最短路径、最小生成树等核心算法。',
    '讲座', '2026-07-26 14:00:00', '2026-07-26 17:00:00', '2026-07-25 18:00:00', '信科院教学楼C201', 45, 'published', NULL, 107);

-- 文学创作社 活动 (club_id=6)
INSERT INTO activities (activity_id, club_id, title, description, type, start_time, end_time, signup_deadline, location, max_participants, status, signin_code, created_by) VALUES
(13, 6, '校园征文比赛', '以"青春与梦想"为主题的校园征文比赛，体裁不限，优秀作品将在校报刊登。',
    '比赛', '2026-07-15 00:00:00', '2026-08-15 00:00:00', '2026-08-10 23:59:59', '线上投稿', 200, 'published', NULL, 103),
(14, 6, '文学经典读书会——《百年孤独》', '共同阅读马尔克斯经典作品，探讨魔幻现实主义文学的魅力。',
    '读书会', '2026-06-05 18:30:00', '2026-06-05 20:30:00', '2026-06-04 18:00:00', '文学院阅览室', 20, 'completed', 'S5T6U7', 103);

-- 化学实验社 活动 (club_id=7)
INSERT INTO activities (activity_id, club_id, title, description, type, start_time, end_time, signup_deadline, location, max_participants, status, signin_code, created_by) VALUES
(15, 7, '趣味化学实验——自制口红与手工皂', '用化学知识亲手制作口红和手工皂，体验化学与生活的奇妙碰撞。',
    '实践', '2026-07-28 14:00:00', '2026-07-28 17:00:00', '2026-07-27 18:00:00', '理学院化学实验楼B101', 25, 'published', 'V8W9X0', 104);

-- 创业实践社 活动 (club_id=8)
INSERT INTO activities (activity_id, club_id, title, description, type, start_time, end_time, signup_deadline, location, max_participants, status, signin_code, created_by) VALUES
(16, 8, '校园创业计划大赛宣讲会', '介绍创业计划大赛的参赛流程和评审标准，邀请往届获奖团队分享经验。',
    '宣讲', '2026-05-20 15:00:00', '2026-05-20 17:00:00', '2026-05-20 12:00:00', '经管院教学楼A101', 60, 'completed', 'Y1Z2A3', 105);

-- ================================================================
-- 6. 活动报名记录（覆盖多种状态：已报名、已签到、已取消报名）
-- ================================================================

-- 活动1 (ACM新生算法入门赛, published): 8人报名
INSERT INTO registrations (activity_id, user_id, status, signin_time) VALUES
(1, 201, 'registered', NULL),
(1, 203, 'registered', NULL),
(1, 205, 'registered', NULL),
(1, 208, 'registered', NULL),
(1, 211, 'registered', NULL),
(1, 214, 'registered', NULL),
(1, 216, 'registered', NULL),
(1, 277, 'registered', NULL);

-- 活动2 (动态规划专题训练, finished): 10人报名，8人签到
INSERT INTO registrations (activity_id, user_id, status, signin_time) VALUES
(2, 202, 'attended', '2026-04-10 08:55:00'),
(2, 203, 'attended', '2026-04-10 08:58:00'),
(2, 205, 'attended', '2026-04-10 09:02:00'),
(2, 208, 'attended', '2026-04-10 09:00:00'),
(2, 211, 'cancelled', NULL),
(2, 214, 'cancelled', NULL),
(2, 233, 'attended', '2026-04-10 08:50:00'),
(2, 235, 'attended', '2026-04-10 09:05:00'),
(2, 239, 'attended', '2026-04-10 08:45:00'),
(2, 255, 'attended', '2026-04-10 08:52:00');

-- 活动4 (校园环保志愿活动, published): 5人报名
INSERT INTO registrations (activity_id, user_id, status, signin_time) VALUES
(4, 221, 'registered', NULL),
(4, 224, 'registered', NULL),
(4, 228, 'registered', NULL),
(4, 253, 'registered', NULL),
(4, 270, 'registered', NULL);

-- 活动5 (敬老院慰问, finished): 全部签到
INSERT INTO registrations (activity_id, user_id, status, signin_time) VALUES
(5, 206, 'attended', '2026-05-15 07:50:00'),
(5, 209, 'attended', '2026-05-15 07:55:00'),
(5, 212, 'attended', '2026-05-15 08:00:00'),
(5, 234, 'attended', '2026-05-15 07:48:00'),
(5, 244, 'attended', '2026-05-15 07:52:00'),
(5, 265, 'attended', '2026-05-15 07:55:00');

-- 活动6 (暑期支教招募, published): 3人报名
INSERT INTO registrations (activity_id, user_id, status, signin_time) VALUES
(6, 221, 'registered', NULL),
(6, 232, 'registered', NULL),
(6, 253, 'registered', NULL);

-- 活动7 (夏季星空观测, published): 6人报名
INSERT INTO registrations (activity_id, user_id, status, signin_time) VALUES
(7, 207, 'registered', NULL),
(7, 210, 'registered', NULL),
(7, 215, 'registered', NULL),
(7, 225, 'registered', NULL),
(7, 237, 'registered', NULL),
(7, 248, 'registered', NULL);

-- 活动8 (天文讲座, finished): 报名+签到
INSERT INTO registrations (activity_id, user_id, status, signin_time) VALUES
(8, 207, 'attended', '2026-06-20 13:50:00'),
(8, 210, 'attended', '2026-06-20 13:55:00'),
(8, 215, 'attended', '2026-06-20 14:00:00'),
(8, 237, 'attended', '2026-06-20 13:52:00'),
(8, 258, 'attended', '2026-06-20 13:58:00'),
(8, 266, 'cancelled', NULL);

-- 活动9 (足球联赛, published): 8人报名
INSERT INTO registrations (activity_id, user_id, status, signin_time) VALUES
(9, 204, 'registered', NULL),
(9, 213, 'registered', NULL),
(9, 218, 'registered', NULL),
(9, 229, 'registered', NULL),
(9, 246, 'registered', NULL),
(9, 252, 'registered', NULL),
(9, 257, 'registered', NULL),
(9, 269, 'registered', NULL);

-- 活动10 (足球训练, ongoing): 5人报名，3人已签到
INSERT INTO registrations (activity_id, user_id, status, signin_time) VALUES
(10, 201, 'attended', '2026-07-18 15:55:00'),
(10, 204, 'attended', '2026-07-18 15:58:00'),
(10, 229, 'attended', '2026-07-18 16:00:00'),
(10, 246, 'registered', NULL),
(10, 252, 'registered', NULL);

-- 活动12 (数据结构讲座, published): 4人报名
INSERT INTO registrations (activity_id, user_id, status, signin_time) VALUES
(12, 205, 'registered', NULL),
(12, 208, 'registered', NULL),
(12, 211, 'registered', NULL),
(12, 214, 'registered', NULL);

-- 活动14 (读书会, finished): 签到
INSERT INTO registrations (activity_id, user_id, status, signin_time) VALUES
(14, 222, 'attended', '2026-06-05 18:25:00'),
(14, 223, 'attended', '2026-06-05 18:28:00'),
(14, 226, 'attended', '2026-06-05 18:30:00'),
(14, 227, 'attended', '2026-06-05 18:26:00'),
(14, 231, 'attended', '2026-06-05 18:29:00');

-- 活动15 (趣味化学, published): 4人报名
INSERT INTO registrations (activity_id, user_id, status, signin_time) VALUES
(15, 234, 'registered', NULL),
(15, 236, 'registered', NULL),
(15, 238, 'registered', NULL),
(15, 241, 'registered', NULL);

-- 活动16 (创业计划宣讲, finished): 签到
INSERT INTO registrations (activity_id, user_id, status, signin_time) VALUES
(16, 254, 'attended', '2026-05-20 14:50:00'),
(16, 256, 'attended', '2026-05-20 14:55:00'),
(16, 259, 'attended', '2026-05-20 15:00:00'),
(16, 260, 'attended', '2026-05-20 14:52:00'),
(16, 263, 'attended', '2026-05-20 14:58:00');

-- ================================================================
-- 7. 财务收支记录（为每个社团添加收支数据）
-- ================================================================

-- ACM编程协会 收支
INSERT INTO finance (club_id, type, amount, description, operator_id) VALUES
(1, 'income', 3000.00, '学校社团经费拨款（2026春季学期）', 101),
(1, 'expense', 450.00, 'ACM新生赛奖品采购（图书+证书）', 101),
(1, 'expense', 180.00, '训练赛零食补给', 202),
(1, 'income', 500.00, '会员会费（2026级新生）', 101);

-- 志愿者协会 收支
INSERT INTO finance (club_id, type, amount, description, operator_id) VALUES
(2, 'income', 2000.00, '学校社团经费拨款', 102),
(2, 'expense', 320.00, '敬老院慰问物资采购（水果+牛奶）', 102),
(2, 'expense', 150.00, '环保活动工具采购', 206);

-- 星空天文社 收支
INSERT INTO finance (club_id, type, amount, description, operator_id) VALUES
(3, 'income', 1500.00, '学校社团经费拨款', 106),
(3, 'expense', 400.00, '观测之夜交通费（包车往返观测点）', 106),
(3, 'expense', 200.00, '望远镜配件购买', 207);

-- 绿茵足球社 收支
INSERT INTO finance (club_id, type, amount, description, operator_id) VALUES
(4, 'income', 2500.00, '学校社团经费拨款', 108),
(4, 'expense', 600.00, '足球采购（比赛用球x3）', 108),
(4, 'expense', 350.00, '队服印号费用', 204);

-- 信科算法社 收支
INSERT INTO finance (club_id, type, amount, description, operator_id) VALUES
(5, 'income', 1200.00, '学院社团经费拨款', 107),
(5, 'expense', 280.00, '算法书籍采购', 107);

-- 文学创作社 收支
INSERT INTO finance (club_id, type, amount, description, operator_id) VALUES
(6, 'income', 1000.00, '学院社团经费拨款', 103),
(6, 'expense', 150.00, '读书会茶歇费用', 103);

-- 化学实验社 收支
INSERT INTO finance (club_id, type, amount, description, operator_id) VALUES
(7, 'income', 1500.00, '学院社团经费拨款', 104),
(7, 'expense', 380.00, '实验耗材采购（试剂+模具）', 104);

-- 创业实践社 收支
INSERT INTO finance (club_id, type, amount, description, operator_id) VALUES
(8, 'income', 1200.00, '学院社团经费拨款', 105),
(8, 'expense', 220.00, '创业计划大赛宣传物料', 105);

-- ================================================================
-- 8. 报销申请（覆盖学院审批和学校审批两级流程）
-- ================================================================

-- 已通过（两级都通过）
INSERT INTO reimbursements (club_id, applicant_id, amount, purpose, receipt_url, college_status, college_reviewer, college_opinion, college_time, school_status, school_reviewer, school_opinion, school_time) VALUES
(5, 107, 280.00, '算法书籍采购报销', '/receipts/demo_receipt_01.jpg', 'approved', 2, '审核通过，金额合理', '2026-04-05 10:00:00', 'approved', 1, '同意报销', '2026-04-06 14:00:00'),
(6, 103, 150.00, '读书会茶歇购买报销', '/receipts/demo_receipt_02.jpg', 'approved', 3, '同意', '2026-06-06 09:00:00', 'approved', 1, '同意报销', '2026-06-07 11:00:00');

-- 学院已通过、学校待审批（用于学校财务监督页测试）
INSERT INTO reimbursements (club_id, applicant_id, amount, purpose, receipt_url, college_status, college_reviewer, college_opinion, college_time, school_status) VALUES
(1, 101, 450.00, 'ACM新生赛奖品采购', '/receipts/demo_receipt_03.jpg', 'approved', 2, '金额合规，同意', '2026-07-15 10:00:00', 'pending'),
(4, 108, 600.00, '训练用足球采购', '/receipts/demo_receipt_04.jpg', 'approved', 4, '请在发票背面注明用途', '2026-07-16 14:00:00', 'pending');

-- 待学院审批（用于学院报销审批页测试）
INSERT INTO reimbursements (club_id, applicant_id, amount, purpose, receipt_url, college_status) VALUES
(7, 104, 380.00, '趣味化学实验耗材采购', '/receipts/demo_receipt_05.jpg', 'pending'),
(8, 105, 220.00, '创业大赛宣传物料', '/receipts/demo_receipt_06.jpg', 'pending'),
(3, 106, 200.00, '望远镜配件采购', '/receipts/demo_receipt_07.jpg', 'pending');

-- 已驳回
INSERT INTO reimbursements (club_id, applicant_id, amount, purpose, receipt_url, college_status, college_reviewer, college_opinion, college_time) VALUES
(2, 206, 500.00, '志愿者马甲定制', '/receipts/demo_receipt_08.jpg', 'rejected', 3, '超出额度，请分批申请或缩减数量', '2026-05-10 09:30:00');

-- ================================================================
-- 9. 换届选举数据
-- ================================================================

-- ACM编程协会：已结束的一届 + 正在报名中的一届
INSERT INTO elections (election_id, club_id, title, election_type, start_time, end_time, status, winner_id, total_votes) VALUES
(1, 1, 'ACM编程协会2026年换届选举', 'president_vice', '2026-03-01 00:00:00', '2026-03-10 23:59:59', 'completed', 101, 15),
(2, 1, 'ACM编程协会副社长补选', 'vice_only', '2026-07-15 00:00:00', '2026-07-30 23:59:59', 'signup', NULL, 0);

-- 绿茵足球社：正在投票中
INSERT INTO elections (election_id, club_id, title, election_type, start_time, end_time, status, total_votes) VALUES
(3, 4, '绿茵足球社2026年换届选举', 'president_only', '2026-07-10 00:00:00', '2026-07-25 23:59:59', 'voting', 8);

-- 文学创作社：已结束
INSERT INTO elections (election_id, club_id, title, election_type, start_time, end_time, status, winner_id, total_votes) VALUES
(4, 6, '文学创作社2026年换届选举', 'president_vice', '2026-03-05 00:00:00', '2026-03-15 23:59:59', 'completed', 103, 8);

-- 选举1的候选人和投票（已结束，张三连任）
INSERT INTO election_candidates (election_id, user_id, position, manifesto, vote_count) VALUES
(1, 101, 'president', '过去一年带领社团成功举办多场ACM训练赛，获得省级二等奖。竞选连任，继续为社团发展贡献力量。', 10),
(1, 205, 'president', '对算法竞赛充满热情，希望带领社团冲击国赛。', 5),
(1, 202, 'vice_president', '擅长活动组织，可以为社团引入更多资源。', 12),
(1, 208, 'vice_president', 'ACM老队员，有丰富的训练赛组题经验，希望担任副社长辅助社长工作。', 3);

-- 选举2的候选人（报名中，3人参选副社长）
INSERT INTO election_candidates (election_id, user_id, position, manifesto, vote_count) VALUES
(2, 203, 'vice_president', '认真负责，有丰富的编程竞赛经验，可以协助社长组织训练赛。', 0),
(2, 211, 'vice_president', '乐于奉献，希望为社团发展出一份力。', 0),
(2, 214, 'vice_president', '曾获省级算法竞赛奖项，可以指导新社员入门。', 0);

-- 选举3的候选人和投票（投票中，足球社）
INSERT INTO election_candidates (election_id, user_id, position, manifesto, vote_count) VALUES
(3, 108, 'president', '现任社长，申请连任。过去一年组织多次友谊赛和训练活动，希望继续为社团服务。', 5),
(3, 204, 'president', '有丰富的足球裁判经验，可以引入更专业的训练体系。', 3);

-- 选举1的投票记录（已结束）
INSERT INTO election_votes (election_id, voter_id, candidate_id) VALUES
(1, 201, (SELECT candidate_id FROM election_candidates WHERE election_id=1 AND user_id=101 LIMIT 1)),
(1, 203, (SELECT candidate_id FROM election_candidates WHERE election_id=1 AND user_id=101 LIMIT 1)),
(1, 208, (SELECT candidate_id FROM election_candidates WHERE election_id=1 AND user_id=101 LIMIT 1)),
(1, 211, (SELECT candidate_id FROM election_candidates WHERE election_id=1 AND user_id=205 LIMIT 1)),
(1, 214, (SELECT candidate_id FROM election_candidates WHERE election_id=1 AND user_id=205 LIMIT 1)),
(1, 216, (SELECT candidate_id FROM election_candidates WHERE election_id=1 AND user_id=101 LIMIT 1)),
(1, 219, (SELECT candidate_id FROM election_candidates WHERE election_id=1 AND user_id=101 LIMIT 1)),
(1, 233, (SELECT candidate_id FROM election_candidates WHERE election_id=1 AND user_id=205 LIMIT 1)),
(1, 235, (SELECT candidate_id FROM election_candidates WHERE election_id=1 AND user_id=205 LIMIT 1)),
(1, 239, (SELECT candidate_id FROM election_candidates WHERE election_id=1 AND user_id=101 LIMIT 1)),
(1, 243, (SELECT candidate_id FROM election_candidates WHERE election_id=1 AND user_id=101 LIMIT 1)),
(1, 255, (SELECT candidate_id FROM election_candidates WHERE election_id=1 AND user_id=101 LIMIT 1)),
(1, 202, (SELECT candidate_id FROM election_candidates WHERE election_id=1 AND user_id=202 LIMIT 1)),
(1, 205, (SELECT candidate_id FROM election_candidates WHERE election_id=1 AND user_id=202 LIMIT 1)),
(1, 208, (SELECT candidate_id FROM election_candidates WHERE election_id=1 AND user_id=208 LIMIT 1));

-- 选举3的投票记录（投票中，已有8票）
INSERT INTO election_votes (election_id, voter_id, candidate_id) VALUES
(3, 201, (SELECT candidate_id FROM election_candidates WHERE election_id=3 AND user_id=108 LIMIT 1)),
(3, 204, (SELECT candidate_id FROM election_candidates WHERE election_id=3 AND user_id=204 LIMIT 1)),
(3, 213, (SELECT candidate_id FROM election_candidates WHERE election_id=3 AND user_id=108 LIMIT 1)),
(3, 218, (SELECT candidate_id FROM election_candidates WHERE election_id=3 AND user_id=108 LIMIT 1)),
(3, 229, (SELECT candidate_id FROM election_candidates WHERE election_id=3 AND user_id=108 LIMIT 1)),
(3, 246, (SELECT candidate_id FROM election_candidates WHERE election_id=3 AND user_id=204 LIMIT 1)),
(3, 252, (SELECT candidate_id FROM election_candidates WHERE election_id=3 AND user_id=204 LIMIT 1)),
(3, 269, (SELECT candidate_id FROM election_candidates WHERE election_id=3 AND user_id=108 LIMIT 1));

-- ================================================================
-- 10. 通知消息（覆盖系统/社团/活动三种类型）
-- ================================================================

-- 系统通知
INSERT INTO notifications (user_id, title, content, type, is_read) VALUES
(101, '系统维护通知', '系统将于7月20日凌晨2:00进行例行维护，预计持续1小时，届时将暂停访问。', 'system', 0),
(102, '系统维护通知', '系统将于7月20日凌晨2:00进行例行维护，预计持续1小时，届时将暂停访问。', 'system', 0),
(201, '欢迎加入SCMS', '欢迎使用学生社团管理系统！在这里你可以浏览社团、报名活动、管理个人信息。', 'system', 1);

-- 社团通知（ACM编程协会全体成员）
INSERT INTO notifications (user_id, title, content, type, related_id, is_read) VALUES
(101, 'ACM新生赛即将开始', 'ACM新生算法入门赛将于7月20日下午2点在信科院实验楼A301举行，请各位社员提前到场准备。', 'club', 1, 0),
(202, 'ACM新生赛即将开始', 'ACM新生算法入门赛将于7月20日下午2点在信科院实验楼A301举行，请各位社员提前到场准备。', 'club', 1, 0),
(203, 'ACM新生赛即将开始', 'ACM新生算法入门赛将于7月20日下午2点在信科院实验楼A301举行，请各位社员提前到场准备。', 'club', 1, 0),
(205, 'ACM新生赛即将开始', 'ACM新生算法入门赛将于7月20日下午2点在信科院实验楼A301举行，请各位社员提前到场准备。', 'club', 1, 0),
(208, 'ACM新生赛即将开始', 'ACM新生算法入门赛将于7月20日下午2点在信科院实验楼A301举行，请各位社员提前到场准备。', 'club', 1, 0),
(211, 'ACM新生赛即将开始', 'ACM新生算法入门赛将于7月20日下午2点在信科院实验楼A301举行，请各位社员提前到场准备。', 'club', 1, 0),
(214, 'ACM新生赛即将开始', 'ACM新生算法入门赛将于7月20日下午2点在信科院实验楼A301举行，请各位社员提前到场准备。', 'club', 1, 0),
(216, 'ACM新生赛即将开始', 'ACM新生算法入门赛将于7月20日下午2点在信科院实验楼A301举行，请各位社员提前到场准备。', 'club', 1, 0),
(219, 'ACM新生赛即将开始', 'ACM新生算法入门赛将于7月20日下午2点在信科院实验楼A301举行，请各位社员提前到场准备。', 'club', 1, 0),
(233, 'ACM新生赛即将开始', 'ACM新生算法入门赛将于7月20日下午2点在信科院实验楼A301举行，请各位社员提前到场准备。', 'club', 1, 0),
(235, 'ACM新生赛即将开始', 'ACM新生算法入门赛将于7月20日下午2点在信科院实验楼A301举行，请各位社员提前到场准备。', 'club', 1, 0);

-- 社团通知（志愿者协会）
INSERT INTO notifications (user_id, title, content, type, related_id, is_read) VALUES
(102, '环保志愿活动招募', '校园环保志愿活动将于7月25日上午8点举行，需要30名志愿者，欢迎大家积极报名！', 'club', 2, 0),
(206, '环保志愿活动招募', '校园环保志愿活动将于7月25日上午8点举行，需要30名志愿者，欢迎大家积极报名！', 'club', 2, 0),
(209, '环保志愿活动招募', '校园环保志愿活动将于7月25日上午8点举行，需要30名志愿者，欢迎大家积极报名！', 'club', 2, 0);

-- 社团通知（足球社）
INSERT INTO notifications (user_id, title, content, type, related_id, is_read) VALUES
(108, '院际足球联赛开赛通知', '本学期院际足球联赛将于7月22日开幕，请全体队员做好准备。', 'club', 4, 0),
(204, '院际足球联赛开赛通知', '本学期院际足球联赛将于7月22日开幕，请全体队员做好准备。', 'club', 4, 0);

-- 活动通知（已报名ACM新生赛的学生，活动报名成功通知）
INSERT INTO notifications (user_id, title, content, type, related_id, is_read) VALUES
(201, '报名成功：ACM新生算法入门赛', '你已成功报名"ACM新生算法入门赛"，请于7月20日14:00准时到信科院实验楼A301参加。', 'activity', 1, 1),
(203, '报名成功：ACM新生算法入门赛', '你已成功报名"ACM新生算法入门赛"，请于7月20日14:00准时到信科院实验楼A301参加。', 'activity', 1, 1),
(205, '报名成功：ACM新生算法入门赛', '你已成功报名"ACM新生算法入门赛"，请于7月20日14:00准时到信科院实验楼A301参加。', 'activity', 1, 0),
(208, '报名成功：ACM新生算法入门赛', '你已成功报名"ACM新生算法入门赛"，请于7月20日14:00准时到信科院实验楼A301参加。', 'activity', 1, 0);

-- 活动签到成功通知
INSERT INTO notifications (user_id, title, content, type, related_id, is_read) VALUES
(201, '签到成功：足球基础技术训练', '你已在"足球基础技术训练"活动中签到成功。', 'activity', 10, 1);

-- 入社申请结果通知
INSERT INTO notifications (user_id, title, content, type, is_read) VALUES
(277, '入社申请已提交', '你申请加入"ACM编程协会"的请求已提交，请等待社长审核。', 'system', 0);

-- 系统公告类通知（给所有用户群发的简化版）
INSERT INTO notifications (user_id, title, content, type, is_read) VALUES
(277, '注册成功', '恭喜你成功注册SCMS系统账号，请完善个人资料。', 'system', 1);

-- ================================================================
-- 11. 更多公告（学校+学院+社团三级）
-- ================================================================

-- 学校公告
INSERT INTO announcements (title, content, scope, publisher_id) VALUES
('关于2026年秋季社团招新工作安排的通知', '各社团负责人请注意：2026年秋季社团招新工作将于9月1日正式启动，请各社团在8月20日前提交招新计划和展位申请。社团管理中心将统一协调安排招新时间和场地。', 'school', 1),
('2026年度优秀社团评选通知', '为了表彰在2025-2026学年中表现突出的学生社团，学校决定开展年度优秀社团评选活动。评选标准包括社团活动质量、成员发展、财务管理等多个维度。请各社团在8月30日前提交申报材料。', 'school', 1);

-- 学院公告
INSERT INTO announcements (title, content, scope, college_id, publisher_id) VALUES
('信科院社团活动场地使用新规定', '为进一步规范学院实验室使用，即日起信科院所属社团借用实验室场地需至少提前3天申请，并在活动后当天完成场地清理。违者将暂停场地使用权限。', 'college', 1, 2),
('文学院社团经费调整通知', '经学院研究决定，自2026年秋季学期起，文学院社团经费标准调整为：普通社团每学期1000元，重点社团每学期2000元。请各社团根据新标准调整预算。', 'college', 2, 3);

-- 社团公告
INSERT INTO announcements (title, content, scope, club_id, publisher_id) VALUES
('ACM编程协会新社员招募中', 'ACM编程协会欢迎所有对算法和编程感兴趣的同学加入！零基础也可，我们提供系统化的算法培训课程。报名方式：在社团广场搜索"ACM编程协会"并提交入社申请。', 'club', 1, 101),
('绿茵足球社暑期训练计划', '暑期足球训练将于7月至8月每周二、四下午4:00-6:00在北校区足球场进行，请各位社员安排好时间，积极参加。', 'club', 4, 108);

-- ================================================================
-- 12. 操作日志
-- ================================================================
INSERT INTO logs (user_id, action, target_type, target_id, detail) VALUES
(101, 'login', 'system', NULL, '用户登录系统'),
(1, 'approve', 'clubs', 1, '通过ACM编程协会创建申请'),
(1, 'approve', 'clubs', 2, '通过志愿者协会创建申请'),
(2, 'approve', 'reimbursements', 1, '审批通过信科算法社书籍采购报销'),
(3, 'approve', 'reimbursements', 2, '审批通过文学创作社茶歇报销'),
(3, 'reject', 'reimbursements', 8, '驳回志愿者协会志愿者马甲定制报销'),
(101, 'create', 'activities', 1, '创建活动"ACM新生算法入门赛"'),
(108, 'create', 'activities', 9, '创建活动"院际足球联赛第一轮"'),
(106, 'create', 'elections', 2, '发起"ACM编程协会副社长补选"选举'),
(1, 'post', 'announcements', NULL, '发布学校公告"2026年秋季社团招新工作安排"');

-- ================================================================
-- 13. 用户限制记录（测试限制管理功能）
-- ================================================================
INSERT INTO user_restrictions (user_id, restrict_type, reason, start_date, end_date, status, set_by) VALUES
(275, 'join_club', '因上学期违规使用社团经费，禁止加入新社团', '2026-07-01', '2026-12-31', 'active', 1),
(275, 'participate_activity', '处罚期间同时限制参加活动', '2026-07-01', '2026-12-31', 'active', 1),
(276, 'apply_club', '恶意重复提交创建社团申请，限制创建社团权限30天', '2026-07-10', '2026-08-09', 'active', 2);

-- ================================================================
-- 14. 数据统计更新（确保计数器与实际数据一致）
-- ================================================================
-- 修正活动报名人数
UPDATE activities a SET current_count = (SELECT COUNT(*) FROM registrations r WHERE r.activity_id = a.activity_id AND r.status != 'cancelled');

-- 修正社团成员数
UPDATE clubs c SET member_count = (SELECT COUNT(*) FROM members m WHERE m.club_id = c.club_id AND m.join_status = 'approved');

SET FOREIGN_KEY_CHECKS = 1;

-- 完成提示
SELECT '============================================' AS '';
SELECT '  模拟数据导入完成！' AS message;
SELECT '============================================' AS '';
SELECT CONCAT('用户总数：', COUNT(*)) AS total_users FROM users;
SELECT CONCAT('社团总数：', COUNT(*)) AS total_clubs FROM clubs;
SELECT CONCAT('社团成员总数：', COUNT(*)) AS total_members FROM members WHERE join_status='approved';
SELECT CONCAT('待审批入社：', COUNT(*)) AS pending_joins FROM members WHERE join_status='pending';
SELECT CONCAT('待审批社团：', COUNT(*)) AS pending_clubs FROM clubs WHERE status='pending';
SELECT CONCAT('活动总数：', COUNT(*)) AS total_activities FROM activities;
SELECT CONCAT('活动报名总数：', COUNT(*)) AS total_registrations FROM registrations;
SELECT CONCAT('财务记录总数：', COUNT(*)) AS total_finance FROM finance;
SELECT CONCAT('报销申请总数：', COUNT(*)) AS total_reimbursements FROM reimbursements;
SELECT CONCAT('选举总数：', COUNT(*)) AS total_elections FROM elections;
SELECT CONCAT('通知总数：', COUNT(*)) AS total_notifications FROM notifications;
SELECT CONCAT('公告总数：', COUNT(*)) AS total_announcements FROM announcements;
SELECT CONCAT('日志总数：', COUNT(*)) AS total_logs FROM logs;
SELECT CONCAT('限制记录总数：', COUNT(*)) AS total_restrictions FROM user_restrictions;
SELECT '============================================' AS '';
