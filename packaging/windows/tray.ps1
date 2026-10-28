$ErrorActionPreference = "Stop"
Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing

$exe = Join-Path $PSScriptRoot "keycast.exe"
if (-not (Test-Path -LiteralPath $exe)) {
    [System.Windows.Forms.MessageBox]::Show("Extract the complete Keycast ZIP before launching.", "Keycast")
    exit 1
}

$process = Start-Process -FilePath $exe -WorkingDirectory $PSScriptRoot -WindowStyle Hidden -PassThru
Start-Sleep -Milliseconds 800
if ($process.HasExited) {
    [System.Windows.Forms.MessageBox]::Show("Keycast did not start. Check whether another instance is running on port 9867.", "Keycast")
    exit 1
}

$context = New-Object System.Windows.Forms.ApplicationContext
$tray = New-Object System.Windows.Forms.NotifyIcon
$tray.Icon = [System.Drawing.SystemIcons]::Application
$tray.Text = "Keycast"
$menu = New-Object System.Windows.Forms.ContextMenuStrip
$open = $menu.Items.Add("Open Settings")
$open.Add_Click({ Start-Process "http://127.0.0.1:9867/" })
$exit = $menu.Items.Add("Exit Keycast")
$exit.Add_Click({ $context.ExitThread() })
$tray.ContextMenuStrip = $menu
$tray.Visible = $true
$tray.Add_DoubleClick({ Start-Process "http://127.0.0.1:9867/" })
$timer = New-Object System.Windows.Forms.Timer
$timer.Interval = 1500
$timer.Add_Tick({
    if ($process.HasExited) {
        $context.ExitThread()
    }
})
$timer.Start()
try {
    [System.Windows.Forms.Application]::Run($context)
} finally {
    $timer.Stop()
    $timer.Dispose()
    $tray.Visible = $false
    $tray.Dispose()
    $menu.Dispose()
    if (-not $process.HasExited) {
        Stop-Process -Id $process.Id -ErrorAction SilentlyContinue
    }
    $process.Dispose()
}
