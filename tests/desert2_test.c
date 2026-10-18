/* SPDX-License-Identifier: GPL-3.0-only */
/* GHOULBOX desert, the second round (docs/DUNGEONS-OF-THE-DESERT.md):
 *   - CANON (P_CANON, seq.c canon_enter): the second playhead half the pattern behind at the interval, the R modes
 *     backwards, TIE / REST, OFF lets go, a kit never
 *   - ARP DRONE (seq.c arp_drone): the lowest key sounds on, the others arpeggiate over it, its gate, the keys let go
 *   - the delay's tape motor (fx.c DLY_SLEW): a TIME change glides the read point, no jump
 *   - the project: P_CANON in its byte (np 127), an older store's tracks read it 0 (core.h params_by_count)
 *   - MENU > ABOUT's figure in the header (ui_draw.c draw_cpu), PRESETS with a page button held (ui.c fam_turn) */
#define UI_TEST_NO_MAIN 1
#include "ui_test.c"

static int bad;
static void cck(const char *what, int ok)
{
    printf("desert2: %-80s %s\n", what, ok ? "ok" : "FAIL");
    bad += !ok;
}

/* track 1: an 8-step loop in C major, C4 on step 0, E4 on step 2, the rest rests; the transport started */
static track_t *canon_pattern(int32_t canon)
{
    track_t *t = &trk[0];
    ui_power_on();
    track_defaults_steps(t);
    t->p[P_SLEN] = 8;
    t->p[P_SGATE] = 64;
    t->p[P_SSWING] = 0;
    t->p[P_ROOT] = 0;
    t->p[P_SCALE] = 1;                                  /* MAJ */
    t->p[P_VOICE] = V_POLY;
    t->p[P_CANON] = (int16_t)canon;
    song.g[G_SWING] = 0;
    t->step[0] = (step_t){{60}, 1, ST_NOTE, 0, 100};
    t->step[2] = (step_t){{64}, 1, ST_NOTE, 0, 100};
    seq_start();
    return t;
}

/* a bar of the loop: the canon's notes as they start (one a step at most), and the step each started on */
static uint32_t canon_bar(track_t *t, uint8_t *notes, uint8_t *steps, uint32_t max)
{
    uint32_t b, n = 0, blocks = 8u * step_samples(t, div_samples((uint32_t)t->p[P_SDIV]), 0) / CTL, last = 0;
    for (b = 0; b < blocks; b++) {
        seq_tick(t, CTL);
        if (t->cn_n && t->cn_notes[0] != last && n < max) {
            notes[n] = t->cn_notes[0];
            steps[n++] = (uint8_t)t->seq_idx;
        }
        last = t->cn_n ? t->cn_notes[0] : 0u;
    }
    return n;
}

static void test_canon(void)
{
    uint8_t nn[8], st[8];
    uint32_t n;
    track_t *t = canon_pattern(2);                      /* 5TH */
    n = canon_bar(t, nn, st, 8);
    cck("CANON 5TH: C4 (step 0) and E4 (step 2) answered a fifth up (G4, B4) four steps later",
        n == 2 && nn[0] == 67 && st[0] == 4 && nn[1] == 71 && st[1] == 6);
    t = canon_pattern(10);                              /* R 5TH: the pattern backwards */
    n = canon_bar(t, nn, st, 8);
    cck("CANON R 5TH: backwards, B4 then G4", n == 2 && nn[0] == 71 && nn[1] == 67);
    t = canon_pattern(4);                               /* OCT */
    n = canon_bar(t, nn, st, 8);
    cck("CANON OCT: an octave up (C5, E5)", n == 2 && nn[0] == 72 && nn[1] == 76);
    t = canon_pattern(8);                               /* -OCT */
    n = canon_bar(t, nn, st, 8);
    cck("CANON -OCT: an octave down (C3, E3)", n == 2 && nn[0] == 48 && nn[1] == 52);
    t = canon_pattern(5);                               /* -3RD on a scale with no note two steps below C4 .. */
    n = canon_bar(t, nn, st, 8);
    cck("CANON -3RD: a third down on the scale (A3, C4)", n == 2 && nn[0] == 57 && nn[1] == 60);
    {   /* TIE holds the answer, REST lets it go; OFF while it sounds: it lets go at the next step */
        uint32_t b, held = 0, blocks;
        t = canon_pattern(2);
        t->step[1] = (step_t){{0}, 0, ST_TIE, 0, 0};
        blocks = step_samples(t, div_samples((uint32_t)t->p[P_SDIV]), 0) / CTL;
        for (b = 0; b < 4u * blocks; b++)
            seq_tick(t, CTL);                           /* steps 0..3 */
        for (b = 0; b < 2u * blocks - 2u; b++) {        /* steps 4 (C4's answer) and 5 (the tie) */
            seq_tick(t, CTL);
            held += t->cn_n != 0;
        }
        cck("CANON: a TIE after the note holds its answer through the tied step", held >= 2u * blocks - 4u);
        t->p[P_CANON] = 0;
        for (b = 0; b < 2u * blocks; b++)
            seq_tick(t, CTL);
        cck("CANON OFF while the answer sounds: it lets go at the next step", t->cn_n == 0);
    }
    t = canon_pattern(2);
    host_preset(t, ENGI_DRUM, 0);
    t->p[P_CANON] = 2;
    n = canon_bar(t, nn, st, 8);
    cck("CANON on a drum kit: nothing (the engine maps the keys itself)", n == 0);
}

