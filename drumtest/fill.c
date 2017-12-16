#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/songlib.h"

/* fills from
 
http://www.christianguitar.org/lessons/lesson.php?id=2

*/

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "fill";
char *PROGRAM_VERSION = "0.01";

static int level;

static int snare;
static int hitom;
static int lotom;
static int crash;
static int hihat;
static int bass;
static int hera;

static int snareInst;
static int snareNote;
static int hitomInst;
static int hitomNote;
static int lotomInst;
static int lotomNote;
static int crashInst;
static int crashNote;
static int hihatInst;
static int hihatNote;
static int bassInst;
static int bassNote;

void
hihat1()
    {
    int i;
    for (i = 0; i < 8; ++i)
        drum(2 * S,hihatInst,hihatNote);
    }

void
hihat2()
    {
    int i;
    for (i = 0; i < 4; ++i)
        drum(2 * S,hihatInst,hihatNote);
    softer(2);
    drum(2 * S,crashInst,crashNote);
    louder(2);
    for (i = 0; i < 3; ++i)
        drum(2 * S,hihatInst,hihatNote);
    }

void
hihat3()
    {
    int i;
    for (i = 0; i < 4; ++i)
        drum(2 * S,hihatInst,hihatNote);
    for (i = 0; i < 4; ++i)
        drum(2 * S,hihatInst,hihatNote);
    }

void
hihat4()
    {
    int i;
    softer(2);
    drum(2 * S,crashInst,crashNote);
    louder(2);
    //drum(I,hihat,0);
    for (i = 0; i < 7; ++i)
        drum(2 * S,hihatInst,hihatNote);
    }

void playHiHat()
    {
    hihat1();
    hihat2();
    hihat3();
    hihat4();
    hihat2();
    hihat3();
    hihat4();
    hihat2();
    hihat3();
    }

void
snare1()
    {
    rest(4 * S);
    drum(8 * S,snareInst,snareNote);
    drum(4 * S,snareInst,snareNote);
    }

void
snare2()
    {
    rest(12 * S);
    drum(4 * S,snareInst,snareNote);
    }

void
snare3()
    {
    rest(4 * S);
    drum(12 * S,snareInst,snareNote);
    }

void
playSnare()
    {
    snare1();
    snare2();
    snare3();
    snare1();
    snare2();
    snare3();
    snare1();
    snare2();
    snare3();
    }

void
bass1()
    {
    softer(10);
    nplay(8 * S,bassInst,bassNote);
    nplay(8 * S,bassInst,bassNote);
    louder(10);
    }

void
hitom2()
    {
    int i;
    for (i = 0; i < 2; ++i)
        {
        drum(1 * S,hitomInst,hitomNote);
        drum(1 * S,hitomInst,hitomNote-1);
        }
    rest(12 * S);
    }

void
hitom3()
    {
    int i;
    rest(8 * S);
    for (i = 0; i < 2; ++i)
        {
        drum(1 * S,hitomInst,hitomNote+1);
        drum(1 * S,lotomInst,hitomNote);
        }
    rest(4 * S);
    }

void
lotom2()
    {
    int i;
    rest(4 * S);
    for (i = 0; i < 2; ++i)
        {
        drum(1 * S,lotomInst,lotomNote+1);
        drum(1 * S,lotomInst,lotomNote);
        }
    rest(8 * S);
    }

void
lotom3()
    {
    int i;
    rest(12 * S);
    for (i = 0; i < 2; ++i)
        {
        drum(1 * S,lotomInst,lotomNote);
        drum(1 * S,lotomInst,lotomNote-1);
        }
    }

void
playHiTom()
    {
    int i;
    for (i = 0; i < 3; ++i)
        {
        rest(16 * S);
        hitom2();
        hitom3();
        }
    }

void
playLoTom()
    {
    int i;
    for (i = 0; i < 3; ++i)
        {
        rest(16 * S);
        lotom2();
        lotom3();
        }
    }

int
main(int *argc,char **argv)
    {
    int i;

    level = 10;
    if (argc > 1)
        level = atoi(argv[1]);

    songInit();

    snare = readScale("/usr/local/share/samples/yamaha-drums/","snare_");
    hitom = readScale("/usr/local/share/samples/yamaha-drums/","tom_");
    hihat = readScale("/usr/local/share/samples/yamaha-drums/","hihat_");
    crash = readScale("/usr/local/share/samples/yamaha-drums/","crash_");
    lotom = readScale("/usr/local/share/samples/yamaha-drums/","floor_");
    hera = readScale("/usr/local/share/samples/drums/","hera_");

    setTempo(100);
    setTime(4,4);
    openOutput("fill.rra",0,0);
    setStride(0.05);
    setSustain(0.995);
    setAmplitude(0.50);

    setSlopAmplitude(0.4);
    setSlopPitch(0.8);

    hihatInst = hera;
    hihatNote = HAT_OPEN;

    snareInst = hera;
    snareNote = SNARE;

    crashInst = hera;
    crashNote = CRASH;

    hitomInst = hera;
    hitomNote = TOM_HIGH;

    lotomInst = hera;
    lotomNote = TOM_LOW;

    bassInst = hera;
    bassNote = TIM_LOW;

    /* hihat */

    softer(2);
    playHiHat();
    louder(2);

    /* snare */

    if (level > 0)
        {
        backwards(9 * W);
        playSnare();
        }

    /* bass */

    if (level > 1)
        {
        backwards(9 * W);
        for (i = 0; i < 9; ++i) bass1();
        }

    /* hi tom */

    if (level > 2)
        {
        backwards(9 * W);
        playHiTom();
        }

    /* lo tom */

    if (level > 2)
        {
        backwards(9 * W);
        playLoTom();
        }

    setSustain(0.995);
    softer(2);
    drum(I,crashInst,crashNote);
    backwards(I);
    drum(I,hera,TIM_LOW);

    closeOutput();

    return 0;
    }

