# Keycast OBS plugin installation (development)

The native plugin is not yet production-ready. The current scaffold does not
render Cubism models. Do not remove a working Keycast installation yet.

## Intended release layout
- Windows: OBS plugin binary and data under OBS-compatible plugin directories.
- No local web server, tray launcher, PowerShell bootstrap, or Go runtime.
- Model files are user-supplied and never included without redistribution rights.
- Cubism SDK redistribution must be reviewed before shipping runtime components.

## Release checklist
- Build and test against supported OBS versions on Windows.
- Load plugin in OBS and confirm Keycast Live2D appears in Sources.
- Test transparent compositing and device reset.
- Test valid and malformed model manifests, missing textures, and large models.
- Test model motion, physics, parameter mapping, input capture lifecycle.
- Test multiple sources and scene transitions.
- Produce SHA-256 manifest and scan the packaged binaries.
- Verify license notices and all bundled dependencies.
