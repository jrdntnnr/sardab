/* SPDX-License-Identifier: GPL-3.0-only */
/* Actual firmware drawing: pitch readouts, note lifecycle, sample upload refresh. */
#define UI_RENDER_NO_MAIN 1
#include "ui_render.c"
#include <assert.h>

static int shown(const char *text)
{
    for (uint32_t i = 0; i < nscr; i++)
        if (!strcmp(scr[i].s, text)) return 1;
    return 0;
}

static void redraw_graph(void)
{
    nscr = npend = 0;
    ui.force = 0;
    draw_graph();
}

/* Partial redraws must retain the exact same pattern, including clipped edges
 * and graph offsets. Otherwise the background crawls when a value changes. */
static void check_pattern_redraw(void)
{
    uint16_t full[37 * 19];
    setup(S_RAST); pal(UI_SARDAB_INDEX);
    cv_oy = 3;
    cv_begin(37, 19, T_BG);
    cv_stone(-2, -5, 42, 28, 0, 0);
    memcpy(full, cv_px, sizeof full);
    cv_begin(37, 19, T_BG);
    for (int x = -2; x < 40; x += 7)
        for (int y = -5; y < 23; y += 4)
            cv_stone(x, y, 7, 4, 0, 0);
    assert(!memcmp(full, cv_px, sizeof full));
    unsigned specks = 0;
    for (unsigned i = 0; i < 37 * 19; i++) specks += full[i] == swap16(T_STONE);
    assert(specks > 0 && specks < 37 * 19 / 8); /* sparse, visible, deterministic */
    cv_oy = 0;
}

int main(void)
{
    check_pattern_redraw();
    for (uint32_t p = 0; p < NPALETTES; p++) {
        uint32_t before = nfind;
        setup(S_SPLASH); pal(p); draw(S_SPLASH);
        assert(nfind == before);
        assert(shown("سرداب") && shown("DUNGEON SYNTH"));
    }
    static const char *const third[12] = {"E", "F", "F#", "G", "G#", "A", "A#", "B", "C", "C#", "D", "D#"};
    for (uint32_t root = 0; root < 12; root++) {
        char want[24];
        setup(S_TUNE2); pal(UI_SARDAB_INDEX);
        TSEL->p[P_ROOT] = root;
        snprintf(want, sizeof want, "%s -37 ct", third[root]);
        draw(S_TUNE2);
        assert(shown(want));                         /* effective preset + trim, transposed label */
        assert(shown(third[root]));                  /* knob still edits its original relative trim */
    }
    setup(S_TUNE_TOTAL); pal(UI_SARDAB_INDEX); draw(S_TUNE_TOTAL);
    assert(shown("F# -126.7 ct"));                   /* temperament precision retained */

    setup(S_RAST); pal(UI_SARDAB_INDEX);
    TSEL->p[P_VOICE] = V_POLY;
    TSEL->engine = TSEL->eng_req;
    memset(TSEL->v, 0, sizeof TSEL->v);
    trk_note_on(TSEL, 60, 100); trk_note_on(TSEL, 67, 100); trk_note_on(TSEL, 64, 100);
    draw(S_RAST);
    assert(shown("E4 -50 ct") && shown("PLAY"));
    assert((scale_playing(TSEL) & 4095u) == ((1u << 0) | (1u << 4) | (1u << 7)));
    for (uint32_t i = 0; i < 12; i++) {
        uint16_t pixel = host_screen[(graph_y() + GOY + 84) * 240u + 20 + i * 18];
        assert((pixel == swap16(T_ACCENT)) == (i == 0 || i == 4 || i == 7));
    }
    trk_note_off(TSEL, 64);
    redraw_graph(); assert(shown("G4 +0 ct"));       /* gate release redraws without UI force */
    TSEL->p[P_MUTE] = 1;
    redraw_graph(); assert(shown("C +0 ct") && shown("ROOT"));

    setup(S_DESERT_MISSING); pal(UI_SARDAB_INDEX); draw(S_DESERT_MISSING);
    assert(shown("LOADED") && shown("USR1 MISSING: SINE") && shown("NO SAMPLE"));
    {
        smp_user_hdr_t *h = (smp_user_hdr_t *)host_slots;
        h->magic = SMP_USER_MAGIC; h->version = 1; h->nz = 1; h->data_len = 2;
        memcpy(h->name, "DUNE TAR", 8);
        h->zone[0] = (smp_zone_t){.n=4, .le=3, .rate=32768, .root16=960, .hi=127};
        smp_user_scan(0);
        redraw_graph(); assert(shown("DUNE TAR") && shown("USR1 READY"));
        h->zone[0].rate = 0;                        /* invalid data must not be called ready */
        smp_user_scan(0);
        redraw_graph(); assert(shown("USR1 MISSING: SINE"));
    }
    setup(S_DESERT_OTHER); pal(UI_SARDAB_INDEX); draw(S_DESERT_OTHER);
    assert(shown("NOT IN LIST") && !shown("LOADED"));
    {
        uint32_t total;
        preset_pos(&total);
        for (uint32_t i = 0; i < total; i++) {
            preset_go(i); ui.msg_t = 0; ui.force = 1;
            draw(S_DESERT);
            assert(shown("LOADED"));
        }
    }
    setup(S_LFO); pal(UI_SARDAB_INDEX); draw(S_LFO); assert(PANEL_W == 220);
    setup(S_LFO); pal(UI_CRYPT_INDEX); draw(S_LFO); assert(PANEL_W == 220);
    puts("desert UI: tuning, live notes, sample refresh, loaded state, torch-free panels and stable Bayer redraws passed");
    return 0;
}
