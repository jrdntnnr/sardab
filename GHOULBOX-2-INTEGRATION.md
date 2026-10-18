# SARDAB 2.1 integration — development preview

This combines the useful Ghoulbox 2.0/2.1 changes (through `eaf04bd`) with
our Felucca 1.5.1 and SLOOP integration. It retains our maqam tuning, Scala
import, instrument engines, sample banks and sequencer. It is built for the
FM-1, but has not yet been flashed or auditioned on physical hardware.

## DESERT preset section

Open **PRESETS**, turn **knob 4 (LIST)** to **DESERT**, then use the
**PRESETS knob** to choose a sound. This section contains:

- DESERT OUD, BRASS QANUN, SANTUR, DUSK SANTUR
- MIJWIZ, ARGHUL, DUNE NEY
- DUNE DRUMS, SAND WIND
- USR1, USR2, USR3

The three USR entries select the corresponding SAMPLE bank, whatever you have
uploaded there. They do not replace its contents. An empty slot is silent;
use the editor's **Samples** tab to upload Oud, Qanun, Ney Air or Tar.
The nine synthesized sounds are built into firmware. User sound presets can
also be saved in the DESERT category. In the editor's **Library → Device
presets**, select **DESERT** from the Show selector.

## Other integrated controls

- **Twelve shared voices**, retaining each engine's cap: PHYS three per track,
  GURDY one, UNISON eight. Layering and chord generators share that budget.
- **Drone latch:** hold GLO and press F#4 / G#4 / A#4 / C#5 for tracks 1–4.
  Enable it, play a chord, and release the keys to keep the drone sounding.
  Disable the latch to release it. Panic also clears it.
- **Optional keyboard layer/split:** tap HOME through to **KEYS**. MODE OFF
  is the default; LAYER plays the selected track and TRK, SPLIT assigns the
  lower keys to TRK. OCT shifts that secondary part. Both parts retain their
  own mapping, maqam, chord and tuning settings. Sequenced accompaniment
  still works with MODE OFF on the short keyboard.
- **GURDY crank:** choose COUP CRANK, then turn SELECT for wheel energy.
  Use GLO + SELECT or HOME → CLOCK for tempo while crank mode is active.
- **Note names:** MENU → CONTROL → NOTE NAMES. Altered notes show their cent
  offset. A tuned chord says TUNED and identifies the first altered note;
  it does not label a neutral-third chord as an ordinary major chord.
- **Temperament:** SCL → CHORD → TEMP: EQUAL, PYTH, MEAN or JUST.
  It adds to the selected maqam and custom cent trims, relative to ROOT.
  Leave EQUAL selected for the original maqam presets. Scala import resets
  TEMP to EQUAL so a previous temperament does not alter the imported scale.
- **Ambience:** FX → TEXTURE → CAVE or TORCH. The existing LEVEL, TONE,
  MOVE, RATE and WHEN controls apply. These are background sources and do
  not consume a synth voice. They share the TEXTURE source selector.
- **USB rescue:** hold OCT− alone while powering on to enter the minimal
  MIDI recovery mode. Two early failed boots also enter rescue. Connect the
  installer and reinstall; resetting from rescue falls back to ROM recovery.
  This cannot recover a damaged loader or an interrupted ROM-level update.

## Saved data and protocol

Existing SARDAB projects (`GBM2`, 125 parameters), presets (versions 8/9),
settings (`GBS1`) and user sample slots remain readable. Old projects default
to equal temperament and keyboard MODE OFF. Existing preset/scale/texture IDs
keep their meanings. The three SAMPLE presets and DESERT category are appended.

New projects keep the 3,840-byte GBM2 container, with a 126-parameter marker;
the packed old parameter positions remain frozen. TEMP uses two spare bits
and KEYS uses a reserved word. Older firmware rejects these new project
records. Save a complete backup before updating or downgrading.

Ghoulbox 2.x FUNA projects and v4/v5 presets with 112 parameters are explicitly
translated: its TEMP does not become our first cent trim, its three new scale
IDs become HIJAZ / HUNG MIN / HIJAZKAR, and engine automation follows the shifted
IDs. PER4 settings translate note-name, screen timeout, click/count-in and
scene fields while keeping the saved restore-session choice.

Upstream ambience imports approximately as an always-on CAVE texture at the
saved level. Its separate crackle density and ambience reverb routing are not
preserved. Felucca PER4 settings still need the explicit source-aware converter
in `tools/migrate_settings.py`; the magic alone cannot distinguish the forks.

Editor INFO now reports 126 track parameters, engine start 118 and 37 globals.
Existing tuning IDs 103–116 and TEXTURE globals 27–32 are unchanged. TEMP is
117, KEYS is 33–36, NOTE NAMES is menu ID 31, and DESERT is category 9.

## Validation and resource decisions

Frozen migration fixtures were generated with the original serializers from
our `b9d47b7` and upstream `eaf04bd`, not with the new importer. Tests exercise
preserved tuning, engine settings, automation, textures and keyboard settings.
A simultaneous Rast seventh chord with JUST and a custom trim is measured
within 0.5 cents; all tonics and temperament tables are checked too.

The larger voice budget intentionally changes two of 152 audio regression
renders and 100 of 720 engine-switch renders. Recompiling the integrated code
with only NVOICE restored to eight reproduces all 152 and 720 previous hashes.
The reference changes therefore reflect the intended voice allocation change.

Lossless Huffman compression of the boot artwork and keycap masks recovers
flash space without reducing sample assets or audio precision. Both generators
assert a pixel-for-pixel round trip. Target instruction budgets cover the new
CAVE/TORCH generator as well as the existing audio paths. Static estimates and
host CPU tests do not establish worst-case hardware timing with twelve voices.

See [current build results](INTEGRATION-STATUS.md) and
[installation instructions](FLASHING.md). Existing screen-layout audit
findings and physical-device validation are still outstanding.
