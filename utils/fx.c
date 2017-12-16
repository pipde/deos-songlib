/*
    FX.C - audio effects for AD1848-based PC soundcards.
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <dos.h>
#include <conio.h>
#include "1848.h"

#define BFSZ        4096    /* main delay buffer */
#define MAX_XTAB    512     /* crossfade lookup table size for pitch change */
#define NA          0.0     /* param not applicable */

#ifndef TRUE
#define TRUE        1
#define FALSE       0
#endif

/* macros for pitch_change delay index manipulation */
#define inc_index(x)        x = (x + 1) & (BFSZ - 1)
#define inc_indexes(x1,x2)  { x2 = x1; inc_index(x1); }

/* key eye on human every once in a while in all effect routines */
#define SCAN_HUMAN          0x40000L    /* sets rate of checking for key */
#define check_human()       {                           \
                                if(scan-- == 0)  {      \
                                    chk_keystroke();    \
                                    scan = SCAN_HUMAN;  \
                                }                       \
                            }
                            
/* program description struct */
struct program {
    char *name;         /* ASCII name of program */
    void (*vec)(struct program *p);
    double dry_mix;     /* dry (unaffected) signal mix */
    double wet_mix;     /* wet (affected) signal mix */
    double feedback;    /* feedback */
    double rate;        /* usually rate of sweep */
    double depth;       /* width of sweep */
    double delay;       /* fixed delay factor (base pitch for phaser) */
} program;

/* handy typedefs */
typedef union bw {
    unsigned char b[2];
    int w;
} bw;
typedef union wl {
    unsigned int w[2];
    long l;
} wl;
    
/* protos */
void thru(struct program *p);
void noise_gate(struct program *p);
void flange_chorus(struct program *p);
void pitch_change(struct program *p);
void phase_shift(struct program *p);
void chk_keystroke(void);
void usage(void);
void init_1848(int samprate, int mic, int gain);
void wr_reg(int reg, int val);
int  rd_reg(int reg);

/* table of programs (it's very easy to modify and add programs) */
struct program programs[] = {
    /*  Name/vector      Dry     Wet   Feedback Rate    Depth   Delay */
    { "Echoes",
        flange_chorus,  0.999,  0.999,  0.7,    0.0,    0.0,    250.0},
    { "Slow flange",
        flange_chorus,  0.999,  0.999,  0.0,    2.0,    6.0,    0.0},
    { "Slow invert flange with feedback",
        flange_chorus,  0.999,  -0.999, -0.7,   2.0,    6.0,    0.0},
    { "Slow chorus",
        flange_chorus,  0.999,  0.999,  0.0,    11.0,   20.0,   20.0},
    { "Cheesy \"take me to your leader\" robot voice",
        flange_chorus,  0.0,    0.999,  0.75,   0.0,    0.0,    12.5},
    { "Crazy pitch bend chorus",
        flange_chorus,  0.999,  0.999,  0.3,    150.0,  40.0,   40.0},
    { "Darth",
        pitch_change,   0.0,    0.999,  0.0,    -0.35,  25.0,   0.0},
    { "Major third up",
        pitch_change,   0.999,  0.999,  0.0,    0.2599, 35.0,   0.0},
    { "Octave up",
        pitch_change,   0.999,  0.999,  0.0,    1.0,    40.0,   0.0},
    { "Munchkins on helium",
        pitch_change,   0.0,    0.999,  0.4,    0.3348, 35.0,   0.0},
    { "Descending echoes",
        pitch_change,   0.0,    0.999,  0.5,    -0.2,   35.0,   120.0},
    { "Ascending echoes",
        pitch_change,   0.0,    0.999,  0.4,    0.2599, 35.0,   120.0},
    { "Phase shift",
        phase_shift,    0.999,  0.999,  0.0,    1.0,    4.0,    100.0},
    { "Slow invert phase shift with feedback",
        phase_shift,    0.999,  -0.999, -0.6,   0.2,    6.0,    100.0},
    { "Noise gate",
        noise_gate,     NA,     NA,     NA,     500.0,  0.05,   NA},
    { "Straight Thru",
        thru,           NA,     NA,     NA,     NA,     NA,     NA},
};
#define NPROGS (sizeof(programs) / sizeof(struct program))

