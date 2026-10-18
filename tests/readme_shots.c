/* SPDX-License-Identifier: GPL-3.0-only */
/* The README's strip (docs/sardab-2.3.png): boot, HOME on DESERT OUD, PRESETS, MENU in the SARDAB palette, from the
 * real drawing code on tests/ui_test.c's stubs. Writes OUTDIR/{boot,home,presets,menu}.ppm (240 x 240); the strip is
 * the four at 2x on a dark ground, 16 px apart (tools/render_readme.py).
 *   cc -O1 -w '-DFELUCCA_VERSION="SARDAB 2.3"' -Ibuild/gen -Ifirmware/src -Itests -o build/host/readme_shots tests/readme_shots.c -lm
 *   build/host/readme_shots build/shots
 *   python3 tools/render_readme.py build/shots */
#define UI_TEST_NO_MAIN 1
#include "ui_test.c"

static void ppm(const char *path)
{
    uint32_t i;
    FILE *f = fopen(path, "wb");
    fprintf(f, "P6\n240 240\n255\n");
    for (i = 0; i < 240u * 240u; i++) {
        uint16_t c = swap16(host_screen[i]);
        uint8_t b[3] = {(uint8_t)((c >> 11) * 255u / 31u), (uint8_t)(((c >> 5) & 63u) * 255u / 63u), (uint8_t)((c & 31u) * 255u / 31u)};
        fwrite(b, 1, 3, f);
    }
    fclose(f);
}

static uint32_t preset_named(const engine_t *e, const char *name)
{
    uint32_t i;
    for (i = 0; i < e->npresets; i++)
        if (!strcmp(e->presets[i].name, name))
            return i;
    return 0;
}

/* a block of the mix, into the scope as audio.c audio_block does (SCOPE OUT) */
static void scope_block(int32_t *o)
{
    uint32_t i;
    mix_block(o, CTL);
    for (i = 1; i < CTL; i += 2u)
        scope_buf[scope_w++ & (SCOPE_N - 1u)] = (int16_t)o[2u * i];
}

int main(int argc, char **argv)
{
    static int32_t o[2 * CTL];
    const char *dir = argc > 1 ? argv[1] : ".";
    char path[512];
    uint32_t b;

    ui_power_on();
    settings.palette = UI_SARDAB_INDEX; palette_set(UI_SARDAB_INDEX);

    /* The actual shared hardware/emulator splash. */
    draw_splash();
    snprintf(path, sizeof path, "%s/boot.ppm", dir); ppm(path);

    /* HOME: DESERT OUD on track 1, a note sounding for the scope */
    host_tracks_init();
    ui_power_on();
    song.master_q12 = 4096;
    settings.palette = UI_SARDAB_INDEX; palette_set(UI_SARDAB_INDEX);
    host_preset(&trk[0], ENGI_PHYS, preset_named(&ENG_PHYS, "DESERT OUD"));
    song.sel = 0;
    song.playing = 1;
    song.g[G_BPM] = 96;
    trk_note_on(&trk[0], 62, 110);
    for (b = 0; b < 40u; b++) scope_block(o);
    trk_note_on(&trk[0], 69, 100);
    for (b = 0; b < 30u; b++) scope_block(o);
    frame(); frame();
    snprintf(path, sizeof path, "%s/home.ppm", dir); ppm(path);

    /* PRESETS at DESERT OUD in the DESERT section */
    list_set(1u + CAT_DESERT);
    go_page(GR_BROWSE);
    frame(); frame();
    snprintf(path, sizeof path, "%s/presets.ppm", dir); ppm(path);

    /* MENU */
    go_home();
    ui.menu = 1; ui.menu_sel = 0;
    frame(); frame();
    snprintf(path, sizeof path, "%s/menu.ppm", dir); ppm(path);
    return 0;
}
