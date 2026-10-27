# Keycast roadmap and implementation backlog

Issue identifiers below are planning IDs, not GitHub issue numbers. Releases follow SemVer (repository template uses bare `X.Y.Z` tags).

## 0.1.0 — Foundation
- KCY-001 Initialize Go and TypeScript monorepo (S, P0)
- KCY-002 Define normalized input event protocol (M, P0)
- KCY-003 Implement Windows Raw Input capture adapter (L, P0)
- KCY-004 Implement pressed-input state engine (M, P0)
- KCY-005 Implement local HTTP and WebSocket server (M, P0)
- KCY-006 Implement versioned configuration persistence (M, P0)
- KCY-007 Establish CI, tests and release workflow (M, P0)

## 0.2.0 — OBS MVP
- KCY-008 Implement transparent browser overlay (M, P0)
- KCY-009 Implement keyboard and mouse visualizers (L, P0)
- KCY-010 Add animations and simultaneous key handling (M, P0)
- KCY-011 Implement configurable input filtering (M, P0)
- KCY-012 Implement reconnect and state resynchronization (M, P0)

## 0.3.0 — Customization
- KCY-013 Build local web configuration dashboard (L, P1)
- KCY-014 Implement full and compact keyboard layouts (M, P1)
- KCY-015 Add visual style customization (M, P1)
- KCY-016 Add live preview and input simulation (M, P1)

## 0.4.0 — Advanced layouts
- KCY-017 Implement visual drag-and-drop layout editor (XL, P1)
- KCY-018 Implement multiple named overlay profiles (M, P1)
- KCY-019 Implement theme presets and import/export (M, P1)
- KCY-020 Add support for extra mouse buttons (S, P1)

## 1.0.0 — Stable release
- KCY-021 Implement custom CSS and animations (L, P2)
- KCY-022 Implement per-application profiles (L, P2)
- KCY-023 Harden privacy, security and reliability (L, P0)
- KCY-024 Complete Windows packaging, docs and QA (L, P0)

## Dependencies
- KCY-001 -> KCY-002 -> KCY-004 -> KCY-005 -> KCY-008/009/010/011/012.
- KCY-003 can run alongside state/server work using a simulated capture provider.
- KCY-006 -> KCY-013 -> KCY-018 -> KCY-017/019.
- Privacy controls and safe local API access are release gates, not optional post-1.0 work.

## First implementation PR
`feat/project-foundation`: Go module, Vite/React app, protocol types, synthetic input provider, loopback HTTP/WebSocket server, transparent simulated overlay, CI and developer documentation.

Definition of done: a local simulated event appears on the transparent overlay; Go/frontend checks pass; no real input capture is required to exercise the development environment.
