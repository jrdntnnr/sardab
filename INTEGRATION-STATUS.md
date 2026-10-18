# SARDAB build status — 2026-10-18

## Latest local rebuild: Bayer UI preview

**sardab-2.3-bayer-ui-preview** adds the AmbientZero blue/red default palette,
shared static Bayer patterns, torch removal, responsive emulator and Focus
Screen fix. [Screens, controls and validation](BAYER-UI.md).

- UI/partial redraw tests, web protocol and emulator DOM checks, package
  update/loader checks and target budgets pass. All **158 audio references
  unchanged**; WASM matches **88,200 native stereo frames** exactly.
- **238 scenes × 12 palettes**, same **244 layout / 912 alignment findings**.
  Chrome playback and desktop/phone visual checks pass; hardware checks pending.
- Application **575,616 / 581,564 bytes**, **5,948 free**. Static RAM **59,172**;
  SDRAM pool **349,504**. Application shrinks by **392 bytes**.
- Package **610,276 bytes**, `FM-1_900`, SHA-256
  `af0689c599957855934e8b986af3b35357437f6b07f2417c3d1cbe7715500b6a`.
- Local installer: `http://127.0.0.1:8792/webapp/installer/`.
  ZIP: `build/sardab-2.3-bayer-ui-preview.zip`. Logs: `build/assessment/bayer-ui/`.
  Source: `jrdntnnr/sardab`, branch `main`. Public emulator and installer:
  `https://jrdntnnr.github.io/sardab/`.

## Previous local rebuild: DESERT UI preview

**sardab-2.3-desert-ui-preview** adds the approved tuning display, DESERT preset
feedback and SARDAB theme cleanup to the parity build below.
[Screens, controls and checks](DESERT-UI.md).

- Six focused native suites, update/loader checks and target budgets pass.
  All **158 audio reference renders are unchanged** from the parity build.
  Rebuilt WASM matches **88,200 native stereo frames** exactly.
- App **576,008 / 581,564 bytes**, **5,556 free**; static RAM **59,172**,
  SDRAM pool **349,504**, both unchanged. Code grows **1,872 bytes**.
- UI audit: **238 scenes × 12 palettes** plus styles/parameter sweeps, with
  the same **244 layout / 912 alignment findings** as before this change.
  No new findings on the new scenes; physical display validation pending.
- Package **610,276 bytes**, identity `FM-1_900`, SHA-256
  `c52c4642c0bb85788f6e5fda5b68e595fc69475817d1df8829c2a4ba988cdbb7`.
- Local installer: `http://127.0.0.1:8792/webapp/installer/`.
  ZIP: `build/sardab-2.3-desert-ui-preview.zip`.
  Evidence: `build/assessment/desert-ui/`. Not pushed publicly.

## Previous local rebuild: Ghoulbox 2.3.2 parity preview

**sardab-2.3-ghoulbox-parity-preview**, displayed as SARDAB 2.3 / GB2.3.2-dune.
[Features, controls and compatibility](GHOULBOX-2.3-PARITY.md): DREAM with a
MAQAM mood, RING drones, FM6 table compression, audio DMA acknowledgement,
project-import and rolling-label fixes, complete official stock-restore path.
No BLE or DX7-import changes. Existing instruments, velocity and 48 kHz USB
audio are retained. Hardware SLICE remains disabled.

- **33 native suites pass**, plus web editor/update/backup suites, a DOM page
  check, and real official V15 hash/whole-image validation. Project migration
  tests also pass AddressSanitizer/UndefinedBehaviorSanitizer.
- DREAM checks **600 scale/tonic combinations × 10 evolution passes**, custom
  masks/cent trims, simultaneous Rast oscillator pitches and save/reload.
- **158 audio references** pass: 143 unchanged, 15 reviewed FM6/mix updates for
  the quadratic lookup. Maximum measured mantissa error **0.00000425 cents**.
- The rebuilt WASM emulator matches **88,200 native stereo frames exactly**.
  This is not a physical-device or live-browser validation of the new build.
- Application **574,136 / 581,564 bytes**; **7,428 bytes free**. Static RAM
  **59,172 / 73,728**; pool **349,504 / 368,640**. Target instruction budgets pass;
  `.ram_text` has 916 instructions/no calls and `.c1_text` 40/no direct calls.
