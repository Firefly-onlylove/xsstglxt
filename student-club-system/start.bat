@echo off
chcp 65001 >nul 2>nul
setlocal enabledelayedexpansion

set "PD=%~dp0"
cd /d "%PD%" >nul 2>&1

set "PORT=8000"
set "NGROK=%USERPROFILE%\ngrok.exe"

:: ═══════════════════ 自动发现 MinGW ═══════════════════
set "MINGW="
for %%d in (
    "C:\mingw4\mingw64"
    "D:\mingw4\mingw64"
    "C:\msys64\mingw64"
    "D:\msys64\mingw64"
    "C:\mingw64"
    "D:\mingw64"
) do if exist "%%~d\bin\gcc.exe" if "!MINGW!"=="" set "MINGW=%%~d"
if "%MINGW%"=="" (
    echo [ERROR] 找不到 MinGW-w64，请安装后重试。
    pause & exit /b 1
)

:: ═══════════════════ 自动发现 MySQL ═══════════════════
set "MYSQL="
for %%d in (
    "C:\Program Files\MySQL\MySQL Server 8.0"
    "C:\Program Files\MySQL\MySQL Server 8.4"
    "C:\Program Files\MySQL\MySQL Server 9.0"
) do if exist "%%~d\bin\mysql.exe" if "!MYSQL!"=="" set "MYSQL=%%~d"
if "%MYSQL%"=="" (
    echo [ERROR] 找不到 MySQL，请安装 MySQL 8.0+ 或手动设置 MYSQL 路径。
    pause & exit /b 1
)

set "PATH=%MINGW%\bin;%MYSQL%\bin;%MYSQL%\lib;%PATH%"

:: ═══════════════════ 找到 make ═══════════════════
set "MAKE=mingw32-make"
if not exist "%MINGW%\bin\mingw32-make.exe" set "MAKE=make"

:: ═══════════════════ 读取数据库配置 ═══════════════════
set "DB_USER=root"
set "DB_PASS="
if exist "%PD%db_config.ini" (
    for /f "usebackq tokens=1* delims==" %%a in (`type "%PD%db_config.ini"`) do (
        set "_k=%%a" & set "_v=%%b"
        set "_k=!_k: =!"
        if "!_k!"=="user"     set "DB_USER=!_v: =!"
        if "!_k!"=="password" set "DB_PASS=!_v: =!"
    )
)
if "%DB_PASS%"=="" set "DB_PASS=200357"

:: ═══════════════════ 解析命令行参数 ═══════════════════
set "MODE=local"
set "ACTION="
if /i "%1"=="stop"     set "ACTION=stop"   & goto :run
if /i "%1"=="build"    set "ACTION=build"  & goto :run
if /i "%1"=="rebuild"  set "ACTION=rebuild"& goto :run
if /i "%1"=="frontend" set "ACTION=fe"     & goto :run
if /i "%1"=="all"      set "ACTION=all"    & goto :run
if /i "%1"=="public"   set "MODE=public"   & set "ACTION=start" & goto :run
if /i "%1"=="local"    set "ACTION=start"  & goto :run

:: 无参数 → 帮助
echo.
echo   SCMS - 学生社团管理系统
echo.
echo   用法: start.bat [命令]
echo.
echo   命令:
echo     local     本地模式 ^(默认^)
echo     public    ngrok 公网模式
echo     build    仅编译后端
echo     rebuild  重新编译后端
echo     frontend 编译前端
echo     all      全部重新编译 + 本地模式
echo     stop     停止服务器
echo.
echo   start.bat local  → 直接启动（本地）
echo   start.bat public → 直接启动（公网）
exit /b 0

:: ═══════════════════ 执行 ═══════════════════
:run
if "%ACTION%"=="stop"   call :do_stop & exit /b 0
if "%ACTION%"=="build"  call :do_build & exit /b 0
if "%ACTION%"=="rebuild" call :do_rebuild & exit /b 0
if "%ACTION%"=="fe"     call :do_frontend & exit /b 0
if "%ACTION%"=="all"    ( call :do_frontend & call :do_rebuild & goto :start )

:: ═══════════════════ MySQL 检测与启动 ═══════════════════
echo [*] 检测 MySQL 连接...
set "MYSQL_SVC="
for %%s in (MySQL80 MySQL84 MySQL90 MySQL8.0 MySQL8.4) do (
    sc query %%s >nul 2>&1
    if !errorlevel! equ 0 if "!MYSQL_SVC!"=="" set "MYSQL_SVC=%%s"
)
if not "%MYSQL_SVC%"=="" (
    sc query "%MYSQL_SVC%" | findstr "STATE" | findstr "4" >nul 2>&1
    if !errorlevel! neq 0 (
        echo     正在启动 MySQL 服务 '%MYSQL_SVC%'...
        net start "%MYSQL_SVC%" >nul 2>&1
    )
)

:try_mysql
mysql -u %DB_USER% -p%DB_PASS% --batch --no-beep -e "SELECT 1" >nul 2>&1
if not errorlevel 1 goto :mysql_ok

