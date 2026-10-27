# Keycast architecture

Status: approved baseline.

## Decisions
- Windows 10/11 x64 first; Go backend.
- Windows Raw Input API (`RegisterRawInputDevices`, `WM_INPUT`) with a dedicated message loop and hidden message-only or suitable receiver window. Verify background capture behavior, device changes, keyboard layouts and secure desktop limits in integration tests.
- React, TypeScript and Vite for web configuration and overlay.
- HTTP/JSON for configuration and WebSocket for live overlay state.
- Versioned JSON files for configuration; single local Go process serving embedded production web assets.
- Bind `127.0.0.1:9867` by default; allow port configuration. Avoid network exposure.
- Platform capture interface isolates Windows implementation for future portability.

## Components
1. `capture`: OS adapter emits physical keyboard/mouse events. No typed text reconstruction.
2. `events`: normalized physical key IDs, device-independent mouse buttons and scroll deltas.
3. `state`: pressed-key/button sets, modifier handling, repeat behavior, snapshots and reset.
4. `filtering`: profile-level allow/deny rules before delivery to browser.
5. `profiles` / `config`: versioned layouts, themes and application rules; atomic writes.
6. `server`: loopback HTTP API and bounded WebSocket fanout; no event persistence.
7. `web/overlay`: read-only rendering, animation and transparent OBS integration.
8. `web/editor`: configuration dashboard, preview and later drag/drop editing.

## Proposed endpoints
- `/`: configuration UI
- `/editor/:profileId`: editor
- `/overlay/:profileId`: OBS source
- `/api/v1/profiles`: profile management (configuration UI only)
- `/ws/v1/overlay/:profileId`: read-only snapshot and live updates

The overlay must never have configuration-write capability. Protect local write endpoints against cross-origin requests and DNS rebinding; avoid assuming loopback alone is authentication. Consider a per-install secret/session for configuration operations. Ensure overlay tokens, if used, cannot mutate settings.

## Event contract (draft)
```json
{
  "version": 1,
  "type": "input.key",
  "sequence": 42,
  "timestamp": 1730000000000,
  "data": { "code": "KeyA", "action": "down", "repeat": false }
}
```
Other event types: `input.mouse_button`, `input.scroll`, `state.snapshot`, `state.reset`. Use physical IDs distinct from user-facing labels. Version the protocol and explicitly document scroll units and sequence reset semantics.

## Connection lifecycle
On connect: validate profile and read-only access, send authoritative filtered snapshot, then ordered live updates. On reconnect: discard stale overlay state, apply new snapshot, then resume events. Backpressure must not silently drop releases and leave stuck indicators; prefer resync/reset on overflow.

## Privacy and threat model
- Never log or persist raw input events; no remote telemetry.
- Provide capture pause/resume and clear state on pause, disconnect and relevant device transitions.
- Displaying printable keys can expose secrets: ship conservative defaults and explain the risk.
- Elevated processes, secure desktop and some Windows contexts may be inaccessible; do not bypass OS protections.
- Treat custom CSS, imported themes and profile data as untrusted; sanitize and isolate appropriately.
- Per-application filtering must remain local; do not send process/window metadata to overlays by default.

## Repository layout (planned)
```text
cmd/keycast/
internal/{capture/windows,events,state,filtering,profiles,config,server}/
web/src/{overlay,editor,components,themes,lib}/
docs/
.github/workflows/
```

## Testing strategy
Unit tests for normalization, filtering, state and migrations; protocol integration tests using a synthetic capture source; browser tests for transparent overlay and reconnect; Windows interactive tests for Raw Input. CI without an interactive desktop should not be treated as hardware validation.
