# SARDAB

Dungeon synth firmware for the M-VAVE FM-1, turned toward the desert.

A sardab is the cellar under a Persian house where the heat does not reach.
This firmware is GHOULBOX (a dungeon-synth fork of Felucca) with maqam tuning,
Arabic rhythm, oud and ney recordings, and the sound of stone rooms.

Version SARDAB 2.3 development preview. The latest build is host/emulator-tested;
physical FM-1 validation is pending. [Build status](docs/INTEGRATION-STATUS.md).

![Persian سرداب splash, DESERT OUD, DESERT presets and DISPLAY menu in the current SARDAB palette](docs/sardab-2.3.png)

## Install

Chrome or Edge, a USB data cable, no other MIDI app open:

[jrdntnnr.github.io/sardab](https://jrdntnnr.github.io/sardab/)

Back up first (the editor's Complete backup). Press Install, allow SysEx, keep
the cable in until the FM-1 restarts. Stuck in update mode: replug, Install again.

Back to Felucca: [its installer](https://hugelton.github.io/Felucca/). Back to
M-VAVE: its "Return to official V15". Details: [FLASHING.md](docs/FLASHING.md).

## What is in it

Felucca's thirteen engines, four tracks, 64-step sequencer, song mode, effects,
MOD matrix, MIDI and USB audio. On top:

| | |
| --- | --- |
| Tuning | 49 scales and a CUSTOM one, eighteen of them maqamat and dastgah; per-note cent trims, four temperaments, Scala import |
| Keyboard | ROOT, MAQAM, MAP (CHR, WHITE, ALL); Arabic note names in the header |
| Chords | diatonic triads and sevenths in the scale's own tuning |
| Rhythm | five iqa'at (MAQSUM, BALADI, SAIDI, AYYUB, MALFUF); SLICE's break is a darbuka |
| Recordings | OUD and NEY built in; PIANO, psaltery, organ, recorder, harp; qanun and tar for the USR slots |
| Models | hurdy-gurdy, santur, mijwiz, arghul, desert oud, dune ney, sand wind |
| Rooms | HALL, ROOM, SPRING, STRINGS (sympathetic strings tuned to what is played) |
| Tape | TAPE and CRSH on the mix; the delay's time glides like a motor |
| Playing | ARP TRILL and DRONE; CANON, a second playhead at a scale interval |
| Scenes | DUNGEON, CRYPT, TAVERN, CHAPEL, CITADEL at power-on |
| Machine | the second core renders parts; USB rescue after two failed boots |

The screen is VT323 on stone, with a sword cursor. The console is off by default.

Guides: [playing in the desert](docs/DUNGEONS-OF-THE-DESERT.md) ·
[tuning and mapping](docs/MICROTONAL.md) · [the sourced synths](docs/SOURCED-SYNTHS.md) ·
[GHOULBOX 2 controls](docs/GHOULBOX-2-INTEGRATION.md) · [GHOULBOX 1.4 manual (PDF)](docs/guide/GHOULBOX-User-Guide-1.4.pdf).

## Compatibility

Projects and user presets from Felucca and GHOULBOX load. Settings backups need
the conversion in [MICROTONAL.md](docs/MICROTONAL.md). Felucca 1.5 and older do
not open projects saved here.

## Build

JieLi toolchain and the AC79 SDK ([BUILDING.md](BUILDING.md)):

```
./build.sh
python3 tools/ghoulbox_serve.py    # http://localhost:8000/webapp/installer/
```

The audio and screen code also run on the build machine:

```
SANITIZE=0 sh tests/run_tests.sh
gcc -O2 -w -Ibuild/gen -Ifirmware/src -o build/host/dungeon_demo tests/dungeon_demo.c -lm
build/host/dungeon_demo build/dungeon_demo     # the scenes and every preset, as WAV
```

## Layout

| Path | What |
| --- | --- |
| `firmware/src` | the app, one translation unit (`felucca.c`) |
| `firmware/hal` | the chip |
| `tools/` | generators, build, package, installer |
| `assets/` | fonts, CC0 samples, the desert recordings |
| `web/` | installer, editor, emulator |
| `tests/` | host tests |
| `docs/upstream/` | [Felucca's README](docs/upstream/FELUCCA-README.md) as it came |

## Lineage

Felucca, by Leo Kuroshita / Hügelton Instruments, GPL-3.0-only.
GHOULBOX, by Jason Persinger, GPL-3.0-only.
SLOOP, JIANT and Melodee lent code; the recordings are CC0 and CC BY.
Who made what: [CREDITS.md](CREDITS.md). Terms: [LICENSING.md](LICENSING.md).

Support the original: [GitHub Sponsors](https://github.com/sponsors/hugelton),
[itch.io](https://hugelton.itch.io/felucca).

M-VAVE and FM-1 are their owners' marks. No affiliation.

Bugs: [issues](https://github.com/jrdntnnr/sardab/issues/new). Plans: [ROADMAP.md](ROADMAP.md).
