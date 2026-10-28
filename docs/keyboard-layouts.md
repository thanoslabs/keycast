# Keyboard layout previews

The OBS overlay supports two browser-side layouts without changing the input protocol:

- `/overlay/default?layout=compact` (default): displays currently held keys.
- `/overlay/default?layout=full`: renders a static keyboard grid and highlights held keys.

The dashboard includes a preview and links to both OBS browser-source URLs. Layout selection is URL-scoped so multiple OBS scenes can use different layouts simultaneously without changing global settings.

This is a first visual layout implementation. It assumes a US-style keyboard and does not yet implement locale-aware keycaps, a full physical scan-code mapping, or saved per-profile layouts. Unknown keys remain visible in compact mode. Mouse visualization and persisted color/scale/opacity settings continue to apply.
