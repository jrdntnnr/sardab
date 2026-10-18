/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments */
/* Effects: per-track DIST and INSERT (1.5), then sends into three
 * shared buses (chorus, tempo delay, reverb). Mono buses, stereo dry mix. */
#define DLY_LEN 65536u           /* 1.49 s: 1/4 fits from 41 BPM, 1/4D from 61, 1/2 from 81 */
#define CHO_LEN 2048u
static int16_t dly_buf[DLY_LEN] __attribute__((section(".pool")));
static int16_t cho_buf[CHO_LEN] __attribute__((section(".pool")));
static const uint16_t REV_COMB[4] = {1116, 1188, 1277, 1356};
static const uint16_t REV_AP[2] = {556, 441};
static int16_t rev_comb[1116 + 1188 + 1277 + 1356] __attribute__((section(".pool")));
static union {                          /* ROOM's allpasses; SPRING's allpass chain (int32: no clamps) */
    int16_t ap[556 + 441];
    int32_t sp[(556 + 441) / 2];
} rev_u __attribute__((section(".pool")));
#define rev_ap (rev_u.ap)
static struct {
    uint32_t dly_w, cho_w, cho_ph;
    uint32_t gr_ph, gr_sp[2];            /* GHOULBOX desert: the GRAIN delay (dly_grain): grain 0's phase (Q16), each
                                          * grain's spray (samples) */
    int32_t gr_s[2];
    uint8_t gr_ready;
    int32_t dly_lp;
    int32_t dl_s1, dl_s2;                /* GHOULBOX desert: the delay read point (Q12 samples) on its way to G_DTIME's,
                                          * two poles (DLY_SLEW): a time change bends the echoes' pitch like a tape
                                          * machine's motor (0: not yet set, snaps) */
    uint16_t comb_i[4], ap_i[2];
    int32_t comb_lp[4];
    uint8_t rtype;                       /* the reverb model running (G_RTYPE: 0 ROOM, 1 SPRING, 2 HALL, 3 STRINGS) */
    uint16_t sp_w;                       /* SPRING: the loop's write index (SP_MASK) */
    int32_t sp_lp, sp_hp, sp_he, sp_size;   /* .. its loop low-pass, low cut (and its remainder), the loop
                                             * length (Q8, glides) */
    uint32_t sp_ph;                      /* .. the output tap's wobble */
    int32_t st_len[4], st_tgt[4];        /* STRINGS (GHOULBOX desert): each line's delay (Q8 samples) and its tuning */
} fx;

/* SPRING (G_RTYPE 1): one spring of a spring tank, mono like the other buses, in the ROOM's own buffers (no
 * RAM of its own): the input and the loop's return -> a low cut (~110 Hz: a spring carries little bass) ->
 * SP_N stretched first-order allpasses, (a + z^-4) / (1 + a z^-4) (after Valimaki, Parker and Abel: below
 * fs / 8 = 5.5 kHz the group delay rises with frequency, the chirp; each pass round the loop adds more of
 * it: the "boing", the drips) -> the loop's delay line (rev_comb, SP_LEN) -> back through a one-pole
 * low-pass (DAMP) and the decay gain (SIZE). The output: the spring's far end, half way along the loop
 * (the first sound 15 .. 30 ms after the send: the tank's own pre-delay; a slow wobble of a sample or two
 * on it), plus a second, quieter pickup at three quarters (a shorter spring beside it: denser). SIZE sets
 * the loop's length (30 .. 60 ms) and its decay; DAMP the loop's low-pass. The allpasses' states: 4
 * samples each (int32), in rev_ap's memory. Changing the model fades the old one's block out and clears both buffers. */
#define SP_LEN 4096u                     /* the loop's line in rev_comb (4937 samples) */
#define SP_MASK (SP_LEN - 1u)
#define SP_N 10u                         /* allpass stages */
#define SP_A 2867                        /* their coefficient, Q12 (0.7: Q12 keeps (x - o) * a in 32 bits up to
                                          * |x - o| < 749000, far past any peak the chain reaches) */
_Static_assert(sizeof rev_comb / 2u >= SP_LEN && sizeof rev_u.sp / 4u >= 4u * (SP_N + 1u), "SPRING in ROOM's buffers");

/* HALL (G_RTYPE 2, 1.2): a smooth, dense, stereo reverb, again in ROOM's buffers (its few states and one block
 * of work in the pool, no .bss). ROOM's 4 combs each ring on their own (the metallic tone); here every echo
 * goes round 8 lines that all feed each other, and they run at half the rate (22.05 kHz: the same memory holds
 * twice the time, so twice the resonances, at half the work; the tail stops near 10 kHz, as a hall's air
 * does). The send (every other sample of it, low-passed 1 2 1; ROOM's level) -> a low cut (~110 Hz: no boom
 * in a long tail) -> 2 allpass diffusers (rev_ap: a drum's click smeared into a burst) -> into the 8 lines
 * (rev_comb, mutually prime lengths, 21 .. 34 ms). Lines 1, 2, 4, 7 are read through slow sines (+-HL_MOD
 * samples, between samples; 0.37 or 0.74 Hz, each in its own phase): the resonances drift instead of ringing.
 * Lines 0, 3, 5, 6, read at their ends, go through a one-pole low-pass (DAMP: the mixing takes every echo
 * through them); every
 * line through its gain (from its length: all decay alike; RT60 below the damping 0.5 s at SIZE 0, 4 s at
 * 90, 9 s at 127; ~2.4 s heard at 90 with DAMP 60); then the 8 x 8 mixing: a conference matrix (Paley,
 * order 8: every line feeds the 7 others, +-1 each, none itself: no flutter at a line's own length), with
 * the send added. Stereo: left lines 0 3 4 -7, right 1 2 5 6 (sums spread over the matrix's rows); the wet
 * bus (mono) gets left + right, rev_side left - right, which mix_block adds to the left and takes from the
 * right (rev_side_mix). Back to 44.1 kHz between samples. Rounding: a loud line's gain rounds to the nearest
 * (the tail decays as it should), a quiet one's towards zero (it ends in exact silence: no limit cycle), and
 * the low-passes keep their steps' remainders (no offset). */
#define HL_NL 8u                         /* lines */
#define HL_NAP 2u                        /* diffusers */
#define HL_NC (CTL / 2u)                 /* a block at half the rate */
#define HL_MOD 8                         /* the taps' swing, samples (read 2 .. 18 samples short of the end) */
#define HL_LPM 0x69u                     /* the lines with a low-pass */
#define HL_KD(d) (32767 - (d) * 180)     /* its coefficient (Q15) for DAMP d: none at 0 .. ~1 kHz at 127 */
#define HL_LOUD 32                       /* a line this loud (two samples' sum) rounds its gain to the nearest */
#define HL_OUT 4                         /* the outputs' gain: the level of ROOM's tail */
static const uint16_t HL_LEN[HL_NL] = {467, 509, 547, 593, 631, 673, 709, 743};   /* (+1 each: the mirror) */
static const uint16_t HL_AP[HL_NAP] = {131, 307};
static const int16_t HL_APG[HL_NAP] = {11469, 10240};               /* Q14: 0.7, 0.625 */
_Static_assert(467 + 509 + 547 + 593 + 631 + 673 + 709 + 743 + HL_NL <= sizeof rev_comb / 2u &&
               131 + 307 <= sizeof rev_u.ap / 2u, "HALL in ROOM's buffers");
_Static_assert(CTL % 2u == 0u && HL_NC + 2u * HL_MOD + 2u < 467u && HL_NC <= 131u, "HALL: one wrap a block at most");
static struct {                          /* HALL's state (in the pool: no .bss) */
    uint16_t i[HL_NL], d[HL_NAP];        /* the lines' write indices, the diffusers' */
    int32_t lp[HL_NL], le[HL_NL];        /* the low-passes and their steps' remainders */
    int32_t x, m, s, hp, he;             /* the send's last sample, its low cut and that step's remainder; the
                                          * last mid and side out (half rate) */
    int16_t g[HL_NL], sz;                /* the lines' gains / sqrt(7) (Q14), for SIZE sz - 1 (0: none yet) */
    uint32_t ph;                         /* the taps' drift */
    uint8_t side;                        /* sd holds this block's stereo difference (HALL running) */
    int32_t sd[CTL];                     /* the block's left minus right (mix_block: rev_side_mix) */
    int32_t bx[HL_NC], l[HL_NC], r[HL_NC];   /* the block at half the rate: the diffused send, left, right */
    int16_t y[HL_NL][HL_NC];             /* the lines' ends, then what goes back into them */
} hl __attribute__((section(".pool")));
#define rev_side (hl.sd)

/* DIST: low cut -> drive (1x..8x, exponential) -> asymmetric soft clip
 * (a little bias = even harmonics) -> tone low-pass that closes with drive ->
 * make-up gain (straight into tanh over the full band, it would sound like a
 * broken digital fuzz). State per part (track_t dist_*). */
static void track_dist(track_t *t, int32_t *b, uint32_t n)
{
    int32_t d = t->p[P_DIST], i, g, k, mk, bias = 2400, b0;
    if (!d)
        return;                                         /* states kept: switching on does not click */
    g = 4096 + d * d * 2;                                /* Q12: 1x .. ~9x, gentle at first */
    k = 32000 - d * 95;                                  /* tone: transparent at low drive .. ~3 kHz, Q15 */
    mk = 30000 - d * 120;                                /* make-up */
    b0 = softclip(bias);
    for (i = 0; i < (int32_t)n; i++) {
        int32_t x = b[i], y;
        t->dist_hp += (x - t->dist_hp + 64) >> 7;           /* ~55 Hz low cut: keep the bass out of the clipper */
        x = clamp(x - t->dist_hp, -230000, 230000);         /* (x >> 2) * g fits 32 bits; the clip is flat out there */
        y = softclip((((x >> 2) * g) >> 10) + bias) - b0;   /* >> 2 first: no overflow for loud poly */
        t->dist_lp1 += mulq15(y - t->dist_lp1, k);         /* two poles: tames the fizz */
        t->dist_lp2 += mulq15(t->dist_lp1 - t->dist_lp2, k);
        b[i] = mulq15(t->dist_lp2, mk);
    }
}

/* master: peak limiter in front of the soft clipper, threshold where tanh is still nearly linear, so chords get
 * quieter instead of crushed. After SLOOP 2.6 (isod89): it looks one block ahead (lim_block). The output is the mix
 * CTL samples late (0.73 ms) and the gain glides over each block to what the loudest sample of it and of the next
 * needs, so a hot kick is already turned down when it comes. Before, the gain caught up in ~4 samples and the first
 * ones went through the knee at up to 4x: a crackle on the beats with MASTER up. A 35 ms hold, then ~46 ms release */
#define LIM_T 18000
#define LIM_HOLD 48u                    /* blocks: a held bass (down to ~29 Hz) keeps one gain, no dip every cycle */
#define LIM_REL 11                      /* the release after the hold: ~46 ms */
typedef struct {
    int32_t dl[CTL], dr[CTL];           /* the mix of the last block (the delay) */
    uint32_t w;                         /* its oldest sample (the next out) */
    int32_t env;                        /* the peak the gain fits under LIM_T (LIM_T: none) */
    int32_t g;                          /* the gain at the end of the last block, Q15 (32768: 1) */
    uint32_t hold;                      /* blocks before the release starts */
} lim_t;
static lim_t lim = {.env = LIM_T, .g = 32768};
static void lim_reset(void)             /* power-on state (tests: runs compare) */
{
    memset(&lim, 0, sizeof lim);
    lim.env = LIM_T;
    lim.g = 32768;
}
/* l, r: n (<= CTL) samples in; out: the n samples of CTL before, limited (before the knee) */
static void lim_block(lim_t *s, int32_t *l, int32_t *r, uint32_t n)
{
    uint32_t i, w = s->w;
    int32_t pk = 0, a, g0 = s->g, g1, dg, e = s->env;

    /* the peak of what goes out now and of what follows it: CTL ahead */
    for (i = 0; i < CTL; i++) {
        a = s->dl[i] < 0 ? -s->dl[i] : s->dl[i], pk = a > pk ? a : pk;
        a = s->dr[i] < 0 ? -s->dr[i] : s->dr[i], pk = a > pk ? a : pk;
    }
    for (i = 0; i < n; i++) {
        a = l[i] < 0 ? -l[i] : l[i], pk = a > pk ? a : pk;
        a = r[i] < 0 ? -r[i] : r[i], pk = a > pk ? a : pk;
    }

    /* the envelope: up at once, held while peaks come near it, then released a block at a time */
    if ((int64_t)pk * 17 >= (int64_t)e * 16) {
        s->hold = LIM_HOLD;
    }
    if (s->hold) {
        s->hold--;
    } else if (e > LIM_T) {
        e -= (int32_t)((((int64_t)(e - LIM_T) * (int32_t)n) >> LIM_REL) + 1);
    }
    if (e < LIM_T) {
        e = LIM_T;
    }
    if (pk > e) {
        e = pk, s->hold = LIM_HOLD;
    }
    s->env = e;

    /* the gain glides from the last block's to this one's; 64-bit: every bit of the limited signal kept */
    g1 = e > LIM_T ? (int32_t)(((uint32_t)LIM_T << 15) / (uint32_t)e) : 32768;
    s->g = g1;
    if (g0 == 32768 && g1 == 32768) {                   /* not limiting (most blocks): the delay only */
        for (i = 0; i < n; i++) {
            int32_t xl = s->dl[w], xr = s->dr[w];
            s->dl[w] = l[i], s->dr[w] = r[i];
            w = (w + 1u) & (CTL - 1u);
            l[i] = xl, r[i] = xr;
        }
        s->w = w;
        return;
    }
    dg = (g1 - g0) / (int32_t)(n ? n : 1u);
    for (i = 0; i < n; i++) {
        int32_t xl = s->dl[w], xr = s->dr[w], g = i + 1u == n ? g1 : g0 + dg * (int32_t)(i + 1u);
        s->dl[w] = l[i], s->dr[w] = r[i];
        w = (w + 1u) & (CTL - 1u);
        if (g < 32768) {
            xl = (int32_t)(((int64_t)xl * g) >> 15);
            xr = (int32_t)(((int64_t)xr * g) >> 15);
        }
        l[i] = xl, r[i] = xr;
    }
    s->w = w;
}
/* MENU > USB LEVEL FIXED (for #42: record over USB with the speaker turned down): the mix goes to master_block at the
 * full MASTER level, USB audio takes that (audio.c uac_tap), and only then does MASTER scale what the DAC gets
 * (usb_fixed_dac). MASTER (0, the default): MASTER before master_block, as always (USB follows the knob) */
