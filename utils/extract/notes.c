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

static void
notes(int instrument,int octave)
    {
    play(W,instrument,octave,C);
    rest(W+W);
    play(W,instrument,octave,D);
    rest(W+W);
    play(W,instrument,octave,E);
    rest(W+W);
    play(W,instrument,octave,F);
    rest(W+W);
    play(W,instrument,octave,G);
    rest(W+W);
    play(W,instrument,octave,A);
    rest(W+W);
    play(W,instrument,octave,B);
    rest(W+W);
    play(W,instrument,octave+1,C);
    rest(W+W);
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
    setAmplitude(1.0);

    openOutput("notes.rra",0,0);

    rest(W);

    notes(instrument,octave);

    closeOutput();

    return 0;
    }
