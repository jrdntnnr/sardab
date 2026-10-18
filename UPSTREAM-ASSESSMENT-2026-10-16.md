# Upstream assessment — 2026-10-16

Checked public default branches and releases on 2026-10-16, around 19:12 UTC.
Compared against SARDAB `ccf64f3` **and the local, uncommitted velocity work**.
Fetched isolated upstream repositories under `build/assessment/upstream-2026-10-16/`;
no upstream code was merged and the installer package was not changed.

## What moved

| Source | Current source | Compared with our integration | Assessment |
| --- | --- | --- | --- |
| [Ghoulbox](https://github.com/jasonpersinger/ghoulbox-fm1-dungeon-synth/commit/8711711a1550be4284b2c81ef31f3602448e7c8c) | `8711711`, Oct 15; release still 2.1 | Five commits after previously fetched `2f001b1` | Browser performance tools, portable state, audio recording/export, mobile audio and generated chord lighting. No device firmware changes in this delta. Good imports. |
| [Felucca](https://github.com/hugelton/Felucca/commit/5738617968a22b5d4c0d3470cfcc95559052f446) | `5738617`, Oct 16; release 1.6.1 | We merged 1.6 and selectively took 1.6.1's update-size checks, safe CPU halt and LARGE fixes | No newer synthesis release. Subsequent commits concern maintenance status, licensing/attribution and credits. Do not replace our existing multicore implementation with another one just to match version numbers. |
| [Melodee](https://github.com/keremimo/melodee/releases/tag/v1.4.1) | Release 1.4.1; main `0c21568`, Oct 16 | Our power sequence came from 1.3.6 | Substantial 1.4 UI, storage, sound and recording changes; newer stock-restore fixes are directly relevant. |
| [JIANT](https://github.com/juanjiant-bit/JIANT/tree/b0aa1fe84143028f6356a493c5f45ebee3f7a495) | Main `b0aa1fe`, version 0.8.2, Oct 16; no GitHub releases returned | Our latest credited pick was 0.6.74 (`eb724db`) | 159 intervening commits including merges; firmware delta 48 files, +5,903/−881 lines. Useful isolated fixes and effects, but a large architectural divergence. |
| [SLOOP](https://github.com/isod89/sloop-fm1/releases/tag/v2.7.1) | `4ec1ac8`, release 2.7.1, Oct 16 | Selected 2.6 features from `d30d655`; nine commits since that snapshot | Sample loops and editor performance/sample tools are useful. Its power and safe CPU-halt improvements overlap fixes we already have. |
| [Velo](https://github.com/charlesvestal/fm1-velocity/tree/8f62ce0d968c91ce8bfdafbb908f81d15351650a) | `8f62ce0`, Oct 14 | Exact source assessed for our port | Unchanged. No newer calibration or aftertouch fix to import. |
| [X0X](https://github.com/charlesvestal/fm1-x0x/releases/tag/v1.1.0) | Main `2e31c51`, release 1.1.0, Oct 15 | Emulator design reference, not a fully merged upstream | Screen-reader protocol, accessible browser controls and phone sizing are relevant. Its percussion/303 application is outside our current focus. |

The public SARDAB default branch still points at `ccf64f3`: no intervening remote
SARDAB changes were found. The velocity preview remains local.

## Maintenance has changed

Felucca's author has [stepped back from active development](https://github.com/hugelton/Felucca/blob/5738617968a22b5d4c0d3470cfcc95559052f446/README.md).
Occasional maintenance may continue, but features, issue responses and PR review
are not guaranteed. The repository remains available. This strengthens the case
for maintaining SARDAB with selected fixes from several sources.

The extracted [Felucca-Engines](https://github.com/hugelton/Felucca-Engines/tree/04ffe903eb87a752305d1e5b10da01b53e4ab004)
repository **is still maintained**. Its head is `04ffe90`, beyond the `c46e008`
submodule pinned by Felucca 1.6.1. It now contains six MIT engines, including
PHASE, plus arithmetic fixes and configurable sample-rate support. PHASE is
already present in SARDAB; this is not an additional instrument for us.

Also checked [felucca-bsp](https://github.com/hugelton/felucca-bsp/tree/6eee4bf973):
latest change concerns USB licensing; earlier changes include experimental BLE
work. This is a separate hardware-layer effort, not a firmware update to drop
onto our scan, velocity and multicore code.

## First batch: small correctness fixes

1. **COMB overflow — take first.** JIANT's
   [8c334da](https://github.com/juanjiant-bit/JIANT/commit/8c334daeffac9d4a0975784ccc9c605fac35d516)
   fixes the output multiply overflowing on loud summed voices. Our
   `voice.c:track_comb` retains the old multiply. A local UBSan probe of the
   actual function with input 160,000 and gain 16,768 reports signed overflow
   in `dsp.c:mulq15`. This confirms the arithmetic defect; it does not measure
   how often a particular preset reaches it on hardware. Port the bounded
   calculation while preserving our tuning and bus-idle behavior.

2. **Clean DX7 imports — excluded by user after this assessment.** JIANT
   [15e179e](https://github.com/juanjiant-bit/JIANT/commit/15e179e4cf87794e37a8e7d2ec780210f73e3142)
   This change was assessed but NOT ported. It
   resets the seven FM6 macros when importing a foreign voice or selecting a
   bank voice. Our bank-selection probe confirms that FB +2 and MOD +25 remain
   after loading B1; the editor PUT path also leaves them unchanged. Reset on
   external import/bank selection, while retaining intentionally saved macro
   values when loading our own projects and presets. Do not describe JIANT's
   measured noise increase as a measurement of SARDAB.

3. **Stock-restore compatibility — take Melodee's approach and adapt.**
   [007e44e](https://github.com/keremimo/melodee/commit/007e44e44041c1f44603a1ea1ec9ed856e4818ab)
   checks backup capability independently of motion/UI versions.
   [0c21568](https://github.com/keremimo/melodee/commit/0c21568a03626f1240229f7f368cfc2a7b09bafb)
   uses the verified official image with an application-only loader, preserving
   device boot data. Our page still uses the vendor-loader path and its
   `archiveInfo` requires a `FELUCCA ` name. Running that function with otherwise
   identical supported INFO packets accepts `FELUCCA 1.6` and rejects
   `SARDAB 2.1`. Correct that existing SARDAB bug as part of the port. These
   findings concern **Return to official V15**, not normal SARDAB installation.
   Reuse our loader and identifiers, with host restore/rejection tests; no
   physical stock restore was performed for this assessment.

4. **Engine arithmetic — selectively take.** Felucca-Engines
   [e88bc70](https://github.com/hugelton/Felucca-Engines/commit/e88bc70200beece3c13b831cafc76c009d20afbb)
   and [7511c0a](https://github.com/hugelton/Felucca-Engines/commit/7511c0a784dc10d5fadc5e06dcb583118e02010e)
   remove signed-shift and overflow undefined behavior, preserving upstream
   render bits. Our copies still contain affected expressions. Port the
   arithmetic hunks and tests; retain our maqam pitch path, ANALOG additions
   and preset tables. Verify SARDAB's own audio references and JieLi output;
   upstream's unchanged Cortex-M0 code size is not a measurement of this chip.

5. **Stagger LED current — evaluate alongside velocity.** JIANT
   [ae72e01](https://github.com/juanjiant-bit/JIANT/commit/ae72e01e9d02b1e2e7c95f18a807953a6cf170fb)
   spreads medium-brightness LED pulses across columns to reduce audible supply
   pulsing. Our medium LEDs still share a phase. Potentially useful, but the
   reported noise followed JIANT's brighter idle setting; it has not been
   reproduced here. Recheck our PA10 top-key velocity probe and scan budget
   when adapting it.

## Second batch: useful for Dungeons of the Desert

| Candidate | Benefit | Integration judgment |
| --- | --- | --- |
| Ghoulbox browser recording, WAV/FLAC/MP3, portable state, chord lighting | Capture performances and see generated chord tones; better mobile playback | High value, little device-flash cost. Adapt browser import validation and identity to SARDAB. Both projects currently use 3,840-byte project containers and matching persisted sector ranges, but equal sizes do not establish format compatibility. Test maqam/Scala, DESERT/USR banks, layers and velocity settings. Keep encoder notices/licenses and our UI additions. |
| JIANT delay read crossfades and grain pitch latched per grain | Fewer clicks when manipulating shimmer and falling echoes | Good fit. Port selectively from `8c334da` plus follow-up `10855a8`/`f801a5d`; preserve our cathedral/HALL implementation. Its later analog/digital/ping-pong delay is a larger sound/parameter change. |
| JIANT WHEEL rotor depth and dust; slow DRIFT modulation | Old organs, breath and motion for desert drones | Good musical fit. DUST partly overlaps our existing TAPE/TEXTURE. New parameter IDs and persistence need design before importing. Source: `d05e199` and its performance follow-up `21b29e7`. |
| SLOOP sample-loop editing and level matching | Better sustained sampled strings/winds and more consistent imported banks | Good fit for Tar and future samples. Our engine already has loop capability; the gain is editor tooling, zero-crossing/WAV-loop handling and workflow. Preserve our three user slots, tuning and bank format. |
| X0X browser accessibility/phone controls | Easier use on small screens and with assistive tools | Browser changes are low-cost. Full spoken-screen support needs SARDAB-specific descriptions and vocabulary; X0X's reported 2.3 KB cost is not our build size. |

## Defer broad imports

- **USB stems/input from JIANT or Melodee:** valuable for recording separate
  parts, but requires adapting descriptors, routing, buffers and interrupt work
  to four SARDAB tracks. Validate on the FM-1 and host OS; do not stack another
  core scheduler or replace our audio pipeline wholesale.
- **JIANT's GEN, step components, WALK/CLIMB and expanded sequencer:** interesting
  future work, but a large format/UI change. Its chord/melody generation must
  use our maqam degrees and tuning; hard-coded conventional harmony is not a
  suitable default for this instrument.
- **Melodee's 384-slot unified preset bank and redesigned UI:** another storage
  and interaction model. It would require explicit migration of DESERT, USR1–3,
  tuning/settings and existing projects. No need to import this to take its
  isolated fixes. Its new SID models are available, but not a priority over
  our acoustic/desert sounds; percussion additions are outside this shortlist.
- **Redundant features:** SARDAB already has the stock supply setup, safe
  CPU halt, multicore rendering, arp modes, latch, per-track MIDI channels,
  custom colours, favorites and microtiming in various forms. Do not count
  their appearance in SLOOP 2.7 as new functionality for us.
- **JIANT delay-tail gate fix:** its timed 4/8-second residue gate is not our
  `bus_idle`/`bus_settle` implementation. The fix is useful context for a delay
  redesign, not evidence that our current delay has that same cutoff bug.

The velocity preview has **11,408 application bytes free**, about 14 KB static
RAM free and 19 KB pool free. Small fixes are realistic; larger feature ports
need measured build deltas. `P_COUNT` is already 127, at the current motion
parameter encoding's limit. New controls require deliberate allocation or a
versioned format change. Keep SARDAB's stable scale/engine IDs and existing
project migration tests throughout.

## Recommended order and evidence

First: COMB, FM6 import, stock restore and engine arithmetic. Next: Ghoulbox's
browser tools, then smooth delay controls. After device validation: selected
WHEEL/DRIFT and sample-editor improvements. Stems and large sequencer changes
should be separate projects with their own compatibility plans.

Evidence is in `build/assessment/upstream-2026-10-16/`: `snapshot.json`, fetched
source repositories, API metadata, `probe.c`, `comb-probe.log`,
`fm6-probe.log` and `stock-backup-probe.log`. The COMB probe intentionally fails
under UBSan; the other probes confirm retained macros and the identity check.
No fixes have been applied or claimed validated by this assessment. The
previous velocity tests remain evidence for their tested paths, not a proof
that every firmware path is free of defects.

Implementation outcome: [October integration](UPSTREAM-PORTS-2026-10.md).