/* globals */
unsigned int SampleRate;            /* sample rate set by init_1848 */
double Buf[BFSZ];                   /* buffer used by delay based effects */


/*
                                main
*/
void
main(int argc, char **argv)
{
    int patch,samprate,gain;
    struct program *p;
    
    /* make sure we got reasonable # of command line params */
    if(argc < 2 || argc > 4) usage();
    
    /* get pointer to selected program */
    patch = atoi(argv[1]);
    if(patch < 0 || patch > NPROGS)
        usage();
    p = &programs[patch];
    
    /* see if sample rate specified (default to 16k) */
    if(argc > 2)
        samprate = atoi(argv[2]);
    else
        samprate = 16;

    /* see if source/gain spec'd */
    if(argc > 3)
        gain = atoi(argv[3]);
    else
        gain = 0x17;                /* default: select mic with gain of 15 */
        
    /* init hardware */
    init_1848(samprate,gain & 0x10, gain & 0xf);

    /* call the routine spec'd in the program */
    printf("\nPatch %d: %s\n",patch,p->name);
    (*p->vec)(p);
    /* never returns */
}

/*
                                thru
    
    Simply reads 1848 input and routes directly thru to output.
    Useful for setting gain for minimum noise and distortion.
    All program parms ignored.
*/
void
thru(struct program *p)
{
    bw data;
    long scan = 0;

    data.w = (int)p->dry_mix;               /* quiet the compiler */
        
    /* disable interrupts, go to it */
    disable();
    while(1) {
        while((inp(SR) & 0x20) == 0);       /* wait for input ready */
        data.b[0] = inp(PDR);               /* read input from chip */
        data.b[1] = inp(PDR);
        
        while((inp(SR) & 0x2) == 0);        /* wait for output ready */
        outp(PDR,data.b[0]);                /* write output to chip */
        outp(PDR,data.b[1]);
        
        check_human();                      /* check on human every so often */
    }
}


