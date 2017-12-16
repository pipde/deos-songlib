#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include "songlib.h"
int drumset= 0;

#define NUMDRUMS 6
#define dir "/usr/local/share/samples/"

extern int style;
extern double swingBeat;
extern int swingDown;

static int kick = -1;
static int snare = -1;
static int hhopen = -1;
static int hhclosed = -1;
static int hhpedal = -1;
static int ride = -1;
static int tomHi = -1;
static int tom = -1;
static int tomLo = -1;
static int crash = -1;
static int splash = -1;

static int rim = -1;
static int stick = -1;
static int cowbell = -1;

static double rollSpeed = 0;
static int rollEmphasis = 0;

static void drumKit(int,int,double,char **,int);

void
loadDrumKit()
    {
    crash = readScale(dir,"/drums-crash/quartertone_");
    splash = readScale(dir,"/drums-splash/quartertone_");
    snare = readScale(dir,"/drums-snare/osd_");
    hhopen = readScale(dir,"/drums-hihat/osd-open_");
    hhclosed = readScale(dir,"/drums-hihat/osd-closed_");
    hhpedal = readScale(dir,"/drums-hihat/osd-pedal_");
    ride = readScale(dir,"/drums-ride/quartertone_");
    ride = readScale(dir,"/drums-splash/quartertone_");
    kick = readScale(dir,"/drums-kick/osd_");
    tomHi = readScale(dir,"/drums-tom/osd-hi_");
    tom = readScale(dir,"/drums-tom/osd_");
    tomLo = readScale(dir,"/drums-tom/osd-lo_");
    rim = readScale(dir,"/drums-snare/rim_");
    stick = readScale(dir,"/drums-stick/soundhead_");
    cowbell = readScale(dir,"/drums-cowbell/plunk_");
    }

/* function drum
 *
 * play an absolute note, varying the amplitude, pitch, and timing,
 * slightly
 *
 */

void
drum(double beats,int inst,int pitch)
    {
    int start;
    double amp;
    RRA *note;
    RRA *adjustedNote;

    double tdelt = randomRange(0,slopTiming);
    double pdelt = randomRange(0,slopPitch);
    double adelt = randomRange(0,2*slopAmplitude) - slopAmplitude;

    // if style is SWING,
    //swing beats are pulled longer and shorter, alternately

    start = getLocation();
    amp = getAmplitude();

    if (style == SWING && swingBeat == beats)
        {
        if (swingDown)
            {
            beats = beats * 4 / 3;
            }
        else
            {
            beats = beats * 2 / 3;
            setAmplitude(amp * 0.5);
            }
        swingDown = !swingDown;
        }

    /* don't back up past the beginning */

    if (start + tdelt < 0) tdelt = -start;

    setLocation(start + beatsToSamples(tdelt));

    setAmplitude(amp * (1 + adelt));

    note = getNumberedNote(inst,pitch);
    adjustedNote = resample(note,pow(SEMITONE,pdelt));
    //adjustedNote = resample(note,1.0);
    //printf("resample factor is %f\n",pow(SEMITONE,pdelt));
    rplay(beats,adjustedNote);
    freeRRA(adjustedNote,0);

    /* restore settings */

    setLocation(start + beatsToSamples(beats));
    setAmplitude(amp);
    }

int
getDrumNote()
    {
    int n = -1;
    while (n < 0 || n > 120)
        n = randomInteger(0,11) * 12;
    return n;
    }

double
setRollSpeed(double x)
    {
    double old = rollSpeed;
    rollSpeed = x;
    return old;
    }

double
getRollSpeed(void)
    {
    return rollSpeed;
    }

int
setRollEmphasis(int x)
    {
    int old = rollEmphasis;
    rollEmphasis = x;
    return old;
    }

int
getRollEmphasis(void)
    {
    return rollEmphasis;
    }

