# FM-1 key velocity assessment — 2026-10-16

Assessed SARDAB firmware ccf64f3 against
[fm1-velocity 8f62ce0](https://github.com/charlesvestal/fm1-velocity/tree/8f62ce0d968c91ce8bfdafbb908f81d15351650a).
The reference checkout and test logs are in `build/assessment/fm1-velocity`.
This records the assessment before implementation. The subsequent SARDAB port
and local package are documented in [VELOCITY.md](VELOCITY.md). Statements below
about missing sensing and unverified build fit describe that earlier state.

## Finding

A selective port is feasible. SARDAB already passes note velocity through its
engines, modulation, MIDI output and live recording. Physical keys currently use
one performance value, `kb_vel` (default 100), in `seq.c:key_on_to`. The missing
piece is per-key sensing and scheduling note-on after that estimate is ready.
Maqam tuning is a separate pitch path; velocity should be attached to the physical
key before its note/chord expansion, including KEYS LAYER.

Velo's current source estimates velocity from contact-resistance readings after
landing, using driven GPIO and a logarithmic map. The README opening describes an
earlier threshold-time approach; the source and dated lab results supersede it.
Twenty keys have direct ADC access. The remaining seven use an indirect PA10 LED
line probe, already measured on hardware but with a narrower, saturating range.
This is expressive sensing without hardware modification, not a conventional
keyboard with two contacts per key. Finger placement affects the reading.

The upstream lab notes report that adaptive calibration increased extremes on real
playing despite passing synthetic tests. They also describe several milliseconds
of sensing delay, with different timing for the indirect probe. Neither sensitivity
nor end-to-end latency has been measured on our SARDAB build/device.

## Proposed integration

1. Keep FIXED as the initial default and add an opt-in SENSE mode. Port sensing
   and the necessary HAL hooks, retaining source attribution; omit the lab synth
   and capture UI. Both repositories mark their relevant source GPL-3.0-only.
2. Give one scheduler ownership of the ADC. SARDAB currently polls MASTER and
   battery synchronously in the main loop; it must not overwrite an in-flight key
   conversion. Merge with our existing scan, LED dimming/breathing and boot sweep.
3. Latch velocity per physical key, then pass it through `key_on_to` to the
   generated notes, layered track and MIDI. Preserve the current pitch/tuning
   paths. Do not import Velo's temporal CHORD grouping by default: our chord
   generator already has one triggering key, whose velocity can feed each note.
4. Start with bounded/manual response settings and resettable calibration, rather
   than adopting the unfinished adaptive algorithm unmodified. Keep a fixed-value
   fallback for the top keys if their probe proves unreliable on this device.
5. Test rapid re-strikes, simultaneous notes, release during measurement, queue
   overflow, mode switches and ADC timeouts for missed notes or stuck voices.
   Verify MIDI, maqam chords, layers, recording and heavy dual-core audio; recheck
   flash/RAM and interrupt budgets. Previous target flash headroom was 13,720 B;
   a port has not yet been built, so its fit is unverified.
6. Validate soft/medium/hard playing and latency on the physical FM-1 with logging.
   Host/emulator tests cannot validate the contact resistance or GPIO probe.

Aftertouch is separate follow-up work. Our AT source is channel-wide; Velo's
per-key pressure would need an explicit voice/chord mapping for polyphonic use.
Likewise, sequencer steps currently store one shared velocity (the strongest
recorded note), not independent velocities for every note of a recorded chord.

## Checks performed

Ran Velo's `sh tests/run_tests.sh`: fastmath, encoder, USB audio, TRS MIDI,
sensing, voice and storage host suites all passed. Update-path tests were skipped
because no Velo device package was built. No Velo firmware was flashed, and no
SARDAB firmware source or installer package was changed for this assessment.

Sources: [README](https://github.com/charlesvestal/fm1-velocity/blob/8f62ce0d968c91ce8bfdafbb908f81d15351650a/README.md),
[hardware lab notes](https://github.com/charlesvestal/fm1-velocity/blob/8f62ce0d968c91ce8bfdafbb908f81d15351650a/notes/01-lab.md),
[sensing code](https://github.com/charlesvestal/fm1-velocity/blob/8f62ce0d968c91ce8bfdafbb908f81d15351650a/firmware/src/app/sense.c).
