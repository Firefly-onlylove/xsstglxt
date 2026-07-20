# SCMS（学生社团管理系统）架构文档

## 1. 项目概览

SCMS 是一个**单体架构**的 Web 应用系统，由 **C11 后端** + **Vue 3 前端**组成，编译为单个可执行文件 `scms.exe`，同时提供 API 服务和静态文件服务。

```
student-club-system/
├── client/          # Vue 3 前端（Vite + Element Plus）
├── database/        # SQL 迁移脚本及完整导出
├── docs/            # 文档
├── public/          # 前端构建产物（scms.exe 直接托管）
├── receipts/        # 上传的报销凭证文件
├── server/          # C 后端源码
│   ├── include/     # 头文件
│   └── src/         # 源文件
│       ├── common/       # 公共基础设施
│       ├── mongoose/     # 嵌入式 HTTP 库
│       ├── school/       # 学校管理员接口
│       ├── college/      # 学院管理员接口
│       └── student/      # 学生接口
│           └── club_admin/  # 社团管理接口
├── storage/         # 备份、导出、凭证等运行时数据
├── db_config.ini    # 数据库连接配置
├── libmysql.dll     # MySQL C 客户端库
├── scms.exe         # 编译产物（单体服务端）
└── start.bat        # 一键启动脚本
```

---

## 2. 技术栈

| 层级 | 技术 | 版本/标准 |
|------|------|-----------|
| **后端语言** | C11 | GCC (MinGW-w64) |
| **HTTP 服务** | Mongoose | 嵌入式单文件库 |
| **数据库** | MySQL 8.0 | C API 直连 |
| **前端框架** | Vue 3 | ^3.4.0 |
| **构建工具** | Vite | ^5.2.0 |
| **UI 框架** | Element Plus | ^2.7.0 |
| **前端路由** | Vue Router 4 | ^4.3.0 (Hash 模式) |
| **认证** | SHA256 + Cookie | 自行实现，无 OpenSSL 依赖 |

---

## 3. 后端架构

### 3.1 启动流程

```
main.c
  ├── load_config("db_config.ini")    → 读取数据库配置
  ├── db_init(&cfg)                   → 初始化连接池（默认5个连接，互斥锁保护）
  ├── auth_init()                     → 初始化会话池（内存哈希表，最多1024槽位）
  ├── mg_http_listen(0.0.0.0:8000)    → 启动 Mongoose HTTP 服务
  └── mg_mgr_poll(200ms)              → 事件循环
```

### 3.2 请求处理流

```
HTTP 请求
  ├── OPTIONS       → 返回 CORS 头（204）
  ├── /api/*        → dispatch_api()
  │   ├── POST /api/login    → handle_login()     (内置)
  │   ├── POST /api/logout   → handle_logout()    (内置)
  │   ├── GET  /api/me       → handle_me()        (内置)
  │   └── 其余               → 遍历路由表匹配，调用对应 handler
  ├── /receipts/*   → 静态文件服务（凭证图片）
  └── 其他           → public/ 静态文件（Vue SPA）
```

### 3.3 源码目录结构

#### 入口

| 文件 | 说明 |
|------|------|
| `src/main.c` | 程序入口：读配置 → 初始化 DB/认证 → 启动 HTTP → 事件循环 |

#### 公共基础设施 (`src/common/`)

| 文件 | 说明 |
|------|------|
| `api.c` | API 辅助层：参数读取（query/form/JSON/path）、JSON 响应构建、权限检查、文件上传 |
| `auth.c` | 认证与会话管理：SHA256 密码校验、Session Token 生成、内存哈希表存储 |
| `db.c` | 数据库访问层：连接池管理、`db_query()`/`db_execute()`、事务、转义、结果集→JSON |
| `handlers_stub.c` | 弱符号桩函数：所有 handler 的默认空实现，允许模块逐步开发 |
| `notification.c` | 通知服务：单人发送、全校/全院/全社团广播 |
| `routes.c` | 静态路由表：~117 条路由，映射 HTTP 方法+URI 模式到 handler 函数 |
| `utils.c` | 工具函数：SHA256 实现、密码验证、输入校验、字符串安全操作、随机码生成 |

