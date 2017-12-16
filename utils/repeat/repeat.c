#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/rra.h"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "repeat";
char *PROGRAM_VERSION = "0.01";

static int processOptions(int,char **);

int Repeats = 6;           //number of repetitions
double LengthFactor = 2.5; //spacing is Length * length of input

int
main(int argc,char **argv)
    {
    int argIndex = 1;

    int i,j,k,spacing,oldSampleCount;
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
        in = fopen(argv[argIndex],"r");
        out = stdout;
        }
    else if (argc-argIndex == 2)
        {
        in = OpenFile(argv[argIndex],"r");
        out = OpenFile(argv[argIndex+1],"w");
        }
    else
        {
        printf("usage: rraecho [<input rra file> [<output rra file>]]\n");
        exit(-1);
        }

    h = readRRA(in,0);
    oldSampleCount = h->samples;

    spacing = LengthFactor * h->samples;

    h->samples = Repeats * (h->samples + spacing);
    writeRRAHeader(out,h,"modifiedBy: repeat",0);

    for (i = 0; i < Repeats; ++i)
        {
        for (j = 0; j < oldSampleCount; ++j)
            {
            for (k = 0; k < h->channels; ++k)
                {
                fprintf(out,"%d\n",h->data[k][j]);
                }
            }
        for (j = 0; j < spacing; ++j)
            {
            for (k = 0; k < h->channels; ++k)
                {
                fprintf(out,"%d\n",0);
                }
            }
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
            case 'r':
                Repeats = atoi(arg);
                argUsed = 1;
                break;
            case 's':
                LengthFactor = atof(arg);
                argUsed = 1;
                break;
            case 'h':
                printf("%s usage:\n",PROGRAM_NAME);
                printf("  %s [OPTIONS] [INFILE [OUTFILE]]\n",PROGRAM_NAME);
                printf("%s options:\n",PROGRAM_NAME);
                printf("  -r N   set the number of repetitions to N\n");
                printf("         default value is %d\n",Repeats);
                printf("  -s N   set the space between repetitions to N * length of the input\n");
                printf("         default value is %f\n",LengthFactor);
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

