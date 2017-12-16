#include "guitar.h"
#include "songlib.h"

static void guitar_Ab_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,6,4,5,6,4,6,3,5,2,4,1,4,IX);
    }

static void guitar_Gsm_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,6,4,5,6,4,6,3,4,2,4,1,4,IX);
    }

static void guitar_Ab6_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,6,4,5,3,4,1,3,1,2,1,1,1,IX);
    }

static void guitar_Ab7_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,1,3,1,2,1,1,2,IX);
    }

static void guitar_Ab9_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,1,3,3,2,1,1,2,IX);
    }

static void guitar_Gsm6_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,3,3,4,2,4,1,4,IX);
    }

static void guitar_Gsm7_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,1,3,1,2,0,1,2,IX);
    }

static void guitar_AbM7_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,1,3,1,2,1,1,3,IX);
    }

static void guitar_Gsdim_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,0,3,1,2,0,1,1,IX);
    }

static void guitar_Abaug_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,2,3,1,2,1,1,0,IX);
    }

static void guitar_Absus_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,1,3,1,2,2,1,4,IX);
    }

static void guitar_A_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,5,0,4,2,3,2,2,2,1,0,IX);
    }

static void guitar_Am_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,5,0,4,2,3,2,2,1,1,0,IX);
    }

static void guitar_A6_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,5,0,4,2,3,2,2,2,1,2,IX);
    }

static void guitar_A7_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,5,0,4,2,3,2,2,2,1,3,IX);
    }

static void guitar_A9_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,5,0,4,2,3,4,2,2,1,3,IX);
    }

static void guitar_Am6_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,6,0,5,0,4,2,3,2,2,1,1,2,IX);
    }

static void guitar_Am7_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,5,0,4,2,3,2,2,1,1,3,IX);
    }

static void guitar_AM7_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,5,0,4,2,3,1,2,2,1,0,IX);
    }

static void guitar_Adim_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,1,3,2,2,1,1,2,IX);
    }

static void guitar_Aaug_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,5,0,4,3,3,2,2,2,1,1,IX);
    }

static void guitar_Asus_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,2,3,2,2,3,1,0,IX);
    }

static void guitar_Bb_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,5,1,4,3,3,3,2,3,1,1,IX);
    }

static void guitar_Bbm_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,5,1,4,3,3,3,2,2,1,1,IX);
    }

static void guitar_Bb6_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,6,1,5,1,4,3,3,3,2,3,1,3,IX);
    }

static void guitar_Bb7_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,3,3,3,2,3,1,4,IX);
    }

static void guitar_Bb9_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,6,1,5,1,4,3,3,1,2,1,1,1,IX);
    }

static void guitar_Bbm6_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,3,3,3,2,2,1,3,IX);
    }

static void guitar_Bbm7_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,3,3,3,2,2,1,4,IX);
    }

static void guitar_BbM7_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,5,1,4,3,3,2,2,3,IX);
    }

static void guitar_Bbdim_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,2,3,3,2,2,1,3,IX);
    }

static void guitar_Bbaug_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,0,3,3,2,3,1,2,IX);
    }

static void guitar_Bbsus_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,3,3,3,2,4,1,1,IX);
    }

static void guitar_B_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,5,2,4,4,3,4,2,4,1,2,IX);
    }

static void guitar_Bm_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,5,2,4,4,3,4,2,3,1,2,IX);
    }

static void guitar_B6_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,6,2,5,2,4,4,3,4,2,4,1,4,IX);
    }

static void guitar_B7_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,6,0,5,2,4,1,3,2,2,0,1,2,IX);
    }

static void guitar_B9_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,5,2,4,1,3,2,2,2,1,2,IX);
    }

static void guitar_Bm6_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,5,3,5,2,4,1,5,IX);
    }

static void guitar_Bm7_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,5,2,4,4,3,2,2,3,1,2,IX);
    }

static void guitar_BM7_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,5,2,4,4,3,3,2,4,IX);
    }

static void guitar_Bdim_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,0,3,1,2,0,1,1,IX);
    }

static void guitar_Baug_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,5,3,4,2,4,1,3,IX);
    }

static void guitar_Bsus_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,4,3,4,2,5,1,2,IX);
    }

static void guitar_C_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,6,3,5,3,4,2,3,0,2,1,1,0,IX);
    }

static void guitar_Cm_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,5,3,4,5,3,5,2,4,1,3,IX);
    }

