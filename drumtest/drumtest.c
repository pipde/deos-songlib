#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/songlib.h"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "drumtest";
char *PROGRAM_VERSION = "0.01";

static int hera;
static int inst;
static int inst2;

int
main()
    {
    int i;

    songInit();

    hera = readScale("/usr/local/share/samples/drums/","hera_");

    setTempo(250);
    setTime(4,4);
    openOutput("drumtest.rra",0,0);
    setStride(0.05);
    setSustain(0.998);
    setAmplitude(0.4);

    setSlopTiming(0.05);
    setSlopAmplitude(0.5);
    setSlopPitch(0.8);

    setPrimaryEmphasis(1);
    setSecondaryEmphasis(1);

    inst = SNARE;
    inst2 = SNARE;

    for (i = 0; i < 32; ++i)
        nplay(It,hera,inst);

    rest(W);

    for (i = 0; i < 16; ++i)
        {
        softer(1.2);
        drum(It,hera,inst);
        louder(1.2);
        drum(It,hera,inst2);
        softer(1.05);
        }

    rest(W);

    for (i = 0; i < 16; ++i)
        {
        softer(1.2);
        drum(It,hera,inst);
        louder(1.2);
        drum(It,hera,inst2);
        louder(1.005);
        }

    rest(W);

    for (i = 0; i < 15; ++i)
        {
        softer(1.2);
        drum(It,hera,inst);
        louder(1.2);
        drum(It,hera,inst2);
        softer(1.005);
        }

    closeOutput();

    return 0;
    }

