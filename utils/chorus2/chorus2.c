#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/rra.h"
#include "/usr/local/include/songlib/wave.h"
#include "/usr/local/include/songlib/filters.h"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "chorus";
char *PROGRAM_VERSION = "0.01";

static int processOptions(int, char **);
static void processData(FILE *,FILE *,RRA *);

static double AmpFactor = 1.00;
static double MinDelay = 0.020;
static double MaxDelay = 0.030;
static double LFreq = 1.00;
static double Wet = 0.50;
static double FB = 0.00;

int
main(int argc,char **argv)
    {
    int argIndex = 1;

    FILE *in,*out;
    RRA *h;

    if (strcmp(argv[0],"rraflanger2") == 0)
        {
        AmpFactor = 0.5;
        MinDelay = 0.00;
        MaxDelay = 0.01;
        LFreq = 0.40;
        Wet = 0.50;
        FB = 0.60;
        }
    else if (strcmp(argv[0],"rravibrato2") == 0)
        {
        AmpFactor = 0.5;
        MinDelay = 0.005;
        MaxDelay = 0.0065;
        LFreq = 5.50;
        Wet = 1.00;
        FB = 0.30;
        }


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
        printf("usage: chorus [<input rra file> [<output wave file>]]\n");
        exit(-1);
        }

    h = newRRAHeader();
    
    readRRAHeader(in,h,0);

    if (h->channels > 1)
        Fatal("chorus2 works on mono files, use schorus2 for stero files\n");

    writeRRAHeader(out,h,"modifiedBy: chorus",0);

    processData(in,out,h);

    fclose(in);
    fclose(out);

    return 0;
    }

static void
processData(FILE *in,FILE *out,RRA *h)
    {
    int amp;

    int lo = (int)(h->sampleRate * MinDelay);
    int hi = (int)(h->sampleRate * MaxDelay);

    mde *m = newMDE(Wet,FB,hi+1,LFreq,lo,hi);

    amp = readRRAAmplitude(in,out,h->bitsPerSample,outputComment);
    while (!feof(in))
        {
        //fprintf(out,"%d\n",setMDE(m,amp));
        int d = updateMDE(m,amp);
        fprintf(out,"%d\n",(int)(d * AmpFactor));
        //getchar();
        amp = readRRAAmplitude(in,out,h->bitsPerSample,outputComment);
        }
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
                printf("%s version %s\n", argv[0], PROGRAM_VERSION);
                exit(0);
                break;
            case 'a':
                AmpFactor = atof(arg);
                argUsed = 1;
                break;
            case 'm':
                MinDelay = atof(arg);
                argUsed = 1;
                break;
            case 'M':
                MaxDelay = atof(arg);
                argUsed = 1;
                break;
            case 'f':
                LFreq = atof(arg);
                argUsed = 1;
                break;
            case 'F':
                FB = atof(arg);
                argUsed = 1;
                break;
            case 'w':
                Wet = atof(arg);
                argUsed = 1;
                break;
            case 'h':
                printf("chorus options:\n");
                printf("  -a N   set amplitude factor to N\n");
                printf("         values > 1 increase loudness\n");
                printf("         values < 1 decrease loudness\n");
                printf("         default is %f\n",AmpFactor);
                printf("  -m N.N set minimum delay to N.N seconds\n");
                printf("         default is %f\n",MinDelay);
                printf("  -M N.N set max delay to N.N seconds\n");
                printf("         default is %f\n",MaxDelay);
                printf("  -f N.N set the LFO frequency to N.N\n");
                printf("         default is %f\n",LFreq);
                printf("  -F N.N set the feedback factor to N.N\n");
                printf("         default is %f\n",FB);
                printf("  -w N.N set how much of the processed signal is mixed in\n");
                printf("         ranges from 0 (none) to 1 (all)\n");
                printf("         default is %f\n",Wet);
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

