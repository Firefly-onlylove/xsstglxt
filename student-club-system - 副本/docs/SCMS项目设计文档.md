# 学生社团管理系统 (SCMS) — 项目设计文档

---

## 版本信息

| 项目 | 说明 |
|------|------|
| **项目名称** | 学生社团管理系统 / Student Club Management System (SCMS) |
| **版本** | v1.0.0 |
| **文档日期** | 2026-07-18 |
| **运行环境** | Windows / Linux |
| **数据库** | MySQL 8.0 |
| **字符集** | utf8mb4 |

---

## 一、项目概述

### 1.1 项目定位

SCMS 是一套面向高校的**学生社团全生命周期管理系统**，覆盖社团的创建审批、成员管理、活动组织、财务报销、换届选举、通知推送等核心业务流程。系统采用 B/S 架构，支持学校管理员、学院管理员、社团管理员、普通学生四类角色。

### 1.2 技术栈

| 层次 | 技术 | 说明 |
|------|------|------|
| **后端语言** | C11 (GCC/MinGW-w64) | 编译为单一可执行文件 scms.exe |
| **HTTP 服务器** | Mongoose 7.x | 轻量嵌入式 Web Server，单文件包含 |
| **数据库** | MySQL 8.0 | C API 直连，连接池管理 |
| **前端框架** | Vue 3 + Vite | SPA 单页应用 |
| **UI 组件库** | Element Plus | 企业级 Vue 3 组件库 |
| **构建工具** | mingw32-make (Windows) / make (Linux) | 自动化编译链接 |

### 1.3 项目目录结构

```
student-club-system/
├── README.md                 # 项目说明
├── .gitignore                # Git 忽略规则
├── scms.exe                  # 编译产物（服务端可执行文件）
├── libmysql.dll              # MySQL 客户端动态库
├── db_config.ini             # 数据库连接配置
│
├── server/                   # 后端 C 源码
│   ├── Makefile              # 编译脚本
│   ├── include/              # 8 个头文件
│   │   ├── common.h          # 全局定义（返回码、宏、常量）
│   │   ├── models.h          # 数据模型结构体（22 个模型）
│   │   ├── api.h             # HTTP/JSON 业务接口辅助层
│   │   ├── db.h              # 数据库访问层接口
│   │   ├── auth.h            # 认证与会话管理
│   │   ├── utils.h           # 工具函数（SHA256、校验、日期）
│   │   ├── notification.h    # 消息通知服务
│   │   └── routes.h          # 路由表声明 + 全部 API 函数原型
│   └── src/
│       ├── main.c            # 程序入口（Mongoose 事件循环 + 路由分发 + 会话）
│       ├── mongoose/         # Mongoose 嵌入式 HTTP 库
│       │   ├── mongoose.c
│       │   └── mongoose.h
│       ├── common/           # 基础设施层（6 个模块）
│       │   ├── api.c         # 请求解析 / JSON 构造 / 文件上传
│       │   ├── auth.c        # 登录认证 / Session 管理
│       │   ├── db.c          # MySQL 连接池 / CRUD / 事务
│       │   ├── notification.c# 通知推送服务
│       │   ├── routes.c      # 路由表定义（~170 条路由）
│       │   └── utils.c       # SHA256 / 校验 / 字符串处理
│       ├── school/           # 学校端（11 个模块）
│       ├── college/          # 学院端（6 个模块）
│       └── student/          # 学生端 + 社团管理端（13 个模块）
│           ├── activity.c / club_browse.c / club_apply.c / club_join.c
│           ├── profile.c / register.c / notification_center.c
│           └── club_admin/   # 社团管理子模块（7 个）
│               ├── activity_mgmt.c / announcement.c / club_dashboard.c
│               ├── election.c / election_vote.c / finance.c / member_mgmt.c
│
├── client/                   # 前端 Vue 3 源码
│   ├── vite.config.js
│   ├── package.json
│   └── src/                  # Vue 组件、路由、状态管理
│
├── database/                 # SQL 脚本
│   ├── 01_create_database.sql   # 建库 + 用户授权
│   ├── 02_create_tables.sql     # 21 张表 DDL
│   ├── 03_triggers.sql          # 触发器（财务自动记账等）
│   ├── 04_init_data.sql         # 初始数据（学院/专业/班级/管理员）
│   └── 05_mock_data.sql         # 测试数据（社团/成员/活动等）
│
├── public/                   # Vue 构建产物（Web 静态根目录）
├── storage/                  # 运行时数据
│   ├── receipts/             # 发票图片上传
│   ├── exports/              # CSV 导出文件
│   └── backups/              # 数据库备份 (.sql)
│
└── docs/                     # 文档
    ├── 设计方案.md
    ├── UI设计需求规格书.md
    ├── 前端UI设计需求文档.md
    ├── 最终项目方案.md
    └── AI负责与自行完成_分工文档.md
```

