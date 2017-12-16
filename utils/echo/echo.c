#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/rra.h"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "rraecho";
char *PROGRAM_VERSION = "0.01";

static int processOptions(int,char **);

double Delay = 0.50; //seconds
double Length = 0; //seconds
double Attenuation = 0.5;

int
main(int argc,char **argv)
    {
    int argIndex = 1;

    int i;
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

    if (h->channels != 1) Fatal("rraecho only works on mono rra files");

    double slength;
    if (Length <= 0)
        slength = h->samples;
    else
        slength = Length * h->sampleRate;

    int sdelay = Delay * h->sampleRate;
    int olength = h->samples;

    h->samples = slength;
    writeRRAHeader(out,h,"modifiedBy: rraecho",0);

    for (i = 0; i < slength; ++i)
        {
        int j;
        int total = 0;
        double factor = 1;
        //printf("total is %d\n",total);
        for (j = 0; j < i / sdelay + 1; ++j)
            {
            if (i < olength + j * sdelay)
                total += h->data[0][i - j * sdelay] * factor;
            factor *= Attenuation;
            //printf("total now is %d\n",total);
            }
        fprintf(out,"%d\n",total);
        //if (i / sdelay > 1)
            //getchar();
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
            case 'a':
                Attenuation = atof(arg);
                argUsed = 1;
                break;
            case 'd':
                Delay = atof(arg);
                argUsed = 1;
                break;
            case 'l':
                Length = atof(arg);
                argUsed = 1;
                break;
            case 'h':
                printf("echo options:\n");
                printf("  -a N   set attenuation of the echo to N\n");
                printf("         default value is %f\n",Attenuation);
                printf("  -d N   set delay of the echo N seconds\n");
                printf("         default value is %f seconds\n",Delay);
                printf("  -l N   set the length of the resulting audio to N\n");
                printf("         default value is the length of the input\n");
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

