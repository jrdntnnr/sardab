# SARDAB UI — status

**18 October 2026:** the approved tuning readout, DESERT preset feedback and
SARDAB visual cleanup are implemented and rebuilt. The latest build adds the
[AmbientZero palette and Bayer patterns](BAYER-UI.md), removes torches and verifies
the browser at desktop and phone sizes. See [DESERT UI](DESERT-UI.md)
for current screens, controls and validation. Page navigation and knob assignments
are unchanged. The following records the earlier UI-0/UI-1 work.

Implements stages UI-0 and UI-1 of [SARDAB_UI_SPEC.md](SARDAB_UI_SPEC.md), plus the SCL root marker brought forward from UI-2 (spec 6.3). Waiting for visual sign-off before the rest of UI-2 (redesigning the SCL, TUNE, CHORD and HOME screens). No audio, tuning, scale, project, protocol or preset changes.

## What exists

| Surface | Change | How to see it |
| --- | --- | --- |
| Device | `SARDAB` palette, appended as id 11 (saved ids unchanged; MONO stays 10). Fresh installs and invalid settings start on SARDAB; a saved palette is kept | MENU > COLOR > SARDAB |
| Device | SCL graph marks the root with a diamond over a white key and under a black key, in every palette. Copper and sandstone are close in hue, so the root is cued by shape as well as colour | SCL, TUNE, CHORD screens |
| Web editor | SARDAB tokens mapped onto the editor's five colour variables, copper selected tab and focus ring, bilingual header lockup. SARDAB is the default brand | `editor.html` (`?theme=light` for light; `?brand=ghoulbox` to return). Remembered per browser |
| Fonts | Vazirmatn (UI, tabular digits) and Reem Kufi (headings, Persian lockup). Both OFL, subset to Latin and Arabic script (Persian), 114 KB total, bundled in `web/fonts/` | — |
| Brand assets | Wordmark and mark, `assets/sardab/*.svg`. Persian سرداب is live text, not outlines | — |

![Device: CRYPT above, SARDAB below](sardab-ui/device-crypt-vs-sardab.png)

![Editor, SARDAB dark](sardab-ui/editor-dark-1440.png)

More: [light](sardab-ui/editor-light-1440.png), [390 px phone](sardab-ui/editor-dark-390.png), [wordmark options](sardab-ui/wordmark-options.png).

## Verification

| Check | Baseline | Now |
| --- | --- | --- |
| Palette contrast (`gen_ui_palettes.py --report`) | pass | pass. SARDAB text/bg 14.1, theme/surf 8.0, accent/surf 6.0; no colour adjusted |
| Web token contrast (WCAG) | — | every text token ≥ 4.5:1 on background and surface, dark and light |
| UI lint findings | 580 | 592: SARDAB's copies of the 3 findings every palette has, in 4 styles. None specific to SARDAB |
| Audio goldens | 144 | 144, 0 changed |
| Palette / settings / text / editor tests | pass | pass (tests now pin MONO at 10 and SARDAB at 11) |
| Editor in mock mode, 1440 / 1024 / 768 / 390 px, both brands | — | no horizontal overflow, no page errors; no `?brand` gives SARDAB |
| Firmware image | 573,400 B | 573,424 B (+24), measured before the root marker |
| Target cost (`target_budget.py`) | ISR +21 %, SLICE absent | unchanged |

Not done: physical-display legibility on an FM-1, Safari, 200 % zoom.

## Decisions

Resolved: free Persian-leaning fonts (Vazirmatn, Reem Kufi); SARDAB as the default on device and editor; root marked by shape.

Wordmark: angular capitals with 45° cuts, from the brand board (option C). Replaces the pixel-grid and Reem Kufi drafts in [wordmark options](sardab-ui/wordmark-options.png).