static void guitar_C6_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,5,0,4,2,3,2,2,1,1,3,IX);
    }

static void guitar_C7_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,6,0,5,3,4,2,3,3,2,1,1,0,IX);
    }

static void guitar_C9_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,5,3,4,2,3,3,2,3,1,3,IX);
    }

static void guitar_Cm6_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,1,3,2,2,1,1,3,IX);
    }

static void guitar_Cm7_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,1,3,3,2,1,1,3,IX);
    }

static void guitar_CM7_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,5,3,4,2,3,0,2,0,1,0,IX);
    }

static void guitar_Cdim_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,1,3,2,2,1,1,2,IX);
    }

static void guitar_Caug_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,2,3,1,2,1,1,0,IX);
    }

static void guitar_Csus_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,3,3,0,2,1,1,3,IX);
    }

static void guitar_Db_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,3,3,1,2,2,1,1,IX);
    }

static void guitar_Csm_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,2,3,1,2,2,1,0,IX);
    }

static void guitar_Db6_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,3,3,3,2,2,1,4,IX);
    }

static void guitar_Db7_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,3,3,4,2,2,1,4,IX);
    }

static void guitar_Db9_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,5,4,4,3,3,4,2,4,1,4,IX);
    }

static void guitar_Csm6_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,2,3,3,2,2,1,4,IX);
    }

static void guitar_Csm7_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,2,3,4,2,2,1,4,IX);
    }

static void guitar_DbM7_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,5,4,4,3,3,1,2,1,1,1,IX);
    }

static void guitar_Csdim_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,2,3,3,2,2,1,3,IX);
    }

static void guitar_Dbaug_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,3,3,2,2,2,1,1,IX);
    }

static void guitar_Dbsus_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,6,3,6,2,7,1,4,IX);
    }

static void guitar_D_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,0,3,2,2,3,1,2,IX);
    }

static void guitar_Dm_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,0,3,2,2,3,1,1,IX);
    }

static void guitar_D6_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,5,0,4,0,3,2,2,0,1,2,IX);
    }

static void guitar_D7_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,0,3,2,2,1,1,2,IX);
    }

static void guitar_D9_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,6,2,5,0,4,0,3,2,2,1,1,0,IX);
    }

static void guitar_Dm6_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,0,3,2,2,0,1,1,IX);
    }

static void guitar_Dm7_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,0,3,2,2,1,1,1,IX);
    }

static void guitar_DM7_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,0,3,2,2,2,1,2,IX);
    }

static void guitar_Ddim_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,0,3,1,2,0,1,1,IX);
    }

static void guitar_Daug_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,0,3,3,2,3,1,2,IX);
    }

static void guitar_Dsus_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,0,3,2,2,3,1,3,IX);
    }

static void guitar_Eb_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,5,3,3,2,4,1,3,IX);
    }

static void guitar_Ebm_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,4,3,3,2,4,1,2,IX);
    }

static void guitar_Eb6_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,1,3,3,2,1,1,3,IX);
    }

static void guitar_Eb7_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,1,3,3,2,2,1,3,IX);
    }

static void guitar_Eb9_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,6,1,5,1,4,1,3,3,2,2,1,1,IX);
    }

static void guitar_Ebm6_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,1,3,3,2,1,1,2,IX);
    }

static void guitar_Ebm7_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,1,3,3,2,2,1,2,IX);
    }

static void guitar_EbM7_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,1,3,3,2,3,1,3,IX);
    }

static void guitar_Ebdim_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,1,3,2,2,1,1,2,IX);
    }

static void guitar_Ebaug_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,1,3,0,2,0,1,3,IX);
    }

static void guitar_Ebsus_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,6,0,5,2,4,2,3,2,2,0,1,0,IX);
    }

static void guitar_E_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,6,0,5,2,4,2,3,1,2,0,1,0,IX);
    }

static void guitar_Em_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,6,0,5,2,4,2,3,0,2,0,1,0,IX);
    }

static void guitar_E6_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,6,0,5,2,4,2,3,1,2,2,1,0,IX);
    }

static void guitar_E7_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,6,0,5,2,4,2,3,1,2,3,1,0,IX);
    }

static void guitar_E9_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,6,0,5,2,4,0,3,1,2,0,1,2,IX);
    }

static void guitar_Em6_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,6,0,5,2,4,2,3,0,2,2,1,0,IX);
    }

