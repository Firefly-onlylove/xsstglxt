# SCMS Launcher
# Usage: .\start.ps1 [command]
#   (无参数)  → 本地模式
#   local     - 本地模式
#   public    - ngrok 公网模式
#   build     - 仅编译后端
#   rebuild   - 重新编译后端
#   frontend  - 编译前端
#   all       - 全部重编译 + 本地
#   stop      - 停止服务器
param([string]$Command = 'local')

$ErrorActionPreference = 'Stop'
$projectDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$port       = 8000

# ═══════════════════ 自动发现工具链 ═══════════════════
function Find-Tool {
    param([string[]]$Paths, [string]$CheckFile)
    foreach ($p in $Paths) {
        if (Test-Path "$p\$CheckFile") { return $p }
    }
    throw "找不到所需工具: $CheckFile，请在以下位置搜索：$($Paths -join ', ')"
}

try { $mingwDir = Find-Tool -Paths @('C:\mingw4\mingw64','D:\mingw4\mingw64','C:\msys64\mingw64','D:\msys64\mingw64','C:\mingw64','D:\mingw64') -CheckFile 'bin\gcc.exe' } catch { Write-Host "[ERROR] $_" -ForegroundColor Red; Pause; exit 1 }
try { $mysqlDir = Find-Tool -Paths @('C:\Program Files\MySQL\MySQL Server 8.0','C:\Program Files\MySQL\MySQL Server 8.4','C:\Program Files\MySQL\MySQL Server 9.0') -CheckFile 'bin\mysql.exe' } catch { Write-Host "[ERROR] $_" -ForegroundColor Red; Pause; exit 1 }

$env:PATH = "$mingwDir\bin;$mysqlDir\bin;$mysqlDir\lib;" + $env:PATH

# ═══════════════════ 辅助函数 ═══════════════════
function Write-Step($msg) { Write-Host "[*] $msg" -ForegroundColor Cyan }
function Write-OK($msg)   { Write-Host "    $msg" -ForegroundColor Green }
function Write-Warn($msg) { Write-Host "    $msg" -ForegroundColor Yellow }

# ═══════════════════ stop ═══════════════════
if ($Command -eq 'stop') {
    Write-Step '停止服务器...'
    netstat -ano | Select-String ":$port" | Select-String 'LISTENING' | ForEach-Object {
        $m = [regex]::Match($_.ToString().Trim(), '(\d+)\s*$')
        if ($m.Success) { Write-Host "    Killing PID $($m.Groups[1].Value)"; taskkill /PID $($m.Groups[1].Value) /F 2>$null | Out-Null }
    }
    taskkill /F /IM ngrok.exe 2>$null | Out-Null
    Write-OK '已停止'
    Pause; exit 0
}

# ═══════════════════ 读配置 ═══════════════════
$dbUser = 'root'; $dbPass = ''
$ini = "$projectDir\db_config.ini"
if (Test-Path $ini) {
    $content = Get-Content $ini -Encoding UTF8
    foreach ($line in $content) {
        if ($line -match '^\s*user\s*=\s*(.*)')     { $dbUser = $matches[1].Trim() }
        if ($line -match '^\s*password\s*=\s*(.*)') { $dbPass = $matches[1].Trim() }
    }
}
if (-not $dbPass) { $dbPass = '200357' }

# ═══════════════════ 构建子命令 ═══════════════════
if ($Command -eq 'build') {
    Write-Step '编译后端...'
    Push-Location "$projectDir\server"
    mingw32-make -j4; if ($LASTEXITCODE -ne 0) { Pop-Location; throw '编译失败' }
    Write-OK '编译完成'; Pop-Location; exit 0
}
if ($Command -eq 'rebuild') {
    Write-Step '重新编译后端...'
    Push-Location "$projectDir\server"
    mingw32-make clean 2>$null | Out-Null
    mingw32-make -j4; if ($LASTEXITCODE -ne 0) { Pop-Location; throw '编译失败' }
    Write-OK '重新编译完成'; Pop-Location; exit 0
}
if ($Command -eq 'frontend') {
    Write-Step '编译前端...'
    Push-Location "$projectDir\client"
    if (-not (Test-Path 'node_modules')) { npm install 2>&1 | Out-Host }
    npm run build 2>&1 | Out-Host
    if ($LASTEXITCODE -ne 0) { Pop-Location; throw '前端编译失败' }
    Write-OK '前端编译完成'; Pop-Location; exit 0
}
if ($Command -eq 'all') {
    Write-Step '编译前端...'
    Push-Location "$projectDir\client"
    if (-not (Test-Path 'node_modules')) { npm install 2>&1 | Out-Host }
    npm run build 2>&1 | Out-Host
    if ($LASTEXITCODE -ne 0) { Pop-Location; throw '前端编译失败' }
    Pop-Location
    Write-Step '重新编译后端...'
    Push-Location "$projectDir\server"
    mingw32-make clean 2>$null | Out-Null
    mingw32-make -j4; if ($LASTEXITCODE -ne 0) { Pop-Location; throw '编译失败' }
    Pop-Location
    Write-OK '全部编译完成'
    $Command = 'start'
}

