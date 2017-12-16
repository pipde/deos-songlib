/* generic filter for resampling
 *
 *identify in.rra
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <assert.h>
#include <limits.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/wave.h"
#include "/usr/local/include/songlib/rra.h"

//#define WRITE_ERROR 1

#define OFFSET  0.15
#define WINDOW  2000
#define SPAN    4

#define STEP 1.0594630944

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "identify";
char *PROGRAM_VERSION = "0.01";

static double initialOffset = OFFSET;
static int Pitch = 0;
static int Note = 0;
static int Frequency = 0;
static double MinFrequency = 13.75;
static double MaxFrequency = 3520.0;
static double ErrorPercent = 98.5;
static int skip;

static double processData(FILE *,RRA *);
static int processOptions(int, char **);
static int correlate(int *,int,int,double *);
static int findMinError(double *,int,double *);
static int findMinErrorAt(double *,int);
#ifdef WRITE_ERROR
static void writeTempRRA(double *,int,int);
#endif

double freqs[] =
    {
    13.75,
    13.75 * STEP,
    13.75 * STEP*STEP,
    13.75 * STEP*STEP*STEP,
    13.75 * STEP*STEP*STEP*STEP,
    13.75 * STEP*STEP*STEP*STEP*STEP,
    13.75 * STEP*STEP*STEP*STEP*STEP*STEP,
    13.75 * STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    13.75 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    13.75 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    13.75 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    13.75 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    27.5,
    27.5 * STEP,
    27.5 * STEP*STEP,
    27.5 * STEP*STEP*STEP,
    27.5 * STEP*STEP*STEP*STEP,
    27.5 * STEP*STEP*STEP*STEP*STEP,
    27.5 * STEP*STEP*STEP*STEP*STEP*STEP,
    27.5 * STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    27.5 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    27.5 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    27.5 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    27.5 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    55,
    55 * STEP,
    55 * STEP*STEP,
    55 * STEP*STEP*STEP,
    55 * STEP*STEP*STEP*STEP,
    55 * STEP*STEP*STEP*STEP*STEP,
    55 * STEP*STEP*STEP*STEP*STEP*STEP,
    55 * STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    55 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    55 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    55 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    55 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    110,
    110 * STEP,
    110 * STEP*STEP,
    110 * STEP*STEP*STEP,
    110 * STEP*STEP*STEP*STEP,
    110 * STEP*STEP*STEP*STEP*STEP,
    110 * STEP*STEP*STEP*STEP*STEP*STEP,
    110 * STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    110 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    110 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    110 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    110 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    220,
    220 * STEP,
    220 * STEP*STEP,
    220 * STEP*STEP*STEP,
    220 * STEP*STEP*STEP*STEP,
    220 * STEP*STEP*STEP*STEP*STEP,
    220 * STEP*STEP*STEP*STEP*STEP*STEP,
    220 * STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    220 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    220 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    220 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    220 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    440,
    440 * STEP,
    440 * STEP*STEP,
    440 * STEP*STEP*STEP,
    440 * STEP*STEP*STEP*STEP,
    440 * STEP*STEP*STEP*STEP*STEP,
    440 * STEP*STEP*STEP*STEP*STEP*STEP,
    440 * STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    440 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    440 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    440 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    440 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    880,
    880 * STEP,
    880 * STEP*STEP,
    880 * STEP*STEP*STEP,
    880 * STEP*STEP*STEP*STEP,
    880 * STEP*STEP*STEP*STEP*STEP,
    880 * STEP*STEP*STEP*STEP*STEP*STEP,
    880 * STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    880 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    880 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    880 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    880 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    1760,
    1760 * STEP,
    1760 * STEP*STEP,
    1760 * STEP*STEP*STEP,
    1760 * STEP*STEP*STEP*STEP,
    1760 * STEP*STEP*STEP*STEP*STEP,
    1760 * STEP*STEP*STEP*STEP*STEP*STEP,
    1760 * STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    1760 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    1760 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    1760 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    1760 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    3520,
    3520 * STEP,
    3520 * STEP*STEP,
    3520 * STEP*STEP*STEP,
    3520 * STEP*STEP*STEP*STEP,
    3520 * STEP*STEP*STEP*STEP*STEP,
    3520 * STEP*STEP*STEP*STEP*STEP*STEP,
    3520 * STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    3520 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    3520 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    3520 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    3520 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    };

char *notes[] =
    {
    "ax", "a#x", "bx", "c0", "c#0", "d0", "d#0", "e0", "f0", "f#0", "g0", "g#0",
    "a0", "a#0", "b0", "c1", "c#1", "d1", "d#1", "e1", "f1", "f#1", "g1", "g#1",
    "a1", "a#1", "b1", "c2", "c#2", "d2", "d#2", "e2", "f2", "f#2", "g2", "g#2",
    "a2", "a#2", "b2", "c3", "c#3", "d3", "d#3", "e3", "f3", "f#3", "g3", "g#3",
    "a3", "a#3", "b3", "c4", "c#4", "d4", "d#4", "e4", "f4", "f#4", "g4", "g#4",
    "a4", "a#4", "b4", "c5", "c#5", "d5", "d#5", "e5", "f5", "f#5", "g5", "g#5",
    "a5", "a#5", "b5", "c6", "c#6", "d6", "d#6", "e6", "f6", "f#6", "g6", "g#6",
    "a6", "a#6", "b6", "c7", "c#7", "d7", "d#7", "e7", "f7", "f#7", "g7", "g#7",
    "a7", "a#7", "b7", "c8", "c#8", "d8", "d#8", "e8", "f8", "f#8", "g8", "g#8",
    "a8", "a#8", "b8", "c9", "c#9", "d9", "d#9", "e9", "f9", "f#9", "g9", "g#9",
    "a9", "a#9", "b9", "c10", "c#10", "d10", "d#10", "e10", "f10", "f#10", "g10", "g#10",
    };

int
main(int argc,char **argv)
    {
    int argIndex = 1;

    int i,index;
    double freq;
    FILE *in,*out;
    char *source = 0;
    RRA *r;

    argIndex = processOptions(argc,argv);

    if (argc-argIndex == 0)
        {
        in = stdin;
        out = stdout;
        }
    else if (argc-argIndex == 1)
        {
        source = argv[argIndex];
        in = OpenFile(source,"r");
        out = stdout;
        }
    else if (argc-argIndex == 2)
        {
        source = argv[argIndex];
        in = OpenFile(source,"r");
        out = OpenFile(argv[argIndex+1],"w");
        }
    else
        {
        printf("usage: %s [<input rra file> [<output wave file>]]\n",argv[0]);
        exit(-1);
        }

    r = readRRA(in,0);
    
    if (r->channels != 1)
        Fatal("%s only works on mono audio files\n",argv[0]);

    freq = processData(out,r);

    index = -1;
    for (i = 0; index < 0 && i < 120; ++i)
        {
        if (freq >= freqs[i] && freq < freqs[i+1])
            {
            if (freq - freqs[i] >= freqs[i+1] - freq)
                index = i+1;
            else
                index = i;
            }
        }

    if (Pitch && source && index < 0)
        printf("echo rraidentity failed to identify %s\n",source);
    else if (Pitch && source != 0)
        printf("rratune %f %s %s.repitch\n",
            freqs[index] / freq,source,source);
    else if (Pitch && index < 0)
        printf("echo rraidentity failed to identify %s\n",source);
    else if (Pitch)
        printf("rratune %f\n", freqs[index] / freq);
    else if (index < 0)
        printf("rraidentity failed to identify %s\n",source);
    else if (Note)
        printf("%s\n", notes[index]);
    else if (Frequency)
        printf("%f\n", freq);
    else
        printf("closest note to %s (freq = %f) is %s (freq = %f)\n"
            "error is %.2f%%, factor is %f\n",
            source,freq, notes[index],freqs[index],
            (1 - freqs[index] / freq) * 100, freqs[index] / freq
            );
            
    fclose(in);
    fclose(out);

    return 0;
    }

static double
processData(FILE *out,RRA *r)
    {
    int i,min,limit;
    int length,window;
    double freq = 0;
    int *data;
    double *errors;
    int count;
    double first,firstDivisor,threshold,worst;

    window = r->sampleRate / MinFrequency;
    length = window * SPAN;
    skip = (r->sampleRate / MaxFrequency) / 2;
    //printf("skip is %d\n",skip);

    errors = (double *) New(sizeof(double) * length);

    //printf("initial offset: %f\n",initialOffset); 

    data = r->data[0] + (int) (initialOffset * r->sampleRate);

    count = correlate(data,length,window,errors);

#ifdef WRITE_ERROR
    writeTempRRA(errors,count,0);
#endif

    limit = findMinError(errors,count,&worst);
    //printf("best is %f at %d, worst is %f\n",errors[limit],limit,worst);
    //printf("error span is %f\n",worst - errors[limit]);
    //getchar();

    threshold = worst - 0.01 * ErrorPercent * (worst - errors[limit]);

    //printf("limit is %d\n",limit);

    freq = r->sampleRate * 1.0 / limit;
    //printf("frequency estimate is %f\n",freq);
    first = freq;
    firstDivisor = 1;

    i = 2;
    while (limit / i > skip * 2)
        {
        min = findMinErrorAt(errors,limit / i);
        //printf("for divisor %d, min found at %d\n",i,min);
        //printf("error is %f, theshold is %f\n",errors[min],threshold);
        if (min > skip && errors[min] < threshold)
            {
            freq = r->sampleRate / (double) min;
            //printf("better frequency found: %f!\n",freq);
            firstDivisor = i;
            }
        ++i;
        }

    return first * firstDivisor;
    }

static int
findMinError(double *errors,int length,double *worst)
    {
    int i;
    int min;

    /* find the first peak of the error function */

    //printf("findMinError: looking from %d to %d\n",skip,length);

    *worst = 0;

    for (i = skip; i < length; ++i)
        {
        if (errors[i] > *worst) *worst = errors[i];
        if (errors[i] < errors[i-1]) break;
        }

    /* starting the downhill side of the first peak */
    /* now find the minumum */

    //printf("searching from min starting from %d\n",i);
    min = i;
    for ( ; i < length; ++i)
        {
        if (errors[i] > *worst) *worst = errors[i];
        if (errors[i] < errors[min]) min = i;
        }

    //printf("wavelength is %d\n",min);

    return min;
    }

