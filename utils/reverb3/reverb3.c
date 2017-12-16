#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <assert.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/songlib.h"
#include "/usr/local/include/songlib/rra.h"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "reverb3";
char *PROGRAM_VERSION = "0.01";

static int processOptions(int, char **);
static void processData(FILE *,FILE *,RRA *);
static double reverb(int,int,int);

static int invert = 1;
static int delay1 = 25;
static int delay2 = 27;
static double ingain = 1;
static double outgain = 0.4;
//circular buffer size
static int size = 1000;

static int **filtered;
static int **unfiltered;

FILE *in,*out;
RRA *h;

int
main(int argc,char **argv)
    {
    int argIndex = 1;

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
        printf("usage: ereverb -a<N> -d<N> "
            "[<input rra file> [<output rra file>]]\n");
        exit(-1);
        }

    songInit();

    h = newRRAHeader();
    
    readRRAHeader(in,h,0);

    writeRRAHeader(out,h,"modifiedBy: ereverb",0);

    //convert from milliseconds to samples
    delay1 = delay1 * h->sampleRate / 1000;
    delay2 = delay2 * h->sampleRate / 1000;

    size = 2 * delay1 + delay2;

    processData(in,out,h);

    fclose(in);
    fclose(out);

    return 0;
    }

static void
processData(FILE *in,FILE *out,RRA *h)
    {
    int i,j;

    //convert delay from seconds to samples

    filtered = (int **) New(sizeof(int *) * h->channels);
    unfiltered = (int **) New(sizeof(int *) * h->channels);

    for (j = 0; j < h->channels; ++j)
        {
        filtered[j] = (int *) New(sizeof(int) * size);
        unfiltered[j] = (int *) New(sizeof(int) * size);
        for (i = 0; i < size; ++i)
           {
           filtered[j][i] = 0;
           unfiltered[j][i] = 0;
           }
        }

    for (i = 0; i < h->samples; ++i)
        for (j = 0; j < h->channels; ++j)
            {
            int amp = readRRAAmplitude(in,out,h->bitsPerSample,outputComment);

            unfiltered[j][i%size] = amp;
            filtered[j][i%size] = reverb(j,i,h->samples);

            fprintf(out,"%d\n",filtered[j][i%size]);
            }
    }

static double
reverb(int channel,int t,int samples)
    {
    double total;

    total = ingain * unfiltered[channel][t%size];
    total += outgain * filtered[channel][(t - delay1 + size) % size]; 
    total += outgain * 0.7 * invert * filtered[channel][(t - delay2 + size) % size];

    return total;
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
            case 'a':
                ingain = atof(arg);
                argUsed = 1;
                break;
            case '1':
                delay1 = atoi(arg);
                argUsed = 1;
                break;
            case '2':
                delay2 = atoi(arg);
                argUsed = 1;
                break;
            case 'g':
                outgain = atof(arg);
                argUsed = 1;
                break;
            default:
                printf("reverb options:\n");
                printf("  -a N.N  set the amplitude factor to N.N\n");
                printf("          default is %f\n",ingain);
                printf("  -g N.N  set echo loudness decay to N.N\n");
                printf("          good values are 0.4 to 0.8\n");
                printf("          default is %f\n",outgain);
                printf("  -1 N    set initial echo delay to <N> milliseconds\n");
                printf("          good values are around 25 ms\n");
                printf("          default is %d\n",delay1);
                printf("  -2 N    set the cancelling delay to <N> milliseconds\n");
                printf("          good values are slightly more than initial delay\n");
                printf("          default is %d\n",delay2);
                printf("  -h      help\n");
                exit(0);
                break;
            }

        if (separateArg && argUsed)
            ++argIndex;

        ++argIndex;
        }

    return argIndex;
}

