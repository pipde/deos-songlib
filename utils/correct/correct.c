/* like autotune (pitch corrects to the closest note)
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <limits.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/wave.h"
#include "/usr/local/include/songlib/rra.h"

extern int interp3(int *,int,double);

#define ST 1.059463094359

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "correct";
char *PROGRAM_VERSION = "0.01";

static int ExtraSamples = 0;
static int *Window;
static int WindowSize = 1000;
static int WindowStart = 0;
static double WindowSpot = 0.0;
static int CorrelationSize = 100;
static int Passes = 3;
static double Bias = 1.0;
static char *LegalNotes[512];
static int LegalNoteOffsets[512];
static int LegalNoteCount = 0;
static double MinFreq = 0;
static double MaxFreq = 0;
static int SampleRate = 0;
static int QuietLevel = 300;

#define MAX_ERROR (275 * 275)

static double Factor = 0;

static void processData(FILE *,FILE *,RRA *);
static int findCycleLength(void);
static double findFactor(int,RRA *);
static double correlate(int,int,int);
static void repitch(FILE *,double);
static int processOptions(int, char **);
static void readLegalNotes(char *);

double freqs[] =
    {
    13.75,
    13.75 * ST,
    13.75 * ST*ST,
    13.75 * ST*ST*ST,
    13.75 * ST*ST*ST*ST,
    13.75 * ST*ST*ST*ST*ST,
    13.75 * ST*ST*ST*ST*ST*ST,
    13.75 * ST*ST*ST*ST*ST*ST*ST,
    13.75 * ST*ST*ST*ST*ST*ST*ST*ST,
    13.75 * ST*ST*ST*ST*ST*ST*ST*ST*ST,
    13.75 * ST*ST*ST*ST*ST*ST*ST*ST*ST*ST,
    13.75 * ST*ST*ST*ST*ST*ST*ST*ST*ST*ST*ST,
    27.5,
    27.5 * ST,
    27.5 * ST*ST,
    27.5 * ST*ST*ST,
    27.5 * ST*ST*ST*ST,
    27.5 * ST*ST*ST*ST*ST,
    27.5 * ST*ST*ST*ST*ST*ST,
    27.5 * ST*ST*ST*ST*ST*ST*ST,
    27.5 * ST*ST*ST*ST*ST*ST*ST*ST,
    27.5 * ST*ST*ST*ST*ST*ST*ST*ST*ST,
    27.5 * ST*ST*ST*ST*ST*ST*ST*ST*ST*ST,
    27.5 * ST*ST*ST*ST*ST*ST*ST*ST*ST*ST*ST,
    55,
    55 * ST,
    55 * ST*ST,
    55 * ST*ST*ST,
    55 * ST*ST*ST*ST,
    55 * ST*ST*ST*ST*ST,
    55 * ST*ST*ST*ST*ST*ST,
    55 * ST*ST*ST*ST*ST*ST*ST,
    55 * ST*ST*ST*ST*ST*ST*ST*ST,
    55 * ST*ST*ST*ST*ST*ST*ST*ST*ST,
    55 * ST*ST*ST*ST*ST*ST*ST*ST*ST*ST,
    55 * ST*ST*ST*ST*ST*ST*ST*ST*ST*ST*ST,
    110,
    110 * ST,
    110 * ST*ST,
    110 * ST*ST*ST,
    110 * ST*ST*ST*ST,
    110 * ST*ST*ST*ST*ST,
    110 * ST*ST*ST*ST*ST*ST,
    110 * ST*ST*ST*ST*ST*ST*ST,
    110 * ST*ST*ST*ST*ST*ST*ST*ST,
    110 * ST*ST*ST*ST*ST*ST*ST*ST*ST,
    110 * ST*ST*ST*ST*ST*ST*ST*ST*ST*ST,
    110 * ST*ST*ST*ST*ST*ST*ST*ST*ST*ST*ST,
    220,
    220 * ST,
    220 * ST*ST,
    220 * ST*ST*ST,
    220 * ST*ST*ST*ST,
    220 * ST*ST*ST*ST*ST,
    220 * ST*ST*ST*ST*ST*ST,
    220 * ST*ST*ST*ST*ST*ST*ST,
    220 * ST*ST*ST*ST*ST*ST*ST*ST,
    220 * ST*ST*ST*ST*ST*ST*ST*ST*ST,
    220 * ST*ST*ST*ST*ST*ST*ST*ST*ST*ST,
    220 * ST*ST*ST*ST*ST*ST*ST*ST*ST*ST*ST,
    440,
    440 * ST,
    440 * ST*ST,
    440 * ST*ST*ST,
    440 * ST*ST*ST*ST,
    440 * ST*ST*ST*ST*ST,
    440 * ST*ST*ST*ST*ST*ST,
    440 * ST*ST*ST*ST*ST*ST*ST,
    440 * ST*ST*ST*ST*ST*ST*ST*ST,
    440 * ST*ST*ST*ST*ST*ST*ST*ST*ST,
    440 * ST*ST*ST*ST*ST*ST*ST*ST*ST*ST,
    440 * ST*ST*ST*ST*ST*ST*ST*ST*ST*ST*ST,
    880,
    880 * ST,
    880 * ST*ST,
    880 * ST*ST*ST,
    880 * ST*ST*ST*ST,
    880 * ST*ST*ST*ST*ST,
    880 * ST*ST*ST*ST*ST*ST,
    880 * ST*ST*ST*ST*ST*ST*ST,
    880 * ST*ST*ST*ST*ST*ST*ST*ST,
    880 * ST*ST*ST*ST*ST*ST*ST*ST*ST,
    880 * ST*ST*ST*ST*ST*ST*ST*ST*ST*ST,
    880 * ST*ST*ST*ST*ST*ST*ST*ST*ST*ST*ST,
    1760,
    1760 * ST,
    1760 * ST*ST,
    1760 * ST*ST*ST,
    1760 * ST*ST*ST*ST,
    1760 * ST*ST*ST*ST*ST,
    1760 * ST*ST*ST*ST*ST*ST,
    1760 * ST*ST*ST*ST*ST*ST*ST,
    1760 * ST*ST*ST*ST*ST*ST*ST*ST,
    1760 * ST*ST*ST*ST*ST*ST*ST*ST*ST,
    1760 * ST*ST*ST*ST*ST*ST*ST*ST*ST*ST,
    1760 * ST*ST*ST*ST*ST*ST*ST*ST*ST*ST*ST,
    3520,
    3520 * ST,
    3520 * ST*ST,
    3520 * ST*ST*ST,
    3520 * ST*ST*ST*ST,
    3520 * ST*ST*ST*ST*ST,
    3520 * ST*ST*ST*ST*ST*ST,
    3520 * ST*ST*ST*ST*ST*ST*ST,
    3520 * ST*ST*ST*ST*ST*ST*ST*ST,
    3520 * ST*ST*ST*ST*ST*ST*ST*ST*ST,
    3520 * ST*ST*ST*ST*ST*ST*ST*ST*ST*ST,
    3520 * ST*ST*ST*ST*ST*ST*ST*ST*ST*ST*ST,
    };

char *notes[] =
    {
    "ax", "a#x", "bx", "c0", "c#0", "d0", "d#0", "e0", "f0", "f#0", "g0", "g#0",
    "a0", "a#0", "b0", "c1", "c#1", "d1", "d#1", "e1", "f1", "f#1", "g1", "g#1",
    "a1", "a#1", "b1", "c2", "c#2", "d2", "d#2", "e2", "f2", "f#2", "g2", "g#2",
    "a2", "a#2", "b2", "c3", "c#3", "d3", "d#3", "e3", "f3", "f#3", "g3", "g#3",
    "a3", "a#3", "b3", "c4", "c#4", "d4", "d#4", "e4", "f4", "f#4", "g4", "g#4",
    "a4", "a#4", "b4", "c5", "c#5", "d5", "d#5", "e5", "f5", "f#5", "g5", "g#5",
    "a5", "a#5", "b5", "c6", "c#6", "d6", "d#6", "e6", "f6", "f#6", "g6", "g#6",
    "a6", "a#6", "b6", "c7", "c#7", "d7", "d#7", "e7", "f7", "f#7", "g7", "g#7",
    "a7", "a#7", "b7", "c8", "c#8", "d8", "d#8", "e8", "f8", "f#8", "g8", "g#8",
    "a8", "a#8", "b8", "c9", "c#9", "d9", "d#9", "e9", "f9", "f#9", "g9", "g#9",
    "a9", "a#9", "b9", "c10", "c#10", "d10", "d#10", "e10", "f10", "f#10", "g10", "g#10",
    };

int
main(int argc,char **argv)
    {
    int argIndex = 1;

    FILE *in,*out;
    RRA *h;

    argIndex = processOptions(argc,argv);

    if (argc-argIndex == 0)
        {
        in = stdin;
        readLegalNotes("notes");
        out = stdout;
        }
    else if (argc-argIndex == 1)
        {
        in = stdin;
        readLegalNotes(argv[argIndex]);
        out = stdout;
        }
    else if (argc-argIndex == 2)
        {
        in = OpenFile(argv[argIndex],"r");
        readLegalNotes(argv[argIndex+1]);
        out = stdout;
        }
    else if (argc-argIndex == 3)
        {
        in = OpenFile(argv[argIndex],"r");
        readLegalNotes(argv[argIndex+1]);
        out = OpenFile(argv[argIndex+2],"w");
        }
    else
        {
        printf("usage: %s\n",argv[0]);
        printf("       %s <legal notes file>\n",argv[0]);
        printf("       %s <input rra file> <legal notes file>\n",argv[0]);
        printf("       %s <input rra file> <legal notes file> <output rra file>\n",argv[0]);
        exit(-1);
        }

    h = newRRAHeader();
    
    readRRAHeader(in,h,0);

    SampleRate = h->sampleRate;

    if (h->channels != 1)
        Fatal("%s requires a mono audio file\n",argv[0]);

    //printf("%f,%f\n",pow(2,h->bitsPerSample-1)-1,pow(Factor,thresholdDb));

    writeRRAHeader(out,h,"modifiedBy: correct",0);

    processData(in,out,h);

    fclose(in);
    fclose(out);

    fprintf(stderr,"Extra samples: %d (%f seconds)\n",
        ExtraSamples,ExtraSamples / (double) h->sampleRate);

    return 0;
    }

static void
readLegalNotes(char *fileName)
    {
    if (fileName == 0) return;

    FILE *fp = OpenFile(fileName,"r");
    char *note;

    fscanf(fp,"%ms",&note);
    LegalNoteCount = 0;
    while (!feof(fp))
        {
        fprintf(stderr,"read note: %s\n",note);
        LegalNotes[LegalNoteCount++] = note;
        fscanf(fp,"%ms",&note);
        }

    int i,j;
    for (i = 0; i < LegalNoteCount; ++i)
        {
        int found = 0;
        for (j = 0; !found && j < sizeof(freqs) / sizeof(double); ++j)
            {
            fprintf(stderr,"comparing %s with %s\n",LegalNotes[i],notes[j]);
            if (strcmp(LegalNotes[i],notes[j]) == 0)
                {
                LegalNoteOffsets[i] = j % 12;
                if (MinFreq == 0 || freqs[j] < MinFreq) MinFreq = freqs[j];
                if (MaxFreq == 0 || freqs[j] > MaxFreq) MaxFreq = freqs[j];
                found = 1;
                }
            }
        if (!found)
            Fatal("note %s is not a legal note\n",LegalNotes[i]);
        }
    }

static void
processData(FILE *in,FILE *out,RRA *h)
    {
    int i,amp;
    int more = 1;
    int maxAmplitude = 0;
    static int windowcount = 0;

    if ((Window = malloc(sizeof(int) * WindowSize)) == 0)
        Fatal("correct: out of memory\n");

    WindowStart = 0;
    while (more)
        {
        maxAmplitude = 0;
        /* fill the window */
        fprintf(stderr,"window %d: ",windowcount++);
        amp = readRRAAmplitude(in,out,h->bitsPerSample,outputComment);
        if (amp > maxAmplitude) maxAmplitude = amp;
        for (i = WindowStart; i < WindowSize; ++i)
            {
            if (feof(in))
                {
                Window[i] = 0;
                more = 0;
                }
            else
                {
                Window[i] = amp;
                }
            amp = readRRAAmplitude(in,out,h->bitsPerSample,outputComment);
            if (amp > maxAmplitude) maxAmplitude = amp;
            }
        ExtraSamples += WindowSize;
        int cycle = findCycleLength();
        if (cycle < 25)
            {
            fprintf(stderr,"no matching cycle: ");
            repitch(out,1.0);
            }
        else
            {
            repitch(out,findFactor(cycle,h));
            }
        }

    free(Window);
    }