---

## 二、架构设计

### 2.1 整体架构

```
┌─────────────────────────────────────────────────────┐
│                    浏览器 (Vue 3 SPA)                 │
├─────────────────────────────────────────────────────┤
│              HTTP/JSON (RESTful API)                 │
│              Cookie: scms_token                      │
├─────────────────────────────────────────────────────┤
│           Mongoose HTTP Server (:8000)               │
│  ┌───────────────┐  ┌─────────────────────────────┐ │
│  │ 静态文件服务    │  │  API 请求分发 (dispatch_api) │ │
│  │ / → public/    │  │  /api/* → 路由表匹配         │ │
│  └───────────────┘  └──────────┬──────────────────┘ │
│  ┌─────────────────────────────┴──────────────────┐ │
│  │          API 上下文构建 (ApiContext)             │ │
│  │  1. Cookie → token → auth_resolve_session → User│ │
│  │  2. URI 通配匹配 → 路由表查找 Handler           │ │
│  │  3. 权限校验 (need_login / api_require_*)       │ │
│  └──────────┬──────────────────────────────────────┘ │
│  ┌──────────┴──────────────────────────────────────┐ │
│  │              业务处理函数 (31 个 .c 文件)         │ │
│  │  sch_* (学校端) / col_* (学院端)                  │ │
│  │  stu_* (学生端) / club_* (社团管理端)             │ │
│  │  pub_* (公共/通知)                                │ │
│  └──────────┬──────────────────────────────────────┘ │
│  ┌──────────┴──────────────────────────────────────┐ │
│  │          db_query / db_execute (连接池)          │ │
│  └──────────┬──────────────────────────────────────┘ │
├─────────────┼───────────────────────────────────────┤
│             ▼                                        │
│        MySQL 8.0 (scms_db / utf8mb4)                 │
└─────────────────────────────────────────────────────┘
```

### 2.2 请求生命周期

```
1. 浏览器发起请求 (带 Cookie: scms_token=xxx)
2. Mongoose ev_handler → dispatch_api
3. 构建 ApiContext { conn, hm, user, method }
4. Cookie 解析 → auth_resolve_session → 填充 ctx.user
5. 匹配内建路由 (login/logout/me) 或遍历路由表
6. need_login 检查 → 若需登录且 ctx.user==NULL 则返回 -8
7. 调用 handler(ctx) 执行业务逻辑
8. handler 内部：
   - api_get_* 读取请求参数
   - db_query / db_execute 操作数据库
   - api_send_* / api_ok_* / api_error 返回 JSON
9. 浏览器接收 JSON → Vue 渲染
```

### 2.3 路由匹配机制

URI 模式支持 `*` 通配**单个路径段**（不跨 `/`）：

