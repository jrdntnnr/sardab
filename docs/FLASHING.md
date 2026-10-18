# Flashing the current FM-1 development build

Rebuilt on 2026-10-18 as **SARDAB 2.3**, with the [Persian splash](PERSIAN-SPLASH.md), [blue/red Bayer theme](BAYER-UI.md), [DESERT UI improvements](DESERT-UI.md), Ghoulbox 2.3.2 DREAM/RING,
maqam-aware generation, the smaller FM6 table, audio DMA acknowledgement and
stock-restore corrections. Existing DESERT instruments, tuning, velocity,
inserts and 48 kHz USB audio remain. BLE is excluded.
[New controls and compatibility](GHOULBOX-2.3-PARITY.md) ·
[Velocity controls](VELOCITY.md) · [Build evidence](INTEGRATION-STATUS.md).

This is a device build with host/emulator validation, awaiting physical tests
of these latest changes. Use the bundled editor: its INFO parameter-count
handling changed. Keep a complete backup from before updating if you may downgrade.

## Install from this computer

1. Save any sounds or music you need to keep. On compatible Felucca/Ghoulbox
   firmware, connect the [local editor](http://127.0.0.1:8792/webapp/editor/),
   use **Complete backup → Save backup**, and close the editor afterward.
   Stock M-VAVE firmware does not support this complete-backup protocol.
   The normal Install button does **not** save a backup automatically.
2. Turn the FM-1 on and connect it directly to the computer with a USB **data**
   cable. Close DAWs and other MIDI applications/tabs. Leave it in its normal
   running mode; the installer requests update mode itself.
3. In **Chrome or Edge**, open
   <http://127.0.0.1:8792/webapp/installer/>. This local page contains
   **sardab-2.3-persian-splash-preview**. Check that exact label before installing.
   The [public installer](https://jrdntnnr.github.io/sardab/webapp/installer/)
   also serves this build; check the same exact label there.
4. Click **Install** and allow **MIDI / SysEx** access when prompted.
5. Keep USB connected and the computer awake through **Verifying**, **Writing**
   and **Restarting**. Wait for **Done** and the FM-1 to boot before closing
   the installer or disconnecting it.

No separate programmer is needed for a normally running FM-1.

## First checks on the device

Velocity remains FIXED initially. To test sensing, hold HOME to open the menu,
release HOME, then turn ALGORITHM
to TOUCH, select VELOCITY with PRESETS and set SENSE with a numbered knob.
Press HOME to save. See the [velocity guide](VELOCITY.md) for response settings.

Start with a low volume. Open **PRESETS**, turn **knob 4 (LIST)** to
**DESERT**, then browse with the **PRESETS knob**. Try **SANTUR**, **MIJWIZ**
and **ARGHUL**. For a simple tuning check, select **ROOT C**,
**MAQAM RAST**, **MAP CHR**, **CHORD OFF**, **TEMP EQUAL**: E and B should be half-flat.
Try **DIA3** for a microtonal triad. Save a small test project and verify that
it returns after restarting.

This build enables the second CPU core. MENU → ABOUT reports the active core
count. To try the same firmware with one core, hold **REC while powering on**;
that setting lasts for the session. Host tests cover the split-render logic,
not actual hardware concurrency or rail voltages.

The Tar bank is optional and is not automatically written into a sample slot
by flashing firmware. Use the editor's Samples controls to prepare/upload Tar.
The DESERT entries USR1/2/3 play the banks in those slots; an empty or invalid
slot shows MISSING and uses the existing sine fallback.

## If the installer cannot connect

- **FM-1 not found:** check the cable supports data, close other MIDI apps,
  and check Chrome's site permission for MIDI/SysEx.
- **Stopped / update mode not found:** only after the installer reports that
  it has stopped, reconnect USB and press Install again to resume. Do not
  interrupt an active write.
- **Installed firmware will not boot normally:** hold OCT− alone at power-on
  for USB rescue, then retry the installer. This recovery feature requires
  this build (or upstream 2.x) to have been installed successfully.
- **Black screen and WL80UBOOT instead of MIDI:** this is a different recovery
  mode; follow the repository's recovery guidance for FM-1 Transporter.

If the local server has stopped, run from the repository root:

```sh
python3 -m http.server 8792 --bind 127.0.0.1 --directory build/device-install
```

## Package identity

- File: `build/device-install/firmware/felucca-sardab-2.3-ghoulbox-parity-preview.fwsc`
- Size: 610,276 bytes; updater identity `FM-1_900`
- SHA-256: `18f6c558870dc092f1e8fa87c9223400a33ea7ddd301e10992ba303d25f734a7`
- Application: 571,096 bytes; 10,468 bytes free
- Static RAM: 59,076 / 73,728 bytes; SDRAM pool: 349,504 / 368,640 bytes

The installer folder includes BUILDINFO.json and SOURCE.patch, recording the
base commit and local source changes used for this preview.

The updater identity is shared with earlier development builds. The display
version is SARDAB 2.1; use the package hash to identify this exact build. This target
profile omits the retired SLICE engine; old SLICE tracks will not play.
Validation logs for this fix are under `build/assessment/menu-fix/`; preceding
upstream checks are under `build/assessment/upstream-port/`.
