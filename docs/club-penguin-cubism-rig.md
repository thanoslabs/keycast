# Club Penguin avatar — Cubism rig contract

## Source and scope
Reuse the previously produced **Club Penguin BongoCat** penguin art, not the default BongoCat character. Keep it as its own `Club Penguin` profile; do not overwrite Standard assets. The existing aligned overlay was 612×354, with numbered keyboard/left-hand states, raised-hand art, and Live2D textures. Those raster states are references for drawing/registration, **not** a ready-to-render Cubism rig.

The original PNG/ZIP art must be supplied to the rigging workspace; it is not committed or reconstructed here. Preserve provenance and verify permission to redistribute any derived artwork. Keep source artwork separate from proprietary Cubism SDK files.

## Authoring deliverables
1. Extract or redraw transparent, clean layers for torso/head, eye whites, pupils, beak, left/right flippers, keyboard, and optional shadows/highlights.
2. Set a common canvas, origin, and pivot coordinate system for every layer; preserve the existing keyboard alignment and wide overlay silhouette.
3. In Cubism Editor, create deformers, parameter keyforms, draw order, idle breathing/blink, flipper press/release, and optional mouse gestures. Export a genuine `.moc3`, `.model3.json`, textures, and referenced motions/physics/expressions. PNG layers alone do not satisfy this step.
4. Provide the editable Cubism project separately when licensing permits, plus a redistribution-cleared runtime export.

## Stable parameter IDs
| Parameter | Range | Semantics |
| --- | --- | --- |
| `ParamFlipperLeft` | 0..1 | 0 released; 1 full keyboard press |
| `ParamFlipperRight` | 0..1 | 0 released; 1 full mouse press |
| `ParamKeyDepth` | 0..1 | normalized analog key travel |
| `ParamMouseClick` | 0..1 | mouse click response |
| `ParamBodyBounce` | 0..1 | transient bounce |
| `ParamBodyAngle` | -1..1 | lean direction |
| `ParamEyeBlink` | 0..1 | blink amount (0 open) |
| `ParamBreathing` | 0..1 | breathing cycle |

Cubism parameter IDs must match these names exactly; do not assume the Cubism Editor's default IDs match. Any additional IDs must be documented in the profile.

## Runtime layout
```
avatars/club-penguin/
  club-penguin.model3.json
  club-penguin.moc3
  textures/
  motions/                 # optional
  expressions/             # optional
  physics3.json            # optional
  profile.json             # Keycast bindings; to be implemented
```
Paths inside the manifest must be relative and stay within the model directory. Do not commit SDK binaries or unlicensed artwork.

## MVP acceptance
- OBS displays the transparent avatar at the intended keyboard alignment.
- WASD drives the left flipper; mouse click drives the right flipper.
- Simultaneous presses blend deterministically, with smooth release to idle.
- Wooting analog depth scales the relevant parameter, with digital fallback.
- No raw key history is stored; model/profile files are local.
- Windows OBS plugin artifact is smoke-tested on a clean OBS installation.

## Parallel delivery tracks
- **Rig:** layered source -> Cubism Editor project -> validated runtime export.
- **Renderer:** licensed SDK integration -> GPU textures/meshes -> OBS lifecycle.
- **Inputs:** native capture bridge -> bindings -> smoothing/blending -> analog fallback.
- **Productization:** OBS property UI -> per-avatar profiles -> Windows CI/package.

The SDK license and artwork redistribution permissions are release gates. Never claim the `.moc3` exists until a genuine Cubism Editor export has been supplied.
