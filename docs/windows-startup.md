# Start Keycast when signing in to Windows

Extract the portable release ZIP to a permanent directory before enabling startup. In PowerShell, from that directory:

```powershell
.\\startup.ps1 -Action Install
```

This creates a shortcut in the current user's Windows Startup folder pointing to the extracted `keycast.exe`. No administrator privileges are required. The shortcut launches the application minimized; keep the extracted directory in place.

To disable startup:

```powershell
.\\startup.ps1 -Action Remove
```

This helper does **not** provide a system-tray icon, background service, or single-instance protection. Keycast continues to run as a console application. Do not enable startup if another Keycast instance already launches at sign-in, because both will compete for port 9867.