static volatile uint8_t fx_usb_fixed;
#define MASTER_FULL 4096                /* main.c: the MASTER knob's top, Q12 */
/* the MASTER pot (ADC 0..1023, smoothed in main.c) to its level, square law: 0 .. 4088 (Q12) */
static inline uint32_t master_of_pot(uint32_t k10) { return (k10 * k10) >> 8; }
static __attribute__((noinline)) void usb_fixed_dac(int32_t *out, uint32_t n)   /* audio ISR, after uac_tap */
{
    uint32_t i;
    int32_t m = (int32_t)song.master_q12;
    for (i = 0; i < 2u * n; i++)
        out[i] = (out[i] * m) >> 12;                /* (|out| <= 32767 after the soft clip: fits) */
}
static volatile uint8_t fx_lowcut;     /* settings: 1 LOWCUT 12 dB/oct ~110 Hz, 2 BASS+ (the small speaker):
                                        * 12 dB/oct ~220 Hz plus the harmonics of the bass (spk_bass) */
static int32_t lc_l1, lc_l2, lc_r1, lc_r2, dc_l, dc_r, dce_l, dce_r;

/* DC blocker (~2 Hz), always on: a leaky integrator of the input (Q6 state) subtracted from it.
 * The >> 12 step keeps its remainder (error feedback, 0..4095) and adds it to the next one, so no
 * part of the step is lost: the state follows the input exactly, down to 0 after the sound stops.
 * (A rounded step of (x - dc) / 4096 would stop moving at |x - dc| < 2048 and leave an offset of up
 * to +-31 at the output after silence.) */
static inline int32_t dc_block(int32_t x, int32_t *dc, int32_t *err)
{
    int32_t e = (x << 6) - *dc + *err, d = e >> 12;
    *err = e - (d << 12);
    *dc += d;
    return x - ((*dc + 32) >> 6);
}

static int32_t lce[4];
static inline int32_t lowcut1(int32_t x, int32_t *lc, int32_t *err, uint32_t sh)   /* x minus its one-pole low-pass */
{
    int32_t e = x - *lc + *err, d = e >> sh;
    *err = e - (d << sh);
    *lc += d;
    return x - *lc;
}

/* BASS+: what the speaker cannot play, heard through its harmonics. The bass below ~150 Hz is clipped at its
 * own envelope (a level-following trapezoid: odd harmonics), then band-passed ~220 Hz..1 kHz and added.
 * The low-pass has 4 poles (#42: with 2, the trapezoid rebuilt the 300 .. 600 Hz of the mix itself, late,
 * and cancelled up to 6 dB of it; now under 0.5 dB) */
static int32_t sb_lp1, sb_lp2, sb_lp3, sb_lp4, sb_env, sb_h1, sb_h2, sb_hl;
static inline int32_t spk_bass(int32_t m)
{
    int32_t a, t, u;
    sb_lp1 += ((m - sb_lp1) * 692) >> 15;
    sb_lp2 += ((sb_lp1 - sb_lp2) * 692) >> 15;
    sb_lp3 += ((sb_lp2 - sb_lp3) * 692) >> 15;
    sb_lp4 += ((sb_lp3 - sb_lp4) * 692) >> 15;
    a = sb_lp4 < 0 ? -sb_lp4 : sb_lp4;
    if (a > sb_env)
        sb_env += (a - sb_env) >> 2;
    else if (sb_env > 0)
        sb_env -= (sb_env >> 11) + 1;
    t = clamp(sb_lp4 * 8, -sb_env, sb_env);
    sb_h1 += (t - sb_h1) >> 5;
    u = t - sb_h1;
    sb_h2 += (u - sb_h2) >> 5;
    u -= sb_h2;
    sb_hl += (u - sb_hl) >> 3;
    return sb_hl + (sb_hl >> 1);      /* x1.5 (#180: x3 peaked at 1.5 .. 3.5 x a full-scale kick, the limiter */
}                                     /* pulled the mix down up to 16 dB on each hit and the buzz took over) */

/* the output stage before the limiter: the DC blocker, the speaker EQ */
static inline void master_pre(int32_t *l, int32_t *r)
{
    *l = dc_block(*l, &dc_l, &dce_l);
    *r = dc_block(*r, &dc_r, &dce_r);
    if (fx_lowcut) {                  /* two one-pole high-passes, error feedback as dc_block (the */
        uint32_t sh = fx_lowcut == 2u ? 5u : 6u;    /* rounded step stopped at |x - lc| < 32: an offset) */
        int32_t b = fx_lowcut == 2u ? spk_bass((*l + *r) >> 1) : 0;
        *l = lowcut1(*l, &lc_l1, &lce[0], sh);
        *l = lowcut1(*l, &lc_l2, &lce[1], sh) + b;
        *r = lowcut1(*r, &lc_r1, &lce[2], sh);
        *r = lowcut1(*r, &lc_r2, &lce[3], sh) + b;
    }
}

/* the master: n (<= CTL) samples of the mix in, out CTL samples late, limited and soft-clipped */
static void master_block(int32_t *l, int32_t *r, uint32_t n)
{
    uint32_t i;
    for (i = 0; i < n; i++) {
        master_pre(&l[i], &r[i]);
    }
    lim_block(&lim, l, r, n);
    for (i = 0; i < n; i++) {
        l[i] = softclip(l[i]);
        r[i] = softclip(r[i]);
    }
}

/* length of one division (N_DIV order) in samples at the song tempo */
static const uint8_t DIV_DEN[6] = {1, 2, 4, 8, 3, 6};    /* original IDs stay fixed; slow rates append */
static uint32_t midi_beat_samples;                    /* zero until an external clock has a measured tempo */
static uint32_t beat_samples(void)
{
    return song.g[G_CLOCK] && midi_beat_samples ? midi_beat_samples : (uint32_t)FS * 60u / (uint32_t)song.g[G_BPM];
}
static uint32_t div_samples(uint32_t div)
{
    uint32_t quarter = beat_samples();
    return div < 6u ? quarter / DIV_DEN[div] : div < 10u ? quarter << (div - 5u) : quarter / DIV_DEN[div % 6u];
}

#include "perform.c"                                 /* the FX hold layer's effects (the master) */
#include "tape.c"
#include "click.c"                                   /* the metronome's click (after the master: audio.c) */

/* the delay's TIME (params.c N_DTIME): N_DIV's ten (div_samples), then 1.5.1's dotted 1/8D 1/16D 1/4D (10, 11, 12: 3/4,
 * 3/8, 3/2 of a quarter); at most DLY_LEN - 1 samples (1.486 s): 1/4D clamps at 60 BPM and below (1/2 below 81) */
static const uint8_t DIV_DOT[3] = {2, 3, 1};           /* 3 quarters >> this */
#define DLY_SLEW 12                      /* each pole's time constant: 2^12 samples (~93 ms) */
#define DLY_Q 12                         /* the read point's fraction bits (65536 samples fit an int32) */

/* one pole's step toward its target: d / 2^DLY_SLEW rounded away from 0, so it arrives (an integer slew would stop
 * 2^DLY_SLEW short) and never overshoots */
static inline int32_t slew_step(int32_t d)
{
    int32_t m = (1 << DLY_SLEW) - 1;
    return d >= 0 ? (d + m) >> DLY_SLEW : -((-d + m) >> DLY_SLEW);
}

static uint32_t delay_samples(void)
{
    uint32_t d = (uint32_t)song.g[G_DTIME];
    uint32_t s = d - 10u < 3u ? (beat_samples() * 3u) >> DIV_DOT[d - 10u] : div_samples(d);
    return s < 16u ? 16u : s >= DLY_LEN ? DLY_LEN - 1u : s;
}

/* x * y >> 15 rounded toward 0: a feedback loop's tail dies out instead of settling at a few LSB (floor rounding kept
 * ROOM at +-41 for ever) */
static inline int32_t mulq15z(int32_t a, int32_t b)
{
    int32_t p = a * b;
    return (p + ((p >> 31) & 32767)) >> 15;
}

/* ROOM (G_RTYPE 0): 4 damped combs + 2 allpasses (Freeverb-like, mono), added to out */
static __attribute__((noinline)) void rev_room(const int32_t *rev_in, int32_t *out, uint32_t n)
{
    uint32_t i, k;
    int32_t size = 25000 + song.g[G_RSIZE] * 50, damp = 32767 - song.g[G_RDAMP] * 200;
    for (i = 0; i < n; i++) {
        int32_t a = 0;
        int16_t *c = rev_comb;
        int32_t in = mulq15(rev_in[i], 2580);           /* 1/8 at -4 dB: level as before the allpass fix */
        for (k = 0; k < 4u; k++) {
            int32_t o = c[fx.comb_i[k]];
            fx.comb_lp[k] = o + mulq15z(fx.comb_lp[k] - o, 32767 - damp);
            c[fx.comb_i[k]] = (int16_t)clamp(in + mulq15z(fx.comb_lp[k], size), -32768, 32767);
            if (++fx.comb_i[k] >= REV_COMB[k])
                fx.comb_i[k] = 0;
            a += o;
            c += REV_COMB[k];
        }
        c = rev_ap;
        for (k = 0; k < 2u; k++) {
            int32_t o = c[fx.ap_i[k]];
            int32_t v = a + ((o + (o < 0)) >> 1);       /* (o / 2 toward 0, see mulq15z; no divide) */
            c[fx.ap_i[k]] = (int16_t)clamp(v, -32768, 32767);
            a = o - a;                                  /* Freeverb: out = buf - in (o - v is a notch comb) */
            if (++fx.ap_i[k] >= REV_AP[k])
                fx.ap_i[k] = 0;
            c += REV_AP[k];
        }
        out[i] += a;
    }
}

/* STRINGS (G_RTYPE 3, GHOULBOX desert): sympathetic strings, as a tanbur's or an oud's open courses ring behind
 * what is played. ROOM's four comb lines, each tuned (a fractional, interpolated delay) to a degree of the scale
 * of the track holding the lowest note: that note, and its third, fifth and seventh scale degrees above (the
 * maqam's own intervals, SCALE_CENTS and all: tuning_pitch16). No note held: the last tuning stays (at power-on
 * the selected track's ROOT, octave 3). A new tuning is reached by a glide (ST_GLIDE, ~40 ms a fifth), so chord
 * changes bend the ringing. SIZE the strings' sustain (the feedback: ~0.3 s .. ~6 s at C3; lower strings ring
 * longer), DAMP their brightness (ROOM's one-pole in the loop). Mono, added to out, no allpasses (they would
 * smear the tone) */
#define ST_GLIDE 16                      /* Q8 samples a sample the delay moves toward its tuning */
#define ST_MIN_LEN (24 * 256)            /* the shortest line (~1.8 kHz) */
#define ST_DEGREES 4

static int32_t scale_snap(const track_t *t, int32_t n);   /* (seq.c, after this file) */
static int32_t scale_up(const track_t *t, int32_t n);

/* the lowest note held on a pitched part, and that part; none: 0 and 0 */
static int32_t st_lowest(const track_t **tp)
{
    uint32_t k, i;
    int32_t low = 128;
    *tp = 0;
    for (k = 0; k < NTRK; k++) {
        const track_t *t = &trk[k];
        if (t->eng_req == ENGI_DRUM || t->eng_req == ENGI_SLICE) {
            continue;
        }
        for (i = 0; i < NVOICE; i++) {
            const voice_t *v = &t->v[i];
            if (v->gate && v->note < low) {
                low = v->note;
                *tp = t;
            }
        }
    }
    return low < 128 ? low : 0;
}

/* the four lines' tunings from the lowest held note (a block's start) */
static void st_tune(void)
{
    const track_t *t;
    int32_t n = st_lowest(&t), k;
    if (!t) {
        if (fx.st_tgt[0]) {
            return;                                     /* (keeps ringing as tuned) */
        }
        t = &trk[song.sel];
        n = 48 + t->p[P_ROOT];
    }
    n = scale_snap(t, clamp(n, 36, 108));
    for (k = 0; k < ST_DEGREES; k++) {
        uint32_t inc = pitch_inc((uint32_t)tuning_pitch16(t, (uint32_t)n)) | 1u;
        int32_t len = (int32_t)(((uint64_t)1 << 40) / inc);          /* the period, Q8 samples */
        fx.st_tgt[k] = clamp(len, ST_MIN_LEN, ((int32_t)REV_COMB[k] - 2) * 256);
        if (!fx.st_len[k]) {
            fx.st_len[k] = fx.st_tgt[k];                /* (from silence: no glide in) */
        }
        n = scale_up(t, scale_up(t, n));
    }
}