#### 学校管理员 (`src/school/`)

| 文件 | 说明 |
|------|------|
| `announcement.c` | 公告管理：发布/列表/删除（校级范围） |
| `backup.c` | 数据库备份：手动备份（调用 mysqldump）、备份记录查询 |
| `club_approval.c` | 社团审批：待审批列表、通过（自动创建社长成员）、驳回、历史记录 |
| `club_mgmt.c` | 社团管理：全社团列表、设置级别、解散、重命名 |
| `finance_oversight.c` | 财务监督：经费总览、社团明细、报销审批（通过/驳回）、学院报销记录 |
| `school_dashboard.c` | 仪表盘：汇总统计（社团/成员/活动/收支/待审批数） |
| `statistics.c` | 数据统计：详细统计、CSV 导出 |
| `system_config.c` | 系统配置：键值对配置读写 |
| `user_mgmt.c` | 用户管理：列表/详情、创建学院管理员、启用/禁用、重置密码、限制/解除 |

#### 学院管理员 (`src/college/`)

| 文件 | 说明 |
|------|------|
| `announcement.c` | 公告管理（学院范围） |
| `class_mgmt.c` | 班级管理：CRUD |
| `club_oversight.c` | 社团监管：本学院社团列表与详情 |
| `college_dashboard.c` | 仪表盘：学院维度统计 |
| `finance_approval.c` | 财务审批：报销审批（通过/驳回）、报销额度管理 |
| `member_mgmt.c` | 学生管理：本学院学生列表、限制/解除 |

#### 学生端 (`src/student/`)

| 文件 | 说明 |
|------|------|
| `activity.c` | 活动中心：浏览/详情/报名/取消/签到/报名列表 |
| `club_apply.c` | 创建社团申请 |
| `club_browse.c` | 社团广场：浏览/搜索/筛选/详情/成员/活动 |
| `club_join.c` | 加入/退出社团 |
| `notification_center.c` | 通知中心：列表/未读数/已读/全部已读 |
| `profile.c` | 个人中心：查看/编辑资料、修改密码 |
| `register.c` | 学生自助注册 |

#### 社团管理 (`src/student/club_admin/`)

| 文件 | 说明 |
|------|------|
| `activity_mgmt.c` | 活动全生命周期：创建→编辑→发布→开始→结束→取消→总结→删除、签到管理 |
| `announcement.c` | 社团公告发布 |
| `club_dashboard.c` | 社团仪表盘 + 社团资料查看/编辑 |
| `election.c` | 换届选举：创建/列表/详情/参选/候选人列表/申请重选 |
| `election_vote.c` | 投票：匿名投票、结果查询 |
| `finance.c` | 社团财务：收支记录 CRUD、凭证上传、报销申请/列表 |
| `member_mgmt.c` | 成员管理：列表/入社审批/移除/任命角色 |

### 3.4 头文件 (`include/`)

| 文件 | 说明 |
|------|------|
| `common.h` | 全局返回码、分页默认值、角色常量、安全宏 |
| `models.h` | 所有数据库实体 C 结构体定义（20 个实体 + 统计模型） |
| `api.h` | API 辅助层完整接口 |
| `auth.h` | 会话/认证接口 |
| `db.h` | 数据库层接口 |
| `routes.h` | 路由结构体 + 所有 handler 函数声明 |
| `utils.h` | 工具函数声明 |
| `notification.h` | 通知服务接口 |

### 3.5 模块启用机制

所有业务 handler 在 `handlers_stub.c` 中有弱符号空实现（返回"功能尚未实现"）。各业务模块用 `#if 1` / `#if 0` 控制编译，强符号覆盖弱符号桩。

### 3.6 JSON 响应规范

所有 API 统一返回格式：

```json
{ "code": 0, "msg": "ok", "data": { ... } }
```

- `code === 0` 表示成功
- HTTP 状态码统一为 200，前端根据 `code` 判断成败
- 列表接口 data 中包含 `list` 数组和 `total` 总数

---

## 4. 前端架构

### 4.1 目录结构