static void test_drone(void)
{
    static const uint8_t CEG[3] = {60, 64, 67}, D[1] = {62};
    track_t *t = &trk[0];
    uint32_t i, ok = 1;
    ui_power_on();
    t->nheld = 0;
    t->p[P_AMODE] = AM_DRONE; t->p[P_AOCT] = 1; t->p[P_APROB] = 127; t->p[P_AGATE] = 64;
    for (i = 0; i < 3u; i++)
        arp_add(t, CEG[i], 100);
    for (i = 0; i < 4u; i++) {
        t->arp_pos = 0xFFFFFFF;
        arp_tick(t, CTL);
        ok &= t->arp_ch[0] == 60 && t->arp_note == (i & 1u ? 67 : 64);
    }
    cck("ARP DRONE: C4 E4 G4 held: C4 sounds on, E4 G4 E4 G4 arpeggiate over it", ok);
    t->arp_off = 1;
    arp_tick(t, CTL);                                   /* the gate ends */
    cck("ARP DRONE: the gate ends the arpeggiated note, the drone sounds on", t->arp_note == 0 && t->arp_ch[0] == 60);
    t->nheld = 0; t->arp_phys = 0;
    arp_add(t, D[0], 100);
    t->arp_pos = 0xFFFFFFF;
    arp_tick(t, CTL);
    cck("ARP DRONE: one key: the drone alone, no arp note", t->arp_ch[0] == 62 && t->arp_note == 0);
    t->nheld = 0; t->arp_phys = 0;
    arp_tick(t, CTL);
    cck("ARP DRONE: the keys let go: the drone ends", t->arp_ch[0] == 0 && t->arp_note == 0);
}

static void test_slew(void)
{
    static int32_t z[CTL], in[CTL], wet[2 * CTL];
    uint32_t b, i, ok = 1;
    int32_t s0, s1, tgt, prev;
    ui_power_on();
    song.g[G_DTIME] = 2;                                /* 1/16 */
    song.g[G_DMIX] = 64;
    for (i = 0; i < CTL; i++)
        in[i] = (int32_t)(i * 977u % 2001u) - 1000;
    for (b = 0; b < 20u; b++)
        fx_buses(z, in, z, wet, CTL);
    s0 = fx.dl_s2;
    cck("the delay's read point stands at TIME's (1/16) once it has settled", s0 == (int32_t)(delay_samples() << DLY_Q));
    song.g[G_DTIME] = 0;                                /* 1/4: four times the delay */
    tgt = (int32_t)(delay_samples() << DLY_Q);
    fx_buses(z, in, z, wet, CTL);
    s1 = fx.dl_s2;
    cck("a TIME change: after one block the read point moved, by under a hundredth of the way (the motor)",
        s1 > s0 && s1 - s0 < (tgt - s0) / 100);
    prev = s1;
    for (b = 0; b < 4u * FS / CTL; b++) {
        fx_buses(z, in, z, wet, CTL);
        ok &= fx.dl_s2 >= prev;
        prev = fx.dl_s2;
    }
    cck("it moves one way only and is there within four seconds", ok && fx.dl_s2 == tgt);
}

