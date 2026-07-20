@echo off
chcp 65001 >nul 2>nul
setlocal enabledelayedexpansion

set "PD=%~dp0"
cd /d "%PD%" >nul 2>&1

set "PORT=8000"
set "NGROK=%USERPROFILE%\ngrok.exe"
set "MYSQL_USER=root"
set "MYSQL_PASS=200357"

:: ============================================================
:: 自动发现 MinGW
:: ============================================================
for %%d in (
    "C:\mingw4\mingw64"
    "D:\mingw4\mingw64"
    "C:\msys64\mingw64"
    "D:\msys64\mingw64"
    "C:\mingw64"
    "D:\mingw64"
) do if exist "%%~d\bin\gcc.exe" set "MINGW=%%~d" & goto :found_mingw
echo [ERROR] 找不到 MinGW-w64
pause & exit /b 1
:found_mingw

:: ============================================================
:: 自动发现 MySQL
:: ============================================================
for %%d in (
    "C:\Program Files\MySQL\MySQL Server 8.0"
    "C:\Program Files\MySQL\MySQL Server 8.4"
    "C:\Program Files\MySQL\MySQL Server 9.0"
) do if exist "%%~d\bin\mysql.exe" set "MYSQL=%%~d" & goto :found_mysql
echo [ERROR] 找不到 MySQL
pause & exit /b 1
:found_mysql

set "PATH=%MINGW%\bin;%MYSQL%\bin;%MYSQL%\lib;%PATH%"

:: ============================================================
:: find make
:: ============================================================
set "MAKE=mingw32-make"
if not exist "%MINGW%\bin\mingw32-make.exe" set "MAKE=make"
if not exist "%MINGW%\bin\%MAKE%.exe" set "MAKE=mingw32-make"

:: ============================================================
:: 读取 db_config.ini
:: ============================================================
if exist "%PD%db_config.ini" (
    for /f "usebackq tokens=1* delims==" %%a in (`type "%PD%db_config.ini"`) do (
        if "%%a"=="password" set "MYSQL_PASS=%%b"
        if "%%a"=="password " set "MYSQL_PASS=%%b"
        if "%%a"=="user" set "MYSQL_USER=%%b"
        if "%%a"=="user " set "MYSQL_USER=%%b"
    )
    for /f "tokens=*" %%x in ("%MYSQL_PASS%") do set "MYSQL_PASS=%%x"
    for /f "tokens=*" %%x in ("%MYSQL_USER%") do set "MYSQL_USER=%%x"
)

:: ============================================================
:: 命令行参数（跳过菜单直接执行）
:: ============================================================
if /i "%1"=="stop"     call :do_stop    & exit /b 0
if /i "%1"=="local"    set "MODE=local"    & goto :prepare
if /i "%1"=="public"   set "MODE=public"   & goto :prepare
if /i "%1"=="build"    call :do_build      & exit /b 0
if /i "%1"=="rebuild"  call :do_rebuild    & exit /b 0
if /i "%1"=="frontend" call :do_frontend   & exit /b 0
if /i "%1"=="all"      call :do_all        & exit /b 0
if not "%1"=="" (
    echo 未知命令: %1
    echo 可用: local public build rebuild frontend all stop
    pause & exit /b 1
)

