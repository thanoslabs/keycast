# Individual key widths

Named profiles using the full keyboard layout can override the relative width of individual keys. Select a profile, choose **Full keyboard**, pick a key and adjust its width from 50% to 300%, then save. **Reset selected key** removes its override.

Overrides are stored as an optional `keyWidths` map in each named profile and are applied in the OBS overlay. Existing profiles without overrides render unchanged. The compact pressed-key layout is not affected.

This is the first per-key customization step: widths only. Arbitrary per-key coordinates, heights and drag handles remain future work.