static void guitar_Em7_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,6,0,5,2,4,0,3,0,2,0,1,0,IX);
    }

static void guitar_EM7_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,6,0,5,2,4,1,3,1,2,0,IX);
    }

static void guitar_Edim_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,2,3,3,2,2,1,3,IX);
    }

static void guitar_Eaug_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,2,3,1,2,1,1,0,IX);
    }

static void guitar_Esus_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,6,0,5,2,4,2,3,2,2,0,1,0,IX);
    }

static void guitar_F_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,6,1,5,3,4,3,3,2,2,1,1,1,IX);
    }

static void guitar_Fm_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,6,1,5,3,4,3,3,1,2,1,1,1,IX);
    }

static void guitar_F6_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,0,3,2,2,1,1,1,IX);
    }

static void guitar_F7_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,6,1,5,3,4,1,3,2,2,1,1,1,IX);
    }

static void guitar_F9_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,3,3,2,2,4,1,3,IX);
    }

static void guitar_Fm6_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,0,3,1,2,1,1,1,IX);
    }

static void guitar_Fm7_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,6,1,5,3,4,1,3,1,2,1,1,1,IX);
    }

static void guitar_FM7_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,3,3,2,2,1,1,0,IX);
    }

static void guitar_Fdim_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,0,3,1,2,0,1,1,IX);
    }

static void guitar_Faug_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,3,3,2,2,2,1,1,IX);
    }

static void guitar_Fsus_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,3,3,3,2,1,1,1,IX);
    }

static void guitar_Fs_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,6,2,5,4,4,4,3,3,2,2,1,2,IX);
    }

static void guitar_Fsm_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,6,2,5,4,4,4,3,2,2,2,1,2,IX);
    }

static void guitar_Gb6_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,5,4,4,4,3,3,2,4,IX);
    }

static void guitar_Fs7_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,4,3,3,2,2,1,0,IX);
    }

static void guitar_Fs9_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,4,3,3,2,5,1,4,IX);
    }

static void guitar_Fsm6_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,1,3,2,2,2,1,2,IX);
    }

static void guitar_Fsm7_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,2,3,2,2,2,1,2,IX);
    }

static void guitar_GbM7_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,4,3,3,2,2,1,1,IX);
    }

static void guitar_Fsdim_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,1,3,2,2,1,1,2,IX);
    }

static void guitar_Gbaug_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,4,3,3,2,3,1,2,IX);
    }

static void guitar_Gbsus_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,4,3,4,2,2,1,2,IX);
    }

static void guitar_G_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,6,3,5,2,4,0,3,0,2,0,1,3,IX);
    }

static void guitar_Gm_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,6,3,5,5,4,5,3,3,2,3,1,3,IX);
    }

static void guitar_G6_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,6,3,5,2,4,0,3,0,2,0,1,0,IX);
    }

static void guitar_G7_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,6,3,5,2,4,0,3,0,2,0,1,1,IX);
    }

static void guitar_G9_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,6,3,5,0,4,0,3,2,2,0,1,1,IX);
    }

static void guitar_Gm6_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,2,3,3,2,3,1,3,IX);
    }

static void guitar_Gm7_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,6,3,5,5,4,3,3,3,2,3,1,3,IX);
    }

static void guitar_GM7_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,5,3,4,2,3,1,2,IX);
    }

static void guitar_Gdim_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,2,3,3,2,2,1,3,IX);
    }

static void guitar_Gaug_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,1,3,0,2,0,1,3,IX);
    }

static void guitar_Gsus_std(double beats,int *inst,int dir)
    {
    playStrings(beats,inst,dir,4,0,3,0,2,1,1,3,IX);
    }

void (*guitar_Ab)(double beats,int *inst,int dir) = guitar_Ab_std;

void (*guitar_Abm)(double beats,int *inst,int dir) = guitar_Gsm_std;

void (*guitar_Ab6)(double beats,int *inst,int dir) = guitar_Ab6_std;

void (*guitar_Ab7)(double beats,int *inst,int dir) = guitar_Ab7_std;

void (*guitar_Ab9)(double beats,int *inst,int dir) = guitar_Ab9_std;

void (*guitar_Abm6)(double beats,int *inst,int dir) = guitar_Gsm6_std;

void (*guitar_Abm7)(double beats,int *inst,int dir) = guitar_Gsm7_std;

void (*guitar_AbM7)(double beats,int *inst,int dir) = guitar_AbM7_std;

