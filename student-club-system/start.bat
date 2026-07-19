@echo off
chcp 65001 >nul 2>nul
setlocal enabledelayedexpansion

set "PD=C:\77FA6~1.17\STUDEN~1"
set "MYSQL=C:\Program Files\MySQL\MySQL Server 8.0"
set "MINGW=C:\mingw4\mingw64"
set "PORT=8000"
set "NGROK=%USERPROFILE%\ngrok.exe"
set "NGROK_URL_FILE=%PD%\ngrok_url.txt"

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
        echo   Killing PID %%a
        taskkill /PID %%a /F >nul 2>&1
    )
    taskkill /F /FI "IMAGENAME eq ngrok.exe" >nul 2>&1
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
    if %errorlevel% neq 0 echo     [WARN] Failed to start MySQL
)
mysql -u root -p123456789 -e "SELECT 1" >nul 2>&1
if %errorlevel% neq 0 (
    echo     [ERROR] Database connection failed
    pause & exit /b 1
)
echo     MySQL ready

:: 1.5 Ensure db_config.ini exists
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

:: 2. Port check
echo [*] Checking port %PORT%...
netstat -ano | findstr ":%PORT%" | findstr "LISTENING" >nul 2>&1
if %errorlevel% equ 0 (
    echo     [WARN] Port %PORT% in use, run: start.bat stop
)

:: 3. Build
if /i "%1"=="build"    call :build & goto :run
if /i "%1"=="rebuild"  call :rebuild & goto :run
if /i "%1"=="frontend" call :frontend & goto :run
if /i "%1"=="all"      call :frontend & call :rebuild & goto :run

:: 4. Ensure frontend (public/) exists - if not, auto-build
if not exist "%PD%\public\index.html" (
    echo [*] public/ not found, auto-building frontend...
    call :frontend
)

goto :run

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
if not exist "node_modules\" (
    echo     Installing npm deps...
    call npm install
)
call npm run build
if %errorlevel% neq 0 ( echo [ERROR] Build failed & cd /d "%PD%" & pause & exit /b 1 )
echo     Frontend build done
cd /d "%PD%"
exit /b 0

:run
if not exist "%PD%\scms.exe" (
    echo [ERROR] scms.exe not found, run: start.bat build
    pause & exit /b 1
)

:: 4. ngrok
set "NGROK_PUBLIC="
if /i "%1"=="public" (
    if exist "%NGROK%" (
        echo [*] Starting ngrok tunnel...
        start "ngrok" "%NGROK%" http %PORT% > nul 2>&1
        echo     Waiting for ngrok to establish tunnel...

        :: Wait up to 10 seconds for ngrok API to respond
        set "NGROK_URL="
        for /L %%i in (1,1,20) do (
            timeout /t 1 /nobreak >nul
            curl -s http://127.0.0.1:4040/api/tunnels > "%NGROK_URL_FILE%" 2>nul
            findstr "public_url" "%NGROK_URL_FILE%" >nul 2>&1
            if !errorlevel! equ 0 (
                for /f "tokens=2 delims=:" %%u in ('findstr "public_url" "%NGROK_URL_FILE%"') do (
                    for /f "tokens=1 delims=," %%v in ("%%u") do (
                        set "NGROK_URL=%%~v"
                    )
                )
                goto :ngrok_done
            )
        )
        :ngrok_done
        if defined NGROK_URL (
            echo     Tunnel established
        ) else (
            echo     [WARN] Could not detect ngrok URL, check %NGROK_URL_FILE%
        )
    ) else (
        echo     [WARN] ngrok not found at %NGROK%
    )
)

echo [*] Starting server...
echo.
echo ============================================================
echo   Local:  http://127.0.0.1:8000
if defined NGROK_URL echo   Public: !NGROK_URL!
echo   Press Ctrl+C to stop
echo ============================================================
echo.

start "" http://127.0.0.1:8000
scms.exe

echo.
echo Server stopped.
taskkill /F /FI "IMAGENAME eq ngrok.exe" >nul 2>&1
pause
