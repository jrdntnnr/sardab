/* SPDX-License-Identifier: GPL-3.0-only */
/* Cross-fork frozen records and actual simultaneous maqam/temperament voices. */
#define TUNING_TEST_NO_MAIN 1
#include "tuning_test.c"
static void read_fixture(void *p, size_t n, const char *name)
{
    FILE *f=fopen(name,"rb"); assert(f && fread(p,1,n,f)==n && fgetc(f)==EOF); fclose(f);
}
static void migration(void)
{
    project_store_t st, saved; project_t q, r; up_rec_t u;
    int16_t v[P_COUNT], def[P_COUNT];
    for(unsigned i=0;i<P_COUNT;i++)def[i]=param_desc_of(0,i)->def;
    read_fixture(&st,sizeof st,"tests/fixtures/sardab-b9d47b7.gbm2");
    assert(proj_import(&q,&st,sizeof st));
    for(unsigned t=0;t<NTRK;t++) {
        assert(q.t[t].p[P_TC0+t]==-23-(int)t && q.t[t].p[P_TC11]==37);
        assert(q.t[t].p[P_SCALE]==16+t%3 && q.t[t].p[P_TEMP]==0 && q.t[t].p[P_E4]==100+t);
    }
    assert(q.motion.event[0].param==(P_E4|MOTION_LOCK) && q.motion.event[1].param==P_PAN);
    proj_tx_get(&q); proj_keys_get(&q);
    assert(song.g[G_TXSRC]==TX_WIND && song.g[G_TXLVL]==73 && song.g[G_TXTONE]==-20 && song.g[G_TXMOVE]==52);
    assert(song.g[G_KMODE]==0 && song.g[G_KTRK]==3 && song.g[G_KOCT]==2);
    assert(proj_pack(&saved,&q) && proj_import(&r,&saved,sizeof saved) && !memcmp(q.t,r.t,sizeof q.t));
    read_fixture(&u,sizeof u,"tests/fixtures/sardab-b9d47b7.up8");
    assert(up_valid(&u)); up_params(&u,v,def);
    assert(v[P_TC0]==-23 && v[P_TC11]==37 && v[P_TEMP]==0 && v[P_E4]==100);

    read_fixture(&st,sizeof st,"tests/fixtures/ghoulbox-2.1.funa");
    assert(proj_import(&q,&st,sizeof st));
    const uint8_t scales[]={22,32,23};
    for(unsigned t=0;t<NTRK;t++) {
        assert(q.t[t].p[P_TEMP]==1+t%3 && q.t[t].p[P_SCALE]==scales[t%3] && q.t[t].p[P_E4]==100+t);
        for(unsigned i=P_TC0;i<=P_TC11;i++)assert(q.t[t].p[i]==0);
    }
    assert(q.motion.event[0].param==(P_E4|MOTION_LOCK) && q.motion.event[1].param==P_PAN);
    assert(q.g[G_TAPE]==31 && q.g[G_CRSH]==27);
    proj_tx_get(&q); proj_keys_get(&q);
    assert(song.g[G_KMODE]==2 && song.g[G_KTRK]==1 && song.g[G_KOCT]==4 && song.g[G_KSPLIT]==7);
    assert(song.g[G_TXSRC]==TX_CAVE && song.g[G_TXLVL]==32 && song.g[G_TXWHEN]==1);
    assert(proj_pack(&saved,&q) && proj_import(&r,&saved,sizeof saved) && !memcmp(q.t,r.t,sizeof q.t) && q.keys==r.keys);
    read_fixture(&u,sizeof u,"tests/fixtures/ghoulbox-2.1.up4");
    assert(up_valid(&u)); up_params(&u,v,def);
    assert(v[P_TEMP]==1 && v[P_SCALE]==22 && v[P_TC0]==0 && v[P_E4]==100);
    puts("Ghoulbox 2.1 and previous SARDAB: frozen projects, presets, automation, tuning, texture, KEYS preserved");
}
/* Construct the upstream 2.3 wire layout from a frozen 2.1 record: its only
 * per-track addition is RING at 104 before the eight engine values. */