static __attribute__((noinline)) void rev_strings(const int32_t *rev_in, int32_t *out, uint32_t n)
{
    uint32_t i, k;
    int32_t fb = 27460 + song.g[G_RSIZE] * 40, damp = song.g[G_RDAMP] * 200;
    st_tune();
    for (i = 0; i < n; i++) {
        int32_t a = 0;
        int16_t *c = rev_comb;
        int32_t in = mulq15(rev_in[i], 2580);
        for (k = 0; k < 4u; k++) {
            int32_t len = (int32_t)REV_COMB[k], d = fx.st_tgt[k] - fx.st_len[k];
            int32_t w = fx.comb_i[k], r, r1, o, o1;
            fx.st_len[k] += clamp(d, -ST_GLIDE, ST_GLIDE);
            r = w - (fx.st_len[k] >> 8);                /* the read point, fx.st_len behind the write */
            r += r < 0 ? len : 0;
            r1 = r - 1 + (r == 0 ? len : 0);
            o = c[r];
            o1 = c[r1];
            o += ((o1 - o) * (fx.st_len[k] & 255)) >> 8;
            fx.comb_lp[k] = o + mulq15z(fx.comb_lp[k] - o, damp);
            c[w] = (int16_t)clamp(in + mulq15z(fx.comb_lp[k], fb), -32768, 32767);
            if (++w >= len) {
                w = 0;
            }
            fx.comb_i[k] = (uint16_t)w;
            a += o;
            c += len;
        }
        out[i] += a;
    }
}

/* SPRING (see the top), added to out */
static __attribute__((noinline)) void rev_spring(const int32_t *rev_in, int32_t *out, uint32_t n)
{
    uint32_t i, k, s = (uint32_t)song.g[G_RSIZE];
    int32_t g = 19661 + (int32_t)s * 85;                /* the loop's gain: 0.6 .. 0.93 */
    int32_t kl = 26000 - song.g[G_RDAMP] * 160;         /* its low-pass: ~9 kHz .. ~1.3 kHz */
    int32_t len = (int32_t)(1323u + ((s * 1323u) >> 7)) << 8, L, L2, L3, f, w;
    int16_t *ln = rev_comb;
    int32_t *ap = rev_u.sp;
    if (!fx.sp_size)
        fx.sp_size = len;
    fx.sp_size += clamp(len - fx.sp_size, -256, 256);   /* SIZE glides (a sample a block at most) */
    L = fx.sp_size >> 8;
    fx.sp_ph += 2u * LFO_INC[24];                       /* the wobble: a slow sine, 1.5 samples deep */
    w = (fx.sp_size >> 1) + ((osc_sine(fx.sp_ph) * 3) >> 8);   /* the far end, Q8 */
    L2 = w >> 8;
    f = w & 255;
    L3 = (L * 3) >> 2;
    for (i = 0; i < n; i++) {
        uint32_t wp = fx.sp_w, j = (wp & 3u) * (SP_N + 1u);
        int32_t x = mulq15(rev_in[i], 2580), r = ln[(wp - (uint32_t)L) & SP_MASK], p, o;
        int32_t t0 = ln[(wp - (uint32_t)L2) & SP_MASK], t1 = ln[(wp - (uint32_t)L2 - 1u) & SP_MASK];
        fx.sp_lp += mulq15(r - fx.sp_lp, kl);
        o = fx.sp_lp * g;
        x += (o + ((o >> 31) & 32767)) >> 15;           /* towards 0: a loop of floors would hold an offset */
        o = x - fx.sp_hp + fx.sp_he;                    /* the low cut, its step's remainder kept (as */
        fx.sp_he = o & 63;                              /* dc_block): no dead band to hold an offset in the loop */
        fx.sp_hp += o >> 6;
        x -= fx.sp_hp;
        p = ap[j];                                      /* the chain: ap[j + k], stage k's output 4 samples ago */
        ap[j] = x;
        for (k = 1; k <= SP_N; k++) {                   /* (lossless: bounded by the loop's input, no clamp) */
            int32_t v = (x - ap[j + k]) * SP_A;         /* towards 0, as the loop's gain: floors would feed */
            o = ap[j + k];                              /* the loop a little offset and noise for ever */
            x = ((v + ((v >> 31) & 4095)) >> 12) + p;
            p = o;
            ap[j + k] = x;
        }
        ln[wp & SP_MASK] = (int16_t)clamp(x, -32768, 32767);
        fx.sp_w = (uint16_t)(wp + 1u);
        out[i] += (t0 + (((t1 - t0) * f) >> 8)) * 4 + ln[(wp - (uint32_t)L3) & SP_MASK] * 2;
    }
}

/* HALL's helpers: m / 2^sh rounded towards zero (a product that feeds back must not floor: a loop of floors
 * holds an offset and a little noise for ever) */
static inline int32_t sh_tz(int32_t m, uint32_t sh) { return (m + ((m >> 31) & ((1 << sh) - 1))) >> sh; }
/* 2^-x, x Q16 (0 .. 16): Q15 (32768 at x = 0). 2^-x = 2^u / 2^(i + 1), u = 1 - frac(x); 2^u by a cubic (1e-4) */
static uint32_t exp2_neg(uint32_t x)
{
    uint32_t i = x >> 16, u = 32768u - ((x & 0xFFFFu) >> 1), p = 2579u;
    p = ((p * u) >> 15) + 7409u;
    p = ((p * u) >> 15) + 22780u;
    p = ((p * u) >> 15) + 32768u;
    return i < 15u ? p >> (i + 1u) : 0u;
}
/* the lines' gains for SIZE s: RT60 = 0.5 s * 2^(s / 30); a line of D samples (22.05 kHz) loses 60 dB in
 * RT60 * 22050 / D passes: g = 2^(-D * 3 log2(10) / (22050 * RT60)) = 2^(-D * 59.2 / 65536 * 2^(-s / 30)),
 * at most 0.99, over sqrt(7) (the matrix's rows have 7 entries of +-1: / sqrt(7) makes it orthogonal) */
static __attribute__((noinline)) void hall_gains(int32_t s)
{
    uint32_t q = exp2_neg((uint32_t)s * 65536u / 30u), k, g;
    for (k = 0; k < HL_NL; k++) {
        g = exp2_neg((HL_LEN[k] * 59u * q) >> 15);
        hl.g[k] = (int16_t)(((g > 32440u ? 32440u : g) * 6193u) >> 15);
    }
    hl.sz = (int16_t)(s + 1);
}

/* an allpass diffuser over x[0 .. n) from q (its next samples, no wrap): its state rounded towards zero */
static inline void hall_ap(int16_t *q, int32_t g, int32_t *x, uint32_t n)
{
    uint32_t i;
    for (i = 0; i < n; i++) {
        int32_t y = q[i], v = x[i] + sh_tz(y * g, 14);
        q[i] = (int16_t)clamp(v, -32768, 32767);
        x[i] = y - ((v * g + 8192) >> 14);
    }
}
/* a line's end over y[0 .. n) from b (no wrap; b[n] is still the line's: its mirror at the end): between b[i]
 * and b[i + 1], f / 256 of the way (rounded to the nearest: it stays between the two), times its gain g (Q14).
 * The gain's product: rounded to the nearest while the line is loud (lo 0: the tail decays as it should; a
 * truncation would make it die early, the sooner the quieter), towards zero once it is quiet (lo 1: the last
 * steps of the tail, which then ends in exact silence) */
static inline void hall_end(const int16_t *b, int32_t f, int16_t *y, uint32_t n, int32_t g, int lo)
{
    uint32_t i;
    if (lo)
        for (i = 0; i < n; i++)
            y[i] = (int16_t)sh_tz((b[i] + (((b[i + 1u] - b[i]) * f + 128) >> 8)) * g, 14);
    else
        for (i = 0; i < n; i++)
            y[i] = (int16_t)(((b[i] + (((b[i + 1u] - b[i]) * f + 128) >> 8)) * g + 8192) >> 14);
}
/* a line's end over y[0 .. n) from b (no wrap), through the low-pass (kd, Q15; the state *lp and its step's
 * remainder *le), times its gain g (rounded as hall_end's) */
static inline void hall_end_lp(const int16_t *b, int16_t *y, uint32_t n, int32_t g, int lo, int32_t *lp, int32_t *le,
                               int32_t kd)
{
    uint32_t i;
    int32_t l = *lp, r = *le, e;
    if (lo)
        for (i = 0; i < n; i++) {
            e = (b[i] - l) * kd + r, r = e & 32767, l += e >> 15;
            y[i] = (int16_t)sh_tz(l * g, 14);
        }
    else
        for (i = 0; i < n; i++) {
            e = (b[i] - l) * kd + r, r = e & 32767, l += e >> 15;
            y[i] = (int16_t)((l * g + 8192) >> 14);
        }
    *lp = l;
    *le = r;
}
/* the block's HL_NC samples of a circular buffer of len from index d in at most two runs, no wrap inside them:
 * body with s (the run's start in the block) and k (its length); d moves on */
#define HL_RUNS(d, len, k, s, body)                                                         \
    for (s = 0; s < HL_NC; s += k) {                                                        \
        k = (len) - (d) < HL_NC - s ? (len) - (d) : HL_NC - s;                              \
        body;                                                                               \
        d = (d) + k >= (len) ? 0u : (d) + k;                                                \
    }

/* HALL (see the top): a block of the send (n = CTL samples: audio.c) -> the mid added to out, the side into
 * rev_side. At half the rate (HL_NC samples) in passes over the block, each with few states (every line is
 * longer than a block: what the lines give in this block was written before it): the send, every other
 * sample, low cut, diffusers (hl.bx); the lines' ends at their drifting taps, low-passed, scaled (hl.y); the
 * outputs and the matrix; back into the lines; the outputs back to 44.1 kHz. */
#define HL_LFO ((uint32_t)(0.37 * CTL / FS * 4294967296.0))   /* the drift: 0.37 Hz (even lines), 0.74 Hz (odd) */
static __attribute__((noinline)) void rev_hall(const int32_t *rev_in, int32_t *out, uint32_t n)
{
    int16_t *b = rev_comb, *a = rev_ap;
    uint32_t i, k, s, m, d;
    int32_t kd = HL_KD(song.g[G_RDAMP]);
    int32_t xp = hl.x, hp = hl.hp, he = hl.he, mp = hl.m, sp = hl.s;
    (void)n;
    if (hl.sz != song.g[G_RSIZE] + 1)
        hall_gains(song.g[G_RSIZE]);
    for (i = 0; i < HL_NC; i++) {
        int32_t x = (xp + 2 * rev_in[2u * i] + rev_in[2u * i + 1u]) >> 2, v;   /* every other sample, 1 2 1 */
        xp = rev_in[2u * i + 1u];
        x = mulq15(clamp(x, -262143, 262143), 2580);    /* (ROOM's level) */
        v = x - hp + he;                                /* the low cut, its step's remainder kept */
        he = v & 31;
        hp += v >> 5;
        hl.bx[i] = x - hp;
    }
    hl.x = xp, hl.hp = hp, hl.he = he;
    for (k = 0; k < HL_NAP; k++) {                      /* the diffusers */
        d = hl.d[k];
        HL_RUNS(d, HL_AP[k], m, s, hall_ap(a + d, HL_APG[k], hl.bx + s, m));
        hl.d[k] = (uint16_t)d;
        a += HL_AP[k];
    }
    hl.ph += HL_LFO;
    for (k = 0; k < HL_NL; k++) {                       /* the lines' ends */
        int32_t g = hl.g[k], f = 0, lo;
        if ((HL_LPM >> k) & 1u)
            d = hl.i[k];                                /* (low-passed: the line's end) */
        else {                                          /* (drifting: HL_MOD + 2 +- HL_MOD samples short of it) */
            uint32_t r = (uint32_t)(((HL_MOD + 2) << 8) +
                                    ((osc_sine(hl.ph * (1u + (k & 1u)) + k * 0x2C000000u) * HL_MOD) >> 7));
            f = (int32_t)(r & 255u);
            d = hl.i[k] + (r >> 8);
            d = d >= HL_LEN[k] ? d - HL_LEN[k] : d;
        }
        s = d + HL_LEN[k] / 2u;                         /* quiet? (two samples half a line apart) */
        s = s >= HL_LEN[k] ? s - HL_LEN[k] : s;
        lo = (b[d] < 0 ? -b[d] : b[d]) + (b[s] < 0 ? -b[s] : b[s]) < HL_LOUD;
        if ((HL_LPM >> k) & 1u) {
            HL_RUNS(d, HL_LEN[k], m, s, hall_end_lp(b + d, hl.y[k] + s, m, g, lo, &hl.lp[k], &hl.le[k], kd));
        } else {
            HL_RUNS(d, HL_LEN[k], m, s, hall_end(b + d, f, hl.y[k] + s, m, g, lo));
        }
        b += HL_LEN[k] + 1u;
    }
    for (i = 0; i < HL_NC; i++) {
        int32_t y0 = hl.y[0][i], y1 = hl.y[1][i], y2 = hl.y[2][i], y3 = hl.y[3][i];
        int32_t y4 = hl.y[4][i], y5 = hl.y[5][i], y6 = hl.y[6][i], y7 = hl.y[7][i], x = hl.bx[i];
        int32_t t, p0, p1, p2, p3, p4, p5, p6;
        hl.l[i] = y0 + y3 + y4 - y7;                    /* the outputs */
        hl.r[i] = y1 + y2 + y5 + y6;
        /* the matrix: C = [0 1; -1 Q], Q (7 x 7) = chi(j - i), the quadratic character mod 7 (+1 at distances
         * 1 2 4, -1 at 3 5 6): row 1 + i of Q y = 2 (the three at +1) - (the sum of the 7) + y_1+i */
        t = y1 + y2 + y3 + y4 + y5 + y6 + y7;
        p0 = y2 + y3 + y5, p1 = y3 + y4 + y6, p2 = y4 + y5 + y7, p3 = y5 + y6 + y1;
        p4 = y6 + y7 + y2, p5 = y7 + y1 + y3, p6 = y1 + y2 + y4;
        y0 += t;
        hl.y[0][i] = (int16_t)clamp(t + x, -32768, 32767);   /* (+ the send: signs spread over the rows) */
        hl.y[1][i] = (int16_t)clamp(2 * p0 + y1 - y0 + x, -32768, 32767);
        hl.y[2][i] = (int16_t)clamp(2 * p1 + y2 - y0 + x, -32768, 32767);
        hl.y[3][i] = (int16_t)clamp(2 * p2 + y3 - y0 + x, -32768, 32767);
        hl.y[4][i] = (int16_t)clamp(2 * p3 + y4 - y0 + x, -32768, 32767);
        hl.y[5][i] = (int16_t)clamp(2 * p4 + y5 - y0 + x, -32768, 32767);
        hl.y[6][i] = (int16_t)clamp(2 * p5 + y6 - y0 + x, -32768, 32767);
        hl.y[7][i] = (int16_t)clamp(2 * p6 + y7 - y0 - x, -32768, 32767);
    }
    b = rev_comb;
    for (k = 0; k < HL_NL; k++) {                       /* back into the lines (and the mirror after a wrap) */
        d = hl.i[k];
        HL_RUNS(d, HL_LEN[k], m, s, for (i = 0; i < m; i++) b[d + i] = hl.y[k][s + i]; if (!d) b[HL_LEN[k]] = b[0]);
        hl.i[k] = (uint16_t)d;
        b += HL_LEN[k] + 1u;
    }
    for (i = 0; i < HL_NC; i++) {                       /* back to 44.1 kHz, between samples */
        int32_t mm = (hl.l[i] + hl.r[i]) * HL_OUT, ss = (hl.l[i] - hl.r[i]) * HL_OUT;
        out[2u * i] += (mp + mm) >> 1;
        out[2u * i + 1u] += mm;
        rev_side[2u * i] = (sp + ss) >> 1;
        rev_side[2u * i + 1u] = ss;
        mp = mm;
        sp = ss;
    }
    hl.m = mp, hl.s = sp;
    hl.side = 1;
}

