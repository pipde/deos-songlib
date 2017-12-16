#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/rra.h"
#include "/usr/local/include/songlib/wave.h"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "rra2wav";
char *PROGRAM_VERSION = "0.01";

static double Amplitude = 1;
static double Skip = 0;

static void filter(FILE *,FILE *);
static void batch(int,char **);
static int processOptions(int, char **);

int
main(int argc,char **argv)
    {
    int argIndex = 1;

    argIndex = processOptions(argc,argv);

    if (argc-argIndex == 0)
        {
        filter(stdin,stdout);
        }
    else if (argc-argIndex == 1)
        {
        filter(OpenFile(argv[argIndex],"r"),stdout);
        }
    else if (argc-argIndex >= 2 && strstr(argv[argIndex+1],".rra") != 0)
        {
        batch(argc-argIndex,argv+argIndex);
        }
    else if (argc-argIndex == 2)
        {
        filter(OpenFile(argv[argIndex],"r"),OpenFile(argv[argIndex+1],"w"));
        }
    else
        {
        printf("usage: rra2wav [<input rra file> [<output wav file>]]\n");
        printf("       rra2wav <input rra file1> <input rra file2> ...\n");
        exit(-1);
        }

    return 0;
    }

static void
filter(FILE *in,FILE *out)
    {
    int i,j,amp;
    int skipSamples;
    RRA *h;

    h = newRRAHeader();
    
    readRRAHeader(in,h,0);

    skipSamples = Skip * h->sampleRate;

    writeWaveHeader(out,h->sampleRate,h->channels,
        h->bitsPerSample,h->samples - h->skip - skipSamples);

    amp = readRRAAmplitude(in,out,h->bitsPerSample,discardComment);
    for (i = 0; !feof(in) && i < h->samples; ++i)
        for (j = 0; !feof(in) && j < h->channels; ++j)
            {
            if (i >= (h->skip + skipSamples) * h->channels)
                writeByteInt(out,amp * Amplitude,h->bitsPerSample/8);
            amp = readRRAAmplitude(in,out,h->bitsPerSample,discardComment);
            }

    fclose(in);
    fclose(out);
    }

static void
batch(int argc,char **argv)
    {
    int i;
    char buffer[512];
    char *base;
    char *suffix = ".wav";
    FILE *in,*out;

    for (i = 0; i < argc; ++i)
        {
        fprintf(stderr,"converting %s to WAV\n",argv[i]);
        in = OpenFile(argv[i],"r");
        base = strtok(argv[i],".");
        if (strlen(base) > sizeof(buffer) - (strlen(suffix) + 2))
            Fatal("file name too long\n");
        sprintf(buffer,"%s%s",base,suffix);
        out = OpenFile(buffer,"w");
        filter(in,out);
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
            case 'a':
                Amplitude = atof(arg);
                argUsed = 1;
                break;
            case 's':
                Skip = atof(arg);
                argUsed = 1;
                break;
            case 'v':
                printf("%s version %s\n", PROGRAM_NAME, PROGRAM_VERSION);
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

