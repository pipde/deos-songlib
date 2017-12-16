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

char *PROGRAM_NAME = "fixclip";
char *PROGRAM_VERSION = "0.01";

static int DipFactor = 20;
static int Checking = 0;
static int Width = 9;
static int Smoothings = 1;
static int Min;
static int Max;

static int processOptions(int,char **);

static void
dip(RRA *r,int channel,int start,int finish)
    {
    int mid = (start + finish) / 2;
    if (r->data[channel][mid] > 0)
        {
        r->data[channel][mid] =
            r->data[channel][mid] - DipFactor * (finish - start);
        assert(r->data[channel][mid] <= Max);
        }
    else
        {
        r->data[channel][mid] =
            r->data[channel][mid] + DipFactor * (finish - start);
        assert(r->data[channel][mid] >= Min);
        }
    }

static void
smooth(RRA *r,int channel,int start,int finish,int pass)
    {
    int i,j;
    int begin = start - Width;
    int end = finish + Width;
    int width = Width * pass;

    //printf("clipping from %d to %d\n",start,finish);
    //printf("smoothing from %d to %d\n",begin,end);
    //for (i = begin; i < end; ++i)
    //    printf("before: %d is %d\n",i,r->data[channel][i]);

    if (begin < 0) begin = 0;
    if (end > r->samples) end = r->samples;

    for (i = begin; i < finish; ++i) //yes, I mean finish, not end
        {
        double total = 0.0;
        int weight = abs((start+finish) / 2 - (i + width/2)) + 1;
        //printf("weight is %d\n",weight);
        for (j = 0; j < width; ++j)
            {
            if (j == width/2)
                total += r->data[channel][i+j] * weight;
            else
                total += r->data[channel][i+j];
            }
        int divisor = width - 1 + weight;
        //printf("average for %d is %d, divisor is %d\n",
        //    i+width/2,(int)(total / divisor),divisor);
        r->data[channel][i+width/2] = (int)(total / divisor);
        //if (r->data[channel][i+width/2] > Max)
        //    {
        //    printf("Max is %d, smoothed amp is %d, weight is %d\n",
        //        Max,r->data[channel][i+width/2],weight);
        //    for (j = 0; j < width; ++j)
        //        printf("    amp[%d] is %d\n",i+j,r->data[channel][i+j]);
        //    exit(-1);
        //    }
        }

    //for (i = begin; i < end; ++i)
    //    printf("after: %d is %d\n",i,r->data[channel][i]);
    //getchar();
    }

static int
tooMuch(int a)
    {
    return a >= Max || a <= Min;
    }

int
main(int argc,char **argv)
    {
    int argIndex = 1;

    int i,j,k,clipped = 0;
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
        //find start of clip
        i = 0; 
        while (1)
            {
            int amp = 0;
            while (i < r->samples && amp < Max && amp > Min)
                amp = r->data[j][i++];

            if (i == r->samples) break;

            --i; //went one step too far

            start = i;
            //printf("start is %d, amp is %d\n",start,r->data[j][start]);

            //find end of clip
            int extreme = r->data[j][start];
            int amp0 = i+0 < r->samples? r->data[j][i+0] : 0;
            int amp1 = i+1 < r->samples? r->data[j][i+1] : 0;
            int amp2 = i+2 < r->samples? r->data[j][i+2] : 0;
            int amp3 = i+3 < r->samples? r->data[j][i+3] : 0;
            int amp4 = i+4 < r->samples? r->data[j][i+4] : 0;
            while (tooMuch(amp0) || tooMuch(amp1) || tooMuch(amp2)
            || tooMuch(amp3) || tooMuch(amp4))
                {
                //printf("sample %d: %d\n",i,amp0);
                amp0 = amp1;
                amp1 = amp2;
                amp2 = amp3;
                amp3 = amp4;
                ++i;
                amp4 = i+4 < r->samples? r->data[j][i+4] : 0;
                }
            //printf("next sample %d: %d\n",i,r->data[j][i]);

            finish = i;

            ++clipped;

            if (Checking)
                {
                fprintf(stderr,"clipped %s from %d (%.4f) to %d (%.4f), "
                    "%d samples\n",
                    extreme == Min? "low" : "high",
                    start,start / (double) r->sampleRate,
                    finish,finish / (double) r->sampleRate,
                    finish-start
                    );
                }
            else
                {
                /* invert any region above the clipping threshold */
                for (k = start; k < finish; ++k)
                   {
                   if (r->data[j][k] > Max)
                       r->data[j][k] = 2 * Max - r->data[j][k];
                   else if (r->data[j][k] < Min)
                       r->data[j][k] = 2 * Min + r->data[j][k];
                    }
                /* move the mid point of the clipping region down */
                dip(r,j,start,finish);
                /* now smooth out the changes */
                for (k = 0; k < Smoothings; ++k)
                    smooth(r,j,start,finish,k+1);
                }
            }
        }
                
    if (!Checking)
        writeRRA(out,r,"modifiedBy: fixclip",0,r->bitsPerSample == 8 ? 127:0);

    fclose(out);

    return clipped;
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
            case 'd':
                DipFactor = atoi(arg);
                argUsed = 1;
                break;
            case 's':
                Smoothings = atoi(arg);
                argUsed = 1;
                break;
            case 'h':
                printf("fixclip options:\n");
                printf("    -c     just check if clips are present\n");
                printf("           clips reported on stderr\n");
                printf("           return value is non-zero if clips present\n");
                printf("    -d N   set the dip factor to N\n");
                printf("           default is %d\n",DipFactor);
                printf("    -h     help\n");
                printf("    -s N   set the number of smoothing passes to N\n");
                printf("           default is %d\n",Smoothings);
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

