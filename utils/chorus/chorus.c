#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <assert.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/rra.h"
#include "/usr/local/include/songlib/wave.h"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "chorus";
char *PROGRAM_VERSION = "0.01";

static int processOptions(int, char **);
static void processData(FILE *,FILE *,RRA *);
static double getDatum(FILE *,FILE *,RRA *,double);
static double interp(int *,int,double);

static double AmpFactor = 2.00;
static double Range = 0.008;
static double Block = 1; //second
static double DelayFactor = 0.7;

static int ReadAhead = 5;

int
main(int argc,char **argv)
    {
    int argIndex = 1;

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
        printf("usage: chorus [<input rra file> [<output wave file>]]\n");
        exit(-1);
        }

    h = newRRAHeader();
    
    readRRAHeader(in,h,0);

    writeRRAHeader(out,h,"modifiedBy: chorus",0);

    processData(in,out,h);

    fclose(in);
    fclose(out);

    return 0;
    }

static void
processData(FILE *in,FILE *out,RRA *h)
    {
    int count;
    double t0,t1,t2,t3,t4;

    t0 = t1 = t2 = t3 = t4 = 0.0;

    double p0 = 1;
    double p1 = p0 + Range;
    double p2 = p1 + Range;
    double p3 = (1 / p1) * 0.99;
    double p4 = (1 / p2) * 1.01;

    int d0 = 0;
    int d1 = 127 * DelayFactor;
    int d2 = 480 * DelayFactor;
    int d3 = 623 * DelayFactor;
    int d4 = 852 * DelayFactor;

    int sblock = h->sampleRate * Block;

    count = 0;
    while (((int) (t0 + 0.5)) < h->samples)
        {
        int total = getDatum(in,out,h,t0 - d0)
              + getDatum(in,out,h,t1 - d1)
              + getDatum(in,out,h,t2 - d2)
              + getDatum(in,out,h,t3 - d3)
              + getDatum(in,out,h,t4 - d4)
              ;

        fprintf(out,"%d\n",(int)(total / 5.0 * AmpFactor));
        //fprintf(out,"%d\n",(int)a2);

        //see if it is time to switch the pitches of the voices
        if (count > 0 && count % (sblock + d0) == 0)
            p0 = 1 / p0;
        if (count > 0 && count % (sblock + d1) == 0)
            p1 = 1 / p1;
        if (count > 0 && count % (sblock + d2) == 0)
            p2 = 1 / p2;
        if (count > 0 && count % (sblock + d3) == 0)
            p3 = 1 / p3;
        if (count > 0 && count % (sblock + d4) == 0)
            p4 = 1 / p4;

        t0 += p0;
        t1 += p1;
        t2 += p2;
        t3 += p3;
        t4 += p4;

        ++count;
        }
    }

static double
getDatum(FILE *in,FILE *out,RRA *h,double t)
    {
    static int edge = -1;
    int ceiling;
    double result;
    
    if (t < 0) return 0;

    if (edge == -1) createRRAData(h);

    /* read in five or so samples ahead */

    ceiling = (int) (t + 0.5) + ReadAhead;

    if (ceiling >= h->samples) ceiling = h->samples - 1;

    while (edge < ceiling)
        {
        ++edge;
        h->data[0][edge] = 
            readRRAAmplitude(in,out,h->bitsPerSample,outputComment);
        }

    result = interp(h->data[0],h->samples,t);

    return result;
    }

static double
interp(int *data,int length,double t)
    {
    int lo = (int) t;
    int hi = lo + 1;
    int lolo = lo;
    int hihi = hi;
    double mid;
    double fraction;
    double result;

    assert (t >= 0);

    if (lo >= length-1) return data[length-1];

    if (lo > 0) --lolo;
    if (hi < length-1) ++hihi;

    fraction = t - lo;
    mid = (3.0 * data[lo] - data[lolo] + 3.0 * data[hi] - data[hihi]) / 4.0;

    if (fraction < 0.5)
        result = data[lo] + (mid - data[lo]) * 2 * fraction;
    else
        result = mid + (data[hi] - mid) * 2 * (fraction - 0.5);

    return result;
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
                AmpFactor = atof(arg);
                argUsed = 1;
                break;
            case 'b':
                Block = atof(arg);
                argUsed = 1;
                break;
            case 'd':
                DelayFactor = atof(arg);
                argUsed = 1;
                break;
            case 'r':
                Range = atof(arg);
                argUsed = 1;
                break;
            case 'R':
                ReadAhead = atoi(arg);
                argUsed = 1;
                break;
            case 'h':
                printf("chorus options:\n");
                printf("  -a N   set amplitude factor to N\n");
                printf("         values > 1 increase loudness\n");
                printf("         values < 1 decrease loudness\n");
                printf("         default is %f\n",AmpFactor);
                printf("  -b N   set Block size to N seconds\n");
                printf("         controls how long voice are unsynchronized\n");
                printf("         default is %f\n",Block);
                printf("  -d N   set DelayFactor to N\n");
                printf("         controls how late voices come in\n");
                printf("         larger values increase lateness\n");
                printf("         smaller values decrease lateness\n");
                printf("         default is %f\n",DelayFactor);
                printf("  -r N   set Range to N\n");
                printf("         controls how much voices are sharp or flat\n");
                printf("         default is %f\n",Range);
                printf("  -R N   set readAhead to N\n");
                printf("         controls how many samples are buffered\n");
                printf("         default is %d\n",ReadAhead);
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

