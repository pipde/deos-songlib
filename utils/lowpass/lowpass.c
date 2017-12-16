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

char *PROGRAM_NAME = "lowpass";
char *PROGRAM_VERSION = "0.01";

static int processOptions(int, char **);
static void processData(FILE *,FILE *,RRA *);

static double AmpFactor = 1.00;
static double Freq = 2000;
static double Style = LOWPASS;

int
main(int argc,char **argv)
    {
    int argIndex = 1;

    FILE *in,*out;
    RRA *h;

    if (strcmp(argv[0],"rrahighpass") == 0)
        {
        Freq = 100;
        Style = HIGHPASS;
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
        Fatal("lowpass works on mono files, use slowpass for stero files\n");

    writeRRAHeader(out,h,"modifiedBy: lowpass",0);

    processData(in,out,h);

    fclose(in);
    fclose(out);

    return 0;
    }

static void
processData(FILE *in,FILE *out,RRA *h)
    {
    int amp;

    fof *m = newFOF(Freq,Style);

    amp = readRRAAmplitude(in,out,h->bitsPerSample,outputComment);
    while (!feof(in))
        {
        //fprintf(out,"%d\n",setMDE(m,amp));
        int d = setFOF(m,amp);
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
            case 'f':
                Freq = atof(arg);
                argUsed = 1;
                break;
            case 'h':
                printf("%s options:\n",argv[0]);
                printf("  -a N   set amplitude factor to N\n");
                printf("         values > 1 increase loudness\n");
                printf("         values < 1 decrease loudness\n");
                printf("         default is %f\n",AmpFactor);
                printf("  -f N.N set cutoff frequency to N.N Hertz\n");
                printf("         frequencies %s the cutoff will be "
                                    "reduced in amplitude\n",
                                    Style == LOWPASS? "above" : "below");
                printf("         default is %f Hz\n",Freq);
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

