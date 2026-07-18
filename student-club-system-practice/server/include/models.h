/**
 * models.h — 数据模型结构体定义
 * 学生社团管理系统 SCMS
 *
 * 每个结构体对应一张数据表；带 "JOIN 展示字段" 注释的成员
 * 不属于本表，是联表查询时用于展示的附加字段。
 */
#ifndef MODELS_H
#define MODELS_H

/* ============ 基础模型 ============ */

typedef struct {
    int  college_id;
    char college_name[101];
    char college_code[21];
    char description[501];
    int  status;
    char created_at[20];
    char updated_at[20];
} College;

typedef struct {
    int  major_id;
    int  college_id;
    char major_name[101];
    char major_code[21];
    int  status;
    char created_at[20];
} Major;

typedef struct {
    int  class_id;
    int  major_id;
    int  college_id;
    char class_name[101];
    char grade[11];
    int  status;
    char created_at[20];
} Class;

typedef struct {
    int  user_id;
    char username[51];
    char password[65];              /* SHA256 十六进制 */
    char real_name[51];
    char role[20];                  /* school_admin/college_admin/club_admin/club_member/general_student */
    int  college_id;
    int  major_id;
    int  class_id;
    char student_no[11];            /* 10 位学号 */
    char id_card[19];               /* 18 位身份证 */
    char phone[12];                 /* 11 位手机号 */
    char email[101];
    int  status;                    /* 1 正常 0 禁用 */
    char last_login[20];
    char created_at[20];
    char updated_at[20];
    /* JOIN 展示字段 */
    char college_name[101];
    char major_name[101];
    char class_name[101];
} User;

typedef struct {
    int  club_id;
    char club_name[101];
    char description[501];
    char category[51];
    char logo_path[256];
    char level[10];                 /* school / college */
    int  college_id;
    int  president_id;
    int  vice_president_count;
    char advisor[51];
    int  member_count;
    char join_permission[15];       /* all / college_only */
    char status[20];                /* pending/approved/rejected/archived/dissolved */
    char reject_reason[256];
    int  created_by;
    char created_at[20];
    char updated_at[20];
    /* JOIN 展示字段 */
    char college_name[101];
    char president_name[51];
} Club;

typedef struct {
    int  member_id;
    int  club_id;
    int  user_id;
    char role[20];                  /* president/vice_president/secretary/treasurer/member */
    char join_status[12];           /* pending/approved/rejected */
    char joined_at[20];
    char left_at[20];               /* 空串 = 在籍 */
    /* JOIN */
    char user_name[51];
    char club_name[101];
    char student_no[11];
} Member;

typedef struct {
    int  activity_id;
    int  club_id;
    char title[201];
    char description[1001];
    char location[201];
    char start_time[20];
    char end_time[20];
    char signup_deadline[20];
    int  max_participants;
    int  current_count;
    int  enable_signin;
    char signin_code[7];            /* 6 位 */
    int  require_summary;
    char summary[2001];
    char status[20];
    char cancel_reason[256];
    char created_at[20];
    char updated_at[20];
    /* JOIN */
    char club_name[101];
} Activity;

typedef struct {
    int  registration_id;
    int  activity_id;
    int  user_id;
    char status[15];                /* registered/cancelled/attended */
    char registered_at[20];
    char cancelled_at[20];
    /* JOIN */
    char activity_title[201];
    char user_name[51];
} Registration;

typedef struct {
    int  signin_id;
    int  activity_id;
    int  user_id;
    char signin_method[10];         /* code / manual */
    char signin_time[20];
    /* JOIN */
    char user_name[51];
} ActivitySignin;

typedef struct {
    int    finance_id;
    int    club_id;
    char   type[10];                /* income / expense */
    double amount;
    char   source[51];              /* 拨款/赞助/活动收入 */
    char   description[501];
    int    operator_id;
    char   record_time[20];
    char   receipt_path[256];
    /* JOIN */
    char   operator_name[51];
    char   club_name[101];
} FinanceRecord;

typedef struct {
    int    reimbursement_id;
    int    club_id;
    int    applicant_id;
    double amount;
    char   description[501];
    char   receipt_path[256];
    char   status[12];              /* pending/approved/rejected */
    int    reviewer_id;
    char   review_comment[501];
    char   submitted_at[20];
    char   reviewed_at[20];
    /* JOIN */
    char   club_name[101];
    char   applicant_name[51];
    char   reviewer_name[51];
} Reimbursement;

typedef struct {
    int    limit_id;
    int    club_id;
    double single_limit;
    char   period_type[15];         /* monthly / semesterly */
    double period_limit;
    double current_period_used;
    char   period_start[11];        /* DATE */
    int    set_by;
    char   updated_at[20];
} ReimbursementLimit;

typedef struct {
    int  election_id;
    int  club_id;
    int  initiated_by;
    char term_start[11];
    char term_end[11];
    char signup_start[20];
    char signup_end[20];
    char vote_start[20];
    char vote_end[20];
    int  president_count;
    int  vice_president_count;
    char status[15];                /* signup/voting/completed/cancelled */
    char result_summary[2001];
    char created_at[20];
    /* JOIN */
    char club_name[101];
} Election;

typedef struct {
    int  candidate_id;
    int  election_id;
    int  user_id;
    char position[20];              /* president / vice_president */
    char signup_time[20];
    /* JOIN */
    char user_name[51];
} ElectionCandidate;

typedef struct {
    int  vote_id;
    int  election_id;
    int  candidate_id;
    int  voter_id;
    char position[20];
    char voted_at[20];
} ElectionVote;

typedef struct {
    int  notification_id;
    int  user_id;
    char title[201];
    char content[2001];
    char type[51];
    int  related_id;
    int  is_read;                   /* 0 未读 1 已读 */
    char created_at[20];
} Notification;

typedef struct {
    int  announcement_id;
    char title[201];
    char content[2001];
    char scope[15];                 /* school/college/club */
    int  college_id;
    int  club_id;
    int  publisher_id;
    char created_at[20];
    /* JOIN */
    char publisher_name[51];
} Announcement;

typedef struct {
    int  restriction_id;
    int  user_id;
    char restriction_type[20];      /* ban_join / ban_activity */
    int  restricted_by;
    char start_time[20];
    char end_time[20];              /* 空串 = 永久 */
    char reason[501];
    int  is_active;
    char created_at[20];
    /* JOIN */
    char user_name[51];
    char restricted_by_name[51];
} UserRestriction;

typedef struct {
    int  log_id;
    int  user_id;
    char action[101];
    char target_type[51];
    int  target_id;
    char detail[2001];
    char created_at[20];
} LogEntry;

/* ============ 统计模型 ============ */

typedef struct {
    int    total_clubs;
    int    total_members;
    int    total_activities;
    int    total_participants;
    double total_income;
    double total_expense;
    int    pending_approvals;
    int    active_elections;
} SystemStats;

typedef struct {
    int    club_id;
    char   club_name[101];
    int    member_count;
    int    activity_count;
    double total_income;
    double total_expense;
    double balance;
} ClubStats;

typedef struct {
    int  election_id;
    int  club_id;
    char club_name[101];
    char status[15];
    int  candidate_count;
    int  voter_count;
    char vote_end[20];
} ElectionStats;

/* ============ 分页模型 ============ */

typedef struct {
    int page;
    int page_size;
    int total;
    int total_pages;
} Pagination;

#endif /* MODELS_H */
