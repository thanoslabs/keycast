# Windows Raw Input capture

Keycast uses Windows Raw Input for physical keyboard key-down and key-up transitions. Run `go run ./cmd/keycast` on an interactive Windows desktop. The existing `-simulate` flag retains the cross-platform synthetic source.

The adapter runs a Win32 message loop on a locked OS thread, registers a hidden receiver window for keyboard usage page 1 / usage 6 with `RIDEV_INPUTSINK`, and converts scan codes into stable physical IDs (`Scan_XX`, `ScanE0_XX`, `ScanE1_XX`). These are intentionally not reconstructed characters or text. Human-readable key labels and mouse capture remain follow-up work.

## Validation
- Build with `GOOS=windows GOARCH=amd64 go build ./cmd/keycast`.
- On Windows, press and release keys while the app is unfocused and verify WebSocket transitions and snapshot state.
- Verify left/right modifiers, extended keys, autorepeat, device unplug, shutdown and restart.
- Confirm no input history is written to logs or disk.
- Secure desktop and elevated-process limitations are respected.

Windows interactive validation is required before marking this feature production-ready; ordinary hosted CI cannot exercise Raw Input hardware.