void
roll(double beats,int inst)
    {
    int i;
    double oldAttack = getSkipAttack();
    double oldSustain = getSustain();
    double oldAmplitude = getAmplitude();
    double loc = getLocation();

    double speed; 
    int emphasis;

    double oldT = setSlopTiming(0.01);
    double oldA = setSlopAmplitude(0.20);
    double oldP = setSlopPitch(0.8);

    speed = rollSpeed == 0.0? T : rollSpeed;
    emphasis = rollEmphasis == 0? 2 : rollEmphasis;

    printf("speed is %f\n",speed);

    setSustain(0.99999);
    drum(speed,inst,getDrumNote());
    setSkipAttack(oldAttack * 1.5);
    drum(speed,inst,getDrumNote());
    setSkipAttack(oldAttack * 2);
    drum(speed,inst,getDrumNote());
    setSkipAttack(oldAttack * 2.5);
    printf("initial beats done\n");
    i = 3;
    while (speed * i < beats)
        {
        if (i % emphasis == 0) louder(1.2);
        drum(speed,inst,getDrumNote());
        if (i % emphasis == 0) softer(1.22);
        ++i;
        }
    louder(2);

    setLocation(loc + beatsToSamples(beats));
    setSlopTiming(oldT);
    setSlopAmplitude(oldA);
    setSlopPitch(oldP);
    setSustain(oldSustain);
    setSkipAttack(oldAttack);
    setAmplitude(oldAmplitude);
    printf("skipAttack is now %f\n",getSkipAttack());
    }

void
flam(double duration,int instrument,int note,int emphasis)
    {
    double oldT = setSlopTiming(0.0);

    softer(5);
    drum(T / 2,instrument,note);
    louder(5);
    louder(emphasis);
    drum(duration-T / 2,instrument,note);
    softer(emphasis);

    setSlopTiming(oldT);
    }

void
drag(double duration,int instrument,int note,int emphasis)
    {
    double oldT = setSlopTiming(0.0);

    softer(5);
    drum(T / 2,instrument,note);
    drum(T / 2,instrument,note);
    louder(5);
    louder(emphasis);
    drum(duration-T,instrument,note);
    softer(emphasis);

    setSlopTiming(oldT);
    }

int
setSnare(int inst)
    {
    int temp = snare;
    snare = inst;
    return temp;
    }

int
setTomHi(int inst)
    {
    int temp = tomHi;
    tomHi = inst;
    return temp;
    }

int
setTom(int inst)
    {
    int temp = tom;
    tom = inst;
    return temp;
    }

int
setTomLo(int inst)
    {
    int temp = tomLo;
    tomLo = inst;
    return temp;
    }

int
setRim(int inst)
    {
    int temp = rim;
    rim = inst;
    return temp;
    }

int
setStick(int inst)
    {
    int temp = stick;
    stick = inst;
    return temp;
    }

int
setCowbell(int inst)
    {
    int temp = cowbell;
    cowbell = inst;
    return temp;
    }

int
setHHOpen(int inst)
    {
    int temp = hhopen;
    hhopen = inst;
    return temp;
    }

int
setHHClosed(int inst)
    {
    int temp = hhclosed;
    hhclosed = inst;
    return temp;
    }

int
setHHPedal(int inst)
    {
    int temp = hhpedal;
    hhpedal = inst;
    return temp;
    }

int
setCrash(int inst)
    {
    int temp = crash;
    crash = inst;
    return temp;
    }

int
setRide(int inst)
    {
    int temp = ride;
    ride = inst;
    return temp;
    }

int
setSplash(int inst)
    {
    int temp = splash;
    splash = inst;
    return temp;
    }

int
setKick(int inst)
    {
    int temp = kick;
    kick = inst;
    return temp;
    }

int
getSnare()
    {
    if (snare == -1) loadDrumKit();
    return snare;
    }

int
getTomHi()
    {
    if (tomHi == -1) loadDrumKit();
    return tomHi;
    }

int
getTom()
    {
    if (tom == -1) loadDrumKit();
    return tom;
    }

int
getTomLo()
    {
    if (tomLo == -1) loadDrumKit();
    return tomLo;
    }

int
getRim()
    {
    if (rim == -1) loadDrumKit();
    return rim;
    }

