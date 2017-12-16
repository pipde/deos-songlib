#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <assert.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/rra.h"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "hiss";
char *PROGRAM_VERSION = "0.01";

static double attenuation = 1;
static double randomness = 0.05;
static double startDelay = 0;
static double hissLength = 0;
static double outSpeed = 0.99999;
static double inSpeed = 0;
//static int pops = 0;
//static int crackles = 0;

static double uniform(double);
static int processOptions(int,char **);

int
main(int argc,char **argv)
    {
    int argIndex = 1;

    int i,j;
    int amp;
    double size;
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
        printf("usage: hiss -sM.N [<input rra file> [<output rra file>]]\n");
        exit(-1);
        }

    h = newRRAHeader();
    
    readRRAHeader(in,h,0);
    writeRRAHeader(out,h,"modifiedBy: hiss",0);

    size = randomness * (pow(2.0,h->bitsPerSample - 1) - 1.0);
    fprintf(stderr,"randomness is %f\n",randomness);
    fprintf(stderr,"size is %f\n",size);

    for (i = 0; !feof(in); ++i)
        for (j = 0; j < h->channels; ++j)
            {
            fscanf(in," %d",&amp);
            amp += uniform(2 * size) - size;
            fprintf(out,"%d\n",(int)(amp * attenuation));
            }

    if (i != h->samples + 1)
        fprintf(stderr,"WARNING: %d samples found, %d expected\n",i,h->samples);

    fclose(in);
    fclose(out);

    return 0;
    }

static double
uniform(double limit)
    {
    return limit * rand() / RAND_MAX;
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
            case 'r':
                randomness = atof(arg);
                argUsed = 1;
                break;
            case 'v':
                printf("%s version %s\n", PROGRAM_NAME, PROGRAM_VERSION);
                exit(0);
                break;
            case 'h':
                printf("hiss options:\n");
                printf("  -a N   attenuates the output by N\n");
                printf("         values > 1 increase loudness\n");
                printf("         values < 1 decrease loudness\n");
                printf("         default is %f\n",attenuation);
                printf("  -f N   fade out after N seconds of hiss\n");
                printf("         default is %f\n",hissLength);
                printf("  -F N   controls the speed of the fade out\n");
                printf("         default is %f\n",outSpeed);
                printf("  -s N   fade in the hiss after N seconds\n");
                printf("         default is %f\n",startDelay);
                printf("  -S N   controls the speed of the fade in\n");
                printf("         default is %f\n",inSpeed);
                printf("  -r N   set the randomness of the hiss to N\n");
                printf("         default is %f\n",randomness);
                //printf("  -p     add pops\n");
                //printf("         default is no pops\n");
                //printf("  -c     add crackles\n");
                //printf("         default is no crackles\n");
                printf("  -v     display the version number\n");
                printf("  -h     help\n");
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

