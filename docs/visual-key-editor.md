# Visual keyboard editor

For a named profile with **Full keyboard** selected, use the interactive keyboard in settings to select a key and drag it. The editor updates the existing `keyOffsets` profile map, so the OBS overlay uses the same offsets as numeric X/Y controls. Dragging is limited to -300 through +300 pixels on each axis, consistent with server validation. Save the profile to persist changes.

The width and height sliders below the visual editor still control the selected key, and **Reset position** clears its offset. The editor preserves existing width, height and position overrides. Keyboard rows remain the layout baseline: keys can overlap when dragged, and the editor does not provide collision avoidance or resize handles yet.
