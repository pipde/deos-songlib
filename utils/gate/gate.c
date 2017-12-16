#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/rra.h"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "gate";
char *PROGRAM_VERSION = "0.01";

static double off = 0.05;
static double on  = 0.06;

static double holdTime = 10;
static double attack = 1.0001;
static double release = 0.9999;

#define BOTTOM 0.000001

static void process(FILE *,FILE *t,RRA *);
static int processOptions(int,char **);

int
main(int argc,char **argv)
    {
    int argIndex = 1;

    FILE *in,*out;
    RRA *h;

    argIndex = processOptions(argc,argv);

    if (argc-argIndex == 0)
        {
        in = stdin;
        out = stdout;
        }
    else if (argc-argIndex == 1)
        {
        in = OpenFile(argv[argIndex],"r");
        out = stdout;
        }
    else if (argc-argIndex == 2)
        {
        in = OpenFile(argv[argIndex],"r");
        out = OpenFile(argv[argIndex+1],"w");
        }
    else
        {
        printf("usage: gate -fN [<input rra file> [<output rra file>]]\n");
        exit(-1);
        }

    h = newRRAHeader();
    
    readRRAHeader(in,h,0);
    writeRRAHeader(out,h,"modifiedBy: gate",0);

    process(in,out,h);

    fclose(in);
    fclose(out);

    return 0;
    }

static void
process(FILE *in,FILE *out,RRA *h)
    {
    int i,j;
    int ion,ioff;
    int *count,*gating;
    int hold;
    double *factor,*bump;
    double max = pow(2,h->bitsPerSample - 1) - 1;

    ion = on * max;
    ioff = off * max;
    hold = h->sampleRate * holdTime / 1000;

    fprintf(stderr,"cutoff is %d\n",ioff);
    fprintf(stderr,"cuton is %d\n",ion);
    fprintf(stderr,"holdtime is %d\n",hold);

    count = (int *) New(sizeof(int) * h->channels);
    gating = (int *) New(sizeof(int) * h->channels);
    factor = (double *) New(sizeof(double) * h->channels);
    bump = (double *) New(sizeof(double) * h->channels);

    for (j = 0; j < h->channels; ++j)
        {
        count [j] = 0;
        gating[j] = 0;
        factor[j] = 1.0;
        bump[j] = 1.0;
        }

    for (i = 0; !feof(in); ++i)
        {
        for (j = 0; j < h->channels; ++j)
            {
            int amp = readRRAAmplitude(in,h->bitsPerSample,0);

            fprintf(out,"%d\n",(int) (amp * factor[j]));

            if (!gating[j] && abs(amp) < ion)
                {
                if (count[j] == hold)
                    {
                    //fprintf(stderr,"gating channel %d at index %d!\n",j,i);
                    gating[j] = 1;
                    bump[j] = release;
                    }
                ++count[j];
                }
            else
                count[j] = 0;

            if (gating[j] && abs(amp) > ion)
                {
                //fprintf(stderr,"loudering channel %d at index %d!\n",j,i);
                gating[j] = 0;
                bump[j] = attack;
                }

            factor[j] *= bump[j];

            if (factor[j] < BOTTOM) factor[j] = BOTTOM;
            else if (factor[j] > 1) factor[j] = 1;
            }
        }

    if (i != h->samples + 1)
        fprintf(stderr,"WARNING: %d samples found, %d expected\n",i,h->samples);
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
            case 'a':
                attack = atof(arg);
                argUsed = 1;
                break;
            case 'H':
                holdTime = atof(arg);
                argUsed = 1;
                break;
            case 'o':
                off = atof(arg);
                on = off * 1.1;
                argUsed = 1;
                break;
            case 'O':
                on = atof(arg);
                argUsed = 1;
                break;
            case 'r':
                release = atof(arg);
                argUsed = 1;
                break;
            case 'v':
                printf("%s version %s\n", PROGRAM_NAME, PROGRAM_VERSION);
                exit(0);
                break;
            case 'h':
                printf("gate options:\n");
                printf("  -b N     set log base to N\n");
                printf("           good values range from >1 upwards\n");
                printf("           lower values mean higher compression\n");
                printf("  -h       help\n");
                exit(0);
                break;
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