| 模式 | 匹配 | 不匹配 |
|------|------|--------|
| `/api/school/clubs/*` | `/api/school/clubs/12` | `/api/school/clubs/12/approve` |
| `/api/club/*/activities/*` | `/api/club/1/activities/5` | `/api/club/1/activities` |

**关键规则**：
- 具体路径必须在通配符之前（如 `/api/clubs/apply` 必须在 `/api/clubs/*` 之前）
- Handler 通过 `api_get_path_int(ctx, idx)` 读取 `/api/` 后的第 idx 段

---

## 三、角色体系

### 3.1 四类角色

| 角色 | role 值 | 权限范围 | 典型账号 |
|------|---------|---------|---------|
| **学校管理员** | `school_admin` | 全校管辖：社团审批、用户管理、财务监督、系统配置、公告发布 | admin |
| **学院管理员** | `college_admin` | 本院管辖：社团监督、报销审批(院级)、学生管理、班级管理 | cs_admin |
| **社团管理员** | `club_admin` | 本社管辖：活动管理、成员审批、财务管理、换届选举、公告 | stu_zhang |
| **社团成员** | `club_member` | 加入社团后可参选、投票、报名活动 | stu_cs02 |
| **普通学生** | `general_student` | 浏览社团、申请加入、报名活动、申请创建社团 | stu_cs01 |

### 3.2 权限控制流

```
┌──────────────┐    POST /api/school/announcements    ┌────────────┐
│  请求到达      │ ──────────────────────────────────→ │ 路由表匹配  │
└──────────────┘                                       └─────┬──────┘
                                                             │
                                              ┌────────────────┴────────────────┐
                                              ▼                                              ▼
                                     need_login=1                              need_login=0
                                     ctx->user 非空?                          直接调用 handler
                                              │
                                    ┌─────────┴─────────┐
                                    ▼                   ▼
                              handler 内调用           直接执行业务
                              api_require_school_admin
                              → 检查 ctx->user->role
                              → 不匹配返回 -4 (权限不足)
```

---

## 四、数据库设计

### 4.1 ER 关系图

```
colleges ───1:N──→ majors ───1:N──→ classes
    │                  │
    │ 1:N              │ 1:N
    ▼                  ▼
  clubs              users
    │                  │
    ├─1:N─→ members ──┤ (user_id)
    ├─1:N─→ activities
    │          │
    │          └─1:N─→ registrations ──N:1─→ users
    │
    ├─1:N─→ finance
    ├─1:N─→ elections ──1:N─→ election_candidates ──1:N─→ election_votes
    ├─1:N─→ reimbursement
    └─1:1─→ announcements (scope=club)

users ──1:N─→ notifications
users ──1:N─→ user_restrictions
users ──1:N─→ logs
```

### 4.2 数据表清单 (21 张表)

| # | 表名 | 说明 | 核心字段 |
|---|------|------|---------|
| 1 | `colleges` | 学院 | college_id, college_name, college_code, status |
| 2 | `majors` | 专业 | major_id, college_id, major_name, major_code |
| 3 | `classes` | 班级 | class_id, major_id, college_id, class_name, grade |
| 4 | `users` | 用户 | user_id, username, password(SHA256), role, college_id, status |
| 5 | `clubs` | 社团 | club_id, club_name, level(school/college), status, president_id, created_by |
| 6 | `members` | 社团成员 | member_id, club_id, user_id, role, join_status, joined_at, left_at |
| 7 | `activities` | 活动 | activity_id, club_id, title, status(draft/published/ongoing/completed/cancelled), checkin_code |
| 8 | `registrations` | 活动报名 | registration_id, activity_id, user_id, status(registered/cancelled/attended) |
| 9 | `activity_signins` | 签到记录 | signin_id, activity_id, user_id, signin_method(code/manual) |
| 10 | `finance` | 财务流水 | finance_id, club_id, type(income/expense), amount, source |
| 11 | `reimbursement` | 报销申请 | reimbursement_id, club_id, amount, receipt_path, status(pending/approved/rejected) |
| 12 | `reimbursement_limits` | 报销额度 | limit_id, club_id, single_limit, period_limit |
| 13 | `elections` | 换届选举 | election_id, club_id, status(signup/voting/completed), term_start, term_end |
| 14 | `election_candidates` | 选举候选人 | candidate_id, election_id, user_id, position |
| 15 | `election_votes` | 选举投票 | vote_id, election_id, candidate_id, voter_id |
| 16 | `notifications` | 站内通知 | notification_id, user_id, title, content, type, is_read |
| 17 | `announcements` | 公告 | announcement_id, title, content, scope(school/college/club), publisher_id |
| 18 | `user_restrictions` | 用户限制 | restriction_id, user_id, restriction_type(ban_join/ban_activity), is_active |
| 19 | `logs` | 操作日志 | log_id, user_id, action, target_type, target_id, detail |
| 20 | `system_config` | 系统配置 | config_id, config_key, config_value, description |
| 21 | `backup_records` | 备份记录 | backup_id, backup_type, file_path, file_size, operator_id, status |