static void test_project(void)
{
    static project_t a, c;
    static project_store_t st;
    int16_t in[P_COUNT], out[P_COUNT], def[P_COUNT];
    uint32_t i;
    ui_power_on();
    project_capture(&a);
    a.t[1].p[P_CANON] = 5;
    cck("the project: P_CANON in its byte (np 128), back as saved",
        proj_pack(&st, &a) && st.raw[66] == 128u && proj_import(&c, &st, sizeof st) && c.t[1].p[P_CANON] == 5 && c.t[0].p[P_CANON] == 0);
    for (i = 0; i < P_COUNT; i++) {
        in[i] = (int16_t)i;
        def[i] = -1;
    }
    params_by_count(out, in, P_COUNT, def);
    i = out[P_CANON] == P_CANON && out[P_E0] == P_E0 && out[P_E0 + 7] == P_E0 + 7 && out[P_TEMP] == P_TEMP;
    params_by_count(out, in, 126u, def);
    i &= out[P_CANON] == -1 && out[P_E0] == P_E0 && out[P_E0 + 7] == P_E0 + 7 && out[P_TEMP] == P_TEMP;
    params_by_count(out, in, 125u, def);
    i &= out[P_CANON] == -1 && out[P_E0] == P_E0 - 1 && out[P_E0 + 7] == P_E0 + 6 && out[P_TEMP] == -1;   /* (E0 at 117 there) */
    cck("params_by_count: a 127-store's P_CANON after P_E7, a 126 / 125 one's the default", i);
}

static uint16_t head_px(int32_t x, int32_t y) { return swap16(host_screen[(uint32_t)(Y_HEAD + y) * 240u + (uint32_t)x]); }

static void test_ui(void)
{
    ui_power_on();
    open_family(FAM_ARP);
    cck("ARP opens on ARP", !strcmp(PAGES[ui.page].title, "ARP"));
    fam_turn(FAM_ARP, 1);
    cck("PRESETS right with ARP held: ARP 2", !strcmp(PAGES[ui.page].title, "ARP 2"));
    fam_turn(FAM_ARP, 1);
    cck("again: CANON", !strcmp(PAGES[ui.page].title, "CANON"));
    fam_turn(FAM_ARP, -1);
    cck("PRESETS left: ARP 2 again", !strcmp(PAGES[ui.page].title, "ARP 2"));
    song.cpu_q8 = 230;                                  /* 89 %: the accent */
    frame();
    {
        int32_t h = (89 * HEAD_CPU_H / 100 + 1) / 2;   /* rows each side of the middle */
        cck("the header's CPU bar at 89 % stands in the accent, 89 % of its rows about the middle row",
            head_px(HEAD_CPU_X, HEAD_CPU_MID) == T_ACCENT && head_px(HEAD_CPU_X, HEAD_CPU_MID - h) == T_ACCENT &&
            head_px(HEAD_CPU_X, HEAD_CPU_MID + h - 1) == T_ACCENT && head_px(HEAD_CPU_X, HEAD_CPU_MID - h - 1) != T_ACCENT &&
            head_px(HEAD_CPU_X, HEAD_CPU_MID + h) != T_ACCENT);
    }
    song.cpu_q8 = 64;                                   /* 25 %: dim, a quarter of the height */
    frame();
    cck("at 25 %: dim, the rows away from the middle clear",
        head_px(HEAD_CPU_X, HEAD_CPU_MID) == T_MID && head_px(HEAD_CPU_X, HEAD_CPU_MID - HEAD_CPU_H / 2) != T_MID);
}

int main(void)
{
    test_canon();
    test_drone();
    test_slew();
    test_project();
    test_ui();
    puts(bad ? "DESERT2 TEST FAILED" : "desert2 test passed");
    return bad != 0;
}