/*
                                noise_gate

    Super simple noise gate to demonstrate how much of the hiss
    comes directly from the ADC on this card, but how quiet the
    DACs are by comparison.
        
    Only parms are:
        rate        decay time in ms
        depth       threshold for turn on
*/
void
noise_gate(struct program *p)
{
    double inval,decay_fac,gain = 0;
    bw data;
    long scan = 0;

    /* calculate decay factor for 20db atten in spec'd time */
    decay_fac = pow(10.0,1.0 / ((p->rate / 1000.0) * (double)SampleRate));
    decay_fac = 1.0 / decay_fac;
        
    /* disable interrupts, go to it */
    disable();
    while(1) {
        while((inp(SR) & 0x20) == 0);       /* wait for input ready */
        data.b[0] = inp(PDR);               /* read input from chip */
        data.b[1] = inp(PDR);

        inval = (double)data.w;
        if(inval > p->depth)                /* see if we crossed threshold */
            gain = 1.0;                     /* turn gate on */
        data.w = (int)(inval * gain);
        
        while((inp(SR) & 0x2) == 0);        /* wait for output ready */
        outp(PDR,data.b[0]);                /* write output to chip */
        outp(PDR,data.b[1]);
        
        gain *= decay_fac;                  /* adjust attenuation */
        
        check_human();                      /* check on human every so often */
    }
}

        
/*
                                flange_chorus
    
    Does flanging/chorusing family of effects based on a single
    varying delay.
    
    dry_mix     mix of unaffected signal (-0.999 to 0.999)
    wet_mix     mix of affected signal (-0.999 - 0.999)
    feedback    amount of recirculation (-0.9 - 0.9)
    rate        rate of delay change in millisecs per sec
    sweep       sweep range in millisecs
    delay       fixed additional delay in millisecs
*/
void
flange_chorus(struct program *p)
{
    int fp,ep1,ep2;
    int step,depth,delay,min_sweep,max_sweep;
    double inval,outval,ifac = 65536.0;
    long scan = 0;
    bw data;
    wl sweep;

    /* fetch params */
    step = (int)(p->rate * 65.536);
    depth = (int)(p->depth * (double)SampleRate / 1000.0);
    delay = (int)(p->delay * (double)SampleRate / 1000.0);
    
    /* init/calc some stuff */
    max_sweep = BFSZ - 2 - delay;
    min_sweep = max_sweep - depth;
    if(min_sweep < 0) {
        printf("Can't do that much delay or depth at this sample rate.\n");
        exit(1);
    }
    sweep.w[1] = (min_sweep + max_sweep) / 2;
    sweep.w[0] = 0;

    /* init store and read ptrs to known value */
    fp = ep1 = ep2 = 0;
    
    /* disable interrupts, go to it */
    disable();
    while(1) {
        while((inp(SR) & 0x20) == 0);       /* wait for input ready */
        data.b[0] = inp(PDR);               /* read input from chip */
        data.b[1] = inp(PDR);

        /* interpolate from the 2 read values */
        outval =
         (Buf[ep1] * sweep.w[0] + Buf[ep2] * (ifac - sweep.w[0])) / ifac;
        
        /* store finished input plus feedback */
        Buf[fp] = (inval = (double)data.w) + outval * p->feedback;
        
        /* develop final output mix */
        outval = outval * p->wet_mix + inval * p->dry_mix;
        if(outval > 32767.0)
            data.w = 32767;
        else if(outval < -32768.0)
            data.w = -32768;
        else
            data.w = (int)outval;
            
        while((inp(SR) & 0x2) == 0);        /* wait for output ready */
        outp(PDR,data.b[0]);                /* write output to chip */
        outp(PDR,data.b[1]);
        
        /* update ptrs */
        fp = (fp + 1) & (BFSZ - 1);
        sweep.l += step;
        ep1 = (fp + sweep.w[1]) & (BFSZ - 1);
        ep2 = (ep1 - 1) & (BFSZ - 1);

        /* check for sweep reversal */      
        if(sweep.w[1] > max_sweep)          /* see if we hit top of sweep */
            step = -step;                   /* reverse */
        else if(sweep.w[1] < min_sweep)     /* or if we hit bottom of sweep */
            step = -step;                   /* reverse */
            
        check_human();                      /* check on human every so often */
    }
}

