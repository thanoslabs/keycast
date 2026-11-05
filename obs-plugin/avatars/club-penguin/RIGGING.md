# Club Penguin rig source preparation

The approved input is `club_penguin_bongobs_textures_fixed.zip` (provided separately).
Its BongOBS artwork is aligned to a 612 x 354 pixel canvas.

## Import policy

- Keep the existing BongOBS modes and original cat models untouched.
- Use the PNGs under `Bango Cat/mode/standard/` and
  `Bango Cat/mode/keyboard/` as *reference artwork*, not compiled Cubism meshes.
- Exclude every `model/` subtree. These contain the original cat `.moc3`
  files and atlases and must not be relabeled as a penguin model.
- Preserve the original pixel origin, canvas dimensions, alpha, and filenames
  during extraction. Do not crop overlays before recording their bounds.
- Source artwork is user-supplied and is not committed to this repository.

## Cubism Editor authoring checklist

1. Separate body, face, beak, eyes, left flipper, right flipper, and input
   peripherals into editable transparent art meshes.
2. Reconstruct any areas hidden by baked composites. The supplied pack does
   not provide fully isolated artwork for every part.
3. Create deformers and keyforms for `ParamFlipperLeft`,
   `ParamFlipperRight`, `ParamMouseClick`, `ParamBodyBounce`,
   `ParamBodyAngle`, `ParamEyeBlink`, and `ParamBreathing`.
4. Confirm both flippers can animate independently without exposing seams.
5. Export a genuine `.moc3`, `.model3.json`, and texture atlas from Cubism
   Editor. Validate using `obs-plugin/tools/validate_penguin_export.py`.
6. Only then connect the exported rig to a functioning Cubism Core renderer
   and OBS input source.

## Status

Artwork reference extraction is complete outside the repo. A native Cubism
Editor project, exported `.moc3`, and OBS GPU renderer remain outstanding.
Do not mark this as a working Live2D avatar.
