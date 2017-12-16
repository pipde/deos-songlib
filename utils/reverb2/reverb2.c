#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "/usr/local/include/songlib/songlib.h"
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/rra.h"
#include "/usr/local/include/songlib/wave.h"
#include "/usr/local/include/songlib/filters.h"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "reverb2";
char *PROGRAM_VERSION = "0.01";

static int processOptions(int, char **);
static void processData(FILE *,FILE *,RRA *);

static double AmpFactor = 1.00;
static double Freq = 8000;
static double Quality = 0.7071;
static double Gain = 6;
static double Style = LOWPASS;

#define COMBS 6
int earlyDelays[COMBS] = { 50,56,61,68,72,78 };
double earlyCombGains[COMBS] = { 0.46,0.47,0.475,0.48,0.49,0.50 };
double earlyLowpassGains[COMBS] = { 0.4482,0.4399,0.4350,0.4316,0.4233,0.3735 };
lpc *early[COMBS];
int lateDelay = 6;
double lateGain = 0.7;
dap *late;

/* vocal mastering 
 *
 * high pass filter with ~100 Hz corner
 * notch filter at ~300 Hz low Q
 * notch filter at ~3000 Hz high Q
 * high shelf filter at ~9000 Hz 2 dB
 */

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
        printf("usage: %s [<input rra file> [<output wave file>]]\n",argv[0]);
        exit(-1);
        }

    h = newRRAHeader();
    
    readRRAHeader(in,h,0);

    if (h->channels > 1)
        Fatal("lowpass works on mono files, use slowpass for stero files\n");

    writeRRAHeader(out,h,"modifiedBy: lowpass",0);

    int i;
    for (i = 0; i < COMBS; ++i)
        early[i] = newLPC(millisecondsToSamples(earlyDelays[i]),
            earlyCombGains[i],earlyLowpassGains[i]);
    late = newDAP(millisecondsToSamples(lateDelay),lateGain);
    processData(in,out,h);

    fclose(in);
    fclose(out);

    return 0;
    }

static int
getCombs(int in)
    {
    int i;
    int middle = 0;
    for (i = 0; i < COMBS; ++i)
        middle += getLPC(early[i],in);
    return middle;
    }

static void
updateCombs(int in)
    {
    int i;
    for (i = 0; i < COMBS; ++i)
        updateLPC(early[i],in);
    }

static void
updateAllpass(int in)
    {
    updateDAP(late,in);
    }

static int
getAllpass(int in)
    {
    return getDAP(late,in);
    }

static int
get(int in)
    {
    return getCombs(in) + getAllpass(in);
    }

static void
update(int in)
    {
    updateAllpass(getCombs(in));
    updateCombs(in);
    }

static void
processData(FILE *in,FILE *out,RRA *h)
    {
    int amp;

    amp = readRRAAmplitude(in,out,h->bitsPerSample,outputComment);
    while (!feof(in))
        {
        fprintf(out,"%d\n",(int)(get(amp) * AmpFactor));
        update(amp);
        //getchar();
        amp = readRRAAmplitude(in,out,h->bitsPerSample,outputComment);
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
                printf("%s version %s\n", argv[0], PROGRAM_VERSION);
                exit(0);
                break;
            case 'a':
                AmpFactor = atof(arg);
                argUsed = 1;
                break;
            case 'f':
                Freq = atof(arg);
                argUsed = 1;
                break;
            case 'g':
                Gain = atof(arg);
                argUsed = 1;
                break;
            case 'q':
                Quality = atof(arg);
                argUsed = 1;
                break;
            case 'h':
                printf("%s options:\n",argv[0]);
                printf("  -a N   set amplitude factor to N\n");
                printf("         values > 1 increase loudness\n");
                printf("         values < 1 decrease loudness\n");
                printf("         default is %f\n",AmpFactor);
                printf("  -f N.N set cutoff or center frequency to N.N Hertz\n");
                printf("         default is %f Hz\n",Freq);
            if (Style == PEAK || Style == LOWSHELF || Style == HIGHSHELF)
                {
                printf("  -g N.N set the gain to N.N\n");
                printf("         positive gains are boosts, negative are cuts\n");
                printf("         default is %f dB\n",Gain);
                }
                printf("  -q N.N set the quality factor to N.N\n");
                printf("         the higher the quality, the more gain at the cutoff\n");
                printf("         default is %f\n",Quality);
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

