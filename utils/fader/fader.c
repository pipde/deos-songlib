#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <assert.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/rra.h"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "fader";
char *PROGRAM_VERSION = "0.01";

static int inrun = 10;
static int outrun = 50;
static double amplify = 1;

static void process(RRA *,FILE *,FILE *);
static int processOptions(int,char **);

int
main(int argc,char **argv)
    {
    int argIndex = 1;
    static FILE *in,*out;

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
        printf("usage: fader -n<N> [<input rra file> [<output rra file>]]\n");
        exit(-1);
        }

    h = newRRAHeader();
    
    readRRAHeader(in,h,0);

    process(h,in,out);

    fclose(in);
    fclose(out);

    return 0;
    }

static void
process(RRA *h,FILE *in,FILE *out)
    {
    int i,j;
    int amp;
    double delta,factor = 0;

    writeRRAHeader(out,h,"modifiedBy: fader",0);

    delta = 1.0 / inrun;
    for (i = 0; !feof(in); ++i)
        {
        for (j = 0; j < h->channels; ++j)
            {
            amp = readRRAAmplitude(in,out,h->bitsPerSample,outputComment);
            fprintf(out,"%d\n",(int)(amp * factor * amplify));
            }
        factor += delta;
        if (i == inrun) { factor = 1.0; delta = 0; }
        if (h->samples - i == outrun) { delta = - 1.0 / outrun; }
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
            case 'i':
                inrun = atoi(arg);
                fprintf(stderr,"inrun is %d\n",inrun);
                argUsed = 1;
                break;
            case 'o':
                outrun = atoi(arg);
                fprintf(stderr,"outrun is %d\n",outrun);
                argUsed = 1;
                break;
            case 'v':
                printf("%s version %s\n", PROGRAM_NAME, PROGRAM_VERSION);
                exit(0);
                break;
            case 'h':
                printf("fader options:\n");
                printf("  -i N     set inrun to N\n");
                printf("           default value is %d\n",inrun);
                printf("  -o N     set outrun to N\n");
                printf("           default value is %d\n",outrun);
                printf("  -a N     set amplification factor to N\n");
                printf("           default value is %f\n",amplify);
                printf("  -h       help\n");
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

