#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <assert.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/rra.h"
#include "/usr/local/include/songlib/wave.h"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "reverb";
char *PROGRAM_VERSION = "0.01";

static int processOptions(int, char **);
static void processData(FILE *,FILE *,RRA *);
static double getDatum(int,double);
static double interp(int *,int,double);
static double reverb(int,int);

#define numberOfEchos 4
static double attenuations[numberOfEchos] = { 0.3, 0.1, 0.075 , 0.03};
static double delays[numberOfEchos] = { 0.03, 0.075, 0.105, 0.135 };

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
	printf("usage: reverb -a<N> -d<N> "
	    "[<input rra file> [<output wave file>]]\n");
	exit(-1);
	}

    h = newRRAHeader();
    
    readRRAHeader(in,h,0);

    writeRRAHeader(out,h,"modifiedBy: reverb",0);

    processData(in,out,h);

    fclose(in);
    fclose(out);

    return 0;
    }

static void
processData(FILE *in,FILE *out,RRA *h)
    {
    int i,j;
    int amp;

    //convert delay from seconds to samples

    for (i = 0; i < h->samples; ++i)
	for (j = 0; j < h->channels; ++j)
	    {
	    amp = (int) reverb(j,i);
	    h->data[j][i] = amp;
	    fprintf(out,"%d\n",amp);
	    }
    }

static double
getDatum(int channel,double t)
    {
    static int edge = -1;
    int i,ceiling;
    double result;
    
    if (t < 0) return 0;

    if (edge == -1) createRRAData(h);

    /* read in five samples ahead */

    ceiling = (int) (t + 0.5) + 5;

    if (ceiling >= h->samples) ceiling = h->samples - 1;

    //printf("channel %d, time %f: edge is %d while ceiling is %d\n",channel,
        //t,edge,ceiling);

    while (edge < ceiling)
        {
	++edge;
	for (i = 0; i < h->channels; ++i)
	    {
	    h->data[i][edge] = readRRAAmplitude(in,h->bitsPerSample,0);
	    }
	}

    result = interp(h->data[channel],h->samples,t);

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

static double
reverb(int channel,int t)
    {
    int i;
    double datum;

    datum = getDatum(channel,t);
    for (i = 0; i < numberOfEchos; ++i)
	datum += attenuations[i]
	    * getDatum(channel,t - h->sampleRate * delays[i]);

    return datum;
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
	     *	       PrintActions = 1;
	     *         break;
	     */
	    case 'v':
	        printf("%s version %s\n", PROGRAM_NAME, PROGRAM_VERSION);
		exit(0);
		break;
	    case 'a':
		attenuations[0] = atof(arg);
		argUsed = 1;
		break;
	    case 'A':
		delays[0] = atof(arg);
		argUsed = 1;
		break;
	    case 'b':
		attenuations[1] = atof(arg);
		argUsed = 1;
		break;
	    case 'B':
		delays[1] = atof(arg);
		argUsed = 1;
		break;
	    case 'c':
		attenuations[2] = atof(arg);
		argUsed = 1;
		break;
	    case 'C':
		delays[2] = atof(arg);
		argUsed = 1;
		break;
	    case 'd':
		attenuations[3] = atof(arg);
		argUsed = 1;
		break;
	    case 'D':
		delays[3] = atof(arg);
		argUsed = 1;
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