int
getStick()
    {
    if (stick == -1) loadDrumKit();
    return stick;
    }

int
getCowbell()
    {
    if (cowbell == -1) loadDrumKit();
    return cowbell;
    }

int
getHHOpen()
    {
    if (hhopen == -1) loadDrumKit();
    return hhopen;
    }

int
getHHClosed()
    {
    if (hhclosed == -1) loadDrumKit();
    return hhclosed;
    }

int
getHHPedal()
    {
    if (hhpedal == -1) loadDrumKit();
    return hhpedal;
    }

int
getCrash()
    {
    if (crash == -1) loadDrumKit();
    return crash;
    }

int
getRide()
    {
    if (ride == -1) loadDrumKit();
    return ride;
    }

int
getSplash()
    {
    if (splash == -1) loadDrumKit();
    return splash;
    }

int
getKick()
    {
    if (kick == -1) loadDrumKit();
    return kick;
    }

#define readPatterns() \
    { \
    va_list ap; \
    patterns[0] = pat; \
    printf("pattern is %s\n",pat); \
    va_start(ap, pat); \
    pat = va_arg(ap,char *); \
    count = 1; \
    while (pat != 0) \
        { \
        if (count > sizeof(patterns)/sizeof(char *)) \
            Fatal("drumKit: too many patterns (missing null terminator?)\n"); \
        patterns[count++] = pat; \
        pat = va_arg(ap,char *); \
        } \
    }

void
drumPat(int instrument,int repeats,double amp,char *pat, ...)
    {
    int count;
    char *patterns[32];

    // macro
    readPatterns();

    drumKit(instrument,repeats,amp,patterns,count);
    }

void
drumkitCrash(int repeats,double amp,char *pat, ...)
    {
    int count;
    char *patterns[32];

    if (crash == -1) loadDrumKit();

    readPatterns();

    drumKit(crash,repeats,amp,patterns,count);
    }

void
drumkitCowbell(int repeats,double amp,char *pat, ...)
    {
    int count;
    char *patterns[32];

    if (crash == -1) loadDrumKit();

    readPatterns();

    drumKit(cowbell,repeats,amp,patterns,count);
    }

void
drumkitRide(int repeats,double amp,char *pat, ...)
    {
    int count;
    char *patterns[32];

    if (ride == -1) loadDrumKit();

    readPatterns();

    drumKit(ride,repeats,amp,patterns,count);
    }

void
drumkitSplash(int repeats,double amp,char *pat, ...)
    {
    int count;
    char *patterns[32];

    if (ride == -1) loadDrumKit();

    readPatterns();

    drumKit(splash,repeats,amp,patterns,count);
    }

void
drumkitSnare(int repeats,double amp,char *pat, ...)
    {
    int count;
    char *patterns[32];

    if (snare == -1) loadDrumKit();

    readPatterns();

    drumKit(snare,repeats,amp,patterns,count);
    }

void
drumkitHHOpen(int repeats,double amp,char *pat, ...)
    {
    int count;
    char *patterns[32];

    if (hhopen == -1) loadDrumKit();

    readPatterns();

    drumKit(hhopen,repeats,amp,patterns,count);
    }

void
drumkitHHClosed(int repeats,double amp,char *pat, ...)
    {
    int count;
    char *patterns[32];

    if (hhclosed == -1) loadDrumKit();

    readPatterns();

    drumKit(hhclosed,repeats,amp,patterns,count);
    }

void
drumkitHHPedal(int repeats,double amp,char *pat, ...)
    {
    int count;
    char *patterns[32];

    if (hhpedal == -1) loadDrumKit();

    readPatterns();

    drumKit(hhpedal,repeats,amp,patterns,count);
    }

void
drumkitKick(int repeats,double amp,char *pat, ...)
    {
    int count;
    char *patterns[32];

    if (kick == -1) loadDrumKit();

    readPatterns();

    printf("amp is %f\n",amp);
    drumKit(kick,repeats,amp,patterns,count);
    }