void (*guitar_Abdim)(double beats,int *inst,int dir) = guitar_Gsdim_std;

void (*guitar_Abaug)(double beats,int *inst,int dir) = guitar_Abaug_std;

void (*guitar_Absus)(double beats,int *inst,int dir) = guitar_Absus_std;

void (*guitar_A)(double beats,int *inst,int dir) = guitar_A_std;

void (*guitar_Am)(double beats,int *inst,int dir) = guitar_Am_std;

void (*guitar_A6)(double beats,int *inst,int dir) = guitar_A6_std;

void (*guitar_A7)(double beats,int *inst,int dir) = guitar_A7_std;

void (*guitar_A9)(double beats,int *inst,int dir) = guitar_A9_std;

void (*guitar_Am6)(double beats,int *inst,int dir) = guitar_Am6_std;

void (*guitar_Am7)(double beats,int *inst,int dir) = guitar_Am7_std;

void (*guitar_AM7)(double beats,int *inst,int dir) = guitar_AM7_std;

void (*guitar_Adim)(double beats,int *inst,int dir) = guitar_Adim_std;

void (*guitar_Aaug)(double beats,int *inst,int dir) = guitar_Aaug_std;

void (*guitar_Asus)(double beats,int *inst,int dir) = guitar_Asus_std;

void (*guitar_Bb)(double beats,int *inst,int dir) = guitar_Bb_std;

void (*guitar_Bbm)(double beats,int *inst,int dir) = guitar_Bbm_std;

void (*guitar_Bb6)(double beats,int *inst,int dir) = guitar_Bb6_std;

void (*guitar_Bb7)(double beats,int *inst,int dir) = guitar_Bb7_std;

void (*guitar_Bb9)(double beats,int *inst,int dir) = guitar_Bb9_std;

void (*guitar_Bbm6)(double beats,int *inst,int dir) = guitar_Bbm6_std;

void (*guitar_Bbm7)(double beats,int *inst,int dir) = guitar_Bbm7_std;

void (*guitar_BbM7)(double beats,int *inst,int dir) = guitar_BbM7_std;

void (*guitar_Bbdim)(double beats,int *inst,int dir) = guitar_Bbdim_std;

void (*guitar_Bbaug)(double beats,int *inst,int dir) = guitar_Bbaug_std;

void (*guitar_Bbsus)(double beats,int *inst,int dir) = guitar_Bbsus_std;

void (*guitar_B)(double beats,int *inst,int dir) = guitar_B_std;

void (*guitar_Bm)(double beats,int *inst,int dir) = guitar_Bm_std;

void (*guitar_B6)(double beats,int *inst,int dir) = guitar_B6_std;

void (*guitar_B7)(double beats,int *inst,int dir) = guitar_B7_std;

void (*guitar_B9)(double beats,int *inst,int dir) = guitar_B9_std;

void (*guitar_Bm6)(double beats,int *inst,int dir) = guitar_Bm6_std;

void (*guitar_Bm7)(double beats,int *inst,int dir) = guitar_Bm7_std;

void (*guitar_BM7)(double beats,int *inst,int dir) = guitar_BM7_std;

void (*guitar_Bdim)(double beats,int *inst,int dir) = guitar_Bdim_std;

void (*guitar_Baug)(double beats,int *inst,int dir) = guitar_Baug_std;

void (*guitar_Bsus)(double beats,int *inst,int dir) = guitar_Bsus_std;

void (*guitar_C)(double beats,int *inst,int dir) = guitar_C_std;

void (*guitar_Cm)(double beats,int *inst,int dir) = guitar_Cm_std;

void (*guitar_C6)(double beats,int *inst,int dir) = guitar_C6_std;

void (*guitar_C7)(double beats,int *inst,int dir) = guitar_C7_std;

void (*guitar_C9)(double beats,int *inst,int dir) = guitar_C9_std;

void (*guitar_Cm6)(double beats,int *inst,int dir) = guitar_Cm6_std;

void (*guitar_Cm7)(double beats,int *inst,int dir) = guitar_Cm7_std;

void (*guitar_CM7)(double beats,int *inst,int dir) = guitar_CM7_std;

void (*guitar_Cdim)(double beats,int *inst,int dir) = guitar_Cdim_std;

void (*guitar_Caug)(double beats,int *inst,int dir) = guitar_Caug_std;

void (*guitar_Csus)(double beats,int *inst,int dir) = guitar_Csus_std;

