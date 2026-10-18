# SARDAB / سرداب — UI & Visual Identity Specification

> **18 October 2026 update:** the user-approved [Bayer UI](BAYER-UI.md) supersedes
> the original colour and texture proposals below. SARDAB now defaults to black,
> blue and red, with static Bayer patterns and no decorative torches.

**Status:** Proposed — design and implementation handoff, not implemented  
**Date:** 11 October 2026  
**Repository:** https://github.com/jrdntnnr/sardab  
**Reference branch / commit:** `microtonal` / `5c16a252e959b5a19261c580f08cf20ee2a9755e`  
**Platform:** M-VAVE FM-1 hardware UI, browser editor, browser emulator, and public-facing assets  
**Companion document:** `SARDAB_DASTGAH_IMPLEMENTATION_PLAN.md` — music engine, storage and sequencing decisions; **not repeated here**.

> **Naming invariant:** The product name is **SARDAB**, not *SIRDAB*. Persian spelling: **سرداب**. The older SIRDAB design exploration is a visual reference, not the final wordmark.

---

## 1. Design brief

SARDAB should feel like **a subterranean electronic instrument recovered from an architectural archive**: severe, tactile, highly legible, and culturally specific without resorting to fantasy-game decoration or generic “Middle Eastern” patterns.

A usable synthesizer first; an artefact second. Think **excavation plan + analogue test equipment + brutalist editorial typography**. The design language should connect the FM-1 interface to the browser editor without forcing a 240-pixel screen to behave like a website.

### Design principles

1. **Music at the centre.** Pitch, cents, selected track, scale, and current modal state are more prominent than decoration.
2. **Restrained geometry.** Thin datum lines, stair-step shapes, coordinate marks, registration crosses, cutaway diagrams, and rectangular plates. Avoid faux parchment, ornate arabesques, domes, carpets, or sacred iconography.
3. **Use colour sparingly.** Mostly charcoal, bone, warm stone; terracotta/copper for focus and activity. An optional muted verdigris distinguishes musical annotations on the web.
4. **High information density, not tiny type.** Small labels are acceptable only if they remain readable on the physical screen. Never shrink text to fit a decorative frame.
5. **Real interaction states.** Present scale vs. dastgāh, supported vs. planned feature, active vs. merely browsed state, and changed vs. saved data unambiguously.
6. **Keep the FM-1 muscle memory.** Physical button and four-knob mappings must stay discoverable. No hidden gesture should be required for basic performance.
7. **Respect provenance.** Retain appropriate Felucca/GHOULBOX author, licence, attribution and source notices; new identity is not a claim of sole authorship of the underlying firmware.

### Explicitly out of scope

This is a **UI-only** specification. Do not add or change tuning tables, audio/DSP algorithms, dastgāh definitions, note scheduling, MIDI/SysEx command formats, `P_COUNT`, project schemas, samples, instruments, or persistence. UI for a future modal-state model may be **designed and prototyped**, but firmware controls may only be wired to capabilities that actually exist. Any additional capability must be developed under the companion engineering plan.

---

## 2. Confirmed interface constraints

This section reflects the reviewed `microtonal` source; re-check against HEAD at implementation time.

| Surface | Current fact | Design implication |
|---|---|---|
| Hardware display | ST7789-class **240 × 240 px**, **RGB565**, SPI | Design native at 240×240; no subpixel-dependent fine detail |
| Layout | 24 px header; four cards at approximately y=28–72; graph panel y=76–198; 38 px footer at y=202–240 | Preserve the core spatial hierarchy in the first pass |
| Controls | Four parameter knobs; SCL family reached through the existing SCL button, with pages cycled using existing controls | Four visible parameter slots per page; no novel input device |
| SCL pages | `SCL` (ROOT, SCL, QNT, TRANS), `CHORD`, `TUNE 1`, `TUNE 2`, `TUNE 3` | Retain existing parameter names and functionality |
| Existing scale graph | `graph_scale()` displays twelve scale key positions | Enhance emphasis and note information, not invent a new musical data source |
| Palette implementation | Five source colours: `BG`, `SURF`, `TEXT`, `THEME`, `ACCENT`; remaining tokens derived | Add a SARDAB palette **append-only**, through the existing generator |
| Fonts | Three generated UI sizes, existing VT323 / GHOULBOX art | Consider replacement carefully; global font changes affect every page |
| Hardware rendering | Canvas-based partial redraw with render signatures, existing drawing-cost checks | Avoid full-screen redraws, live textures, alpha-heavy animations |
| Web editor | Single-file `web/editor.html`, current tabs and EN/JA strings, CSS custom properties | Refactor gently; do not lose SysEx/editor behavior or mock mode |
| Emulator | `web/emu/index.html` renders the actual 240×240 device canvas | Use it for visual QA, not just static mockups |
| QA baseline | Integration report records **580 UI layout findings and 1,154 alignment findings** plus broader target-cost failures | Distinguish new regressions from pre-existing failures; do not claim clean baseline |