void
drumkitTomHi(int repeats,double amp,char *pat, ...)
    {
    int count;
    char *patterns[32];

    if (tomHi == -1) loadDrumKit();

    readPatterns();

    drumKit(tomHi,repeats,amp,patterns,count);
    }

void
drumkitTom(int repeats,double amp,char *pat, ...)
    {
    int count;
    char *patterns[32];

    if (tom == -1) loadDrumKit();

    readPatterns();

    drumKit(tom,repeats,amp,patterns,count);
    }

void
drumkitTomLo(int repeats,double amp,char *pat, ...)
    {
    int count;
    char *patterns[32];

    if (tomLo == -1) loadDrumKit();

    readPatterns();

    drumKit(tomLo,repeats,amp,patterns,count);
    }

void
drumkitRim(int repeats,double amp,char *pat, ...)
    {
    int count;
    char *patterns[32];

    if (rim == -1) loadDrumKit();

    readPatterns();

    drumKit(rim,repeats,amp,patterns,count);
    }

void
drumkitStick(int repeats,double amp,char *pat, ...)
    {
    int count;
    char *patterns[32];

    if (stick == -1) loadDrumKit();

    readPatterns();

    drumKit(stick,repeats,amp,patterns,count);
    }

void
drumKit(int instrument,int repeats,double amp,char **patterns,int count)
    {
    int i,j,k;
    double duration;

    for (i = 0; i < repeats; ++i)
        {
        for (j = 0; j < count; ++j)
            {
            char *p = patterns[j];
            int length = strlen(p);
            duration = W / length;
            //printf("length is %d, duration is %f\n",length,duration);

            for (k = 0; k < length; ++k)
                if (strchr("oO^AvVxXrRbBgGfFdD",p[k]) != 0)
                    {
                    int n = getDrumNote();
                    louder(amp);
                    if (p[k] == 'd')
                        drag(duration,instrument,n,1);
                    else if (p[k] == 'D')
                        drag(duration,instrument,n,2);
                    else if (p[k] == 'f')
                        flam(duration,instrument,n,1);
                    else if (p[k] == 'F')
                        flam(duration,instrument,n,2);
                    else if (p[k] == 'g')
                        {
                        softer(4);
                        drum(duration,instrument,n);
                        louder(4);
                        }
                    else if (p[k] == 'G')
                        {
                        softer(6);
                        drum(duration,instrument,n);
                        louder(6);
                        }
                    else if (p[k] == 'R')
                        {
                        louder(2);
                        roll(duration,instrument);
                        softer(2);
                        }
                    else if (p[k] == 'r')
                        roll(duration,instrument);
                    else if (p[k] == 'B')
                        {
                        setRollSpeed(T/16);
                        roll(duration,instrument);
                        setRollSpeed(0);
                        }
                    else if (p[k] == 'b')
                        {
                        softer(2);
                        setRollSpeed(T/16);
                        roll(duration,instrument);
                        setRollSpeed(0);
                        louder(2);
                        }

                    else if (p[k] == '^')
                        drum(duration,instrument,n+1);
                    else if (p[k] == 'A')
                        {
                        louder(2);
                        drum(duration,instrument,n+1);
                        softer(2);
                        }
                    else if (p[k] == 'v')
                        {
                        if (n == 0)
                            drum(duration,instrument,n);
                        else
                            drum(duration,instrument,n-1);
                        }
                    else if (p[k] == 'V')
                        {
                        louder(2);
                        if (n == 0)
                            drum(duration,instrument,n);
                        else
                            drum(duration,instrument,n-1);
                        softer(2);
                        }
                    else if (p[k] == 'X' || p[k] == 'O')
                        {
                        louder(2);
                        drum(duration,instrument,n);
                        softer(2);
                        }
                    else
                        drum(duration,instrument,n);
                    softer(amp);
                    }
                else
                    rest(duration);
            }
        }
    }

void
drumline(int times,void (*body)(void),void (*ending)(void))
    {
    int i;

    for (i = 0; i < times - 1; ++i)
        body();

    if (ending)
        ending();
    else
        body();
    }

