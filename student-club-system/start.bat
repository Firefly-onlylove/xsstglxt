@echo off
chcp 65001 >nul 2>nul
setlocal enabledelayedexpansion

set "PD=C:\77FA6~1.17\STUDEN~1"
set "MYSQL=C:\Program Files\MySQL\MySQL Server 8.0"
set "MINGW=C:\mingw4\mingw64"
set "PORT=8000"
set "NGROK=%USERPROFILE%\ngrok.exe"

cd /d "%PD%" >nul 2>&1
if %errorlevel% neq 0 (
    echo [ERROR] Cannot access project
    pause
    exit /b 1
)

set "PATH=%MINGW%\bin;%MYSQL%\bin;%MYSQL%\lib;%PD%;%PATH%"

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

echo [*] Checking MySQL...
sc query MySQL80 | findstr "STATE" | findstr "4" >nul 2>&1
if %errorlevel% neq 0 (
    echo     Starting MySQL...
    net start MySQL80 >nul 2>&1
)
mysql -u root -p123456789 -e "SELECT 1" >nul 2>&1
if %errorlevel% neq 0 (
    echo     [ERROR] Database connection failed
    pause & exit /b 1
)
echo     MySQL ready

if not exist "%PD%\db_config.ini" (
    echo     Creating db_config.ini...
    (
        echo [database]
        echo host = 127.0.0.1
        echo port = 3306
        echo user = root
        echo password = 123456789
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
    echo     [WARN] ngrok not found, LAN mode only
    goto :skip_ngrok
)
echo [*] Starting ngrok tunnel...
start "ngrok" "%NGROK%" http %PORT% >nul 2>&1
echo     Waiting for tunnel (up to 15s)...
for /L %%i in (1,1,15) do (
    ping -n 2 127.0.0.1 >nul
    curl -s http://127.0.0.1:4040/api/tunnels 2>nul | findstr "public_url" >nul 2>&1
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
mingw32-make -j4
if %errorlevel% neq 0 ( echo [ERROR] Build failed & cd /d "%PD%" & pause & exit /b 1 )
echo     Build done
cd /d "%PD%"
exit /b 0

:rebuild
echo [*] Rebuilding backend...
cd /d "%PD%\server"
mingw32-make clean >nul 2>&1
mingw32-make -j4
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
