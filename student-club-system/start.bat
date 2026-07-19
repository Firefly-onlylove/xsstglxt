@echo off
chcp 65001 >nul 2>nul
setlocal enabledelayedexpansion

set "PD=%~dp0"
cd /d "%PD%" >nul 2>&1

set "PORT=8000"
set "NGROK=%USERPROFILE%\ngrok.exe"
set "MYSQL_USER=root"
set "MYSQL_PASS=123456789"

for %%d in (
    "C:\mingw4\mingw64"
    "C:\msys64\mingw64"
    "C:\mingw64"
) do if exist "%%~d\bin\gcc.exe" set "MINGW=%%~d" & goto :found_mingw
echo [ERROR] MinGW not found. Please install MinGW-w64 or set MINGW path.
pause & exit /b 1
:found_mingw

for %%d in (
    "C:\Program Files\MySQL\MySQL Server 8.0"
    "C:\Program Files\MySQL\MySQL Server 8.4"
) do if exist "%%~d\bin\mysql.exe" set "MYSQL=%%~d" & goto :found_mysql
echo [ERROR] MySQL not found. Please install MySQL or set MYSQL path.
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
echo   SCMS - Student Club Management System
echo ============================================================
echo.
echo   [1] Local      - LAN only (http://127.0.0.1:8000)
echo   [2] Public     - ngrok public URL
echo   [3] Build + Public
echo   [4] Rebuild Frontend + Public
echo   [5] Rebuild All + Public
echo   [6] Stop Server
echo   [0] Exit
echo.
set /p "CHOICE=Enter option (0-6): "

if "%CHOICE%"=="0" exit /b 0
if "%CHOICE%"=="1" set "MODE=local" & goto :prepare
if "%CHOICE%"=="2" set "MODE=public" & goto :prepare
if "%CHOICE%"=="3" set "MODE=public" & set "DO_BUILD=1" & goto :prepare
if "%CHOICE%"=="4" set "MODE=public" & set "DO_FRONTEND=1" & goto :prepare
if "%CHOICE%"=="5" set "MODE=public" & set "DO_ALL=1" & goto :prepare
if "%CHOICE%"=="6" call :do_stop & goto :menu
echo Invalid choice.
ping -n 2 127.0.0.1 >nul
goto :menu

:do_stop
echo Stopping server on port %PORT%...
for /f "tokens=5" %%a in ('netstat -ano ^| findstr ":%PORT%" ^| findstr "LISTENING" 2^>nul') do (
    echo   Killing scms PID %%a
    taskkill /PID %%a /F >nul 2>&1
)
taskkill /F /IM ngrok.exe >nul 2>&1
echo Done.
pause
exit /b 0

:prepare
echo.
echo ============================================================
echo   SCMS - Student Club Management System
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

echo [*] Checking MySQL...
set "MYSQL_SVC="
for %%s in (MySQL80 MySQL84 MySQL MySQL8.0) do (
    sc query %%s >nul 2>&1
    if !errorlevel! equ 0 set "MYSQL_SVC=%%s"
)
if "%MYSQL_SVC%"=="" (
    echo     [WARN] MySQL service not found, trying to start anyway
) else (
    sc query "%MYSQL_SVC%" | findstr "STATE" | findstr "4" >nul 2>&1
    if !errorlevel! neq 0 (
        echo     Starting MySQL service '%MYSQL_SVC%'...
        net start "%MYSQL_SVC%" >nul 2>&1
    )
)

mysql -u %MYSQL_USER% -p%MYSQL_PASS% --batch --no-beep -e "SELECT 1" >nul 2>&1
if %errorlevel% neq 0 (
    echo     [ERROR] MySQL connection failed (user=%MYSQL_USER% password=%MYSQL_PASS%)
    echo     Edit MYSQL_PASS at top of this file or check db_config.ini
    pause & exit /b 1
)
echo     MySQL ready

if not exist "%PD%\db_config.ini" (
    echo     Creating db_config.ini...
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
    echo     db_config.ini created
)

if not exist "%PD%\public\index.html" (
    echo     [*] public/ not found, building frontend...
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
    echo     [WARN] ngrok not found at %NGROK%, LAN mode only
    goto :skip_ngrok
)
echo [*] Starting ngrok tunnel...
start "ngrok" "%NGROK%" http %PORT% --log=stdout >nul 2>&1
echo     Waiting for tunnel (up to 15s)...
set "NGROK_READY=0"
for /L %%i in (1,1,15) do (
    powershell -Command "try { $r=Invoke-RestMethod -Uri 'http://127.0.0.1:4040/api/tunnels' -TimeoutSec 2; if ($r.tunnels) { exit 0 } } catch {} exit 1" >nul 2>&1
    if !errorlevel! equ 0 set "NGROK_READY=1"
)
if "%NGROK_READY%"=="1" (echo     ngrok tunnel ready) else (echo     [WARN] ngrok may still be starting...)
:skip_ngrok

if not exist "%PD%\scms.exe" (
    echo [ERROR] scms.exe not found, compiling...
    call :rebuild
)

echo [*] Starting server...
echo.
echo ============================================================
echo   Local:  http://127.0.0.1:8000
echo   Press Ctrl+C to stop
echo ============================================================
echo.

start "" http://127.0.0.1:8000
scms.exe

echo.
echo Server stopped.
taskkill /F /IM ngrok.exe >nul 2>&1
pause
exit /b 0

:build
echo [*] Building backend...
cd /d "%PD%\server"
%MAKE% -j4
if %errorlevel% neq 0 ( echo [ERROR] Build failed & cd /d "%PD%" & pause & exit /b 1 )
echo     Build done
cd /d "%PD%"
exit /b 0

:rebuild
echo [*] Rebuilding backend...
cd /d "%PD%\server"
%MAKE% clean >nul 2>&1
%MAKE% -j4
if %errorlevel% neq 0 ( echo [ERROR] Build failed & cd /d "%PD%" & pause & exit /b 1 )
echo     Rebuild done
cd /d "%PD%"
exit /b 0

:frontend
echo [*] Building frontend...
cd /d "%PD%\client"
if not exist "node_modules\" call npm install
call npm run build
if %errorlevel% neq 0 ( echo [ERROR] Frontend build failed & cd /d "%PD%" & pause & exit /b 1 )
echo     Frontend build done
cd /d "%PD%"
exit /b 0
