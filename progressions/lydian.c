#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/songlib.h"

#define dir "/usr/local/share/samples/guitar-electric/"
#define base "clean_"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "lydian";
char *PROGRAM_VERSION = "0.01";

static int inst;
static int octave;

int
main()
    {
    int i;

    songInit();

    inst = readScale(dir,base);
    octave = 2;

    openOutput("lydian.rra",0,0);

    setSustain(0.9999);

    setKey(C);
    setMode(LYDIAN);

    c1(Q,inst,octave);
    maj3(Q,inst,octave,degree(1));
    c2(Q,inst,octave);
    maj3(Q,inst,octave,degree(2));
    c3(Q,inst,octave);
    min3(Q,inst,octave,degree(3));
    c4(Q,inst,octave);
    dim3(Q,inst,octave,degree(4));
    c5(Q,inst,octave);
    maj3(Q,inst,octave,degree(5));
    c6(Q,inst,octave);
    min3(Q,inst,octave,degree(6));
    c7(Q,inst,octave);
    min3(Q,inst,octave,degree(7));
    c1(Q,inst,octave+1);

    silence(H);
    
    // a progression
    //      suggested: I
    //      suggested: at least one of II, iv, and vii
    //      suggested: at least one of iii, V, and vi

    for (i = 0; i < 4; ++i)
        {
	c1(Q,inst,1);
	c5(Qd,inst,1);
	c1(Q,inst,1);
	c7(Qd,inst,1);
	c1(Q,inst,1);
	c5(Q,inst,1);
	}

    closeOutput();

    return 0;
    }

