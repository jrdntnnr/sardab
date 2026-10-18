/* SPDX-License-Identifier: GPL-3.0-only */
/* 2026-10 upstream ports: actual DSP, persistence and modulation, no hardware. */
#define UI_TEST_NO_MAIN 1
#include "ui_test.c"
#include <assert.h>
int main(void)
{
    uint32_t i, j;
    ui_power_on();
    /* NOISE: bounded, bipolar, smooth and different for each track. */
    int32_t lo = 32767, hi = -32768, prev = 0;
    uint32_t different = 0;
    for (i = 0; i < 8192; i++) {
        mns_clk = i;
        int32_t a = mod_tsrc(&trk[0], MS_NOISE, 0), b = mod_tsrc(&trk[1], MS_NOISE, 0);
        assert(a >= -32768 && a <= 32767);
        if (i) assert(abs(a - prev) < 1600);
        lo = a < lo ? a : lo; hi = a > hi ? a : hi;
        different += a != b; prev = a;
    }
    assert(lo < -20000 && hi > 20000 && different > 8000);
    trk[0].p[P_M1SRC] = MS_NOISE; trk[0].p[P_M1DST] = MD_CUT; trk[0].p[P_M1AMT] = 32;
    mod_begin(&trk[0]);
    assert(mod.on && mod.cut != 0);
    mod_end(&trk[0]);
    puts("upstream: smooth NOISE range, independence and matrix routing PASS");
    /* A pitch edit cannot move a grain while its window is audible. */
    memset(&fx, 0, sizeof fx); song.g[G_DPIT] = 12;
    dly_grain(4000);
    assert(fx.gr_s[0] == 65536 && fx.gr_s[1] == 65536);
    song.g[G_DPIT] = -12;
    dly_grain(4000);
    assert(fx.gr_s[0] == 65536 && fx.gr_s[1] == 65536);
    for (i = 2; i < GR_LEN / 2; i++) dly_grain(4000);
    assert(fx.gr_s[0] == 65536 && fx.gr_s[1] == -32768);
    for (; i < GR_LEN; i++) dly_grain(4000);
    assert(fx.gr_s[0] == -32768 && fx.gr_s[1] == -32768);
    puts("upstream: grain pitch changes only at each zero-weight boundary PASS");
    /* Loud COMB used to overflow on its output multiply (also run under UBSan). */
    int32_t out[CTL];
    memset(trk, 0, sizeof trk); comb_p16[0] = 60 * 16;
    for (i = 0; i < CTL; i++) out[i] = 160000;
    track_comb(&trk[0], out, CTL, 64, 0);
    for (i = 0; i < CTL; i++) assert(out[i] > 0 && out[i] < 100000);
    memset(comb_buf, 0, sizeof comb_buf);
    memset(comb_w, 0, sizeof comb_w);
    for (i = 0; i < CTL; i++) out[i] = -160000;
    track_comb(&trk[0], out, CTL, 64, 0);
    for (i = 0; i < CTL; i++) assert(out[i] < 0 && out[i] > -100000);
    puts("upstream: loud positive/negative COMB bounded without sign inversion PASS");
    /* AMP: all knob corners, loud chords, sustained and then silence. */
    ui_power_on();
    track_t *t = &trk[0];
    for (uint32_t a = 0; a <= 127; a += 127)
    for (uint32_t b = 0; b <= 127; b += 127)
    for (uint32_t c = 0; c <= 127; c += 127) {
        ins_rest(t);
        t->p[P_ITYPE] = IT_AMP; t->p[P_IMIX] = 127;
        t->p[P_IA] = a; t->p[P_IB] = b; t->p[P_IC] = c;
        int64_t dc = 0; int32_t peak = 0;
        for (j = 0; j < 4000; j++) {
            for (i = 0; i < CTL; i++) out[i] = (((j * CTL + i) % 251) * 1400) - 175000;
            track_insert(t, out, CTL);
            for (i = 0; i < CTL; i++) {
                assert(abs(out[i]) < 200000);
                if (j > 3000) dc += out[i];
                if (abs(out[i]) > peak) peak = abs(out[i]);
            }
        }
        assert(peak > 0 && llabs(dc / (999 * CTL)) < 300);
        t->p[P_IMIX] = 0;
        for (j = 0; j < 8; j++) { memset(out, 0, sizeof out); track_insert(t, out, CTL); }
        for (i = 0; i < CTL; i++) out[i] = (int32_t)i * 123 - 16000;
        int32_t dry[CTL]; memcpy(dry, out, sizeof dry); track_insert(t, out, CTL);
        assert(!memcmp(out, dry, sizeof dry));
    }
    assert(!strcmp(ins_desc(IT_AMP, 0)->label, "DRIVE"));
    assert(!strcmp(ins_desc(IT_AMP, 1)->label, "TONE"));
    t->p[P_M1SRC] = MS_NOISE; t->p[P_ITYPE] = IT_AMP;
    project_t saved, restored; project_store_t packed;
    project_capture(&saved);
    assert(proj_pack(&packed, &saved));
    assert(proj_import(&restored, &packed, sizeof packed));
    assert(restored.t[0].p[P_ITYPE] == IT_AMP && restored.t[0].p[P_M1SRC] == MS_NOISE);
    puts("upstream: AMP corners, DC, bypass, labels and project round-trip PASS");
    return 0;
}