```
client/src/
├── main.js              # Vue 应用入口，注册 Element Plus + 图标
├── App.vue              # 根组件 <router-view />
├── api/
│   └── index.js         # HTTP 客户端：api.get/post/put/delete/upload()
├── router/
│   └── index.js         # Hash 路由配置 + 导航守卫
├── layouts/
│   └── MainLayout.vue   # 主布局：顶栏导航 + 角色菜单 + 通知铃铛 + 用户下拉
├── components/
│   ├── DataTable.vue    # 通用数据表格（分页 + 插槽列）
│   ├── FilterBar.vue    # 搜索/筛选栏
│   ├── ModalForm.vue    # 通用弹窗表单
│   └── StatCards.vue    # 统计卡片
├── composables/
│   └── useClub.js       # 获取当前用户管理的社团 ID
├── styles/
│   └── theme.css        # 全局主题变量
└── views/
    ├── Login.vue        # 登录/注册页
    ├── NotFound.vue     # 404 页面
    ├── school/          # 学校管理员页面（8 页）
    ├── college/         # 学院管理员页面（6 页）
    ├── student/         # 学生页面（8 页）
    └── club-admin/      # 社团管理页面（5 页）
```

### 4.2 角色-路由对应关系

#### 学校管理员 (`/school/*`) — 8 页

| 路由 | 页面 | 说明 |
|------|------|------|
| `/school/dashboard` | Dashboard.vue | 全校仪表盘 |
| `/school/club-approval` | ClubApproval.vue | 社团注册审批 |
| `/school/club-mgmt` | ClubMgmt.vue | 社团管理 |
| `/school/user-mgmt` | UserMgmt.vue | 用户管理 |
| `/school/finance` | Finance.vue | 财务监督 |
| `/school/statistics` | Statistics.vue | 数据统计 |
| `/school/announcement` | Announcement.vue | 公告管理 |
| `/school/system-config` | SystemConfig.vue | 系统配置 |

#### 学院管理员 (`/college/*`) — 6 页

| 路由 | 页面 | 说明 |
|------|------|------|
| `/college/dashboard` | Dashboard.vue | 学院仪表盘 |
| `/college/class-mgmt` | ClassMgmt.vue | 班级管理 |
| `/college/club-oversight` | ClubOversight.vue | 社团监管 |
| `/college/finance-approval` | FinanceApproval.vue | 财务审批 |
| `/college/student-mgmt` | StudentMgmt.vue | 学生管理 |
| `/college/announcement` | Announcement.vue | 公告管理 |

#### 学生 (`/student/*`) — 8 页

| 路由 | 页面 | 说明 |
|------|------|------|
| `/student/club-square` | ClubSquare.vue | 社团广场 |
| `/student/club-detail/:id` | ClubDetail.vue | 社团详情 |
| `/student/activity-center` | ActivityCenter.vue | 活动中心 |
| `/student/activity/:id` | ActivityDetail.vue | 活动详情 |
| `/student/my-clubs` | MyClubs.vue | 我的社团 |
| `/student/profile` | Profile.vue | 个人中心 |
| `/student/messages` | MessageCenter.vue | 消息中心 |
| `/student/create-club` | CreateClub.vue | 创建社团 |

#### 社团管理 (`/club-admin/*`) — 5 页

| 路由 | 页面 | 说明 |
|------|------|------|
| `/club-admin/dashboard` | Dashboard.vue | 社团仪表盘 |
| `/club-admin/member-mgmt` | MemberMgmt.vue | 成员管理 |
| `/club-admin/activity-mgmt` | ActivityMgmt.vue | 活动管理 |
| `/club-admin/finance` | Finance.vue | 社团财务 |
| `/club-admin/election` | Election.vue | 换届选举 |

### 4.3 导航守卫

```
router.beforeEach
  ├── 公开路由 (/login) → 直接放行
  └── 受保护路由 → GET /api/me 校验登录 + 角色匹配
      ├── 未登录 → 重定向 /login
      ├── 角色不匹配 → 重定向到角色首页
      └── 通过 → 放行
```

### 4.4 API 请求模式

```js
// 前端 API 层封装 fetch()，自动携带 Cookie
const res = await api.get('/api/xxx', params)
if (res.code === 0) {
  data.value = res.data.list || []
  total.value = res.data.total || 0
}
```

