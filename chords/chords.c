#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/songlib.h"

#define dir "/usr/local/share/samples/"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "chords";
char *PROGRAM_VERSION = "0.01";

static int instrument;
static int octave;

static void
riff1(int startNote,int shift)
    {
    chord(I,instrument,octave,startNote+shift,4,0);
    chord(I,instrument,octave,startNote+shift,4,0);
    chord(I,instrument,octave,startNote+shift+4,-4,+3,0);
    chord(I,instrument,octave,startNote+shift+4,-4,+3,0);
    chord(I,instrument,octave,startNote+shift+7,-7,0);
    chord(I,instrument,octave,startNote+shift+7,-7,0);
    chord(I,instrument,octave,startNote+9+shift,-9,0);
    chord(I,instrument,octave,startNote+7+shift,-7,0);
    }

static void
riff2(int startNote,int shift)
    {
    play(Q,instrument,octave,startNote+shift);
    play(I,instrument,octave,startNote+shift);
    play(I,instrument,octave,startNote+shift-2);
    play(Q,instrument,octave,startNote+shift-4);
    play(Q,instrument,octave,startNote+shift-4);
    }

static void
verse(double location,int repeats,int shift)
    {
    int i;

    setLocation(location);

    startMeasure();

    play(W,instrument,octave,A+shift);

    for (i = 0; i < 1 + repeats; ++i)
        {
        riff1(D,shift);
        riff1(D,shift);
        riff1(G,shift);
        riff1(G,shift);
        riff1(D,shift);
        riff1(D,shift);

        riff2(Db+12,shift);
        riff2(B,shift);

        silence(measuresToBeats(3));
        }

    backwards(H);
    play(W,instrument,octave,D+shift);

    checkMeasure();
    }

int
main()
    {
    double location;

    songInit();

    instrument = readScale(dir,"mandolin/sample_");

    setTempo(132);
    setTime(4,4);
    openOutput("chords.rra",0,0);
    setStride(0.05);
    setSustain(0.9999);
    setAmplitude(0.1);

    octave = 4;

    location = getLocation();

    verse(location,1,0);

    closeOutput();

    return 0;
    }
