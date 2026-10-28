# Keycast

[![Keycast Build](https://github.com/thanoslabs/keycast/actions/workflows/keycast-build.yml/badge.svg?branch=main)](https://github.com/thanoslabs/keycast/actions/workflows/keycast-build.yml)
[![Windows Build](https://github.com/thanoslabs/keycast/actions/workflows/windows-build.yml/badge.svg?branch=main)](https://github.com/thanoslabs/keycast/actions/workflows/windows-build.yml)
[![Latest release](https://img.shields.io/github/v/release/thanoslabs/keycast?sort=semver&display_name=tag)](https://github.com/thanoslabs/keycast/releases/latest)

Keycast is a localhost-only keyboard and mouse input visualizer for OBS. It captures Windows keyboard and mouse events and displays them in a customizable transparent browser-source overlay.

## Windows quick start

1. Download the **Windows amd64 ZIP** from [Releases](https://github.com/thanoslabs/keycast/releases/latest) and extract the entire archive into a permanent folder.
2. Run `keycast.exe`. If your ZIP contains `tray.vbs`, you can launch through it to get a notification-area icon instead.
3. Open [http://127.0.0.1:9867/](http://127.0.0.1:9867/) to edit settings and named profiles.
4. In OBS, add a **Browser** source pointing to `http://127.0.0.1:9867/overlay/default`. Enable transparency in your source's CSS/layout as needed. For a named profile, copy its overlay URL from the dashboard.
5. Keep Keycast running while streaming or recording.

The executable binds to `127.0.0.1` by default. Input events are processed live; raw input history is not saved.

## Configuration

The web dashboard supports keyboard layouts, keyboard/mouse placement and scaling, per-key adjustments, theme presets, animations, input filtering, custom CSS, and named profile import/export. The dashboard includes a simulated input preview.

Settings are saved to the operating system's user configuration directory under `Keycast/settings.json` unless overridden with `-config`.

## Run from source

Requirements: Go 1.23+, Node.js 22+, npm.

```sh
cd web && npm install && npm run build && cd ..
go run ./cmd/keycast -simulate
```

The portable executable serves built web assets from a `web/` folder next to `keycast.exe`. When running from source, use the Vite development server (`cd web && npm run dev`) for the dashboard. Simulated mode works on non-Windows systems; native Raw Input capture requires Windows.

## Troubleshooting

- **Dashboard doesn't load:** verify Keycast is running and visit `http://127.0.0.1:9867/api/v1/health`.
- **Another instance is running:** stop the existing process before starting another on port 9867.
- **OBS overlay is blank:** confirm the browser-source URL and that the web assets were extracted alongside the executable.
- **No input appears:** check whether native capture started, or use `-simulate` to isolate overlay problems.
- **Tray icon missing:** use the complete extracted ZIP, not the standalone executable; `tray.vbs` is only available in releases that bundle the tray launcher.

## Development

Run `go test ./...` and `cd web && npm run build`. Pull requests are checked by the Go, frontend, and Windows cross-compilation workflows. Releases use bare SemVer tags.

See the [documentation](docs/) for further setup details.
