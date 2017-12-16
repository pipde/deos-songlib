#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/songlib.h"


#define dir "/usr/local/share/samples/"
//#define base "guitar/classical_"
#define base "mandolin/sample_"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "quickstart";
char *PROGRAM_VERSION = "0.01";

static int last = 0;

static void
verse(int instrument,int octave)
    {
    startMeasure();

    //line 1

    chord(H,instrument,octave,C,-5,-12,IX);

    chord(Q,instrument,octave,C,2,-5,-12,IX);
    chord(Q,instrument,octave,C,7,14,IX);


    chord(Q,instrument,octave,E,-9,-16,IX);

    chord(H,instrument,octave,D,5,17,IX);
    chord(Q,instrument,octave,F,-8,-15,IX);
    
    chord(Q,instrument,octave,F,-8,-15,IX);
    chord(H,instrument,octave,D,7,17,IX);
    chord(Q,instrument,octave,F,-8,-15,IX);

    chord(Q,instrument,octave,E,-9,-16,IX);
    chord(Q,instrument,octave,D,5,-2,IX);
    chord(H,instrument,octave,C,-5,-12,IX);

    //line 2

    play(Q,instrument,octave,C);
    min(H,instrument,octave,G);
    play(H,instrument,octave,G);
    play(Q,instrument,octave,A);
    play(Q,instrument,octave,G);
    play(Q,instrument,octave,F);

    play(Hd,instrument,octave,G);
    chord(Q,instrument,octave+1,E,-9,-16,IX);

    chord(Q,instrument,octave,C,7,16,IX);
    chord(Q,instrument,octave+1,E,-9,-16,IX);
    chord(Q,instrument,octave,D,-2,IX);
    chord(Q,instrument,octave,C,7,12,IX);

    chord(H,instrument,octave+1,C,-8,-12,IX); //first half of next measure

    //line 3

    play(H,instrument,octave,C); //last half of measure

    play(Q,instrument,octave,G);
    play(Q,instrument,octave,F);
    play(H,instrument,octave,E);

    play(H,instrument,octave,F);
    chord(H,instrument,octave,G,-7,IX);

    chord(H,instrument,octave,D,7,IX);
    chord(Hd,instrument,octave,D,-3,IX); //first quarter of next measure

    //line 4

    play(Q,instrument,octave,C); //last 3/4 of measure
    play(Q,instrument,octave,C);
    play(Q,instrument,octave,D);

    play(Q,instrument,octave,E);
    play(Q,instrument,octave,F);
    play(Q,instrument,octave,E);
    play(Q,instrument,octave,D);

    play(Q,instrument,octave,C);
    play(Q,instrument,1,A);
    chord(H,instrument,octave,F,-3,-7,IX);

    chord(Q,instrument,octave,F,-3,-7,IX);

    if (last) setSustain(0.999999);

    chord(Hd,instrument,octave,C,4,7,IX);
//END:

    checkMeasure();
    }

int
main()
    {
    int instrument;
    int octave = 3;

    songInit();

    instrument = readScale(dir,base);

    setTempo(250);
    setTime(4,4);
    setStride(0.05);
    setSustain(0.99995);
    setAmplitude(0.3);

    openOutput("quickstart.rra",0,0);

    beginAccelerando(1);
    verse(instrument,octave);
    last = 1;
    verse(instrument,octave);

    closeOutput();

    return 0;
    }