void (*guitar_Cs)(double beats,int *inst,int dir) = guitar_Db_std;

void (*guitar_Csm)(double beats,int *inst,int dir) = guitar_Csm_std;

void (*guitar_Cs6)(double beats,int *inst,int dir) = guitar_Db6_std;

void (*guitar_Cs7)(double beats,int *inst,int dir) = guitar_Db7_std;

void (*guitar_Cs9)(double beats,int *inst,int dir) = guitar_Db9_std;

void (*guitar_Csm6)(double beats,int *inst,int dir) = guitar_Csm6_std;

void (*guitar_Csm7)(double beats,int *inst,int dir) = guitar_Csm7_std;

void (*guitar_CsM7)(double beats,int *inst,int dir) = guitar_DbM7_std;

void (*guitar_Csdim)(double beats,int *inst,int dir) = guitar_Csdim_std;

void (*guitar_Csaug)(double beats,int *inst,int dir) = guitar_Dbaug_std;

void (*guitar_Cssus)(double beats,int *inst,int dir) = guitar_Dbsus_std;

void (*guitar_Db)(double beats,int *inst,int dir) = guitar_Db_std;

void (*guitar_Dbm)(double beats,int *inst,int dir) = guitar_Csm_std;

void (*guitar_Db6)(double beats,int *inst,int dir) = guitar_Db6_std;

void (*guitar_Db7)(double beats,int *inst,int dir) = guitar_Db7_std;

void (*guitar_Db9)(double beats,int *inst,int dir) = guitar_Db9_std;

void (*guitar_Dbm6)(double beats,int *inst,int dir) = guitar_Csm6_std;

void (*guitar_Dbm7)(double beats,int *inst,int dir) = guitar_Csm7_std;

void (*guitar_DbM7)(double beats,int *inst,int dir) = guitar_DbM7_std;

void (*guitar_Dbdim)(double beats,int *inst,int dir) = guitar_Csdim_std;

void (*guitar_Dbaug)(double beats,int *inst,int dir) = guitar_Dbaug_std;

void (*guitar_Dbsus)(double beats,int *inst,int dir) = guitar_Dbsus_std;

void (*guitar_D)(double beats,int *inst,int dir) = guitar_D_std;

void (*guitar_Dm)(double beats,int *inst,int dir) = guitar_Dm_std;

void (*guitar_D6)(double beats,int *inst,int dir) = guitar_D6_std;

void (*guitar_D7)(double beats,int *inst,int dir) = guitar_D7_std;

void (*guitar_D9)(double beats,int *inst,int dir) = guitar_D9_std;

void (*guitar_Dm6)(double beats,int *inst,int dir) = guitar_Dm6_std;

void (*guitar_Dm7)(double beats,int *inst,int dir) = guitar_Dm7_std;

void (*guitar_DM7)(double beats,int *inst,int dir) = guitar_DM7_std;

void (*guitar_Ddim)(double beats,int *inst,int dir) = guitar_Ddim_std;

void (*guitar_Daug)(double beats,int *inst,int dir) = guitar_Daug_std;

void (*guitar_Dsus)(double beats,int *inst,int dir) = guitar_Dsus_std;

void (*guitar_Eb)(double beats,int *inst,int dir) = guitar_Eb_std;

void (*guitar_Ebm)(double beats,int *inst,int dir) = guitar_Ebm_std;

void (*guitar_Eb6)(double beats,int *inst,int dir) = guitar_Eb6_std;

void (*guitar_Eb7)(double beats,int *inst,int dir) = guitar_Eb7_std;

void (*guitar_Eb9)(double beats,int *inst,int dir) = guitar_Eb9_std;

void (*guitar_Ebm6)(double beats,int *inst,int dir) = guitar_Ebm6_std;

void (*guitar_Ebm7)(double beats,int *inst,int dir) = guitar_Ebm7_std;

void (*guitar_EbM7)(double beats,int *inst,int dir) = guitar_EbM7_std;

void (*guitar_Ebdim)(double beats,int *inst,int dir) = guitar_Ebdim_std;

void (*guitar_Ebaug)(double beats,int *inst,int dir) = guitar_Ebaug_std;

void (*guitar_Ebsus)(double beats,int *inst,int dir) = guitar_Ebsus_std;

void (*guitar_E)(double beats,int *inst,int dir) = guitar_E_std;

void (*guitar_Em)(double beats,int *inst,int dir) = guitar_Em_std;

