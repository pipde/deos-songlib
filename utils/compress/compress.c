#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/rra.h"

#define factor 1.122018454301963  

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "compress";
char *PROGRAM_VERSION = "0.01";

static double amplify = 1;
static double peakRatio = 4;
static int thresholdDb = -10; //in decibels
static double attack = 0.3; //in seconds
static double release = 0.8; //in seconds
static int windowSize = 500;
static int **window = 0;

static double rms(int,int,int);
static int **makeWindow(int);
static int processOptions(int,char **);

int
main(int argc,char **argv)
    {
    int argIndex = 1;

    int i,j,maxValue;
    double ratio;
    int threshold;
    double attackDelta,releaseDelta;
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
        printf("usage: compress -fN [<input rra file> [<output rra file>]]\n");
        exit(-1);
        }

    h = newRRAHeader();
    
    readRRAHeader(in,h,0);

    //printf("%f,%f\n",pow(2,h->bitsPerSample-1)-1,pow(factor,thresholdDb));
    maxValue = pow(2,h->bitsPerSample-1)-1;
    threshold = maxValue * pow(factor,thresholdDb);
    attackDelta = peakRatio - 1;
    if (attack > 0)
        attackDelta = attackDelta / (attack * h->sampleRate);
    releaseDelta = peakRatio - 1;
    if (release > 0)
        releaseDelta = releaseDelta / (release * h->sampleRate);

    window = makeWindow(h->channels);

    fprintf(stderr,"threshold: %d\n",threshold);
    fprintf(stderr,"amplify; %f\n",amplify);
    //printf("attackdelta is %f\n",attackDelta);

    writeRRAHeader(out,h,"modifiedBy: compress",0);

    ratio = 1;
    for (i = 0; !feof(in); ++i)
        for (j = 0; j < h->channels; ++j)
            {
            int amp = readRRAAmplitude(in,out,h->bitsPerSample,outputComment);

            if (rms(i,amp,j) > threshold)
                {
                ratio = ratio + attackDelta;
                if (ratio > peakRatio) 
                    ratio = peakRatio;
                //printf("above threshold, ratio is %f\n",ratio);
                }
            else
                {
                ratio = ratio - releaseDelta;
                if (ratio < 1)
                    ratio = 1;
                //printf("below threshold, ratio is %f\n",ratio);
                }

            fprintf(out,"%d\n",(int) (amplify * amp / ratio));
            }

    if (i != h->samples + 1)
        fprintf(stderr,"WARNING: %d samples found, %d expected\n",i,h->samples);

    fclose(in);
    fclose(out);

    return 0;
    }

static int **
makeWindow(int channels)
    {
    int i,j;
    int **input;

    input = (int **) New(sizeof(int *) * channels);

    for (i = 0; i < channels; ++i)
        {
        input[i] = (int *) New(sizeof(int *) * windowSize);
        for (j = 0; j < windowSize; ++j)
            input[i][j] = 0;
        }

    return input;
    }

static double
rms(int count,int amp,int channel)
    {
    int index;
    static double last[2] = {0,0};

    index = count % windowSize;

    last[channel] +=
        amp * amp - window[channel][index] * window[channel][index];

    window[channel][index] = amp;
        
    //printf("count is %d, amp is %d, last is %f, rms is %f\n",
        //count,amp,last,pow(last,0.5));

    return pow(last[channel],0.5);
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
            case 'r':
                peakRatio = atof(arg);
                argUsed = 1;
                break;
            case 't':
                thresholdDb = atoi(arg);
                argUsed = 1;
                break;
            case 'A':
                attack = atof(arg);
                argUsed = 1;
                break;
            case 'R':
                release = atof(arg);
                argUsed = 1;
                break;
            case 'v':
                printf("%s version %s\n", PROGRAM_NAME, PROGRAM_VERSION);
                exit(0);
                break;
            case 'h':
                printf("compress options:\n");
                printf("  -a N     set amplification factor to N\n");
                printf("           amplification happens after compression\n");
                printf("           default value is %f\n",amplify);
                printf("  -t N     set threshold to N decibels\n");
                printf("           good values range from -30 upwards\n");
                printf("           lower values mean more of the signal is"
                    " compressed\n");
                printf("           default value is %d\n",thresholdDb);
                printf("  -r N     set compression ratio to N\n");
                printf("           good values range from 2 upwards\n");
                printf("           higher values mean higher compression\n");
                printf("           default value is %f\n",peakRatio);
                printf("  -A N     set attack to N seconds\n");
                printf("           good values range from 0 to 0.5 seconds\n");
                printf("           higher values mean longer to reach full compression\n");
                printf("           default value is %f\n",attack);
                printf("  -R N     set release to N seconds\n");
                printf("           good values range from 0 to 1.0 second\n");
                printf("           higher values mean longer to turn off compression\n");
                printf("           default value is %f\n",attack);
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

