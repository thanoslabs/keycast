# Install Keycast PNG Avatar in OBS (Windows)

The **OBS Plugin Windows** GitHub Actions workflow builds a native Windows plugin and uploads a `keycast-obs-windows-x64` artifact **only if the actual compilation and tests succeed**. A green prerequisite-only check is not sufficient.

1. Open [OBS Plugin Windows workflow](https://github.com/thanoslabs/keycast/actions/workflows/obs-windows.yaml).
2. Open a successful run on the branch or manually run the workflow, then download the `keycast-obs-windows-x64` artifact.
3. Close OBS. Extract the ZIP, and copy the contained `keycast` directory into `%ProgramData%\\obs-studio\\plugins\\` (create the `plugins` folder if needed). The DLL should end up at `%ProgramData%\\obs-studio\\plugins\\keycast\\bin\\64bit\\keycast.dll`.
4. Start OBS, add a source named **Keycast Live2D**, and leave **PNG avatar mode** checked.
5. Select aligned 612×354 PNG files for the body, background, and idle/pressed left/right flipper layers. The artwork is **not currently embedded in the ZIP**; use the previously prepared Club Penguin PNG layers. The original BongOBS Cubism `.moc3` is not required.
6. For key overlays, select a single **Key overlay folder** containing aligned `W.png`, `A.png`, `S.png`, and `D.png` (uppercase filenames). The corresponding image appears when that key is held. Missing files are simply skipped. Use **Reload assets** after editing or replacing PNGs in place; you do not need to change their paths. This also refreshes body, background, and flipper images.
7. Press WASD and the left mouse button. The left and right flippers should switch independently. If input capture fails, check whether OBS and the foreground application run at different privilege levels.

If the source does not appear, open **Help → Log Files → View Current Log** and search for `keycast`. A missing artifact means the Windows build has not passed; do not install a Linux DLL.

This is a development test build, not a signed installer. It has not yet passed a real OBS Windows runtime smoke test.
