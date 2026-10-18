# SARDAB 2.3 — Bayer UI preview

Serve this folder locally: `python3 -m http.server 8792 --bind 127.0.0.1`.
Open http://127.0.0.1:8792/webapp/installer/ in Chrome or Edge.
Back up with the bundled editor before installing. Check the displayed version
is **sardab-2.3-bayer-ui-preview**. See docs/FLASHING.md and docs/BAYER-UI.md.

The default SARDAB palette now uses black, blue and red, with static Bayer
patterns and no decorative torches. A saved alternative palette is preserved;
select MENU > DISPLAY > COLOR > SARDAB to use the new theme.
SCL/TUNE effective tuning readouts and DESERT sample status remain included.
Open webapp/emulator/ for the rebuilt browser version; refresh an open tab.

Built and host/emulator tested, including desktop/phone Chrome checks.
Physical FM-1 validation of this build remains pending.
Source: https://github.com/jrdntnnr/sardab/tree/main
Public installer: https://jrdntnnr.github.io/sardab/webapp/installer/
BUILDINFO.json identifies the firmware. SOURCE.patch applies to the source base
commit listed there. SHA256SUMS covers every packaged file except itself.
