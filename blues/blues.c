#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/songlib.h"

#define dir "/usr/local/share/samples/mandolin/"
#define base "sample_"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "quickstart";
char *PROGRAM_VERSION = "0.01";

static int last = 0;

static void
bar(int instrument,int octave,int degree,int e0,int e1,int e2,int e3,int e4)
    {
    double drag = I;
    // first, play the chord
    play(Q + drag,instrument,octave,getcnote(degree,0));
    play(Q - drag,instrument,octave,getcnote(degree,1));
    play(Q,instrument,octave,getcnote(degree,2));
    // now play extras
    play(Q,instrument,octave,getcnote(degree,2) + e0);
    play(Q,instrument,octave,getcnote(degree,2) + e0 + e1);
    play(Q,instrument,octave,getcnote(degree,2) + e0 + e1 + e2);
    play(Q,instrument,octave,getcnote(degree,2) + e0 + e1 + e2 + e3);
    play(Q,instrument,octave,getcnote(degree,2) + e0 + e1 + e2 + e3 + e4);
    }

static void
twelve_1a(int instrument,int octave,int e0,int e1,int e2,int e3,int e4)
    {
    int i;

    startMeasure();

    for (i = 0; i < 2; ++i)
        {
        bar(instrument,octave,1,e0,e1,e2,e3,e4);
        }

    checkMeasure();
    }

static void
twelve_1b(int instrument,int octave,int e0,int e1,int e2,int e3,int e4)
    {
    twelve_1a(instrument,octave,e0,e1,e2,e3,e4);
    }

static void
twelve_2a(int instrument,int octave,int e0,int e1,int e2,int e3,int e4)
    {
    int i;

    startMeasure();

    for (i = 0; i < 2; ++i)
        {
        bar(instrument,octave,4,e0,e1,e2,e3,e4);
        }

    checkMeasure();
    }

static void
twelve_2b(int instrument,int octave,int e0,int e1,int e2,int e3,int e4)
    {
    startMeasure();

    twelve_1a(instrument,octave,e0,e1,e2,e3,e4);

    checkMeasure();
    }

static void
twelve_3a(int instrument,int octave,int e0,int e1,int e2,int e3,int e4)
    {
    startMeasure();

    bar(instrument,octave,5,e0,e1,e2,e3,e4);
    bar(instrument,octave,4,e0,e1,e2,e3,e4);

    checkMeasure();
    }

static void
twelve_3b(int instrument,int octave,int e0,int e1,int e2, int e3, int e4)
    {
    twelve_2b(instrument,octave,e0,e1,e2,e3,e4);
    }

int
main()
    {
    int instrument;
    int octave = 2;

    songInit();

    instrument = readScale(dir,base);

    setTempo(250);
    setTime(4,4);
    setStride(0.05);
    setSustain(0.99995);
    setAmplitude(0.4);

    openOutput("blues.rra",0,0);

    setMode(IONIAN);

    twelve_1a(instrument,octave,2,1,-1,-2,-3);
    twelve_1b(instrument,octave,2,1,-1,-2,-3);
    twelve_2a(instrument,octave,2,1,-1,-2,-3);
    twelve_2b(instrument,octave,2,1,-1,-2,-3);
    twelve_3a(instrument,octave,2,1,-1,-2,-3);
    twelve_3b(instrument,octave,2,1,-1,-2,-3);
    c(1,H,instrument,octave);

    closeOutput();

    return 0;
    }