echo     [WARN] MySQL 连接失败 (user=%DB_USER%)
echo     请输入本机 MySQL root 密码，直接回车跳过：
set "INPUT_PASS="
for /f "delims=" %%p in ('powershell -Command "try{$s=Read-Host -AsSecureString;if($s.Length -eq 0){''}else{[Runtime.InteropServices.Marshal]::PtrToStringAuto([Runtime.InteropServices.Marshal]::SecureStringToBSTR($s))}}catch{''}"') do set "INPUT_PASS=%%p"
if "%INPUT_PASS%"=="" ( echo     已取消。 & pause & exit /b 1 )
set "DB_PASS=%INPUT_PASS%"
goto :try_mysql

:mysql_ok
echo     MySQL 连接成功

:: ═══════════════════ 确保配置文件存在 ═══════════════════
if not exist "%PD%db_config.ini" (
    echo [*] 生成 db_config.ini...
    (
        echo [database]
        echo host = 127.0.0.1
        echo port = 3306
        echo user = %DB_USER%
        echo password = %DB_PASS%
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
)

:: ═══════════════════ 确保 public 目录存在 ═══════════════════
if not exist "%PD%public\index.html" (
    echo [*] 未找到 public/，正在编译前端...
    call :do_frontend
)

:: ═══════════════════ 确保 scms.exe 存在 ═══════════════════
if not exist "%PD%scms.exe" (
    echo [*] 未找到 scms.exe，正在编译...
    call :do_rebuild
)

:: ═══════════════════ ngrok（公网模式） ═══════════════════
if /i not "%MODE%"=="public" goto :start
if not exist "%NGROK%" (
    echo [WARN] 未找到 ngrok ^(%NGROK%^)，回退到本地模式
    set "MODE=local"
    goto :start
)

:: 先杀掉旧 ngrok 进程
taskkill /F /IM ngrok.exe >nul 2>&1
echo [*] 启动 ngrok 隧道...
start "ngrok" "%NGROK%" http %PORT% --log=stdout >nul 2>&1

echo     等待隧道就绪...
set "NGROK_OK=0"
for /L %%i in (1,1,12) do (
    timeout /t 1 /nobreak >nul
    powershell -Command "try{$r=Invoke-RestMethod 'http://127.0.0.1:4040/api/tunnels' -TimeoutSec 2;if($r.tunnels.public_url){exit 0}}catch{}exit 1" >nul 2>&1
    if !errorlevel! equ 0 (
        for /f "delims=" %%u in ('powershell -Command "try{(Invoke-RestMethod 'http://127.0.0.1:4040/api/tunnels' -TimeoutSec 2).tunnels[0].public_url}catch{''}"') do set "PUBLIC_URL=%%u"
        echo     ngrok 就绪: !PUBLIC_URL!
        set "NGROK_OK=1"
        goto :start
    )
)
echo [WARN] ngrok 启动超时，仍在后台尝试...

:: ═══════════════════ 启动服务器 ═══════════════════
:start
echo.
echo ============================================================
echo   SCMS - 学生社团管理系统
echo   本地:  http://127.0.0.1:%PORT%
if "%NGROK_OK%"=="1" echo   公网:  %PUBLIC_URL%
echo   按 Ctrl+C 停止
echo ============================================================
echo.
scms.exe

:: 退出清理
echo.
echo 服务器已停止。
taskkill /F /IM ngrok.exe >nul 2>&1
exit /b 0

:: ═══════════════════ :do_stop ═══════════════════
:do_stop
echo 停止端口 %PORT% 上的服务...
for /f "tokens=5" %%a in ('netstat -ano ^| findstr ":%PORT%" ^| findstr "LISTENING" 2^>nul') do (
    taskkill /PID %%a /F >nul 2>&1
)
taskkill /F /IM ngrok.exe >nul 2>&1
echo 已停止。
exit /b 0

:: ═══════════════════ :do_build ═══════════════════
:do_build
echo [*] 编译后端...
cd /d "%PD%server"
%MAKE% -j4
if %errorlevel% neq 0 ( echo [ERROR] 编译失败 & cd /d "%PD%" & pause & exit /b 1 )
echo     编译完成
cd /d "%PD%"
exit /b 0

:: ═══════════════════ :do_rebuild ═══════════════════
:do_rebuild
echo [*] 重新编译后端...
cd /d "%PD%server"
%MAKE% clean >nul 2>&1
%MAKE% -j4
if %errorlevel% neq 0 ( echo [ERROR] 编译失败 & cd /d "%PD%" & pause & exit /b 1 )
echo     重新编译完成
cd /d "%PD%"
exit /b 0

:: ═══════════════════ :do_frontend ═══════════════════
:do_frontend
echo [*] 编译前端...
cd /d "%PD%client"
if not exist "node_modules\" call npm install
call npm run build
if %errorlevel% neq 0 ( echo [ERROR] 前端编译失败 & cd /d "%PD%" & pause & exit /b 1 )
echo     前端编译完成
cd /d "%PD%"
exit /b 0