static double
findFactor(int cycle,RRA *h)
    {
    int i,j;
    int best;
    double freq = (h->sampleRate / cycle) * Bias;
    double diff;
    double closest = 0;
    double factor;

    best = 0;
    for (i = 1; i < sizeof(freqs) / sizeof(double); ++i)
        {
        int found = 0;
        for (j = 0; !found && j < LegalNoteCount; ++j)
            if (i % 12 == LegalNoteOffsets[j])
                found = 1;
        if (!found) continue;
         
        diff = fabs(freqs[i] - freq);
        if (closest == 0 || diff < closest)
            {
            closest = diff;
            best = i;
            }
        }

    fprintf(stderr,"matching frequency to %f (%d spc) is %f, %s: ",
        freq,cycle,freqs[best],notes[best]);
    //getchar();

    factor = freqs[best] / freq;
        
    return factor;
    }

static int
findCycleLength(void)
    {
    int j,start;
    int bestCycle = 0;
    double error = 0;
    double smallestError = -1;
    int smallestCycle = 0.90 * SampleRate / MaxFreq;
    int largestCycle = 1.10 * SampleRate / MinFreq;
    int maxAmplitude;

    fprintf(stderr,"smallest cycle allowed: %d\n",smallestCycle);
    fprintf(stderr,"largest cycle allowed: %d\n",largestCycle);
    /*
    for (j = 0; j < WindowSize; ++j)
        fprintf(stderr,"%d: %d\n",j,Window[j]);
    */

    start = 0;
    for (j = 0; j < Passes; ++j)
        {
        fprintf(stderr,"Pass %d...\n",j);
        /* skip to first zero (upward) past start */
        int i = start + 1;
        while (i < WindowSize)
            {
            //printf("amp = %d\n",Window[i-1]);
            if (Window[i-1] < 0 && Window[i] >= 0) break;
            ++i;
            }
        start = i;

        //printf("amp = %d\n",Window[i]);
        fprintf(stderr,"found cycle start at location %d!\n",i);
        //getchar();

        while (1)
            {
            /* find the next zero (upward) */
            ++i;
            while (i < WindowSize-CorrelationSize)
                {
                //fprintf(stderr,"amp = %d\n",Window[i-1]);
                if (abs(Window[i-1]) > maxAmplitude)
                    maxAmplitude = abs(Window[i-1]);
                if (Window[i-1] < 0 && Window[i] >= 0) break;
                ++i;
                }
            if (abs(Window[i]) > maxAmplitude)
                maxAmplitude = abs(Window[i]);

            if (i == WindowSize - CorrelationSize) break;
            if (i - start > largestCycle) break;
            if (i - start < smallestCycle) continue;
            if (maxAmplitude < QuietLevel) continue;
            fprintf(stderr,"found cycle end at location %d!\n",i);
            fprintf(stderr,"max amplitude is %d\n",maxAmplitude);

            //printf("amp = %d\n",Window[i]);
            //fprintf(stderr,"found possible cycle end: %d samples!\n",i - start);
            //getchar();
            error = correlate(start,i,CorrelationSize);
            //fprintf(stderr,"for pass %d, cycle length %d, error is %f\n",j,i-start,error);
            if ((smallestError < 0 || error < smallestError))
                {
                bestCycle = i - start;
                fprintf(stderr,"improvement: cycle is %d (%f < %f)\n",
                    bestCycle,error,smallestError);
                smallestError = error;
                }
            //getchar();
            }
        }

    //if (smallestError > MAX_ERROR)
    //    return 0;
    //else
        return bestCycle;
    }