/*
                                pitch_change
    
    dry_mix     mix of unaffected signal (-0.999 to 0.999)
    wet_mix     mix of affected signal (-0.999 - 0.999)
    feedback    amount of recirculation (-0.9 - 0.9)
    rate        amount of pitch change (see table below for values)
    depth       sweep range in millisecs for generating pitch shift
    delay       fixed additional delay

    Semitones      Up              Down
        1       0.059463        -0.056126
        2       0.122462        -0.109101
        3       0.189207        -0.159104
        4       0.259921        -0.206299
        5       0.334840        -0.250846
        6       0.414214        -0.292893
        7       0.498307        -0.332580
        8       0.587401        -0.370039
        9       0.681793        -0.405396
        10      0.781797        -0.438769
        11      0.887749        -0.470268
        12      1.000000        -0.500000
*/
void
pitch_change(register struct program *p)
{
    int fp,ep1,ep2,ep3,ep4;
    int depth,delay,min_sweep,max_sweep,sweep_up;
    int i,step,xfade,xfade_cnt,active,active_cnt,chanA;
    long scan = 0;
    double inval,outval,comp,ifac = 65536.0;
    double blendA,blendB,*fadeA,*fadeB;
    static double fade_out[MAX_XTAB],fade_in[MAX_XTAB];
    bw data;
    wl sweep;

    /* fetch params */
    step = (int)(p->rate * 65535.0);
    sweep_up = p->rate > 0;
    depth = (int)(p->depth * (double)SampleRate / 1000.0);
    delay = (int)(p->delay * (double)SampleRate / 1000.0);
    xfade = (int)(12.0 * (double)SampleRate / 1000.0);
    
    /* init/calc some stuff */
    max_sweep = BFSZ - 2 - delay;
    min_sweep = max_sweep - depth;
    active = max_sweep - min_sweep - (int)(xfade * p->rate) - 2;
    if(xfade > MAX_XTAB) {
        printf("Can't do pitch change crossfade at this sample rate.\n");
        exit(1);
    }
    if(min_sweep < 0) {
        printf("Can't do that much delay or depth at this sample rate.\n");
        exit(1);
    }
            
    /* build the crossfade lookup tables */
    for(i = 0; i < xfade; i++) {
        fade_in[i] = cos((double)i * M_PI_2 / (double)xfade);
        fade_out[i] = sin((double)i * M_PI_2 / (double)xfade);
    }
    
    /* init store and read ptrs to known value, chanA active 1st */
    fp = ep3 = ep4 = xfade_cnt = 0;
    sweep.l = 0;
    if(sweep_up)
        ep1 = ep2 = min_sweep;
    else
        ep1 = ep2 = max_sweep;
    active_cnt = active;
    blendA = 1.0;
    blendB = 0.0;
    fadeA = fade_out;
    fadeB = fade_in;
    chanA = TRUE;
            
    /* disable interrupts, go to it */
    disable();
    while(1) {
        while((inp(SR) & 0x20) == 0);       /* wait for input ready */
        data.b[0] = inp(PDR);               /* read input from chip */
        data.b[1] = inp(PDR);

        /* messy expression to interpolate from both pairs of read ptrs */
        comp = ifac - sweep.w[0];
        outval =
         ((Buf[ep1] * sweep.w[0] + Buf[ep2] * comp) * blendA +
          (Buf[ep3] * sweep.w[0] + Buf[ep4] * comp) * blendB) / ifac;
        
        /* store finished input plus feedback */
        Buf[fp] = (inval = (double)data.w) + outval * p->feedback;
        
        /* develop final output mix */
        outval = outval * p->wet_mix + inval * p->dry_mix;
        if(outval > 32767.0)                /* clip output if necessary */
            data.w = 32767;
        else if(outval < -32768.0)
            data.w = -32768;
        else
            data.w = (int)outval;
                        
        while((inp(SR) & 0x2) == 0);        /* wait for output ready */
        outp(PDR,data.b[0]);                /* write output to chip */
        outp(PDR,data.b[1]);
        
        check_human();                      /* check on human every so often */

        /* see if crossfade active */
        if(xfade_cnt) {
            xfade_cnt--;
            blendA = fadeA[xfade_cnt];
            blendB = fadeB[xfade_cnt];
        }
        
        /* update store ptr */
        inc_index(fp);
        
        /* see which direction */
        if(sweep_up) {
            /* update sweep */
            sweep.l += (unsigned)step;
            
            /* always inc at least once */
            inc_indexes(ep1,ep2);
            inc_indexes(ep3,ep4);
            
            /* if sweep didn't overflow, we're done */
            if(sweep.w[1] == 0) continue;
            
            /* sweep overflowed, inc again */
            inc_indexes(ep1,ep2);
            inc_indexes(ep3,ep4);
            sweep.w[1] = 0;
            
            /* see if it's time to switch over to other delay channel */
            if(active_cnt-- == 0) {
                xfade_cnt = xfade;      /* initiate crossfade */
                active_cnt = active;    /* start counter on new channel */
                if(chanA) {             /* A has been active, go to B */
                    chanA = FALSE;
                    ep3 = (fp + min_sweep) & (BFSZ - 1);
                    fadeA = fade_out;
                    fadeB = fade_in;
                }
                else {
                    chanA = TRUE;
                    ep1 = (fp + min_sweep) & (BFSZ - 1);
                    fadeB = fade_out;
                    fadeA = fade_in;
                }
            }
        }
        /* do downward sweep */
        else {
            sweep.l += step;            /* update sweep */
            
            /* if sweep didn't overflow, inc ptrs, that's all */
            if(sweep.w[1] == 0) {
                inc_indexes(ep1,ep2);
                inc_indexes(ep3,ep4);
                continue;
            }
            /* sweep overflowed, check on stuff but skip ptr inc */         
            sweep.w[1] = 0;
            
            /* see if it's time to switch over to other delay channel */
            if(active_cnt-- == 0) {
                xfade_cnt = xfade;
                active_cnt = active;
                if(chanA) {             /* A has been active, go to B */
                    chanA = FALSE;
                    ep3 = (fp + max_sweep) & (BFSZ - 1);
                    fadeA = fade_out;
                    fadeB = fade_in;
                }
                else {
                    chanA = TRUE;
                    ep1 = (fp + max_sweep) & (BFSZ - 1);
                    fadeB = fade_out;
                    fadeA = fade_in;
                }
            }
        } /* end down sweep */
    } /* end main loop */
}

