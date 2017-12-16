#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/rra.h"

#define PI 3.141592653589

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "fhartley";
char *PROGRAM_VERSION = "0.01";

static int channel = 0;
static FILE *in,*out;

static void analyze(RRA *);
static void fht(double *,int);
static double hartley(double *,int,int);
static int processOptions(int,char **);

int
main(int argc,char **argv)
    {
    int argIndex = 1;

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
        printf("usage: fhartley -c<N> "
            "[<input rra file> [<output rra file>]]\n");
        exit(-1);
        }

    h = readRRA(in,0);

    analyze(h);

    fclose(in);
    fclose(out);

    return 0;
    }

static void
analyze(RRA *h)
    {
    int i;
    int samples;
    double *fdata;

    if (channel >= h->channels)
        Fatal("not enough channels (channel zero is the first channel)\n");

    samples = 1;
    while (samples * 2 <= h->samples)
        samples *= 2;

    fprintf(out,"RRAUDIO\n");
    fprintf(out,"createdBy: fhartley\n");
    fprintf(out,"channels: 1\n");
    fprintf(out,"samples: %d\n",samples);
    fprintf(out,"%%%%\n");

    fdata = (double *) New(sizeof(double) * samples);

    for (i = 0; i < samples; ++i)
        {
        fdata[i] = (double) h->data[channel][i];
        }

    /* Produce the hartley data here.  */

    fht(fdata,samples);

    for (i = 0; i < samples; ++i)
        {
        fprintf(out,"%f\t\t! frequency bin: %f\n",
            fdata[i],1.0 * i * h->sampleRate / (double) samples);
        }
    }

static void
fht(double *data,int size)
    {
    int i;
    double *evens,*odds;

    if (size == 1)
        {
        data[0] = hartley(data,1,0);
        return;
        }

    evens = (double *) New(sizeof(double) * size / 2);
    odds = (double *) New(sizeof(double) * size / 2);

    for (i = 0; i < size / 2; ++i)
        {
        evens[i] = data[i*2];
        odds[i] = data[i*2+1];
        }
    
    fht(evens,size/2);
    fht(odds,size/2);

    for (i = 0; i < size / 2; ++i)
        {
        double factor = 2 * PI * i / (double) size;

        data[i] = 0.5 *
            (evens[i]
                + odds[i] * cos(factor)
                + odds[(size-i) % (size/2)] * sin(factor));
        
        //printf("level %d: H[%d] = 1/2 * (H0[%d] + H1[%d]*cos(%f) + "
            //"H1[%d]*sin(%f))\n",
            //size,i,i,i,factor,(size-i)%(size/2),factor);
        //getchar();
        data[i + size / 2] = 0.5 *
            (evens[i]
                - odds[i] * cos(factor)
                - odds[(size-i) % (size/2)] * sin(factor));
        //printf("level %d: H[%d] = 1/2 * (H0[%d] - H1[%d]*cos(%f) - "
            //"H1[%d]*sin(%f))\n",
            //size,i+size/2,i,i,factor,(size-i)%(size/2),factor);
        //getchar();
        }
    free(evens);
    free(odds);
    }

static double
hartley(double *data,int size,int k)
    {
    int n;
    double total;
    double factor;
    
    factor = 2 * PI * k / (double) size;

    total = 0.0;
    for (n = 0; n < size; ++n)
        {
        double alpha = factor * n;
        total += data[n] * (cos(alpha) + sin(alpha));
        }

    return total;
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
                channel = atoi(arg);
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
