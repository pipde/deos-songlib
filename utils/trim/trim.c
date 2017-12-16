#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <assert.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/rra.h"
#include "/usr/local/include/songlib/wave.h"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "normalize";
char *PROGRAM_VERSION = "0.01";

static int processOptions(int,char **);

static int span;

int
main(int argc,char **argv)
    {
    int argIndex = 1;

    int i,start,end;
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
        printf("usage: trim [-h] [<input rra file> [<output rra file>]]\n");
        exit(-1);
        }

    h = readRRA(in,0);

    if (h->channels > 1)
        Fatal("trim: mono files only\n");

    start = 0;
    for (i = 0; i < h->samples; ++i)
        {
        if (h->data[0][i] > span || h->data[0][i] < -span) break;
        ++start;
        }

    fprintf(stderr,"trim: removing %d samples at start\n",start);

    end = 0;
    for (i = h->samples - 1; i >= 0; --i)
        {
        if (h->data[0][i] > span || h->data[0][i] < -span) break;
        ++end;
        }

    fprintf(stderr,"trim: removing %d samples at end\n",end);

    h->samples -= start + end;

    writeRRAHeader(out,h,"modifiedBy: trrim",0);

    for (i = start; i < h->samples + start; ++i)
        fprintf(out,"%d\n",h->data[0][i]);

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
            case 's':
                span = abs(atoi(arg));
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

