// Carter Musick
// CS 591
// Spring 2012
// rradistort

/* filter that distorts an rra file primarily by clipping
 *
 * rradistort -n in.rra out.rra     # makes all positive values negative
 * rradistort -p in.rra out.rra     # makes all negative values positive
 * rradistort -l1.5 in.rra out.rra     # sets the clipping value at 1.5
 * rradistort -f200 in.rra out.rra     # sets the feedback delay at 200 samples
 *
 *
 *  experimentation may be required for best results
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

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "filter";
char *PROGRAM_VERSION = "0.01";

static void processData(FILE *,FILE *,RRA *);
static int processOptions(int, char **);

static double step = 1.0;
int factor;
int level;
double transition;
int direction;
int sign;
int delaySize;
int loc = 0;

int
main(int argc,char **argv)
    {
    int argIndex = 1;

    int old;
    FILE *in,*out;
    RRA *r;

    level = 20000;
    sign = 0;
    direction = 0;
    delaySize = 1;
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

    r = newRRAHeader();
    readRRAHeader(in, r, 0);
    
    if (r->channels != 1)
        Fatal("distorter only works on mono audio files\n");

    factor = ((int)pow(2,r->bitsPerSample - 1)/level) / 3;
    if (level >= 20000) {
        factor = 1;
    }

    /* compute new number of samples, saving the old number */
    old = r->samples;
    r->samples = (int) (r->samples / step);

    writeRRAHeader(out,r,"modifiedBy: rraDistorter",0);

    /* restore the old number */
    r->samples = old;

    processData(in,out,r);

    fclose(in);
    fclose(out);

    return 0;
    }

static void
processData(FILE *in,FILE *out, RRA *r)
{
    int i,j;
    int count = 0;
    int delayLine[delaySize];
    for (i=0; i < r->samples; ++i) {
        for (j=0; j < r->channels; ++j)
        {
            int amp = readRRAAmplitude(in, out, r->bitsPerSample, outputComment);

            int temp = amp;
            if (count < delaySize)
            {
                delayLine[count] = temp;
            }
            else
            {
               if (loc >= delaySize)
               {
                    loc = 0;
               }
               temp += delayLine[loc];
               if (temp >= level) {
                    temp = level;
               }
               else if (temp <= -level) {
                    temp = -level;
               }     
               delayLine[loc] = temp;
               loc++;
            }
            count++;
            if (sign == 1) {
                if (temp < 0)
                    temp = -temp;
                }
            if (sign == 2) {
                if (temp > 0)
                    temp = -temp;
                }
            
            if (temp >= level) {
                temp = level;
            }
            else if (temp <= -level) {
                temp = -level;
            }
            else
            {
                // these calculations make the transitions into and out of
                // the clipping levels smoother
                double fLevel = (double) level;
                double fTemp = (double) temp;
                double dec = fTemp/fLevel;
                double newDec = dec - ((dec*dec*dec)/3.0);
                double newAmp = newDec * level;
                temp =(int) newAmp;
            }
            fprintf(out, "%d\n", temp * factor);
        }
    }
    while (delayLine[loc] * delayLine[loc] > 100)
    {
        if (loc >= 20)
        {
            loc = 0;
        }
        int a;
        a = delayLine[loc];
        delayLine[loc] = (2/3)*a;
        loc++;
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
             *     examples are -m4096 or -m 4096
             *     case 'm':
             *         MemorySize = atol(arg);
             *         argUsed = 1;
             *         break;
             * when option does not have an argument, do this
             *     case 'a':
             *         PrintActions = 1;
             *         break;
             */
            case 'v':
                printf("%s version %s\n", PROGRAM_NAME, PROGRAM_VERSION);
                exit(0);
                break;
            case 'l':
                level = atof(arg) * 1000;
                break;
            case 'f':
                delaySize = atof(arg);
                break;
            case 'p':
                sign = 1;
                break;
            case 'n':
                sign = 2;
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