`normalizeData()` 自动将裸数组包装为 `{list, total}` 格式。

---

## 5. 数据库架构

### 5.1 连接信息

- **数据库**: `scms_db`
- **字符集**: `utf8mb4`（客户端连接需 `SET NAMES utf8mb4`）
- **引擎**: InnoDB
- **连接池**: 默认 5 连接，最大 16

### 5.2 表结构总览（21 张表）

#### 基础数据

| 表名 | 说明 | 关键字段 |
|------|------|----------|
| `colleges` | 学院 | college_id, college_name, college_code, status |
| `majors` | 专业 | major_id, college_id, major_name, major_code |
| `classes` | 班级 | class_id, major_id, college_id, class_name, grade |

#### 用户与权限

| 表名 | 说明 | 关键字段 |
|------|------|----------|
| `users` | 用户（全部角色） | user_id, username, password(SHA256), real_name, role ENUM(school_admin/college_admin/club_admin/club_member/general_student), college_id, student_no, status |
| `user_restrictions` | 用户限制 | restriction_id, user_id, restriction_type(ban_join/ban_activity), reason, is_active |

#### 社团体系

| 表名 | 说明 | 关键字段 |
|------|------|----------|
| `clubs` | 社团 | club_id, club_name, level ENUM(school/college), college_id, president_id, member_count, status ENUM(pending/approved/rejected/archived/dissolved) |
| `members` | 社团成员 | member_id, club_id, user_id, role ENUM(president/vice_president/secretary/treasurer/member), join_status ENUM(pending/approved/rejected) |

#### 活动体系

| 表名 | 说明 | 关键字段 |
|------|------|----------|
| `activities` | 活动 | activity_id, club_id, title, max_participants, current_count, signin_code, status ENUM(draft/published/ongoing/completed/cancelled/archived) |
| `registrations` | 活动报名 | registration_id, activity_id, user_id, status ENUM(registered/cancelled/attended) |
| `activity_signins` | 签到记录 | signin_id, activity_id, user_id, signin_method ENUM(code/manual) |

#### 财务体系

| 表名 | 说明 | 关键字段 |
|------|------|----------|
| `finance` | 财务流水 | finance_id, club_id, type ENUM(income/expense), amount, receipt_path |
| `reimbursement` | 报销申请 | reimbursement_id, club_id, applicant_id, amount, receipt_path, status ENUM(pending/approved/rejected) |
| `reimbursement_limits` | 报销额度 | limit_id, club_id, single_limit, period_type ENUM(monthly/semesterly), period_limit, current_period_used |

#### 选举体系

| 表名 | 说明 | 关键字段 |
|------|------|----------|
| `elections` | 换届选举 | election_id, club_id, term_start/end, signup_start/end, vote_start/end, status ENUM(signup/voting/completed/cancelled) |
| `election_candidates` | 候选人 | candidate_id, election_id, user_id, position ENUM(president/vice_president) |
| `election_votes` | 投票记录 | vote_id, election_id, candidate_id, voter_id, position |

#### 消息与公告

| 表名 | 说明 | 关键字段 |
|------|------|----------|
| `notifications` | 通知消息 | notification_id, user_id, title, content, type, is_read |
| `announcements` | 公告 | announcement_id, title, content, scope ENUM(school/college/club), college_id, club_id |

#### 系统运维

| 表名 | 说明 | 关键字段 |
|------|------|----------|
| `logs` | 操作日志 | log_id, user_id, action, target_type, target_id, detail |
| `backup_records` | 备份记录 | backup_id, backup_type ENUM(auto/manual), file_path, file_size |
| `system_config` | 系统配置 | config_id, config_key(UNIQUE), config_value, description |

### 5.3 触发器（5 个）

| 触发器 | 用途 |
|--------|------|
| `trg_member_after_insert` | 成员批准时自动 `clubs.member_count + 1` |
| `trg_member_after_update` | 成员状态变更时维护 `member_count`（批准+1，退出-1） |
| `trg_registration_after_insert` | 活动报名时自动 `activities.current_count + 1` |
| `trg_registration_after_update` | 取消报名时自动 `activities.current_count - 1` |
| `trg_reimbursement_after_update` | 报销批准时累加 `reimbursement_limits.current_period_used` |

