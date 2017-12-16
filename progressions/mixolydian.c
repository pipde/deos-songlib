#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/songlib.h"

#define dir "/usr/local/share/samples/guitar-electric/"
#define base "clean_"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "mixolydian";
char *PROGRAM_VERSION = "0.01";

static int inst;
static int octave;

int
main()
    {
    int i;

    songInit();

    inst = readScale(dir,base);

    openOutput("mixolydian.rra",0,0);

    setSustain(0.9999);

    setKey(C);
    setMode(MIXOLYDIAN);
    octave = 2;

    c1(Q,inst,octave);
    maj3(Q,inst,octave,degree(1));
    c2(Q,inst,octave);
    min3(Q,inst,octave,degree(2));
    c3(Q,inst,octave);
    dim3(Q,inst,octave,degree(3));
    c4(Q,inst,octave);
    maj3(Q,inst,octave,degree(4));
    c5(Q,inst,octave);
    min3(Q,inst,octave,degree(5));
    c6(Q,inst,octave);
    min3(Q,inst,octave,degree(6));
    c7(Q,inst,octave);
    maj3(Q,inst,octave,degree(7));
    c1(Q,inst,octave+1);

    silence(H);
    
    // a progression
    //      suggested: c1
    //      suggested: at least one of c3, c5, and c7
    //      suggested: at least one of c2, c4, and c6

    octave = 4;

    for (i = 0; i < 3; ++i)
        {
	c2(Q,inst,octave);
	c2(Qd,inst,octave);
	c2(I,inst,octave);
	c3(Qd,inst,octave);
	c3(I,inst,octave);
	c1(H,inst,octave);
	}
    c1(Q,inst,octave);
    c2(Q,inst,octave);
    c7(Q,inst,octave-1);
    c5(W,inst,octave-1);

    closeOutput();

    return 0;
    }

