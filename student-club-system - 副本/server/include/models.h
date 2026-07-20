/**
 * models.h — 数据模型结构体定义
 * 学生社团管理系统 SCMS
 *
 * 每个结构体对应一张数据库表；标注了"JOIN 展示字段"的成员
 * 不属于本表字段，是联表查询时附加的展示用字段。
 */
#ifndef MODELS_H
#define MODELS_H

/* ============ 基础数据模型 ============ */

/** 学院 */
typedef struct {
    int  college_id;          // 学院ID（主键，自增）
    char college_name[101];   // 学院名称
    char college_code[21];    // 学院编码
    char description[501];    // 学院简介
    int  status;              // 状态：1 正常 0 停用
    char created_at[20];      // 创建时间
    char updated_at[20];      // 更新时间
} College;

/** 专业 */
typedef struct {
    int  major_id;            // 专业ID（主键，自增）
    int  college_id;          // 所属学院ID
    char major_name[101];     // 专业名称
    char major_code[21];      // 专业编码
    int  status;              // 状态：1 正常 0 停用
    char created_at[20];      // 创建时间
} Major;

/** 班级 */
typedef struct {
    int  class_id;            // 班级ID（主键，自增）
    int  major_id;            // 所属专业ID
    int  college_id;          // 所属学院ID
    char class_name[101];     // 班级名称
    char grade[11];           // 年级（如 "2024"）
    int  status;              // 状态：1 正常 0 停用
    char created_at[20];      // 创建时间
} Class;

/** 用户 */
typedef struct {
    int  user_id;             // 用户ID（主键，自增）
    char username[51];        // 用户名
    char password[65];        // 密码（SHA256 十六进制哈希，64字符+结束符）
    char real_name[51];       // 真实姓名
    char role[20];            // 角色：school_admin(学校管理员)/college_admin(学院管理员)/club_admin(社团社长)/club_member(社团成员)/general_student(普通学生)
    int  college_id;          // 所属学院ID
    int  major_id;            // 所属专业ID
    int  class_id;            // 所属班级ID
    char student_no[11];      // 学号（10 位数字）
    char id_card[19];         // 身份证号（18 位，末位可为 X）
    char phone[12];           // 手机号（11 位数字）
    char email[101];          // 电子邮箱
    int  status;              // 状态：1 正常 0 禁用
    char last_login[20];      // 最后登录时间
    char created_at[20];      // 注册时间
    char updated_at[20];      // 更新时间
    /* ── JOIN 展示字段 ── */
    char college_name[101];   // 学院名称（JOIN 查询填充）
    char major_name[101];     // 专业名称（JOIN 查询填充）
    char class_name[101];     // 班级名称（JOIN 查询填充）
} User;

/** 社团 */
typedef struct {
    int  club_id;             // 社团ID（主键，自增）
    char club_name[101];      // 社团名称
    char description[501];    // 社团简介
    char category[51];        // 社团类别
    char logo_path[256];      // 社团标志图片路径
    char level[10];           // 社团级别：school(校级)/college(院级)
    int  college_id;          // 所属学院ID
    int  president_id;        // 社长用户ID
    int  vice_president_count;// 副社长人数
    char advisor[51];         // 指导老师
    int  member_count;        // 当前成员数（触发器自动维护）
    char join_permission[15]; // 加入权限：all(全校可加入)/college_only(仅本院)
    char status[20];          // 社团状态：pending(待审批)/approved(已通过)/rejected(已驳回)/archived(已归档)/dissolved(已解散)
    char reject_reason[256];  // 驳回原因
    int  created_by;          // 创建者用户ID
    char created_at[20];      // 创建时间
    char updated_at[20];      // 更新时间
    /* ── JOIN 展示字段 ── */
    char college_name[101];   // 学院名称（JOIN 查询填充）
    char president_name[51];  // 社长姓名（JOIN 查询填充）
} Club;

/** 社团成员 */
typedef struct {
    int  member_id;           // 成员记录ID（主键，自增）
    int  club_id;             // 所属社团ID
    int  user_id;             // 用户ID
    char role[20];            // 社团内角色：president(社长)/vice_president(副社长)/secretary(秘书)/treasurer(财务)/member(普通成员)
    char join_status[12];     // 加入状态：pending(待审批)/approved(已通过)/rejected(已拒绝)
    char joined_at[20];       // 加入时间
    char left_at[20];         // 退出时间（空串表示仍在籍）
    /* ── JOIN 展示字段 ── */
    char user_name[51];       // 用户姓名（JOIN 查询填充）
    char club_name[101];      // 社团名称（JOIN 查询填充）
    char student_no[11];      // 学号（JOIN 查询填充）
} Member;

