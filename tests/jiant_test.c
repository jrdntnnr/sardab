/* SPDX-License-Identifier: GPL-3.0-only */
/* Host test of the four JIANT ports (GHOULBOX desert), same sources as the firmware (hostsim.c):
 *   build/host/jiant_test
 * 1. FILTER TYPE COMB (voice.c track_comb): a fourth TYPE on ANALOG; a note through it sounds, differs from LP,
 *    stays bounded and dies away after the note; every other engine ignores TYPE 3.
 * 2. ENV LOOP (P_ELOOP, once P_ED_FX; voice.c env_tick): ON, a held note's envelope climbs again every time it
 *    reaches the sustain; OFF it never does.
 * 3. ANALOG INT (P_E7, once KTR): osc 2 INT semitones from osc 1 (+12: twice the zero crossings); the key tracking
 *    of the cutoff is fixed at what KTR 64 gave; factory presets hold INT 0. A project or a user preset from before
 *    (no marker) loads an ANALOG track's E7 as 0, not as semitones; one written now keeps it.
 * 4. the GRAIN delay (G_DPIT, G_DSPRY; fx.c dly_grain): two globals after KEYS', DLY 2's page; PITCH +12 doubles the
 *    tail's zero crossings; SPRAY stays bounded; both survive a project's pack / import. */
#define UI_TEST_NO_MAIN 1
#include "ui_test.c"                /* (the firmware with upreset.c and project.c, as fun10_test) */

static int bad;
static void jcheck(const char *what, int ok)
{
    printf("jiant: %-78s %s\n", what, ok ? "ok" : "FAIL");
    bad += !ok;
}

static int32_t out_buf[2 * CTL];
static int32_t peak_l, zc_l;
static int32_t last_l;
static void jblocks(uint32_t n)                      /* renders n blocks: the left peak and zero crossings */
{
    uint32_t i;
    while (n--) {
        mix_block(out_buf, CTL);
        for (i = 0; i < CTL; i++) {
            int32_t x = out_buf[2u * i], a = x < 0 ? -x : x;
            if (a > peak_l) {
                peak_l = a;
            }
            if ((x > 1000 && last_l <= -1000) || (x < -1000 && last_l >= 1000)) {
                zc_l++;
            }
            if (a > 1000) {
                last_l = x;
            }
        }
    }
}
static void jmeter(void) { peak_l = zc_l = last_l = 0; }

static void jfresh(void)                         /* the boot state (no FX tails), as insert_test's fresh */
{
    uint32_t k;
    memset(dly_buf, 0, sizeof dly_buf);
    memset(cho_buf, 0, sizeof cho_buf);
    memset(rev_comb, 0, sizeof rev_comb);
    memset(rev_ap, 0, sizeof rev_ap);
    memset(&fx, 0, sizeof fx);
    memset(&pf, 0, sizeof pf);
    memset(ins, 0, sizeof ins);
    memset(comb_buf, 0, sizeof comb_buf);
    for (k = 0; k < NTRK; k++) {
        pf.mg[k] = 32768;
    }
    perf_held = perf_latched = 0;
    lim_reset();
    dc_l = dc_r = dce_l = dce_r = 0;
    vage = 0;
    rng_state = 0x1234567u;
    mod_seed = 0x2545F491u;
    song.playing = 0;
    memset(trk, 0, sizeof trk);
    memset(&mod, 0, sizeof mod);
    memset(sl, 0, sizeof sl);
    memset(&motion, 0, sizeof motion);
    host_tracks_init();
    for (k = 0; k < NPART; k++) {
        host_preset(&trk[k], 0, 0);
    }
    song.sel = 0;
}

