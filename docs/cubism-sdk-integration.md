# Cubism SDK integration

Keycast's OBS source currently accepts a path to a `.model3.json` file and
reports missing files through OBS logging. **It does not parse or render Cubism
models yet.** The proprietary Cubism Native SDK is not bundled.

## User model directory
Keep the model's original exported directory structure, including its
`.model3.json`, `.moc3`, texture images, motion JSON, expression JSON, and
physics JSON. In OBS, add a **Keycast Live2D** source and select the model's
`.model3.json` in source properties. A `.moc3` alone is insufficient: the
manifest maps textures and other dependencies.

## Required rendering integration
1. Review Cubism SDK for Native redistribution, proprietary framework license,
   OBS/libobs GPL compatibility and third-party notices.
2. Integrate the SDK as an optional, locally provided dependency; never fetch
   SDK binaries from unverified URLs or commit them to the repository.
3. Parse `.model3.json` references with a proper JSON parser and validate all
   paths and file sizes before loading.
4. Initialize the Cubism framework, load `.moc3` through its documented API,
   bind textures and render with a supported OBS graphics backend.
5. Implement texture lifecycle and device-reset handling inside OBS's graphics
   context. Test Direct3D 11 and OpenGL paths separately.
6. Apply motion/physics updates once per frame, then composite the avatar with
   premultiplied-alpha correctness.
7. Test a user-supplied model with explicit redistribution permission.

Do not treat a successful OBS plugin compilation as proof of Cubism model
compatibility. Rendering remains blocked until the SDK integration is complete.
