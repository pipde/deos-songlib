#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/songlib.h"

//#define dir "/usr/local/share/samples/guitar/"
//#define base "loud_"
#define dir "/usr/local/share/samples/guitar-electric/"
#define base "clean_"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "prog";
char *PROGRAM_VERSION = "0.01";

static int inst;

static void
playMode(int mode,int octave)
    {
    int i;
    setMode(mode);

    for (i = 0; i < 2; ++i)
        {
        c(1,Q,inst,octave);
        c(6,H,inst,octave-1);
        c(4,Hd,inst,octave-1);
        c(5,Q,inst,octave-1);
        c(5,H,inst,octave-1);
        c(4,Hd,inst,octave-1);
        }
    silence(H);
    }


int
main()
    {
    songInit();

    inst = readScale(dir,base);

    openOutput("prog.rra",0,0);

    setSustain(0.9999);
    setAmplitude(0.1);

    setKey(C);

    /* major modes */

    playMode(IONIAN,3);
    playMode(LYDIAN,3);
    playMode(MIXOLYDIAN,3);

    silence(W);

    /* minor modes */

    playMode(AEOLIAN,3);
    playMode(PHRYGIAN,3);
    playMode(LOCRIAN,3);
    playMode(DORIAN,3);
    playMode(MELODIC_MINOR,3);
    playMode(HARMONIC_MINOR,3);

    closeOutput();

    return 0;
    }

