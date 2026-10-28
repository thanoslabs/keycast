# Individual key positioning

Full keyboard named profiles can offset each key from its normal row position. Select a key in the dashboard and enter horizontal (X) and vertical (Y) offsets in pixels, from -300 to +300. Reset position removes that key's offset. Save the profile to apply changes in the OBS overlay.

Offsets are stored in an optional `keyOffsets` map, for example `"Space": {"x": 12, "y": -4}`. Existing profiles without offsets retain their original layout. Width and height overrides remain independent.

Offsets use CSS transforms: neighboring keys do not reflow, and large offsets may overlap other keys or extend beyond the keyboard panel. A freeform drag-and-drop key canvas with collision and bounds handling remains future work.