void
d44a()
    {
    louder(2);
    drum(Q,kick,getDrumNote());
    silence(I);
    drum(I,kick,getDrumNote());
    drum(H,kick,getDrumNote());
    softer(2);
    }

void
d44b()
    {
    int i;

    softer(5);
    for (i = 0; i < 8; ++i)
        drum(I,hhclosed,getDrumNote());
    louder(5);

    backwards(W);

    silence(Q);
    drum(H,snare,getDrumNote());
    louder(2);
    drum(Q,snare,getDrumNote());
    softer(2);

    backwards(W);

    drum(Q,tom,getDrumNote());
    drum(Q,tom,getDrumNote());
    drum(Q,tom,getDrumNote());
    drum(Q,tom,getDrumNote());
    }

void
d44c()
    {
    int i;

    softer(5);
    for (i = 0; i < 8; ++i)
        drum(I,hhclosed,getDrumNote());
    louder(5);

    backwards(W);

    silence(Q);
    drum(H,snare,getDrumNote());
    louder(2);
    drum(Q,snare,getDrumNote());
    softer(2);

    backwards(W);

    drum(H,tomLo,getDrumNote());
    drum(Q,tomLo,getDrumNote());
    drum(Q,tomLo,getDrumNote());
    }

void
d44d()
    {
    int i;

    softer(5);
    for (i = 0; i < 8; ++i)
        drum(I,hhclosed,getDrumNote() + i % 2);
    louder(5);

    backwards(W);

    silence(Q);
    drum(H,snare,getDrumNote()+1);
    louder(2);
    drum(Q,snare,getDrumNote());
    softer(2);

    backwards(W);

    drum(H,tomLo,getDrumNote());
    drum(I,tom,getDrumNote());
    drum(Qd,tomLo,getDrumNote());
    }

void
d44e()
    {
    int i;

    for (i = 0; i < 8; ++i)
        drum(I,hhpedal,getDrumNote() + i % 2);

    backwards(W);

    silence(Q);
    drum(H,snare,getDrumNote()+1);
    louder(2);
    drum(Q,snare,getDrumNote());
    softer(2);

    backwards(W);

    drum(H,tomLo,getDrumNote());
    drum(Qd,tomLo,getDrumNote()-1);
    drum(I,tomLo,getDrumNote());
    }

void
d44f()
    {
    int i;
    double old;

    softer(5);
    for (i = 0; i < 8; ++i)
        drum(I,hhpedal,getDrumNote() + i % 2);
    louder(5);

    backwards(W);

    old = setSustain(0.99999);
    silence(Q);
    drum(H,snare,getDrumNote());
    louder(2);
    drum(Q,snare,getDrumNote()+1);
    softer(2);
    setSustain(old);

    backwards(W);

    drum(H,kick,getDrumNote()-1);
    drum(I,kick,getDrumNote());
    drum(Q,kick,getDrumNote());
    drum(I,kick,getDrumNote()-1);
    }

void
d44g()
    {
    int i;

    softer(3);
    for (i = 0; i < 8; ++i)
        drum(I,hhclosed,getDrumNote() + i % 2);
    louder(3);

    backwards(W);

    silence(Q);
    drum(H,snare,getDrumNote()+1);
    louder(2);
    drum(Q,snare,getDrumNote());
    softer(2);

    backwards(W);

    drum(Qd,tom,getDrumNote()-1);
    drum(I,tom,getDrumNote());
    drum(Qd,tom,getDrumNote());
    drum(I,tom,getDrumNote()-1);
    }


void
d44h()
    {
    int i;

    for (i = 0; i < 8; ++i)
        drum(I,hhclosed,getDrumNote() + i % 2);

    backwards(W);

    silence(Q);
    drum(H,drumset,getDrumNote()+2);
    louder(2);
    drum(Q,drumset,getDrumNote()+1);
    softer(2);

    backwards(W);

    drum(I,tomLo,getDrumNote());
    drum(Q,tom,getDrumNote());
    drum(H,tom,getDrumNote());
    drum(I,tomLo,getDrumNote());
    }

