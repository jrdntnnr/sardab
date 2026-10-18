# SARDAB 2.3 — Ghoulbox 2.3.2 integration

Local preview, 2026-10-18. Ports are based on Ghoulbox
`f23fbc593f8af29dfa01174278f08ee79155ed27`, retaining SARDAB's maqams,
DESERT sounds, contact velocity, sequencer extensions, inserts and USB audio.
BLE and DX7-import changes are excluded.

## DREAM

From the HOME screen, tap HOME through **MIXER → CLOCK → KEYS → DREAM**.
The four knobs control **EVOL**, **DENS**, **DRFT**, and **MOOD**.
A piano key chooses the tonic and generates four 64-step parts using the four
loaded sounds. Existing steps prompt for replacement on the first generation
in a session. Subsequent keys regenerate immediately, as in Ghoulbox.
Stop a playing SONG chain before generating. Ordinary sequencer editing and
saving work on the generated steps.

- **EVOL** enables changes at pattern boundaries.
- **DENS** sets how busy the parts are (1–8).
- **DRFT** controls the evolution interval (1: every eight passes; 8: every pass).
- **MOOD** offers the six Ghoulbox modes: MIN, DOR, PHRY, HARM, MAJ, MIX.
- **MAQAM**, the default additional mood, copies the selected track's scale,
  twelve cent trims, Scala membership mask, and temperament to all four tracks.
  The pressed key supplies the new tonic. Choose the source maqam on SCL first.
  This generates ambient patterns using that pitch collection; it is not a
  model of traditional maqam melodic development.

DREAM replaces steps and automation on all four tracks and sets LEN 64, DIV 1/8,
ROOT, scale, CHORD OFF and ARP OFF. Sounds remain loaded. DESERT instrument names
help assign melody, pad or arpeggio roles. Generated notes use the same per-note
microtonal rendering as live keys and chords. Save a project to keep the result
and DREAM settings; project/sound loads invalidate the generator's cached state.

## RING

Select a **GURDY** track, tap **EDIT** to **RING**, and set ON. While playback
runs, the drones continue through rests as the melody string fades. STOP,
MUTE, or RING OFF releases them. New HURDY GURDY, DRONE WHEEL and BAGPIPE factory
loads enable RING. Previously saved sounds/projects without it load with RING OFF.

## Other changes

- Ghoulbox's smaller quadratic FM6 lookup table saves about 3 KB. Measured
  interpolation error is below 0.0001 cent over a fractional octave. This changes
  exact FM6 sample hashes, so the affected FM6/mix reference renders were reviewed
  and updated; no DX7 import work is included.
- JIANT's early DMA acknowledgement prevents an audio-buffer event arriving
  during rendering from being cleared at the end of the interrupt.
- Fixed an older project-import defaults array that read beyond its bounds when
  initializing parameters appended after the engine block.
- Fixed rolling values erasing the descender of the FREQ label.
- Editor controls and offline descriptors include DREAM/RING; preset-library
  migration retains CANON and RING separately from the eight engine parameters.

## Storage and update compatibility

Existing SARDAB parameter IDs remain unchanged. RING is appended at **127**;
P_COUNT is now **128**, P_E0 remains **118**, and globals are **43**. The seven-bit
INFO count byte uses **0 to mean 128**. Use the editor bundled with this build.
Old editors do not understand that count. Saved GBM2 records remain 3,840 bytes;
RING occupies a previously unused bit, DREAM two spare tail bytes. Older SARDAB
projects/presets load, and Ghoulbox 2.3's FUNA/113-parameter records migrate TEMP,
RING, engine values, motion locks and DREAM controls. Older firmware cannot read
new 128-parameter saves: retain your pre-update backup for downgrading.

The return-to-stock correction follows Melodee
`a7553c93eda29863eb9648406c6811abf2b8c020`: validate the official V15 hash and
serve its complete, unchanged logical image with **its official loader**. Do not
use the earlier app-only stock-restore package. The installer distinguishes
`ota-FM-1_900` (SARDAB) from `ota-FM-1_015` (stock), refusing mismatched loaders
before writing. Device boot-head validation remains in force. A CHECK HEAD /
FAILED -5 refusal means stock boot data differs; no bypass is attempted.

## Scope and limits

The device build retains 48 kHz USB audio and all existing instruments. Our
already smaller drawn keycaps/text splash are kept instead of upstream bitmap
art. SLICE remains disabled in the hardware build, as before; the emulator
includes it. Firmware and emulator host checks do not establish physical-device
behavior. Test DREAM generation/save/reload, quarter-tone chords, RING stop/mute,
velocity recording and USB audio on your FM-1 before relying on a performance.

Build evidence is under `build/assessment/ghoulbox-2.3.2/`. The installer package
includes `BUILDINFO.json`, `SOURCE.patch` and `SHA256SUMS` for reproducibility.