void (*guitar_E6)(double beats,int *inst,int dir) = guitar_E6_std;

void (*guitar_E7)(double beats,int *inst,int dir) = guitar_E7_std;

void (*guitar_E9)(double beats,int *inst,int dir) = guitar_E9_std;

void (*guitar_Em6)(double beats,int *inst,int dir) = guitar_Em6_std;

void (*guitar_Em7)(double beats,int *inst,int dir) = guitar_Em7_std;

void (*guitar_EM7)(double beats,int *inst,int dir) = guitar_EM7_std;

void (*guitar_Edim)(double beats,int *inst,int dir) = guitar_Edim_std;

void (*guitar_Eaug)(double beats,int *inst,int dir) = guitar_Eaug_std;

void (*guitar_Esus)(double beats,int *inst,int dir) = guitar_Esus_std;

void (*guitar_F)(double beats,int *inst,int dir) = guitar_F_std;

void (*guitar_Fm)(double beats,int *inst,int dir) = guitar_Fm_std;

void (*guitar_F6)(double beats,int *inst,int dir) = guitar_F6_std;

void (*guitar_F7)(double beats,int *inst,int dir) = guitar_F7_std;

void (*guitar_F9)(double beats,int *inst,int dir) = guitar_F9_std;

void (*guitar_Fm6)(double beats,int *inst,int dir) = guitar_Fm6_std;

void (*guitar_Fm7)(double beats,int *inst,int dir) = guitar_Fm7_std;

void (*guitar_FM7)(double beats,int *inst,int dir) = guitar_FM7_std;

void (*guitar_Fdim)(double beats,int *inst,int dir) = guitar_Fdim_std;

void (*guitar_Faug)(double beats,int *inst,int dir) = guitar_Faug_std;

void (*guitar_Fsus)(double beats,int *inst,int dir) = guitar_Fsus_std;

void (*guitar_Fs)(double beats,int *inst,int dir) = guitar_Fs_std;

void (*guitar_Fsm)(double beats,int *inst,int dir) = guitar_Fsm_std;

void (*guitar_Fs6)(double beats,int *inst,int dir) = guitar_Gb6_std;

void (*guitar_Fs7)(double beats,int *inst,int dir) = guitar_Fs7_std;

void (*guitar_Fs9)(double beats,int *inst,int dir) = guitar_Fs9_std;

void (*guitar_Fsm6)(double beats,int *inst,int dir) = guitar_Fsm6_std;

void (*guitar_Fsm7)(double beats,int *inst,int dir) = guitar_Fsm7_std;

void (*guitar_FsM7)(double beats,int *inst,int dir) = guitar_GbM7_std;

void (*guitar_Fsdim)(double beats,int *inst,int dir) = guitar_Fsdim_std;

void (*guitar_Fsaug)(double beats,int *inst,int dir) = guitar_Gbaug_std;

void (*guitar_Fssus)(double beats,int *inst,int dir) = guitar_Gbsus_std;

void (*guitar_Gb)(double beats,int *inst,int dir) = guitar_Fs_std;

void (*guitar_Gbm)(double beats,int *inst,int dir) = guitar_Fsm_std;

void (*guitar_Gb6)(double beats,int *inst,int dir) = guitar_Gb6_std;

void (*guitar_Gb7)(double beats,int *inst,int dir) = guitar_Fs7_std;

void (*guitar_Gb9)(double beats,int *inst,int dir) = guitar_Fs9_std;

void (*guitar_Gbm6)(double beats,int *inst,int dir) = guitar_Fsm6_std;

void (*guitar_Gbm7)(double beats,int *inst,int dir) = guitar_Fsm7_std;

void (*guitar_GbM7)(double beats,int *inst,int dir) = guitar_GbM7_std;

void (*guitar_Gbdim)(double beats,int *inst,int dir) = guitar_Fsdim_std;

void (*guitar_Gbaug)(double beats,int *inst,int dir) = guitar_Gbaug_std;

void (*guitar_Gbsus)(double beats,int *inst,int dir) = guitar_Gbsus_std;

void (*guitar_G)(double beats,int *inst,int dir) = guitar_G_std;

void (*guitar_Gm)(double beats,int *inst,int dir) = guitar_Gm_std;

void (*guitar_G6)(double beats,int *inst,int dir) = guitar_G6_std;

void (*guitar_G7)(double beats,int *inst,int dir) = guitar_G7_std;

