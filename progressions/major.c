#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/songlib.h"

#define dir "/usr/local/share/samples/guitar-electric/"
#define base "clean_"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "major";
char *PROGRAM_VERSION = "0.01";

static int inst;

int
main()
    {
    songInit();

    inst = readScale(dir,base);

    openOutput("major.rra",0,0);

    setSustain(0.9999);

    setKey(C);
    setMode(MAJOR);

    cI(Qd,inst,2);
    maj3(Qd,inst,2,C);
    cii(Qd,inst,2);
    min3(Qd,inst,2,D);
    ciii(Qd,inst,2);
    min3(Qd,inst,2,E);
    cIV(Qd,inst,2);
    maj3(Qd,inst,2,F);
    cV(Qd,inst,2);
    maj3(Qd,inst,2,G);
    cvi(Qd,inst,2);
    min3(Qd,inst,2,A);
    cviid(Qd,inst,2);
    dim3(Qd,inst,2,B);
    cI(Qd,inst,3);

    closeOutput();

    return 0;
    }