static void migration23(void)
{
    project_store_t old, st, saved; project_t q,r; up_rec_t u;
    read_fixture(&old,sizeof old,"tests/fixtures/ghoulbox-2.1.funa");
    st=old; unsigned stride=112+2+NSTEP*9, pos=68;
    for(unsigned t=0;t<NTRK;t++) {
        memcpy(st.raw+pos,old.raw+68+t*stride,104); st.raw[pos+104]=65;
        memcpy(st.raw+pos+105,old.raw+68+t*stride+104,stride-104); pos+=stride+1;
    }
    memcpy(st.raw+pos,old.raw+68+NTRK*stride,sizeof(chain_config_t)+sizeof(motion_store_t));
    st.raw[66]=113;
    uint16_t controls=656; memcpy(st.raw+PROJ_FM6_OFF-6,&controls,2);
    motion_store_t *m=(motion_store_t *)(st.raw+pos+sizeof(chain_config_t));
    m->event[0].param=(105+4)|MOTION_LOCK;
    m->event[1].param=104|MOTION_LOCK; m->event[1].value=1;
    uint32_t hash=proj_hash(st.raw,sizeof st-4);memcpy(st.raw+sizeof st-4,&hash,4);
    assert(proj_import(&q,&st,sizeof st));
    assert(q.dream==656 && q.keys && q.g[G_TAPE]==31);
    for(unsigned t=0;t<NTRK;t++) {
        assert(q.t[t].p[P_RING]==1 && q.t[t].p[P_TEMP]==1+t%3 && q.t[t].p[P_E4]==100+t);
        for(unsigned k=P_TC0;k<=P_TC11;k++) assert(!q.t[t].p[k]);
    }
    assert(q.motion.event[0].param==(P_E4|MOTION_LOCK));
    assert(q.motion.event[1].param==(P_RING|MOTION_LOCK));
    assert(proj_pack(&saved,&q) && proj_import(&r,&saved,sizeof saved));
    assert(r.dream==q.dream && !memcmp(q.t,r.t,sizeof q.t));
    read_fixture(&u,sizeof u,"tests/fixtures/ghoulbox-2.1.up4");
    memmove(u.packed+105,u.packed+104,8);u.packed[104]=65;u.np=113;
    int16_t v[P_COUNT],def[P_COUNT];
    for(unsigned i=0;i<P_COUNT;i++)def[i]=param_desc_of(u.engine,i)->def;
    assert(up_valid(&u));up_params(&u,v,def);
    assert(v[P_RING]==1 && v[P_TEMP]==1 && v[P_TC0]==0 && v[P_TC1]==0 && v[P_E4]==100);
    /* Previous SARDAB np127 has the same packed record, without the RING bit or DREAM word. */
    st=saved;st.raw[66]=127;
    motion_store_t *legacy=(motion_store_t *)(st.raw+68+NTRK*(PROJ_PARAM_BYTES+2+NSTEP*9)+sizeof(chain_config_t));
    legacy->event[1].param=P_PAN; /* old firmware had no RING parameter */
    for(unsigned t=0;t<NTRK;t++) st.raw[68+t*(PROJ_PARAM_BYTES+2+NSTEP*9)+PROJ_TEMP_BYTE]&=~32u;
    hash=proj_hash(st.raw,sizeof st-4);memcpy(st.raw+sizeof st-4,&hash,4);
    assert(proj_import(&q,&st,sizeof st) && q.dream==0);
    for(unsigned t=0;t<NTRK;t++) assert(q.t[t].p[P_RING]==0 && q.t[t].p[P_E4]==100+t);
    puts("Ghoulbox 2.3: RING/TEMP/engine migration, motion locks, DREAM/KEYS; old SARDAB np127 defaults preserved");
}
static void temperament_audio(void)
{
    /* Independent expected just-intonation offsets, Rast's thirds and sevenths
     * lowered by 50ct plus an owner trim on the third. Phase of real voices. */
    const uint8_t notes[]={60,64,67,71};
    const double cents[]={0,-50+13-13.7,2,-50-11.7};
    reset(); track_t *t=&trk[0]; t->p[P_SCALE]=16; t->p[P_TEMP]=3; t->p[P_TC4]=13;
    t->p[P_CHRD]=CH_DIA7;
    uint8_t chord[4]; assert(chord_build(t,60,chord)==4 && !memcmp(chord,notes,4));
    uint32_t phases[4]; int32_t out[CTL];
    for(unsigned i=0;i<4;i++){trk_note_on(t,notes[i],100);phases[i]=voice(t,notes[i])->ph[0];}
    track_render(t,out,CTL);
    for(unsigned i=0;i<4;i++){
        double want=440*pow(2,((int)notes[i]-69+cents[i]/100)/12);
        double got=(uint32_t)(voice(t,notes[i])->ph[0]-phases[i])*(double)FS/CTL/4294967296.0;
        assert(fabs(error(got,want))<0.5);
    }
    for(unsigned mode=0;mode<4;mode++)for(unsigned root=0;root<12;root++)for(unsigned n=24;n<100;n++) {
        t->p[P_TEMP]=mode; t->p[P_ROOT]=root;
        unsigned pc=(n+12-root)%12;
        double c=SCALE_CENTS[16][pc]+(pc==4?13:0)+TEMPER[mode][pc]/10.;
        double want=440*pow(2,((int)n-69+c/100)/12);
        uint32_t inc=cents_inc(tuning_pitch16(t,n),0,tuning_residual(t,n));
        assert(fabs(error(inc*(double)FS/4294967296.,want))<0.5);
    }
    puts("temperaments + maqam + owner trims: simultaneous seventh chord, all tonics, pitch error <0.5ct");
}
int main(void){ migration(); migration23(); temperament_audio(); return 0; }
