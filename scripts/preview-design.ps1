<#
.SYNOPSIS
    Instant GUI Preview
#>
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$PluginName)

$ErrorActionPreference = "Stop"
$RootPath = (Get-Item "$PSScriptRoot\..").FullName
$BuildDir = "$RootPath\build"

Write-Host "--- APC PREVIEW: $PluginName ---" -ForegroundColor Cyan

# 1. Configure (if missing)
if (-not (Test-Path "$BuildDir\plugins\$PluginName\project.sln")) {
    Write-Host "Configuring..." -ForegroundColor Yellow
    cmake -B "$BuildDir" -G "Visual Studio 17 2022" -A x64 --fresh
}

# 2. Build Standalone
Write-Host "Compiling Standalone..." -ForegroundColor Gray
cmake --build "$BuildDir" --config Release --target "$($PluginName)_Standalone"
if ($LASTEXITCODE -ne 0) { throw "Build Failed" }

# 3. Launch & Monitor
$Exe = Get-ChildItem -Path "$BuildDir" -Recurse -Filter "$($PluginName).exe" | Where-Object { $_.FullName -match "Standalone" } | Select-Object -First 1

if ($Exe) {
    Write-Host "Launching..." -ForegroundColor Green
    $p = Start-Process -FilePath $Exe.FullName -PassThru -Wait
    if ($p.ExitCode -ne 0) {
        Write-Warning "CRASH DETECTED. Check Documents/APC_CRASH_REPORT.txt"
    }
} else {
    Write-Error "Standalone EXE not found."
}
