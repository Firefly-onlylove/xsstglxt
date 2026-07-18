## =============================================================
## Makefile — 学生社团管理系统 SCMS（C + Mongoose Web 架构）
## 产物：scms.exe（Windows）/ scms（Linux）
##
## 用法：
##   make            编译（Release）
##   make debug      编译（带调试符号）
##   make clean      清理产物
##   make run        编译并运行
##
## 依赖：
##   - MinGW-w64 (Windows) 或 gcc (Linux)
##   - MySQL C API（libmysqlclient / libmariadb 开发包）
##
## ⚠ 首次使用请按下方 "路径配置" 改成你机器上的实际路径。
## =============================================================

## ─────────────────────── 路径配置（按需修改） ───────────────────────

## MySQL C API 头文件目录（含 mysql.h）与库目录。
## Windows 常见位置（MySQL 8.0 安装自带 C API）：
##   MYSQL_INC = C:/Program Files/MySQL/MySQL Server 8.0/include
##   MYSQL_LIB = C:/Program Files/MySQL/MySQL Server 8.0/lib
## 或使用 MariaDB Connector/C（更轻量，推荐）：
##   MYSQL_INC = C:/Program Files/MariaDB/MariaDB Connector C 64-bit/include
##   MYSQL_LIB = C:/Program Files/MariaDB/MariaDB Connector C 64-bit/lib

ifeq ($(OS),Windows_NT)
    MYSQL_INC ?= C:/Program Files/MySQL/MySQL Server 8.0/include
    MYSQL_LIB ?= C:/Program Files/MySQL/MySQL Server 8.0/lib
    TARGET     = ../scms.exe
    ## Windows/MySQL 8.0 的客户端库名是 libmysql.lib（不是 libmysqlclient）。
    ## GNU ld 用 -l:libmysql.lib 直接按文件名链接这个 MSVC 导入库。
    MYSQL_LINK    = -l:libmysql.lib
    ## Windows 需要链接 socket 库（Mongoose 用）
    PLATFORM_LIBS = -lws2_32
    RM  = del /Q /F
    RMDIR = rmdir /S /Q
else
    MYSQL_INC ?= /usr/include/mysql
    MYSQL_LIB ?= /usr/lib/x86_64-linux-gnu
    TARGET     = ../scms
    ## Linux 一般是 libmysqlclient（或 MariaDB 的 libmariadb）
    MYSQL_LINK    = -lmysqlclient
    PLATFORM_LIBS = -lpthread
    RM  = rm -f
    RMDIR = rm -rf
endif

## ─────────────────────── 编译器与选项 ───────────────────────

CC      = gcc
CSTD    = -std=c11
WARN    = -Wall -Wextra -Wno-unused-parameter
OPT     = -O2
DEBUG   = -g -O0 -DDEBUG

## 头文件搜索路径：项目 include/ + Mongoose + MySQL
INCLUDES = -Iinclude -Isrc/mongoose -I"$(MYSQL_INC)"

## Mongoose 编译宏：启用 TLS 关闭（不需要 https），启用目录列举
MG_FLAGS = -DMG_ENABLE_DIRLIST=1

CFLAGS  = $(CSTD) $(WARN) $(OPT) $(INCLUDES) $(MG_FLAGS)

## 链接库：MySQL 客户端 + 平台库
LDFLAGS = -L"$(MYSQL_LIB)"
LDLIBS  = $(MYSQL_LINK) $(PLATFORM_LIBS)

## ─────────────────────── 源文件收集 ───────────────────────

## 入口
SRC_MAIN     = src/main.c

## Mongoose 库
SRC_MONGOOSE = src/mongoose/mongoose.c

## 公共模块（基础设施）
SRC_COMMON   = src/common/db.c \
               src/common/utils.c \
               src/common/auth.c \
               src/common/api.c \
               src/common/notification.c \
               src/common/routes.c

## 业务模块：自动收集三端所有 .c（含子目录 club_admin）。
## 注意：备用文件当前为全注释，编译出空对象文件不影响链接；
##       你解注释真实实现后，其强符号会覆盖 handlers_stub.c 的弱符号。
SRC_SCHOOL   = $(wildcard src/school/*.c)
SRC_COLLEGE  = $(wildcard src/college/*.c)
SRC_STUDENT  = $(wildcard src/student/*.c) $(wildcard src/student/club_admin/*.c)

SRCS = $(SRC_MAIN) $(SRC_MONGOOSE) $(SRC_COMMON) \
       $(SRC_SCHOOL) $(SRC_COLLEGE) $(SRC_STUDENT)

## 目标文件
OBJS = $(SRCS:.c=.o)

## ─────────────────────── 构建规则 ───────────────────────

.PHONY: all debug clean run rebuild

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) $(LDLIBS) -o $@
	@echo ""
	@echo "==================================================="
	@echo " 编译完成：$(TARGET)"
	@echo " 运行：./$(TARGET)   然后浏览器访问 http://127.0.0.1:8000"
	@echo "==================================================="

## Mongoose 单独用较宽松告警编译（第三方库，不追究其 warning）
src/mongoose/mongoose.o: src/mongoose/mongoose.c
	$(CC) $(CSTD) -O2 $(INCLUDES) $(MG_FLAGS) -w -c $< -o $@

## 通用编译规则
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

debug: CFLAGS := $(CSTD) $(WARN) $(DEBUG) $(INCLUDES) $(MG_FLAGS)
debug: clean $(TARGET)

rebuild: clean all

run: all
	cd .. && ./$(notdir $(TARGET))

clean:
	-@rm -f src/*.o src/common/*.o src/school/*.o src/college/*.o src/student/*.o src/student/club_admin/*.o src/mongoose/*.o $(TARGET) 2>/dev/null || true
	@echo "已清理。"
