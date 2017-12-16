#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/rra.h"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "stereo";
char *PROGRAM_VERSION = "0.01";

#define CENTER 0
#define LEFT 1
#define RIGHT 2

static int LeftDelay = 0;
static int RightDelay = 0;
static double LeftAttenuation = 1.0;
static double RightAttenuation = 1.0;

static int processOptions(int, char **);

int
main(int argc,char **argv)
    {
    int argIndex = 1;

    int i,leftSamplesDelayed,rightSamplesDelayed,maxDelayed;
    int Min,Max,clips = 0;
    int *data;
    FILE *in,*out;
    RRA *track;

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
        printf("usage: stereo [-aNN.N] [-dNN.N] [-l] [-r] "
            "[<input rra> [<output rra>]]\n");
        exit(-1);
        }

    track = newRRAHeader();
    readRRAHeader(in,track,0);
    
    if (track->channels != 1)
        Fatal("stereo requires a mono input");

    track->channels = 2;

    Min = (int) -pow(2,track->bitsPerSample-1);
    Max = (int) pow(2,track->bitsPerSample-1) - 1;

    writeRRAHeader(out,track,"modifiedBy: stereo",0);

    leftSamplesDelayed = (int) (LeftDelay * (track->sampleRate / 1000.0));
    rightSamplesDelayed = (int) (RightDelay * (track->sampleRate / 1000.0));
    maxDelayed = leftSamplesDelayed > rightSamplesDelayed?
        leftSamplesDelayed : rightSamplesDelayed;
    data = (int *) New(sizeof(int) * (track->samples + maxDelayed));

    for (i = 0; i < track->samples + maxDelayed; ++i)
        {
        int x,y,z;

        if (i >= track->samples)
            z = 0;
        else
            z = readRRAAmplitude(in,out,track->bitsPerSample,outputComment);

        data[i] = z;

        if (i < leftSamplesDelayed)
            x = 0;
        else
            x = data[i-leftSamplesDelayed];

        if (i < rightSamplesDelayed)
            y = 0;
        else
            y = data[i-rightSamplesDelayed];

        fprintf(out,"%d\n%d\n",
            (int)(x*LeftAttenuation),(int)(y*RightAttenuation));

        if (x > Max || x < Min || y > Max || y < Min)
            ++clips;
        }

    fclose(in);
    fclose(out);

    fprintf(stderr,"%s clips: %d\n",argv[0],clips);
    return 0;
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
            case 'c':
                LeftDelay = 0;
                LeftAttenuation = 1.0;
                RightDelay = 0;
                RightAttenuation = 1.0;
                break;
            case 'l':
                LeftDelay = atoi(arg);
                argUsed = 1;
                break;
            case 'L':
                LeftAttenuation = atof(arg);
                argUsed = 1;
                break;
            case 'r':
                RightDelay = atoi(arg);
                argUsed = 1;
                break;
            case 'R':
                RightAttenuation = atof(arg);
                argUsed = 1;
                break;
            case 'h':
                printf("stereo options:\n");
                printf("  -l N    set the left channel delay to N milliseconds\n");
                printf("          default is %d\n",LeftDelay);
                printf("  -L N.N  set the attenuation of the left track to N.N\n");
                printf("          default is %f\n",LeftAttenuation);
                printf("  -r N    set the right channel delay to N milliseconds\n");
                printf("          default is %d\n",RightDelay);
                printf("  -R N.N  set the attenuation of the right track to N.N\n");
                printf("          default is %f\n",RightAttenuation);
                printf("  -c      set both channels as equals\n");
                printf("          sets delays to zero, attenuations to one\n");
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