void
d44i()
    {
    int location = getLocation();

    drum(H,tomLo,getDrumNote());
    drum(Q,tom,getDrumNote());

    setLocation(location);

    silence(I);
    drum(S,snare,getDrumNote());
    drum(S,snare,getDrumNote()+1);
    drum(S,snare,getDrumNote());
    drum(S,snare,getDrumNote()+1);
    drum(I,snare,getDrumNote());
    drum(I,snare,getDrumNote()+1);
    rest(Q);

    setLocation(location+beatsToSamples(W));
    }

void
d44j()
    {
    int i;
    int location = getLocation();

    softer(3);
    for (i = 0; i < 2; ++i)
        {
        drum(I,hhpedal,getDrumNote()-1);
        drum(I,hhpedal,getDrumNote());
        drum(I,hhpedal,getDrumNote());
        drum(I,hhpedal,getDrumNote()-1);
        }
    louder(3);

    setLocation(location);

    silence(Q-T);
    drum(Q,snare,getDrumNote()-1);
    silence(Q);
    drum(Q,snare,getDrumNote()-1);

    setLocation(location);

    drum(Q,drumset,getDrumNote()-1);
    drum(Q,drumset,getDrumNote());
    drum(Q,drumset,getDrumNote()-1);
    drum(Q,drumset,getDrumNote());

    setLocation(location+beatsToSamples(W));
    }

void
d24a()
    {
    int location = getLocation();

    louder(4);
    drum(Qd,drumset,TIM_MIDDLE);
    drum(I,drumset,TIM_LOW);
    softer(4);

    setLocation(location);

    silence(I);
    drum(St,drumset,SNARE-1);
    drum(St,drumset,SNARE);
    louder(2);
    drum(S,drumset,SNARE-1);
    drum(Q,drumset,SNARE-1);
    softer(2);

    setLocation(location+beatsToSamples(H));
    }

void
d44bfill()
    {
    drum(I,drumset,SNARE);
    drum(I,drumset,TIM_HIGH);

    drum(I,drumset,SNARE);
    drum(I,drumset,TIM_MIDDLE);

    drum(I,drumset,SNARE);
    drum(I,drumset,TIM_LOW);

    drum(Q,drumset,TIM_MIDDLE);
    backwards(Q);
    louder(2);
    drum(Q,drumset,CRASH);
    softer(2);
    }

void
d44cfill()
    {
    drum(I,drumset,SNARE+1);
    drum(S,drumset,SNARE);
    drum(S,drumset,SNARE);

    drum(I,drumset,SNARE+1);
    drum(I,drumset,SNARE+1);

    drum(S,drumset,SNARE);
    drum(S,drumset,SNARE);
    drum(S,drumset,SNARE+1);
    drum(S,drumset,SNARE);

    drum(I,drumset,TIM_HIGH);
    drum(I,drumset,TIM_HIGH);

    drum(Q,drumset,TIM_MIDDLE);
    backwards(Q);
    drum(Q,drumset,CRASH);
    backwards(Q);
    }


void
d44dfill()
    {
    startMeasure();
    drum(Qt,drumset,TIM_HIGH);
    drum(It,drumset,TIM_MIDDLE);
    drum(It,drumset,TIM_MIDDLE);
    drum(Qt,drumset,TIM_HIGH);

    drum(Qt,drumset,TIM_MIDDLE);
    drum(It,drumset,TIM_LOW);
    drum(It,drumset,TIM_LOW);
    drum(Qt,drumset,TIM_MIDDLE);

    drum(Qt,drumset,TIM_MIDDLE);
    drum(It,drumset,TIM_LOW);
    drum(It,drumset,TIM_LOW);
    drum(Qt,drumset,TIM_MIDDLE);

    drum(Q,drumset,SNARE);
    checkMeasure();
    }

