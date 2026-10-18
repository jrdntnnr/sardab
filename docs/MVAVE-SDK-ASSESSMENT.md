# Official M-VAVE FM1 Open SDK assessment

Inspected 2026-10-16 at commit
`5d3eac2573dbd054254fabda6fde099adc594348` in
[m-vave/fm1_open](https://github.com/m-vave/fm1_open/tree/5d3eac2573dbd054254fabda6fde099adc594348).
The checkout is retained under `build/assessment/fm1-open`.

**Keep SARDAB as our firmware base and use the official SDK as a hardware and
transport reference.** It provides useful new integration opportunities, but
replacing our application would require porting maqam tuning, chord behavior,
DESERT instruments, persistence and velocity again. No SDK platform replacement
or Bluetooth/USB audio feature is included in the October preview.

## What the SDK supplies

| Area | Official implementation | Implication for SARDAB |
| --- | --- | --- |
| Panel | SPI2 DMA scanning; 10 kHz TIMER5; debounced digital keys | Candidate to reduce scan work, but must preserve our contact-velocity ADC schedule and LED behavior. |
| Bluetooth | Audio input, BLE MIDI peripheral/central, discovery and connection APIs | Useful integration path; native SDK transport services are required, not just copied headers. |
| USB audio | Stereo input/output, block reads and stream-reset handling | Useful reference for an input/mixer port. Our audio scheduling still needs adaptation. |
| Audio engine | 44.1 kHz stereo, 128-frame blocks, native prebuilt engine library | Different runtime contract from our engine; assess scheduling and memory before adoption. |
| Instruments | PLUCK, PSG and other engines | PLUCK/PSG are candidates. DX7 import work remains excluded. |
| Storage | Eight tracks, eight sample slots, sixteen projects, shared 456 KiB sample area | Different from SARDAB's three ADPCM banks; not a compatible storage upgrade. |

The native build uses Windows PowerShell and PI32v2 clang 4.0.1r3; its browser
build uses WASI SDK 27. `sdk/lib/libfm1_engine.a` contains native PI32v2 objects.
App/browser sources are supplied, but the native system library is a separate
binary dependency. The linker layout, 536,576-byte user area and update family 7
also differ from our current package. This is a porting effort, not a drop-in
replacement for our JieLi toolchain or flash layout.

Relevant source: [panel scanner](https://github.com/m-vave/fm1_open/blob/5d3eac2573dbd054254fabda6fde099adc594348/app/board/fm1_input.h),
[transport API](https://github.com/m-vave/fm1_open/blob/5d3eac2573dbd054254fabda6fde099adc594348/sdk/include/fm1.h),
[audio I/O API](https://github.com/m-vave/fm1_open/blob/5d3eac2573dbd054254fabda6fde099adc594348/sdk/include/fm1_audio_io.h),
[PLUCK](https://github.com/m-vave/fm1_open/blob/5d3eac2573dbd054254fabda6fde099adc594348/third_party/synth/pluck.c).

## What we verified

- All **41 key/button locations and seven encoder wirings** match SARDAB's map.
  The official panel API still reports keys as on/off: it does not provide
  built-in contact velocity or aftertouch. Our opt-in sensing work remains useful.
- Compiled and rendered the official PLUCK in a standalone host probe. It
  produced audio, but its eight-voice state consumes **33,268 bytes**, exceeding
  our remaining **19,136-byte pool** if added unchanged. It also needs independent
  fractional pitch per voice for maqam chords; integer notes plus global bend
  are insufficient. Our existing PHYS strings already cover related synthesis.
- Used the actual bundled **FM-1 V15** package to test our stock-restore builder.
  Its pinned hash passed, restored flash matched the stock image byte-for-byte,
  and our application-only loader was retained. This verifies package assembly,
  not a physical restore on the user's device. The stock image is not bundled
  with our preview.

Evidence: `build/assessment/upstream-port/logs/mvave-wiring.json`,
`mvave-pluck.log` and `web-upstream-real-stock.log`. The SDK itself was not built
or flashed; the PLUCK host probe is a limited component test.

## Integration order

1. Prototype DMA panel scanning with contact velocity enabled, then measure
   latency, contact response and audio noise on hardware.
2. Investigate USB input and Bluetooth transport integration against our runtime;
   establish memory, interrupt and storage compatibility before changing the base.
3. Port useful PLUCK/PSG behavior only with bounded memory and per-voice microtonal
   pitch. Preserve existing projects and DESERT banks.

The immediate SDK benefits are confirmed wiring and a real stock-package test.
Our completed upstream additions and remaining larger ports are documented in
[October integration](UPSTREAM-PORTS-2026-10.md).
