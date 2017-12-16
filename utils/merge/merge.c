#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/songlib.h"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "merge";
char *PROGRAM_VERSION = "0.01";

char *names[12] = {"c","c#","d","d#","e","f","f#","g","g#","a","a#","b"};

int
main(int argc, char **argv)
    {
    int i;
    int instrument1,instrument2;

    songInit();

    instrument1 = readScale(argv[1],argv[2]);
    instrument2 = readScale(argv[1],argv[3]);

    setTempo(132);
    setTime(4,4);
    setSustain(0.99995);
    setAmplitude(0.5);

    for (i = 24; i <= 120; ++i)
        {
        char buffer[512];
        RRA *n1,*n2;
        sprintf(buffer,"merged_%s%d.rra",names[i % 12],i / 12);
        openOutput(buffer,0,0);
        n1 = getNumberedNote(instrument1,i);
        n2 = getNumberedNote(instrument2,i);

        rplay(samplesToBeats(n1->samples),n1);
        backwards(samplesToBeats(n1->samples));
        rplay(samplesToBeats(n2->samples),n2);

        closeOutput();
        }

    return 0;
    }
