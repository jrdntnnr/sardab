/* SPDX-License-Identifier: GPL-3.0-only */
/* Contact captures -> real keyboard, tuning/chords, layer, MIDI and recording. */
#define UI_TEST_NO_MAIN 1
#include "ui_test.c"
#include <assert.h>
static void start(void)
{
    ui_power_on(); usb.config = 1; mo_r = mo_w; kb_prev = 0; kb_vel = 100;
    for (uint32_t t=0;t<NTRK;t++) {
        trk_all_off(&trk[t]);
        for (uint32_t v=0;v<NVOICE;v++) trk[t].v[v].active=0;
        trk[t].p[P_AMODE]=0; trk[t].p[P_VOICE]=V_POLY; trk[t].p[P_CHRD]=0;
    }
    menu_put(MI_VMODE,1); kv_hw_on=1;
}
static void vpress(uint32_t k)
{
    kv_contact(k,1); fm1_in.notes |= 1u<<k; events_block(CTL);
}
static void land(uint32_t k,uint32_t value)
{
    kv_sample(k,value); kv_sample(k,value); events_block(CTL);
}
static void release(uint32_t k)
{
    kv_contact(k,0); fm1_in.notes &= ~(1u<<k); events_block(CTL);
}
static unsigned ons(unsigned expected)
{
    unsigned n=0;
    for(uint32_t i=mo_r;i!=mo_w;i++) {
        uint32_t p=midi_out_q[i%MQ];
        if((p&15u)==9u) { assert((p>>24)==expected); n++; }
    }
    return n;
}
static unsigned voices(const track_t *t, unsigned expected)
{
    unsigned n=0;
    for(unsigned i=0;i<NVOICE;i++) if(t->v[i].active && t->v[i].gate) {
        assert(t->v[i].mvel==expected); n++;
    }
    return n;
}
int main(void)
{
    /* Whole mapping range, both sensor types, all curves and touch settings. */
    for(unsigned k=0;k<=20;k+=20) for(unsigned curve=0;curve<3;curve++) for(unsigned touch=0;touch<3;touch++) {
        unsigned prev= k ? 1:127;
        for(unsigned raw=0;raw<=1023;raw++) {
            unsigned v=kv_map(k,raw,(curve<<1)|(touch<<3));
            assert(v>=1 && v<=127); assert(k ? v>=prev : v<=prev); prev=v;
        }
    }
    start(); vpress(7); assert(ons(100)==0);
    land(7,750); unsigned vel=kv_map(7,750,kv_config); assert(ons(vel)==1);
    release(7); assert(!kb_chn[7]);
    /* Quick release while measuring cannot emit a late note. */
    start(); vpress(7); release(7); kv_sample(7,550); fm1_ms+=30; events_block(CTL); assert(ons(100)==0);
    /* No/invalid ADC result falls back once; no retrigger on a late result. */
    start(); vpress(7); kv_sample(7,1023); kv_sample(7,0); fm1_ms+=KV_TIMEOUT_MS; events_block(CTL);
    assert(ons(100)==1); land(7,550); assert(ons(100)==1); release(7);
    /* Mode switch pending, held, and repeated strokes. */
    start(); vpress(7); menu_put(MI_VMODE,0); events_block(CTL); assert(ons(100)==1);
    menu_put(MI_VMODE,1); land(7,550); assert(ons(100)==1); release(7); mo_r=mo_w;
    for(unsigned i=0;i<100;i++) { vpress(7); land(7,750); release(7); assert(ons(vel)==1); mo_r=mo_w; }
    /* Independent simultaneous keys (no event ring to overflow). */
    start(); vpress(7); vpress(9); kv_sample(7,550); kv_sample(7,550); kv_sample(9,850); kv_sample(9,850); events_block(CTL);
    assert(kv_velocity[7]==127 && kv_velocity[9]==1); release(7); release(9);
    /* Every physical key can finish a capture together without a queue cap. */
    start();
    for(unsigned k=0;k<27;k++) { if(k>=20) kv_sample(k,770); kv_contact(k,1); kv_sample(k,k<20?700:805); kv_sample(k,k<20?700:805); }
    assert(kv_keys((1u<<27)-1, fm1_ms,100,0,0)==(1u<<27)-1);
    for(unsigned k=0;k<27;k++) assert(kv_velocity[k]==kv_map(k,k<20?700:35,kv_config));
    /* Top row baseline -> two probe samples; separate fixed fallback. */
    start(); kv_sample(20,770); vpress(20); land(20,804); assert(ons(kv_map(20,34,kv_config))==1); release(20);
    start(); menu_put(MI_VTOP,1); vpress(20); assert(ons(100)==1); release(20);
    /* Maqam DIA3 and layered track inherit one velocity; pitch offsets remain. */
    start(); trk[0].p[P_ROOT]=0; trk[0].p[P_SCALE]=16; trk[0].p[P_CHRD]=CH_DIA3;
    song.g[G_KMODE]=1; song.g[G_KTRK]=3; song.g[G_KOCT]=2;
    vpress(7); land(7,750); assert(ons(vel)==kb_chn[7]+kb_chn2[7]); assert(kb_chn[7]==3 && kb_chn2[7]==1);
    for(unsigned t=0;t<NTRK;t++) for(unsigned i=0;i<NVOICE;i++) if(trk[t].v[i].active && trk[t].v[i].gate) assert(trk[t].v[i].mvel==vel);
    assert(tuning_cents(&trk[0],64)==-50); /* actual Rast E half-flat */
    release(7); assert(!kb_chn[7] && !kb_chn2[7]);
    /* Live recording retains the attack velocity (one velocity per step). */
    start(); track_defaults_steps(&trk[0]); song.playing=1; song.rec=1; trk[0].seq_pos=0;
    vpress(7); land(7,750); assert(trk[0].step[0].vel==vel); release(7);
    /* Playback uses the recorded velocity; same-step notes share the maximum. */
    song.rec=0; mo_r=mo_w;
    seq_step(&trk[0], &trk[0].step[0], 0, 4410, 0); assert(voices(&trk[0],vel)==1);
    start(); track_defaults_steps(&trk[0]); song.playing=1; song.rec=1; trk[0].seq_pos=0;
    vpress(7); land(7,850); vpress(9); land(9,550);
    assert(trk[0].step[0].n==2 && trk[0].step[0].vel==127);
    release(7); release(9);
    /* Sorted/octave lists keep the velocity attached to its original note. */
    start(); track_t *t=&trk[0]; t->p[P_AMODE]=AM_UP; t->p[P_AOCT]=2; t->p[P_AORDER]=0;
    input_on(t,67,35); input_on(t,60,115); input_on(t,64,70);
    uint32_t notes[64], count; uint8_t velocities[64];
    assert(arp_list(t,notes,velocities,&count)==6 && count==3);
    const uint32_t expected_notes[]={60,64,67,72,76,79};
    const uint8_t expected_vel[]={115,70,35,115,70,35};
    for(unsigned i=0;i<6;i++) { assert(notes[i]==expected_notes[i]); assert(velocities[i]==expected_vel[i]); }
    arp_remove(t,60); assert(arp_list(t,notes,velocities,&count)==4);
    assert(notes[0]==64 && velocities[0]==70 && notes[1]==67 && velocities[1]==35);
    t->p[P_AMODE]=AM_REPEAT; assert(arp_list(t,notes,velocities,&count)==1 && notes[0]==64 && velocities[0]==70);
    t->p[P_AHOLD]=1; arp_remove(t,67); arp_remove(t,64);
    input_on(t,62,48); assert(arp_list(t,notes,velocities,&count)==1 && notes[0]==62 && velocities[0]==48);
    /* Actual sensed key -> every arp mode -> voices, recording and playback. */
    for(unsigned mode=1;mode<=AM_DRONE;mode++) {
        start(); t=&trk[0]; t->p[P_AMODE]=mode; t->p[P_AOCT]=1; t->p[P_APROB]=127;
        vpress(7); land(7,750); vpress(9); land(9,750);
        assert(t->nheld==2 && held_vel[0][0]==vel && held_vel[0][1]==vel);
        song.playing=1; song.rec=1; t->seq_pos=0; t->seq_idx=0; track_defaults_steps(t);
        mo_r=mo_w; arp_off(t); arp_fire(t,4410); assert(voices(t,vel)>0);
        assert(t->step[0].vel==vel && t->step[0].n);
        song.rec=0; arp_off(t); mo_r=mo_w;
        seq_step(t,&t->step[0],0,4410,0); assert(voices(t,vel)==t->step[0].n);
        release(7); release(9);
    }
    /* DRONE/CHORD retain different live velocities even though recording shares one. */
    for(unsigned mode=0;mode<2;mode++) {
        start(); t=&trk[0]; t->p[P_AMODE]=mode?AM_DRONE:AM_CHORD; t->p[P_APROB]=127; t->p[P_AOCT]=1;
        input_on(t,67,35); input_on(t,60,115); arp_fire(t,4410);
        unsigned found=0;
        for(unsigned v=0;v<NVOICE;v++) if(t->v[v].active && t->v[v].gate) {
            assert(t->v[v].mvel==35 || t->v[v].mvel==115); found|=t->v[v].mvel==35?1:2;
        }
        assert(found==3);
    }
    /* Settings stay in the existing tagged GBS1 layout; old/unknown defaults. */
    start(); menu_put(MI_VCURVE,2); menu_put(MI_VTOUCH,1); menu_put(MI_VTOP,1);
    persist_t p={0}; settings_export(&p); uint32_t cfg=kv_config; kv_config=0;
    assert(settings_import(&p,sizeof p)); assert(kv_config==cfg);
    p.favorites.factory[15][5]=0; assert(settings_import(&p,sizeof p)); assert(!(kv_config&1));
    /* Wraparound timeout. */
    start(); fm1_ms=0xfffffff8u; vpress(7); fm1_ms+=KV_TIMEOUT_MS; events_block(CTL); assert(ons(100)==1); release(7);
    start(); ui.menu=1; ui.menu_sel=MI_VMODE; ui_prefs=PREF_LARGE; mt.pos=MTAB_TOUCH*MT_ONE; mt.frame=ui.frame;
    screen_save("build/assessment/velocity", "touch-menu-large");
    ui_prefs=0; screen_save("build/assessment/velocity", "touch-menu");
    puts("velocity: curves, independent attacks, timeout/release, modes, top probe, maqam chords, layers, MIDI, recording/playback, all arp modes, persistence pass");
    return 0;
}