/*
                                phase_shift

    Digital version of the popular '70s effect.  This one
    does 4 stages just like old MXR Phase 90 stompbox.
    
    dry_mix     mix of unaffected signal (-0.999 to 0.999)
    wet_mix     mix of affected signal (-0.999 - 0.999)
    feedback    amount of recirculation (-0.9 - 0.9)
    rate        rate of sweep in cycles per second
    depth       sweep range in octaves
    delay       base frequency of sweep
*/
void
phase_shift(register struct program *p)
{
    long scan = 0;
    double wp,min_wp,max_wp,range,coef,sweepfac;
    double inval,x1,outval = 0.0;
    static double lx1,ly1,lx2,ly2,lx3,ly3,lx4,ly4;
    bw data;

    /* calc params for sweeping filters */  
    wp = min_wp = (M_PI * p->delay) / (double)SampleRate;
    range = pow(2.0,p->depth);
    max_wp = (M_PI * p->delay * range) / (double)SampleRate;
    p->rate = pow(range,p->rate / ((double)SampleRate / 2));
    sweepfac = p->rate;
    
    /* disable interrupts, go to it */
    disable();
    while(1) {
        coef = (1.0 - wp) / (1.0 + wp);     /* calc coef for current freq */
        
        while((inp(SR) & 0x20) == 0);       /* wait for input ready */
        data.b[0] = inp(PDR);               /* read input from chip */
        data.b[1] = inp(PDR);

        x1 = (inval = (double)data.w) + p->feedback * ly4;
        ly1 = coef * (ly1 + x1) - lx1;      /* do 1st filter */
        lx1 = x1;
        ly2 = coef * (ly2 + ly1) - lx2;     /* do 2nd filter */
        lx2 = ly1;
        ly3 = coef * (ly3 + ly2) - lx3;     /* do 3rd filter */
        lx3 = ly2;
        ly4 = coef * (ly4 + ly3) - lx4;     /* do 4th filter */
        lx4 = ly3;
        
        /* develop final output mix */
        outval = ly4 * p->wet_mix + inval * p->dry_mix;
        if(outval > 32767.0)                /* clip output if necessary */
            data.w = 32767;
        else if(outval < -32768.0)
            data.w = -32768;
        else
            data.w = (int)outval;
        
        while((inp(SR) & 0x2) == 0);        /* wait for output ready */
        outp(PDR,data.b[0]);                /* write output to chip */
        outp(PDR,data.b[1]);

        wp *= sweepfac;                     /* adjust freq of filters */
        if(wp > max_wp)                     /* max? */
            sweepfac = 1.0 / p->rate;       /* sweep back down */
        else if(wp < min_wp)                /* min? */
            sweepfac = p->rate;             /* sweep back up */

        check_human();                      /* check on human every so often */
    }
}


/*
                                chk_keystroke
    
    Sees if human has hit a key.  If so, exits.  This routine makes
    a click cuz it has to turn on interrupts briefly.
*/
void
chk_keystroke(void)
{
    enable();
    if(kbhit()) {
        getch();
        exit(0);
    }
    disable();
}

/*
                                usage
*/
void
usage(void)
{
    int i;
    static char *use = 
    "Usage: fx [patch] [[samp_rate]] [[src_gain]]\n"
    " patch       Selects effect program.  Value should be 0-%d.\n"
    " samp_rate   Selects sampling frequency.  Value should be 5,6,8,\n"
    "             9,11,16,18,22,27,32,33,37,44, or 48. Defaults to 16k.\n"
    " src_gain    Selects source and gain.  0-15 adjust line input level.\n"
    "             16-31 select the microphone input and adjust its level.\n"
    "             Defaults to 23 for microphone input.\n";
    
    printf(use,NPROGS - 1);
    for(i = 0; i < NPROGS; i++)
        printf("Patch %2d:\t%s\n",i,programs[i].name);
    exit(1);
}


