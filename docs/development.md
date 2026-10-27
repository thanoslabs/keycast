# Development guide (planned)

## Prerequisites
Windows 10/11 x64, Go, Node.js and npm. Exact versions will be pinned in `.tool-versions` during foundation implementation.

## Development workflow
The first implementation PR will add runnable commands for Go tests, frontend lint/build and local startup. Until then, this repository contains planning documentation, not a working Keycast binary.

## Engineering conventions
- Keep capture, normalization, filtering and rendering decoupled.
- Test with a synthetic event source before depending on native Windows desktop APIs.
- Do not log input payloads, even at debug level.
- Maintain backward-compatible versioned protocol and config migrations.
- Run formatting, linting, unit tests and build checks in GitHub Actions.
- Follow the organization template's SemVer conventions and PR review flow.
