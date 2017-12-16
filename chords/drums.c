#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/songlib.h"

#define dir "/usr/local/share/samples/"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "chords";
char *PROGRAM_VERSION = "0.01";

static int drumset;
static int octave;

static void
verse(double location,int repeats)
    {
    int i;

    setLocation(location);

    startMeasure();

    silence(H);
    drumroll(H,drumset,TOM_MIDDLE,I);

    for (i = 0; i < 1 + repeats; ++i)
        {

	drumline(6,drumset,d44b,d44c);

	backwards(H);
	drumroll(Q,drumset,SNARE,S);
	drum(I,drumset,TOM_HIGH);
	drum(I,drumset,TOM_HIGH-1);

	drumline(4,drumset,d44a,0);

	d44i(drumset);
	}

    silence(H);

    checkMeasure();
    }


int
main()
    {
    double location;

    songInit();

    drumset = readScale(dir,"drums/hera_");

    setTempo(132);
    setTime(4,4);
    openOutput("drums.rra",0,0);
    setStride(0.05);
    setSustain(0.9999);
    setAmplitude(0.1);

    octave = 3;

    location = getLocation();

    verse(location,1);

    closeOutput();

    return 0;
    }

