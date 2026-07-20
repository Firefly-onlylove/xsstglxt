# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 个人偏好

1. 所有输出内容（提示、过程、结果）一律使用中文
2. 每次代码修改完成后，自动提交并推送至 GitHub
3. 所有代码均需包含必要的注释，包括函数、结构体、枚举等
4. 在每次任务完成后检查任务列表的状态，及时更新任务状态
5. 所有代码均需包含必要的性能优化，包括数据库查询优化、内存管理等
6. 所有代码均需包含必要的代码质量检查，包括代码格式化、注释检查等

## 项目概览

SCMS（学生社团管理系统）— C11 HTTP 服务端 + Vue 3 SPA 单体仓库。

## 构建命令

```bash
# 后端（在 server/ 目录下）
cd server && make          # 发布构建 → ../scms.exe
cd server && make debug    # 调试构建（-g -O0 -DDEBUG）
cd server && make run      # 构建并启动
cd server && make clean    # 清理

# 前端（在 client/ 目录下）
cd client && npm run dev   # Vite 开发服务器（:5173，API 代理到 :8000）
cd client && npm run build # 生产构建 → ../public/
```

## 技术栈

- **后端：** C11 + GCC（MinGW-w64，路径 `C:\mingw4\mingw64\bin\`）+ Mongoose 嵌入式 HTTP 库 + MySQL 8.0 C API
- **前端：** Vue 3.4 + Vite 5.2 + Element Plus 2.7 + Vue Router 4.3（Hash 模式）
- **数据库：** MySQL，库名 `scms_db`，连接配置在 `db_config.ini`。数据库字符集 utf8mb4，但客户端连接使用 gbk — 查询前需 `SET NAMES utf8mb4`
- **系统 Python：** 2.7.3（Anaconda 32位）— 不支持 f-string，ASCII 编码问题需用 `.encode('utf-8')`

## 服务端架构关键点

### 入口与请求流

`server/src/main.c` — 启动顺序：读 `db_config.ini` → `db_init()` 连接池 → `auth_init()` 会话池 → Mongoose 监听 `0.0.0.0:8000`。事件处理：OPTIONS 返回 CORS 头 → `/api/*` 走 `dispatch_api()` → `/receipts/*` 提供静态文件 → 其余走 `public/`。

### 路由系统

`server/src/common/routes.c` — 约 118 条路由，覆盖四类角色端口：`sch_`（学校管理员）、`col_`（学院管理员）、`stu_`（学生）、`club_`（社团管理）。`*` 匹配单一路径段。`api_get_path_int(ctx, n)` 按 `api/` 后的段索引（从 0 开始）提取参数。

### API 辅助层（`include/api.h` + `src/common/api.c`）

- 参数读取：`api_get_param()`（先 query string 再 POST body）、`api_get_json_str/int/double()`（JSON body）、`api_get_path_int(idx)`（URL 路径段）、`api_get_form_str/double()`（multipart）
- 响应：`api_ok()` → `{"code":0,"msg":"ok"}`；`api_ok_data(json)` 包装已有 JSON；`api_error(code, msg)` 返回错误。所有响应 HTTP 状态码统一 200，由前端根据 `code` 字段判断
- 权限：`api_require_login()`、`api_require_role()`、`api_require_school/college_admin()`、`api_require_club_admin(ctx, club_id)`
- `api_send_result(MYSQL_RES*)` 自动将结果集转为 JSON 数组；`api_send_result_paged()` 额外添加分页字段
- 文件上传：`api_save_uploaded_file()` 保存到指定目录，生成随机文件名防覆盖

### 数据库层（`include/db.h` + `src/common/db.c`）

- 互斥锁保护的连接池（默认 5，最大 16，可配）
- `db_query(fmt, ...)` → `MYSQL_RES*`；`db_execute(fmt, ...)` → 受影响行数
- 便捷取值：`db_query_int()`、`db_query_double()`、`db_query_str()`
- `db_escape(str)` 用于用户输入的 SQL 转义，用后 `free()`
- 事务：`db_begin()` / `db_commit()` / `db_rollback()`
- `db_result_to_json_array()` 根据 MySQL 字段类型自动判断 JSON 值是否需要引号

### 认证（`src/common/auth.c`）

- SHA256 密码哈希（自行实现，无 OpenSSL 依赖）
- 32 位十六进制 token 通过 `scms_token` Cookie 下发
- 内存哈希表存储会话（最多 1024 槽位），每次请求重新加载用户信息确保实时性

### 模块启用机制

所有业务处理函数在 `handlers_stub.c` 中有默认空实现（返回"功能尚未实现"）。各模块通过 `#if 1` / `#if 0` 控制编译，强符号覆盖 stub。

### JSON 响应格式

所有 API 统一返回：`{"code": 0, "msg": "...", "data": {...}}`。`code === 0` 表示成功。

## 前端架构关键点

### API 层（`src/api/index.js`）

封装 `fetch()`：`api.get/post/put/delete/upload()`。所有请求带 `credentials: 'include'` 和 `ngrok-skip-browser-warning: 1` 头。响应数组自动包装为 `{list: array, total: count}`（`normalizeData()`）。

### 路由（`src/router/index.js`）

Hash 模式路由。四类角色各自独立子路由：`/school/*`（9页）、`/college/*`（6页）、`/student/*`（8页）、`/club-admin/*`（5页）。全局 `beforeEach` 守卫调用 `GET /api/me` 校验角色权限，无权限重定向到角色首页。

### 共享布局与组件

- `MainLayout.vue` — 顶部导航栏，菜单项按角色过滤
- `DataTable.vue` — 通用表格，支持插槽列和分页
- `FilterBar.vue` — 搜索/筛选栏，emit `search` 和 `reset` 事件
- `ModalForm.vue` — 通用弹窗表单，声明式字段配置
- `useClub.js` — 获取当前用户管理的社团 ID 的组合式函数

### 前端请求结果处理模式

```js
const res = await api.get("/api/xxx", params);
if (res.code === 0) {
  data.value = res.data.list || [];
  total.value = res.data.total || 0;
}
```

## 数据库

迁移脚本在 `database/` 目录下，按编号顺序执行。21 张表，5 个触发器（成员计数、活动报名计数、报销累计自动维护）。完整结构见 `database/export_full.sql`。

## Git 工作流

- 主分支：`1`
- 远程：`https://github.com/Firefly-onlylove/7.git`
- Git 操作需从仓库根目录执行：`git -C "C:/7.17sxxm" <command>`
