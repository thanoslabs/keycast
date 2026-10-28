# Independent overlay component sizes

Named profiles can resize the keyboard and mouse independently from 50% to 200%. Open the profile placement editor and adjust either size slider, then save the profile. The existing overall profile scale continues to apply to the entire overlay.

The new optional `keyboardScale` and `mouseScale` fields are stored inside each profile's placement object. Missing or zero values mean 100%, preserving existing saved profiles. The default profile and legacy side-by-side overlays are unchanged.

This change covers whole-component sizing only. Individual key placement and dimensions are a separate milestone.
