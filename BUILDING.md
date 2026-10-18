# Building Felucca

The build makes three files in `build/`:

| File | What |
| --- | --- |
| `felucca.bin` | the firmware app |
| `loader/ota.bin` | the update loader |
| `felucca.fwsc` | the installable package (app + loader) |

## Prerequisites (macOS)

- Python 3 with Pillow and fontTools: `pip3 install Pillow fonttools` (the UI font and icons are
  rasterised at build time)
- Docker Desktop. The JieLi toolchain is Linux x86-64 only; the build runs each tool in a
  `linux/amd64` `debian:bookworm-slim` container (Rosetta on Apple silicon). Keep the source
  tree in a folder Docker can share, e.g. under `/Users`.
- The JieLi Linux toolchain (clang 4.0.1 for pi32v2, from JieLi's package server):

  ```
  tools/get_toolchain.sh            # installs to ~/.jieli/toolchain
  ```

- The JieLi AC79 SDK (Apache-2.0). The package uses three of its files
  (`cpu/wl82/tools/uboot.boot`, `cfg_tool.bin`, `cfg/eq_cfg_hw.bin`); they are not part of this tree.

  ```
  git clone --depth 1 --branch AC79NN_SDK_V1.2.1_2023-12-13 \
      https://gitee.com/Jieli-Tech/fw-AC79_AIoT_SDK.git ~/fw-AC79_AIoT_SDK
  ```

- Node.js (optional, for the web tests).
- Emscripten (optional, `emcc`: the browser emulator, `web/emu/`).

On Linux x86-64 the toolchain runs natively and Docker is not needed.

## Build

```
./build.sh
```

`JIELI_TOOLCHAIN` and `AC79_SDK` override the default locations
(`~/.jieli/toolchain`, `~/fw-AC79_AIoT_SDK`).

`./build.sh --release 1.0` makes a release build: the package identity becomes `FM-1_910`
and the version string `v1.0`; the package is `build/felucca-1.0.fwsc`, and
`build/release-1.0/` holds what a release ships: the package, the app
(`felucca-1.0-app.bin`), `SHA256SUMS`, the sample attribution, `LICENSE`, `LICENSING.md` and
`LICENSES/` (the package contains Apache-2.0 SDK files, so the licence texts travel with it).

Build options (environment, `0` or `1`; defaults in `firmware/src/felucca.c`, `core.h` and `icons.c`):

| Flag | Default | |
| --- | --- | --- |
| `FELUCCA_FLASH` | 1 | settings, presets and projects in flash |
| `FELUCCA_OTA` | 1 | update entry (needs `FELUCCA_FLASH`) |
| `FELUCCA_CDC` | 1 | USB serial console |
| `FELUCCA_CDC_DEFAULT` | 1 | `0`: the console is built in but left out of USB from boot (as MENU > USB SERIAL OFF) |
| `FELUCCA_UAC` | 1 | USB audio input (the master output, stereo, 44.1 or 48 kHz) |
| `FELUCCA_UAC_48K` | 1 | `0`: the USB audio input at 44.1 kHz only, as up to 1.0.5.2 (`firmware/src/usb.c`) |
| `FELUCCA_UAC_AS_FIRST` | 1 | the audio control header lists the audio streaming interface before the MIDI one, so the USB audio driver of older macOS (up to 15) finds it; `0`: 1.0 to 1.5's order, byte for byte (`firmware/src/usb.c`) |
| `FELUCCA_UART` | 1 | TRS MIDI IN |
| `FELUCCA_DUAL_CORE` | 1 | the second core renders some synth parts (JIANT; `hal/fm1_multicore.h`). REC held at power-on: one core for the session; MENU > ABOUT shows CORES 1 / 2 and why. It starts only after the supplies took the stock setup (below) |
| `FELUCCA_SLICE` | 0 | target dungeon profile omits the already-hidden SLICE engine to fit flash; `1` exceeds the current slot |
| `FELUCCA_ICONS` | 1 | parameter icons on the knob cards |
| `FELUCCA_FM4` | 0 | the retired DIGITAL engine (4-operator FM) instead of its FM6 conversion |

`FELUCCA_USB_LAYOUT` (`0` to `3`, default `0`) picks other USB descriptor layouts for testing; see
`firmware/src/usb.c`.

## Samples

The CC0 instrument samples that the SAMPLE engine uses are in `assets/samples-cc0/`
(Versilian Studios, see `ATTRIBUTION.txt` there). `tools/fetch_cc0.py` downloads them
again from the source repositories. Without that folder the build still works, with no
instrument sets in the SAMPLE engine and no PIANO in SLICE (the PIANO set's middle C): they play a plain sine
at the note's pitch instead, and the screen says NO SAMPLE.

## Browser emulator

```
./build.sh
web/emu/build.sh
python3 -m http.server -d build/emu 8790     # open http://localhost:8790/
```

`web/emu/felucca_web.c` builds the firmware's sources as the host tests do, against a simulated FM-1, and
Emscripten compiles it to WebAssembly (`build/emu/felucca.wasm`, with `index.html`, `worklet.js` and the font in `fonts/`). The page
keeps the flash's storage sectors (projects, user presets, settings) in the browser. Not emulated: USB (audio,
MIDI out, the editor, updates), TRS MIDI and the user sample slots. A build of it is on the site:
<https://hugelton.github.io/Felucca/webapp/try/>.

## Tests

```
tests/run_tests.sh
```

Runs the host tests and, with Node.js, the web page tests (and, with Emscripten too, the browser emulator's). Run it after `./build.sh`
(it uses `build/` and needs `AC79_SDK` set as for the build). The suites cover flash storage,
user presets, projects of every format, backup, the keys and knobs, MIDI (USB, TRS, clock,
control), USB audio, the update entry and loader, the command-line installer, the UI (the real
drawing code against stubs: every screen in every palette is rendered and checked for clipped or
overlapping text; PNGs land in `build/ui_new/`), every engine (DRUM, NOISE, PHYS, FM6, SLICE, the
DIGITAL conversion), the chord keys, the modulation matrix, the FX layer, the reverbs, the SLICER
and swing. With `DAISYSP` pointing at a DaisySP checkout, the PHYS models are also compared with
their floating-point originals; without it that test is skipped.

The regression suite (`tests/regress.c`) renders every engine and preset and compares a
hash of each render with `tests/golden.txt`; it also checks levels, voices and the CPU
cost (`tests/cpu_baseline.txt`, `tests/target_budget.txt`). After an intended change of
the sound, `GOLDEN_UPDATE=1 sh tests/run_tests.sh` rewrites the hashes; `BUDGET_UPDATE=1`
does the same for the cost files.

## Install

For the current locally rebuilt package, follow the [device flashing instructions](docs/FLASHING.md).
They identify the exact build and its local installer; the public installer below
may still serve an older published preview.

Use the web installer in Chrome or Edge:
<https://jrdntnnr.github.io/sardab/webapp/installer/>. It installs this fork’s development preview; see [validation status](docs/INTEGRATION-STATUS.md).

From the command line (needs `pip3 install mido python-rtmidi`):

```
python3 tools/fm1_install.py build/felucca.fwsc
python3 tools/fm1_install.py --info          # identity of the connected FM-1
```

Or, to install your own build from the web installer, make a local copy of the site and open it from `localhost`
(Web MIDI needs a secure context):

```
python3 web/make_site.py build/felucca.fwsc dev /tmp/felucca-site
cd /tmp/felucca-site && python3 -m http.server 8000
# open http://localhost:8000/webapp/installer/
```

Installing firmware is at your own risk. If an install fails and the FM-1 no longer
starts, recovery needs [FM-1-transporter](https://github.com/kurogedelic/FM-1-transporter).

## Microtonal dungeon profile

The target builder defaults to `FELUCCA_SLICE=0`. Engine ID 13 remains reserved; this profile cannot play old SLICE tracks. SAMPLE, GRAIN, GURDY, DRUM/CRYPT, all dungeon sounds and the SLICER effect remain available. Native host tests can generate full sample fixtures with `python3 -c 'import sys; sys.path.insert(0,"tools"); import build; build.generate()'`.

On Apple silicon, a Lima VM with Rosetta/binfmt and an amd64 Debian container also works. Point a `docker` wrapper at `limactl shell ghoul nerdctl`, with your own `LIMA_HOME`, then use:

```sh
JIELI_TOOLCHAIN=/path/to/jieli/toolchain python3 tools/build.py --sdk /path/to/ac79-sdk
```

The three SDK assets are checked against SHA-256 hashes in `tools/build.py`. The tested toolchain is `jieli-linux-toolchains-20260730.1`. Do not publish the downloaded compiler or the local VM.

For the package built with SLICE omitted, run the static target check with `FELUCCA_SLICE=0 python3 tests/target_budget.py`. This only permits those two SLICE functions to be absent; other missing or over-budget functions still fail.

## SARDAB 2.1 graphics and target profile

The current build draws the SARDAB boot identity and keycap pills in code;
`gen_ghoulbox_art.py` was removed in the latest integration. Keep generated headers
from the same build profile: the device build omits SLICE to fit flash, whereas
full host/emulator tests may generate assets with SLICE enabled. Run a fresh
target build after such tests before distributing firmware. The target-budget
check for this profile is `FELUCCA_SLICE=0 python3 tests/target_budget.py`.

## The stock supply setup

The SPL leaves the core rail SYSVDD at 11 (1.26 V) and VDC14 at 3 (1.40 V) on its
LDO. M-VAVE's own application raises them to 15 (1.38 V) and 7 (1.60 V) and turns
the DCDC on before it runs; so does every build since this one, in `fm1_cstart`
before the second core starts (`hal/fm1_power.h`, Melodee 1.3.6's sequence from
the stock app at `0x02004F90..0x0200509A`, in its order, with its settling loops).
A P33 transfer that times out stops the sequence; a read-back that is not the
stock's keeps the second core off (MENU > ABOUT: CORES 1). `tests/power_test.c`
checks the transaction trace, every timeout and the read-backs on the host; it
measures no voltage.
