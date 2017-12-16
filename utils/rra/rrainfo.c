#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/rra.h"
#include "/usr/local/include/songlib/wave.h"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "rrainfo";
char *PROGRAM_VERSION = "0.01";

static void info(char *,FILE *);
static int processOptions(int, char **);

int samples;
int sampleRate;
int bitsPerSample;
int channels;

int
main(int argc,char **argv)
    {
    int argIndex = 1;

    int i;

    argIndex = processOptions(argc,argv);

    if (argIndex == argc)
        info("stdin",stdin);
    else
        {
        for (i = argIndex; i < argc; ++i)
            {
            FILE *fp = OpenFile(argv[i],"r");
            info(argv[i],OpenFile(argv[i],"r"));
            fclose(fp);
            }
        }

    return 0;
    }

static void
info(char *name,FILE *in)
    {
    RRA *h;

    h = newRRAHeader();
    readRRAHeader(in,h,0);

    printf("%s\n",name);
    if (channels)
        printf("    %d\n",h->channels);
    else if (sampleRate)
        printf("    %d\n",h->sampleRate);
    else if (samples)
        printf("    %d\n",h->samples);
    else if (bitsPerSample)
        printf("    %d\n",h->bitsPerSample);
    else
        {
        printf("    sampleRate: %d\n",h->sampleRate);
        printf("    bitsPerSample: %d\n",h->bitsPerSample);
        printf("    channels: %d\n",h->channels);
        printf("    samples: %d\n",h->samples);
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
                printf("%s version %s\n", PROGRAM_NAME, PROGRAM_VERSION);
                exit(0);
                printf("%s\n",arg);
                break;
            case 'c':
                channels = 1;
                break;
            case 'r':
                sampleRate = 1;
                break;
            case 'b':
                bitsPerSample = 1;
                break;
            case 's':
                samples = 1;
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

