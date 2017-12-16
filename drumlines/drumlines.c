#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/songlib.h"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "drumlines";
char *PROGRAM_VERSION = "0.01";

static int drumset;

int
main()
    {
    
    songInit();

    setTempo(140);
    setTime(4,4);
    openOutput("drumlines.rra",0,0);
    setStride(0.05);
    setSustain(0.9995);
    setAmplitude(.15);

    loadDrumKit();

    drumline(4,d44a,0);
    drumline(4,d44b,0);
    drumline(4,d44c,0);
    drumline(4,d44d,0);
    //drumline(4,d44e,0);
    //drumline(4,d44f,0);
    //drumline(4,d44g,0);
    //drumline(4,d44h,0);
    //drumline(4,d44i,0);
    //drumline(4,d44j,0);

    closeOutput();

    return 0;
    }

