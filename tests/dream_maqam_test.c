/* SPDX-License-Identifier: GPL-3.0-only */
/* DREAM shares the selected maqam's pitch map, including Scala and user trims. */
#define UI_TEST_NO_MAIN 1
#include "ui_test.c"
#include <assert.h>
int main(void)
{
    unsigned pieces=0;
    for(unsigned scl=0;scl<NELEM(SCALE_MASK);scl++) for(unsigned root=0;root<12;root++) {
        ui_power_on(); song.sel=2;
        for(unsigned t=0;t<NTRK;t++) { set_engine_of(&trk[t],0); engine_block(&trk[t]); apply_preset_to(&trk[t],0); }
        TSEL->p[P_SCALE]=scl; TSEL->p[P_TEMP]=3;
        TSEL->p[P_TC4]=13; TSEL->p[P_TC11]=-7;
        TSEL->p[P_TMASK0]=0x15; TSEL->p[P_TMASK1]=0x24;
        unsigned mask=scale_mask(TSEL); int16_t tune[15];
        memcpy(tune,&TSEL->p[P_TC0],sizeof tune);
        song.g[G_DMOOD]=6; dream_compose(root);
        for(unsigned pass=0;pass<10;pass++) {
            for(unsigned t=0;t<NTRK;t++) {
                assert(trk[t].p[P_SCALE]==scl && trk[t].p[P_ROOT]==root);
                assert(!memcmp(tune,&trk[t].p[P_TC0],sizeof tune));
                for(unsigned k=0;k<NSTEP;k++) for(unsigned j=0;j<trk[t].step[k].n;j++) {
                    unsigned n=trk[t].step[k].note[j], pc=(n+12-root)%12;
                    assert(mask>>pc&1u);
                    double cents=SCALE_CENTS[scl][pc]+tune[pc]+TEMPER[3][pc]/10.;
                    double want=440.*pow(2.,((int)n-69+cents/100.)/12.);
                    double got=cents_inc(tuning_pitch16(&trk[t],n),0,tuning_residual(&trk[t],n))*(double)FS/4294967296.;
                    assert(fabs(1200.*log2(got/want))<0.5);
                }
            }
            dream_evolve();
        }
        pieces++;
    }
    /* Empty custom scale remains bounded and produces a root, never divides by zero. */
    TSEL->p[P_SCALE]=SCALE_CUSTOM_ID; TSEL->p[P_TMASK0]=TSEL->p[P_TMASK1]=0;
    dream_compose(0); assert(dream.mask); dream_evolve();
    /* Independent Rast pitches, then actual simultaneous oscillator phases of the generated notes. */
    ui_power_on(); song.sel=0;
    for(unsigned t=0;t<NTRK;t++) { set_engine_of(&trk[t],0); engine_block(&trk[t]); apply_preset_to(&trk[t],3); } /* ANALOG SINE KEY */
    trk[0].p[P_SCALE]=16; trk[0].p[P_TC4]=13;
    song.g[G_DMOOD]=6; dream_compose(0);
    const unsigned notes[]={60,64,67,71}; const int cents[]={0,-37,0,-50};
    for(unsigned t=0;t<NTRK;t++) { memset(trk[t].v,0,sizeof trk[t].v); trk[t].engine=trk[t].eng_req=0; trk[t].xf_on=0; trk[t].p[P_E1]=0; }
    uint32_t phase[4]; voice_t *v[4]; int32_t audio[CTL];
    for(unsigned i=0;i<4;i++) {
        trk_note_on(&trk[i],notes[i],96); v[i]=0;
        for(unsigned j=0;j<NVOICE;j++) if(trk[i].v[j].active && trk[i].v[j].note==notes[i])v[i]=&trk[i].v[j];
        assert(v[i]); phase[i]=v[i]->ph[0];
    }
    for(unsigned i=0;i<4;i++) {
        track_render(&trk[i],audio,CTL);
        double got=(uint32_t)(v[i]->ph[0]-phase[i])*(double)FS/CTL/4294967296.;
        double want=440.*pow(2.,((int)notes[i]-69+cents[i]/100.)/12.);
        assert(fabs(1200.*log2(got/want))<0.5);
    }
    /* Full runtime save/restore of DREAM controls, RING and existing tuning. */
    project_t q; project_store_t st;
    song.g[G_DREAM]=1; song.g[G_DDENS]=8; song.g[G_DDRFT]=3; trk[0].p[P_RING]=1;
    project_capture(&q); assert(proj_pack(&st,&q)); assert(proj_import(&q,&st,sizeof st));
    song.g[G_DMOOD]=0; assert(project_restore_runtime(&q)==0);
    assert(song.g[G_DREAM]==1 && song.g[G_DDENS]==8 && song.g[G_DDRFT]==3 && song.g[G_DMOOD]==6);
    assert(trk[0].p[P_RING]==1 && trk[0].p[P_TC4]==13 && !dream.valid);
    /* Reduced FM6 LUT: a full fractional octave sweep against the analytic function. */
    double worst=0;
    for(int32_t x=0;x<16777216;x+=127) {
        double got=fm6_mant(x)/1073741824.,want=pow(2.,x/16777216.);
        double err=fabs(1200.*log2(got/want)); if(err>worst)worst=err;
    }
    assert(worst<0.0001);
    printf("DREAM: %u scale/tonic combinations x 10 evolutions; Scala, trims, polyphonic Rast, persistence; FM6 max error %.8f cents\n",pieces,worst);
    return 0;
}