/* the reverb's buffers and states to silence (the model changed) */
static void rev_clear(void)
{
    uint32_t i;
    for (i = 0; i < sizeof rev_comb / 2u; i++)
        rev_comb[i] = 0;
    for (i = 0; i < sizeof rev_u.ap / 2u; i++)          /* (int16: 997 of them, an odd count the int32 view misses one of) */
        rev_u.ap[i] = 0;
    for (i = 0; i < 4u; i++) {
        fx.comb_lp[i] = 0;
        fx.st_len[i] = fx.st_tgt[i] = 0;                 /* STRINGS: tuned afresh, no glide in */
    }
    for (i = 0; i < HL_NL; i++)
        hl.lp[i] = hl.le[i] = 0;
    fx.sp_lp = fx.sp_hp = fx.sp_he = 0;
    hl.x = hl.m = hl.s = hl.hp = hl.he = 0;
}

static int32_t part_buf[CTL];                            /* a part's block (mix_part); the fade of a model change */
static int32_t c1_buf[CTL];                              /* (JIANT 0.6.71) the second core's part_buf (mix_part) */

/* the model changed (to rt): the old one's block fades out (HALL's stereo difference with it), its buffers are
 * cleared, the new one starts from silence */
static __attribute__((noinline)) void rev_switch(const int32_t *rev_in, int32_t *wet, uint32_t n, int32_t rt)
{
    int32_t *t = part_buf, g = 65536, d = 65536 / (int32_t)n;
    uint32_t i;
    for (i = 0; i < n; i++)
        t[i] = 0;
    if (fx.rtype == 3u)
        rev_strings(rev_in, t, n);
    else if (fx.rtype == 2u)
        rev_hall(rev_in, t, n);
    else if (fx.rtype)
        rev_spring(rev_in, t, n);
    else
        rev_room(rev_in, t, n);
    for (i = 0; i < n; i++, g -= d) {
        wet[i] += mulq16(t[i], (uint32_t)g);
        rev_side[i] = mulq16(rev_side[i], (uint32_t)g);   /* (HALL only: hl.side set) */
    }
    rev_clear();
    fx.rtype = (uint8_t)rt;
}

/* a bus with nothing to do is skipped: its input 0 a block and, for long enough before, its state at most BUS_QUIET
 * (-84 dBFS). The chorus: its buffer all 0 (exact); the delay: every sample of its buffer read; a reverb: a second.
 * Skipped, the state stays as it was (a leftover of +-BUS_QUIET at most, under what comes next) */
enum { BUS_CHO, BUS_DLY, BUS_REV, BUS_N };
#define BUS_QUIET 2
static const uint32_t BUS_HOLD[BUS_N] = {CHO_LEN, DLY_LEN, FS};
static uint32_t bus_q[BUS_N];                         /* samples each has been quiet (input 0, state <= BUS_QUIET) */
static uint8_t bus_skip = 1;                          /* 0: every bus always runs (tests: the reference) */
static int32_t rev_tmp[CTL];

static int blk_zero(const int32_t *x, uint32_t n)
{
    int32_t a = 0;
    uint32_t i;
    for (i = 0; i < n; i++) {
        a |= x[i];
    }
    return !a;
}
/* bus b, its input zero or not this block: skip it? */
static int bus_idle(uint32_t b, int zero)
{
    if (!zero) {
        bus_q[b] = 0;
    }
    return bus_skip && zero && bus_q[b] >= BUS_HOLD[b];
}
/* .. after it ran: its state this block at most BUS_QUIET (loud 0) or not */
static void bus_settle(uint32_t b, int zero, int loud, uint32_t n)
{
    if (!zero || loud) {
        bus_q[b] = 0;
        return;
    }
    if (bus_q[b] < BUS_HOLD[b]) {
        bus_q[b] += n;
    }
}
/* process the three buses for one block; sends in, wet stereo-equal out */
/* the GRAIN delay (GHOULBOX desert, after JIANT; G_DPIT / G_DSPRY not 0): the line is read by two grains of GR_LEN
 * samples, half a grain apart, each faded in and out (triangles: their sum is flat), each sweeping its read point so it
 * plays PITCH semitones up / down (as a tape pitch shifter); a grain starting again jumps back, SPRAY further by a random
 * amount (up to ~190 ms). In the feedback: each repeat a further PITCH away (shimmer, falling echoes), scattered. The
 * same line (dly_buf), no RAM of its own */
#define GR_LEN 2048u
static const uint32_t GR_RATIO[25] = {32768, 34716, 36781, 38968, 41285, 43740, 46341, 49097, 52016, 55109, 58386, 61858,
                                      65536, 69433, 73562, 77936, 82570, 87480, 92682, 98193, 104032, 110218, 116772,
                                      123715, 131072};   /* 2^(st/12), Q16 */
static __attribute__((noinline)) int32_t dly_grain(uint32_t dl)
{
    int32_t s = (int32_t)GR_RATIO[clamp(song.g[G_DPIT], -12, 12) + 12] - 65536, x = 0;
    uint32_t j;
    if (!fx.gr_ready) {
        fx.gr_s[0] = fx.gr_s[1] = s;
        fx.gr_ready = 1;
    }   /* the read point's sweep over a grain, Q8 */
    fx.gr_ph = (fx.gr_ph + 65536u / GR_LEN) & 0xFFFFu;
    for (j = 0; j < 2u; j++) {
        uint32_t ph = (fx.gr_ph + j * 32768u) & 0xFFFFu, d, di, gl = dl;
        int32_t a, b, w;
        uint32_t e;
        if (ph < 65536u / GR_LEN) {                     /* zero-weight boundary: latch pitch and spray */
            fx.gr_s[j] = s;
            fx.gr_sp[j] = song.g[G_DSPRY] ? rng() % ((uint32_t)song.g[G_DSPRY] * 64u + 1u) : 0u;
        }
        if (gl + fx.gr_sp[j] > DLY_LEN - 2u - GR_LEN * 2u) {   /* a long TIME (the whole line): room for the sweep */
            gl = DLY_LEN - 2u - GR_LEN * 2u - fx.gr_sp[j];
        }
        e = (GR_LEN * (uint32_t)(fx.gr_s[j] < 0 ? -fx.gr_s[j] : fx.gr_s[j])) >> 8;
        d = ((gl + fx.gr_sp[j]) << 8) + ((e * ((fx.gr_s[j] > 0 ? 65535u - ph : ph) >> 4)) >> 12);   /* Q8: up, the delay shrinks
                                                         * over the grain; down, it grows */
        di = d >> 8;
        if (di > DLY_LEN - 2u) {
            di = DLY_LEN - 2u;
        }
        a = dly_buf[(fx.dly_w - di) & (DLY_LEN - 1u)];
        b = dly_buf[(fx.dly_w - di - 1u) & (DLY_LEN - 1u)];
        w = (int32_t)(ph < 32768u ? ph : 65535u - ph);  /* the triangle, 0 .. 32767 */
        x += ((a + (((b - a) * (int32_t)(d & 255u)) >> 8)) * w) >> 15;
    }
    return x;
}

static void fx_buses(const int32_t *cho_in, const int32_t *dly_in, const int32_t *rev_in, int32_t *wet,
                     uint32_t n)
{
    uint32_t i, dl = delay_samples();
    int32_t fb = song.g[G_DFDBK] * 230, col = 2000 + song.g[G_DCOLOR] * 240;
    int32_t dmix = song.g[G_DMIX] * 258;
    int32_t gr = song.g[G_DPIT] || song.g[G_DSPRY];     /* the GRAIN delay (dly_grain) */
    int32_t cdepth = song.g[G_CDEPTH] * 6, rt;
    if (!gr) fx.gr_ready = 0;
    uint32_t cinc = LFO_INC[song.g[G_CRATE] & 127] / CTL;
    int zc = blk_zero(cho_in, n), zd = blk_zero(dly_in, n), zr = blk_zero(rev_in, n);
    if (pf.ew1 && fb < THR_FB)
        fb += mulq16(THR_FB - fb, (uint32_t)pf.ew1 << 1);   /* ECHO THROW (perform.c): more feedback while held, 0.7
                                                             * at most (the loop's gain below 1: it never runs away) */
    for (i = 0; i < n; i++) {
        wet[i] = 0;
    }

    /* chorus: modulated short delay, 5..15 ms */
    if (bus_idle(BUS_CHO, zc)) {
        fx.cho_ph += cinc * n;
        fx.cho_w += n;
    } else {
        for (i = 0; i < n; i++) {
            int32_t r, f, c0, c1;
            uint32_t ri;
            cho_buf[fx.cho_w & (CHO_LEN - 1u)] = (int16_t)clamp(cho_in[i] >> 1, -32768, 32767);
            fx.cho_ph += cinc;
            r = (400 << 8) + ((osc_sine(fx.cho_ph) + 32768) * cdepth >> 8);   /* Q8 delay: read between samples */
            ri = (uint32_t)r >> 8;
            f = r & 255;
            c0 = cho_buf[(fx.cho_w - ri) & (CHO_LEN - 1u)];
            c1 = cho_buf[(fx.cho_w - ri - 1u) & (CHO_LEN - 1u)];
            wet[i] += (c0 + (((c1 - c0) * f) >> 8)) << 1;
            fx.cho_w++;
        }
        bus_settle(BUS_CHO, zc, 0, n);                  /* (its buffer 0 once its input was 0 for CHO_LEN) */
    }

    /* delay with a low-passed feedback; loud: what it read was above BUS_QUIET */
    if (bus_idle(BUS_DLY, zd)) {
        fx.dly_w += n;
        fx.dl_s1 = fx.dl_s2 = (int32_t)dl << DLY_Q;    /* (silent: the motor is there already) */
    } else {
        uint32_t loud = 0;
        int32_t tgt = (int32_t)dl << DLY_Q;
        if (!fx.dl_s2)
            fx.dl_s1 = fx.dl_s2 = tgt;
        for (i = 0; i < n; i++) {
            int32_t x;
            uint32_t rd;
            fx.dl_s1 += slew_step(tgt - fx.dl_s1);
            fx.dl_s2 += slew_step(fx.dl_s1 - fx.dl_s2);
            rd = (uint32_t)fx.dl_s2 >> DLY_Q;
            if (gr) {
                x = dly_grain(rd);
            } else {
                int32_t c0 = dly_buf[(fx.dly_w - rd) & (DLY_LEN - 1u)];
                int32_t c1 = dly_buf[(fx.dly_w - rd - 1u) & (DLY_LEN - 1u)];
                x = c0 + (((c1 - c0) * ((fx.dl_s2 >> (DLY_Q - 8)) & 255)) >> 8);   /* (between samples while it moves) */
            }
            fx.dly_lp += mulq15(x - fx.dly_lp, col);
            dly_buf[fx.dly_w & (DLY_LEN - 1u)] =
                (int16_t)clamp((dly_in[i] >> 1) + mulq15(fx.dly_lp, fb), -32768, 32767);
            fx.dly_w++;
            loud |= (uint32_t)(x + BUS_QUIET) > 2u * BUS_QUIET;
            wet[i] += mulq15(x << 1, dmix);
        }
        bus_settle(BUS_DLY, zd, (int)loud, n);
    }

    /* the reverb: its input 0, into rev_tmp first to see whether it is quiet */
    rt = song.g[G_RTYPE];
    rt = rt >= 1 && rt <= 3 ? rt : 0;                    /* ROOM, SPRING, HALL, STRINGS (GHOULBOX desert) */
    hl.side = 0;
    if (rt != fx.rtype) {
        bus_q[BUS_REV] = 0;
        rev_switch(rev_in, wet, n, rt);
        return;
    }
    if (bus_idle(BUS_REV, zr)) {
        return;
    }
    {
        int32_t *o = zr ? rev_tmp : wet;
        uint32_t loud = 0;
        if (zr) {
            for (i = 0; i < n; i++) {
                o[i] = 0;
            }
        }
        if (rt == 3)
            rev_strings(rev_in, o, n);
        else if (rt == 2)
            rev_hall(rev_in, o, n);
        else if (rt)
            rev_spring(rev_in, o, n);
        else
            rev_room(rev_in, o, n);
        if (zr) {
            for (i = 0; i < n; i++) {
                loud |= (uint32_t)(o[i] + BUS_QUIET) > 2u * BUS_QUIET;
                wet[i] += o[i];
            }
            for (i = 0; hl.side && i < n; i++) {        /* (HALL: its stereo difference too) */
                loud |= (uint32_t)(rev_side[i] + BUS_QUIET) > 2u * BUS_QUIET;
            }
        }
        bus_settle(BUS_REV, zr, (int)loud, n);
    }
}

