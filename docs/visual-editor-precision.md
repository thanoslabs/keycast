# Visual keyboard editor precision controls

The named-profile full keyboard editor supports dragging individual keys and resizing a selected key using the handles on its right (width) and bottom (height). Sizes are clamped to 50–300%, and offsets to ±300 px. Changes update the existing profile override maps and take effect in OBS after saving.

Use **Snap to grid** for drag positioning (4, 8, 16 or 32 px); disable it for pixel precision. Focus a key and press arrow keys to nudge 1 px or Shift + arrows for 10 px. The **Check overlaps** button highlights intersecting keys based on their current rendered rectangles; it is advisory and does not move keys. After further edits, run the check again.

The **Reset all selected key customizations** button clears width, height and offset overrides together. Existing individual reset controls remain available.