- Package **610,276 bytes**, identity `FM-1_900`, SHA-256
  `bbf32e99568bba17f77d6906680e773e902239b02b3456f701bc13658155aacb`.
- Logs: `build/assessment/ghoulbox-2.3.2/`. Local installer:
  `http://127.0.0.1:8792/webapp/installer/`. ZIP:
  `build/sardab-2.3-ghoulbox-parity-preview.zip`.
  These changes have not been pushed or deployed to the public installer.


## Previous local rebuild: HOME menu refresh fix

The menu accepted knob changes to TOUCH and some LED settings but retained their
old displayed values. `menu_put` now invalidates the open menu on a value change,
including editor-originated changes. This addresses a confirmed display bug;
it does not establish that every reported hardware input problem has this cause.
Release HOME after the menu opens before turning the knobs.

- New regression checks every editable row with all four knobs, at both text
  sizes, comparing the resulting screen with a forced redraw. SENSE enable,
  OCT controls and settings reload also pass. The test failed before the fix.
- Six focused native suites pass: menu values, full UI, velocity, editor, update
  and loader. Existing target budgets pass. The rebuilt emulator matches 88,200
  native frames exactly. No physical device or live browser validation.
- Application **571,096 / 581,564 bytes**, leaving **10,468 bytes**. Static RAM
  **59,076 / 73,728**; pool **349,504 / 368,640**. ISR instruction counts unchanged.
- Package **610,276 bytes**, identity `FM-1_900`, SHA-256
  `18f6c558870dc092f1e8fa87c9223400a33ea7ddd301e10992ba303d25f734a7`.
  Local installer label: **sardab-2.1-menu-fix-preview**. Display: SARDAB 2.1.
  Logs: `build/assessment/menu-fix/`. Public deployment remains unchanged.

## Previous local rebuild: upstream integration preview

Built from `ccf64f3` plus local velocity and upstream changes. Full feature/use
notes: [October ports](UPSTREAM-PORTS-2026-10.md). The newly published vendor SDK
is assessed in [M-VAVE SDK](MVAVE-SDK-ASSESSMENT.md).

- Added AMP INSERT, smooth NOISE modulation, grain-boundary pitch updates,
  bounded COMB and engine arithmetic fixes, and staggered LED mid-level pulses.
- Editor: sustain loops/WAV markers, zero-crossing snapping, sample gain and
  level matching. Emulator: portable backups, MIDI panel controls, chord LEDs,
  mobile audio recovery, focus mode, WAV/FLAC/MP3 recording and phone sizing.
- Stock restore recognizes SARDAB and uses our app-only loader with the pinned
  official V15 image. Tested with the actual V15 supplied in M-VAVE's SDK:
  restored flash is byte-identical to V15; our loader is retained.
- Application **571,064 / 581,564 bytes**, leaving **10,500 bytes**. Static RAM
  **59,076 / 73,728**; pool **349,504 / 368,640**. RAM interrupt code: 916
  instructions, no calls; core-1 code: 40 instructions, no direct calls.
- **26 native suites pass**, including update/loader simulations. The 158 audio
  references stay unchanged. New DSP cases pass address/undefined-behavior
  sanitizers. Existing target budgets pass without increased limits; separate
  entries monitor the new AMP/grain routines.
- Editor, sample/restore, codec and audio recovery tests pass. The emulator
  matches **88,200 native frames exactly**; Tar import/restart and editor-created
  sample-loop sustain/release pass. Page startup is checked with a DOM model;
  live browser automation was unavailable in this session. No claim of browser
  visual or physical hardware validation is made.
- Package **610,276 bytes**, identity `FM-1_900`, SHA-256
  `119640b16b9368758e8d06ac679d9014c765e1c4c6547d480c8468d936d764b9`.
  Local installer label: **sardab-2.1-upstream-preview**. Display: SARDAB 2.1.
  Logs: `build/assessment/upstream-port/`. Public deployment is unchanged.
- DX7 import changes excluded. WHEEL/DRIFT storage changes, USB/BLE integration,
  large sequencer/storage rewrites and physical validation remain separate work.

## Previous local rebuild: contact velocity preview