### 4.3 触发器和自动化

| 触发器 | 作用 |
|--------|------|
| `trg_finance_update_balance` | finance 表 INSERT/UPDATE 时自动更新 clubs 余额统计 |
| `trg_reimb_approve_to_finance` | 报销审批通过后自动写入 finance 表 |
| `trg_election_settle` | 选举结果产生后自动更新 members.role 和 clubs.president_id |
| `trg_member_count_update` | members 增删时同步 clubs.member_count |
| `trg_activity_count_update` | 活动状态变化时同步统计 |

---

## 五、API 接口设计

### 5.1 通用规范

- **基础路径**: `http://{host}:8000/api/`
- **认证方式**: Cookie `scms_token` (HTTP-Only, SameSite=Lax)
- **请求格式**: `application/json` (GET 用 query string, POST/PUT 用 JSON body)
- **响应格式**: `{"code": 0, "msg": "ok", "data": {...}}`

### 5.2 返回码定义

| 返回码 | 常量 | 含义 |
|--------|------|------|
| 0 | OK | 成功 |
| -1 | ERR_DB | 数据库错误 |
| -2 | ERR_NOT_FOUND | 记录不存在 |
| -3 | ERR_DUPLICATE | 重复记录 |
| -4 | ERR_PERMISSION | 权限不足 |
| -5 | ERR_VALIDATION | 数据校验失败 |
| -6 | ERR_FULL | 名额已满 |
| -7 | ERR_STATUS | 状态不允许 |
| -8 | ERR_AUTH | 认证失败 |
| -9 | ERR_INPUT | 输入参数错误 |
| -10 | ERR_QUOTA_EXCEED | 超出额度 |
| -11 | ERR_RESTRICTED | 用户被限制 |
| -12 | ERR_ELECTION | 选举流程错误 |
| -13 | ERR_VOTE_TIED | 平票 |
| -99 | ERR_SYSTEM | 系统级错误 |

### 5.3 完整 API 端点清单 (97 个)

#### 公共接口 (无需登录或全角色，6个)

| 方法 | 端点 | 处理函数 | 说明 |
|------|------|---------|------|
| POST | `/api/register` | pub_register | 用户注册 |
| POST | `/api/login` | handle_login | 登录(内建) |
| POST | `/api/logout` | handle_logout | 登出(内建) |
| GET | `/api/me` | handle_me | 当前用户(内建) |
| GET | `/api/colleges` | pub_colleges_list | 学院下拉列表 |
| GET | `/api/majors` | pub_majors_list | 专业下拉列表(可按college_id筛选) |
| GET | `/api/classes` | pub_classes_list | 班级下拉列表(可按major_id筛选) |

#### 通知中心 (需登录，4个)