/* after the buses: HALL's stereo difference to the left and from the right of the dry mix (the wet bus adds the
 * same to both) */
static __attribute__((noinline)) void rev_side_mix(int32_t *l, int32_t *r, uint32_t n)
{
    uint32_t i;
    for (i = 0; i < n; i++) {
        l[i] += rev_side[i];
        r[i] -= rev_side[i];
    }
}

/* INSERT (1.5, Discussions #78 and #177): one effect per track, after DIST and before the SLICER, with a dry / wet
 * MIX. TYPE (P_ITYPE, N_ITYPE; stored, append-only) and its three values A B C (P_IA .. P_IC, what they mean: params.c
 * ins_desc):
 *   SOFT HARD FOLD FUZZ  DRIVE (1x .. ~13x, squared), TONE (two one-pole low-passes at F_CUTOFF), LEVEL (F_DB, as the
 *                        track's LEVEL: 112 is 0 dB). A slow low cut in front (~27 Hz), a DC blocker after (FUZZ is
 *                        lopsided). SOFT tanh, HARD a flat clip at a voice's level, FOLD a triangle folder (the wave
 *                        reflected at +-16384: every fold more harmonics), FUZZ a hard top and a soft, halved bottom;
 *   CRUSH                BITS (1 .. 16, of a voice's full scale), RATE (a held sample, 689 Hz .. 44.1 kHz, INS_HOLD:
 *                        aliasing as the old samplers), LPF (two one-pole low-passes at F_CUTOFF after them, #177);
 *   PHASR                RATE (F_LFOHZ), DEPTH (the sweep around ~850 Hz: 0 still .. the whole 150 Hz .. 4.8 kHz),
 *                        FDBK (0 .. 0.7): the FX layer's four all-pass stages (perform.c PH_A), mono, per track;
 *   FLANG CHOR           RATE, DEPTH, FDBK: a 256-sample line per track (5.8 ms); FLANG swept 0.25 .. 5 ms (the FX
 *                        layer's squared triangle), feedback 0 .. 0.85; CHOR a sine around 3.2 ms (+-2.3 ms at DEPTH
 *                        127), feedback 0 .. 0.4. A DC blocker on the swept copy (a clipped loud input is lopsided).
 * The swept ones are the pedals': their wet is (dry + swept) / 2, the notches need both, so MIX 100 % is the whole
 * effect and less a lighter one. MIX (P_IMIX) blends dry and wet linearly; MIX 0 or TYPE OFF: the track bit for bit
 * as before (nothing runs). MIX glides (2.9 ms, SL_SLOPE); a TYPE change fades the old one out, clears the state,
 * fades the new one in; so does a start from MIX 0 (no stale line or filter). With SPREAD (mix_spread) the insert
 * takes the mono sum b, as DIST does; the side passes it by.
 * Order: DIST -> INSERT -> SLICER: the insert colours the sound as DIST does; the SLICER's gate (and its STUT
 * recordings) comes after, so its edges stay sharp (no flanger feedback or phaser ringing smears them) and a STUT
 * repeats the processed sound. State in the pool, per track (ins[], 580 B each); noinline, its own loops: the audio
 * ISR's inlined path gains one test per part (track_t ins_run) */
enum { IT_OFF, IT_SOFT, IT_HARD, IT_FOLD, IT_FUZZ, IT_CRUSH, IT_PHASER, IT_FLANGER, IT_CHORUS, IT_AMP, IT_N };
#define IL_LEN 256u                     /* FLANG / CHOR: the line, samples (a power of 2: masked) */
static const uint8_t INS_HOLD[16] = {64, 48, 40, 32, 24, 20, 16, 12, 10, 8, 6, 5, 4, 3, 2, 1};   /* CRUSH RATE */
typedef struct {
    int32_t w;                          /* the wet share now, Q15 */
    uint8_t type;                       /* the type running */
    uint8_t cnt;                        /* CRUSH: samples left of the held one */
    uint16_t wp;                        /* FLANG / CHOR: the line's next write */
    uint32_t ph;                        /* the sweep's phase */
    int32_t hp, lp1, lp2, dc, hold;     /* low cut (Q8), the two low-passes, DC blocker (Q8), CRUSH's held sample */
    int32_t px[PH_ST], py[PH_ST], fb;   /* PHASR: each stage's last input and output, the fed back */
    int16_t line[IL_LEN];               /* FLANG / CHOR, at a quarter (headroom) */
} ins_t;
static ins_t ins[NTRK] __attribute__((section(".pool")));

static inline int32_t ins_mixq(int32_t m) { return m >= 127 ? 32768 : m <= 0 ? 0 : m * 258; }   /* MIX -> Q15 */
/* the one-pole low-pass (topology-preserving) coefficient of cutoff index c, Q16: G = g / (1 + g), g = tan(pi fc / FS) */
static int32_t ins_lpk(int32_t c)
{
    uint32_t g = SVF_G[clamp(c, 0, 127)];
    return (int32_t)((g << 16) / (4096u + g));
}
/* the drives' and CRUSH's shape of v (a voice's full scale 32768; v already through DRIVE): the curve the INSERT
 * graph draws too */
static inline int32_t ins_shape(uint32_t ty, int32_t v, uint32_t sh)
{
    switch (ty) {
    case IT_AMP: {                                   /* static approximation for the transfer display */
        int32_t a = softclip(clamp(v + (v >> 1), -400000, 400000) + 7000) - softclip(7000);
        return softclip(a * 4 - 3500) - softclip(-3500);
    }
    case IT_SOFT:
        return softclip(v);
    case IT_HARD:
        return clamp(v, -24000, 24000);
    case IT_FOLD: {
        int32_t u = (int32_t)((uint32_t)(v + 16384) & 0xFFFFu) - 32768;   /* a triangle of period 65536: +-16384 */
        u = 16384 - (u < 0 ? -u : u);
        return u + (u >> 1);                            /* x 1.5 (no divide) */
    }
    case IT_FUZZ:
        return v > 0 ? (v > 20000 ? 20000 : v) : softclip(v) >> 1;
    case IT_CRUSH:                                      /* sh: 16 - BITS; to the nearest step */
        return sh ? ((v + (1 << (sh - 1u))) >> sh) << sh : v;
    default:
        return v;
    }
}
static inline uint32_t ins_bits_sh(int32_t a) { return 15u - (uint32_t)(clamp(a, 0, 127) >> 3); }   /* BITS 1..16 */

/* SOFT .. FUZZ: low cut -> DRIVE -> shape -> TONE -> LEVEL -> DC blocker; CRUSH: held sample -> bits -> LPF */
static __attribute__((noinline)) void ins_drive(ins_t *s, const track_t *t, int32_t *b, uint32_t n, int32_t tw)
{
    uint32_t i, ty = s->type, crush = ty == IT_CRUSH, sh = ins_bits_sh(t->p[P_IA]);
    uint32_t hold = INS_HOLD[clamp(t->p[P_IB], 0, 127) >> 3];
    int32_t a = t->p[P_IA], g = 4096 + a * a * 3;       /* DRIVE, Q12: 1x .. 12.8x, gentle at first */
    int32_t k = ins_lpk(crush ? t->p[P_IC] : t->p[P_IB]), lvl = crush ? 4096 : LEVEL_Q12[t->p[P_IC] & 127];
    for (i = 0; i < n; i++) {
        int32_t x = b[i], y, v;
        s->w += clamp(tw - s->w, -SL_SLOPE, SL_SLOPE);
        if (crush) {
            if (!s->cnt) {
                s->hold = ins_shape(IT_CRUSH, clamp(x, -262144, 262143), sh);
                s->cnt = (uint8_t)hold;
            }
            s->cnt--;
            y = s->hold;
        } else {
            s->hp += ((x << 8) - s->hp) >> 8;            /* ~27 Hz low cut (Q8: no offset left by the rounding) */
            v = clamp(x - (s->hp >> 8), -230000, 230000);
            y = ins_shape(ty, ((v >> 3) * g) >> 9, 0);    /* (>> 3 first: 32 bits) */
        }
        y <<= 4;                                        /* (x16: the low cutoffs' rounding leaves no offset) */
        v = mulq16(y - s->lp1, (uint32_t)k);             /* TONE / LPF: two one-pole low-passes */
        y = v + s->lp1;
        s->lp1 = y + v;
        v = mulq16(y - s->lp2, (uint32_t)k);
        y = v + s->lp2;
        s->lp2 = y + v;
        y = (y + 8) >> 4;
        if (!crush) {
            y = (y * lvl) >> 12;                        /* LEVEL */
            s->dc += ((y << 8) - s->dc) >> 9;           /* DC blocker, ~14 Hz (FUZZ's lopsided halves) */
            y -= s->dc >> 8;
        }
        b[i] = pf_mix(x, y, s->w);
    }
}

/* PHASR, FLANG, CHOR: the sweep at RATE (one LFO per track), DEPTH, FDBK */
static __attribute__((noinline)) void ins_swept(ins_t *s, const track_t *t, int32_t *b, uint32_t n, int32_t tw)
{
    uint32_t i, inc = LFO_INC[clamp(t->p[P_IA], 0, 127)] / CTL, dep = (uint32_t)clamp(t->p[P_IB], 0, 127);
    int32_t c = clamp(t->p[P_IC], 0, 127);
    if (s->type == IT_PHASER) {                         /* the coefficients a block at a time (at its middle) */
        uint32_t tr = pf_tri(s->ph + (n >> 1) * inc), q = 32768u + (uint32_t)(((int32_t)tr - 32768) * (int32_t)dep / 127);
        int32_t a = PH_A[q >> 11] + (((PH_A[(q >> 11) + 1u] - PH_A[q >> 11]) * (int32_t)(q & 2047u)) >> 11);
        int32_t fbk = c * 180;                          /* 0 .. 0.7, Q15 */
        for (i = 0; i < n; i++) {
            int32_t x = b[i], xs = clamp((x + 4) >> 3, -16383, 16383), u = xs + ((s->fb * fbk + 16384) >> 15), y = u;
            uint32_t st;
            s->w += clamp(tw - s->w, -SL_SLOPE, SL_SLOPE);
            for (st = 0; st < PH_ST; st++) {            /* y = a (u - y1) + u1 */
                y = clamp(s->px[st] + ((a * (u - s->py[st]) + 8192) >> 14), -65535, 65535);
                s->px[st] = u;
                s->py[st] = y;
                u = y;
            }
            s->fb = clamp(y, -32767, 32767);
            y <<= 2;                                    /* (y: at an eighth) */
            s->dc += ((y << 8) - s->dc) >> 9;           /* DC blocker, ~14 Hz (a clipped loud input is lopsided) */
            b[i] = pf_mix(x, (x >> 1) + y - (s->dc >> 8), s->w);   /* (dry + all-passed) / 2 */
        }
        s->ph += n * inc;
        return;
    }
    {
        int32_t chorus = s->type == IT_CHORUS, fbk = c * (chorus ? 103 : 219);   /* 0 .. 0.4 / 0 .. 0.85, Q15 */
        uint32_t dq = dep * 516u;                       /* DEPTH, Q16 */
        for (i = 0; i < n; i++, s->ph += inc) {
            int32_t x = b[i], r0, r1, v, fr;
            uint32_t d;                                 /* the delay, Q16 samples */
            s->w += clamp(tw - s->w, -SL_SLOPE, SL_SLOPE);
            if (chorus) {
                d = (uint32_t)((140 << 16) + ((osc_sine(s->ph) * (int32_t)dep * 50) >> 5));   /* 3.2 +- 2.3 ms */
            } else {
                uint32_t tr = pf_tri(s->ph), sw = (tr * tr) >> 16;   /* longer near the short end (perform.c) */
                d = (11u << 16) + 209u * (((sw >> 1) * dq) >> 15);  /* 0.25 .. 5 ms */
            }
            r0 = (int32_t)(s->wp - (d >> 16)) & (int32_t)(IL_LEN - 1u);
            r1 = (r0 - 1) & (int32_t)(IL_LEN - 1u);
            fr = (int32_t)((d >> 1) & 0x7FFFu);
            v = s->line[r0] + (((s->line[r1] - s->line[r0]) * fr) >> 15);
            s->line[s->wp] = (int16_t)clamp((x >> 2) + softclip(mulq15(v, fbk)), -32767, 32767);
            s->wp = (uint16_t)((s->wp + 1u) & (IL_LEN - 1u));
            v <<= 1;                                    /* (v: at a quarter) */
            s->dc += ((v << 8) - s->dc) >> 9;           /* DC blocker (the line's clip on a loud lopsided wave) */
            b[i] = pf_mix(x, (x >> 1) + v - (s->dc >> 8), s->w);   /* (dry + delayed) / 2 */
        }
    }
}

