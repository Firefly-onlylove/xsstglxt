# 学生社团管理系统 — AI负责与自行完成 分工文档

---

## 快速导航

| 部分 | 谁做 | 状态 |
|------|------|------|
| 一、数据库脚本 | AI | ⬜ 待生成 |
| 二、头文件（include/） | AI | ⬜ 待生成 |
| 三、公共模块（src/common/） | AI | ⬜ 待生成 |
| 四、程序入口（src/main.c） | AI | ⬜ 待生成 |
| 五、构建文件（Makefile + db_config.ini） | AI | ⬜ 待生成 |
| 六、学校端（src/school/） | 你 | 🔲 已建空目录 |
| 七、学院端（src/college/） | 你 | 🔲 已建空目录 |
| 八、学生端+社团管理（src/student/） | 你 | 🔲 已建空目录 |
| 九、项目文档（docs/） | 你 | 🔲 已建空目录 |

---

## 一、AI 负责部分（基础设施，你拿来直接用）

这些文件涉及 **MySQL C API / libmysqlclient 编译链接 / SHA256 哈希 / Makefile 语法** 等未学知识，全部由 AI 生成完整代码。

### 文件清单

| 序号 | 文件路径 | 说明 | 你需不需要改 |
|------|----------|------|:---:|
| 1 | `sql/01_create_database.sql` | 建库脚本 | 否 |
| 2 | `sql/02_create_tables.sql` | 建21张表 | 否 |
| 3 | `sql/03_triggers.sql` | 5个触发器 | 否 |
| 4 | `sql/04_init_data.sql` | 初始数据（6学院+账号+8社团） | 否 |
| 5 | `include/common.h` | 返回码、宏定义 | 否 |
| 6 | `include/db.h` | 数据库接口声明 | 否 |
| 7 | `include/models.h` | 全部结构体定义 | 否 |
| 8 | `include/utils.h` | 工具函数声明（含SHA256） | 否 |
| 9 | `include/ui.h` | 界面函数声明 | 否 |
| 10 | `src/common/db.c` | 数据库连接池、CRUD封装 | 否 |
| 11 | `src/common/auth.c` | 登录认证、权限检查 | 否 |
| 12 | `src/common/utils.c` | SHA256、校验、字符串/日期处理 | 否 |
| 13 | `src/common/ui.c` | 菜单渲染、表格、分页 | 否 |
| 14 | `src/common/notification.c` | 消息通知服务 | 否 |
| 15 | `src/main.c` | 程序入口、登录分流 | 否 |
| 16 | `Makefile` | 编译脚本 | 改路径 |
| 17 | `db_config.ini` | 数据库连接配置 | **改IP/密码** |

---

## 二、你负责部分（业务模块）

这些模块都遵循同一模式：**接收输入 → 拼SQL字符串 → 调db_query/db_execute → 处理结果 → 显示**。

用到的知识全是已学过的：结构体、字符串、循环、指针。

### 学校端（10个文件）

| 文件 | 功能概述 | 预估 |
|------|----------|:---:|
| `src/school/school_main.c` | 主菜单+路由 | 0.5h |
| `src/school/club_approval.c` | 社团审批（通过/驳回/列表/记录查询） | 1h |
| `src/school/club_mgmt.c` | 社团管理（查看详情/级别设定/解散） | 1h |
| `src/school/college_mgmt.c` | 学院管理（增删改停用） | 0.5h |
| `src/school/major_mgmt.c` | 专业管理（增删改） | 0.5h |
| `src/school/user_mgmt.c` | 用户管理（列表/创建学院管理员/禁用/重置密码/限制） | 1.5h |
| `src/school/finance_oversight.c` | 全校财务监督+校级报销审批 | 1h |
| `src/school/statistics.c` | 全校数据统计与导出CSV | 1h |
| `src/school/announcement.c` | 全校公告发布与管理 | 0.5h |
| `src/school/backup.c` | 数据库备份与恢复 | 0.5h |
| `src/school/system_config.c` | 系统配置（学期等） | 0.5h |
| **小计** | | **8.5h** |

### 学院端（7个文件）

| 文件 | 功能概述 | 预估 |
|------|----------|:---:|
| `src/college/college_main.c` | 主菜单+路由 | 0.5h |
| `src/college/class_mgmt.c` | 班级管理（增删改） | 0.5h |
| `src/college/club_oversight.c` | 本院社团监督 | 0.5h |
| `src/college/finance_approval.c` | 院级报销审批+额度设定 | 1h |
| `src/college/member_mgmt.c` | 本院学生管理（限制/解除） | 1h |
| `src/college/statistics.c` | 本院数据统计与导出 | 1h |
| `src/college/announcement.c` | 本院公告发布 | 0.5h |
| **小计** | | **5h** |

### 学生端（10个文件）

| 文件 | 功能概述 | 预估 |
|------|----------|:---:|
| `src/student/student_main.c` | 主菜单+按角色显示不同菜单 | 1h |
| `src/student/register.c` | 用户注册（填写+校验+入库） | 1h |
| `src/student/club_browse.c` | 社团浏览/搜索/详情 | 1h |
| `src/student/club_apply.c` | 申请创建社团 | 0.5h |
| `src/student/club_join.c` | 加入/退出社团 | 1h |
| `src/student/activity_view.c` | 活动查看/搜索/报名/取消 | 1h |
| `src/student/activity_signin.c` | 活动签到（输入签到码） | 0.5h |
| `src/student/personal_center.c` | 个人中心（查看/修改信息/修改密码） | 1h |
| `src/student/my_clubs.c` | 我的社团列表 | 0.5h |
| `src/student/announcement_view.c` | 查看公告/通知列表 | 0.5h |
| **小计** | | **8h** |

