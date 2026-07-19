@echo off
chcp 65001 >nul 2>nul
setlocal enabledelayedexpansion

set "PD=%~dp0"
cd /d "%PD%" >nul 2>&1

set "PORT=8000"
set "NGROK=%USERPROFILE%\ngrok.exe"
set "MYSQL_USER=root"
set "MYSQL_PASS=200357"

for %%d in (
    "C:\mingw4\mingw64"
    "C:\msys64\mingw64"
    "C:\mingw64"
    "D:\mingw64"
    "D:\msys64\mingw64"
) do if exist "%%~d\bin\gcc.exe" set "MINGW=%%~d" & goto :found_mingw
echo [ERROR] 找不到 MinGW，请安装 MinGW-w64 或手动设置 MINGW 路径。
pause & exit /b 1
:found_mingw

for %%d in (
    "C:\Program Files\MySQL\MySQL Server 8.0"
    "C:\Program Files\MySQL\MySQL Server 8.4"
) do if exist "%%~d\bin\mysql.exe" set "MYSQL=%%~d" & goto :found_mysql
echo [ERROR] 找不到 MySQL，请安装 MySQL 或手动设置 MYSQL 路径。
pause & exit /b 1
:found_mysql

set "PATH=%MINGW%\bin;%MYSQL%\bin;%MYSQL%\lib;%PATH%"

set "MAKE=mingw32-make"
if not exist "%MINGW%\bin\mingw32-make.exe" set "MAKE=make"
if not exist "%MINGW%\bin\%MAKE%.exe" set "MAKE=mingw32-make"

if /i "%1"=="stop" call :do_stop & exit /b 0
if /i "%1"=="build"    set "MODE=local" & set "DO_BUILD=1" & goto :prepare
if /i "%1"=="rebuild"  set "MODE=local" & set "DO_REBUILD=1" & goto :prepare
if /i "%1"=="frontend" set "MODE=local" & set "DO_FRONTEND=1" & goto :prepare
if /i "%1"=="all"      set "MODE=local" & set "DO_ALL=1" & goto :prepare
if /i "%1"=="public"   set "MODE=public" & goto :prepare
if /i "%1"=="local"    set "MODE=local" & goto :prepare