/** 社团活动 */
typedef struct {
    int  activity_id;         // 活动ID（主键，自增）
    int  club_id;             // 所属社团ID
    char title[201];          // 活动标题
    char description[1001];   // 活动描述
    char location[201];       // 活动地点
    char start_time[20];      // 开始时间
    char end_time[20];        // 结束时间
    char signup_deadline[20]; // 报名截止时间
    int  max_participants;    // 最大报名人数（0 表示不限）
    int  current_count;       // 当前报名人数（触发器自动维护）
    int  enable_signin;       // 是否启用签到：1 启用 0 不启用
    char signin_code[7];      // 签到码（6 位数字）
    int  require_summary;     // 是否要求提交活动总结：1 需要 0 不需要
    char summary[2001];       // 活动总结内容
    char status[20];          // 活动状态：draft(草稿)/published(已发布)/ongoing(进行中)/finished(已结束)/cancelled(已取消)
    char cancel_reason[256];  // 取消原因
    char created_at[20];      // 创建时间
    char updated_at[20];      // 更新时间
    /* ── JOIN 展示字段 ── */
    char club_name[101];      // 社团名称（JOIN 查询填充）
} Activity;

/** 活动报名记录 */
typedef struct {
    int  registration_id;     // 报名记录ID（主键，自增）
    int  activity_id;         // 活动ID
    int  user_id;             // 报名用户ID
    char status[15];          // 报名状态：registered(已报名)/cancelled(已取消)/attended(已出席)
    char registered_at[20];   // 报名时间
    char cancelled_at[20];    // 取消报名时间
    /* ── JOIN 展示字段 ── */
    char activity_title[201]; // 活动标题（JOIN 查询填充）
    char user_name[51];       // 用户姓名（JOIN 查询填充）
} Registration;

/** 活动签到记录 */
typedef struct {
    int  signin_id;           // 签到记录ID（主键，自增）
    int  activity_id;         // 活动ID
    int  user_id;             // 签到用户ID
    char signin_method[10];   // 签到方式：code(签到码签到)/manual(手动签到)
    char signin_time[20];     // 签到时间
    /* ── JOIN 展示字段 ── */
    char user_name[51];       // 用户姓名（JOIN 查询填充）
} ActivitySignin;

/** 社团财务记录 */
typedef struct {
    int    finance_id;        // 记录ID（主键，自增）
    int    club_id;           // 所属社团ID
    char   type[10];          // 收支类型：income(收入)/expense(支出)
    double amount;            // 金额
    char   source[51];        // 来源/用途（如：拨款、赞助、活动收入）
    char   description[501];  // 详细说明
    int    operator_id;       // 经办人用户ID
    char   record_time[20];   // 记录时间
    char   receipt_path[256]; // 凭证文件路径
    /* ── JOIN 展示字段 ── */
    char   operator_name[51]; // 经办人姓名（JOIN 查询填充）
    char   club_name[101];    // 社团名称（JOIN 查询填充）
} FinanceRecord;

/** 报销申请 */
typedef struct {
    int    reimbursement_id;  // 报销ID（主键，自增）
    int    club_id;           // 所属社团ID
    int    applicant_id;      // 申请人用户ID
    double amount;            // 报销金额
    char   description[501];  // 报销说明
    char   receipt_path[256]; // 发票图片路径
    char   status[12];        // 报销状态：pending(待审批)/approved(已通过)/rejected(已驳回)
    int    reviewer_id;       // 审批人用户ID
    char   review_comment[501];// 审批意见
    char   submitted_at[20];  // 提交时间
    char   reviewed_at[20];   // 审批时间
    /* ── JOIN 展示字段 ── */
    char   club_name[101];    // 社团名称（JOIN 查询填充）
    char   applicant_name[51];// 申请人姓名（JOIN 查询填充）
    char   reviewer_name[51]; // 审批人姓名（JOIN 查询填充）
} Reimbursement;

/** 报销额度 */
typedef struct {
    int    limit_id;          // 额度记录ID（主键，自增）
    int    club_id;           // 社团ID
    double single_limit;      // 单次报销上限（元）
    char   period_type[15];   // 周期类型：monthly(每月)/semesterly(每学期)
    double period_limit;      // 周期累计报销上限（元）
    double current_period_used;// 当前周期已使用金额（触发器自动维护）
    char   period_start[11];  // 当前周期起始日期（DATE 格式）
    int    set_by;            // 设置人用户ID
    char   updated_at[20];    // 更新时间
} ReimbursementLimit;

/** 换届选举 */
typedef struct {
    int  election_id;         // 选举ID（主键，自增）
    int  club_id;             // 所属社团ID
    int  initiated_by;        // 发起人用户ID
    char term_start[11];      // 任期起始日期（DATE 格式）
    char term_end[11];        // 任期结束日期（DATE 格式）
    char signup_start[20];    // 报名开始时间
    char signup_end[20];      // 报名截止时间
    char vote_start[20];      // 投票开始时间
    char vote_end[20];        // 投票截止时间
    int  president_count;     // 应选社长人数
    int  vice_president_count;// 应选副社长人数
    char status[15];          // 选举状态：signup(报名阶段)/voting(投票阶段)/completed(已完成)/cancelled(已取消)
    char result_summary[2001];// 选举结果摘要
    char created_at[20];      // 创建时间
    /* ── JOIN 展示字段 ── */
    char club_name[101];      // 社团名称（JOIN 查询填充）
} Election;

