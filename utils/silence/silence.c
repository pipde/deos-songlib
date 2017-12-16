/*
 * silence
 *
 * make what appears to be silent spans more silent
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/songlib.h"
#include "/usr/local/include/songlib/rra.h"

#define factor 1.122018454301963  

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "silence";
char *PROGRAM_VERSION = "0.01";

static double amplify = 1;
static double volumeUpWait = 0.01;
static double volumeUpRamp = 0.01;
static double volumeDownWait = 0.05;
static double volumeDownRamp = 0.1;
static int thresholdDb = -30; //in decibels
static int windowSize = 500;
static int *window = 0;

static double rms(int,int);
static int *makeWindow(void);
static int processOptions(int,char **);

int
main(int argc,char **argv)
    {
    int argIndex = 1;

    int i,maxValue;
    int threshold;
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
        printf("usage: %s -fN [<input rra file> [<output rra file>]]\n",
            argv[0]);
        exit(-1);
        }

    h = newRRAHeader();
    
    readRRAHeader(in,h,0);

    if (h->channels != 1)
        Fatal("%s requires a mono audio file\n",argv[0]);

    //printf("%f,%f\n",pow(2,h->bitsPerSample-1)-1,pow(factor,thresholdDb));

    maxValue = pow(2,h->bitsPerSample-1)-1;
    threshold = maxValue * pow(factor,thresholdDb);

    double volumeUpDelta =  1.0 / secondsToSamples(volumeUpRamp);
    double volumeDownDelta =  1.0 / secondsToSamples(volumeDownRamp);
    int volumeUpWaitSamples =  secondsToSamples(volumeUpWait);
    int volumeDownWaitSamples =  secondsToSamples(volumeDownWait);

    window = makeWindow();

    fprintf(stderr,"threshold: %d\n",threshold);
    fprintf(stderr,"amplify; %f\n",amplify);
    fprintf(stderr,"volume down wait; %d\n",volumeDownWaitSamples);
    fprintf(stderr,"volume down delta; %f\n",volumeDownDelta);

    writeRRAHeader(out,h,"modifiedBy: silence",0);

    int reducing = 0;
    double transitionAttenuation = 1;
    int transitionSamples = 0;
    int bottomMessage = 1;
    int topMessage = 1;

    for (i = 0; !feof(in); ++i)
        {
        int amp = readRRAAmplitude(in,out,h->bitsPerSample,outputComment);

        if (rms(i,amp) < threshold && i > windowSize)
            {
            if (!reducing) 
                {
                reducing = 1;
                transitionSamples = 0;
                fprintf(stderr,"waiting to reduce volume at %d samples\n",
                    i);
                fprintf(stderr,"attenuation is %f\n",transitionAttenuation);
                bottomMessage = 0;
                //getchar();
                }
            else if (transitionSamples < volumeDownWaitSamples)
                {
                ++transitionSamples;
                }
            else
                {
                transitionAttenuation -= volumeDownDelta;
                //fprintf(stderr,"reducing volume to %f at %d samples\n",
                    //transitionAttenuation,i);
                }
            }
        else if (i > windowSize)
            {
            if (reducing) 
                {
                reducing = 0;
                transitionSamples = 0;
                fprintf(stderr,"starting to increasing volume at %d samples\n",
                    i);
                fprintf(stderr,"attenuation is %f\n",transitionAttenuation);
                topMessage = 0;
                //getchar();
                }
            else if (transitionSamples < volumeUpWaitSamples)
                {
                ++transitionSamples;
                }
            else
                {
                transitionAttenuation += volumeUpDelta;
                //fprintf(stderr,"increasing volume to %f at %d samples\n",
                    //transitionAttenuation,i);
                }
            }
        if (transitionAttenuation > 1)
            {
            if (!topMessage)
                {
                fprintf(stderr,"at volume top\n");
                topMessage = 1;
                //getchar();
                }
            transitionAttenuation = 1;
            }
        else if (transitionAttenuation < 0)
            {
            if (!bottomMessage)
                {
                fprintf(stderr,"at volume bottom\n");
                bottomMessage = 1;
                //getchar();
                }
            transitionAttenuation = 0;
            }

        fprintf(out,"%d\n",(int) (amplify * amp * transitionAttenuation));
        }

    if (i != h->samples + 1)
        fprintf(stderr,"WARNING: %d samples found, %d expected\n",i,h->samples);

    fclose(in);
    fclose(out);

    return 0;
    }

static int *
makeWindow()
    {
    int i;
    int *input = (int *) New(sizeof(int) * windowSize);

    for (i = 0; i < windowSize; ++i)
        input[i] = 0;

    return input;
    }

static double
rms(int count,int amp)
    {
    int index;
    static double last = 0;

    index = count % windowSize;

    last += amp * amp - window[index] * window[index];

    window[index] = amp;
        
    //printf("count is %d, amp is %d, last is %f, rms is %f\n",
        //count,amp,last,pow(last,0.5));
    //getchar();

    return pow(last,0.5);
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
            case 'a':
                amplify = atof(arg);
                argUsed = 1;
                break;
            case 't':
                thresholdDb = atoi(arg);
                argUsed = 1;
                break;
            case 'u':
                volumeUpWait = atof(arg);
                argUsed = 1;
                break;
            case 'U':
                volumeUpRamp = atof(arg);
                argUsed = 1;
                break;
            case 'd':
                volumeDownWait = atof(arg);
                argUsed = 1;
                break;
            case 'D':
                volumeDownRamp = atof(arg);
                argUsed = 1;
                break;
            case 'v':
                printf("%s version %s\n", PROGRAM_NAME, PROGRAM_VERSION);
                exit(0);
                break;
            case 'h':
                printf("silence:\n");
                printf("    reduces the volume on regions that appear to\n");
                printf("    be silent - it is mainly used to remove\n");
                printf("    recording hiss and background noises\n");
                printf("\n");
                printf("silence options:\n");
                printf("  -a N     set amplification factor to N\n");
                printf("           amplification happens after compression\n");
                printf("           default value is %f\n",amplify);
                printf("  -t N     set threshold to N decibels\n");
                printf("           good values range from -20 downwards\n");
                printf("           lower values mean more of the audio is "
                    "unchanged\n");
                printf("           default value is %d\n",thresholdDb);
                printf("  -u N     set volume up wait time to N seconds\n");
                printf("           wait time is how long to wait before "
                    "increasing volume\n");
                printf("           good values range from 0 to 0.01 seconds\n");
                printf("           default value is %f\n",volumeUpWait);
                printf("  -U N     set volume up ramp time to N seconds\n");
                printf("           ramp time is how long it takes to achieve "
                    "full volume\n");
                printf("           once the wait time has passed\n");
                printf("           good values range from 0.01 upwards\n");
                printf("           default value is %f\n",volumeUpRamp);
                printf("  -d N     set volume down wait time to N seconds\n");
                printf("           good values range from 0 to 0.02 seconds\n");
                printf("           default value is %f\n",volumeDownWait);
                printf("  -D N     set volume down ramp time to N seconds\n");
                printf("           good values range from 0.02 upwards\n");
                printf("           default value is %f\n",volumeDownRamp);
                printf("  -v       display this program's version number\n");
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