:menu
cls
echo.
echo ============================================================
echo   SCMS - 学生社团管理系统
echo ============================================================
echo.
echo   [1] 本地模式  - 仅局域网访问 (http://127.0.0.1:8000)
echo   [2] 公网模式  - ngrok 外网访问
echo   [3] 编译后端 + 公网模式
echo   [4] 编译前端 + 公网模式
echo   [5] 全部重新编译 + 公网模式
echo   [6] 停止服务器
echo   [0] 退出
echo.
set /p "CHOICE=请输入选项 (0-6): "

if "%CHOICE%"=="0" exit /b 0
if "%CHOICE%"=="1" set "MODE=local" & goto :prepare
if "%CHOICE%"=="2" set "MODE=public" & goto :prepare
if "%CHOICE%"=="3" set "MODE=public" & set "DO_BUILD=1" & goto :prepare
if "%CHOICE%"=="4" set "MODE=public" & set "DO_FRONTEND=1" & goto :prepare
if "%CHOICE%"=="5" set "MODE=public" & set "DO_ALL=1" & goto :prepare
if "%CHOICE%"=="6" call :do_stop & goto :menu
echo 无效选项，请重新输入。
ping -n 2 127.0.0.1 >nul
goto :menu

:do_stop
echo 正在停止端口 %PORT% 上的服务器...
for /f "tokens=5" %%a in ('netstat -ano ^| findstr ":%PORT%" ^| findstr "LISTENING" 2^>nul') do (
    echo   正在终止 scms PID %%a
    taskkill /PID %%a /F >nul 2>&1
)
taskkill /F /IM ngrok.exe >nul 2>&1
echo 已停止。
pause
exit /b 0

:prepare
echo.
echo ============================================================
echo   SCMS - 学生社团管理系统
echo ============================================================
echo.

if exist "%PD%\db_config.ini" (
    for /f "usebackq tokens=1* delims==" %%a in (`type "%PD%\db_config.ini"`) do (
        if "%%a"=="password" set "MYSQL_PASS=%%b"
        if "%%a"=="password " set "MYSQL_PASS=%%b"
    )
    for /f "usebackq tokens=1* delims==" %%a in (`type "%PD%\db_config.ini"`) do (
        if "%%a"=="user" set "MYSQL_USER=%%b"
        if "%%a"=="user " set "MYSQL_USER=%%b"
    )
)

for /f "tokens=*" %%x in ("%MYSQL_PASS%") do set "MYSQL_PASS=%%x"
for /f "tokens=*" %%x in ("%MYSQL_USER%") do set "MYSQL_USER=%%x"

echo [*] 正在检查 MySQL...
set "MYSQL_SVC="
for %%s in (MySQL80 MySQL84 MySQL MySQL8.0) do (
    sc query %%s >nul 2>&1
    if !errorlevel! equ 0 set "MYSQL_SVC=%%s"
)
if "%MYSQL_SVC%"=="" (
    echo     [WARN] 未找到 MySQL 服务，尝试直接连接
) else (
    sc query "%MYSQL_SVC%" | findstr "STATE" | findstr "4" >nul 2>&1
    if !errorlevel! neq 0 (
        echo     正在启动 MySQL 服务 '%MYSQL_SVC%'...
        net start "%MYSQL_SVC%" >nul 2>&1
    )
)

mysql -u %MYSQL_USER% -p%MYSQL_PASS% --batch --no-beep -e "SELECT 1" >nul 2>&1
if errorlevel 1 (
    echo     [ERROR] MySQL 连接失败 user=%MYSQL_USER% password=%MYSQL_PASS%
    echo     请修改本文件顶部的 MYSQL_PASS 或检查 db_config.ini
    pause & exit /b 1
)
echo     MySQL 连接成功

if not exist "%PD%\db_config.ini" (
    echo     正在创建 db_config.ini...
    (
        echo [database]
        echo host = 127.0.0.1
        echo port = 3306
        echo user = %MYSQL_USER%
        echo password = %MYSQL_PASS%
        echo database = scms_db
        echo pool_size = 5
        echo.
        echo [server]
        echo receipt_dir = storage/receipts
        echo export_dir  = storage/exports
        echo backup_dir  = storage/backups
        echo.
        echo [app]
        echo auto_backup_hour = 3
        echo session_timeout = 0
    ) > "%PD%\db_config.ini"
    echo     db_config.ini 创建成功
)

if not exist "%PD%\public\index.html" (
    echo     [*] 未找到 public/ 目录，正在编译前端...
    cd /d "%PD%\client"
    if not exist "node_modules\" call npm install
    call npm run build
    cd /d "%PD%"
)

if "%DO_BUILD%"=="1"    call :build
if "%DO_REBUILD%"=="1"  call :rebuild
if "%DO_FRONTEND%"=="1" call :frontend
if "%DO_ALL%"=="1"      ( call :frontend & call :rebuild )

if not "%MODE%"=="public" goto :skip_ngrok
if not exist "%NGROK%" (
    echo     [WARN] 未找到 ngrok (%NGROK%)，回退到本地模式
    goto :skip_ngrok
)
echo [*] 正在启动 ngrok 隧道...
start "ngrok" "%NGROK%" http %PORT% --log=stdout >nul 2>&1
echo     等待隧道就绪（最多 15 秒）...
set "NGROK_READY=0"
for /L %%i in (1,1,15) do (
    powershell -Command "try { $r=Invoke-RestMethod -Uri 'http://127.0.0.1:4040/api/tunnels' -TimeoutSec 2; if ($r.tunnels) { exit 0 } } catch {} exit 1" >nul 2>&1
    if !errorlevel! equ 0 set "NGROK_READY=1"
)
if "%NGROK_READY%"=="1" (echo     ngrok 隧道就绪) else (echo     [WARN] ngrok 可能仍在启动中...)
:skip_ngrok

if not exist "%PD%\scms.exe" (
    echo [ERROR] 找不到 scms.exe，正在编译...
    call :rebuild
)

echo [*] 正在启动服务器...
echo.
echo ============================================================
echo   本地访问:  http://127.0.0.1:8000
echo   按 Ctrl+C 停止服务器
echo ============================================================
echo.

start "" http://127.0.0.1:8000
scms.exe

echo.
echo 服务器已停止。
taskkill /F /IM ngrok.exe >nul 2>&1
pause
exit /b 0

:build
echo [*] 正在编译后端...
cd /d "%PD%\server"
%MAKE% -j4
if %errorlevel% neq 0 ( echo [ERROR] 编译失败 & cd /d "%PD%" & pause & exit /b 1 )
echo     编译完成
cd /d "%PD%"
exit /b 0

:rebuild
echo [*] 正在重新编译后端...
cd /d "%PD%\server"
%MAKE% clean >nul 2>&1
%MAKE% -j4
if %errorlevel% neq 0 ( echo [ERROR] 编译失败 & cd /d "%PD%" & pause & exit /b 1 )
echo     重新编译完成
cd /d "%PD%"
exit /b 0

:frontend
echo [*] 正在编译前端...
cd /d "%PD%\client"
if not exist "node_modules\" call npm install
call npm run build
if %errorlevel% neq 0 ( echo [ERROR] 前端编译失败 & cd /d "%PD%" & pause & exit /b 1 )
echo     前端编译完成
cd /d "%PD%"
exit /b 0