/** 选举候选人 */
typedef struct {
    int  candidate_id;        // 候选人ID（主键，自增）
    int  election_id;         // 所属选举ID
    int  user_id;             // 候选人用户ID
    char position[20];        // 竞选职位：president(社长)/vice_president(副社长)
    char signup_time[20];     // 报名时间
    /* ── JOIN 展示字段 ── */
    char user_name[51];       // 候选人姓名（JOIN 查询填充）
} ElectionCandidate;

/** 选举投票记录 */
typedef struct {
    int  vote_id;             // 投票ID（主键，自增）
    int  election_id;         // 所属选举ID
    int  candidate_id;        // 被投票候选人ID
    int  voter_id;            // 投票人用户ID（匿名投票时不记录，为 0）
    char position[20];        // 投票职位：president(社长)/vice_president(副社长)
    char voted_at[20];        // 投票时间
} ElectionVote;

/** 系统通知 */
typedef struct {
    int  notification_id;     // 通知ID（主键，自增）
    int  user_id;             // 接收用户ID
    char title[201];          // 通知标题
    char content[2001];       // 通知内容
    char type[51];            // 通知类型（如 club_apply_result、join_result 等）
    int  related_id;          // 关联业务ID（无关联时为 0）
    int  is_read;             // 是否已读：0 未读 1 已读
    char created_at[20];      // 通知时间
} Notification;

/** 公告 */
typedef struct {
    int  announcement_id;     // 公告ID（主键，自增）
    char title[201];          // 公告标题
    char content[2001];       // 公告内容
    char scope[15];           // 发布范围：school(全校)/college(学院)/club(社团)
    int  college_id;          // 目标学院ID（scope=college 时有效）
    int  club_id;             // 目标社团ID（scope=club 时有效）
    int  publisher_id;        // 发布人用户ID
    char created_at[20];      // 发布时间
    /* ── JOIN 展示字段 ── */
    char publisher_name[51];  // 发布人姓名（JOIN 查询填充）
} Announcement;

/** 用户限制记录 */
typedef struct {
    int  restriction_id;      // 限制记录ID（主键，自增）
    int  user_id;             // 被限制用户ID
    char restriction_type[20];// 限制类型：ban_join(禁止加入社团)/ban_activity(禁止参加活动)
    int  restricted_by;       // 设置限制的管理员用户ID
    char start_time[20];      // 限制开始时间
    char end_time[20];        // 限制结束时间（空串表示永久）
    char reason[501];         // 限制原因
    int  is_active;           // 是否生效中：1 生效 0 已解除
    char created_at[20];      // 创建时间
    /* ── JOIN 展示字段 ── */
    char user_name[51];       // 被限制用户姓名（JOIN 查询填充）
    char restricted_by_name[51];// 设置限制的管理员姓名（JOIN 查询填充）
} UserRestriction;

/** 操作日志 */
typedef struct {
    int  log_id;              // 日志ID（主键，自增）
    int  user_id;             // 操作用户ID
    char action[101];         // 操作动作描述
    char target_type[51];     // 操作对象类型（如 club、activity 等）
    int  target_id;           // 操作对象ID
    char detail[2001];        // 操作详情
    char created_at[20];      // 操作时间
} LogEntry;

/* ============ 统计模型 ============ */

/** 系统全局统计数据 */
typedef struct {
    int    total_clubs;       // 社团总数
    int    total_members;     // 成员总数
    int    total_activities;  // 活动总数
    int    total_participants;// 活动参与总人次
    double total_income;      // 总收入
    double total_expense;     // 总支出
    int    pending_approvals; // 待审批事项数
    int    active_elections;  // 进行中的选举数
} SystemStats;

/** 单个社团统计数据 */
typedef struct {
    int    club_id;           // 社团ID
    char   club_name[101];    // 社团名称
    int    member_count;      // 成员数
    int    activity_count;    // 活动数
    double total_income;      // 总收入
    double total_expense;     // 总支出
    double balance;           // 结余（收入-支出）
} ClubStats;

/** 选举统计数据 */
typedef struct {
    int  election_id;         // 选举ID
    int  club_id;             // 社团ID
    char club_name[101];      // 社团名称
    char status[15];          // 选举状态
    int  candidate_count;     // 候选人总数
    int  voter_count;         // 已投票人数
    char vote_end[20];        // 投票截止时间
} ElectionStats;

/* ============ 分页模型 ============ */

/** 分页信息 */
typedef struct {
    int page;                 // 当前页码
    int page_size;            // 每页条数
    int total;                // 总记录数
    int total_pages;          // 总页数
} Pagination;

#endif /* MODELS_H */
