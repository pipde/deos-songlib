#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/songlib.h"

#define dir "/usr/local/share/samples/guitar-electric/"
#define base "distorted01_"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "riff";
char *PROGRAM_VERSION = "0.01";

static int inst;

int
main()
    {
    int i;
    int octave;

    songInit();

    inst = readScale(dir,base);

    openOutput("riff.rra",0,0);

    setSustain(0.9999);
    setAmplitude(0.1);
    setStride(0.01);
    setStrideSlop(0.05);

    octave = 2;

    for (i = 0; i < 2; ++i)
        {
	i38p(H,inst,octave,E);
	i59p(H,inst,octave,C);
	setAmplitude(getAmplitude() * 0.8);
	i47p(Id,inst,octave,F);
	setAmplitude(getAmplitude() / 0.8);
	i59p(W,inst,octave,D);

	i37p(H,inst,octave,E);
	i59p(H,inst,octave,C);
	setAmplitude(getAmplitude() * 0.8);
	i47p(Id,inst,octave,F);
	setAmplitude(getAmplitude() / 0.8);
	i59p(W,inst,octave,D);

	i38p(H,inst,octave,E);
	i59p(H,inst,octave,C);
	setAmplitude(getAmplitude() * 0.8);
	i38p(Id,inst,octave,E);
	setAmplitude(getAmplitude() / 0.8);
	i47p(W,inst,octave,F);

	i38p(H,inst,octave,E);
	i47p(H,inst,octave,F);
	setAmplitude(getAmplitude() * 0.8);
	i37p(Id,inst,octave,A);
	setAmplitude(getAmplitude() / 0.8);
	if(i == 1) setSustain(0.99999);
	i57p(W,inst,octave,G);
	}

    closeOutput();

    return 0;
    }