| 方法 | 端点 | 处理函数 | 说明 |
|------|------|---------|------|
| GET | `/api/notifications` | pub_notif_list | 通知列表(分页, 可筛选未读) |
| GET | `/api/notifications/unread` | pub_notif_unread | 未读数量 |
| POST | `/api/notifications/read` | pub_notif_read | 标记单条已读 |
| POST | `/api/notifications/read-all` | pub_notif_read_all | 全部已读 |

#### 学校端 — 仪表盘/统计 (3个)

| 方法 | 端点 | 处理函数 | 说明 |
|------|------|---------|------|
| GET | `/api/school/dashboard` | sch_dashboard | 仪表盘(社团数/成员数/活动数/待审批数) |
| GET | `/api/school/stats` | sch_stats | 分类/学院统计 |
| GET | `/api/school/stats/export` | sch_stats_export | CSV导出 |

#### 学校端 — 社团审批 (4个)

| 方法 | 端点 | 处理函数 | 说明 |
|------|------|---------|------|
| GET | `/api/school/clubs/pending` | sch_club_pending | 待审批社团列表 |
| GET | `/api/school/clubs/history` | sch_club_approved_history | 审批历史 |
| POST | `/api/school/clubs/{id}/approve` | sch_club_approve | 批准建社(设级别/自动任命社长) |
| POST | `/api/school/clubs/{id}/reject` | sch_club_reject | 驳回建社(需填原因) |

#### 学校端 — 社团管理 (4个)

| 方法 | 端点 | 处理函数 | 说明 |
|------|------|---------|------|
| GET | `/api/school/clubs` | sch_club_list | 所有社团列表 |
| GET | `/api/school/clubs/{id}` | sch_club_detail | 社团详情 |
| POST | `/api/school/clubs/{id}/level` | sch_club_set_level | 设置社团级别 |
| POST | `/api/school/clubs/{id}/dissolve` | sch_club_dissolve | 解散社团 |

#### 学校端 — 学院/专业管理 (8个)

| 方法 | 端点 | 说明 |
|------|------|------|
| GET/POST | `/api/school/colleges` | 学院列表/创建 |
| PUT | `/api/school/colleges/{id}` | 更新学院 |
| POST | `/api/school/colleges/{id}/toggle` | 启停学院 |
| GET/POST | `/api/school/majors` | 专业列表/创建 |
| PUT | `/api/school/majors/{id}` | 更新专业 |
| DELETE | `/api/school/majors/{id}` | 删除专业 |

#### 学校端 — 用户管理 (9个)

| 方法 | 端点 | 说明 |
|------|------|------|
| GET | `/api/school/users` | 用户列表 |
| GET | `/api/school/users/{id}` | 用户详情 |
| POST | `/api/school/users/college-admin` | 创建学院管理员 |
| POST | `/api/school/users/{id}/toggle` | 禁用/启用用户 |
| POST | `/api/school/users/{id}/reset-password` | 重置密码 |
| POST | `/api/school/users/{id}/restrict` | 限制用户 |
| POST | `/api/school/users/{id}/lift-restriction` | 解除限制 |
| GET | `/api/school/restrictions` | 限制记录列表 |

#### 学校端 — 财务监督 (5个)

| 方法 | 端点 | 说明 |
|------|------|------|
| GET | `/api/school/finance` | 全校财务总览 |
| GET | `/api/school/finance/clubs` | 各社团财务汇总 |
| GET | `/api/school/finance/{club_id}` | 某社团财务明细 |
| GET | `/api/school/reimbursements/pending` | 待审校级报销 |
| POST | `/api/school/reimbursements/{id}/approve` | 通过报销 |
| POST | `/api/school/reimbursements/{id}/reject` | 驳回报销 |

#### 学校端 — 公告/系统 (7个)

