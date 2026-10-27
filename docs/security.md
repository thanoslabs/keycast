# Keycast security and privacy

Keycast observes global keyboard and mouse activity to render an OBS overlay. Input visualization can expose passwords and other sensitive information on stream.

## Defaults and safeguards
- Bind only to loopback by default; never expose the server to the LAN or internet without explicit future design and review.
- Do not store, log, replay or transmit raw input history to external services.
- Provide pause/resume and clear pressed state on pause.
- Use conservative defaults for printable-key visualization and communicate risks clearly.
- Enforce origin checks and protect configuration-changing HTTP endpoints from CSRF and DNS rebinding.
- Separate read-only overlay access from write-capable configuration access.
- Validate and constrain imported layouts, themes and custom CSS.
- Do not attempt to capture Windows secure desktop or bypass elevated-process protections.
- Treat foreground-application metadata as local-only unless explicitly configured otherwise.

## Security testing
Check cross-origin access, port binding, invalid profile IDs, oversized WebSocket messages, bounded event queues, reconnection state and import validation before release.
