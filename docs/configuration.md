# Local overlay settings (initial implementation)

The local Go server exposes `GET /api/v1/settings` and `PUT /api/v1/settings` for the default overlay. Settings are versioned JSON and persisted atomically to the user's OS configuration directory (`Keycast/settings.json`), or to the path supplied by `-config`.

Settings include accent color, opacity, scale, and mouse visibility. The configuration dashboard at the Vite development root loads and saves these settings. The overlay reads them on load and polls periodically for changes. Settings do not contain raw input history.

The server remains bound to `127.0.0.1`; API writes require a matching local Origin header when sent from a browser. Production web embedding, named profiles, advanced layouts, and cross-process locking are future work.
