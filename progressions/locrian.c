#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/songlib.h"

#define dir "/usr/local/share/samples/guitar-electric/"
#define base "clean_"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "locrian";
char *PROGRAM_VERSION = "0.01";

static int inst;
static int octave;

int
main()
    {
    songInit();

    inst = readScale(dir,base);

    openOutput("locrian.rra",0,0);

    setSustain(0.9999);

    setKey(C);
    setMode(LOCRIAN);
    octave = 2;

    c1(Q,inst,octave);
    dim3(Q,inst,octave,degree(1));
    c2(Q,inst,octave);
    maj3(Q,inst,octave,degree(2));
    c3(Q,inst,octave);
    min3(Q,inst,octave,degree(3));
    c4(Q,inst,octave);
    min3(Q,inst,octave,degree(4));
    c5(Q,inst,octave);
    maj3(Q,inst,octave,degree(5));
    c6(Q,inst,octave);
    maj3(Q,inst,octave,degree(6));
    c7(Q,inst,octave);
    min3(Q,inst,octave,degree(7));
    c1(Q,inst,octave+1);

    closeOutput();

    return 0;
    }

