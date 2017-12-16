#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/songlib.h"

#define dir "/usr/local/share/samples/guitar-electric/"
#define base "clean_"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "ionian";
char *PROGRAM_VERSION = "0.01";

static int inst;
static int octave;

int
main()
    {
    songInit();

    inst = readScale(dir,base);
    octave = 2;

    openOutput("ionian.rra",0,0);

    setSustain(0.9999);

    setKey(C);
    setMode(IONIAN); /* or setMode(MAJOR); */

    /*
    c1(Q,inst,octave);
    maj3(Q,inst,octave,degree(1));
    c2(Q,inst,octave);
    min3(Q,inst,octave,degree(2));
    c3(Q,inst,octave);
    min3(Q,inst,octave,degree(3));
    c4(Q,inst,octave);
    maj3(Q,inst,octave,degree(4));
    c5(Q,inst,octave);
    maj3(Q,inst,octave,degree(5));
    c6(Q,inst,octave);
    min3(Q,inst,octave,degree(6));
    c7(Q,inst,octave);
    dim3(Q,inst,octave,degree(7));
    c1(Q,inst,octave+1);

    silence(H);

    c17(Q,inst,octave);
    maj7(Q,inst,octave,degree(1));
    c27(Q,inst,octave);
    min7(Q,inst,octave,degree(2));
    c37(Q,inst,octave);
    min7(Q,inst,octave,degree(3));
    c47(Q,inst,octave);
    maj7(Q,inst,octave,degree(4));
    c57(Q,inst,octave);
    dom7(Q,inst,octave,degree(5));
    c67(Q,inst,octave);
    min7(Q,inst,octave,degree(6));
    c77(Q,inst,octave);
    hdim7(Q,inst,octave,degree(7));
    c17(Q,inst,octave+1);

    silence(H);
    */

    c19(Q,inst,octave);
    maj9(Q,inst,octave,degree(1));
    c29(Q,inst,octave);
    minmaj9(Q,inst,octave,degree(2));
    c39(Q,inst,octave);
    min9(Q,inst,octave,degree(3));
    c49(Q,inst,octave);
    maj9(Q,inst,octave,degree(4));
    c59(Q,inst,octave);
    dom9(Q,inst,octave,degree(5));
    c69(Q,inst,octave);
    minmaj9(Q,inst,octave,degree(6));
    c79(Q,inst,octave);
    hdim9(Q,inst,octave,degree(7));
    c19(Q,inst,octave+1);

    closeOutput();

    return 0;
    }