/* AMP after JIANT d05e199: two biased triodes, coupling high-pass and cabinet.
 * Lives in INSERT so SARDAB's parameter IDs and old DIST sounds stay intact.
 * Uses the swept effect's otherwise idle px/py state, no extra pool allocation. */
static __attribute__((noinline)) void ins_amp(ins_t *s, const track_t *t, int32_t *b, uint32_t n, int32_t tw)
{
    uint32_t i;
    int32_t drive = t->p[P_IA], g = 4096 + drive * drive * 2;
    int32_t k = 9000 + t->p[P_IB] * 86, level = LEVEL_Q12[t->p[P_IC] & 127];
    for (i = 0; i < n; i++) {
        int32_t x = b[i], y, v, hp;
        s->w += clamp(tw - s->w, -SL_SLOPE, SL_SLOPE);
        s->hp += (x - s->hp + 64) >> 7;
        v = ((clamp(x - s->hp, -230000, 230000) >> 2) * g) >> 10;
        y = softclip(clamp(v + (v >> 1), -400000, 400000) + 7000) - softclip(7000);
        s->px[0] += (y - s->px[0]) >> 6;
        y = softclip((((y - s->px[0]) * (24 + drive)) >> 4) - 3500) - softclip(-3500);
        s->lp1 += mulq15(y - s->lp1, k);
        s->lp2 += mulq15(s->lp1 - s->lp2, k);
        hp = s->lp2 - s->px[1] - mulq15(s->py[1], 21000);
        s->py[1] = clamp(s->py[1] + mulq15(hp, 8400), -90000, 90000);
        s->px[1] = clamp(s->px[1] + mulq15(s->py[1], 8400), -90000, 90000);
        y = s->lp2 + mulq15(s->py[1], 19660);
        y = (mulq15(y, 24000 - drive * 70) * level) >> 12;
        s->dc += ((y * 256) - s->dc) >> 9;
        b[i] = pf_mix(x, y - (s->dc >> 8), s->w);
    }
}

/* the INSERT at rest at once (mix_part: TYPE turned OFF while the part was silent, its fade never ran) */
static __attribute__((noinline)) void ins_rest(track_t *t)
{
    memset(&ins[(uint32_t)(t - trk) % NTRK], 0, sizeof(ins_t));
    t->ins_run = 0;
}

/* the track's INSERT on its block b (mix_part, mix_spread): nothing while TYPE is OFF or MIX is 0 and faded */
static __attribute__((noinline)) void track_insert(track_t *t, int32_t *b, uint32_t n)
{
    ins_t *s = &ins[(uint32_t)(t - trk) % NTRK];
    int32_t ty = clamp(t->p[P_ITYPE], 0, IT_N - 1), tw;
    if (s->type != ty && !s->w)                         /* faded out: the new type from rest */
        s->type = (uint8_t)ty;
    tw = s->type == ty && ty ? ins_mixq(t->p[P_IMIX]) : 0;
    if (!s->w) {
        if (!tw) {
            t->ins_run = 0;
            return;                                     /* dry: bit for bit */
        }
        memset(s, 0, sizeof *s);                        /* starting: no stale line or filter */
        s->type = (uint8_t)ty;
    }
    if (s->type == IT_AMP)
        ins_amp(s, t, b, n, tw);
    else if (s->type >= IT_PHASER)
        ins_swept(s, t, b, n, tw);
    else
        ins_drive(s, t, b, n, tw);
    t->ins_run = s->w != 0;                             /* (mix_part calls again while it fades) */
}

/* one block of the whole mix (shared with hostsim.c): events -> each part (with its modulation matrix)
 * -> dist -> INSERT -> SLICER -> level / pan / sends -> buses -> master; out: stereo Q15 */
static void events_block(uint32_t n);                    /* seq.c */
static int32_t send_c[CTL], send_d[CTL], send_r[CTL], wet[CTL], mix_l[CTL], mix_r[CTL];
/* (JIANT 0.6.71) what the second core's parts add, summed into the above after it is done (mix_block mc_join) */
static struct { int32_t l[CTL], r[CTL], c[CTL], d[CTL], rv[CTL]; } c1_mix;

/* SPREAD (#148): a POLY or UNISON part whose SPRD is above 0. Its voices start left and right of PAN in turn (voice.c
 * voice_start; UNISON's detuned voices alternate), at PAN -/+ SPRD / 2 (SPRD 127: hard left and right), with the pan
 * law PAN has (the far side down, the near one as it is). track_render gives every voice (b, as always) and the left
 * ones (spread_buf); here d = left - right, the side signal, so for a pair of pans L = b (gla + glb) / 2 + d (gla - glb)
 * / 2 (R alike): the mono sum is b's, the level of a centre pan less by SPRD / 256 at most. b goes through DIST, the
 * SLICER and the FX layer's mute key as always; d gets the SLICER's gate and the mute's ramp (slicer_track_sd,
 * perf_mute_sd) and passes DIST by (the spread of a distorted chord is its voices' clean difference). The sends
 * take b, mono, as before. SPRD 0, MONO or LEGATO: mix_part's own loop, bit for bit as before. The cost: a block's
 * clear and sum of spread_buf, the side and a wider mix loop, per part; per voice none (it picks its buffer) */
static __attribute__((noinline)) void mix_spread(track_t *t, int32_t *b, uint32_t n)
{
    int32_t *d = SPREAD_BUF;
    uint32_t i, c1 = MC_CPU() & 1u;                     /* (JIANT 0.6.71) on the second core: its own mix and sends */
    int32_t *ol = c1 ? c1_mix.l : mix_l, *or_ = c1 ? c1_mix.r : mix_r;
    int32_t *sc = c1 ? c1_mix.c : send_c, *sd = c1 ? c1_mix.d : send_d, *sr = c1 ? c1_mix.rv : send_r;
    int32_t lvl = LEVEL_Q12[t->p[P_LEVEL] & 127], pan = t->p[P_PAN], s = (t->p[P_SPRD] + 1) >> 1;
    int32_t pa = clamp(pan - s, -64, 63), pb = clamp(pan + s, -64, 63);
    int32_t gla = 4096 - (pa > 0 ? pa * 64 : 0), gra = 4096 + (pa < 0 ? pa * 64 : 0);
    int32_t glb = 4096 - (pb > 0 ? pb * 64 : 0), grb = 4096 + (pb < 0 ? pb * 64 : 0);
    int32_t glm = (gla + glb) >> 1, grm = (gra + grb) >> 1, gld = (gla - glb) >> 1, grd = (gra - grb) >> 1;
    int32_t c = t->p[P_CHOR] * 258, dl = t->p[P_DLY] * 258, r = t->p[P_REV] * 258, pk = t->peak;
    int32_t xmax = c > dl ? c : dl;
    xmax = 0x7FFFFFFF / ((xmax > r ? xmax : r) | 1);
    for (i = 0; i < n; i++)
        d[i] = (d[i] << 1) - b[i];                      /* left - right */
    track_dist(t, b, n);
    if (t->p[P_ITYPE] | t->ins_run)
        track_insert(t, b, n);                          /* (the mono sum, as DIST) */
    slicer_track_sd(t, b, d, n);
    if ((pf.mute >> (t - trk)) & 1u)
        perf_mute_sd((uint32_t)(t - trk), b, d, n);
    for (i = 0; i < n; i++) {
        int32_t x = ((b[i] >> 2) * lvl) >> 10, a = x < 0 ? -x : x, y = ((d[i] >> 2) * lvl) >> 10;
        int32_t xs = clamp(x, -xmax, xmax);
        if (a > pk)
            pk = a;
        if (c)
            sc[i] += mulq15(xs, c);
        if (dl)
            sd[i] += mulq15(xs, dl);
        if (r)
            sr[i] += mulq15(xs, r);
        ol[i] += ((x * glm) >> 12) + ((y * gld) >> 12);
        or_[i] += ((x * grm) >> 12) + ((y * grd) >> 12);
    }
    t->peak = pk;
}

/* one synth part into the dry mix and the sends; a part with no voice sounding costs
 * the LFO tick and a cleared buffer only (after the DIST tail has run out) */
static void mix_part_run(track_t *t, uint32_t n);
static int32_t send_top[NTRK], send_lim[NTRK];        /* mix_part_run: a part's largest send, 0x7FFFFFFF / it */
/* a sound's fade (voice.c sound_change): the whole part rendered with the sound it had, then the new one back */
static void mix_part(track_t *t, uint32_t n)
{
    static int16_t pnew[2][P_COUNT];                    /* (JIANT 0.6.71: a core's own) */
    int16_t *pn = pnew[MC_CPU() & 1u];
    uint32_t k = (uint32_t)(t - trk) % NTRK;
    if (!(snd_is_fading(t) && t->xf_on && t->xf)) {
        mix_part_run(t, n);
        return;
    }
    memcpy(pn, t->p, sizeof pnew[0]);
    memcpy(t->p, snd_old[k], sizeof t->p);
    mix_part_run(t, n);
    memcpy(t->p, pn, sizeof pnew[0]);
}

static void mix_part_run(track_t *t, uint32_t n)
{
    uint32_t i, c1 = MC_CPU() & 1u;                     /* (JIANT 0.6.71) on the second core: its own buffers */
    int32_t *b = c1 ? c1_buf : part_buf;
    mod_begin(t);                                       /* the matrix's per-block values into t->p (mod.c) */
    SPREAD_ON = t->p[P_SPRD] > 0 && (trk_vmode(t) == V_POLY || trk_vmode(t) == V_UNISON);   /* (SPREAD, mix_spread) */
    if (track_render(t, b, n))
        t->tail = t->p[P_ITYPE] ? 64 : 16;              /* blocks of DIST / INSERT state to run out after the last
                                                         * voice (the INSERT's: 46 ms, a flanger's feedback) */
    else if ((!t->tail || !(t->p[P_DIST] | t->p[P_ITYPE]) || !--t->tail) && !slicer_busy(t)) {
        slicer_track(t, 0, n);                          /* (the SLICER's step clock runs on) */
        if (t->ins_run && !t->p[P_ITYPE])
            ins_rest(t);                                /* TYPE OFF while silent: no fade-out left for the next note */
        if (mod.on)
            mod_end(t);
        return;
    }
    if (SPREAD_ON)
        mix_spread(t, b, n);
    else {
        track_dist(t, b, n);
        if (t->p[P_ITYPE] | t->ins_run)
            track_insert(t, b, n);                      /* the INSERT (1.5): off, nothing */
        slicer_track(t, b, n);                          /* slicer.c: before the level, pan and sends */
        if ((pf.mute >> (t - trk)) & 1u)
            perf_mute((uint32_t)(t - trk), b, n);       /* perform.c: a black key in the FX layer */
    }
    if (!SPREAD_ON) {                                   /* (the mix's values read after the calls: none is held over
                                                         * them, no spill in the loop: the ISR's cost as before 1.5) */
        int32_t lvl = LEVEL_Q12[t->p[P_LEVEL] & 127], pan = t->p[P_PAN];
        int32_t gl = 4096 - (pan > 0 ? pan * 64 : 0), gr = 4096 + (pan < 0 ? pan * 64 : 0);
        int32_t c = t->p[P_CHOR] * 258, d = t->p[P_DLY] * 258, r = t->p[P_REV] * 258, pk = t->peak;
        int32_t xmax = c > d ? c : d, k = (int32_t)((uint32_t)(t - trk) % NTRK);
        int32_t *ol = c1 ? c1_mix.l : mix_l, *or_ = c1 ? c1_mix.r : mix_r;
        int32_t *sc = c1 ? c1_mix.c : send_c, *sd = c1 ? c1_mix.d : send_d, *sr = c1 ? c1_mix.rv : send_r;
        xmax = (xmax > r ? xmax : r) | 1;
        if (xmax != send_top[k]) {                      /* (the divide only when a send moved) */
            send_top[k] = xmax;
            send_lim[k] = 0x7FFFFFFF / xmax;
        }
        xmax = send_lim[k];                             /* sends: loud chords at a high LEVEL */
        for (i = 0; i < n; i++) {
            int32_t x = ((b[i] >> 2) * lvl) >> 10, a = x < 0 ? -x : x;   /* pre-shift: 8 loud voices */
            int32_t xs = clamp(x, -xmax, xmax);         /* sends: mulq15 would overflow */
            if (a > pk)
                pk = a;
            if (c)
                sc[i] += mulq15(xs, c);
            if (d)
                sd[i] += mulq15(xs, d);
            if (r)
                sr[i] += mulq15(xs, r);
            ol[i] += (x * gl) >> 12;
            or_[i] += (x * gr) >> 12;
        }
        t->peak = pk;
    }
    if (pf.thr)                                         /* perform.c: REV / ECHO THROW (1.5.1), the sends towards full */
        perf_throw(t, b, c1 ? c1_mix.d : send_d, c1 ? c1_mix.rv : send_r, n);
    if (mod.on)
        mod_end(t);                                     /* the stored values back */
}

/* the HOME oscilloscope (ui_graph.c graph_scope): every other sample of the left channel. MENU > DISPLAY > SCOPE (1.2,
 * Discussion #165): OUT (0, the default) what goes out, after MASTER (audio.c audio_block, before the click); MIX the
 * mix before MASTER (and before the FX layer, SPEAKER EQ and the limiter), at the level MASTER at its top gives, so its
 * size does not follow the volume. The click is in neither */
#define SCOPE_N 512u
static int16_t scope_buf[SCOPE_N];
static uint32_t scope_w;
static volatile uint8_t scope_mix;
static __attribute__((noinline)) void scope_take_mix(uint32_t n)
{
    uint32_t i;
    for (i = 1; i < n; i += 2u)                         /* (((x >> 2) * MASTER_FULL) >> 10 is x) */
        scope_buf[scope_w++ & (SCOPE_N - 1u)] = (int16_t)clamp(mix_l[i] + wet[i], -32767, 32767);
}

