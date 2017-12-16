#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/songlib.h"

#define dir "/usr/local/share/samples/guitar-electric/"
#define base "clean_"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "minor";
char *PROGRAM_VERSION = "0.01";

static int inst;

int
main()
    {
    songInit();

    inst = readScale(dir,base);

    openOutput("minor.rra",0,0);

    setSustain(0.9999);

    setKey(C);
    setMode(MINOR);

    ci(Qd,inst,2);
    min3(Qd,inst,2,C);
    ciid(Qd,inst,2);
    dim3(Qd,inst,2,D);
    cIII(Qd,inst,2);
    maj3(Qd,inst,2,Ds);
    civ(Qd,inst,2);
    min3(Qd,inst,2,F);
    cv(Qd,inst,2);
    min3(Qd,inst,2,G);
    cVI(Qd,inst,2);
    maj3(Qd,inst,2,Gs);
    cVII(Qd,inst,2);
    maj3(Qd,inst,2,As);
    ci(Qd,inst,3);

    closeOutput();

    return 0;
    }

