/* identify possible frequencies based upon zero crossings
 *
 * identifyz2 in.rra
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <assert.h>
#include <limits.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/wave.h"
#include "/usr/local/include/songlib/rra.h"

//#define WRITE_ERROR 1

#define OFFSET  0.15
#define WINDOW  2000
#define SPAN    4

#define STEP 1.0594630944

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "identifyz2";
char *PROGRAM_VERSION = "0.01";

static double initialOffset = OFFSET;

static int processOptions(int, char **);

double freqs[] =
    {
    13.75,
    13.75 * STEP,
    13.75 * STEP*STEP,
    13.75 * STEP*STEP*STEP,
    13.75 * STEP*STEP*STEP*STEP,
    13.75 * STEP*STEP*STEP*STEP*STEP,
    13.75 * STEP*STEP*STEP*STEP*STEP*STEP,
    13.75 * STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    13.75 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    13.75 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    13.75 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    13.75 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    27.5,
    27.5 * STEP,
    27.5 * STEP*STEP,
    27.5 * STEP*STEP*STEP,
    27.5 * STEP*STEP*STEP*STEP,
    27.5 * STEP*STEP*STEP*STEP*STEP,
    27.5 * STEP*STEP*STEP*STEP*STEP*STEP,
    27.5 * STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    27.5 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    27.5 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    27.5 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    27.5 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
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
    3520,
    3520 * STEP,
    3520 * STEP*STEP,
    3520 * STEP*STEP*STEP,
    3520 * STEP*STEP*STEP*STEP,
    3520 * STEP*STEP*STEP*STEP*STEP,
    3520 * STEP*STEP*STEP*STEP*STEP*STEP,
    3520 * STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    3520 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    3520 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    3520 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    3520 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    };

char *notes[] =
    {
    "ax", "a#x", "bx", "c0", "c#0", "d0", "d#0", "e0", "f0", "f#0", "g0", "g#0",
    "a0", "a#0", "b0", "c1", "c#1", "d1", "d#1", "e1", "f1", "f#1", "g1", "g#1",
    "a1", "a#1", "b1", "c2", "c#2", "d2", "d#2", "e2", "f2", "f#2", "g2", "g#2",
    "a2", "a#2", "b2", "c3", "c#3", "d3", "d#3", "e3", "f3", "f#3", "g3", "g#3",
    "a3", "a#3", "b3", "c4", "c#4", "d4", "d#4", "e4", "f4", "f#4", "g4", "g#4",
    "a4", "a#4", "b4", "c5", "c#5", "d5", "d#5", "e5", "f5", "f#5", "g5", "g#5",
    "a5", "a#5", "b5", "c6", "c#6", "d6", "d#6", "e6", "f6", "f#6", "g6", "g#6",
    "a6", "a#6", "b6", "c7", "c#7", "d7", "d#7", "e7", "f7", "f#7", "g7", "g#7",
    "a7", "a#7", "b7", "c8", "c#8", "d8", "d#8", "e8", "f8", "f#8", "g8", "g#8",
    "a8", "a#8", "b8", "c9", "c#9", "d9", "d#9", "e9", "f9", "f#9", "g9", "g#9",
    "a9", "a#9", "b9", "c10", "c#10", "d10", "d#10", "e10", "f10", "f#10", "g10", "g#10",
    };

char *
closestNote(double freq)
    {
    int i,index = -1;
    for (i = 0; index < 0 && i < 120; ++i)
        {
        if (freq >= freqs[i] && freq < freqs[i+1])
            {
            if (freq - freqs[i] >= freqs[i+1] - freq)
                index = i+1;
            else
                index = i;
            }
        }

    return notes[index];
    }

int
main(int argc,char **argv)
    {
    int argIndex = 1;

    int i,count,prev,curr;
    double freq;
    FILE *in,*out;
    char *source = 0;
    RRA *h;
    static int skip;

    argIndex = processOptions(argc,argv);

    if (argc-argIndex == 0)
        {
        in = stdin;
        out = stdout;
        }
    else if (argc-argIndex == 1)
        {
        source = argv[argIndex];
        in = OpenFile(source,"r");
        out = stdout;
        }
    else if (argc-argIndex == 2)
        {
        source = argv[argIndex];
        in = OpenFile(source,"r");
        out = OpenFile(argv[argIndex+1],"w");
        }
    else
        {
        printf("usage: %s [<input rra file>]\n",argv[0]);
        exit(-1);
        }

    h = newRRAHeader();
    
    readRRAHeader(in,h,0);

    if (h->channels != 1)
        Fatal("%s requires a mono audio file\n",argv[0]);

    /* skip over the first part */

    skip = initialOffset * h->sampleRate;
    printf("skip is %d\n",skip);
    for (i = 0; i < skip; ++i)
        (void) readRRAAmplitude(in,out,h->bitsPerSample,outputComment);

    /* find first upward zero */

    prev = readRRAAmplitude(in,out,h->bitsPerSample,outputComment);
    curr = readRRAAmplitude(in,out,h->bitsPerSample,outputComment);

    while (prev >= 0 || curr != 0)
        {
        prev = curr;
        curr = readRRAAmplitude(in,out,h->bitsPerSample,outputComment);
        }

    count = 0;
    while (count < 2000)
        {
        prev = readRRAAmplitude(in,out,h->bitsPerSample,outputComment);
        curr = readRRAAmplitude(in,out,h->bitsPerSample,outputComment);
        count += 2;

        /* find next upward zero */
    
        while (prev >= 0 || curr != 0)
            {
            prev = curr;
            curr = readRRAAmplitude(in,out,h->bitsPerSample,outputComment);
            ++count;
            }

        freq = h->sampleRate / (double) count;
        printf("possible wavelength/frequency/note: %d/%f/%s\n",
            count,freq,closestNote(freq));
        }

    fclose(in);
    fclose(out);

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
            case 'v':
                printf("%s version %s\n", PROGRAM_NAME, PROGRAM_VERSION);
                exit(0);
                break;
            case 'o':
                initialOffset = atof(arg);
                argUsed = 1;
                break;
            case 'h':
                printf("identifyz options:\n");
                printf("  -f     only report the detected frequency\n");
                printf("  -p     generate a command to repitch the note\n");
                printf("  -n     only report the nearest note\n");
                printf("  -o N   start analyzing N seconds into the note\n");
                printf("  -v     display the version number\n");
                printf("  -h     help\n");
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

