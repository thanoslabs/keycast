# Named overlay profiles

Keycast supports up to 32 named OBS overlay profiles. Each profile saves its own accent, opacity, scale, mouse visibility, and keyboard layout.

The existing settings remain the default profile, preserving previous settings and OBS URLs. Open the dashboard to add, edit, delete and save profiles. Profile names use lowercase letters, digits and hyphens, up to 32 characters.

Example OBS browser-source URL: `http://127.0.0.1:5173/overlay/default?profile=stream`.

The dashboard offers a live preview and a copyable URL. Profiles are stored in the existing settings JSON and reloaded by the overlay. Unknown profile names fall back to default appearance.
