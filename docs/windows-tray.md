# Windows tray launcher

From the extracted portable release ZIP, double-click `tray.vbs` instead of `keycast.exe` to start Keycast with a notification-area icon and hidden console window. Double-click the icon or choose **Open Settings** to open the dashboard. Choose **Exit Keycast** to stop the child process and remove the icon.

The launcher uses Windows PowerShell and .NET Windows Forms already available on typical Windows installations; it does not alter settings or install a service. It only controls the Keycast process it starts. If the server fails to start (for example because port 9867 is occupied), the launcher displays an error and exits. The icon uses the generic Windows application icon pending custom branding.

**Note:** This is a script-based tray launcher, not a native tray implementation inside the Go executable. The existing startup helper still launches `keycast.exe` directly. A future change can integrate tray startup once the launcher has been verified on Windows.
