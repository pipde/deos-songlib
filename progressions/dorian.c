#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/songlib.h"

#define dir "/usr/local/share/samples/guitar-electric/"
#define base "clean_"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "dorian";
char *PROGRAM_VERSION = "0.01";

static int inst;
static int octave;

int
main()
    {
    int i;

    songInit();

    inst = readScale(dir,base);

    openOutput("dorian.rra",0,0);

    setSustain(0.9999);

    setKey(C);
    setMode(DORIAN);
    octave = 2;

    c1(Q,inst,octave);
    min3(Q,inst,octave,degree(1));
    c2(Q,inst,octave);
    min3(Q,inst,octave,degree(2));
    c3(Q,inst,octave);
    maj3(Q,inst,octave,degree(3));
    c4(Q,inst,octave);
    maj3(Q,inst,octave,degree(4));
    c5(Q,inst,octave);
    min3(Q,inst,octave,degree(5));
    c6(Q,inst,octave);
    dim3(Q,inst,octave,degree(6));
    c7(Q,inst,octave);
    maj3(Q,inst,octave,degree(7));
    c1(Q,inst,octave+1);

    silence(H);
    
    // a progression
    //      suggested: c1
    //      suggested: at least one of c2, c4, and c6
    //      suggested: at least one of c3, c5, and c7

    octave = 4;

    for (i = 0; i < 3; ++i)
	{
	c1(Q,inst,octave);
	c3(Qd,inst,octave);
	c5(I,inst,octave);
	c2(Q,inst,octave);
	c4(Q,inst,octave);
	}
    c4(H,inst,octave);
    c3(W,inst,octave);

    closeOutput();

    return 0;
    }