int iar_mce;
        
/*
                                init_1848
    
    Sets up for programmed I/O at spec'd sample rate.  Initiates
    simultaneous capture and play mode.
*/
void
init_1848(int samprate,int mic,int gain)
{   
    int i,version;
    struct sr {
        int handle;             /* handy value for humans */
        unsigned int actual;    /* actual sample rate */
        int code;               /* code for chip */
    };
    struct sr *srp;
    struct sr sr_tab[NUM_SRATES] = {
        {5,5152,S5152},     {6,6615,S6615},     {8,8000,S8000},
        {9,9600,S9600},     {11,11025,S11025},  {16,16000,S16000},
        {18,18900,S18900},  {22,22050,S22050},  {27,27428,S27428},
        {32,32000L,S32000}, {33,33075L,S33075}, {37,37800L,S37800},
        {44,44100L,S44100}, {48,48000L,S48000}};

    /* look up code for spec'd sample rate */   
    for(i = 0, srp = sr_tab; i < NUM_SRATES; i++, srp++) {
        if(srp->handle == samprate || srp->actual == samprate)
            break;
    }
    if(i == NUM_SRATES) usage();
                
    delay(0);                           /* calibrate delay routine */
    
    version = rd_reg(MISCINFO);         /* see what kind of chip we have */
    if(version == 0xff) {
        printf("Can't find 1848 chip.\n");
        exit(1);
    }
    printf("Found AD1848%c CODEC\n",(version & 0xf) + 'A'); 
    /* NOTE: the rev K chip is reportedly much quieter and worth getting */
    
    /* prepare to change/init chip setup */
    wr_reg(LOUTCTL,0x80);               /* mute left DAC */
    wr_reg(ROUTCTL,0x80);               /* mute right DAC */
    wr_reg(DIGMIX,0);                   /* mute digital mix */
    wr_reg(LAUX1CTL,0x80);              /* mute left aux1 in */
    wr_reg(RAUX1CTL,0x80);              /* mute right aux1 in */
    wr_reg(LAUX2CTL,0x80);              /* mute left aux2 in */
    wr_reg(RAUX2CTL,0x80);              /* mute right aux2 in */
    
    /* change chip config */
    outp(IAR,iar_mce = 0x40);           /* allow mode changes */
    wr_reg(INTCFG,0xc8);                /* set auto calibrate */
    wr_reg(CLKDATFMT,0x40 | srp->code);
    
    while(inp(IAR) & 0x80)              /* wait for chip to come out of init */
        ;
    
    outp(IAR,iar_mce = 0x00);           /* clear mode change enable */
    delay(2000);                        /* wait for auto calib to kick in */
    
    while(rd_reg(TSTINIT) & 0x20)       /* wait for auto calibration */
        ;

    wr_reg(LOUTCTL,0);                  /* unmute the DACs */
    wr_reg(ROUTCTL,0);
    i = (mic ? 0xa0 : 0x40) | (gain & 0xf);/* decide gain and signal source */
    wr_reg(LINCTL,i);                   /* unmute the inputs */
    wr_reg(RINCTL,i);
    wr_reg(INTCFG,0xc3);                /* kick off capture and play */
    
    printf("Sample rate initialized to %u.\n",srp->actual);
    SampleRate = srp->actual;
}

/*
                                wr_reg
*/
void
wr_reg(int reg, int val)
{
    outp(IAR,iar_mce | reg);
    outp(IDR,val);
}

/*
                                rd_reg
*/
int
rd_reg(int reg)
{
    outp(IAR,iar_mce | reg);
    return(inp(IDR));
}

