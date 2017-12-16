#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/rra.h"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "hartley";
char *PROGRAM_VERSION = "0.01";

static int channel = 0;
static FILE *in,*out;

static void analyze(RRA *);
static double hartley(int *,int,int);
static int processOptions(int,char **);

int
main(int argc,char **argv)
    {
    int argIndex = 1;

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
        printf("usage: hartley -c<N> "
            "[<input rra file> [<output rra file>]]\n");
        exit(-1);
        }

    h = readRRA(in,0);

    analyze(h);

    fclose(in);
    fclose(out);

    return 0;
    }

static void
analyze(RRA *h)
    {
    int k;
    int *data;

    if (channel >= h->channels)
        Fatal("not enough channels (channel zero is the first channel)\n");

    fprintf(out,"RRAUDIO\n");
    fprintf(out,"createdBy: hartley\n");
    fprintf(out,"channels: 1\n");
    fprintf(out,"samples: %d\n",h->samples);
    fprintf(out,"%%%%\n");

    data = h->data[channel];

    /* Produce the hartley data here.  */

    for (k = 0; k < h->samples; ++k)
        {
        fprintf(out,"%f\t\t!frequency bin: %f\n",
            hartley(data,h->samples,k) / h->samples,
            (1.0 * k * h->sampleRate) / (double) h->samples);
        }
    }

static double
hartley(int *data,int size,int k)
    {
    int n;
    double total;
    double factor;
    
    factor = 2 * 3.141592653589 * k / (double) size;

    total = 0.0;
    for (n = 0; n < size; ++n)
        {
        double alpha = factor * n;
        total += data[n] * (cos(alpha) + sin(alpha));
        }

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
            case 'c':
                channel = atoi(arg);
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
