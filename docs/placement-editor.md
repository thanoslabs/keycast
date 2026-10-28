# Overlay placement editor

Named profiles can optionally store keyboard and mouse positions on a fixed 960 × 540 canvas. The dashboard offers a pointer-driven editor: select a named profile, drag either component, then save the profile. Use Reset positions to restore the initial arrangement.

Positions are saved with the profile and applied by its OBS browser-source URL. Existing profiles without placement data continue using the original side-by-side layout. The default profile is unchanged.

This first iteration positions the keyboard and mouse as blocks. Individual key repositioning, snap-to-grid, resizing and keyboard-accessible drag controls are follow-up work. Test OBS transparency, profile switching, save/reload and keyboard/mouse events on Windows before merging.
