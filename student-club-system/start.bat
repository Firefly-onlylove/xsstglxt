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

:: stop
if /i "%1"=="stop" (
    echo Stopping server on port %PORT%...
    for /f "tokens=5" %%a in ('netstat -ano ^| findstr ":%PORT%" ^| findstr "LISTENING" 2^>nul') do (
        echo   Killing scms PID %%a
        taskkill /PID %%a /F >nul 2>&1
    )
    taskkill /F /IM ngrok.exe >nul 2>&1
    echo Done.
    pause
    exit /b 0
)

echo ============================================================
echo   SCMS - Student Club Management System
echo ============================================================
echo.

:: 1. MySQL
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

:: 2. db_config.ini
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

:: 3. public/ auto-build
if not exist "%PD%\public\index.html" (
    echo     [*] public/ not found, building frontend...
    cd /d "%PD%\client"
    if not exist "node_modules\" call npm install
    call npm run build
    cd /d "%PD%"
)

:: 4. Build commands
if /i "%1"=="build"    call :build
if /i "%1"=="rebuild"  call :rebuild
if /i "%1"=="frontend" call :frontend
if /i "%1"=="all"      ( call :frontend & call :rebuild )

:: 5. ngrok public
set "NGROK_URL="
if /i "%1"=="public" (
    if exist "%NGROK%" (
        echo [*] Starting ngrok tunnel...
        start "ngrok" "%NGROK%" http %PORT% >nul 2>&1
        echo     Waiting for tunnel...
        for /L %%i in (1,1,15) do (
            timeout /t 1 /nobreak >/dev/null
            curl -s http://127.0.0.1:4040/api/tunnels 2>/dev/null | findstr "public_url" >nul 2>&1
            if !errorlevel! equ 0 goto :ngrok_ready
        )
        :ngrok_ready
        echo     ngrok tunnel ready
    ) else (
        echo     [WARN] ngrok not found
    )
)

:: 6. Verify scms.exe
if not exist "%PD%\scms.exe" (
    echo [ERROR] scms.exe not found, run: start.bat build
    pause & exit /b 1
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