| 方法 | 端点 | 说明 |
|------|------|------|
| GET/POST | `/api/school/announcements` | 公告列表/发布 |
| DELETE | `/api/school/announcements/{id}` | 删除公告 |
| POST | `/api/school/backup` | 创建备份 |
| GET | `/api/school/backups` | 备份列表 |
| GET/POST | `/api/school/config` | 系统配置查看/修改 |

#### 学院端 (13个)

| 方法 | 端点 | 说明 |
|------|------|------|
| GET | `/api/college/dashboard` | 学院仪表盘 |
| GET | `/api/college/stats` | 本院统计 |
| GET | `/api/college/stats/export` | 统计CSV导出 |
| GET/POST/PUT/DELETE | `/api/college/classes[/{id}]` | 班级CRUD |
| GET | `/api/college/clubs[/{id}]` | 本院社团列表/详情 |
| GET/POST | `/api/college/reimbursements/pending` | 待审报销 |
| POST | `/api/college/reimbursements/{id}/approve\|reject` | 审批报销 |
| GET/POST | `/api/college/limits[/{club_id}]` | 报销额度 |
| GET | `/api/college/students` | 本院学生列表 |
| POST | `/api/college/students/{id}/restrict\|lift-restriction` | 学生限制 |
| GET/POST | `/api/college/announcements` | 学院公告 |

#### 学生端 — 社团广场 (8个)

| 方法 | 端点 | 说明 |
|------|------|------|
| GET | `/api/clubs` | 社团浏览(搜索/筛分类/按级别) |
| GET | `/api/clubs/{id}` | 社团详情 |
| GET | `/api/clubs/{id}/members` | 社团成员列表 |
| GET | `/api/clubs/{id}/activities` | 社团公开活动 |
| POST | `/api/clubs/apply` | 申请创建新社团 |
| POST | `/api/clubs/{id}/join` | 申请加入社团 |
| POST | `/api/clubs/{id}/leave` | 退出社团 |

#### 学生端 — 活动 (5个)

| 方法 | 端点 | 说明 |
|------|------|------|
| GET | `/api/activities` | 活动列表 |
| GET | `/api/activities/{id}` | 活动详情 |
| POST | `/api/activities/{id}/register` | 报名活动 |
| POST | `/api/activities/{id}/cancel` | 取消报名 |
| POST | `/api/activities/{id}/signin` | 输入签到码签到 |

#### 学生端 — 个人中心 (5个)

| 方法 | 端点 | 说明 |
|------|------|------|
| GET | `/api/profile` | 查看个人信息 |
| PUT | `/api/profile` | 更新个人信息 |
| POST | `/api/profile/password` | 修改密码 |
| GET | `/api/my/clubs` | 我的社团 |
| GET | `/api/my/applications` | 我的建社申请记录 |

#### 社团管理端 — 基本信息 (3个)

| 方法 | 端点 | 说明 |
|------|------|------|
| GET | `/api/club/{id}/dashboard` | 社团仪表盘 |
| GET | `/api/club/{id}/profile` | 查看社团资料 |
| PUT | `/api/club/{id}/profile` | 更新社团资料 |

#### 社团管理端 — 成员管理 (6个)

| 方法 | 端点 | 说明 |
|------|------|------|
| GET | `/api/club/{id}/members` | 成员列表 |
| GET | `/api/club/{id}/join-requests` | 待审批入社申请 |
| POST | `/api/club/{id}/join-requests/{mid}/approve` | 通过入社 |
| POST | `/api/club/{id}/join-requests/{mid}/reject` | 拒绝入社 |
| POST | `/api/club/{id}/members/{mid}/remove` | 移除成员 |
| POST | `/api/club/{id}/members/{mid}/appoint` | 任命角色 |

#### 社团管理端 — 活动管理 (11个)

