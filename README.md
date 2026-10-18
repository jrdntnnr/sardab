# SARDAB 2.3 — Persian splash preview

Build **sardab-2.3-persian-splash-preview**. The boot name is now سرداب, using one renderer on the
physical FM-1 and in the emulator. README screenshots show the current theme.
See docs/PERSIAN-SPLASH.md and docs/FLASHING.md.

Serve this folder: `python3 -m http.server 8792 --bind 127.0.0.1`.
Open http://127.0.0.1:8792/webapp/installer/ in Chrome or Edge, or use
https://jrdntnnr.github.io/sardab/webapp/installer/.
Save a complete backup before updating. Physical-device validation is pending.
Source commit: 830bb5831fe8451f30ba895e347411832e7a5f39, branch main. Firmware SHA-256 is in BUILDINFO.json.
SOURCE.patch applies to the source_base_commit; SHA256SUMS covers bundled files.
