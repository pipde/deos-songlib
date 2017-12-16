#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/rra.h"
#include "/usr/local/include/songlib/wave.h"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "rradata";
char *PROGRAM_VERSION = "0.01";

static int processOptions(int, char **);

int
main(int argc,char **argv)
    {
    int argIndex = 1;

    int i,amp;
    char *input;
    FILE *in;
    RRA *h;

    argIndex = processOptions(argc,argv);

    input = 0;
    if (argc-argIndex == 0)
        {
        in = 0;
        }
    else if (argc-argIndex == 1)
        {
        input = argv[argIndex];
        }
    else
        {
        printf("usage: rradata [<input rra file> [<output wave file>]]\n");
        exit(-1);
        }

    if (input != 0)
        in = OpenFile(input,"r");

    h = newRRAHeader();

    readRRAHeader(in,h,0);

    for (i = 0; !feof(in) && i < h->samples * h->channels; ++i)
        {
        amp = readRRAAmplitude(in,stdout,h->bitsPerSample,outputComment);
        printf("%d\n",amp);
        }

    fclose(in);

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
                printf("%s\n",arg);
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

