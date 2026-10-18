# Felucca licensing

Felucca is free software, licensed under the GNU General Public License, version 3 only
(`GPL-3.0-only`, full text in `LICENSE`). That covers the code and its own assets. A few
bundled or ported parts keep their own licences; they are listed below, and their licence
texts are in `LICENSES/`.

Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments

## What is GPL-3.0-only

Every file in this tree that carries an `SPDX-License-Identifier: GPL-3.0-only` header:

- the firmware: `firmware/` (app, HAL, update loader)
- the build script and tools: `build.sh`, `tools/`
- the web pages (installer, editor, the browser emulator `web/emu/`) and their tests: `web/` (not the Fukiai, DotGothic16, Vazirmatn and Reem Kufi fonts, below)
- the host tests: `tests/`

Three source files are ports and keep the licence of their originals:
`firmware/src/phys_dsp.c` (DaisySP, MIT), `firmware/src/phys_symp.c` (Rings, MIT) and
`firmware/src/fm6_core.c` (msfa, Apache-2.0). The firmware built with them is GPL-3.0-only as
a whole.

Felucca 1.6.1 moved its ANALOG, LOFI, TRIO, WHEEL and NOISE engines to
[Felucca-Engines](https://github.com/hugelton/Felucca-Engines) under MIT. This tree carries
the 1.6 copies of those files, altered, under GPL-3.0-only as before; it does not take the
submodule.

The SANTUR / DUSK SANTUR and MIJWIZ / ARGHUL additions are our reduced
implementations of Quilcom / Spogg's published instrument designs, built with
the existing PHYS/TRIO DSP. No Quilcom source, waveforms or plugin binaries are
bundled. See [design provenance and differences](docs/SOURCED-SYNTHS.md).

You may use, study, change and share Felucca under the GPL. If you distribute Felucca, or
firmware derived from it, you must also give your recipients its complete corresponding
source under the same licence. That includes devices that ship with modified Felucca inside.

## Hügelton Instruments' own work

All by Hügelton Instruments (Leo Kuroshita), in this tree:

| What | Licence | Where |
| --- | --- | --- |
| Felucca: firmware, tools, web pages, tests | GPL-3.0-only | `LICENSE` |
| The PHASE engine's waveforms: a C port of the oscillator of CrispyZebra (<https://github.com/hugelton/CrispyZebra>) | GPL-3.0 | `firmware/src/eng_phase.c` |
| The DRUM voices and kit | GPL-3.0-only | `firmware/src/drum_voice.c`, `firmware/src/eng_drum.c` |
| The Hügelton Sample Pack: Felucca's drum sounds, made by `tools/gen_waves.py`, from which the SLICE engine's BREAK is built (not CC0) | GPL-3.0-only | `tools/gen_waves.py` |
| The controls picture | GPL-3.0-only | `docs/controls.jpg` |
| The Fukiai icon font (<https://github.com/hugelton/Fukiai>): the firmware's icons (rasterised at build time by `tools/gen_aa_icons.py`) and the web editor's | MIT | `web/fukiai.ttf`, `LICENSES/MIT-Fukiai.txt`, `web/FUKIAI-LICENSE.txt` |

## Third-party material

| What | Licence | Where |
| --- | --- | --- |
| Inter Tight font by The Inter Project Authors (GHOULBOX: in the tree, not used by the build): the UI text, rasterised into the firmware at build time (`tools/gen_aa_font.py`; the generated tables are not offered as a font, and the font declares no Reserved Font Name) | SIL OFL 1.1 | `assets/fonts/InterTight[wght].ttf`, `LICENSES/OFL-InterTight.txt` (also `assets/fonts/OFL.txt`) |
| VT323 font by The VT323 Project Authors (Peter Hull) (GHOULBOX): the UI text, rasterised into the firmware at build time (`tools/gen_aa_font.py --preset ghoulbox`; the generated tables are not offered as a font) | SIL OFL 1.1 | `assets/fonts/VT323-Regular.ttf`, `LICENSES/OFL-VT323.txt` |
| Pirata One font by Rodrigo Fuenzalida and Nicolas Massi (GHOULBOX): the wordmark of the web pages and the guide (the firmware draws its name in VT323: flash) | SIL OFL 1.1 (Reserved Font Name "Pirata") | `assets/fonts/PirataOne-Regular.ttf`, `LICENSES/OFL-PirataOne.txt` |
| Instrument samples (Versilian Studios VSCO-2 Community Edition, VCSL): the SAMPLE sets, also the SLICE engine's PIANO (the PIANO set's middle C) | CC0 1.0 | `assets/samples-cc0/`, provenance in `ATTRIBUTION.txt` there |
| DaisySP by Electrosmith, Corp and Emilie Gillet (<https://github.com/electro-smith/DaisySP>): the PHYS engine's modal and string models and the resonator, ported to fixed point | MIT | `firmware/src/phys_dsp.c`, `LICENSES/MIT-DaisySP.txt` |
| Rings by Emilie Gillet (<https://github.com/pichenettes/eurorack>): the PHYS engine's sympathetic strings, ported to fixed point | MIT | `firmware/src/phys_symp.c`, `LICENSES/MIT-Rings.txt` |
| msfa by Google Inc. and Pascal Gauthier, from Dexed (<https://github.com/asb2m10/dexed>): the FM6 engine's synthesis, ported to integer C (Dexed itself is GPL-3.0; only msfa is used; the FM6 factory patches are Felucca's own) | Apache-2.0 | `firmware/src/fm6_core.c`, `LICENSES/Apache-2.0-msfa.txt` |
| klattsch by Tony Gies (<https://github.com/tgies/klattsch>): design reference for the VOICE engine; no code copied. Formant data from Klatt (1980) / Hillenbrand et al. (1995) | MIT (klattsch) | credit only |
| JIANT (<https://github.com/juanjiant-bit/JIANT>), a Felucca fork: the second core (its start, guards and faults, after Melodee by keremimo <https://github.com/keremimo/melodee> and X0X), the COMB filter, ENV LOOP, ANALOG INT and the GRAIN delay, ported to this tree | GPL-3.0 | `firmware/hal/fm1_multicore.h`, `firmware/src/fx.c`, `voice.c`, `eng_analog.c` |
| X0X by charlesvestal (<https://github.com/charlesvestal/fm1-x0x>), a Felucca fork: the design of the browser emulator (the worklet, the device clock driven by its audio, the exports); its files credit it in their headers | GPL-3.0 | `web/emu/` |
| DotGothic16 font by The DotGothic16 Project Authors (<https://github.com/fontworks-fonts/DotGothic16>): the browser emulator's knob labels, served with its page (cut to printable ASCII; the font declares no Reserved Font Name) | SIL OFL 1.1 | `web/emu/fonts/DotGothic16-subset.woff`, `LICENSES/OFL-DotGothic16.txt` (also `web/emu/fonts/OFL.txt`) |
| Vazirmatn font by The Vazirmatn Project Authors (<https://github.com/rastikerdar/vazirmatn>) (SARDAB): the web editor's text and numbers, Latin and Persian, served with its page (cut to Latin and Arabic script; the font declares no Reserved Font Name) | SIL OFL 1.1 | `web/fonts/vazirmatn.woff2`, `LICENSES/OFL-Vazirmatn.txt` (also `web/fonts/OFL-Vazirmatn.txt`) |
| Reem Kufi font by The Reem Kufi Project Authors (<https://github.com/aliftype/reem-kufi>) (SARDAB): the web editor's headings and Persian wordmark text, served with its page (cut to Latin and Arabic script; its Reserved Font Name "Josefin Sans" is not used) | SIL OFL 1.1 | `web/fonts/reem-kufi.woff2`, `LICENSES/OFL-ReemKufi.txt` (also `web/fonts/OFL-ReemKufi.txt`) |
| JieLi AC79 SDK by JieLi Technology: three of its files go into every `.fwsc` package (below); none are in this tree | Apache-2.0 | `LICENSES/Apache-2.0.txt` |

On the device, HOME held > ABOUT opens the information screen; turning PRESETS scrolls on into
CREDITS, a short list of these authors, licences and source URLs.

## JieLi SDK files in the packages (Apache-2.0)

A `.fwsc` package made by `tools/build.py` (with `tools/fm1pkg_make.py`; this is the package the
web installer installs) holds three unmodified files from the JieLi AC79 SDK
(<https://gitee.com/Jieli-Tech/fw-AC79_AIoT_SDK>, `cpu/wl82/tools/`). They are read from your SDK
checkout at build time (see BUILDING.md); no SDK files are in this tree.

| File in the package | What it is |
| --- | --- |
| `uboot.boot` | the first-stage boot loader (SPL) |
| `cfg_tool.bin` | the chip configuration block |
| `eq_cfg_hw.bin` | the default hardware EQ table |

They are licensed under the Apache License, Version 2.0 (`LICENSES/Apache-2.0.txt`), not under
the GPL, and their copyright stays with JieLi Technology. The SDK has no NOTICE file. Apache-2.0
files may be distributed together with GPL-3.0 code; Felucca itself stays GPL-3.0-only.

Every distribution of a package carries the licence texts with it: the release folder
(`tools/build.py --release`) and the site (`web/make_site.py`, next to the package in
`firmware/` and linked from the installer page).

## No vendor material

No M-VAVE material is part of Felucca. The firmware links no vendor code and contains no vendor
data, and the packages hold only Felucca and the three SDK files above. The installer's
"Return to official V15" holds only the official file's size and SHA-256: you select the
official firmware file you downloaded yourself, and it is checked and installed in your browser,
never uploaded or redistributed.

## Contributions

Contributions are welcome under GPL-3.0-only.

## Trademarks

"Felucca" and "Hügelton Instruments" are names of Hügelton Instruments.

"M-VAVE" and "FM-1" are trademarks of their respective owners. Felucca is independent
firmware that runs on FM-1 hardware. It is not affiliated with, endorsed by or supported
by those owners.

## Radio

Felucca never enables the Bluetooth / Wi-Fi radio of the hardware.

## Desert recordings

`assets/desert/oud*`, `qanun*` and `ney-air*` audio/data are CC0-1.0, separate from
the GPL firmware. The oud and ney-air recordings are also built in as the SAMPLE sets OUD
and NEY (tools/gen_samples.py SET_SRC), in place of Felucca's FLUTE and SAX. Oud: hammondman; Turkish kanun: barisbozkurt; ney: aliaref.
[Source URLs, license links and hashes](assets/desert/sources.json) and
[processing/usage notes](assets/desert/README.md) travel with the sample pack.
The source MP3 previews were converted and pitch-calibrated; original masters
are not included.

`assets/desert/tar-*.wav`, `tar.hdr` and `tar.bin` are adapted **CC BY** recordings
from the Berklee Intersectional Soundbox Archive, 2025: Erfan Hessampour
(instrumentalist), Yangyang Hua (engineer), Shuyi Xiong (editor). The publisher
does not state a CC BY version; we preserve its declaration without assigning
one. These files are not CC0 or GPL. [Recording pages and source hashes](assets/desert/tar-sources.json)
and [credits and modification notice](assets/desert/README.md#persian-tar--cc-by)
must travel with redistributed Tar banks. The bank uses public AAC previews,
trimmed, resampled, pitch-calibrated and ADPCM-encoded.

## Contact velocity

The weak-drive key sensing and PA10 top-key probe in
`firmware/hal/fm1_velocity.h` and the deferred ADC setup are adapted from
[Charles Vestal's fm1-velocity](https://github.com/charlesvestal/fm1-velocity/tree/8f62ce0d968c91ce8bfdafbb908f81d15351650a),
GPL-3.0-only, on the Felucca HAL by Leo Kuroshita / Hügelton Instruments.
`firmware/src/key_velocity.h` implements SARDAB's bounded manual response and
note-on scheduling using that sensing method; Velo's lab synth and unfinished
adaptive calibration are not included. These additions are GPL-3.0-only.

## October 2026 upstream ports

- JIANT `8c334da`: bounded COMB output and grain-boundary pitch updates; `d05e199` / `21b29e7`: smooth NOISE modulation and AMP triode/cabinet processing. SARDAB adapts AMP to its existing INSERT controls. `ae72e01`: staggered LED mid-level pulses. GPL-3.0; source: <https://github.com/juanjiant-bit/JIANT>.
- Melodee `007e44e` / `0c21568`: backup-capability detection and verified stock restoration with our application-only loader. GPL-3.0; source: <https://github.com/keremimo/melodee>.
- SLOOP `3a1350e`: K-weighted level matching and per-file attenuation. Its sample-loop workflow informed SARDAB's loop controls. GPL-3.0; source: <https://github.com/isod89/sloop-fm1>.
- Felucca-Engines `e88bc70` / `7511c0a`: defined wrapping/shift arithmetic, adapted into the existing GPL engine files; MIT, <https://github.com/hugelton/Felucca-Engines>. The parent Felucca attribution remains in the source headers.
- GHOULBOX through `8711711`: emulator portable backups, MIDI panel controls, focus mode, mobile audio recovery and recording/export; GPL-3.0, <https://github.com/jasonpersinger/ghoulbox-fm1-dungeon-synth>. X0X `a4777af` informed minimum-width phone controls.

The emulator's separately loaded codecs are browser-only dependencies:

| Component | Licence | Licence text |
| --- | --- | --- |
| libflac.js 5.6.0 wrapper | MIT | `LICENSES/MIT-libflac.js.txt` |
| libFLAC 1.3.4 | BSD-3-Clause | `LICENSES/BSD-3-Clause-libFLAC.txt` |
| wasm-media-encoders 0.7.0 wrapper | MIT | `LICENSES/MIT-wasm-media-encoders.txt` |
| LAME MP3 codec | LGPL-2.0-or-later | `LICENSES/LGPL-2.0-LAME.txt`, `LICENSES/LAME-NOTICE.txt` |

Exact codec source revisions, build links, hashes and replacement instructions are in
`web/emu/vendor/README.md`, distributed beside the built modules. LAME stays a separate,
replaceable WASM file; no codec is linked into the firmware or audio worklet.