---

## 6. 完整 API 路由表（117 条）

### 6.1 公开接口（无需登录）

| 方法 | 路径 | Handler |
|------|------|---------|
| POST | `/api/register` | `pub_register` |
| GET | `/api/colleges` | `pub_colleges_list` |
| GET | `/api/majors` | `pub_majors_list` |
| GET | `/api/classes` | `pub_classes_list` |

### 6.2 通知中心

| 方法 | 路径 | Handler |
|------|------|---------|
| GET | `/api/notifications` | `pub_notif_list` |
| GET | `/api/notifications/unread` | `pub_notif_unread` |
| POST | `/api/notifications/read` | `pub_notif_read` |
| POST | `/api/notifications/read-all` | `pub_notif_read_all` |

### 6.3 学校管理员 — 仪表盘与统计

| 方法 | 路径 | Handler |
|------|------|---------|
| GET | `/api/school/dashboard` | `sch_dashboard` |
| GET | `/api/school/stats` | `sch_stats` |
| GET | `/api/school/stats/export` | `sch_stats_export` |

### 6.4 学校管理员 — 社团管理

| 方法 | 路径 | Handler |
|------|------|---------|
| GET | `/api/school/clubs` | `sch_club_list` |
| GET | `/api/school/clubs/pending` | `sch_club_pending` |
| GET | `/api/school/clubs/history` | `sch_club_approved_history` |
| GET | `/api/school/clubs/*` | `sch_club_detail` |
| POST | `/api/school/clubs/*/approve` | `sch_club_approve` |
| POST | `/api/school/clubs/*/reject` | `sch_club_reject` |
| POST | `/api/school/clubs/*/level` | `sch_club_set_level` |
| POST | `/api/school/clubs/*/dissolve` | `sch_club_dissolve` |
| POST | `/api/school/clubs/*/rename` | `sch_club_rename` |

### 6.5 学校管理员 — 用户管理

| 方法 | 路径 | Handler |
|------|------|---------|
| GET | `/api/school/users` | `sch_user_list` |
| GET | `/api/school/users/*` | `sch_user_detail` |
| POST | `/api/school/users/college-admin` | `sch_user_create_college_admin` |
| POST | `/api/school/users/*/toggle` | `sch_user_toggle` |
| POST | `/api/school/users/*/reset-password` | `sch_user_reset_pwd` |
| POST | `/api/school/users/*/restrict` | `sch_user_restrict` |
| POST | `/api/school/users/*/lift-restriction` | `sch_user_lift_restrict` |
| GET | `/api/school/restrictions` | `sch_restriction_list` |

### 6.6 学校管理员 — 财务监督

| 方法 | 路径 | Handler |
|------|------|---------|
| GET | `/api/school/finance` | `sch_finance_overview` |
| GET | `/api/school/finance/clubs` | `sch_finance_overview` |
| GET | `/api/school/finance/*` | `sch_finance_club` |
| GET | `/api/school/reimbursements/pending` | `sch_reimb_pending` |
| GET | `/api/school/reimbursements/college` | `sch_reimb_college_list` |
| POST | `/api/school/reimbursements/*/approve` | `sch_reimb_approve` |
| POST | `/api/school/reimbursements/*/reject` | `sch_reimb_reject` |

### 6.7 学校管理员 — 公告、备份、配置

| 方法 | 路径 | Handler |
|------|------|---------|
| GET | `/api/school/announcements` | `sch_announce_list` |
| POST | `/api/school/announcements` | `sch_announce_post` |
| DELETE | `/api/school/announcements/*` | `sch_announce_delete` |
| GET | `/api/school/backups` | `sch_backup_list` |
| POST | `/api/school/backup` | `sch_backup_create` |
| GET | `/api/school/config` | `sch_config_get` |
| POST | `/api/school/config` | `sch_config_set` |

### 6.8 学院管理员 — 仪表盘与统计

| 方法 | 路径 | Handler |
|------|------|---------|
| GET | `/api/college/dashboard` | `col_dashboard` |
| GET | `/api/college/stats` | `col_stats` |
| GET | `/api/college/stats/export` | `col_stats_export` |

### 6.9 学院管理员 — 班级管理