while (--sampleFrames >= 0)
    {
    n = n++; // initialized with n = 0
    deltai = delayMin + (delayMax-delayMin)/2 *(1+ sin(2 * pi * freq * 1/44100 * n)); // sweep 0.5 – 2 ms

    // Delay line using Lagrange interpolator
    di = floorf(deltai) - 1;
    t0 = deltai - di;
    t1 = t0 - 1;
    t2 = t0 - 2;
    t3 = t0 - 3;
    b0 = -t1*t2*t3/6;
    b1 =  t0*t2*t3/2;
    b2 = -t0*t1*t3/2;
    b3 =  t0*t1*t2/6;

    //Lelf channel only here
    if (inPointL == delayBufferSize)
    inPointL = 0;
    delay_bufferL[inPointL++] = gain*(*in1) +  room * feedbackL;
    for (i=0;i<4;i++){
    outPointL[i] = (inPointL - di + i);
    if (outPointL[i] < 0)
    outPointL[i] += delayBufferSize;
    listateL[i] = delay_bufferL[outPointL[i]];
    }
    feedbackL = (b0*listateL[0] + b1*listateL[1] + b2*listateL[2] + b3*listateL[3]);
    feedbackLOut = lowShelf(feedbackL); // used lowsheld filter within the loop 
    }


//-------------------------------------------------------------------------------------------------------
// Chorus VST plug in
// Jan 6, 2006, Kevin Kuang
// http://ccrma.stanford.edu/~kuangzn
//-------------------------------------------------------------------------------------------------------

#ifndef __chorus_H
#include "chorus.hpp"
#endif

#include <stdio.h>
#include <math.h>


//-------------------------------------------------------------------------------------------------------
chorus::chorus (audioMasterCallback audioMaster)
    : AudioEffectX (audioMaster, 1, 5)  // 1 program, 3 parameters
{
    fGain       = 0;                    // default input gain = 0 dB
    fDelayMin   = 0.005;                // default Delay time = 0
    fDelayMax   = 0.100;                // default Delay time = 0
    fWetDry     = 0.5;                  // default wet/dry factor = 50%
    sampleCount = 0;                    // reset sample counter
    n = 0;

    inPointL=0; 
    inPointR=0; 
    delayBufferSize=44100;              //maximum delay 44100 samples

    setNumInputs (2);                   // stereo in
    setNumOutputs (2);                  // stereo out
    setUniqueID ('Deli');               // identify
    canMono ();                         // makes sense to feed both inputs with the same signal
    canProcessReplacing ();             // supports both accumulating and replacing output
    strcpy (programName, "Default");    // default program name
    
    resume();

}


//-------------------------------------------------------------------------------------------------------
chorus::~chorus ()
{
    // nothing to do here
}

//-------------------------------------------------------------------------------------------------------
void chorus::setProgramName (char *name)
{
    strcpy (programName, name);
}

//-----------------------------------------------------------------------------------------
void chorus::getProgramName (char *name)
{
    strcpy (name, programName);
}

//------------------------------------------------------------------------
bool chorus::getEffectName (char* name)
{
    strcpy (name, "chorus");
    return true;
}

//------------------------------------------------------------------------
bool chorus::getProductString (char* text)
{
    strcpy (text, "chorus");
    return true;
}

//------------------------------------------------------------------------
bool chorus::getVendorString (char* text)
{
    strcpy (text, "kk Productions");
    return true;
}

//-----------------------------------------------------------------------------------------
void chorus::setParameter (long index, float value)
{
    switch (index)
    {
    case kGain:
        fGain = value;
        gain = pow(10.0,(60.0*fGain-30.0)/20.0);
        break;
    case kWetDry:
        fWetDry = value;
        wetdry = fWetDry;
        break;
    case kFreq:
        fFreq = value;
        freq = fFreq * 10;
        break;
    case kDelayMin:
        fDelayMin = value;
        delayMin = fDelayMin * 44100; // delayMin in samples, fDelayMin in ms
        break;
    case kDelayMax:
        fDelayMax = value;
        delayMax = fDelayMax * 44100;
        break;
    }

}

//-----------------------------------------------------------------------------------------
float chorus::getParameter (long index)
{

    switch (index)
    {
        case kGain:
            return fGain;
            break;
        case kDelayMin:
            return fDelayMin;
            break;
        case kDelayMax:
            return fDelayMax;
            break;
        case kWetDry:
            return fWetDry;
            break;
        case kFreq:
            return fFreq;
            break;
    }

}

