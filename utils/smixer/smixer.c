#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "/usr/local/include/songlib/songlib.h"
#include "/usr/local/include/songlib/rra.h"
#include "/usr/local/include/songlib/debug.h"

char *PROGRAM_NAME = "smixer";
char *PROGRAM_VERSION = "0.01";

#define MAX_RRAS 20

static int processOptions(int,char **);
static void setStereoChannels(char *,int);
static void setTrackAmplitudes(char *,int);

static char *stereo = 0;
static char *amplify;
static double attenuation = 0.7;
static int delay = 70; //milliseconds
static double trackAmps[512];

int
main(int argc,char **argv)
    {
    int argIndex;
    int i,j;
    int inRRACount;
    int maxSamples;
    int delayedSamples;
    RRA *r,*rras[MAX_RRAS];
    FILE *out,*rrafilepointers[MAX_RRAS];

    // should have two inputs and one output at least

    argIndex = processOptions(argc,argv);

    inRRACount = argc - argIndex; // 1 program name

    if (inRRACount < 1)
        {
        puts("smixer in.rra [more rra files ...]");
        exit(-1);
        }

    out = stdout;

    /* read in rras, keeping track of the highest sampleCount */

    maxSamples = 0;

    for (i = 0; i < inRRACount; ++i)
        {
        if (i >= MAX_RRAS)
            {
            printf(" skipping (too many input files)\n");
            continue;
            }

        rrafilepointers[i] = OpenFile(argv[argIndex + i],"r");

        rras[i] = newRRAHeader();
        readRRAHeader(rrafilepointers[i],rras[i],0);

        if (rras[i]->channels != 1)
            Fatal("%s requires mono inputs\n",argv[0]);

        if (maxSamples < rras[i]->samples)
            maxSamples = rras[i]->samples;
        }

    delayedSamples = rras[0]->sampleRate * delay / 1000;
    maxSamples += delayedSamples;

    /* allocate space for the delayed samples */

    for (i = 0; i < inRRACount; ++i)
        {
        rras[i]->samples = maxSamples;
        createRRAData(rras[i]);
        }

    /* assign channels if not already done */

    setStereoChannels(argv[0],inRRACount);

    /* assign amplitudes if not already done */

    setTrackAmplitudes(argv[0],inRRACount);

    r = newRRAHeader();

    r->sampleRate = rras[0]->sampleRate;
    r->bitsPerSample = rras[0]->bitsPerSample;
    r->channels = 2;
    r->samples = maxSamples;

    writeRRAHeader(out,r,"createdBy: smixer",0);

    for (i = 0; i < maxSamples; ++i)
        {
        int totalLeft = 0;
        int totalRight = 0;
        for (j = 0; j < inRRACount; ++j)
            {
            if (i < rras[j]->samples)
                {
                double amp = readRRAAmplitude(rrafilepointers[j],out,
                    rras[j]->bitsPerSample,outputComment);
                if (stereo[j] == 'l' || stereo[j] == 'c')
                    totalLeft += amp * trackAmps[j];
                if (stereo[j] == 'c' || stereo[j] == 'r')
                    totalRight += amp * trackAmps[j];
                rras[j]->data[0][i] = amp;
                }
            else
                rras[j]->data[0][i] = 0;

            if (i >= delayedSamples)
                {
                if (stereo[j] == 'l')
                    totalRight +=
                        rras[j]->data[0][i-delayedSamples]
                        * trackAmps[j] * attenuation;
                else if (stereo[j] == 'r')
                    totalLeft +=
                        rras[j]->data[0][i-delayedSamples]
                        * trackAmps[j] * attenuation;
                }
            }
        fprintf(out,"%d\n",totalLeft);
        fprintf(out,"%d\n",totalRight);
        }

    for (i = 0; i < inRRACount; ++i)
        fclose(rrafilepointers[i]);

    return 0;
    }

static void
setTrackAmplitudes(char *prog,int count)
    {
    int ampCount;
    char *amp;

    if (amplify == 0)
        {
        switch (count)
            {
            case 1: amplify = "1"; break;
            case 2: amplify = "1,1"; break;
            case 3: amplify = "1,1,1"; break;
            case 4: amplify = "1,1,1,1"; break;
            case 5: amplify = "1,1,1,1,1"; break;
            case 6: amplify = "1,1,1,1,1,1"; break;
            case 7: amplify = "1,1,1,1,1,1,1"; break;
            case 8: amplify = "1,1,1,1,1,1,1,1"; break;
            case 9: amplify = "1,1,1,1,1,1,1,1,1"; break;
            default:
                Fatal("%s: please specify -s and -A options",prog);
            }
        }

    fprintf(stderr,"amplify: %s\n",amplify);
    ampCount = 0;
    amp = strtok(amplify,",");
    fprintf(stderr,"amp: %s\n",amp);
    while (ampCount < sizeof(trackAmps) / sizeof(double) && amp != 0)
        {
        trackAmps[ampCount++] = atof(amp);
        amp = strtok(0,",\n");
        fprintf(stderr,"amp: %s\n",amp);
        }

    if (ampCount == 1)
        {
        int i;
        for (i = 1; i < count; ++i)
            trackAmps[i] = trackAmps[0];
        ampCount = count;
        }

    if (ampCount != count)
        Fatal("%s: -A option (%d values) "
            "does not match input audio file count (%d)\n",
            prog,ampCount,count);
    }

static void
setStereoChannels(char *prog,int count)
    {
    if (stereo == 0)
        {
        switch (count)
            {
            case 1: stereo = "c"; break;
            case 2: stereo = "rl"; break;
            case 3: stereo = "rcl"; break;
            case 4: stereo = "rccl"; break;
            case 5: stereo = "rrcll"; break;
            case 6: stereo = "rrccll"; break;
            case 7: stereo = "rrrclll"; break;
            case 8: stereo = "rrrcclll"; break;
            case 9: stereo = "rrrrcllll"; break;
            default:
                Fatal("%s: please specify -s option",prog);
            }
        }

    if (strlen(stereo) != count)
        Fatal("%s: -s option does not match input audio file count\n",prog);
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

    while (argIndex < argc
        && argv[argIndex][0] == '-' && argv[argIndex][1] != '\0')
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
            case 'v':
                printf("%s version %s\n", PROGRAM_NAME, PROGRAM_VERSION);
                exit(0);
                break;
            case 'a':
                amplify = strdup(arg);
                argUsed = 1;
                break;
            case 'A':
                attenuation = atof(arg);
                argUsed = 1;
                break;
            case 's':
                stereo = strdup(arg);
                argUsed = 1;
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
