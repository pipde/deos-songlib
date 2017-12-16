#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/songlib.h"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "filter";
char *PROGRAM_VERSION = "0.01";

/* filter designer at http://cisco.cscs.wmin.ac.uk/filter_design.html */

static void readCoefficients(char *);
static double firFilter(double,double *,double *,int,int *);
static int processOptions(int,char **);

#define MAX_TAPS 1024
double coefficients[MAX_TAPS];
double totalWeight;
int numberOfTaps;
char *coefficientFile = "coefficients";
double attenuation = 0;

int
main(int argc,char **argv)
    {
    int argIndex = 1;

    int i,j,index;
    double max,**last;
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
	fprintf(stderr,"usage: flanger -h -a<N.M> -w<N.M> -d<N.M> -l<N.M> "
	    "[<input rra file> [<output rra file>]]\n");
	exit(-1);
	}

    readCoefficients(coefficientFile);

    if (attenuation == 0) attenuation = 1.0 / totalWeight;

    h = newRRAHeader();
    
    readRRAHeader(in,h,0);

    writeRRAHeader(out,h,"modifiedBy: filter",0);

    max = pow(2,h->bitsPerSample - 1) - 1;

    last = (double **) New(sizeof(double *) * h->channels);
    for (i = 0; i < h->channels; ++i)
	{
        last[i] = (double *) New(sizeof(double) * numberOfTaps);
	last[i][0] = readRRAAmplitude(in,h->bitsPerSample,0) / max;
	fprintf(out,"%f\n",attenuation * last[i][0]);
	}

    for (i = 0; i < h->channels; ++i)
	for (j = 1; j < numberOfTaps; ++j)
	    last[i][j] = last[i][0];

    index = 0;
    for ( ; i < h->samples; ++i)
	for (j = 0; j < h->channels; ++j)
	    {
	    int amp = readRRAAmplitude(in,h->bitsPerSample,0);
	    fprintf(out,"%f\n",
		attenuation *
		    firFilter(amp / max,last[j],
		        coefficients,numberOfTaps,&index));
	    }

    fclose(in);
    fclose(out);

    return 0;
    }

static void
readCoefficients(char *fileName)
    {
    FILE *fp;
    double coeff;

    fprintf(stderr,"reading coefficients: %s\n",fileName);

    fp = OpenFile(fileName,"r");

    totalWeight = 0;
    numberOfTaps = 0;
    while (fscanf(fp," %lf",&coeff) != -1)
        {
	if (numberOfTaps >= MAX_TAPS)
	    Fatal("filter: too many coefficients!\n");
	coefficients[numberOfTaps] = coeff;
	totalWeight += coeff;
	++numberOfTaps;
	}

    fprintf(stderr,"total coefficient weight: %f\n",totalWeight);
    }


static double
firFilter(double value,double *buffer,double *coeffs,int length,int *index)
    {
    int i;
    double total;

    buffer[*index] = value;

    total = 0; 
    for (i = 0; i < length; i++) 
	{ 
	total += coeffs[i] * buffer[(*index + i) % length]; 
	} 
    *index = (*index + 1) % length; 

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

    while (argIndex < argc && *argv[argIndex] == '-')
        {
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
	        attenuation = atof(arg);
		argUsed = 1;
		break;
	    case 'f':
	        coefficientFile = arg;
		argUsed = 1;
		break;
	    case 'h':
		printf("filter options:\n");
		printf("  -h       help\n");
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