# ═══════════════════ MySQL 检测 ═══════════════════
Write-Step '检测 MySQL 连接...'
$svcNames = @('MySQL80','MySQL84','MySQL90','MySQL8.0','MySQL8.4')
$foundSvc = $null
foreach ($s in $svcNames) {
    $info = sc query $s 2>&1 | Out-String
    if ($info -match 'SERVICE_NAME') { $foundSvc = $s; break }
}
if ($foundSvc) {
    $state = sc query $foundSvc 2>&1 | Out-String
    if ($state -notmatch 'RUNNING') {
        Write-Host "    启动 MySQL 服务 '$foundSvc'..."
        net start $foundSvc 2>$null | Out-Null
        Start-Sleep 2
    }
}

$retry = 0
while ($retry -lt 3) {
    mysql -u $dbUser -p$dbPass --batch --no-beep -e 'SELECT 1' 2>$null | Out-Null
    if ($LASTEXITCODE -eq 0) { break }
    $retry++
    if ($retry -lt 3) {
        $dbPass = Read-Host -AsSecureString "    MySQL 密码 (user=$dbUser)" | ForEach-Object { [Runtime.InteropServices.Marshal]::PtrToStringAuto([Runtime.InteropServices.Marshal]::SecureStringToBSTR($_)) }
        if (-not $dbPass) { Write-Warn '已取消'; Pause; exit 1 }
    }
}
if ($retry -ge 3) { Write-Host "[ERROR] MySQL 连接失败" -ForegroundColor Red; Pause; exit 1 }
Write-OK 'MySQL 连接成功'

# ═══════════════════ 确保必要文件 ═══════════════════
if (-not (Test-Path $ini)) {
    Write-Step '生成 db_config.ini...'
    @"
[database]
host = 127.0.0.1
port = 3306
user = $dbUser
password = $dbPass
database = scms_db
pool_size = 5

[server]
receipt_dir = storage/receipts
export_dir  = storage/exports
backup_dir  = storage/backups

[app]
auto_backup_hour = 3
session_timeout = 0
"@ | Out-File -FilePath $ini -Encoding UTF8
    Write-OK '配置已生成'
}

if (-not (Test-Path "$projectDir\public\index.html")) {
    Write-Step '未找到 public/，编译前端...'
    Push-Location "$projectDir\client"
    if (-not (Test-Path 'node_modules')) { npm install 2>&1 | Out-Host }
    npm run build 2>&1 | Out-Host
    Pop-Location
}

if (-not (Test-Path "$projectDir\scms.exe")) {
    Write-Step '未找到 scms.exe，正在编译...'
    Push-Location "$projectDir\server"
    mingw32-make clean 2>$null | Out-Null
    mingw32-make -j4; if ($LASTEXITCODE -ne 0) { Pop-Location; throw '编译失败' }
    Pop-Location
}

# ═══════════════════ ngrok（公网模式） ═══════════════════
$ngrok = "$env:USERPROFILE\ngrok.exe"
$publicUrl = $null
if ($Command -eq 'public') {
    if (Test-Path $ngrok) {
        taskkill /F /IM ngrok.exe 2>$null | Out-Null
        Write-Step '启动 ngrok 隧道...'
        Start-Process $ngrok -ArgumentList "http $port --log=stdout" -WindowStyle Hidden
        for ($i = 0; $i -lt 15; $i++) {
            Start-Sleep 1
            try { $publicUrl = (Invoke-RestMethod 'http://127.0.0.1:4040/api/tunnels' -TimeoutSec 2).tunnels[0].public_url; break } catch {}
        }
        if ($publicUrl) { Write-OK "ngrok 就绪: $publicUrl" } else { Write-Warn 'ngrok 启动超时，仍在后台尝试...' }
    } else {
        Write-Warn "未找到 ngrok ($ngrok)，回退到本地模式"
    }
}

# ═══════════════════ 启动 ═══════════════════
Write-Host ''
Write-Host '============================================================' -ForegroundColor Green
Write-Host '  SCMS - 学生社团管理系统' -ForegroundColor Green
Write-Host '============================================================' -ForegroundColor Green
Write-Host "  本地: http://127.0.0.1:$port"
if ($publicUrl) { Write-Host "  公网: $publicUrl" }
Write-Host '  按 Ctrl+C 停止' -ForegroundColor White
Write-Host ''

Set-Location $projectDir
Start-Process "http://127.0.0.1:$port"
& .\scms.exe

Write-Host ''
Write-Host '服务器已停止。' -ForegroundColor Yellow
taskkill /F /IM ngrok.exe 2>$null | Out-Null
