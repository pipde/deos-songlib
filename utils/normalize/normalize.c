#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>
#include <math.h>
#include <assert.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/rra.h"
#include "/usr/local/include/songlib/wave.h"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "normalize";
char *PROGRAM_VERSION = "0.01";

static double amplitude = .5;
static int stats = 0;

static int processOptions(int,char **);

int
main(int argc,char **argv)
    {
    int argIndex = 1;

    int i,j,amp,count;
    int largest,smallest;
    double scale;
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
        printf("usage: normalize [-h] [<input rra file> [<output rra file>]]\n");
        exit(-1);
        }

    h = readRRA(in,0);

    largest = h->data[0][0];
    smallest = h->data[0][0];

    count = 0;
    for (i = 0; i < h->samples; ++i)
        {
        for (j = 0; j < h->channels; ++j)
            {
            amp = h->data[j][i];
            if (amp > largest) { largest = amp;}
            if (amp < smallest) { smallest = amp;}
            }
        ++count;
        }

    if (largest > -smallest)
        scale = pow(2,h->bitsPerSample - 1) / largest * amplitude;
    else
        scale = pow(2,h->bitsPerSample - 1) / -smallest * amplitude;

    fprintf(stderr,"normalize: largest value is %d\n",largest);
    fprintf(stderr,"normalize: smallest value is %d\n",smallest);
    fprintf(stderr,"normalize: scale is %f\n",scale);
    if (stats) exit(0);

    h->samples = count;

    for (i = 0; i < h->samples; ++i)
        for (j = 0; j < h->channels; ++j)
            {
            h->data[j][i] *= scale;
            if (h->data[j][i] > INT_MAX - 1)
                fprintf(stderr,"normalize: overflow\n");
            else if (h->data[j][i] < -INT_MAX + 1)
                fprintf(stderr,"normalize: underflow\n");
            }

    writeRRA(out,h,"modifiedBy: normalize",0,h->bitsPerSample == 8 ? 127:0);

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
            case 'a':
                amplitude = atof(arg);
                argUsed = 1;
                break;
            case 's':
                stats = 1;
                break;
            case 'h':
                printf("normalize options:\n");
                printf("    -a N   set max level to to <N>\n");
                printf("           good values are 0.8 to 0.9\n");
                printf("    -s     just report statistics (no conversion)\n");
                printf("    -h     help\n");
                exit(0);
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

