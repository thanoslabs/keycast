# Native OBS plugin release gate

A production Windows release requires:
1. Windows CI **compiles** the native OBS plugin against compatible OBS headers/libs.
2. Cubism Native SDK backend actually instantiates, updates, and draws a .moc3 model.
3. All GPU textures and meshes are allocated and released in the OBS graphics context.
4. The Club Penguin runtime export is present and licensed for distribution.
5. Install into the correct OBS plugin directory and test clean OBS startup/shutdown.
6. Verify keyboard, mouse, simultaneous inputs, and digital fallback.
7. Run Defender scans and sign releases where practical.

Do not publish a placeholder DLL as a working Live2D plugin.
The current Windows workflow checks prerequisites only; it is not an installer.
