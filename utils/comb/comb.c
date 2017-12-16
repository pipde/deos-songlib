#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/songlib.h"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "comb";
char *PROGRAM_VERSION = "0.01";

#define getFiltered(x,y,s) (y < 0 ? 0 : interp(filtered+x,s,y))
#define getUnfiltered(x,y,s) (y < 0 ? 0 : interp(unfiltered+x,s,y))

static double comb(int,int,int);
static int processOptions(int,char **);

int currentIndex;
double **filtered;
double **unfiltered;
double gain = 0.5;
double delay;
double delayFactor = 2;

int
main(int argc,char **argv)
    {
    int argIndex = 1;

    int i,j;
    double max;
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
        fprintf(stderr,"usage: comb -h -a<N.M> -w<N.M> -d<N.M> -l<N.M> "
            "[<input rra file> [<output rra file>]]\n");
        exit(-1);
        }

    setInterp(3);

    h = newRRAHeader();
    
    readRRAHeader(in,h,0);

    writeRRAHeader(out,h,"modifiedBy: comb",0);

    delay = h->sampleRate * delayFactor / 1000;
    fprintf(stderr,"delay is %f samples\n",delay);

    filtered = (double **) New(sizeof(double *) * h->channels);
    unfiltered = (double **) New(sizeof(double *) * h->channels);
    for (i = 0; i < h->channels; ++i)
        {
        filtered[i] = (double *) New(sizeof(double) * h->samples);
        unfiltered[i] = (double *) New(sizeof(double) * h->samples);
        }

    max = pow(2,h->bitsPerSample - 1) - 1;

    for (currentIndex = 0; currentIndex < h->samples; ++currentIndex)
        for (j = 0; j < h->channels; ++j)
            {
            double amp = readRRAAmplitude(in,0,h->bitsPerSample,0) / max;

            unfiltered[j][currentIndex] = amp;
            filtered[j][currentIndex] = comb(j,currentIndex,h->samples);

            fprintf(out,"%f\n",filtered[j][currentIndex]);
            }

    fclose(in);
    fclose(out);

    return 0;
    }

static double
comb(int channel,int index,int samples)
    {
    double result;

    int delayed = index - delay;

    result =  getUnfiltered(channel,delayed,samples)
        + gain * getFiltered(channel,delayed,samples);

    //printf("comb returns %f\n",result);
    //getchar();

    return result;
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

    while (argIndex < argc && *argv[argIndex] == '-')
        {
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
            case 'd':
                delayFactor = atoi(arg);
                argUsed = 1;
                break;
            case 'g':
                gain = atof(arg);
                argUsed = 1;
                break;
            case 'h':
                printf("comb options:\n");
                printf("  -gN      set feedback gain to <N>\n");
                printf("  -dN      set delay to <N> milliseconds\n");
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