static int
findMinErrorAt(double *errors,int spot)
    {
    int i;
    int min;

    min = spot;
    //printf("search from %f seconds to %f seconds\n",
    //    (spot - skip/2.) / 44100,
    //    (spot + skip/2.) / 44100);
    for (i = spot - skip/2; i < spot + skip/2; ++i)
    for (i = spot - skip/2; i < spot + skip/2; ++i)
        {
        if (errors[i] < errors[min]) min = i;
        }

    if (min == spot-2 || min == spot || min == spot+2)
        return min;
    else
        return 0;
    }

static int
correlate(int *data,int length,int window, double *result)
    {
    int i,j;
    double sum;
    double totalSum;

    totalSum = 0;
    result[0] = 1.0;
    for (i = 1; i < length - window; ++i)
        {
        sum = 0;
        for (j = 0; j < window; ++j)
            sum += pow(data[j] - data[i + j],2);
        totalSum += sum;

        result[i] = sum / (totalSum / i);
        }

    return length - window;
    }

#ifdef WRITE_ERROR
static void
writeTempRRA(double *data,int length,int suffix)
    {
    int i;
    FILE *fp;
    double max;
    char buffer[512];
    int scale = pow(2,15) - 1;

    sprintf(buffer,"error%d.rra",suffix);
    fp = fopen(buffer,"w");
    fprintf(fp,"RRAUDIO\n");
    fprintf(fp,"samples: %d\n",length);
    fprintf(fp,"%%%%\n");

    max = 0;
    for (i = 0; i < length; ++i)
        {
        if (data[i] > max) max = data[i];
        if (-data[i] > max) max = -data[i];
        }

    for (i = 0; i < length; ++i)
        {
        fprintf(fp,"%d\n",(int) ((data[i] / max) * scale));
        }

    fclose(fp);
    }
#endif

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
            case 'p':
                Pitch = 1;
                break;
            case 'f':
                Frequency = 1;
                break;
            case 'n':
                Note = 1;
                break;
            case 'o':
                initialOffset = atof(arg);
                argUsed = 1;
                break;
            case 'h':
                printf("identify options:\n");
                printf("  -f     only report the detected frequency\n");
                printf("  -p     generate a command to repitch the note\n");
                printf("  -n     only report the nearest note\n");
                printf("  -o N   start analyzing N seconds into the note\n");
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