**Source pointers:** `firmware/src/lcd.c`, `gfx.c`, `ui.c`, `ui_draw.c`, `ui_graph.c`, `params.c`, `ui_menu.c`, `tools/gen_ui_palettes.py`, `tools/gen_ghoulbox_art.py`, `web/editor.html`, `web/emu/index.html`, `docs/INTEGRATION-STATUS.md`.

---

## 3. Identity system

### 3.1 Wordmark and bilingual treatment

- Latin master: **SARDAB**, uppercase, custom geometric letterforms; compact and angular rather than distressed medieval blackletter.
- Persian companion: **سرداب**, properly connected and in the correct right-to-left order. The Persian word is **not** letterspaced as independent isolated glyphs.
- Lockup hierarchy:
  - **Primary:** `SARDAB` with `سرداب` directly beneath or aligned at the opposite end of the baseline.
  - **Compact:** `SARDAB` only.
  - **Editorial:** `سرداب` first, `SARDAB` secondary when Persian terminology is the subject.
- Descriptor: `DESERT DUNGEON SYNTH` or, in functional contexts, `MICROTONAL SYNTHESIZER`. Descriptor is subordinate; it is not part of the brand name.
- A mark resembling a **descending stair / section cut / subterranean shaft** may accompany the wordmark, but must be independently recognisable at 16×16, 24×24 and 48×48.
- Do **not** use the current skull or sword as the SARDAB brand mark. Preserve upstream/legacy assets as needed for credits and history.

### 3.2 Geometric vocabulary

**Allowed:** hard orthogonal corners, stepped contours, horizontal rules, annotation numerals, crosshairs, thin-grid construction lines, negative-space arches as sectional geometry, offset frame cuts.

**Avoid:** simulated aged paper over operating controls; dense halftone textures behind small text; blinking flames, animated glyphs, ornamental borders on all controls, generic sci-fi neon, or excessive archaeological text.

### 3.3 Iconography

Use a single 1–1.5 px visual stroke at device scale (or hard-pixel primitives). Default line ends square. Icons must remain interpretable without colour:

| Concept | Proposed motif | Rule |
|---|---|---|
| Dastgāh / mode | Three nested right-angle bands | Never imply a note is active through icon alone |
| Tuning / cents | Vertical datum with ± notches | Expose signed cent value in text |
| Root / tonic | Solid diamond on pitch rail | Unique marker shape for root |
| Altered note | Half-filled square / split diamond | Use both a marker and a `+/-ct` label |
| Modal state | Two offset nested bands | States displayed only when data/capability exists |
| Return / forud | Bent return line | Label `FORUD`; icon is supplementary |
| User samples | Short waveform bounded by plates | Keep existing sample-edit interactions |
| Warning | Outlined square with `!` | Don't borrow the copper active marker for errors |

**Asset budget:** Prefer drawing primitives over embedding many anti-aliased bitmaps. Verify flash increase and binary section sizes with the current build.

---

## 4. Colour system

The earlier visual exploration used near-black, brick/terracotta, sandstone, stone and earth. These become a **functional palette**, with brighter control colours than the poster so text remains legible on RGB565 hardware.

### 4.1 Primary SARDAB Dark palette — exact proposed HEX

| Token | HEX | Usage | Hardware mapping |
|---|---|---|---|
| Obsidian | `#101511` | Main canvas / visual ground | `BG` |
| Deep stone | `#262B25` | Cards, menu panels, graph surface | `SURF` |
| Bone | `#EADFC9` | Main text, note names, values | `TEXT` |
| Sandstone light | `#DFB587` | Stable parameter values, rails, selected graph features | `THEME` |
| Copper active | `#DC916C` | Cursor, hot knob, active step, focused state | `ACCENT` |

**Functional contrast, based on proposed RGB565-quantized source pairs:** Bone on Obsidian ≈14.2:1; Sandstone on Deep stone ≈8.0:1; Copper active on Deep stone ≈6.0:1. These are spot checks, **not** substitutes for the full generated palette audit (derived colours, raised areas, selection fills and monochrome compatibility).

### 4.2 Web-only supporting tones

| Token | HEX | Usage |
|---|---|---|
| Poster terracotta | `#A66143` | Large identity artwork, decorative linework only; **not** normal small text on dark surfaces |
| Oxidised copper / verdigris | `#89A89B` | Modal-note annotation and tertiary data-series accent; must also use shapes/labels |
| Muted bone | `#B0A895` | Secondary labels |
| Stone rule | `#4F554C` | Borders, gridlines, dividers (not label text) |
| Near-void | `#0A0D0B` | Art-only recessed negative space |
| Error | `#F08C83` | Validation errors and genuinely destructive states |
| Warning | `#E8C37C` | Warnings / unsaved changes; accompany with text |
| Success | `#9EC7A8` | Verified successful actions, with message |

### 4.3 SARDAB Light — secondary editor theme

A light variant is useful for diagrams, print, and accessibility; it should not become the default identity.

| Token | HEX |
|---|---|
| Background | `#EFE9DA` |
| Surface | `#E4DACA` |
| Primary text | `#20231D` |
| Stable accent | `#614B39` |
| Interactive accent | `#87462C` |
| Rule | `#AEA697` |

