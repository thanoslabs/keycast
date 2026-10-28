# Portable Windows release

Download `keycast-windows-amd64.zip` from the latest GitHub release, extract the archive, and run `keycast.exe`. Keep the `web/` directory beside the executable. Open `http://127.0.0.1:9867/` to configure profiles, and use `http://127.0.0.1:9867/overlay/default` as an OBS browser source.

The executable listens only on loopback. It stores settings in the operating system user configuration directory. Windows AMD64 is currently the supported distribution target.

The release also provides a standalone executable for diagnostic/API-only use; it does not contain the web UI. Use the ZIP for the complete experience. `SHA256SUMS` lists hashes for both artifacts.
