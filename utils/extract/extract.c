/* extract notes (or song phrases)
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <assert.h>
#include <limits.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/wave.h"
#include "/usr/local/include/songlib/rra.h"

#define SILENCE_OFFSET 100
#define SILENCE_RUN 1000

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "extract";
char *PROGRAM_VERSION = "0.01";

static int Repeats = 1;
static char *Base = "take";
static double Gap = 0.5;
static double Threshold = -30;
static int Note = 0;
static double Prelude = 0.15; //seconds
static int StartingNote = 0;
static int TakeNumber = 0;

static void convertThreshold(WAVE *);
static void processData(WAVE *);
static int extractNote(int *,int,int i,WAVE *);
static void writeNote(int *,int,WAVE *);
static int processOptions(int, char **);
static FILE *Timings = 0;
static int TimingData = 1;

int
main(int argc,char **argv)
    {
    int argIndex = 1;
    char *source = 0;

    argIndex = processOptions(argc,argv);

    if (argc-argIndex == 1)
        {
        source = argv[argIndex];
        }
    else
        {
        printf("usage: %s [OPTIONS] <input wav file>\n",argv[0]);
        exit(-1);
        }

    if (TimingData)
        {
        char buffer[512];
        snprintf(buffer,sizeof(buffer)-1,"%s%d_timings",Base,TakeNumber);
        Timings = OpenFile(buffer,"w");
        printf("note timings are stored in %s\n",buffer);
        }

    Note = StartingNote;
    processData(readWave(source));
    printf("%d notes extracted.\n",Note);

    if (TimingData) fclose(Timings);

    return 0;
    }

static void
processData(WAVE *w)
    {
    int i;
    int *data = w->channels[0];
    int size = w->sampleCount;

    printf("wave read.\n");

    //displayWave(w);

    //convert Gap from seconds to samples
    Gap = Gap * w->SampleRate;

    //convert Threshold from decibels to amplitude
    convertThreshold(w);

    for (i = 0; i < size; ++i)
        {
        if (data[i] > Threshold)
            {
            i -= Prelude * w->SampleRate;
            if (i < 0) i = 0;
            fprintf(Timings,"%.3f\n",i * 1.0 / w->SampleRate);
            i = extractNote(data,size,i,w);
            }
        ++i;
        }
    }

static void
convertThreshold(WAVE *w)
    {
    int i;
    double sum;

    sum = 0;
    if (Threshold == 0.0)
        {
        for (i = 0; i + SILENCE_OFFSET < w->sampleCount && i < SILENCE_RUN; ++i)
            {
            int amp = abs(w->channels[0][SILENCE_OFFSET+i]);
            //printf("sum %d is %f (%d)\n",i+SILENCE_OFFSET,sum,amp);
            sum += amp;
            }
        Threshold = 4 * sum / SILENCE_RUN;
        }
    else
        Threshold = (pow(2,w->BitsPerSample-1)-1) * pow(10,Threshold/20);

    //printf("Threshold is %f\n",Threshold);
    }

static int
extractNote(int *data,int size,int i,WAVE *w)
    {
    int j;
    int low;
    int *phrase;
    int length = 1000;

    phrase = New(sizeof(int) * length);

    j = 0;
    low = 0;
    while (i < size)
        {
        if (j == length)
            {
            length *= 2;
            phrase = ReNew(phrase,sizeof(int) * length);
            }
        phrase[j++] = data[i++];
        if (abs(phrase[j-1]) < Threshold)
            ++low;
        else
            low = 0;
        if (low > Gap) break;
        }

    writeNote(phrase,j,w);
    free(phrase);

    return i;
    }

static void
writeNote(int* data,int size,WAVE *w)
    {
    int i;
    FILE *fp;
    char buffer[512];

    //printf("Note is %d\n",Note);
    if (Repeats == 1)
        snprintf(buffer,sizeof(buffer),"%s%d_%d.rra",
            Base,TakeNumber,Note);
    else
        snprintf(buffer,sizeof(buffer),"%s%d_%d.rra",
            Base,TakeNumber + Note % Repeats,Note / Repeats);

    fp = OpenFile(buffer,"w");

    //printf("extracting note to %s...\n",buffer);

    fprintf(fp,"RRAUDIO\n");
    fprintf(fp,"channels: 1\n");
    fprintf(fp,"sampleRate: %d\n",w->SampleRate);
    fprintf(fp,"bitsPerSample: %d\n",w->BitsPerSample);
    fprintf(fp,"samples: %d\n",size);
    fprintf(fp,"%%%%\n");

    double gain = 0.1;
    double amplitude = 1.0;
    for (i = 0; i < size; ++i)
        {
        fprintf(fp,"%d\n",(int)((1 - amplitude) * data[i]));
        amplitude = amplitude * gain;
        }

    fclose(fp);

    ++Note;
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
            case 'b':
                Base = arg;
                argUsed = 1;
                break;
            case 'c':
                TimingData = 0;
                break;
            case 'g':
                Gap = atof(arg);
                argUsed = 1;
                break;
            case 'p':
                Prelude = atof(arg);
                argUsed = 1;
                break;
            case 'r':
                Repeats = atoi(arg);
                argUsed = 1;
                break;
            case 's':
                StartingNote = atoi(arg);
                argUsed = 1;
                break;
            case 't':
                TakeNumber = atoi(arg);
                argUsed = 1;
                break;
            case 'T':
                Threshold = atof(arg);
                argUsed = 1;
                break;
            case 'h':
                printf("usage:\n");
                printf("  %s [OPTIONS] [INFILE]\n",PROGRAM_NAME);
                printf("options:\n");
                printf("  -b S   set the base name of the extract phrase to S\n");
                printf("         default is %s\n",Base);
                printf("  -c     do not store timing data\n");
                printf("         default is to store timing data\n");
                printf("  -g N.N set the longest gap in a phrase to N.N seconds\n");
                printf("         default is %f seconds\n",Gap);
                printf("  -p N.N set the phrase lead-in to N.N seconds\n");
                printf("         default is %f seconds\n",Prelude);
                printf("  -r N   set the number of note or phrase repeats to N\n");
                printf("         default is %d\n",Repeats);
                printf("  -s N   set the staring note or phrase number to N\n");
                printf("         default is %d\n",StartingNote);
                printf("  -t N   set the take number to N\n");
                printf("         default is %d\n",TakeNumber);
                printf("  -T N.N set the threshold for silence to N.N decibels\n");
                printf("         higher numbers mean more tolerance for noise\n");
                printf("         a zero threshold means the threshold is set automatically\n");
                printf("         default is %.2f\n",Threshold);
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