Light-mode values are design proposals. **Run WCAG and actual-device colour tests before enabling**. Where AA contrast fails, modify the token, never lower a test threshold.

### 4.4 Colour behavior rules

- `THEME` = stable readable information; `ACCENT` = **the one current active thing**. Preserve the distinction already present in `gfx.c`.
- An altered note should remain visible without colour: split marker plus signed cent amount. It is **not** equivalent to an active note.
- Error red must never look like the copper focus colour; pair it with `ERROR` or equivalent explicit wording.
- Limit simultaneous saturated colours to two within a hardware view.
- Decorative terracotta `#A66143` is intentionally darker than the functional copper. Do not use them interchangeably for selected controls.
- RGB565 conversion may visibly change colour; review actual `tools/gen_ui_palettes.py --report` results and emulator/device screenshots.
- Keep existing palettes and their saved IDs stable. Add `SARDAB` at the **end** of `PALETTES`, regenerate `ui_palettes.h`, and retain `UI_PALETTE_MIGRATE` and the old `CRYPT` ID.
- Default palette on **fresh SARDAB-branded installs** may be SARDAB after existing settings migration tests; never overwrite a returning user's saved palette silently.

### 4.5 Web CSS tokens

Use semantic tokens, not scattered hex literals:

```css
:root[data-brand="sardab"][data-theme="dark"] {
  color-scheme: dark;
  --sd-bg: #101511;
  --sd-surface: #262B25;
  --sd-text: #EADFC9;
  --sd-value: #DFB587;
  --sd-active: #DC916C;
  --sd-muted: #B0A895;
  --sd-rule: #4F554C;
  --sd-annotate: #89A89B;
  --sd-error: #F08C83;
  --sd-warning: #E8C37C;
  --sd-success: #9EC7A8;
}
```

Provide the same names under `[data-theme="light"]`, with accessible light values. Add fallbacks for the current `--bg`, `--fg`, `--dim`, `--line`, `--theme` system; avoid a broad replacement of every style rule in one commit.

---

## 5. Typography

### 5.1 Latin and numbers

- **Brand display:** a bespoke cut/stencil-ish geometric SARDAB lettering system. Build as an SVG master for web and a downsampled monochrome/4-bit alpha image for firmware.
- **Web UI labels:** **Space Grotesk** or equivalent geometrical sans, with fallback system sans. Keep a compact, industrial appearance rather than a conventional SaaS dashboard.
- **Measurement and tables:** **IBM Plex Mono** (or a licensed equivalent), tabular numbers, `0`, `+50`, `−25`, `ct`, MIDI notes and version/build data. Use uppercase labels sparingly.
- **Do not hide tuning precision** behind stylised lettering. All numeric measurements use the monospaced face.

### 5.2 Persian

- **Primary Persian UI font:** a properly shaped, readable family such as **Vazirmatn**; its use and exact licence/asset packaging must be verified during implementation.
- Custom Persian **display** lettering may be angular, but the text must retain correct joining, diacritics when necessary, and a legible small-size fallback.
- Use HTML `lang="fa" dir="rtl"` on individual Persian labels. Keep Latin parameter IDs, signed cents and note names in explicit `dir="ltr"` spans, e.g. `+50 ct` does not reorder visually.
- Do not globally set the editor to RTL unless the entire application is deliberately translated. **Bilingual technical UI is not the same as a Persian-localised application.**
- Device fonts remain ASCII/Latin transliterations in V1. Persian script on the **hardware** is restricted to a prepared logo bitmap if it fits comfortably; full RTL shaping is **not** part of this UI-only project.

### 5.3 Display sizing and fit

- Keep firmware's existing three size roles unless tests show they are insufficient. Their real pixel widths/metrics come from the generated font, not design-tool point sizes.
- Minimum required readable characters on hardware: **selected page, key parameter labels, note root, current scale, signed cents, track**.
- In 57 px cards, abbreviate deliberately (`ROOT`, `SCL`, `QNT`, `TRN`) rather than ellipsise changing values.
- All text that conveys a status must survive the existing `tests/ui_render` fit and alignment hooks; render long scale names and the widest negative cent value `−63`.

---

## 6. Hardware UI — screen architecture

### 6.1 Preserve the native structure

The redesign may alter colours, frames, ornament and graph treatment but must retain predictable device layout:

```text
240 × 240 display  (schematic; not pixel-perfect type metrics)
┌─────────────────────────────────────────┐  y=0
│ TRACK 1  / 120 BPM            STATUS    │  header: 24 px
├─────────┬─────────┬─────────┬───────────┤  y≈28
│ ROOT    │ SCL     │ QNT     │ TRN       │
│   D     │ BAYATI  │ SNAP    │  +0       │  four ~57 px cards
├─────────┴─────────┴─────────┴───────────┤  y≈76
│                                         │
│             SCALE / MODE               │  graph panel: 122 px
│             (one focused chart)         │
│                                         │
├─────────────────────────────────────────┤  y≈202
│  transport/step hints     TRACK / PAGE  │  footer: 38 px
└─────────────────────────────────────────┘  y=240
```

**Do not substitute the earlier 16:9 promotional mockup as the hardware layout.** Hardware work must use square 240×240 captures and the emulator canvas.