| 方法 | 端点 | 说明 |
|------|------|------|
| GET | `/api/club/{id}/activities` | 活动列表(含草稿) |
| POST | `/api/club/{id}/activities` | 创建活动(草稿) |
| PUT | `/api/club/{id}/activities/{aid}` | 修改活动 |
| POST | `/api/club/{id}/activities/{aid}/publish` | 发布(生成签到码) |
| POST | `/api/club/{id}/activities/{aid}/start` | 开始活动 |
| POST | `/api/club/{id}/activities/{aid}/finish` | 结束活动 |
| POST | `/api/club/{id}/activities/{aid}/cancel` | 取消活动(通知报名者) |
| POST | `/api/club/{id}/activities/{aid}/summary` | 提交活动总结 |
| DELETE | `/api/club/{id}/activities/{aid}` | 删除活动 |
| GET | `/api/club/{id}/activities/{aid}/signins` | 签到名单 |
| POST | `/api/club/{id}/activities/{aid}/manual-signin` | 手动补签 |

#### 社团管理端 — 财务 (3个)

| 方法 | 端点 | 说明 |
|------|------|------|
| GET | `/api/club/{id}/finance` | 财务流水 |
| GET | `/api/club/{id}/reimbursements` | 报销记录 |
| POST | `/api/club/{id}/reimbursements` | 提交报销(含发票上传) |

#### 社团管理端 — 换届选举 (8个)

| 方法 | 端点 | 说明 |
|------|------|------|
| GET | `/api/club/{id}/elections` | 选举列表 |
| POST | `/api/club/{id}/elections` | 发起选举 |
| GET | `/api/club/{id}/elections/{eid}` | 选举详情 |
| POST | `/api/club/{id}/elections/reelect` | 社长申请连任 |
| POST | `/api/club/{id}/elections/{eid}/signup` | 报名参选 |
| GET | `/api/club/{id}/elections/{eid}/candidates` | 候选人排名 |
| POST | `/api/club/{id}/elections/{eid}/vote` | 匿名投票 |
| GET | `/api/club/{id}/elections/{eid}/result` | 选举结果 |

#### 社团管理端 — 公告 (1个)

| 方法 | 端点 | 说明 |
|------|------|------|
| POST | `/api/club/{id}/announcements` | 发布社团公告 |

---

## 六、核心业务模块

### 6.1 活动状态机

```
                  publish           start           finish
  draft ──────────────────→ published ─────→ ongoing ─────→ completed
    │                            │              │
    └────────── cancel ──────────┴──────────────┘
                  │
                  ▼
              cancelled (终态)
```

- **draft**: 草稿，可修改/删除
- **published**: 已发布，学生可报名 (signup_deadline 截止前)
- **ongoing**: 进行中，不可报名，可签到
- **completed**: 已结束，可提交总结
- **cancelled**: 已取消，自动通知所有报名者

### 6.2 选举流程

```
发起选举 (signup阶段)
    │
    ├── 社长申请连任 (POST reelect)
    ├── 成员报名参选 (POST signup)
    │
    ▼
进入投票阶段
    │
    ├── 成员匿名投票 (POST vote)
    │
    ▼
选举结束 (completed)
    │
    ├── 触发器结算: 更新 members.role + clubs.president_id
    └── 通知当选者
```

### 6.3 报销审批流程

```
社长提交 (POST club/{id}/reimbursements)
    │  含发票图片 multipart 上传
    │  校验单次上限 + 周期累计上限
    ▼
学院管理员审批 (college_admin)
    ├── 通过 → 触发器自动写入 finance 表
    └── 驳回 → 通知社长
    │
    ▼
校级社团 → 学校管理员审批 (school_admin)
```

### 6.4 通知推送体系

| 通知类型 | 推送时机 | 接收范围 |
|---------|---------|---------|
| `announcement` | 发布公告 | 全校/全院/全社 |
| `club_apply` | 学生申请建社 | 全体学校管理员 |
| `club_apply_result` | 建社审批结果 | 申请人 |
| `join_result` | 入社审批结果 | 申请人 |
| `member_removed` | 被移出社团 | 被移除者 |
| `role_change` | 角色变更 | 当事人 |
| `activity_cancelled` | 活动取消 | 所有已报名者 |
| `election_start` | 选举开始 | 全体社员 |
| `election_result` | 选举结果 | 当选者 |
| `reimbursement_result` | 报销审批结果 | 社长 |
| `restriction_applied/lifted` | 限制施加/解除 | 当事人 |