void
d44rolla()
    {
    double oldAmplitude = getAmplitude();
    double oldSlop = slopTiming;
    double shift = T/8;

    slopTiming /= 5;

    beginCrescendo(0.25);

    drum(It-shift,drumset,SNARE+1);
    drum(It+shift,drumset,SNARE);
    drum(It-shift,drumset,SNARE+1);

    drum(It+shift,drumset,SNARE-1);
    drum(It-shift,drumset,SNARE+1);
    drum(It+shift,drumset,SNARE+1);

    drum(It-shift,drumset,SNARE-1);
    drum(It+shift,drumset,SNARE+1);
    drum(It-shift,drumset,SNARE);

    drum(It+shift,drumset,SNARE-1);
    drum(It-shift,drumset,SNARE);
    drum(It+shift,drumset,SNARE);

    drum(It-shift,drumset,SNARE);
    drum(It+shift,drumset,SNARE-1);
    drum(It-shift,drumset,SNARE);

    drum(It+shift,drumset,SNARE-1);
    drum(It-shift,drumset,SNARE-1);
    drum(It+shift,drumset,SNARE);

    endCrescendo();

    setAmplitude(oldAmplitude);
    slopTiming = oldSlop;

    drum(Q,drumset,SNARE);

    backwards(W);

    drum(H,drumset,TIM_LOW);
    drum(H,drumset,TIM_LOW);
    }

void
d44rollc()
    {
    int i;
    double oldAmplitude = getAmplitude();
    double oldSlop = slopTiming;
    double shift = T/4;

    slopTiming /= 4;
    beginCrescendo(0.35);

    for (i = 0; i < 3; ++i)
        {
        drum(S-shift,drumset,SNARE+1);
        drum(S+shift,drumset,SNARE);
        drum(S-shift,drumset,SNARE);
        drum(S+shift,drumset,SNARE-1);
        }

    endCrescendo();
    slopTiming = oldSlop;

    setAmplitude(oldAmplitude);

    drum(Q,drumset,SNARE);
    backwards(Q-T);
    drum(Q,drumset,CRASH);
    backwards(Q+T);

    backwards(W);

    drum(H,drumset,TIM_LOW);
    drum(H,drumset,TIM_LOW);
    }

void
d44rolld()
    {
    drum(Qt,drumset,SNARE+1);
    drum(Qt,drumset,SNARE);
    drum(Qt,drumset,SNARE+1);

    drum(Qt,drumset,SNARE);
    drum(Qt,drumset,SNARE-1);
    drum(Qt,drumset,SNARE);

    drum(Qt,drumset,SNARE-1);
    drum(Qt,drumset,SNARE-1);
    drum(Qt,drumset,SNARE-1);

    drum(Qt,drumset,SNARE+1);
    drum(Qt,drumset,SNARE-1);
    drum(Qt,drumset,SNARE+1);

    backwards(W);

    louder(2);
    drum(H,drumset,TIM_LOW);
    drum(H,drumset,TIM_LOW);
    softer(2);
    }

void
d44rolle()
    {
    drum(I,drumset,SNARE-1);
    drum(I+T/2,drumset,SNARE+1);

    louder(2);
    drum(I-T/2,drumset,SNARE-1);
    softer(2);

    drum(I+T/2,drumset,SNARE+1);

    louder(2);
    drum(I-T/2,drumset,SNARE-1);
    softer(2);

    drum(I+T/2,drumset,SNARE);

    louder(2);
    drum(I-T/2,drumset,SNARE-1);
    softer(2);

    drum(I,drumset,SNARE-1);

    backwards(W);

    drum(H,drumset,TIM_LOW);
    drum(H,drumset,TIM_LOW);
    }

void
d44rollf()
    {
    drum(Ht,drumset,SNARE);
    drum(Ht,drumset,SNARE+1);
    drum(Ht,drumset,SNARE);
    drum(Ht,drumset,SNARE+1);
    drum(Ht,drumset,SNARE+1);
    drum(Ht,drumset,SNARE);

    backwards(W);

    drum(H,drumset,TIM_LOW);
    drum(H,drumset,TIM_LOW);
    }