Built from `ccf64f3` plus the local source changes recorded in the install
folder's `SOURCE.patch`. [Controls](VELOCITY.md) · [Flashing](FLASHING.md).

- Opt-in SENSE, three curves, three touch sensitivities and a top-key probe/fixed
  choice. Existing settings start FIXED. One nonblocking scheduler owns key,
  MASTER and battery ADC conversions and restores GPIO before digital scanning.
- Velocity reaches maqam chords, layered tracks, live MIDI output and recording.
  The arp now retains input velocity through ordering/octaves and every mode,
  including CHORD and DRONE. Playback retains the recorded value; each sequencer
  step still has one shared velocity, taking the strongest recorded attack.
- Application **570,156 / 581,564 bytes**, leaving **11,408 bytes**. Static RAM
  **59,060 / 73,728**; SDRAM pool **349,504 / 368,640**. RAM interrupt section:
  916 instructions, no calls; core-1 section: 40 instructions, no direct calls.
- All checked target budgets pass. Existing ALNK/TIMER5 limits are unchanged;
  separately monitored velocity/keyboard helpers have new baseline entries.
  These are static instruction checks, not measured hardware interrupt times.
- **23 focused native suites pass**, including the new sensing/HAL suites,
  motion/arp behavior, all suites from the prior rebuild and update simulations
  against this exact package. The 158 audio references remain unchanged.
  Velocity and HAL tests also pass address/undefined-behavior sanitizers.
- Editor tests pass. Emulator: 88,200 frames bit-identical to the native build;
  Tar import/playback passes. TOUCH menu inspected at both text sizes. No full
  UI audit or physical hardware validation was performed.
- Package **610,276 bytes**, identity `FM-1_900`, SHA-256
  `23f5270be992a73cdefad5d62ea085ac1c94a80c5fbb64a9964ecc07ffa05b7f`.
  Local installer label: **sardab-2.1-velocity-preview**. Display: SARDAB 2.1.
  The public deployment remains separate. Logs: `build/assessment/velocity/`.

Contact response, finger-placement effects, the upper-key probe and end-to-end
latency require testing on the user's device. No adaptive calibration or
pressure/aftertouch was added. Host/emulator checks cannot certify these.

## Previous local rebuild: ccf64f3

Rebuilt on 2026-10-15 after pulling the Felucca 1.6 / Melodee / JIANT changes.
Firmware sources match `ccf64f3`. Local web fixes supply emulator HAL stubs
for the new hardware-startup references and align the editor with 127 track
parameters / 39 globals. They fix Scala eligibility, CANON descriptors, legacy
preset imports and visibility of KEYS/grain-delay controls. These changes do
not change the device binary.

- Application **567,844 / 581,564 bytes**, leaving **13,720 bytes**.
- Static RAM **58,436 / 73,728**; SDRAM pool **349,504 / 368,640**.
- RAM interrupt code: 916 instructions, no calls. Core-1 code: 40 instructions,
  no direct calls. All checked target instruction budgets pass.
- Package **610,276 bytes**, identity `FM-1_900`, SHA-256
  `8384cbc301b7d689f320a4aa4e146632559ee95651af8c21f763cc9db6aeb574`.
- **19 focused host suites pass**: boot guard, supply-startup transaction logic,
  dual-core split rendering, desert additions, old project/preset migration,
  microtonal chord pitches and mapping, backup, MIDI, keyboard layers, functional
  UI, sample-bank audio, editor protocol and update-entry/loader simulations.
  Audio regression: **158 references, none changed**, no audio health, routing,
  voice-budget or host CPU-budget failures.
- Web editor/protocol, backup, Scala and desert asset checks pass. The emulator
  renders 88,200 frames bit-for-bit against the native comparison build; Tar
  import/playback checks pass. These checks use Node, not a live browser/device.
- The build uses the current default dual-core configuration, with SLICE omitted.
  REC held at power-on selects one core for the session. Host tests do not
  validate hardware concurrency or supply voltages.
- Local installer label: **sardab-2.1-ccf64f3**. The public deployment is separate.
  [Installation instructions](FLASHING.md). Logs and build identity live in
  `build/assessment/device-ccf64f3/`.

This exact rebuilt package has not been flashed in this session. The full UI
layout audit was not rerun; earlier findings below are historical, not a
measurement of this revision.