/* the master with the FX layer's effects between its level and master_block (perform.c) */
static __attribute__((noinline)) void perf_master(int32_t *out, uint32_t n)
{
    uint32_t i;
    int32_t mg = fx_usb_fixed ? MASTER_FULL : (int32_t)song.master_q12;   /* (USB LEVEL FIXED: MASTER after) */
    for (i = 0; i < n; i++) {
        mix_l[i] = ((mix_l[i] >> 2) * mg) >> 10;
        mix_r[i] = ((mix_r[i] >> 2) * mg) >> 10;
    }
    perf_block(mix_l, mix_r, n);
    master_block(mix_l, mix_r, n);
    for (i = 0; i < n; i++) {
        out[2u * i] = mix_l[i];
        out[2u * i + 1u] = mix_r[i];
    }
}

/* ---- TEXTURE (after SLOOP 2.6 by isod89, GPL-3.0; FX > TEXTURE): a looping bed under the mix, the glue of lo-fi
 * and ambient tracks, without taking a track. SRC: a generated noise (no memory, nothing to load) or a user sample slot looped (USR1..4, its first
 * zone at its own pitch; a looped zone from its loop, else whole), LEVEL, TONE (left a low-pass, right a high-pass:
 * tx_tone), MOVE: how much it breathes over RATE (1 .. 16 bars of the tempo; its level and brightness, a sine from the
 * transport's start), WHEN: PLAY (fades in and out with the transport) or ALWAYS. Into the dry mix (no sends), before
 * the TAPE and the master.
 *   VINYL  hiss, rumble, crackle and the odd pop, each crackle somewhere left to right
 *   TAPE   hiss tilted dark, its level drifting slowly (flutter)
 *   RAIN   a soft bed and droplets, left and right apart
 *   WIND   noise through two resonant low-passes wandering slowly, gusts
 *   RADIO  a band of static (500 Hz .. 3 kHz), coarse, in bursts that come and go
 *   ROOM   room tone: a low rumble and a little air
 * Its level: x * LEVEL^2 / 16384 into the mix (the sources at about 8000 rms: LEVEL 64 about -30 dBFS rms out, 127
 * about -18). Off (and faded out): nothing runs, the mix bit for bit as without */

#include "ambience.c"
enum { TX_OFF, TX_VINYL, TX_TAPE, TX_RAIN, TX_WIND, TX_RADIO, TX_ROOM, TX_USR1, TX_CAVE = TX_USR1 + 4, TX_TORCH, TX_N };
static struct {
    int32_t rnd;                                        /* noise state (never 0) */
    int32_t g;                                          /* gain now (Q15 of the LEVEL target), ramped a block */
    uint8_t src, play;                                  /* the source running; the transport as last seen */
    uint32_t ph;                                        /* MOVE's phase (a turn per RATE) */
    int32_t a[2], b[2], c[2], d[2], e[2];               /* per side: the sources' states (see texture_gen) */
    int32_t lpm[2];                                     /* MOVE's brightness (one-pole) */
    int32_t t0, t1, t2;                                 /* slow random walks / timers */
    tsvf_t wc[2];                                       /* WIND: this block's filters */
    int32_t wz[2][2];
    uint32_t upos, ufrac;                               /* USR: the read position, Q16 fraction */
    int32_t up, ui, us0, us1;                           /* USR: ADPCM predictor, step index, the two samples interpolated */
    uint32_t ugen;                                      /* USR: the slots' scan count (an upload restarts it) */
    int32_t ulast, uoff;                                /* USR: the last value out; the step at a wrap, gliding to 0
                                                         * (a sample that does not end where it starts: no click) */
    uint8_t uwrap;                                      /* USR: tx_usr_next went round */
    int32_t zl[2], zh[2];                               /* TONE: its low-pass, the high-pass's low-pass */
} tx = {.rnd = 0x2545F491};
static inline int32_t tx_n16(void) { return (int32_t)(noise32(&tx.rnd) >> 16) - 32768; }   /* -32768 .. 32767 */
static void texture_reset(uint32_t src)
{
    int32_t rnd = tx.rnd, g = tx.g;
    amb.on = 0;
    uint32_t ph = tx.ph;
    uint8_t play = tx.play;
    memset(&tx, 0, sizeof tx);
    tx.rnd = rnd ? rnd : 0x2545F491;
    tx.g = g, tx.ph = ph, tx.play = play;
    tx.src = (uint8_t)src;
    tx.t0 = 50 << 8, tx.t1 = 60 << 8;                  /* (WIND: the two cutoffs, RADIO / TAPE: the level) */
    tx.ugen = smp_user_gen - 1u;                        /* (USR: the zone looked up on the first block) */
}
/* USR k: the next sample of its first zone, looped (sample_next's decoder, its own state) */
static inline int32_t tx_usr_next(const smp_zone_t *z)
{
    uint32_t pos = tx.upos, b = SMP_DATA[z->off + (pos >> 1)];
    uint32_t code = (pos & 1u) ? (b >> 4) : (b & 15u);
    int32_t step = IMA_STEP[(uint32_t)tx.ui <= 88u ? tx.ui : 88], vd = step >> 3;
    if (code & 4u) vd += step;
    if (code & 2u) vd += step >> 1;
    if (code & 1u) vd += step >> 2;
    tx.up = clamp(tx.up + ((code & 8u) ? -vd : vd), -32768, 32767);
    tx.ui = clamp(tx.ui + IMA_IDX[code & 7u], 0, 88);
    if (++pos > (z->looped ? z->le : z->n - 1u)) {      /* round again: from the loop (its state), or the top */
        pos = z->looped ? z->ls : 0u;
        tx.up = z->looped ? z->pred : 0;
        tx.ui = z->looped ? z->idx : 0;
        tx.uwrap = 1;
    }
    tx.upos = pos;
    return tx.up;
}
/* n samples of source src (both sides) into l, r, at about 8000 rms */
static __attribute__((noinline)) void texture_gen(uint32_t src, int32_t *l, int32_t *r, uint32_t n)
{
    uint32_t i, k;
    if (src == TX_CAVE || src == TX_TORCH) { cave_gen(l, r, n, src == TX_TORCH); return; }
    if (src >= TX_USR1) {
        uint32_t s = src - TX_USR1;
        const smp_zone_t *z = s < SMP_USER_SLOTS && usr_nz[s] ? &usr_zone[s][0] : 0;
        if (!z || !z->n) {
            for (i = 0; i < n; i++)
                l[i] = r[i] = 0;
            return;
        }
        if (tx.ugen != smp_user_gen) {                  /* a new upload (or the first block): from the top */
            tx.ugen = smp_user_gen;
            tx.upos = 0, tx.ufrac = 0, tx.up = tx.ui = 0;
            tx.us0 = 0, tx.us1 = tx_usr_next(z);
            tx.uwrap = 1;                               /* (from where the old one was: no click) */
        }
        for (i = 0; i < n; i++) {
            int32_t v;
            tx.ufrac += z->rate;                        /* (its own rate: Q16 of 44.1 kHz) */
            while (tx.ufrac >= 65536u) {
                tx.ufrac -= 65536u;
                tx.us0 = tx.us1;
                tx.us1 = tx_usr_next(z);
            }
            v = (tx.us0 + (((tx.us1 - tx.us0) * (int32_t)(tx.ufrac >> 1)) >> 15)) >> 1;
            if (tx.uwrap) {                             /* (2.6) round again: from the value it was at */
                tx.uwrap = 0;
                tx.uoff = tx.ulast - v;
            }
            tx.uoff -= (tx.uoff >> 6) + (tx.uoff > 0) - (tx.uoff < 0);   /* (~1.5 ms) */
            l[i] = r[i] = tx.ulast = v + tx.uoff;
        }
        return;
    }
    switch (src) {
    case TX_VINYL:                                      /* a: rumble, b: crackle, c: its decay, d: hiss tilt */
        for (i = 0; i < n; i++) {
            uint32_t z = noise32(&tx.rnd);
            int32_t w = (int32_t)(z >> 16) - 32768, h;
            tx.a[0] += (w >> 5) - (tx.a[0] >> 7);       /* rumble: integrated, leaky */
            if ((z & 0xFFFFu) < 60u) {                  /* crackle, ~40 a second, anywhere left to right */
                int32_t amp = 6000 + (int32_t)((z >> 8) & 0x3FFFu) + ((z & 1u) ? 0 : 12000 * ((z & 0x30u) == 0u));
                int32_t pan = (int32_t)((z >> 20) & 0xFFu);
                amp = (z & 0x100u) ? -amp : amp;
                tx.b[0] = (amp * (256 - pan)) >> 8;
                tx.b[1] = (amp * pan) >> 8;
            }
            for (k = 0; k < 2u; k++) {
                h = (k ? (int32_t)(noise32(&tx.rnd) >> 16) - 32768 : w);
                tx.d[k] += (h - tx.d[k]) >> 2;          /* the hiss: dark, low (the crackle on top) */
                (k ? r : l)[i] = (tx.d[k] >> 4) + (tx.a[0] >> 2) + tx.b[k];
                tx.b[k] -= tx.b[k] >> 2;                /* (a click: a few samples) */
            }
        }
        break;
    case TX_TAPE:                                       /* t0: the level drifting (Q8), t2: its next target */
        if (!(tx.t1++ & 31))
            tx.t2 = (24 << 8) + (int32_t)((noise32(&tx.rnd) >> 20) & 0x7FFu) * 4;
        tx.t0 += (tx.t2 - tx.t0) >> 6;
        for (i = 0; i < n; i++)
            for (k = 0; k < 2u; k++) {
                int32_t w = tx_n16();
                tx.a[k] += (w - tx.a[k]) >> 2;          /* tilted dark: mostly the low-passed part */
                (k ? r : l)[i] = (((tx.a[k] >> 1) + (w >> 3)) * (tx.t0 >> 8)) >> 5;
            }
        break;
    case TX_RAIN:                                       /* a: the bed's low-pass, b: a droplet's level, c: its noise */
        for (i = 0; i < n; i++)
            for (k = 0; k < 2u; k++) {
                uint32_t z = noise32(&tx.rnd);
                int32_t w = (int32_t)(z >> 16) - 32768;
                tx.a[k] += (w - tx.a[k]) >> 4;          /* the bed: ~450 Hz */
                if ((z & 0xFFFFu) < 420u)               /* a droplet: ~280 a second a side */
                    tx.b[k] = 4000 + (int32_t)((z >> 4) & 0x3FFFu);
                tx.c[k] += (w - tx.c[k]) >> 1;
                (k ? r : l)[i] = (tx.a[k] >> 1) + (((w - tx.c[k]) * tx.b[k]) >> 14);
                tx.b[k] -= tx.b[k] >> 5;
            }
        break;
    case TX_WIND: {                                     /* t0, t1: the cutoffs wandering (Q8 index), t2: their targets */
        if (!(noise32(&tx.rnd) & 31u))
            tx.t2 = (int32_t)(noise32(&tx.rnd) & 0xFFFFu);   /* new targets now and then: 34 .. 78 */
        tx.t0 += (((34 << 8) + (tx.t2 & 0xFF) * 44) - tx.t0) >> 7;
        tx.t1 += (((34 << 8) + (tx.t2 >> 8) * 44) - tx.t1) >> 7;
        tsvf_coef(&tx.wc[0], tx.t0, 95);
        tsvf_coef(&tx.wc[1], tx.t1, 95);
        for (i = 0; i < n; i++)
            for (k = 0; k < 2u; k++) {
                int32_t y = tsvf_lp(&tx.wc[k], tx_n16() >> 2, &tx.wz[k][0], &tx.wz[k][1]);
                (k ? r : l)[i] = clamp(y, -32767, 32767);
            }
        break;
    }
    case TX_RADIO:                                      /* a, b: the band's two low-passes, t0: the burst level (Q8),
                                                         * t1: samples left of the burst, c: the held sample */
        if ((tx.t1 -= (int32_t)n) <= 0) {
            uint32_t z = noise32(&tx.rnd);
            tx.t1 = 1500 + (int32_t)((z >> 8) & 0x3FFFu);   /* 35 .. 400 ms */
            tx.t2 = (z & 3u) == 0u ? 40 << 8 : (110 + (int32_t)((z >> 24) & 0x7Fu)) << 8;   /* (a gap now and then) */
        }
        tx.t0 += (tx.t2 - tx.t0) >> 2;
        for (i = 0; i < n; i++) {
            int32_t w = tx_n16(), y;
            if (!(i & 3u))
                tx.c[0] = w;                            /* coarse: held four samples */
            tx.a[0] += (tx.c[0] - tx.a[0]) >> 1;        /* < ~3 kHz */
            tx.b[0] += (tx.a[0] - tx.b[0]) >> 4;        /* the lows taken out: > ~500 Hz */
            y = (((tx.a[0] - tx.b[0]) >> 1) * (tx.t0 >> 8)) >> 6;
            l[i] = y;
            r[i] = y - (y >> 2);
        }
        break;
    case TX_ROOM:                                       /* a: the rumble, b: its low-pass, c: air */
        for (i = 0; i < n; i++)
            for (k = 0; k < 2u; k++) {
                int32_t w = tx_n16();
                tx.a[k] += (w >> 4) - (tx.a[k] >> 8);
                tx.b[k] += (tx.a[k] - tx.b[k]) >> 5;
                (k ? r : l)[i] = (tx.b[k] >> 1) + (w >> 6);
            }
        break;
    default:
        for (i = 0; i < n; i++)
            l[i] = r[i] = 0;
        break;
    }
}
/* TONE: below 0 a one-pole low-pass closing to ~220 Hz, above 0 a high-pass opening to ~3 kHz (the low-pass taken
 * out), 0 open (the high-pass at ~3 Hz). Both always run: no jump crossing 0 */