| 方法 | 路径 | Handler |
|------|------|---------|
| GET | `/api/college/classes` | `col_class_list` |
| POST | `/api/college/classes` | `col_class_create` |
| PUT | `/api/college/classes/*` | `col_class_update` |
| DELETE | `/api/college/classes/*` | `col_class_delete` |

### 6.10 学院管理员 — 社团监管

| 方法 | 路径 | Handler |
|------|------|---------|
| GET | `/api/college/clubs` | `col_club_list` |
| GET | `/api/college/clubs/*` | `col_club_detail` |

### 6.11 学院管理员 — 财务审批

| 方法 | 路径 | Handler |
|------|------|---------|
| GET | `/api/college/reimbursements/pending` | `col_reimb_pending` |
| POST | `/api/college/reimbursements/*/approve` | `col_reimb_approve` |
| POST | `/api/college/reimbursements/*/reject` | `col_reimb_reject` |
| GET | `/api/college/limits` | `col_limit_list` |
| POST | `/api/college/limits/*` | `col_limit_set` |

### 6.12 学院管理员 — 学生管理

| 方法 | 路径 | Handler |
|------|------|---------|
| GET | `/api/college/students` | `col_student_list` |
| POST | `/api/college/students/*/restrict` | `col_student_restrict` |
| POST | `/api/college/students/*/lift-restriction` | `col_student_lift` |

### 6.13 学院管理员 — 公告

| 方法 | 路径 | Handler |
|------|------|---------|
| GET | `/api/college/announcements` | `col_announce_list` |
| POST | `/api/college/announcements` | `col_announce_post` |
| DELETE | `/api/college/announcements/*` | `col_announce_delete` |

### 6.14 学生端 — 社团广场

| 方法 | 路径 | Handler |
|------|------|---------|
| GET | `/api/clubs` | `stu_club_browse` |
| GET | `/api/clubs/*` | `stu_club_detail` |
| GET | `/api/clubs/*/members` | `stu_club_members` |
| GET | `/api/clubs/*/activities` | `stu_club_activities` |
| POST | `/api/clubs/apply` | `stu_club_apply` |
| POST | `/api/clubs/*/join` | `stu_club_join` |
| POST | `/api/clubs/*/leave` | `stu_club_leave` |

### 6.15 学生端 — 活动

| 方法 | 路径 | Handler |
|------|------|---------|
| GET | `/api/activities` | `stu_activity_list` |
| GET | `/api/activities/*` | `stu_activity_detail` |
| POST | `/api/activities/*/register` | `stu_activity_register` |
| POST | `/api/activities/*/cancel` | `stu_activity_cancel` |
| POST | `/api/activities/*/signin` | `stu_activity_signin` |
| GET | `/api/activities/*/registrations` | `stu_activity_registrations` |

### 6.16 学生端 — 个人中心与我的社团

| 方法 | 路径 | Handler |
|------|------|---------|
| GET | `/api/profile` | `stu_profile_get` |
| PUT | `/api/profile` | `stu_profile_update` |
| POST | `/api/profile/password` | `stu_change_password` |
| GET | `/api/my/clubs` | `stu_my_clubs` |
| GET | `/api/my/applications` | `stu_my_applications` |

### 6.17 社团管理 — 仪表盘与资料

| 方法 | 路径 | Handler |
|------|------|---------|
| GET | `/api/club/*/dashboard` | `club_dashboard` |
| GET | `/api/club/*/profile` | `club_profile_get` |
| PUT | `/api/club/*/profile` | `club_profile_update` |

### 6.18 社团管理 — 成员管理

| 方法 | 路径 | Handler |
|------|------|---------|
| GET | `/api/club/*/members` | `club_member_list` |
| GET | `/api/club/*/join-requests` | `club_join_pending` |
| POST | `/api/club/*/join-requests/*/approve` | `club_join_approve` |
| POST | `/api/club/*/join-requests/*/reject` | `club_join_reject` |
| POST | `/api/club/*/members/*/remove` | `club_member_remove` |
| POST | `/api/club/*/members/*/appoint` | `club_member_appoint` |

### 6.19 社团管理 — 活动管理

