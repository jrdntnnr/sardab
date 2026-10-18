# SARDAB October upstream integration

The later [SARDAB 2.3 integration](GHOULBOX-2.3-PARITY.md) supersedes the
parameter counts and stock-restore description below.

This is a selective port onto SARDAB 2.1 and the local contact-velocity work.
DX7 import changes are explicitly excluded. Existing maqam tuning, key layouts,
chord generation, sequencer velocity, DESERT presets and sample banks remain.

## Device changes

- **AMP insert:** press FX, select INSERT, turn TYPE to AMP. DRIVE, TONE and LEVEL
  control two biased saturation stages and a cabinet model; INSERT MIX blends it.
  Adapted from JIANT. The existing DIST effect and existing INSERT type numbers
  retain their meanings. AMP is appended as type 9 and uses the existing effect
  state, avoiding another parameter ID or pool allocation.
- **NOISE modulation:** in the modulation matrix choose SRC = NOISE and a
  destination such as CUT or PITCH. It wanders smoothly through random values,
  independently per track, with a new target about every 46 ms. AMT controls
  depth. S&H/SLEW and the stored source IDs retain their meanings; NOISE is 11.
- **Grain delay:** changing PITCH latches the new pitch at each grain's silent
  boundary. Existing tape-style TIME smoothing is retained. Turning grain mode
  entirely on/off is not a new crossfade feature.
- **Audio arithmetic:** bounded COMB output prevents overflow on loud summed
  voices; signed shifts/wrapping arithmetic corrected in ANALOG, LOFI, NOISE,
  TRIO and WHEEL. These do not replace the microtonal pitch path.
- **LED scanning:** mid-brightness pulses are staggered between columns, retaining
  each column's duty cycle and the velocity sampling schedule. Reduced audible
  supply noise is the upstream motivation; it has not been measured on our FM-1.

Projects still contain 127 track parameters and retain their current format.
New AMP/NOISE settings require this firmware: older builds cannot reproduce them.

## Sample editor

In the editor's **Samples** tab, prepare files in USR1, USR2 or USR3:

- **Gain** attenuates each file by 0 to 24 dB, in preview and uploaded data.
- **Match levels** turns louder files down using K-weighted loudness estimates.
  This is a practical sample-matching tool, not integrated programme LUFS.
- **Sustain loop** enables a loop; Start/End are milliseconds in the original
  file, adjusted to remain inside a trimmed selection. End is inclusive in the
  stored sample. **Snap loop to zero crossings** searches nearby crossings;
  it reduces boundary discontinuities but cannot guarantee a seamless timbre.
- Forward WAV `smpl` loop markers are imported, including markers after the
  audio chunk. Other audio formats still use browser decoding without markers.
- Preview toggles playback/stop, including loops. On the FM-1, use the SAMPLE
  engine and enable its **LOOP** control to hear the sustain loop. Loop predictor
  and step index are stored with the existing ADPCM zone header.

These changes prepare uploaded data; no extra firmware sample-bank format is
introduced. Tar and existing DESERT assets retain their current bytes.

## Browser emulator

Open **Tools** for portable backup/import/recovery, MIDI panel controls,
fixed-or-input velocity, audio recording and focus mode. Recording exports
24-bit WAV or FLAC, or MP3, using bundled local codec modules. Mobile audio
startup/resume follows browser gesture requirements; an audio recovery button
appears when playback is suspended. Generated chord notes light on the keyboard.
Phone controls have a minimum width and scroll within their panel.

Existing browser saves retain their local storage key. Exported portable files
use the SARDAB format identifier to avoid importing incompatible Ghoulbox states.
The older CC20–27 knob option remains: when enabled it takes precedence over
panel button CC25–27. Disable that legacy option to use the complete new panel map.

## Installer

**Correction, October 17:** do not use this stock-restore path pending repair.
The application-only package omits stock USR/configuration files. See the
[new assessment](UPSTREAM-ASSESSMENT-2026-10-17.md#important-correction-return-to-stock).
The description below records the implementation and its limited tests, not
proof of a complete stock restore. Normal SARDAB installation is unaffected.

Return-to-stock now recognizes SARDAB/GHOULBOX/Felucca backup capability without
requiring an old motion/UI protocol. It still accepts only the pinned unmodified
M-VAVE V15 package. The verified stock image is paired with our application-only
loader to preserve the device's boot data. The stock file stays local and is not
redistributed. No physical return-to-stock operation was performed.

## Deliberately deferred

- WHEEL depth/dust and per-voice DRIFT need a careful storage migration. JIANT
  repurposes old DIGITAL envelope slots; SARDAB still converts legacy DIGITAL
  sounds and keys library parameters by those labels. We have not repurposed
  them or silently changed old sounds. NOISE offers additional movement now.
- USB input/stems, the expanded JIANT sequencer and Melodee's 384-slot preset/UI
  rewrite cross audio scheduling, MIDI, persistence and SARDAB's tuning paths.
  They require separate integration work and device testing.
- Full X0X screen-reader support requires its screen-description protocol;
  phone sizing alone does not provide that feature.
- DX7 import/reset changes were not taken, as requested.

Source versions and comparison evidence: [upstream assessment](UPSTREAM-ASSESSMENT-2026-10-16.md).
