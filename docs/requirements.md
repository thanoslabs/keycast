# Keycast requirements

Status: approved scope · Windows-first · Go backend · local web UI.

## Product
Keycast visualizes keyboard and mouse input in real time through a transparent OBS browser source. The application runs locally, with native input capture separated from the browser overlay and web configuration editor.

## Functional requirements
| ID | Requirement | Release |
| --- | --- | --- |
| INP-01 | Keyboard key-down and key-up capture | 0.1 |
| INP-02 | Left, right, and middle mouse buttons | 0.1 |
| INP-03 | Mouse scroll wheel, including horizontal where available | 0.2 |
| INP-04 | Simultaneous key combinations and modifiers | 0.2 |
| INP-05 | Configurable input filtering | 0.2 |
| INP-06 | Additional mouse buttons | 0.4 |
| OVR-01 | Transparent OBS browser source | 0.2 |
| OVR-02 | Key press/release animations | 0.2 |
| OVR-03 | Mouse button and scroll indicators | 0.2 |
| OVR-04 | Full and compact keyboard layouts | 0.3 |
| OVR-05 | Configurable colors, sizes, and opacity | 0.3 |
| OVR-06 | Custom key positioning and layouts | 0.4 |
| OVR-07 | Theme presets and theme import/export | 0.4 |
| OVR-08 | Custom CSS and animations | 1.0 |
| CFG-01 | Local web configuration interface | 0.3 |
| CFG-02 | Overlay preview and simulated inputs | 0.3 |
| CFG-03 | Persistent configuration | 0.1 |
| CFG-04 | Copy OBS browser-source URL | 0.3 |
| CFG-05 | Multiple named overlay profiles | 0.4 |
| CFG-06 | Drag-and-drop layout editor | 0.4 |
| CFG-07 | Per-application profiles | 1.0 |
| SYS-01 | Loopback-only server by default | 0.1 |
| SYS-02 | Never persist raw input history | 0.1 |
| SYS-03 | Automatic reconnection and state resynchronization | 0.2 |

## Cross-cutting acceptance criteria
- Input state tracks simultaneous presses, repeats, releases and device loss without permanently stuck indicators.
- Initial overlay state is sent as a snapshot on connection; subsequent events are incremental.
- No raw key/mouse event history is written to disk, analytics or logs.
- Global capture can be paused; paused state clears visible inputs and is clearly indicated.
- Web server binds to 127.0.0.1 by default. Configuration API validates request origin and uses appropriate local access controls.
- OBS browser source works with transparent backgrounds and recovers after reconnect.
- User configuration is schema-versioned and migrated without silent loss.
- Custom CSS/theme imports are treated as untrusted input and constrained to avoid accessing configuration privileges.
- Target performance budgets (to be validated): p95 capture-to-overlay update under 50ms, 60 FPS animations, Go process under 150 MB at idle/normal use, startup under 3 seconds, reconnect within 5 seconds.

## Out of scope for v1.0
Gamepad, MIDI, remote/public hosting, raw input recording, cloud accounts, Linux and macOS capture implementations.

## Definition of MVP (0.2)
Launch locally; add OBS browser source; see keyboard/mouse input and simultaneous keys; filter events; reconnect cleanly; persist config; no raw input history.