### 6.2 Default performance / HOME

**Objective:** Immediate track, sound and performance context.

- Top header: selected track, BPM, recording/USB/battery status as supported. Preserve existing indicators.
- Four cards: current knob labels/values, with focused copper accent on the knob most recently touched (existing behavior). No random “animated electronics” purely for appearance.
- Centre graph: existing oscilloscope / tracks / levels choices continue working; optional architectural rail/graticule is drawn with subdued `LINE` tokens and never obscures the scope.
- Footer: engine / preset / current page hierarchy; truncate only subordinate descriptions.
- Typography and shapes can shed sword, torch and stone-grain decorations gradually, in favor of plates/datum lines; treat this as a **system-wide visual migration**, not isolated SCL screens.

### 6.3 Existing SCL — functional, shippable now

**Current knobs:** `K1 ROOT`, `K2 SCL`, `K3 QNT`, `K4 TRANS`.

Retain exact parameter wiring. Enhance the scale graph to show at a glance:

- All **12 chromatic identities** in one octave, clearly split into active/inactive notes.
- **Root** as solid copper diamond or equivalent uniquely shaped marker.
- **In-scale** notes as sandstone marks; **out-of-scale** notes as dim stubs.
- **Altered pitch** as split mark and, on focused selection, a signed cent label. Show **factory+trim total only if already derivable from the active state**; otherwise label the two sources separately.
- Bottom graph legend: a few labels such as `ROOT D · BAYATI` and `E −50 ct`, only when values can be computed from the device.
- Do not attempt to print twelve cent labels simultaneously on a 220 px stage. Use selected/focused note detail and existing `TUNE 1–3` pages for full numeric access.
- A Persian name may be displayed in the **web** alongside a scale row when authoritative metadata exists, but the hardware SCL name stays within current ASCII descriptor budget.

**SCL wireframe:**

```text
┌───────────────────────────────────────┐
│ T1  SCL                     120 BPM   │
├────────┬─────────┬─────────┬──────────┤
│ ROOT   │ SCL     │ QNT     │ TRN      │
│   D    │ BAYATI  │ SNAP    │   0      │
├────────┴─────────┴─────────┴──────────┤
│ SCALE                                 │
│   ◆  ──■──·──■──■──·──■──·──■──■──■   │
│   D  E♭ ...        (actual 12 marks)  │
│                                       │
│   ALTERED  E  −50 ct                 │
├───────────────────────────────────────┤
│ [step indicators]          SCL  1/5  │
└───────────────────────────────────────┘
```

**Note:** glyphs in this wireframe are **conceptual**. Use built-in font-compatible characters/icons, not literal `◆` or `♭` unless confirmed in the firmware font. Avoid claiming a particular note alteration without reading live scale and trim data.

### 6.4 TUNE 1 / TUNE 2 / TUNE 3

Retain the existing four-parameter-per-page cents workflow (twelve trims total):

- Show each degree/key identity, signed cent adjustment and its **relationship to a neutral 0**; do not round away the sign.
- Use short horizontal mini-bars through a centre zero datum for each of the four values. Negative extends left, positive extends right.
- Positive vs negative should differ in direction and `+`/`−`, not just colour.
- If a displayed value is only a per-track trim, caption `TRIM` rather than misleadingly calling it total pitch deviation.
- A firmware UI-only change must **not** modify the cent range, root mapping or tuning calculation.

### 6.5 CHORD

- Preserve chord name, degree/voicing and the existing in-scale/tuned indication.
- Use a single horizontal pitch rail consistent with SCL, with chord tones marked by blocks and an optional root diamond.
- Do not label diatonic chord controls as traditional dastgāh harmony. The feature is a **creative chord utility**.
- Selected voicing must be readable with no reliance on small colour changes.

### 6.6 Future MODE and STATE screens — feature gated

These screens become active **only after the companion engineering project exposes authoritative modal records and state selection**. Until then, use design mocks or a web-only **Reference mode** with a clearly marked non-playing preview; never show a fake editable state selector on the device.

**MODE page proposal (future):**

| Knob | Field | Meaning |
|---|---|---|
| K1 | `MODE` | Dastgāh or āvāz selection from actual available records |
| K2 | `STATE` | Current playable state, not an arbitrary unsupported gusheh name |
| K3 | `ROOT` | The same live pitch reference exposed by the tuning engine |
| K4 | `APPLY` / context | Only if corresponding operation exists; otherwise preserve a useful read-only data field |

**State chart proposal:**

- Header: `SHUR` with `شور` in the editor; hardware name `SHUR`.
- Underlying pitch rail: present degree positions.
- Special marks: root, shāhed (focal tone), ist (resting tone), forud destination — **only where modal metadata actually provides them**.
- Distinguish `ACTIVE` from `REFERENCE/PREVIEW` explicitly.
- Show `UNMAPPED` for a manually edited or ambiguous scale rather than inventing a dastgāh identity.
- Source/citation and tuning limitations live in web details, not crowded into the device chart.