## Previous build evidence — 2026-10-13


This fork integrates selected **Ghoulbox 2.1** changes (`eaf04bd`) into our
**Felucca 1.5.1 / SLOOP** base, retaining maqam tuning, Scala import, tuned chords
and our instruments. [Controls and migration](GHOULBOX-2-INTEGRATION.md).
The source and deployments belong to
[jrdntnnr/sardab](https://github.com/jrdntnnr/sardab).

**Status: buildable development preview, not a hardware-validated release.**
No FM-1 has been flashed or acoustically tested in this work.

### Combined build at bb65def

- **DESERT** groups our nine synthesized sounds and SAMPLE **USR1/2/3**.
  Device category browsing, loading and settings restoration pass; the editor
  exposes the same collection. Sample banks are uploaded separately.
- Application **579,304 / 581,564 bytes** (2,260 free); static RAM
  **56,068 / 73,728**; SDRAM pool **333,172 / 368,640**. The RAM interrupt
  section is 916 instructions with no calls. All checked target budgets pass,
  including CAVE/TORCH. Boot artwork/keycaps are compressed losslessly.
- Package **610,086 bytes**, identity `FM-1_900`, SHA-256
  `2ab4bd41b416f432b0068c2ffc8117da0c023508ba19825403a9f7b94c082e76`.
  On-device version is SARDAB 2.1 / Ghoulbox 2.1-dune.
- Frozen old SARDAB and upstream 2.1 project/preset migrations pass. Settings
  tests cover the shifted note-name, click, timeout and scene fields and preserve
  the saved restore-session choice. Layout, latch, crank, microtonal chord,
  pitch, storage, backup and editor functional checks pass.
- All **152 audio reference renders** and **720 engine-switch renders** pass
  after the intentional twelve-voice reference update. Rebuilding the same
  integration with eight voices reproduces every old reference. No other
  audio reference changes were accepted.
- Maqam + temperament + owner-trim pitch measurements stay within **0.5 cents**.
  Instrument, sample-bank, Scala and WebAssembly audio/persistence checks pass.
  Update-entry and loader simulations pass against this exact package.
- The broad host suite is **not fully green**. The UI audit reports **596 lint
  findings and 968 alignment findings**, versus the earlier 596 / 1,258;
  those findings remain unresolved. The PHASER host-cost estimate is about
  **148 extra instructions/sample**, just over its 2.5% estimate threshold.
  The same failure reproduces on the unchanged `b9d47b7` source with this
  compiler. No budget was relaxed to hide it. Device CPU headroom and listening
  checks remain pending.

The expanded SAMPLE preset table required updating two old fixed-count tests;
the corrected sample and MIDI program-change checks pass independently.
Validation logs are in `build/assessment/ghoulbox-2/`.
[Flashing instructions and exact package](FLASHING.md).

## Device build from b9d47b7

Rebuilt after the SLOOP port on 2026-10-12. Application **579,484 bytes**
(2,080 free); static RAM **52,724 / 73,728**; SDRAM pool **332,132 / 368,640**.
The 610,086-byte `FM-1_900` package has SHA-256
`fedac81c9a026e213c17e66e04743899a327f925df1c8cbdd30f3b024ddf2301`.

The target-cost check passes against the budgets supplied in this revision.
All 152 audio goldens pass against this revision's updated reference files.
Texture, idle buses, limiter, sound switching, tuning/chords, project/preset
storage, backup, MIDI, editor and functional UI checks pass. The 28 instrument
pitch measurements remain within 1.529 cents with detuning disabled; all four
microtonal chord renders pass. Rebuilt emulator and Tar import checks pass,
as do update-entry, loader and command-line installer simulations. No physical
installation has been performed. The previous full UI render audit was not
repeated and its findings remain unresolved.

[Flashing instructions and exact package](FLASHING.md).

## Earlier instrument build (d0bd9bc)

The current source adds PHYS SANTUR / DUSK SANTUR and TRIO MIJWIZ / ARGHUL,
independent compact adaptations of published Quilcom designs. They use the
existing DSP and allocate no additional synthesis state. See
[controls and provenance](SOURCED-SYNTHS.md). These additions are on the
`microtonal` source branch; the previously published preview assets do not
include them.

- 28 rendered notes across C3–C6, including Rast half flats and a separate
  cent trim: worst fundamental error **1.529 cents**, with detuning disabled.
- All four presets render simultaneous microtonal DIA3 chords; note release,
  project/preset persistence and independently tuned arghul drone checks pass.
- Native functional tests and all **152 audio golden comparisons** pass.
  PHYS parameter/model sweeps and host CPU checks pass. The instrument
  sanitizer run excludes shift and signed-overflow checks used by legacy
  fixed-point arithmetic.
- Rebuilt WebAssembly passes audio/MIDI, persistence and Tar import checks.
  Startup tape hiss is checked separately from exact silence with TAPE off.
- Editor, web backup, Scala and all sample-bank tests pass independently of
  the host script's UI-audit stop.
- Final target build: **575,228-byte application** (6,336 bytes free),
  **50,548 bytes static RAM**, **332,132-byte SDRAM pool**, 916 instructions
  in RAM-resident interrupt code with no calls. `build/felucca.fwsc` is
  610,086 bytes, identity `FM-1_900`, SHA-256
  `c8eea3ab9ee848003ee71e4eda7cc07a530512b210ee9ed92683107236e392ed`.
  Update-entry and loader simulations pass against this final package.
- The full host script still stops at the UI render audit: **596 lint findings
  and 1,258 alignment findings** over 222 screens and twelve palettes.
  These are unresolved; the separate functional UI tests pass.
- The static target estimator flags PHYS at **400 vs 61** and the audio ISR at
  **45,394 vs 37,502**. PHYS disassembly includes backward jumps from separate
  model branches into the shared output block; the estimator counts these as
  loops. Its score is not a measured cycle cost. No budget was raised; device
  CPU and listening validation remain pending.

The older preview evidence below describes the published build, not this
instrument update.

## Preview 2: Dungeons of the Desert

- CITADEL: Bayati on D, 78 BPM; five appended presets and three patterns.
- Optional CC0 Oud / Qanun / Ney Air user banks and editor preparation controls.
- Detuned SUB / LFSR fine-pitch fixes; saturated clock correction; consistent
  precision for all SAMPLE/GRAIN notes on new scales or tracks with cent trims.
- Actual bank playback through DIA7, owner trims and release tests; web WAV
  integrity, root and capacity checks. Playback-rate error stays below one cent;
  source recordings retain natural pitch movement. Native sanitizer checks use
  the repository's existing fixed-point/XIP exclusions.
- **144 audio goldens pass:** five new presets added; all 139 old hashes unchanged.
  Host audio-health, voice/routing and CPU-budget checks pass.
- Final target build and source snapshot match. Hardware checks remain pending.

[Playing guide](DUNGEONS-OF-THE-DESERT.md) · [Sample provenance](../assets/desert/README.md).
The previous full-suite findings below remain unless explicitly updated here;
this update reruns affected native, web, storage and target checks.

## Implemented

- 40 factory scales, including editable Rast/Bayati variants, plus CUSTOM.
- Per-track, root-relative cent trims applied independently to sounding voices.
- Built-in DIA3/DIA7 chords, all five voicings, and altered-note indicators.
- Octave Scala `.scl` import (1–12 degrees; ratios or cents; bounded keyboard
  mapping); no `.kbm`, non-octave periods or more than twelve keys per octave.
- Compact GBM2 projects and preset versions 8/9, with legacy migration tests.
- GHOULBOX scenes, CRYPT drums, revised GURDY, dungeon samples/presets,
  cassette effects and MIDI controls alongside Felucca's modern sequencer,
  stereo HALL, inserts and FM6 bank support.
- Separate settings storage for MIDI learn and GURDY favorites; explicit
  source selection when converting ambiguous old PER4 settings backups.

See [MICROTONAL.md](MICROTONAL.md) for controls, scales and compatibility.
GURDY retains its one-voice cap; use a polyphonic engine for full chords.
External MIDI output remains ordinary note numbers.

## Build evidence

The verified JieLi `jieli-linux-toolchains-20260730.1` compiler runs in an
amd64 Debian container via an Apple-silicon Lima VM with Rosetta. The three
required AC79 SDK packaging assets are SHA-256 checked by the builder.

| Item | Final preview build |
| --- | --- |
| Application | 573,400 bytes (8,164 bytes free) |
| Static RAM | 50,548 / 73,728 bytes |
| SDRAM pool | 332,132 / 368,640 bytes |
| RAM-resident interrupt code | 916 instructions, no calls |
| Loader | 6,930 bytes |
| Firmware package | 610,086 bytes; identity `FM-1_900` |
| Package SHA-256 | `f8eaf0b27c35ecc9ded8e319d7a977a3f8615275ec2c3cb4d44b9ed1e939620d` |

The target profile omits the already-hidden SLICE engine to fit the original
application slot. Engine ID 13 stays reserved, but old SLICE tracks cannot
play in this package. SAMPLE, GRAIN, GURDY, DRUM and the SLICER effect remain.
Host tests retain the full SLICE implementation. [Build instructions](../BUILDING.md).

## Verified on the host

- **7,560 chord cases:** nine maqam presets × twelve roots × seven degrees ×
  triads/sevenths × five voicings. Simultaneous oscillator phase measurements,
  with custom trims, stay within 0.5 cents. Dedicated ASan/UBSan run passes.
- Keyboard WHITE mapping, transposed Bayati MIDI input, CHORD/UP arpeggiation,
  chord recording/playback, releases, custom scale masks and tuned chord labels.
- Fine pitch math, rendered sine frequencies, SAMPLE/GRAIN playback rates,
  independent GURDY melody/drone tuning, track isolation and live retuning.
- **139 original audio golden renders:** zero changed hashes, health failures,
  voice/routing failures, CPU-budget failures or crashes. These golden files
  match GHOULBOX 1.4; they were not regenerated to hide integration failures.
- **720 engine-switch comparisons:** zero differences from the existing
  Felucca reference. Tests explicitly select the original DRUM KIT because
  GHOULBOX added two presets where the reference firmware had only one.
- Project/preset migration, compact storage, settings conversion, persistence,
  MIDI controls, editor protocol and web backup/Scala tests.
- Legacy PIANO HD in USR1 matches the built-in piano bit-for-bit at all twelve
  tested keys. The sample test now addresses the fixed USR1 ID rather than
  mistaking GHOULBOX’s expanded factory-set count for a user-slot ID.
- Native UI functional tests, including the optional legacy FM4 build, pass.
  Text reference and font-spacing checks pass.
- Update entry and loader simulations pass with the final package, including
  interrupted transfers, refused damaged images and write-window boundaries.

## Remaining findings

The full suite is **not green**. No thresholds were raised or failed tests
silently disabled:

- UI render audit: **580 layout findings and 1,154 alignment findings** across
  221 screens, eleven palettes and the parameter sweep. Examples include
  clipped wide numeric values, splash geometry and FM6 chart spacing. The
  audit reports zero text-on-identical-background errors and zero off-palette
  GREY/MONO pixels. The Rast chord screen has also been visually inspected.
- Target static cost: the audio ISR still scores **45,394 vs 37,502**, exceeding
  the existing +10% limit. The prior `noise_render` failure now passes after
  the tuning-rate changes; no budget was raised. These are compiler
  disassembly estimates, not measured device timings. Host CPU checks pass;
  that does not establish FM-1 audio headroom.
- Physical installation, USB enumeration/audio, save/reboot/recovery and
  worst-case playback must still be tested on an FM-1. No connected device
  was available. A browser-driven visual editor test was not performed;
  protocol, parser and deployment-file checks are automated.

Release assets include selected validation logs and checksums. The full-suite
log retains two SAMPLE failures from stale upstream test assumptions; the
corrected test’s separate `sample-final.log` passes. Local complete
logs live under ignored `build/assessment/`; UI audit reports are in
`build/ui_new/`.

## Maintenance judgment

This is worth maintaining for a dungeon-synth instrument: GURDY, the sample
sets, scenes, cassette treatment and musical presets remain useful additions
to Felucca. The expensive part is keeping project formats, parameter IDs,
settings and engine state compatible across two active histories. This merge
adds frozen migration fixtures and pitch/chord tests to make those changes
reviewable. Keep upstream merges explicit and use this preview as the basis
for hardware testing before relying on it as a performance instrument.