### 社团管理子模块（7个文件）

| 文件 | 功能概述 | 预估 |
|------|----------|:---:|
| `src/student/club_admin/club_profile.c` | 社团信息查看与修改 | 0.5h |
| `src/student/club_admin/member_mgmt.c` | 成员管理（审批入社/移除/角色任命） | 1.5h |
| `src/student/club_admin/activity_mgmt.c` | 活动管理（发布/修改/取消/签到/总结） | 1.5h |
| `src/student/club_admin/finance_mgmt.c` | 报销申请与记录查询 | 1h |
| `src/student/club_admin/election_mgmt.c` | 发起换届投票+申请连任 | 1.5h |
| `src/student/club_admin/election_vote.c` | 投票（匿名） | 0.5h |
| `src/student/club_admin/announcement.c` | 社团通知发布 | 0.5h |
| **小计** | | **7h** |

---

## 三、5天工作计划

```
第1天 ─ 环境搭建
  ├─ 上午：安装MySQL → 执行SQL脚本 → 改db_config.ini → make编译 → 跑通登录
  └─ 下午：学生端 register.c + club_browse.c + club_join.c

第2天 ─ 学生端
  ├─ 上午：activity_view.c + activity_signin.c + personal_center.c + my_clubs.c
  └─ 下午：student_main.c + club_apply.c + announcement_view.c

第3天 ─ 社团管理
  ├─ 上午：club_profile.c + member_mgmt.c + activity_mgmt.c
  └─ 下午：finance_mgmt.c + election_mgmt.c + election_vote.c + announcement.c

第4天 ─ 学校端+学院端
  ├─ 上午：学校端 club_approval + club_mgmt + college_mgmt + major_mgmt
  └─ 下午：学校端 user_mgmt + finance_oversight + 学院端 college_main + class_mgmt

第5天 ─ 收尾+联调
  ├─ 上午：学校端 statistics + announcement + backup + system_config
  │        学院端 club_oversight + finance_approval + member_mgmt + statistics + announcement
  └─ 下午：全程联调 + Bug修复 + 各端school_main/college_main组装
```

---

## 四、当前项目目录结构

```
student-club-system/
│
├── AI负责与自行完成_分工文档.md   ← 你正在看
│
├── sql/                             ← 目录已建，文件待AI生成
│   ├── 01_create_database.sql
│   ├── 02_create_tables.sql
│   ├── 03_triggers.sql
│   └── 04_init_data.sql
│
├── include/                         ← 目录已建，文件待AI生成
│   ├── common.h
│   ├── db.h
│   ├── models.h
│   ├── utils.h
│   └── ui.h
│
├── src/
│   ├── main.c                       ← 待AI生成
│   ├── common/                      ← 目录已建，文件待AI生成
│   │   ├── db.c
│   │   ├── auth.c
│   │   ├── utils.c
│   │   ├── ui.c
│   │   └── notification.c
│   │
│   ├── school/                      ← 空目录，你写（10个.c）
│   ├── college/                     ← 空目录，你写（7个.c）
│   └── student/                     ← 空目录，你写（10个.c）
│       └── club_admin/              ← 空目录，你写（7个.c）
│
├── receipts/                        ← 空目录，存放发票图片
├── exports/                         ← 空目录，存放导出文件
├── backups/                         ← 空目录，存放数据库备份
├── docs/                            ← 空目录，你写文档
│
├── Makefile                         ← 待AI生成
└── db_config.ini                    ← 待AI生成
```

---

## 五、备用代码说明

如果某个模块你写不出来，告诉我模块名称，我会在对应目录生成 `xxx（备用）.c` 文件：

| 备用文件的位置 | 举例 |
|------|------|
| `src/school/club_approval（备用）.c` | 学校端-社团审批 |
| `src/school/user_mgmt（备用）.c` | 学校端-用户管理 |
| `src/college/finance_approval（备用）.c` | 学院端-报销审批 |
| `src/student/register（备用）.c` | 学生端-注册 |
| ... | 所有34个模块都有备用 |

**需要备用文件时告诉我具体模块名即可，不要一次全要。**

---

## 六、业务代码编写模板（参考）

所有模块几乎都是这个套路，对照写就行：

```c
#include "../include/common.h"
#include "../include/db.h"
#include "../include/models.h"
#include "../include/ui.h"
#include "../include/utils.h"

void your_module_function(User *current_user) {
    // 1. 拼 SQL 字符串
    char sql[1024];
    snprintf(sql, sizeof(sql),
        "SELECT col1, col2, col3 FROM table_name WHERE condition='%s'",
        some_value);

    // 2. 执行查询
    MYSQL_RES *result = db_query("%s", sql);
    if (!result) {
        printf("查询失败: %s\n", db_error());
        return;
    }

    // 3. 遍历结果
    MYSQL_ROW row;
    printf("%-10s %-20s %-10s\n", "列1", "列2", "列3");
    while ((row = mysql_fetch_row(result)) != NULL) {
        printf("%-10s %-20s %-10s\n", row[0], row[1], row[2]);
    }

    // 4. 释放
    mysql_free_result(result);
}
```

**注意三个关键点：**
- 用户输入的字符串先用 `db_escape(str)` 转义再拼入 SQL
- `db_query` 后必须 `mysql_free_result`
- 密码永远调用 `utils_sha256(password, hash)` 后再存储
