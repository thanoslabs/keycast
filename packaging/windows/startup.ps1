param(
    [ValidateSet("Install", "Remove")]
    [string]$Action = "Install"
)

$ErrorActionPreference = "Stop"
$startup = [Environment]::GetFolderPath("Startup")
$shortcutPath = Join-Path $startup "Keycast.lnk"
$launcherPath = Join-Path $PSScriptRoot "tray.vbs"

if ($Action -eq "Remove") {
    if (Test-Path -LiteralPath $shortcutPath) {
        Remove-Item -LiteralPath $shortcutPath -Force
    }
    Write-Host "Keycast startup shortcut removed."
    exit 0
}

if (-not (Test-Path -LiteralPath $launcherPath)) {
    throw "tray.vbs not found beside this script. Extract the complete release ZIP first."
}

$shell = New-Object -ComObject WScript.Shell
$shortcut = $shell.CreateShortcut($shortcutPath)
$shortcut.TargetPath = Join-Path $env:SystemRoot "System32\\wscript.exe"
$shortcut.Arguments = '"' + $launcherPath + '"'
$shortcut.WorkingDirectory = $PSScriptRoot
$shortcut.Description = "Start Keycast at Windows sign-in"
$shortcut.WindowStyle = 7
$shortcut.Save()
Write-Host "Keycast will start when this user signs in. Remove with: .\startup.ps1 -Action Remove"