static double
correlate(int start,int offset,int length)
    {
    int i;
    double sum;

    sum = 0;
    for (i = 0; i < length; ++i)
        {
        //printf("   comparing %d with %d\n",Window[start+i],Window[offset+i]);
        //getchar();
        sum += pow(Window[start+i] - Window[offset + i],2);
        }

    return sum / length;
    }

#define GAIN 1.00003
static void
repitch(FILE *out,double rfactor)
    {
    int reached = 0;

    if (Factor == 0) Factor = rfactor;

    fprintf(stderr,"old factor is %f, new factor is %f\n",Factor,rfactor);
    //getchar();

    //if (rfactor == 1.0)
    //    Factor = rfactor;
    //else
    //    Factor = (Factor + rfactor) / 2;
    //Factor = (2 * Factor + rfactor) / 3.0;
    while (WindowSpot < WindowSize - 1)
        {
        //printf("Spot is %f, Factor is %f\n",WindowSpot,Factor);
        //getchar();
        fprintf(out,"%d\n",(int) interp3(Window,WindowSize,WindowSpot));
        --ExtraSamples;

        WindowSpot += Factor; 
        if (Factor < rfactor)
            {
            Factor *= GAIN;
            if (Factor >= rfactor)
                {
                Factor = rfactor;
                if (!reached)
                    {
                    fprintf(stderr,"repitch factor %f reached\n",rfactor);
                    reached = 1;
                    }
                }
            }
        else
            {
            Factor /= GAIN;
            if (Factor <= rfactor)
                {
                Factor = rfactor;
                if (!reached)
                    {
                    fprintf(stderr,"repitch factor %f reached\n",rfactor);
                    reached = 1;
                    }
                }
            }
        }

    if (!reached) fprintf(stderr,"repitch factor %f NOT reached\n",rfactor);
    if (!reached) fprintf(stderr,"current factor is %f\n",Factor);
    WindowSpot = WindowSpot - (int) WindowSpot;
    Window[0] = Window[WindowSize - 1];
    WindowStart = 1;
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
            case 'w':
                WindowSize = atoi(arg);
                argUsed = 1;
                break;
            case 'b':
                Bias = atof(arg);
                argUsed = 1;
                break;
            case 'c':
                CorrelationSize = atoi(arg);
                argUsed = 1;
                break;
            case 'p':
                Passes = atoi(arg);
                argUsed = 1;
                break;
            case 'h':
                printf("correct usage:\n");
                printf("  rracorrect [OPTIONS] [INFILE [OUTFILE]]\n");
                printf("correct options:\n");
                printf("  -b N.N set the bias to N.N\n");
                printf("         a bias above one will preferentially shift to higher notes\n");
                printf("         a below one bias will preferentially shift to lower notes\n");
                printf("         default is %.1f\n",Bias);
                printf("  -c N   set the correlation size to N\n");
                printf("         default is %d\n",CorrelationSize);
                printf("  -p N   set the number of passes looking for cycle starts to N\n");
                printf("         default is %d\n",Passes);
                printf("  -w N   set the window size to N\n");
                printf("         default is %d\n",WindowSize);
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

