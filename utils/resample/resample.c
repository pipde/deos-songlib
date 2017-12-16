#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/rra.h"
#include "/usr/local/include/songlib/songlib.h"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "resample";
char *PROGRAM_VERSION = "0.01";

static int processOptions(int, char **);
int Rate = 44100;

int
main(int argc,char **argv)
    {
    int argIndex = 1;

    FILE *in,*out;
    RRA *h;

    argIndex = processOptions(argc,argv);

    if (argc-argIndex == 1)
        {
        Rate = atof(argv[argIndex]);
        in = stdin;
        out = stdout;
        }
    else if (argc-argIndex == 2)
        {
        Rate = atof(argv[argIndex]);
        in = OpenFile(argv[argIndex+1],"r");
        out = stdout;
        }
    else if (argc-argIndex == 3)
        {
        Rate = atof(argv[argIndex]);
        in = OpenFile(argv[argIndex+1],"r");
        out = OpenFile(argv[argIndex+2],"w");
        }
    else
        {
        printf("usage: %s sampleRate [<input rra file> [<output rra file>]]\n",
            argv[0]);
        exit(-1);
        }

    setInterp(3);

    fprintf(stderr,"new sample rate is %d\n",Rate);
    if (Rate == 0) exit(1);

    RRA *orig = readRRA(in,0);

    double factor = orig->sampleRate / (double) Rate;

    h = resample(orig,factor);

    h->sampleRate = Rate;

    writeRRA(out,h,"modifiedBy: resample",0,0);

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

    (void) arg;
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
            case 'h':
                printf("usage: rraresample sampleRate [<input rra file> [<output rra file>]]\n");
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

