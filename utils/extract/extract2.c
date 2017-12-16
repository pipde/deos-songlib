/*
 * extract: extract single notes from a wave audio stream
 *          notes in the wave must be separated by silence
 *          single notes are extracted to RRA format
 *
 * written by Madhav Rao
 * modified by John Lusth
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <stdarg.h>
#include <string.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/songlib.h"
#include "/usr/local/include/songlib/rra.h"
#include "/usr/local/include/songlib/wave.h"

#define FADE_IN 3.00 
#define FADE_OUT 0.99 

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */
char *PROGRAM_NAME = "rraextract";
char *PROGRAM_VERSION = "0.03";

static void extract(WAVE *);
static int extractFile(WAVE *,int);
static int processOptions(int, char **);

int startThreshold = 100;
int finishThreshold = 100;
int startWindowSize = 5;
int finishWindowSize = 500;
double minimumSeconds = 0.25;

int
main(int argc,char **argv)
    {
    int argIndex = 1;

    FILE *in;
    WAVE *w;

    argIndex = processOptions(argc,argv);

    if (argc-argIndex == 0)
        {
        in = stdin;
        }
    if (argc-argIndex == 1)
        {
        in = OpenFile(argv[argIndex],"r");
        }
    else
        {
        printf("usage: rraextract [-cN] [-iNNN.NN] [-tNNN.NN] [-wNNN.NN] "
            "[<inFile>]\n");
        exit(-1);
        }

    printf("reading wave...\n");
    w = readWaveFP(in);

    if (w->NumChannels != 1)
        Fatal("can only extract from mono\n");

    extract(w);

    return 0;
    }

static void 
extract(WAVE *w)    
    {
    int i = 0;
    int run = 0;
    int *data;

    data = w->channels[0];

    while (i < w->sampleCount)
        {
        if (abs(data[i]) > startThreshold)
            {
            ++run;
            //printf("%f: %d (run is %d)\n",
            //    samplesToSeconds(i),data[i],run);
            }
        else
            {
            if (run != 0) printf("%f: %d (run is 0)\n",
                samplesToSeconds(i),data[i]);
            run = 0;
            }

        if (run > startWindowSize)
            {
            i = extractFile(w,i-startWindowSize);
            run = 0;
            }
        else
            ++i;
        }
    }

static int
extractFile(WAVE *w,int start)
    {
    static int count = 0;

    double factor;
    int rewind;
    int *data;
    int i,run;
    int finish;
    char buffer[512];
    FILE *fp;

    sprintf(buffer,"x_%d_note.rra",count);

    printf("extracting %s...\n",buffer);

    // rewind 1/100th of a second
    
    rewind = secondsToSamples(0.01);

    start -= rewind;

    if (start < 0) start = 0;

    printf("note starts %f seconds into the audio stream\n",
        samplesToSeconds(start));

    /* pull out the data for easily handling */

    data = w->channels[0];

    /* fade in the note */

    i = start;
    factor = 0.00001;
    while (factor < 1 && i < start + rewind)
        {
        printf("datum at %d was %d\n",i,data[i]);
        data[i] = (int) (data[i] * factor);
        printf("datum at %d now is %d\n",i,data[i]);
        factor *= FADE_IN;
        ++i;
        }

    // find the end of the note

    run = 0;
    finish = start;
    while (finish < w->sampleCount)
        {
        /* don't start counting until rewind is passed */
        if (finish < start + rewind) run = 0;

        if (abs(data[finish]) < finishThreshold)
            {
            ++run;
            //printf("%f: %d (run is %d)\n",
            //    samplesToSeconds(finish),data[finish],run);
            }
        else
            {
            //if (run != 0)
            //    {
            //    printf("%f: %d (run is 0)\n",
            //        samplesToSeconds(finish),data[finish]);
            //    }
            run = 0;
            }

        if (run > finishWindowSize) break;

        ++finish;
        }

    printf("note ends %f seconds later\n",samplesToSeconds(finish - start));

    if (samplesToSeconds(finish - start) < minimumSeconds)
        {
        printf("note is too short (needs to be %f seconds), discarding\n",
            minimumSeconds);
        return finish;
        }

    /* write out the header of the extracted note */

    fp = OpenFile(buffer,"w");
    fprintf(fp,"RRAUDIO\n");
    fprintf(fp,"channels: 1\n");
    fprintf(fp,"sampleRate: %d\n",w->SampleRate);
    fprintf(fp,"samples: %d\n",finish - start);
    fprintf(fp,"bitsPerSample: %d\n",w->BitsPerSample);
    fprintf(fp,"createdBy: rraextract\n");
    fprintf(fp,"%%%%\n");

    /* write out the amplitudes of the extracted note */

    i = start;
    while (i < finish-finishWindowSize)
        {
        if (i < start + 10)
            printf("datum at %d now is %d\n",i,data[i]);
        fprintf(fp,"%d\n",data[i]);
        ++i;
        }

    /* fade out the extracted note */

    factor = 1;
    while (i < finish)
        {
        fprintf(fp,"%d\n",(int)(data[i] * factor));
        factor *= FADE_OUT;
        ++i;
        }

    fclose(fp);

    ++count;

    //printf("press Enter to continue...");
    //getchar();

    return finish;
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

        fprintf(stderr,"%s \n",argv[argIndex]);

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
                printf("autoconfig not yet implemented\n");
                exit(0);

            case 'h':
                printf("rraextract options:\n");
                printf("    -a     autoconfigure thresholds\n");
                printf("    -h     display options\n");
                printf("    -mNN.N set minimum note length to NN.N seconds\n");
                printf("    -tN    set start threshold to N\n");
                printf("    -TN    set finish threshold to N\n");
                printf("    -wN    set start window size to N samples\n");
                printf("    -WN    set finish window size to N samples\n");
                break;

            case 'm':
                minimumSeconds = atof(arg);
                argUsed = 1;
                break;

            case 't':
                startThreshold = atoi(arg);
                argUsed = 1;
                break;

            case 'T':
                finishThreshold = atoi(arg);
                argUsed = 1;
                break;

            case 'w':
                startWindowSize = atoi(arg);
                argUsed = 1;
                break;

            case 'W':
                finishWindowSize = atoi(arg);
                argUsed = 1;
                break;

            /*
            case 'w':
                windowTime = atof(&argv[argIndex][3]);
                argUsed = 1;
                printf("window Time: %3.2f seconds \n",windowTime); 
                break;
            */

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