static void analog_fresh(track_t *t, uint32_t wave, int32_t mix)
{
    jfresh();                                        /* track 1: ANALOG SAW LEAD */
    t->p[P_VOICE] = V_POLY;
    t->p[P_E0] = (int16_t)wave;
    t->p[P_E1] = 0; t->p[P_E2] = (int16_t)mix; t->p[P_E3] = 0;
    t->p[P_E4] = 127; t->p[P_E5] = 0; t->p[P_E6] = 0; t->p[P_E7] = 0;
    t->p[P_FTYPE] = 0;
    t->p[P_ED_FLT] = t->p[P_LD_FLT] = 0;
    t->p[P_ATK] = 0; t->p[P_DEC] = 0; t->p[P_SUS] = 127; t->p[P_REL] = 0;
    t->p[P_DIST] = t->p[P_CHOR] = t->p[P_DLY] = t->p[P_REV] = 0;
    t->p[P_LEVEL] = 100;
    song.g[G_DPIT] = song.g[G_DSPRY] = 0;
}

static void test_comb(void)
{
    track_t *t = &trk[0];
    int32_t pk_lp, pk_comb;
    uint32_t e;
    int ok;
    jcheck("TYPE: LP BP HP COMB (COMB appended, value 3)",
          TP[P_FTYPE].max == 3 && str_eq(TP[P_FTYPE].names[3], "COMB") && str_eq(TP[P_FTYPE].names[0], "LP"));
    analog_fresh(t, 0, 0);
    trk_note_on(t, 60, 100);
    jblocks(FS / 4u / CTL);
    trk_note_off(t, 60);
    jblocks(FS / 5u / CTL);
    jmeter();
    jblocks(FS / 10u / CTL);                         /* 200..300 ms after the note: LP (REL 0) is gone */
    pk_lp = peak_l;
    analog_fresh(t, 0, 0);
    t->p[P_FTYPE] = 3; t->p[P_E4] = 64; t->p[P_E5] = 120;
    jmeter();
    trk_note_on(t, 60, 100);
    jblocks(FS / 4u / CTL);
    ok = peak_l > 2000 && peak_l < 32767 * 2;
    trk_note_off(t, 60);
    jblocks(FS / 5u / CTL);
    jmeter();
    jblocks(FS / 10u / CTL);                         /* 200..300 ms after the note: the comb still rings */
    pk_comb = peak_l;
    ok &= pk_comb > 300 && pk_comb > pk_lp * 4;
    jblocks(FS / 2u / CTL);                          /* the ring dies away */
    jmeter();
    jblocks(FS / 10u / CTL);
    ok &= peak_l < 50;
    printf("    (300 ms after the note: LP %d, COMB %d; 0.5 s after: %d)\n", pk_lp, pk_comb, peak_l);
    jcheck("COMB: a C4 through it sounds, bounded, rings 300 ms after the note (LP does not), silent 0.5 s on", ok);
    ok = 1;
    for (e = 2; e < NENGINES; e++) {                 /* the other engines: TYPE 3 is nothing to them (no crash) */
        if (!eng_ok(e) || e == ENGI_DRUM || e == ENGI_SLICE) {
            continue;
        }
        host_preset(t, e, 0);
        t->p[P_FTYPE] = 3;
        jmeter();
        trk_note_on(t, 60, 100);
        jblocks(FS / 20u / CTL);
        trk_note_off(t, 60);
        jblocks(FS / 10u / CTL);
        ok &= peak_l < 32767 * 2;
    }
    t->p[P_FTYPE] = 0;
    jcheck("  every other engine with TYPE 3: bounded, no crash", ok);
}

static void test_env_loop(void)
{
    track_t *t = &trk[0];
    uint32_t k, up, rises[2];
    int32_t last;
    jcheck("LOOP: P_ED_FX is ENV LOOP, OFF / ON, default OFF",
          str_eq(TP[P_ELOOP].label, "LOOP") && TP[P_ELOOP].max == 1 && TP[P_ELOOP].def == 0 && P_ELOOP == P_ED_FX);
    for (up = 0; up < 2u; up++) {
        analog_fresh(t, 0, 0);
        t->p[P_ATK] = 20; t->p[P_DEC] = 30; t->p[P_SUS] = 40;
        t->p[P_ELOOP] = (int16_t)up;
        trk_note_on(t, 60, 100);
        jblocks(FS / 10u / CTL);                      /* past the first attack */
        rises[up] = 0;
        last = t->v[0].env;
        for (k = 0; k < FS * 2u / CTL; k++) {        /* 2 s held: how often the envelope turns up again */
            int32_t e;
            jblocks(1);
            e = t->v[0].env;
            if (e > last + (1 << 16) && t->v[0].stage == 1u) {
                rises[up]++;
            }
            last = e;
        }
        trk_note_off(t, 60);
        jblocks(FS / 2u / CTL);
    }
    printf("    (rises OFF %u, ON %u)\n", rises[0], rises[1]);
    jcheck("  ON: the held note's envelope climbs again from the sustain (an ADSR-shaped LFO); OFF: never",
          rises[1] >= 3u && rises[0] == 0u);
}

