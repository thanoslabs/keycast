# Overlay placement editor

Named profiles can optionally store keyboard and mouse positions on a fixed 960 × 540 canvas. The dashboard offers a pointer-driven editor: select a named profile, drag either component, then save the profile. Use Reset positions to restore the initial arrangement.

Positions are saved with the profile and applied by its OBS browser-source URL. Existing profiles without placement data continue using the original side-by-side layout. The default profile is unchanged.

This first iteration positions the keyboard and mouse as blocks. Individual key repositioning, snap-to-grid, resizing and keyboard-accessible drag controls are follow-up work. Test OBS transparency, profile switching, save/reload and keyboard/mouse events on Windows before merging.

## Precision controls

The editor offers optional snap-to-grid at 4, 8, 16 or 32 pixels, toggleable grid guides, arrow-key nudging (Shift + arrow for larger steps), and numeric X/Y coordinate fields. These are editor preferences; the saved profile continues to store only pixel positions, so existing profile files remain compatible. Save the profile to persist the new coordinates.