static void tx_tone(int32_t *x, uint32_t n, int32_t tone, int32_t *zl, int32_t *zh)
{
    int32_t u = 64 + (tone < 0 ? tone : 0), v = tone > 0 ? tone : 0;
    int32_t al = clamp(1024 + u * u * 31 / 4, 0, 32767), ah = 16 + v * v * 7 / 2;
    uint32_t i;
    for (i = 0; i < n; i++) {
        *zl += ((x[i] - *zl) >> 2) * al >> 13;          /* (>> 2 .. >> 13: no overflow at +-260000) */
        *zh += ((*zl - *zh) >> 2) * ah >> 13;
        x[i] = *zl - *zh;
    }
}
static __attribute__((noinline)) void texture_mix(int32_t *l, int32_t *r, uint32_t n)   /* (out of the ISR: OFF costs a call a block) */
{
    static int32_t tl[CTL], tr[CTL];
    static const uint8_t BARS[5] = {1, 2, 4, 8, 16};
    uint32_t src = (uint32_t)clamp(song.g[G_TXSRC], 0, TX_N - 1), i, bar;
    /* the sources to about 8000 rms (VINYL ~5000: its crackle peaks), Q8; the user slots as they are (/ 2) */
    static const int16_t TX_SCALE[TX_USR1 + 1] = {0, 800, 301, 693, 1918, 149, 343, 256};
    int32_t lvl = clamp(song.g[G_TXLVL], 0, 127), on, g0, g1, mv, depth, u, lfo, k0, k1, sc;
    if (song.playing && !tx.play)
        tx.ph = 0;                                      /* MOVE from the transport's start */
    tx.play = (uint8_t)song.playing;
    on = src && src == tx.src && (song.playing || song.g[G_TXWHEN]) && lvl;
    if (!tx.g && !on) {                                 /* silent: nothing runs (and a new source starts from rest) */
        if (src != tx.src)
            texture_reset(src);
        return;
    }
    if (n > CTL)
        return;
    g0 = tx.g;                                          /* fades over ~0.1 s (238 a block), as DUST's bed */
    g1 = tx.g = clamp(tx.g + (on ? 238 : -238), 0, 32767);
    bar = (uint32_t)FS * 240u / (uint32_t)clamp(song.g[G_BPM], 20, 300);   /* a bar of 4/4, samples */
    tx.ph += (uint32_t)((0xFFFFFFFFu / (bar * BARS[clamp(song.g[G_TXRATE], 0, 4)])) * n);
    lfo = sine_i(tx.ph - 0x40000000u);                  /* (from its top: a start at full) */
    depth = clamp(song.g[G_TXMOVE], 0, 127) * 258;
    u = mulq15(depth, (32767 - lfo) >> 1);              /* 0 .. depth, the dip */
    mv = 32767 - mulq15(u, 21000);                      /* the level breathing: down to ~36 % */
    k0 = 32767 - mulq15(u, 28000);                      /* the brightness: a one-pole closing */
    texture_gen(tx.src, tl, tr, n);
    sc = tx.src >= TX_CAVE ? 2048 : TX_SCALE[tx.src < TX_USR1 ? tx.src : TX_USR1];
    for (i = 0; i < n; i++) {                           /* each source to its level, MOVE's colour (Q13: no overflow */
        tx.lpm[0] += ((((tl[i] * sc) >> 8) - tx.lpm[0]) >> 2) * (k0 >> 2) >> 11;   /* up to +-260000) */
        tx.lpm[1] += ((((tr[i] * sc) >> 8) - tx.lpm[1]) >> 2) * (k0 >> 2) >> 11;
        tl[i] = tx.lpm[0], tr[i] = tx.lpm[1];
    }
    tx_tone(tl, n, clamp(song.g[G_TXTONE], -64, 63), &tx.zl[0], &tx.zh[0]);
    tx_tone(tr, n, clamp(song.g[G_TXTONE], -64, 63), &tx.zl[1], &tx.zh[1]);
    k1 = mulq15(lvl * lvl * 2, mv);                     /* LEVEL^2 (x 2: Q15 of 4096 at 127 .. ) x MOVE */
    for (i = 0; i < n; i++) {
        int32_t g = g0 + (((g1 - g0) * (int32_t)i) / (int32_t)CTL), gk = mulq15(g, k1);
        l[i] += (tl[i] * (gk >> 4)) >> 11;              /* x LEVEL^2 / 16384: (x * gk / 32768) at full */
        r[i] += (tr[i] * (gk >> 4)) >> 11;
    }
    if (!g1 && src != tx.src)
        texture_reset(src);                             /* faded out: the new source from rest */
}

/* (JIANT 0.6.71) the second core (hal/fm1_multicore.h). Each block, while core 0 renders its parts, core 1 renders
 * some synth parts of its own into its own buffers (c1_buf, c1_mix; mod.c, fm6_core.c, voice.c, eng_phase.c keep
 * theirs a core); they are added in after it is done: the same sum, the same sound as one core (the buses, TEXTURE,
 * TAPE and the master stay on core 0, after both). The parts go to whichever core has less to do, by what each took
 * the last blocks (TIMER4), the voices breaking ties; the GURDY parts go together (one crank). A core 1 that misread,
 * faulted or did not come back is held for good: its block of those parts is dropped and core 0 renders every part
 * from the next one. REC held at power-on: core 0 alone (mc_off). MENU > ABOUT says which (mc_status) */
#if FELUCCA_DUAL_CORE
#define mc_run(f, a) fm1_mc_run(f, a)
#define mc_wait() fm1_mc_wait()
#define mc_live() (fm1_mc_on != 0)
#define MC_TICKS() fm1_mc_ticks()
#define MC_VW 24u                                        /* a voice-weight unit in ticks (~1 us) */
#else
static int mc_run(void (*f)(uint32_t), uint32_t a)      /* (the host: at once, as core 1) */
{
    if (!mc_host)
        return 0;
    mc_host_cpu = 1;
    f(a);
    mc_host_cpu = 0;
    return 1;
}
static int mc_wait(void) { return 0; }
#define mc_live() (mc_host != 0)
#define MC_TICKS() 0u                                    /* (no clock: the voices alone) */
#define MC_VW 1u
#endif
static volatile uint8_t mc_off;                          /* REC held at power-on: one core this session (main.c) */
static volatile int8_t mc_cur = -1;                      /* the part core 1 is in (a job given up: its matrix put back) */
static uint8_t mc_mask_last;                             /* the parts core 1 had the last block (MENU > ABOUT, tests) */
static uint32_t mc_jobs;                                 /* blocks core 1 rendered parts in (console status) */
static uint32_t mc_tk[NTRK];                             /* what each part took the last blocks, smoothed (ticks) */
static void mix_part_t(track_t *t, uint32_t n)          /* mix_part, timed (on either core) */
{
    uint32_t t0 = MC_TICKS(), i = (uint32_t)(t - trk) % NTRK, d;
    mix_part(t, n);
    d = MC_TICKS() - t0;
    mc_tk[i] = d > mc_tk[i] ? mc_tk[i] + ((d - mc_tk[i]) >> 1) : mc_tk[i] - ((mc_tk[i] - d) >> 3);   /* (up fast) */
}
static __attribute__((noinline)) void mc_job(uint32_t a)  /* core 1: the parts of mask a & 0xF, a >> 8 samples */
{
    uint32_t i, n = a >> 8;
    for (i = 0; i < n; i++)
        c1_mix.l[i] = c1_mix.r[i] = c1_mix.c[i] = c1_mix.d[i] = c1_mix.rv[i] = 0;
    for (i = 0; i < NPART; i++)
        if ((a >> i) & 1u) {
            mc_cur = (int8_t)i;
            mix_part_t(&trk[i], n);
        }
    mc_cur = -1;
}
static uint32_t mc_cost(const track_t *t)               /* a part's share of the work, roughly: its voices by engine */
{
    uint32_t k, v = 0, w = ENGINES[t->engine] == &ENG_FM6 ? 4u : 3u;
    for (k = 0; k < NVOICE; k++)
        v += t->v[k].active;
    return v ? 1u + v * w : 0u;
}
/* the parts for core 1 this block (a bit each): whichever core has less to do takes the dearest left. The GURDY parts
 * go together, as one (the crank is one for all): on one core or the other */
static __attribute__((noinline)) uint32_t mc_split(void)
{
    uint32_t i, j, c0 = 0, c1 = 0, mask = 0, cost[NPART + 1], unit[NPART + 1], nu = 1, left = 0;
    cost[0] = unit[0] = 0;                               /* (item 0: the GURDY parts) */
    for (i = 0; i < NPART; i++) {
        uint32_t v = mc_cost(&trk[i]), c = v ? mc_tk[i] + v * MC_VW : 0u;
        if (!c)
            continue;
        if (ENGINES[trk[i].engine] == &ENG_GURDY) {
            cost[0] += c;
            unit[0] |= 1u << i;
        } else {
            cost[nu] = c;
            unit[nu++] = 1u << i;
        }
    }
    for (i = 0; i < nu; i++)
        if (unit[i])
            left |= 1u << i;
    while (left) {
        for (j = nu, i = 0; i < nu; i++)
            if (((left >> i) & 1u) && (j == nu || cost[i] > cost[j]))
                j = i;
        left &= ~(1u << j);
        if (c1 < c0) {
            c1 += cost[j];
            mask |= unit[j];
        } else {
            c0 += cost[j];
        }
    }
    return mask;
}
static __attribute__((noinline)) void mc_join(uint32_t n)   /* core 1's parts into the mix */
{
    uint32_t i;
    for (i = 0; i < n; i++) {
        mix_l[i] += c1_mix.l[i];
        mix_r[i] += c1_mix.r[i];
        send_c[i] += c1_mix.c[i];
        send_d[i] += c1_mix.d[i];
        send_r[i] += c1_mix.rv[i];
    }
}
static __attribute__((noinline)) void mc_lost(void)    /* core 1 given up mid-job: a part's matrix values put back */
{
    int32_t k = mc_cur;
    if (k >= 0 && k < (int32_t)NPART && mod_c[1].on) {
        mod_c[0] = mod_c[1];
        mod_end(&trk[k]);
    }
    mc_cur = -1;
#if FELUCCA_DUAL_CORE
    fm1_mc_bar_set();                                    /* one core until power-off (hal/fm1_multicore.h) */
#endif
}
static const char *mc_status(void)                      /* MENU > ABOUT's line, the console's */
{
#if FELUCCA_DUAL_CORE
    static const char *const LOST[4] = {"CORES 1 (SINCE A FAULT)", "CORES 1 (BAD READ)", "CORES 1 (NO ANSWER)",
                                        "CORES 1 (FAULT)"};
    if (fm1_mc_on)
        return "CORES 2";
    if (mc_off)
        return "CORES 1 (REC AT POWER-ON)";
    if (!fm1_power_ready)
        return "CORES 1 (POWER)";                        /* the supplies did not take the stock setup (hal/fm1_power.h) */
    if (fm1_mc_lost)
        return LOST[fm1_mc_lost & 3u];
    return fm1_mc_barred() ? LOST[0] : "CORES 1 (NO START)";
#else
    return mc_host ? "CORES 2" : "CORES 1";
#endif
}

static void mix_block(int32_t *out, uint32_t n)
{
    uint32_t i, c1m = 0, c1j = 0;
    int perf;
    int32_t mg = fx_usb_fixed ? MASTER_FULL : (int32_t)song.master_q12;   /* (USB LEVEL FIXED: MASTER after) */
    for (i = 0; i < n; i++)
        send_c[i] = send_d[i] = send_r[i] = mix_l[i] = mix_r[i] = 0;
    mns_clk++;
    events_block(n);
    perf = perf_begin(n);                               /* the FX hold layer at work (perform.c) */
    if (!mc_off && mc_live()) {                         /* (JIANT 0.6.71) core 1 at its parts meanwhile */
        c1m = mc_split();
        if (c1m && mc_run(mc_job, c1m | n << 8))
            c1j = 1;
        else
            c1m = 0;
    }
    for (i = 0; i < NPART; i++)
        if (!((c1m >> i) & 1u))
            mix_part_t(&trk[i], n);
    if (c1j) {
        if (!mc_wait()) {
            mc_join(n);
            mc_jobs++;
        } else {
            mc_lost();                                  /* (its parts' block dropped; one core from here) */
        }
    }
    mc_mask_last = (uint8_t)c1m;
    texture_mix(mix_l, mix_r, n);                       /* TEXTURE: the bed, dry */
    if (perf)
        perf_pre(mix_l, mix_r, send_d, send_r, n);
    fx_buses(send_c, send_d, send_r, wet, n);
    if (hl.side)
        rev_side_mix(mix_l, mix_r, n);                  /* HALL: stereo */
    if (scope_mix)
        scope_take_mix(n);
    for (i = 0; i < n; i++) {
        mix_l[i] += wet[i];
        mix_r[i] += wet[i];
    }
    tape_block(mix_l, mix_r, n);
    if (perf) {
        perf_master(out, n);
        return;
    }
    for (i = 0; i < n; i++) {
        mix_l[i] = ((mix_l[i] >> 2) * mg) >> 10;
        mix_r[i] = ((mix_r[i] >> 2) * mg) >> 10;
    }
    master_block(mix_l, mix_r, n);
    for (i = 0; i < n; i++) {
        out[2u * i] = mix_l[i];
        out[2u * i + 1u] = mix_r[i];
    }
}