| 方法 | 路径 | Handler |
|------|------|---------|
| GET | `/api/club/*/activities` | `club_activity_list` |
| POST | `/api/club/*/activities` | `club_activity_create` |
| PUT | `/api/club/*/activities/*` | `club_activity_update` |
| DELETE | `/api/club/*/activities/*` | `club_activity_delete` |
| POST | `/api/club/*/activities/*/publish` | `club_activity_publish` |
| POST | `/api/club/*/activities/*/start` | `club_activity_start` |
| POST | `/api/club/*/activities/*/finish` | `club_activity_finish` |
| POST | `/api/club/*/activities/*/cancel` | `club_activity_cancel` |
| POST | `/api/club/*/activities/*/summary` | `club_activity_summary` |
| GET | `/api/club/*/activities/*/signins` | `club_signin_list` |
| POST | `/api/club/*/activities/*/manual-signin` | `club_signin_manual` |

### 6.20 社团管理 — 财务

| 方法 | 路径 | Handler |
|------|------|---------|
| GET | `/api/club/*/finance` | `club_finance_list` |
| POST | `/api/club/*/finance` | `club_finance_create` |
| DELETE | `/api/club/*/finance/*` | `club_finance_delete` |
| POST | `/api/club/*/upload-receipt` | `club_reimb_upload` |
| POST | `/api/club/*/reimbursements` | `club_reimb_create` |
| GET | `/api/club/*/reimbursements` | `club_reimb_list` |

### 6.21 社团管理 — 换届选举

| 方法 | 路径 | Handler |
|------|------|---------|
| GET | `/api/club/*/elections` | `club_election_list` |
| POST | `/api/club/*/elections` | `club_election_create` |
| GET | `/api/club/*/elections/*` | `club_election_detail` |
| POST | `/api/club/*/elections/reelect` | `club_election_reelect` |
| POST | `/api/club/*/elections/*/signup` | `club_election_signup` |
| GET | `/api/club/*/elections/*/candidates` | `club_election_candidates` |
| POST | `/api/club/*/elections/*/vote` | `club_election_vote` |
| GET | `/api/club/*/elections/*/result` | `club_election_result` |

### 6.22 社团管理 — 公告

| 方法 | 路径 | Handler |
|------|------|---------|
| POST | `/api/club/*/announcements` | `club_announce_post` |

---

## 7. 构建系统

### 7.1 后端编译

```bash
cd server && make          # 发布构建（-O2）→ ../scms.exe
cd server && make debug    # 调试构建（-g -O0 -DDEBUG）
cd server && make clean    # 清理
```

- 编译器：GCC (MinGW-w64)，路径 `C:\mingw4\mingw64\bin\`
- 标准：C11 (`-std=c11`)
- 链接：MySQL C API (`libmysql.lib`)，Windows Socket (`ws2_32`)
- 输出：单体 `scms.exe`

### 7.2 前端构建

```bash
cd client && npm run dev    # Vite 开发服务器 (:5173)，API 代理 → :8000
cd client && npm run build  # 生产构建 → ../public/
```

---

## 8. 配置说明

`db_config.ini` 结构：

```ini
[database]
host = 127.0.0.1
port = 3306
user = root
password = 123456789
database = scms_db
pool_size = 5

[server]
receipt_dir = storage/receipts
export_dir  = storage/exports
backup_dir  = storage/backups

[app]
auto_backup_hour = 3
session_timeout = 0
```

---

## 9. 关键设计决策

1. **单体二进制** — 一个 `scms.exe` 同时提供 API 和静态文件服务，无需 Nginx 反向代理
2. **内存会话** — 会话存储于内存哈希表，无 Redis 依赖，重启后全部失效
3. **原始 SQL** — 无 ORM 层，直接拼接 SQL（需注意 `db_escape()` 防注入）
4. **弱符号桩** — 通过 `__attribute__((weak))` 实现模块可选编译，允许渐进开发
5. **Hash 路由** — 前端使用 Hash 模式，避免后端静态文件路由与前端路由冲突
6. **Cookie 认证** — 通过 `scms_token` Cookie 传递 32 位十六进制 Token
7. **统一状态码** — 所有 API HTTP 状态码 200，业务状态通过 `code` 字段区分
