#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "/usr/local/include/songlib/songlib.h"
#include "/usr/local/include/songlib/rra.h"
#include "/usr/local/include/songlib/debug.h"

char *PROGRAM_NAME = "fastmixer";
char *PROGRAM_VERSION = "0.02";

#define MAX_RRAS 20

static int processOptions(int,char **);

static double attenuation = 1.0;
static int skip = 0;

int
main(int argc,char **argv)
    {
    int argIndex;
    int i,j,k;
    int inRRACount;
    int maxChannels,maxSamples,maxSkip;
    RRA *r,*rraheaders[MAX_RRAS];
    FILE *out,*rrafiles[MAX_RRAS];

    // should have two inputs and one output at least

    argIndex = processOptions(argc,argv);

    inRRACount = argc - argIndex; // 1 program name, 1 output file

    if (inRRACount < 1)
        {
        puts("fastmixer in.rra [more rra files ...]");
        exit(-1);
        }

    out = stdout;

    /* read in rras, keeping track of the highest sampleCount */

    maxSkip = maxChannels = maxSamples = 0;

    for (i = 0; i < inRRACount; ++i)
        {
        if (i >= MAX_RRAS)
            {
            printf(" skipping (too many input files)\n");
            continue;
            }

        if (strcmp(argv[argIndex + i],"-") == 0)
            rrafiles[i] = stdin;
        else
            rrafiles[i] = OpenFile(argv[argIndex + i],"r");

        rraheaders[i] = newRRAHeader();
        readRRAHeader(rrafiles[i],rraheaders[i],0);

        if (maxSamples < rraheaders[i]->samples)
            maxSamples = rraheaders[i]->samples;
        if (maxChannels < rraheaders[i]->channels)
            maxChannels = rraheaders[i]->channels;
        if (maxSkip < rraheaders[i]->skip)
            maxSkip = rraheaders[i]->skip;
        }
    if (skip != 0) maxSkip = skip;

    r = newRRAHeader();
    r->sampleRate = rraheaders[0]->sampleRate;
    r->bitsPerSample = rraheaders[0]->bitsPerSample;
    r->channels = maxChannels;
    r->samples = maxSamples - maxSkip;

    writeRRAHeader(out,r,"createdBy: fmix",0);

    for (i = 0; i < maxSamples; ++i)
        {
        for (j = 0; j < maxChannels; ++j)
            {
            int total = 0;
            for (k = 0; k < inRRACount; ++k)
                {
                if (i < rraheaders[k]->samples && j < rraheaders[k]->channels)
                    {
                    //total += readRRAAmplitude 
                                        //( rrafiles[k], rraheaders[k]->bitsPerSample, 0);
                    total += readRRAAmplitude(rrafiles[k],out,rraheaders[k]->bitsPerSample,outputComment);
                    }
                }
            if (i >= maxSkip)
                fprintf(out,"%d\n",(int)(total * attenuation));
            }
        }

    for (i = 0; i < inRRACount; ++i)
        fclose(rrafiles[i]);

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
                attenuation = atof(arg);
                argUsed = 1;
                break;
            case 's':
                skip = secondsToSamples(atof(arg));
                argUsed = 1;
                break;
            case 'S':
                skip = beatsToSamples(atof(arg));
                argUsed = 1;
                break;
            case 'h':
                printf("fastmixer options:\n");
                printf("  -a N   set amplitude factor to N\n");
                printf("         values > 1 increase loudness\n");
                printf("         values < 1 decrease loudness\n");
                printf("         default is %f\n",attenuation);
                printf("  -s N   skip the first N seconds of each track\n");
                printf("         default is %f\n",samplesToSeconds(skip));
                printf("  -S N   skip the first N beats of each track\n");
                printf("         default is %f\n",samplesToBeats(skip));
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