---

## 七、安全设计

### 7.1 认证

- 密码使用 **SHA256** 哈希存储（64位十六进制）
- 登录成功后生成 **32位随机 token**，通过 HttpOnly Cookie 下发
- Session 保存在内存哈希池中（`auth.c`），无状态校验

### 7.2 授权

- 路由层：`need_login=1` 的路由先校验登录态
- 业务层：handler 内部调用 `api_require_school_admin` / `api_require_college_admin` / `club_require_manager`
- 数据行级：member/club 操作均校验归属关系

### 7.3 SQL 注入防护

- 所有用户输入通过 `db_escape()` 调用 `mysql_real_escape_string` 转义
- 数值参数使用 `%d` 格式化（类型安全）

### 7.4 文件上传安全

- 文件名随机化（时间戳+6位随机码）
- 扩展名白名单（仅保留字母数字）
- 目录穿越防护（`utils_ensure_upload_dir`）

---

## 八、部署说明

### 8.1 编译

```bash
# Windows (MinGW-w64)
cd server
mingw32-make          # Release
mingw32-make debug    # Debug (带符号)
mingw32-make rebuild  # 清理重编译

# Linux
cd server
make
```

### 8.2 配置

编辑 `db_config.ini`:
```ini
[database]
host = 127.0.0.1
port = 3306
user = scms_user
password = scms_pass_2026
database = scms_db
pool_size = 5
```

### 8.3 运行

```bash
./scms.exe        # 启动，监听 :8000
# 浏览器访问 http://127.0.0.1:8000
```

### 8.4 依赖

| 依赖 | 版本 | 说明 |
|------|------|------|
| MySQL Server | 8.0+ | 数据库 |
| MySQL C API | 8.0 (libmysql.lib/.dll) | C 客户端库 |
| MinGW-w64 | 13.1.0+ | Windows 编译工具链 (Linux 用 GCC) |
| Node.js | 18+ | 前端构建 (非运行时依赖) |
| Mongoose | 7.x (内嵌) | HTTP 服务器库 |

---

## 九、测试结果

### 9.1 测试概况

| 指标 | 数值 |
|------|------|
| 总测试端点 | 45 |
| 通过 | 43 |
| 预期失败 (环境依赖) | 2 |
| 通过率 | **95.6%** |

### 9.2 测试数据

- 用户：35 个（含 7 个管理员 + 28 个学生）
- 社团：8 个（4 校级 + 4 院级 + 3 个待审批）
- 活动：8 个（覆盖 draft/published/ongoing/completed 全状态）
- 成员关系、财务记录、报销、选举：均有测试数据

### 9.3 Bug 修复记录

| # | 文件 | 问题 | 状态 |
|---|------|------|------|
| 1 | `notification_center.c` | `receiver_id` → `user_id` (列名不匹配) | ✅ |
| 2 | `routes.c` | `/api/school/finance/clubs` 被通配符截获 | ✅ |
| 3 | `activity.c` | INSERT `enrolled` vs UPDATE `registered` 不匹配 | ✅ |
| 4 | `activity_mgmt.c` | 空 end_time/signup_deadline SQL 拼接错误 | ✅ |
| 5 | 7个 club_admin 文件 | `api_get_path_int` 索引偏移 (ctx,2→ctx,1) | ✅ |
| 6 | `school_dashboard.c` | 与 `statistics.c` 重复定义函数 | ✅ |
| 7 | `Makefile` | MinGW/PE 不支持 `__attribute__((weak))` | ✅ |
