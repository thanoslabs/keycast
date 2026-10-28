# Wooting 60HE analog input (experimental)

The optional Windows provider uses Wooting's Analog SDK and emits **physical travel** values between 0 and 1, independent of the key's digital actuation point. Launch `keycast.exe -wooting-analog` to enable it. Normal Raw Input capture continues if the SDK is unavailable.

Install a compatible Wooting Analog SDK through Wootility. The application looks for `wooting_analog_sdk_dist.dll` using Windows DLL resolution. If you distribute the SDK DLL with Keycast, obtain it from Wooting's official releases and comply with its license; it is **not bundled** in this PR.

The overlay shows proportional fill on keys that report analog depth. Digital-only keys remain normal. Analog samples are transient WebSocket messages, not saved to settings or input history.

## Validation needed on a physical 60HE

- Confirm the SDK detects the keyboard and its current firmware.
- Verify key mapping for WASD, 1–5, left Ctrl, left Shift and Space.
- Check key release clears depth, and disconnection falls back to digital indicators.
- Measure smoothness and CPU use; the initial poll interval is 16 ms.

SDK API: https://github.com/WootingKb/wooting-analog-sdk/blob/develop/docs/SDK_USAGE.md