**Future navigation constraints:** If a `MODE` or `STATE` page is added, append it after current SCL pages, avoid rearranging SCL/CHORD/TUNE 1–3, and test button cycling, held layers, menu return and four-knob mapping. The page additions are blocked until their parameters/protocol exist.

### 6.7 Menus, lists, ABOUT and identity assets

- Replace GHOULBOX-only visual branding on **SARDAB product surfaces** where explicitly approved, but keep recognisable third-party attribution and historical credits in `ABOUT`.
- Replace sword selection cursor with a **copper bracket or slim stepped index marker**. Highlight selected text; retain unmistakable focus in MONO mode.
- Treat a menu as an instrument calibration list, not an ornate RPG dialogue panel: rectangular panels, compact rows, shallow dividers, no fake stone/torch animation.
- `ABOUT` should display `SARDAB`, firmware identifier/version, upstream Felucca and GHOULBOX provenance, licence, credits, build date and source QR/link as applicable.
- If branded artwork increases flash cost, prefer the compact ASCII wordmark and staged introduction of the Persian glyph bitmap rather than dropping safety/UI functionality.

---

## 7. Browser editor — page structure

### 7.1 Editorial layout

**Target:** A genuinely designed music workstation interface, not a marketing landing page and not the old form with merely recoloured buttons.

- Desktop (roughly ≥1100 px): narrow left instrument rail; generous main editing stage; right contextual inspection rail.
- Tablet (760–1099 px): main stage and collapsible inspector, instrument rail becomes a horizontal strip.
- Mobile (<760 px): single column, track/status always easy to locate, horizontal tabs if needed; no 3-column layout squeezed into phone width.
- Background: Obsidian. In the header and side rails, use small catalogue-style annotation and coordinates sparingly, e.g. `SD—01`, `TRACK 01`, `TUNING / 001`.
- Interactions are **native UI controls** with high-fidelity states; no nonfunctional Three.js effects in the editor. Geometric decorations may use SVG or CSS and must be strictly secondary.

**Desktop hierarchy:**

```text
┌────────────────────────────────────────────────────────────────────────────┐
│ SARDAB  سرداب     PROJECT / 001           USB: CONNECTED      120 BPM     │
├──────────────┬───────────────────────────────────┬─────────────────────────┤
│ TRACKS       │ TUNING / MODE                     │ INSPECTOR               │
│ 01  OUD      │                                   │                         │
│ 02  FM PAD   │   MODAL PITCH DIAGRAM             │ SHUR / شور              │
│ 03  NEY      │   [12-key rail or scale chart]    │ ROOT: D                 │
│ 04  DRUM     │                                   │ DEGREE / CT             │
│              │   DEGREE TABLE / AUDIO PREVIEW   │ FOCAL / CADENCE (later) │
├──────────────┴───────────────────────────────────┴─────────────────────────┤
│ TRANSPORT / STEP STRIP       DIRTY · SAVE / UPLOAD STATUS                  │
└────────────────────────────────────────────────────────────────────────────┘
```

**Use existing controls and capabilities** inside this layout. The diagram is an inspection and authoring tool, not a separate tuning computation engine.

### 7.2 Navigation and information architecture

