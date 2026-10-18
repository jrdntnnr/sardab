# Upstream assessment — 2026-10-17

Compared GitHub default branches and releases with the October 16 snapshot,
including the official SDK checked later that day. Inspected changed source in
isolated assessment repositories. No firmware changes were merged, rebuilt or
flashed for this assessment. The current local installer remains
`sardab-2.1-menu-fix-preview`; the user reports that build working on hardware.
That report is not validation of every feature.

## Important correction: return to stock

**Do not use our current “Return to official V15” path until corrected.**
Melodee's [a7553c9](https://github.com/keremimo/melodee/commit/a7553c93ed)
reverses the application-only stock restore that we adopted on October 16.
Copying the stock `flash.bin` into our package preserves those bytes, but does
not restore the other stock package files. Our prior real-V15 test proved the
flash-image copy and loader preservation, not a complete stock restoration.

Confirmed locally with the actual packages: official V15 also contains `USR`
(73,226 bytes) and `isd_config.ini` (3,296 bytes), plus other metadata; our donor
package contains only `flash.bin` and `ota.bin`. `buildStockRestorePackage`
copies only `stock.flash` into that donor. Thus the upstream correction applies
to our implementation too. Normal SARDAB installation is a separate path and
is not implicated by this finding.

Melodee now sends the unchanged verified V15 package through the official
loader, retains the boot-head compatibility check, and checks loader identity
before both install and resume. A boot-head mismatch stops the operation;
it is not bypassed. Port that complete behavior and its wrong-loader tests
before offering stock restore again. No restore operation was attempted here.

## Changes since the last assessment