//-----------------------------------------------------------------------------------------
void chorus::getParameterName (long index, char *label)
{
    switch (index)
    {
    case kGain:
        strcpy(label, "Input Gain");
        break;
    case kDelayMin:
        strcpy(label, "Delay Min");
        break;
    case kDelayMax:
        strcpy(label, "Delay Max");
        break;
    case kWetDry:
        strcpy(label, "Wet/Dry");
        break;
    case kFreq:
        strcpy(label, "Dealy Freq");
        break;
    };
}

//-----------------------------------------------------------------------------------------
void chorus::getParameterDisplay (long index, char *text)
{

    switch (index)
    {
    case kGain:
        float2string(20.0*log10(gain), text);
        break;
    case kDelayMin:
        ms2string(delayMin, text);
        break;
    case kDelayMax:
        ms2string(delayMax, text);
        break;
    case kWetDry:
        long2string(wetdry*100, text);
        break;
    case kFreq:
        long2string(freq*10, text);
        break;
    };
}

//-----------------------------------------------------------------------------------------
void chorus::getParameterLabel(long index, char *label)
{
switch (index)
    {
    case kGain :
        strcpy(label, "dB");
        break;
    case kDelayMin :
        strcpy(label, "ms");
        break;
    case kDelayMax :
        strcpy(label, "ms");
        break;
    case kWetDry :
        strcpy(label, "%");
        break;
    case kFreq :
        strcpy(label, "Hz");
        break;
    };
}

void chorus::clear()
{
    for ( int i=0; i<44100; i++){
    delay_bufferL[i]=0, delay_bufferR[i]=0, listateL[i]=0, listateR[i]=0;}
    AudioEffectX::resume ();
}


//-----------------------------------------------------------------------------------------
void chorus::process (float **inputs, float **outputs, long sampleFrames)
{
}

//-----------------------------------------------------------------------------------------
void chorus::processReplacing (float **inputs, float **outputs, long sampleFrames)
{
    float *in1  =  inputs[0];
    float *in2  =  inputs[1];
    float *out1 =  outputs[0];
    float *out2 =  outputs[1];
    float T = 1/44100;
    int di=0;
    double pi = 3.1415926;
    double nT=0, deltai=0, t0=0, t1=0, t2=0, t3=0, b0=0, b1=0, b2=0, b3=0;
    
    while (--sampleFrames >= 0)
    {
    nT = 1/44100 * (n++);
    deltai = (delayMax-delayMin)/2 + delayMin + (delayMax-delayMin)/2 * sin(2 * pi * freq * nT);
    // Delay line using Lagrange interpolator
    di = floorf(deltai) - 1;
    t0 = deltai - di;
    t1 = t0 - 1;
    t2 = t0 - 2;
    t3 = t0 - 3;
    b0 = -t1*t2*t3/6;
    b1 =  t0*t2*t3/2;
    b2 = -t0*t1*t3/2;
    b3 =  t0*t1*t2/6;


        //Lelf channel
        if (inPointL == delayBufferSize)
            inPointL = 0;
        
        delay_bufferL[inPointL++] = gain*(*in1);
        for (i=0;i<4;i++)
        {
            outPointL[i] = (inPointL - di + i);
        if (outPointL[i] < 0)
            outPointL[i] += delayBufferSize;
            listateL[i] = delay_bufferL[outPointL[i]];
        }
        feedbackL = (b0*listateL[0] + b1*listateL[1] + b2*listateL[2] + b3*listateL[3]);
        
        //Right channel
        if (inPointR == delayBufferSize)
            inPointR = 0;

        delay_bufferR[inPointR++] = gain*(*in2);
        for (j=0;j<4;j++)
        {
            outPointR[j] = (inPointR - di + j);
            if (outPointR[j] < 0)
                outPointR[j] += delayBufferSize;
            listateR[j] = delay_bufferR[outPointR[j]];
        }
        feedbackR = (b0*listateR[0] + b1*listateR[1] + b2*listateR[2] + b3*listateR[3]);

        // output
        (*out1++) = (*in1++)*wetdry + (1-wetdry) * feedbackL;
        (*out2++) = (*in2++)*wetdry + (1-wetdry) * feedbackR;
        
   }
}

