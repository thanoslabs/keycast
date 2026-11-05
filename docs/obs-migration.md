# Keycast native OBS + Cubism migration

## Product contract
Keycast becomes an OBS source plugin. No standalone local server, browser-source runtime, or Go process is required. Existing input mappings and avatar assets are migration inputs, not the new runtime.

## Milestones
- M0: OBS plugin skeleton, Windows packaging, CI, installation smoke test.
- M1: Load .model3.json/.moc3, textures and render transparent Cubism output.
- M2: Motion, expression, physics, blinking and parameter interpolation.
- M3: Keyboard/mouse capture, mapping and trigger engine.
- M4: OBS-integrated configuration dock and model preview.
- M5: Wooting analog input, profile import/export and complex triggers.
- M6: Installation, documentation, signed/reproducible release work and hardening.

## SDK and licensing gate
Live2D Cubism SDK for Native and its dependencies are not vendored. Before M1, review current Live2D SDK licensing, redistributable runtime terms and OBS GPL compatibility. Keep any proprietary SDK artifacts out of source control. A compatible distribution strategy must be documented before distributing binaries.

## Migration and removal plan
The existing Go executable, HTTP server, web editor, Windows tray scripts, and old release artifacts should be removed in a dedicated cleanup PR after the OBS plugin build and release path is verified. Do not remove existing user data schemas before documenting conversion to the new model-profile format.

## Definition of done
M0 is complete only when a clean Windows runner builds a loadable OBS plugin, produces a correctly structured OBS plugin ZIP and smoke testing confirms the source appears in OBS. This scaffold alone does not satisfy M0.