**UI milestone A (works with today's firmware):** Refine existing tabs without changing their meaning. Give SCL a **Tuning workspace** within existing sound/parameter navigation that exposes root, scale, quantise, transpose and trim values. Keep all existing editor actions and mock-mode behavior.

**UI milestone B (web-side reference data):** Add a distinct **Modes** subview or tab when `assets/dastgah.json` is reviewed. It can compare known modal references and preview static metadata with a clear label such as `REFERENCE ONLY — NOT APPLIED TO DEVICE`. Avoid communicating that selecting a catalogue row changes device tuning unless it really does.

**UI milestone C (live modal features):** Only after firmware support arrives, promote the Modes view to an operational editor and include an explicit apply interaction with success/failure/dirty state.

**Compatibility rules:** Keep existing `sound`, `fm6`, `sequencer`, `tracks`, `library`, `samples`, `projects`, `settings` flows intact. The editor currently remembers tab selection in URL/local storage, supports mock device and derives descriptor details from hardware. Avoid hardcoded page assumptions; test fallback to the previous layout if new modal capabilities are not advertised.

### 7.3 Tuning editor

**Core controls (all currently grounded):**

- Track switcher 1–4, with mute/record state as available.
- Root note selector.
- Scale selector with current scale name, compact category and any available tuning annotations.
- Quantization and transpose controls.
- Twelve-row cent-trim table, preserving 12 distinct MIDI key identities and current ±63-cent range.
- **Audition keyboard / preview** only through an existing supported sound pathway; if not available, mark it as design-only and do not promise playback.
- `CUSTOM` scale editing and Scala import must retain their existing, explicit save/upload semantics.

**Pitch diagram:** horizontal 0–1200-cent octave rail (axis labels are actual cents), plotted note points, relative-note labels and membership. Root = diamond. Altered = split/outline marker. A focused note exposes base cents, per-track trim and resulting pitch offset **if all values are available**. Use tooltips on desktop and tap-to-open detail on mobile.

**Data table:** Columns: degree/key identity, MIDI base pitch, factory offset (when known), trim, resulting cents, in-scale status. Use `—` for unavailable data, never silently zero-fill. Preserve negative signs. Show tuning values as *reference* versus *device state* as separate concepts.

**Validation:** range errors, duplicate mappings, unsupported period, metadata lacking references, failed upload, stale device status, and unsaved modifications all have distinct text responses. Never colour an error as merely copper focus.

### 7.4 Dastgāh library (future metadata-dependent)

A reference catalogue for Shur and subsequent reviewed entries. Do not invent the complete seven-dastgāh catalogue in the UI merely for symmetry.

**List:** Persian name, Latin transliteration, category (`DASTGĀH` / `ĀVĀZ`), number of documented states, status (`SOURCE NEEDED`, `REVIEWED`, `EXPERIMENTAL`, `PLAYABLE`) and citations in detail panel.

**Detail:** Modal pitch hierarchy, shāhed/ist/forud tags if supported by sources, variant and state comparison, melodic notes, tuning source and revision. Explain that pitch collections are **interpretations**, not canonical universal quarter-tone recipes.

**Actions:** `COMPARE`, `AUDITION` (only with real sound), `APPLY TO TRACK` (only with firmware support), `COPY AS CUSTOM` (only if existing APIs safely express it). No inert action buttons.

### 7.5 Samples and instrument presets

The new desert instruments (DESERT OUD, BRASS QANUN, DUNE NEY; optional Oud/Qanun/Ney Air sample banks) should appear with correct **engine versus recording** distinctions:

- **Synthesized preset:** built into the firmware, no sample upload necessary.
- **Recorded bank:** optional, lives in a user sample slot and replaces that slot when uploaded.
- Clear capacity/use and confirmation states, with backup reminder before overwriting user slots.
- Do not hide the existing WAV preview/trim waveform functionality behind an ornamental UI.

### 7.6 States and affordances

Every meaningful editor operation must define at least:

| State | Appearance | Required content |
|---|---|---|
| Disconnected | Dim header indicator, controls disabled only if device-dependent | `CONNECT FM-1`, mock/demo link as supported |
| Connected | Verdigris dot **and** text | Device name/version when available |
| Busy | Stable progress / spinner with text | What is loading or uploading |
| Unsaved changes | Copper/warning marker **and** text | `UNSAVED`, scope of changes |
| Read-only reference | Distinct outlined label | `REFERENCE ONLY`, no misleading Apply |
| Unsupported device | Neutral warning | Required firmware capability; fallback path |
| Error | Error tone, icon and actionable copy | Reason + retry/recovery where safe |
| Success | Small confirmation, not modal interruption | What changed and where |

Use visible keyboard focus and distinguish selection from hover. Do not make any hardware write happen on hover or mere catalogue browsing.

---

## 8. Components and spacing

### 8.1 Shared component vocabulary

- **Rail:** one-pixel datum line with numeric ticks; used in pitch and time graphs, no gratuitous frame.
- **Plate:** rectangular control surface with thin border or shallow filled plane; radius 0–2 px.
- **Index marker:** copper bracket/square; denotes selected item.
- **Register:** small code such as `SD/001`, used for page or section index (never replaces real names).
- **Measurement:** monospace value with visible signed units (`−50 ct`).
- **Annotation:** quieter supporting information near a diagram or under a control.
- **Inspector:** fixed region showing details of active selection.

### 8.2 Web dimensions (design targets)

- Spacing system: **4 / 8 / 12 / 16 / 24 / 32 / 48 px**.
- Major workspace rail: about 192–232 px; inspector: 260–320 px at desktop; remaining width flexible.
- Form control height: 36–40 px desktop. Tap target at least **44×44 CSS px** for touch.
- Main body line-height: around 1.4–1.55; Latin operational text ≥13–14 px. Persian body generally ≥15–16 px, subject to glyph rendering tests.
- Dividers: 1 px; very low-contrast rules are for grouping only, never primary state encoding.
- Header is mostly typography and status, not a hero illustration. Reserve the giant bilingual wordmark for landing/ABOUT/empty state rather than consuming permanent edit workspace.
- No more than two primary buttons in a detail panel. `Apply` is visually distinct from `Preview` and `Export`.

---

## 9. Motion, interaction and accessibility

- **Hardware:** default static visual style. Limit changes to existing knob focus transitions and necessary metering. No pulsing ornament behind scales. Partial redraw only; do not add graph animation that churns every frame.
- **Web:** transitions 100–160 ms for hover/focus/panel change, with `prefers-reduced-motion: reduce` disabling them. Avoid scroll parallax and decorative audio-reactive distortions.
- **Accessibility:** minimum WCAG AA 4.5:1 for ordinary text and 3:1 for essential graphics where applicable. The full firmware palette generator has **stricter, specific checks**; do not weaken those checks.
- **Keyboard:** tab order follows the operational workflow, focus ring always visible, arrow-key interactions documented and not hijacked globally.
- **Screen readers:** label track, scale, current cent value, changed status and write operation. Chart includes a textual/table alternative. RTL name and LTR numeric fields are explicitly annotated.
- **Zoom:** browser editor remains operable at 200% zoom on desktop and on a 390-pixel phone viewport.
- **Contrast modes:** test original `GREY`, `HI-CON`, `MONO` device palettes against any new glyphs or architectural borders. Shape, underline and text must carry state even without copper.

---

## 10. Engineering map — specific files

| File | UI-only changes | Constraints |
|---|---|---|
| `tools/gen_ui_palettes.py` | Append the SARDAB five-colour palette; run contrast report | Do not change old palette IDs or thresholds |
| `firmware/src/gfx.c` | Use existing semantic colour tokens; small optional datum/mark primitives | Preserve RGB565 and partial redraw cost |
| `firmware/src/ui_draw.c` | Retheme header/cards/footer, selected/hot states | Keep y bands and knob mappings |
| `firmware/src/ui_graph.c` | Redraw `graph_scale()` marks, focused note/cent labels, chord graph styling | Must derive all data from existing actual values; avoid changes to scale maths |
| `firmware/src/ui_menu.c` | New rectangular menu/list aesthetic, selected-row bracket, ABOUT branding | Keep credits/QR paths and navigation |
| `firmware/src/ui.c` | UI page titles/styling only where justified | **No new parameter IDs** in this UI-only task |
| `firmware/src/params.c` | Generally **no change**; verify displayed labels | Do not alter parameter arrays/page schema without matching feature plan |
| `tools/gen_ghoulbox_art.py` or successor | Generate separate SARDAB logo/shaft assets and compact device bitmaps | Preserve historical source/licences; avoid destructive overwrite |
| `tools/gen_aa_font.py` | Optional later font/bitmap generation support | No speculative global font replacement in UI PR1 |
| `assets/fonts/`, `LICENSING.md` | Add licence-verified font references if packaged | Respect font licences; omit unused fonts from builds |
| `web/editor.html` | Design tokens, responsive shell, tuning workspace, captions and state/selection UI | Keep protocol, old tabs, mock mode and sample editing |
| `web/emu/index.html` | Optional SARDAB wrapper/header while keeping actual emulated display | Preserve 240×240 native canvas and device input behavior |
| `web/make_site.py` | Package new styles/assets if needed | Check base paths in generated Pages site |
| `tests/ui_render.c`, `tests/ui_render.py` | Add SARDAB palette/screen captures, pixel/lint comparisons | Record existing failures; no unexpected growth |
| `tests/ui_test.c`, `web/test_web.mjs` | Page order, labels, palette compatibility, responsive/mode fallback | No false positives based on static mocks |
| `docs/` | Screenshots, visual tokens, UI reference guide and attribution | User guide updated only for features that exist |

**Build note:** Generated `ui_palettes.h` and artwork headers should be produced through the existing generator/build pipeline, not maintained by hand. Confirm their exact output locations in the active build before editing.

---

## 11. UI-only pull request sequence

### UI-0 — Baseline capture and design-token approval

- [ ] Record current HEAD, build size, display metrics and existing UI lint/alignment failures.
- [ ] Capture the existing SCL, CHORD, TUNE, HOME, menu, ABOUT and web editor states in emulator/browser.
- [ ] Render exact proposed palette in RGB565, run generator contrast audit and adjust colours if needed.
- [ ] Approve three core visuals: HOME, SCL, web tuning workspace (including RTL Persian name).
- [ ] Freeze wordmark spelling **SARDAB / سرداب**; no remaining accidental `SIRDAB` strings in new brand assets.

**Deliverable:** approved palette and screenshot baseline; no audio/data changes.

### UI-1 — Non-destructive visual identity / palette

- [ ] Append `SARDAB` to the firmware palette generator and regenerate source.
- [ ] Implement CSS tokens, typography and responsive editor shell.
- [ ] Add approved SARDAB wordmark as web SVG; generate measured small bitmap for device ABOUT/boot only if memory allows.
- [ ] Preserve old themes, saved palette IDs, protocol paths and source attribution.
- [ ] Verify hardware colour contrast and MONO/GREY/HI-CON parity.

**Gate:** existing UI workflow unaffected, display redraw and flash deltas recorded, web mock mode works.

### UI-2 — Performance and tuning pages

- [ ] Retheme the device HOME/menus/cards/footer while keeping established layout.
- [ ] Redesign SCL twelve-note graph and root/alteration iconography.
- [ ] Improve TUNE 1–3 signed trim presentation and CHORD readability.
- [ ] Implement device and browser screenshots for long names, max/min trims and all four tracks.
- [ ] Add the tuning workspace in the editor using **existing** ROOT/SCL/QNT/TRN/trim APIs.

**Gate:** all existing controls and scales still work; UI audit shows no new unwaived clipping or misalignment.

### UI-3 — Reference mode library (UI / content only)

- [ ] Design and render web-only Shur reference page against reviewed static metadata (if available).
- [ ] Distinguish reference metadata from device state with explicit badge/text.
- [ ] Prepare future MODE/STATE screen assets but do **not** register inactive firmware controls.
- [ ] Implement bilingual type, LTR signed-cent values and responsive modal diagrams.

**Gate:** no action suggests a device state change when none occurred; no tuning-engine changes.

### UI-4 — Integration after music-engine rollout (separate dependency)

- [ ] When firmware capabilities exist, add hardware `MODE`/`STATE` pages behind capability tests.
- [ ] Add real `APPLY TO TRACK`, live state indicators and failure reconciliation in web UI.
- [ ] Test device/editor synchronization, old firmware fallback and edited scale ambiguity.
- [ ] Ship only alongside the companion musical feature's validated acceptance tests.

**Gate:** UI cannot claim support absent in the device; no UI-only PR is required to deliver features that need new storage/protocol.

---

## 12. Acceptance and QA checklist

### Hardware

- [ ] Exact display-size screenshots at **240×240 px**, not presentation mockups.
- [ ] The root, chosen scale, quantisation and transpose always remain visible and correctly bound.
- [ ] Longest existing scale labels and new names fit or use a deliberate documented abbreviation.
- [ ] Twelve-note scale marks are distinguishable; focused altered notes show a signed cent value.
- [ ] Device SCL→CHORD→TUNE 1→TUNE 2→TUNE 3 navigation preserved, unless independently approved later.
- [ ] All four knobs control the same parameters as before; no visual cue contradicts the assignment.
- [ ] All palettes still render; SARDAB palette can be selected and saved; legacy palette settings migrate identically.
- [ ] Colour contrast generator remains green; `MONO` and `HI-CON` symbols remain understandable.
- [ ] No new overlap/clipping findings; known baseline failures itemised separately, no threshold weakening.
- [ ] Full redraw and partial redraw costs measured; flash/RAM/image headroom documented.
- [ ] Physical display legibility checked on an FM-1 at typical viewing distance **before** release claim.

### Web

- [ ] Browser layouts inspected at desktop 1440 px, 1024 px, 768 px, and phone 390 px.
- [ ] Persian **سرداب** and **شور** are correctly joined and ordered; cent values remain LTR.
- [ ] No essential functionality depends on colour alone or hover alone.
- [ ] Root and cent-edit controls mirror connected device values and show data provenance or unavailable state accurately.
- [ ] `CUSTOM`, `.scl` import, sample preparation/upload, backup and projects still work.
- [ ] Disconnect/reconnect, stale values, upload failure, unsaved state and unsupported-version fallbacks are clear.
- [ ] No inert buttons or faux “Apply” actions in reference-only screens.
- [ ] Keyboard navigation and 200% zoom do not break essential workflows.
- [ ] Captures tested in actual current Chrome/Safari where Web MIDI support differs; no false connection claims.

### Recommended commands

```bash
# Run from the repository root. Verify exact scripts/options on your branch.
python3 tools/gen_ui_palettes.py --report
sh tests/run_tuning_tests.sh
sh tests/run_tests.sh
node web/test_web.mjs
# Compare emulator/device screenshots and tests/ui_render audit artifacts.
```

Any pre-existing red result should be captured in the PR description with **baseline vs current**, rather than quietly adjusted or ignored.

---

## 13. Deliverables and sign-off

1. **Design tokens:** exact HEX, RGB565 verification, CSS mapping and existing-palette compatibility.
2. **Artwork:** Latin/Persian wordmarks, logo mark, icon primitives, licence/attribution manifest.
3. **Hardware visual design:** pixel-native HOME, SCL, TUNE, CHORD, menus, ABOUT and high-contrast variants.
4. **Web visual design:** responsive desktop/mobile editor, pitch-rail/table components, reference mode detail, explicit operational states.
5. **Working UI implementation:** tested C and HTML/CSS/JS only; no silent protocol or audio changes.
6. **QA pack:** screenshot comparisons, contrast findings, UI lint deltas, flash and draw-cost deltas, workflow test results.
7. **Documentation:** concise style guide, UI conventions, screenshots and changes to displayed names/captions.

**Final decision points for product owner:** approve exact wordmark shape; accept the recommended dark/copper palette after device viewing; choose whether SARDAB becomes the fresh-install default theme or an optional skin; approve the amount of legacy RPG chrome replaced; approve the browser workspace layout. These are design decisions, **not prerequisites to drafting the UI**.

---

## 14. Codex handoff / implementation instruction

> Work on `jrdntnnr/sardab`, branch `microtonal`, starting by confirming the current HEAD and reading `SARDAB_UI_SPEC.md` and `SARDAB_DASTGAH_IMPLEMENTATION_PLAN.md`. Implement **UI-0 and UI-1 only** first. Do not alter DSP, tuning data, scale IDs, project serialization, MIDI/SysEx, sound presets or user samples. Preserve stored palette identities, existing menu/page navigation, credits and licensing. Reuse `tools/gen_ui_palettes.py`, firmware token names and the real 240×240 emulator. Generate screenshots, verify RGB565 contrast and old palette parity, record flash/draw-cost changes, run UI/web regression tests, and stop with a PR for visual sign-off before redesigning SCL/TUNE screens. Never enable MODE/STATE device controls without their real supporting capabilities. The definitive spelling is `SARDAB / سرداب`.

**Implementation order:** palette/brand → existing SCL/TUNE/CHORD/HOME → web tuning workspace → reference UI → future real modal-state integration.
