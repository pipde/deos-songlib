#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <assert.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/songlib.h"
#include "/usr/local/include/songlib/rra.h"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "reverb2";
char *PROGRAM_VERSION = "0.01";

static int processOptions(int, char **);
static void processData(FILE *,FILE *,RRA *);

static int stages = 5;
static int delay = 25;
static double a = 2;
static double g = 0.8;
//circular buffer size
static int size;

static int ***s;

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
    delay = delay * h->sampleRate / 1000;

    size = 2 * stages * delay;

    processData(in,out,h);

    fclose(in);
    fclose(out);

    return 0;
    }

static void
processData(FILE *in,FILE *out,RRA *h)
    {
    int i,j,k;

    //convert delay from seconds to samples

    s = (int ***) New(sizeof(int **) * stages);

    for (k = 0; k < stages; ++k)
        {
        s[k] = (int **) New(sizeof(int *) * h->channels);
        for (j = 0; j < h->channels; ++j)
            {
            s[k][j] = (int *) New(sizeof(int) * size);
            for (i = 0; i < size; ++i)
               {
               s[k][j][i] = 0;
               }
            }
        }

    for (i = 0; i < h->samples; ++i)
        for (j = 0; j < h->channels; ++j)
            {
            int p = i % size;
            int q = (i - delay + size) % size;
            int amp = readRRAAmplitude(in,out,h->bitsPerSample,outputComment);

            s[0][j][p] = amp;
            for (k = 1; k < stages; ++k)
                s[k][j][p] = -g * s[k-1][j][p] +
                    (1 - g*g) * (s[k-1][j][q] + g * s[k][j][q]);
            //  s5[j][k] = -g * s4[j][k] +
            //      (1 - g*g) * (s4[j][m] + g * s5[j][m]);

            fprintf(out,"%d\n",(int) (s[stages-1][j][p] * a));
            }
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
                a = atof(arg);
                argUsed = 1;
                break;
            case 'd':
                delay = atoi(arg);
                argUsed = 1;
                break;
            case 'g':
                g = atof(arg);
                argUsed = 1;
                break;
            case 's':
                stages = atoi(arg);
                argUsed = 1;
                break;
            default:
                printf("reverb options:\n");
                printf("  -a N.N  set the input gain to N.N\n");
                printf("          default is %f\n",a);
                printf("  -g N.N  set reverb loudness decay to N.N\n");
                printf("          good values are 0.4 to 0.8\n");
                printf("          higher values need higher -a\n");
                printf("          default is %f\n",g);
                printf("  -d N    set reverb delay to N milliseconds\n");
                printf("          good values are around 25 ms\n");
                printf("          default is %d\n",delay);
                printf("  -s N    set the number of stages to N\n");
                printf("          good values are around 5\n");
                printf("          the more stages, the higher -a needs to be\n");
                printf("          default is %d\n",stages);
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

