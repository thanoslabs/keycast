# Simulated input development

Run `go run ./cmd/keycast -simulate`. Separately run `cd web && npm install && npm run dev` and open the Vite site at `/overlay/default`.

The Go synthetic provider repeatedly presses ControlLeft, ShiftLeft, and KeyK, then releases them. No real input is captured.

The server provides a loopback WebSocket at `/ws/v1/overlay/default` with an initial `state.snapshot` followed by `input.key` transitions. The browser reconnects and resets its state after disconnect.

The frontend is served by Vite during development; production embedding and native Windows capture are not part of this PR.