static uint32_t zc_of(track_t *t, int32_t semis)
{
    analog_fresh(t, 0, 127);                         /* osc 2 alone */
    t->p[P_E7] = (int16_t)semis;
    jmeter();
    trk_note_on(t, 57, 100);                         /* A3, 220 Hz */
    jblocks(FS / CTL);
    trk_note_off(t, 57);
    jblocks(FS / 10u / CTL);
    return (uint32_t)zc_l;
}

static void test_int(void)
{
    track_t *t = &trk[0];
    const param_desc_t *d = &ENG_ANALOG.edit[7];
    uint32_t z0, z12, zm12, k;
    int ok;
    jcheck("INT: ANALOG's 8th value, semitones -24..24, default 0 (KTR retired: its 64 is fixed in the cutoff)",
          str_eq(d->label, "INT") && d->fmt == F_SEMI && d->min == -24 && d->max == 24 && d->def == 0);
    z0 = zc_of(t, 0); z12 = zc_of(t, 12); zm12 = zc_of(t, -12);
    ok = z0 > 400u && z12 > z0 * 19u / 10u && z12 < z0 * 21u / 10u && zm12 > z0 * 9u / 20u && zm12 < z0 * 11u / 20u;
    printf("    (zc INT 0 %u, +12 %u, -12 %u)\n", z0, z12, zm12);
    jcheck("  osc 2 at INT +12 / -12: twice / half the zero crossings of INT 0 (A3)", ok);
    ok = 1;
    for (k = 0; k < ENG_ANALOG.npresets; k++) {
        ok &= ENG_ANALOG.presets[k].e[7] == 0;
    }
    jcheck("  the factory ANALOG presets: INT 0", ok);
    {   /* a project: INT kept; the same store without the marker: an ANALOG track's E7 is 0, another engine's kept */
        project_t a, b, c;
        static project_store_t st;
        analog_fresh(t, 0, 0);
        t->p[P_E7] = 7;
        host_preset(&trk[1], ENGI_PHYS, 0);
        trk[1].p[P_E7] = 9;
        project_capture(&a);
        ok = proj_pack(&st, &a) && proj_import(&b, &st, sizeof st) && b.t[0].p[P_E7] == 7 && b.t[1].p[P_E7] == 9;
        ok &= st.raw[PROJ_DESERT_OFF] != 0u;
        st.raw[PROJ_DESERT_OFF] = 0;                 /* (as every project before wrote it) */
        memcpy(st.raw + PROJ_STORE_SIZE - 4u, &(uint32_t){proj_hash(st.raw, PROJ_STORE_SIZE - 4u)}, 4);
        ok &= proj_import(&c, &st, sizeof st) && c.t[0].p[P_E7] == 0 && c.t[1].p[P_E7] == 9;
        jcheck("  a project keeps INT; one from before the marker: ANALOG's E7 (KTR then) loads as 0, PHYS's kept", ok);
    }
    {   /* a user preset: the record's category byte carries the marker (bit 7); a record without it: E7 0 */
        up_rec_t *r;
        song.sel = 0;
        analog_fresh(t, 0, 0);
        t->p[P_E7] = -5;
        {
            int rc = up_store(2, "INT TEST"), l1, l2;
            r = up_rec(2);
            t->p[P_E7] = 0;
            l1 = up_load(2);
            ok = (rc == 0 || rc == 3) && (r->packed[UP_CAT_AT] & 0x80u) != 0u && up_cat(2) == CAT_LEAD && l1 == 0 && t->p[P_E7] == -5;
            printf("    (store %d, cat byte %02x, cat %u, load %d, E7 %d)\n", rc, r->packed[UP_CAT_AT], (unsigned)up_cat(2), l1, t->p[P_E7]);
            r->packed[UP_CAT_AT] &= 0x7Fu;           /* (a record of before) */
            l2 = up_load(2);
            ok &= l2 == 0 && t->p[P_E7] == 0;
            printf("    (without the mark: load %d, E7 %d)\n", l2, t->p[P_E7]);
        }
        jcheck("  a user preset keeps INT (marker in the category byte); one of before loads E7 as 0", ok);
    }
}