:: ============================================================
:: 无参数 → 交互菜单
:: ============================================================
:menu
cls
echo.
echo ============================================================
echo   SCMS - 学生社团管理系统
echo ============================================================
echo.
echo   [1] 本地模式  — 仅局域网访问 (http://127.0.0.1:8000^)
echo   [2] 公网模式  — ngrok 外网访问
echo   [3] 编译后端
echo   [4] 编译前端
echo   [5] 全部重新编译
echo   [6] 停止服务器
echo   [0] 退出
echo.
set /p "CHOICE=请输入选项 (0-6): "

if "%CHOICE%"=="0" exit /b 0
if "%CHOICE%"=="1" set "MODE=local"    & goto :prepare
if "%CHOICE%"=="2" set "MODE=public"   & goto :prepare
if "%CHOICE%"=="3" call :do_build      & pause & goto :menu
if "%CHOICE%"=="4" call :do_frontend   & pause & goto :menu
if "%CHOICE%"=="5" call :do_all        & pause & goto :menu
if "%CHOICE%"=="6" call :do_stop       & pause & goto :menu
echo 无效选项，请重新输入。
ping -n 2 127.0.0.1 >nul
goto :menu

:: ============================================================
:: 准备工作（MySQL + 编译检查）
:: ============================================================
:prepare
echo.
echo ============================================================
echo   SCMS - 学生社团管理系统
echo ============================================================
echo.

echo [*] 正在检查 MySQL...
set "MYSQL_SVC="
for %%s in (MySQL80 MySQL84 MySQL90 MySQL8.0 MySQL8.4) do (
    sc query %%s >nul 2>&1
    if !errorlevel! equ 0 if "!MYSQL_SVC!"=="" set "MYSQL_SVC=%%s"
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

:try_mysql
mysql -u %MYSQL_USER% -p%MYSQL_PASS% --batch --no-beep -e "SELECT 1" >nul 2>&1
if not errorlevel 1 goto :mysql_ok

echo     [WARN] MySQL 连接失败 (user=%MYSQL_USER%)
echo     请输入本机 MySQL root 密码，直接回车跳过:
for /f "delims=" %%p in ('powershell -Command "try{$s=Read-Host -AsSecureString;if($s.Length -eq 0){''}else{[Runtime.InteropServices.Marshal]::PtrToStringAuto([Runtime.InteropServices.Marshal]::SecureStringToBSTR($s))}}catch{''}"') do set "MYSQL_PASS=%%p"
for /f "tokens=*" %%x in ("%MYSQL_PASS%") do set "MYSQL_PASS=%%x"
if "%MYSQL_PASS%"=="" ( echo     已取消。 & pause & exit /b 1 )
goto :try_mysql

:mysql_ok
echo     MySQL 连接成功

:: ==== 确保 db_config.ini 存在 ====
if not exist "%PD%db_config.ini" (
    echo     [*] 正在创建 db_config.ini...
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
    ) > "%PD%db_config.ini"
    echo     db_config.ini 创建成功
)

:: ==== 确保 public/ 存在 ====
if not exist "%PD%public\index.html" (
    echo     [*] 未找到 public/，正在编译前端...
    call :do_frontend
)

:: ==== ngrok (公网模式) ====
if not "%MODE%"=="public" goto :skip_ngrok
if not exist "%NGROK%" (
    echo     [WARN] 未找到 ngrok ^(%NGROK%^)，回退到本地模式
    goto :skip_ngrok
)
echo [*] 正在启动 ngrok 隧道...
taskkill /F /IM ngrok.exe >nul 2>&1
start "ngrok" "%NGROK%" http %PORT% --log=stdout >nul 2>&1
echo     等待隧道就绪（最多 15 秒）...
set "NGROK_READY=0"
for /L %%i in (1,1,15) do (
    powershell -Command "try{$r=Invoke-RestMethod -Uri 'http://127.0.0.1:4040/api/tunnels' -TimeoutSec 2;if($r.tunnels){exit 0}}catch{}exit 1" >nul 2>&1
    if !errorlevel! equ 0 set "NGROK_READY=1"
)
if "%NGROK_READY%"=="1" (echo     ngrok 隧道就绪) else (echo     [WARN] ngrok 可能仍在启动中...)
:skip_ngrok

:: ==== 确保 scms.exe 存在 ====
if not exist "%PD%scms.exe" (
    echo [ERROR] 找不到 scms.exe，正在编译...
    call :do_rebuild
)

:: ==== 启动 ====
echo [*] 正在启动服务器...
echo.
echo ============================================================
echo   本地访问:  http://127.0.0.1:8000
echo   按 Ctrl+C 停止服务器
echo   输入 [stop] 并回车停止服务器
echo ============================================================
echo.
start "" http://127.0.0.1:8000

:: 启动 scms.exe 并持续检查是否有 stop 命令输入
set "STOP_FLAG="
start "" /B scms.exe

:wait_loop
set /p "STOP_INPUT="
if /i "!STOP_INPUT!"=="stop" (
    echo 正在停止服务器...
    for /f "tokens=5" %%a in ('netstat -ano ^| findstr ":%PORT%" ^| findstr "LISTENING" 2^>nul') do (
        echo   正在终止 scms PID %%a
        taskkill /PID %%a /F >nul 2>&1
    )
    goto :after_stop
)
goto :wait_loop

:after_stop

echo.
echo 服务器已停止。
taskkill /F /IM ngrok.exe >nul 2>&1
pause
exit /b 0

:: ============================================================
:: 功能子程序
:: ============================================================
:do_stop
echo 正在停止端口 %PORT% 上的服务器...
for /f "tokens=5" %%a in ('netstat -ano ^| findstr ":%PORT%" ^| findstr "LISTENING" 2^>nul') do (
    echo   正在终止 scms PID %%a
    taskkill /PID %%a /F >nul 2>&1
)
taskkill /F /IM ngrok.exe >nul 2>&1
echo 已停止。
exit /b 0

:do_build
echo [*] 正在编译后端...
cd /d "%PD%server"
%MAKE% -j4
if %errorlevel% neq 0 ( echo [ERROR] 编译失败 & cd /d "%PD%" & exit /b 1 )
echo     编译完成
cd /d "%PD%"
exit /b 0

:do_rebuild
echo [*] 正在重新编译后端...
cd /d "%PD%server"
%MAKE% clean >nul 2>&1
%MAKE% -j4
if %errorlevel% neq 0 ( echo [ERROR] 编译失败 & cd /d "%PD%" & exit /b 1 )
echo     重新编译完成
cd /d "%PD%"
exit /b 0

:do_frontend
echo [*] 正在编译前端...
cd /d "%PD%client"
if not exist "node_modules\" call npm install
call npm run build
if %errorlevel% neq 0 ( echo [ERROR] 前端编译失败 & cd /d "%PD%" & exit /b 1 )
echo     前端编译完成
cd /d "%PD%"
exit /b 0

:do_all
call :do_frontend
call :do_rebuild
exit /b 0
