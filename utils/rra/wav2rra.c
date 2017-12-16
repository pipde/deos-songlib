#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/rra.h"
#include "/usr/local/include/songlib/wave.h"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "wav2rra";
char *PROGRAM_VERSION = "0.02";

static void filter(FILE *,FILE *);
static void batch(int,char **);
static int processOptions(int, char **);
static double Attenuation = 1.0;

static int Track = -1;

int
main(int argc,char **argv)
    {
    int argIndex = 1;

    argIndex = processOptions(argc,argv);

    if (argc-argIndex == 1)
        {
        filter(OpenFile(argv[argIndex],"r"),stdout);
        }
    else if (argc-argIndex >= 2 && strstr(argv[argIndex+1],".wav") != 0)
        {
        batch(argc-argIndex,argv+argIndex);
        }
    else if (argc-argIndex == 2)
        {
        filter(OpenFile(argv[argIndex],"r"),OpenFile(argv[argIndex+1],"w"));
        }
    else
        {
        printf("usage: wav2rra <input wav file> [<output rra file>]\n");
        printf("       wav2rra <input wav file1> <input wav file2> ...\n");
        exit(-1);
        }

    return 0;
    }

static void
filter(FILE *in,FILE *out)
    {
    int i,amp;
    WAVE *w;
    RRA *h;

    w = newWave();
    //printf("reading wave header...\n");
    readWaveHeader(w,in);
    //printf("wave header read.\n");

    h = newRRAHeader();

    h->sampleRate = w->SampleRate;
    h->samples = w->sampleCount;
    //fprintf(stderr,"%d samples in wav\n",w->sampleCount);
    h->bitsPerSample = w->BitsPerSample;
    if (Track == -1)
        h->channels = w->NumChannels;
    else
        h->channels = 1;
    h->data = w->channels;
    h->maxAmp = w->maxAmp;

    if (h->sampleRate == 0) h->sampleRate = 44100;

    writeRRAHeader(out,h,"modifiedBy: wav2rra",0);

    for (i = 0; !feof(in); ++i)
        {
        amp = readByteInt(in,h->bitsPerSample/8);
        if (amp >= w->maxAmp)
            amp = -2 * h->maxAmp + amp;

        if (Track == -1 || i % w->NumChannels == Track)
            fprintf(out,"%d\n",(int)(amp * Attenuation));
        }

    if ((i - 1) / w->NumChannels != h->samples)
        fprintf(stderr,"WARNING: %d samples expected, %d found\n",
            h->samples, i / w->NumChannels);

    fclose(in);
    fclose(out);
    }

static void
batch(int argc,char **argv)
    {
    int i;
    char buffer[512];
    char *base;
    char *suffix = ".rra";
    FILE *in,*out;

    for (i = 0; i < argc; ++i)
        {
        fprintf(stderr,"converting %s to RRA\n",argv[i]);
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
            case 'h':
                printf("%s [-v] [-a] [-t N] [input.wav [output.rra]]\n",
                    PROGRAM_NAME);
                exit(0);
                break;
            case 'a':
                Attenuation = atof(arg);
                argUsed = 1;
                break;
            case 't':
                Track = atoi(arg);
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