static uint32_t tail_zc(track_t *t, int32_t pit, int32_t spray)
{
    analog_fresh(t, 3, 0);                           /* SIN osc 1 */
    t->p[P_DLY] = 127;
    song.g[G_DTIME] = 1; song.g[G_DFDBK] = 0; song.g[G_DCOLOR] = 127; song.g[G_DMIX] = 127;
    song.g[G_DPIT] = (int16_t)pit; song.g[G_DSPRY] = (int16_t)spray;
    song.g[G_BPM] = 120;
    jblocks(2u * FS / CTL);                           /* the line empty */
    trk_note_on(t, 69, 100);                         /* A4 for 1/4 s */
    jblocks(FS / 4u / CTL);
    trk_note_off(t, 69);
    jblocks(FS / 20u / CTL);                          /* the dry gone, the repeat not yet (1/8 at 120: 250 ms) */
    jmeter();
    jblocks(FS / 4u / CTL);                           /* the tail: the one repeat */
    return (uint32_t)zc_l;
}

static void test_grain(void)
{
    track_t *t = &trk[0];
    uint32_t z0, z12, p, found = 0;
    int ok;
    jcheck("GRAIN: G_DPIT PITCH -12..12 and G_DSPRY SPRAY 0..127 after KEYS', G_COUNT 39",
          G_DPIT == G_KOCT + 1u && G_DSPRY == G_DPIT + 1u && G_COUNT == 43u && str_eq(GP[G_DPIT].label, "PITCH") &&
          GP[G_DPIT].min == -12 && GP[G_DPIT].max == 12 && str_eq(GP[G_DSPRY].label, "SPRAY") && GP[G_DSPRY].max == 127);
    for (p = 0; p < NPAGES; p++) {
        if (str_eq(PAGES[p].title, "DLY 2") && PAGES[p].fam == FAM_FX && PAGES[p].scope == SC_GLOBAL &&
            PAGES[p].id[0] == G_DPIT && PAGES[p].id[1] == G_DSPRY) {
            found = 1;
        }
    }
    jcheck("  DLY 2: a global FX page, PITCH SPRAY", found);
    z0 = tail_zc(t, 0, 0);
    z12 = tail_zc(t, 12, 0);
    ok = z0 > 100u && z12 > z0 * 16u / 10u && z12 < z0 * 25u / 10u;
    printf("    (tail zc PITCH 0 %u, +12 %u)\n", z0, z12);
    jcheck("  PITCH +12: the repeat's zero crossings about twice PITCH 0's (an A4 burst)", ok);
    tail_zc(t, -7, 100);
    ok = peak_l > 100 && peak_l < 32767 * 2;
    printf("    (tail peak %d)\n", peak_l);
    jcheck("  PITCH -7 SPRAY 100: a tail, bounded", ok);
    {
        project_t a, b;
        static project_store_t st;
        song.g[G_DPIT] = -3; song.g[G_DSPRY] = 77;
        project_capture(&a);
        ok = proj_pack(&st, &a) && proj_import(&b, &st, sizeof st) && b.dpit == -3 && b.dspry == 77;
        song.g[G_DPIT] = song.g[G_DSPRY] = 0;
        jcheck("  PITCH and SPRAY survive a project's pack / import", ok);
    }
}

int main(void)
{
    host_tracks_init();
    ui_power_on();
    test_comb();
    test_env_loop();
    test_int();
    test_grain();
    puts(bad ? "JIANT TEST FAILED" : "jiant test passed");
    return bad != 0;
}
