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

char *PROGRAM_NAME = "fixwrap";
char *PROGRAM_VERSION = "0.01";

static double Threshold = 0.50;
static int Checking = 0;
static int Min;
static int Max;

static int processOptions(int,char **);

int
main(int argc,char **argv)
    {
    int argIndex = 1;

    int i,j,k,wraparounds = 0;
    int start,finish;
    FILE *in,*out;
    RRA *r;

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
        printf("usage: fixclip [-h] [<input rra file> [<output rra file>]]\n");
        exit(-1);
        }

    r = readRRA(in,0);
    fclose(in);

    Min = (int) -pow(2,r->bitsPerSample-1);
    Max = (int) pow(2,r->bitsPerSample-1) - 1;

    //printf("Min is %d\n",Min);
    //printf("Max is %d\n",Max);

    for (j = 0; j < r->channels; ++j)
        {
        //find start of wraparound
        i = 0; 
        while (1)
            {
            int prev = 0,amp = 0;
            while (i < r->samples)
                {
                if ((prev > Max * Threshold && amp < 0)
                 || (prev < Min * Threshold && amp > 0))
                    break;
                prev = amp;
                amp = r->data[j][i++];
                }

            if (i == r->samples) break;

            i -= 2; //went two steps too far

            start = i;
            //printf("start is %d, amp is %d\n",start,r->data[j][start]);

            //find the end
            ++i;
                    
            if (r->data[j][start] > 0)
                while (i < r->samples && r->data[j][i] < 0) ++i;
            else
                while (i < r->samples && r->data[j][i] > 0) ++i;

            finish = i+1;

            //for (k = start; k < finish; ++k)
            //    printf("    wrap: %d\n",r->data[j][k]);

            ++wraparounds;

            if (Checking)
                {
                fprintf(stderr,"wraparound from %d (%.4f) to %d (%.4f), "
                    "%d samples\n",
                    start,start / (double) r->sampleRate,
                    finish,finish / (double) r->sampleRate,
                    finish-start
                    );
                //getchar();
                }
            else
                {
                /* invert the region above the clipping threshold */
                if (r->data[j][start] > 0)
                    for (k = start+1; k < finish-1; ++k)
                        r->data[j][k] = Max + r->data[j][k] - Min;
                else
                    for (k = start+1; k < finish-1; ++k)
                        r->data[j][k] = Min + r->data[j][k] - Max;
                //printf("after inversion\n");
                //for (k = start; k < finish; ++k)
                //    printf("    wrap: %d\n",r->data[j][k]);
                //getchar();
                }
            }
        }
                
    if (!Checking)
        writeRRA(out,r,"modifiedBy: fixclip",0,r->bitsPerSample == 8 ? 127:0);

    fclose(out);

    return wraparounds;
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
                Checking = !Checking;
                break;
            case 't':
                Threshold = atof(arg);
                argUsed = 1;
                break;
            case 'h':
                printf("fixclip options:\n");
                printf("    -c     just check if wraparounds are present\n");
                printf("           wraparounds reported on stderr\n");
                printf("           return value is non-zero if wraparounds present\n");
                printf("    -t 0.N set the threshold factor to 0.N\n");
                printf("           default is %f\n",Threshold);
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

