#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/songlib.h"


#define dir "/usr/local/share/samples/"
#define base "mandolin/sample_"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "testlib";
char *PROGRAM_VERSION = "0.01";

int
main()
    {
    int instrument;
    int octave = 2;

    songInit();

    instrument = readScale(dir,base);

    setTempo(250);
    setTime(4,4);
    setStride(1.0);
    setSustain(0.99995);
    setAmplitude(0.3);

    openOutput("testlib.rra",0,0);

    cD7(2,2*W,instrument,octave);
    icD7(2,1,2*W,instrument,octave);
    icD7(2,2,2*W,instrument,octave);
    //cm7(1,2*W,instrument,octave);
    //ca7(1,2*W,instrument,octave);

    //ic7(1,1,2*W,instrument,octave);
    //ic7(1,2,2*W,instrument,octave);
    //cM7th(1,2*W,instrument,octave);
    //cm7th(1,2*W,instrument,octave);

    closeOutput();

    return 0;
    }
