#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/songlib.h"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "flanger";
char *PROGRAM_VERSION = "0.01";

static int    depth;
static double depthFactor = 4.0;
static int    minDelay;
static double minDelayFactor = 0.0;
static int    offset;
static double offsetFactor = 0.0;
static int    speed;
static double speedFactor = 0.5;

static double feedback = 0;
static double attenuation = 1.0;

static double flangeFactor = 1.0;
static double flangeFactorDelta = 0.0;

static double sawtooth(int,int,int,int);
static int processOptions(int,char **);

static FILE *in,*out;

int
main(int argc,char **argv)
    {
    int argIndex = 1;

    int i;
    int *data;
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

    songInit();

    h = newRRAHeader();
    
    readRRAHeader(in,h,0);

    minDelay = (int) (h->sampleRate * minDelayFactor / 1000);
    depth = (int) (h->sampleRate * depthFactor / 1000);
    speed = (int) (h->sampleRate / speedFactor);
    offset = (int) (offsetFactor * speed) % speed;

    writeRRAHeader(out,h,"modifiedBy: flanger",0);

    data = (int *) New(sizeof(int) * h->samples);

    if (h->channels != 1)
        Fatal("please run through mono first\n");

    for (i = 0; i < h->samples; ++i)
        {
        double delayed;

        data[i] = readRRAAmplitude(in,out,h->bitsPerSample,outputComment);

        delayed = i - minDelay - sawtooth(speed,depth,offset,i);

        if (delayed >= 0)
            {
            int amp = interp(data,h->samples,delayed);
            fprintf(out,"%d\n", data[i] +
                    (int) (flangeFactor *
                        attenuation * amp));
            flangeFactor += flangeFactorDelta;
            if (flangeFactor < 0) flangeFactor = 0;
            if (flangeFactor > 1) flangeFactor = 1;
            data[i] += feedback * amp;
            }
        else
            fprintf(out,"%d\n",data[i]);
        }

    fclose(in);
    fclose(out);

    return 0;
    }

#define UP 0
#define DOWN 1

static double
sawtooth(int width,int max,int offset,int index)
    {
    int spot;
    double result;

    spot = (offset + index) % width;
    //printf("spot is %d, max is %d\n",spot,max);

    if (spot < width / 2)
        result = max * spot / (width / 2.0);
    else
        result = max * (width - spot) / (width / 2.0);

    //printf("delay is %f\n",result);
    //getchar();

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
             *         PrintActions = 1;
             *         break;
             */
            case 'a':
                attenuation = atof(arg);
                argUsed = 1;
                break;
            case 'd':
                depthFactor = atof(arg);
                argUsed = 1;
                break;
            case 'f':
                feedback = atof(arg);
                argUsed = 1;
                break;
            case 'm':
                minDelayFactor = atof(arg);
                argUsed = 1;
                break;
            case 'o':
                offsetFactor = atof(arg);
                argUsed = 1;
                break;
            case 's':
                speedFactor = atof(arg);
                argUsed = 1;
                break;
            case 'v':
                printf("%s version %s\n", PROGRAM_NAME, PROGRAM_VERSION);
                exit(0);
                break;
            case 'h':
                printf("flanger options:\n");
                printf("  -a N   attenuates the output by N\n");
                printf("         values > 1 increase loudness\n");
                printf("         values < 1 decrease loudness\n");
                printf("         default is %f\n",attenuation);
                printf("  -d N   set maximum flanging delay to N\n");
                printf("         good values range from 0 to 5\n");
                printf("         default is %f\n",depthFactor);
                printf("  -f N   set feedback gain to N\n");
                printf("         good values range from -0.9..0.9\n");
                printf("         default is %f\n",feedback);
                printf("  -m N   set minimum value of the delay to N\n");
                printf("         default is %f\n",minDelayFactor);
                printf("  -o N   set offset into flanging cycle to N\n");
                printf("         good values range from 0 to max delay\n");
                printf("         default is %f\n",offsetFactor);
                printf("  -s N   set speed of a full delay cycle to N\n");
                printf("         default is %f\n",speedFactor);
                printf("  -v     display the version number\n");
                printf("  -h     help\n");
                exit(0);
                break;
            default:
                Fatal("%s: option %s not understood\n",
                    PROGRAM_NAME,argv[argIndex]);
            }

        if (separateArg && argUsed)
            ++argIndex;

        ++argIndex;
        }

    return argIndex;
    }

