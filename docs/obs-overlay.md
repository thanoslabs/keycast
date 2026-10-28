# OBS browser overlay (development)

Run `go run ./cmd/keycast` on Windows to capture physical keyboard and mouse input, or `go run ./cmd/keycast -simulate` to preview simulated keyboard events. Run `cd web && npm install && npm run dev` in another terminal.

Add the Vite development URL ending in `/overlay/default` as an OBS Browser Source (for example `http://127.0.0.1:5173/overlay/default`). The page background is transparent. Keep both processes running; this is not yet a bundled production web server.

The keyboard panel shows currently pressed keys, mapping common Windows physical scan codes to readable labels. Unknown scan codes remain visible as raw codes rather than being misidentified. The mouse panel highlights five buttons and briefly displays wheel direction. No pointer coordinates or typed text are stored.

Reconnects clear stale state and request a fresh snapshot. Sequence gaps trigger a reconnect. Manual acceptance should cover modifier combinations, side buttons, wheel events, browser-source transparency, and reconnection.

This is a first OBS-facing UI, not the final keyboard layout or a packaged release.
