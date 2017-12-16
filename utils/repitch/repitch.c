// Carter Musick
// CS 591
// Fall 2012
// rrarepitch

/* filter that determines pitch of single note, and repitches it to
 * the closest known frequency
 * NOTE: Accuracy is not guaranteed
 *
 * using a -o option lowers the risk of an inaccurate guess.
 * -o option is not necessary, but guesses in lower octaves
 * have a decent likelihood of being incorrect.
 *
 * certain instruments may also be less accurate than others.
 *
 *
 * rrarepitch -o4 in.rra out.rra   # use this option to repitch a note you
 *                                 think (or know) is in octave 4.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <assert.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/wave.h"
#include "/usr/local/include/songlib/rra.h"


char *PROGRAM_NAME = "rrarepitch";
char *PROGRAM_VERSION = "1.0";

static void processData(FILE *,FILE *,RRA *, RRA *);
static double interp(int *,int,double);
static int processOptions(int, char **);
static int targetFreqs[100];

static double step = 1.0;
int octave;
int skip;

int
main(int argc,char **argv)
    {
    int freqCount;
    double twelfthRoot;
    skip = 10;
    twelfthRoot = pow(2.0, (1.0/12.0));

    double baseFreq = 8.859;    // number of samples for Eb 8 at 44,100 sample rate
    for (freqCount = 0; freqCount < 100; freqCount++)
    {
        double target = baseFreq * pow(twelfthRoot,freqCount);
        int samples = (int)target;
        targetFreqs[freqCount] = samples;
    }

    int argIndex = 1;

    FILE *in,*out;
    RRA *r, *r2;

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
        printf("usage: filter [<input rra file> [<output wave file>]]\n");
        exit(-1);
        }

    r2 = readRRA(in,0);
    fclose(in);

    if (argc-argIndex == 0)
        {
        in = stdin;
        }
    else if (argc-argIndex == 1)
        {
        in = OpenFile(argv[argIndex],"r");
        }
    else if (argc-argIndex == 2)
        {
        in = OpenFile(argv[argIndex],"r");
        }
    else
        {
        exit(-1);
        }

    r = newRRAHeader();
    readRRAHeader(in, r, 0);
    
    if (r->channels != 1)
        Fatal("rrarepitch only works on mono audio files\n");

    processData(in,out,r, r2);

    fclose(in);
    fclose(out);

    return 0;
    }

static void
processData(FILE *in,FILE *out,RRA *r, RRA *r2)
    {
    int amplitudes[3000];
    int error = 10000000;
    int errorLoc = 0;
    int i;
    for (i=0; i < r->samples; ++i) {
        int amp = readRRAAmplitude(in, out, r->bitsPerSample, outputComment);

            if (i>=50000 && i<53000)
            {
                amplitudes[i-50000] = amp;
            }
        }
    int k;
    for(k=skip; k < 2900; k++) // skip a certain number of samples based on octave
    {
        int l;
        int error1 = 0;
        for (l=0; l<skip; l++) {
           error1 += abs(amplitudes[l] - amplitudes[k+l]);
        }
        if ((abs(error1)) < (abs(error)))
        {
            error = error1;
            errorLoc = k;
        }
    }
    int closest = 0;
    int difference = 500;
    for (k=0; k<100; k++)
    {
        int newDif = abs(errorLoc - targetFreqs[k]);

        if (newDif <= difference)
        {
            difference = newDif;
            closest = targetFreqs[k];
        }
    }
    printf("Wavelength: %d samples \n", errorLoc);
    printf("Original Frequency: %d Hz\n", 44100/errorLoc);
    printf("Lowest error: %d \n", error);

    step = (double)errorLoc / closest;
    printf("Step: %f \n", step);
    int old = r2->samples;
    r2->samples = (int) (r2->samples / step);
    r2->samples = old;
    writeRRAHeader(out,r2,"modifiedBy: rrarepitch",0);

    double t = 0;
    while (t < r2->samples)
        {
        fprintf(out,"%d\n",(int) interp(r2->data[0],r2->samples,t));
        t = t + step;
        }

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
            case 'o':
                octave = atof(arg);
                skip = 10 * pow(2, (7-octave));
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

