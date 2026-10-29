# Custom reactive animations — implementation plan

## User story
A streamer can upload their own character/overlay assets, map key/mouse events to poses or animation clips, preview them live, and use the result as a transparent OBS browser source. Bongo Cat-style two-paw input is a built-in **template**, not a copyrighted asset pack.

## Architecture
- Keep existing keyboard/mouse overlays as the default. Add optional `reactiveScene` to named profiles, versioned separately from settings v1.
- Store binary assets in the per-user Keycast config directory, **not** embedded in settings JSON. Use opaque generated IDs and a manifest with content type, dimensions, SHA-256, size and original display name.
- Provide loopback-only authenticated-by-local-origin asset endpoints: upload/list/delete and immutable GET by opaque ID. Restrict to PNG, WebP, GIF in v1; validate file signatures and decoded image bounds, enforce per-file and aggregate quotas; no SVG, HTML, JS, external URLs, or path traversal.
- Serve assets with `nosniff`, strict MIME types and a restrictive Content Security Policy. Imported/exported profiles reference asset IDs; a future self-contained scene bundle can package files with a manifest.
- Add a pure event-to-scene reducer in the web client. Use normalized input codes for key down/up, mouse button, wheel and movement; do not persist input history. Use the existing WebSocket state snapshot for reconnection.
- Scene has ordered layers with x/y, scale, rotation, anchor, opacity, z-index, idle asset and event-specific states. A trigger has input match, priority, hold/release policy, optional duration, cooldown and frame timing. A deterministic precedence rule resolves simultaneous triggers.
- For Bongo-style presets, use an idle base plus left/right paw layers. Left and right are independent so simultaneous keys/mouse buttons show both paws; default bindings are configurable. Provide only procedural placeholders, no third-party copyrighted artwork.
- Preview uses the same scene renderer as OBS, with test event buttons and simulated input. Preserve transparent browser-source output and existing profile/placement settings.
- Rendering uses image swapping and CSS transitions first. Sprite-sheet frame sequences follow after a manifest format is established. Respect reduced-motion preferences.

## Delivery PRs
1. **Asset library**: upload/list/delete API, safe file validation, quota, tests, UI uploader and thumbnails.
2. **Scene schema and runtime**: typed layer/trigger config, Go validation, event reducer, transparent scene renderer, reconnect behavior and tests.
3. **Bongo-style editor**: drag/drop layers, idle/pressed state binding, simultaneous left/right paw template, live preview and event tester.
4. **Animation timeline**: sprite frames, frame timing, easing, interrupt/priority policies, import/export of scene bundles and regression tests.

## Acceptance tests
- Idle, left-only, right-only and both-down render correctly for keyboard and mouse mappings.
- Key-up and WebSocket reconnect restore correct states without sticky poses.
- Wheel and movement pulses expire automatically; held key states persist until release.
- Malformed, oversized, disguised or traversing uploads are rejected; deleted assets cannot be fetched.
- Multiple OBS browser sources and profiles do not leak or mutate one another.
- Existing profiles and default overlays continue working unchanged.
- Build and test Go, web and Windows portable release; validate on Windows/OBS with user-supplied artwork.
