#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/songlib.h"

#define dir "/usr/local/share/samples/guitar-electric/"
#define base "clean_"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "prog";
char *PROGRAM_VERSION = "0.01";

static int inst;
static int octave;

int
main()
    {
    songInit();

    inst = readScale(dir,base);

    openOutput("harmonic_minor.rra",0,0);

    setSustain(0.9999);

    setKey(C);
    setMode(HARMONIC_MINOR);
    octave = 2;

    c1(Q,inst,octave);
    min3(Q,inst,octave,degree(1));
    c2(Q,inst,octave);
    dim3(Q,inst,octave,degree(2));
    c3(Q,inst,octave);
    //aug3(Q,inst,octave,degree(3));
    chord(Q,inst,octave,degree(3),4,8,0);
    c4(Q,inst,octave);
    min3(Q,inst,octave,degree(4));
    c5(Q,inst,octave);
    maj3(Q,inst,octave,degree(5));
    c6(Q,inst,octave);
    maj3(Q,inst,octave,degree(6));
    c7(Q,inst,octave);
    dim3(Q,inst,octave,degree(7));
    c1(Q,inst,octave+1);

    closeOutput();

    return 0;
    }