void (*guitar_G9)(double beats,int *inst,int dir) = guitar_G9_std;

void (*guitar_Gm6)(double beats,int *inst,int dir) = guitar_Gm6_std;

void (*guitar_Gm7)(double beats,int *inst,int dir) = guitar_Gm7_std;

void (*guitar_GM7)(double beats,int *inst,int dir) = guitar_GM7_std;

void (*guitar_Gdim)(double beats,int *inst,int dir) = guitar_Gdim_std;

void (*guitar_Gaug)(double beats,int *inst,int dir) = guitar_Gaug_std;

void (*guitar_Gsus)(double beats,int *inst,int dir) = guitar_Gsus_std;

void (*guitar_Gs)(double beats,int *inst,int dir) = guitar_Ab_std;

void (*guitar_Gsm)(double beats,int *inst,int dir) = guitar_Gsm_std;

void (*guitar_Gs6)(double beats,int *inst,int dir) = guitar_Ab6_std;

void (*guitar_Gs7)(double beats,int *inst,int dir) = guitar_Ab7_std;

void (*guitar_Gs9)(double beats,int *inst,int dir) = guitar_Ab9_std;

void (*guitar_Gsm6)(double beats,int *inst,int dir) = guitar_Gsm6_std;

void (*guitar_Gsm7)(double beats,int *inst,int dir) = guitar_Gsm7_std;

void (*guitar_GsM7)(double beats,int *inst,int dir) = guitar_AbM7_std;

void (*guitar_Gsdim)(double beats,int *inst,int dir) = guitar_Gsdim_std;

void (*guitar_Gsaug)(double beats,int *inst,int dir) = guitar_Abaug_std;

void (*guitar_Gssus)(double beats,int *inst,int dir) = guitar_Absus_std;

/* function playStrings
 *
 * play a set of notes simultaneously on a
 * stringed instrument
 *
 * notes are given as string and fret pairs with
 * a strum direction before the note list
 *
 * strum direction of down will play the notes
 * in the order they are passed to the function
 *
 */

void
playStrings(double beats,int *inst,int dir,int string,int fret,...)
    {
    int strings[512];
    int frets[512];
    va_list ap;
    int i = 0;
    int j = 0;
    strings[i] = string;
    frets[i++] = fret;

    va_start(ap, fret);

    int curArg = va_arg(ap,int);
    while (curArg != IX)
        {
        if (i >= sizeof(strings) / sizeof(int))
            {
            }
        else
            {
            strings[i] = curArg;
            frets[i] = va_arg(ap,int);
            ++i;
            curArg = va_arg(ap,int);
            }
        }
    int note = 0;
    int loc = getLocation();
    if (strideMode == 0)
        {
        if (dir == STRUM_UP)
            {
            for (j = i - 1; j >= 0; --j)
                {
                if (j == i - 1) louder(1.5);
                if (j == i - 2) louder(1.25);
                setLocation(loc);
                forwards(stride*note);
                nplay(beats-(stride*note),inst[strings[j]],frets[j]);
                if (j == i - 1) louder(0);
                if (j == i - 2) louder(0);
                ++note;
                }
            }
        else
            {
            for (j = 0; j < i; ++j)
                {
                if (j == 0) louder(1.5);
                if (j == 1) louder(1.25);
                setLocation(loc);
                forwards(stride*note);
                nplay(beats-(stride*note),inst[strings[j]],frets[j]);
                if (j == 0) louder(0);
                if (j == 1) louder(0);
                ++note;
                }
            }
        }
    else if (strideMode == 1)
        {
        if (dir == STRUM_UP)
            {
            for (j = i - 1; j >= 0; --j)
                {
                if (j == i - 1) louder(1.5);
                if (j == i - 2) louder(1.25);
                setLocation(loc);
                forwards(stride*note);
                nplay(beats,inst[strings[j]],frets[j]);
                if (j == i - 1) louder(0);
                if (j == i - 2) louder(0);
                ++note;
                }
            }
        else
            {
            for (j = 0; j < i; ++j)
                {
                if (j == 0) louder(1.5);
                if (j == 1) louder(1.25);
                setLocation(loc);
                forwards(stride*note);
                nplay(beats,inst[strings[j]],frets[j]);
                if (j == 0) louder(0);
                if (j == 1) louder(0);
                ++note;
                }
            }
        }
    setLocation(loc+beatsToSamples(beats));
    }