| Source | Current revision | Change and relevance |
| --- | --- | --- |
| [Ghoulbox](https://github.com/jasonpersinger/ghoulbox-fm1-dungeon-synth/tree/f23fbc593f8af29dfa01174278f08ee79155ed27) | `f23fbc5`; release **2.3.2**; 36 commits | DREAM ambient composition/evolution, GURDY RING drones, flash savings, editor grouping fixes, lessons and release tooling. Strong musical fit after adaptation. |
| [JIANT](https://github.com/juanjiant-bit/JIANT/tree/c5f9b17255f7add0eac157020bcfdef2029e0bc1) | `c5f9b17`; main identifies **0.9.1**; 55 commits | USB/audio scheduling fixes, memory savings, longer delay, DJ filter, finer recorded timing, ANIMAL engine and singing VOICE; extensive UI/sequencer redesign. Main version is not a GitHub release; its earlier stable marker names 0.8.5. |
| [Melodee](https://github.com/keremimo/melodee/tree/84e2d97e958eef0e6e7e1d2c09d10de7f160cff3) | `84e2d97`; two main commits; **1.4.2-rc.1** prerelease | Stock-restore correction and CI changes on main. Separately, the nightly-based release candidate includes chord strums, pattern halve/double, voice budgets, CPU and memory work. Stable remains 1.4.1. |
| [M-VAVE SDK](https://github.com/m-vave/fm1_open/tree/b178ac43c25f010c60a878056fb5331fe693fd62) | `b178ac4`; **V13** source; one commit | Per-track custom tuning, alternate tuning, persistence and per-voice cents support. GitHub's latest listed release remains V12. |
| [felucca-bsp](https://github.com/hugelton/felucca-bsp/tree/7d8f8490c2245f334775889cb7107573b12289e0) | `7d8f849`; 21 commits | Velocity/pressure scanning, USB playback and 24-bit capture, crash/performance diagnostics, recovery tooling and a CPU instruction-interaction report. |

Unchanged: Felucca `5738617` (release 1.6.1), Felucca-Engines `04ffe90`,
SLOOP `4ec1ac8` (2.7.1), X0X `2e31c51` (1.1.0), fm1-velocity `8f62ce0`.
Public SARDAB `microtonal` remains `ccf64f3`: the local velocity, upstream ports
and menu fix have not been published there.

## What to take, and what needs adaptation

1. **Correct stock restore first.** The previous package-assembly tests need
   full-package/loader-routing coverage; matching `flash.bin` is insufficient.
2. **Audit JIANT's audio IRQ fix.** [92be89d](https://github.com/juanjiant-bit/JIANT/commit/92be89d2ad84c1fc34167a32a1f2f3a6a4f16937)
   acknowledges the DMA half before rendering, so a flip during a late render
   remains pending. SARDAB still acknowledges after rendering in `audio.c`.
   This is a concrete shared code pattern, not a measurement of lost audio on
   our device. Its broader USB resampler/core-1 changes need separate memory
   and scheduler checks; do not copy its reported underrun figures as ours.
3. **Recover flash space selectively.** Ghoulbox's 2.2 savings combine compressed
   art/keycaps, a smaller FM6 lookup table with quadratic interpolation, and
   disabling native 48 kHz USB audio. Upstream's 19,328-byte saving is not our
   measured saving. Prioritize lossless assets; preserve 48 kHz unless an
   explicit feature tradeoff is made. FM6 changes need audio/tuning regression
   checks because their output is not bit-identical. Our current build has
   10,468 application bytes free; upstream's new 16 KiB floor would fail it.
4. **Adapt DREAM for desert maqam.** It generates ordinary editable sequence
   steps from the four existing sounds and can evolve them. Its current mood
   table selects MIN/DOR/PHRY/HARM/MAJ/MIX and sets track root/scale; its note
   helper reads semitone masks. Preserve the chosen maqam and its tuned degree
   mapping instead of allowing generation to reset it. Add roles for our
   DESERT instruments. Import its project/sound-load reset fix too.
5. **GURDY RING is a good fit.** It sustains drones through rests; 2.3.2 also
   silences them when the track is muted. Take that fix with the feature.
   Upstream adds a track parameter before engine slots; our parameter IDs and
   motion encoding are already crowded. Allocate storage deliberately and test
   old projects/presets; do not merge upstream's layout literally.
6. **Consider strums, longer delay and a master filter next.** Strum scheduling
   must preserve our independently tuned chord notes, velocities, note-offs and
   recorded onsets. JIANT's finer microtiming changes stored step semantics.
   Its memory savings may help, but shared reverb buffers and compressed banks
   must be assessed against our actual lifetimes and formats.

## Official SDK tuning now matters more

V13's `groove_tuning` has main and alternate tables of twelve whole-cent offsets
per track, indexed by absolute pitch class, range -127..+127 cents. A live hold
or recorded step flag chooses the alternate table. Voices capture tuning at
note-on; existing sounding notes do not retune. Project export can append a
200-byte tuning block; saved tuning occupies a sector previously available to
the sample bank. This is not SARDAB's format.

The updated PLUCK API now accepts cents for individual voices, removing the
specific coarse-pitch obstacle identified in our V12 assessment. Its memory
cost still needs attention. The SDK has no dedicated tuning editor, and these
tables do not replace our named maqam presets, Scala support and keyboard maps.
None of `sdk/include/` changed in V13: no new Bluetooth-output API was found in
this delta. Bluetooth audio-out remains outside our work, as requested.

## New source projects worth tracking

[fm1-base](https://github.com/charlesvestal/fm1-base/tree/6fa5ae2101589bcc5e3a5e75ceda018f444b3c28)
and [CTL-1](https://github.com/charlesvestal/fm1-ctl/tree/e181b4a1ac1510ce18ebf6e271a143c2fe364ccd)
now provide a platform and controller implementation with reported working BLE
MIDI and contact velocity/aftertouch. These are better current references than
the unchanged standalone fm1-velocity experiment. The base also supports WiFi,
but documents substantial radio memory cost and mutually exclusive WiFi/BLE
builds. Neither radio support nor aftertouch was added to SARDAB in this check.

The BSP's [CPU note](https://github.com/hugelton/felucca-bsp/blob/7d8f8490c2245f334775889cb7107573b12289e0/docs/CPU.md)
reports crashes in an AAC decoder when wide multiplies are interleaved with
16-bit loads. This merits targeted disassembly/device investigation; it is
not evidence that SARDAB has reproduced that fault or a reason to indiscriminately
rewrite all DSP multiplication.

API snapshots, comparison patches and fetch logs are retained in
`build/assessment/upstream-2026-10-17/`. This is source assessment; new upstream
features have not been ported or hardware-validated here.
