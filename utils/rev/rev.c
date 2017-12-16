#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/rra.h"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "rev";
char *PROGRAM_VERSION = "0.01";

static int *data;
static double seconds = 0;
static int spot = 0;
static int max = 0;

static int isAvailable(FILE *);
static void store(int);
static int retrieve(void);
static int processOptions(int,char **);

int
main(int argc,char **argv)
    {
    int argIndex = 1;

    int i,j;
    int amp;
    char *source;
    FILE *in,*out;
    RRA *h;

    argIndex = processOptions(argc,argv);

    if (argc-argIndex == 0)
        {
        in = stdin;
        source = "stdin";
        out = stdout;
        }
    else if (argc-argIndex == 1)
        {
        in = OpenFile(argv[argIndex],"r");
        source = argv[argIndex];
        out = stdout;
        }
    else if (argc-argIndex == 2)
        {
        in = OpenFile(argv[argIndex],"r");
        source = argv[argIndex];
        out = OpenFile(argv[argIndex+1],"w");
        }
    else
        {
        printf("usage: rev -sN.M [<input rra file> [<output rra file>]]\n");
        exit(-1);
        }

    h = newRRAHeader();
    
    readRRAHeader(in,h,0);

    if (seconds == 0.0 && max == 0)
        max = 440;
    else if (max == 0)
        max = (int) (seconds * h->sampleRate);

    fprintf(stderr,"rev width: %d samples\n",max);

    data = malloc(sizeof(int) * max);

    writeRRAHeader(out,h,"modifiedBy: rev",0);

    for (i = 0; i < h->samples; ++i)
        for (j = 0; j < h->channels; ++j)
            {
            amp = readRRAAmplitude(in,h->bitsPerSample,0);
            store(amp);
            while (isAvailable(in))
                fprintf(out,"%d\n",retrieve());
            }

    while (isAvailable(in))
        fprintf(out,"%d\n",retrieve());

    fclose(in);
    fclose(out);

    return 0;
    }

static void
store(int amp)
    {
    data[spot++] = amp;
    }

static int
retrieve()
    {
    return data[--spot];
    }

static int
isAvailable(FILE *in)
    {
    static int available = 0;

    if (spot == max || feof(in))
        available = 1;
    else if (spot == 0)
        available = 0;

    return available;
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
	    case 's':
            seconds = atof(arg);
            argUsed = 1;
            break;
	    case 'S':
            max = atoi(arg);
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

