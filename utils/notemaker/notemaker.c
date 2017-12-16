#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/rra.h"
#include "/usr/local/include/songlib/wave.h"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "notemaker";
char *PROGRAM_VERSION = "0.01";

#define STEP 1.0594630944

double freqs[120] =
    {
    55,
    55 * STEP,
    55 * STEP*STEP,
    55 * STEP*STEP*STEP,
    55 * STEP*STEP*STEP*STEP,
    55 * STEP*STEP*STEP*STEP*STEP,
    55 * STEP*STEP*STEP*STEP*STEP*STEP,
    55 * STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    55 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    55 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    55 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    55 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    110,
    110 * STEP,
    110 * STEP*STEP,
    110 * STEP*STEP*STEP,
    110 * STEP*STEP*STEP*STEP,
    110 * STEP*STEP*STEP*STEP*STEP,
    110 * STEP*STEP*STEP*STEP*STEP*STEP,
    110 * STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    110 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    110 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    110 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    110 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    220,
    220 * STEP,
    220 * STEP*STEP,
    220 * STEP*STEP*STEP,
    220 * STEP*STEP*STEP*STEP,
    220 * STEP*STEP*STEP*STEP*STEP,
    220 * STEP*STEP*STEP*STEP*STEP*STEP,
    220 * STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    220 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    220 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    220 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    220 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    440,
    440 * STEP,
    440 * STEP*STEP,
    440 * STEP*STEP*STEP,
    440 * STEP*STEP*STEP*STEP,
    440 * STEP*STEP*STEP*STEP*STEP,
    440 * STEP*STEP*STEP*STEP*STEP*STEP,
    440 * STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    440 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    440 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    440 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    440 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    880,
    880 * STEP,
    880 * STEP*STEP,
    880 * STEP*STEP*STEP,
    880 * STEP*STEP*STEP*STEP,
    880 * STEP*STEP*STEP*STEP*STEP,
    880 * STEP*STEP*STEP*STEP*STEP*STEP,
    880 * STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    880 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    880 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    880 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    880 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    1760,
    1760 * STEP,
    1760 * STEP*STEP,
    1760 * STEP*STEP*STEP,
    1760 * STEP*STEP*STEP*STEP,
    1760 * STEP*STEP*STEP*STEP*STEP,
    1760 * STEP*STEP*STEP*STEP*STEP*STEP,
    1760 * STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    1760 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    1760 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    1760 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    1760 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    };

char *notes[120] =
    {
    "a0", "a#0", "b0", "c1", "c#1", "d1", "d#1", "e1", "f1", "f#1", "g1", "g#1",
    "a1", "a#1", "b1", "c2", "c#2", "d2", "d#2", "e2", "f2", "f#2", "g2", "g#2",
    "a2", "a#2", "b2", "c3", "c#3", "d3", "d#3", "e3", "f3", "f#3", "g3", "g#3",
    "a3", "a#3", "b3", "c4", "c#4", "d4", "d#4", "e4", "f4", "f#4", "g4", "g#4",
    "a4", "a#4", "b4", "c5", "c#5", "d5", "d#5", "e5", "f5", "f#5", "g5", "g#5",
    "a5", "a#5", "b5", "c6", "c#6", "d6", "d#6", "e6", "f6", "f#6", "g6", "g#6",
    "a6", "a#6", "b6", "c7", "c#7", "d7", "d#7", "e7", "f7", "f#7", "g7", "g#7",
    "a7", "a#7", "b7", "c8", "c#8", "d8", "d#8", "e8", "f8", "f#8", "g8", "g#8",
    "a8", "a#8", "b8", "c9", "c#9", "d9", "d#9", "e9", "f9", "f#9", "g9", "g#9",
    };

static int seconds = 8;
static int jump = 50;
static int sampleRate = 44100;

static int processOptions(int,char **);

int
main(int argc,char **argv)
    {
    int argIndex = 1;

    int i,j,k;
    FILE *out;
    RRA *h;

    argIndex = processOptions(argc,argv);

    h = newRRAHeader();

    h->samples = sampleRate * seconds;
    h->sampleRate = sampleRate;
    h->channels = 1;
    
    for (i = 0; i < 120; i+=1)
        {
        int length;
        char buffer[512];
        int data[44100];

        length = (int) (sampleRate / freqs[i]);
        if (length <= 0) continue;
        sprintf(buffer,"note_%s.rra",notes[i]);
        out = OpenFile(buffer,"w");
        writeRRAHeader(out,h,"createdBy: notemaker",0);
        printf("length is %d\n",length);
        data[0] = 0;
        for (k = 1; k < length; ++k)
            {
            data[k] = data[k-1] + randomInteger(-jump,jump);
            }
        k = 0;
        for (j = 0; j < h->samples; ++j)
            {
            fprintf(out,"%d\n",data[k]);
            ++k;
            if (k >= length) k = 0;
            }
        fclose(out);
        }

    return 0;
    }

/* only -oXXX  or -o XXX options */

static int
processOptions(int argc, char **argv)
{
    int argIndex;
    int argUsed;
    int separateArg;
    char *arg;

    argIndex = 1;

    while (argIndex < argc && *argv[argIndex] == '-') {

        separateArg = 0;
        argUsed = 0;

        if (argv[argIndex][2] == '\0')
            {
            arg = argv[argIndex+1];
            separateArg = 1;
            }
        else
            arg = argv[argIndex]+2;

        switch (argv[argIndex][1])
            {
            /*
             * when option has an argument, do this
             *
             *     examples are -m4096 or -m 4096
             *
             *     case 'm':
             *         MemorySize = atol(arg);
             *         argUsed = 1;
             *         break;
             *
             *
             * when option does not have an argument, do this
             *
             *     example is -a
             *
             *     case 'a':
             *         PrintActions = 1;
             *         break;
             */
            case 's':
                seconds = atof(arg);
                argUsed = 1;
                break;
            case 'j':
                jump = atoi(arg);
                argUsed = 1;
                break;
            case 'v':
                printf("%s version %s\n", PROGRAM_NAME, PROGRAM_VERSION);
                exit(0);
                break;
            default:
                Fatal("option %s not understood\n",argv[argIndex]);
            }

        if (separateArg && argUsed)
            ++argIndex;

        ++argIndex;
        }

    return argIndex;
}

