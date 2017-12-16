/* not like autotune - pitch corrects to a set of legal notes
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

#define ST 1.059463094359

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "pitch-a-fix";
char *PROGRAM_VERSION = "0.01";

static int SampleDelta = 0;
static int SamplesRead = 0;
static int SamplesWritten = 0;
static int SamplesStored = 0;
static int SamplesRepitched = 0;
static int *Window;
static int WindowStart = 0;
static int WindowEnd = 0;
static int WindowSize = 0;
static int CycleCount = 10;
static int OctavesAllowed = 3;
static int Lag = 4;
static int SampleRate;
static int LongestCycle;
static int ShortestCycle;
static char *LegalNotes[512];
static int LegalNoteIndices[512];
static int LegalNoteCount = 0;
static double Reach = 1.0;
static double MinFactor;
static double MaxFactor;
static int AbsoluteThreshold = 0;
static double RelativeThreshold = 0;

static int LastNote = -1;
static double LastNoteBias = 1.25;
static int LastNoteCount = 0;

static void processData(FILE *,FILE *,RRA *);
static int findCycleLength(double *,int *);
static double findFactor(int,int *);
static double findAnyFactor(int,int *);
static double findLegalFactor(int,int *);
static double correlate(int);
static void repitch(FILE *,int,double,int);
static int processOptions(int, char **);
static int interpolate(double);
static void readLegalNotes(FILE *);
static void skip(int,FILE *,FILE *,RRA *);
static void advance(FILE *,FILE *,RRA *);
static int peek(int);
static int crossing(int,int *);

static double Bias = 0.5;
static int Report = 0;
static FILE *ReportFP = 0;

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

    FILE *in,*out,*nfp;
    RRA *h;

    argIndex = processOptions(argc,argv);

    if (argc-argIndex == 1)
        {
        nfp = OpenFile(argv[argIndex],"r");
        in = stdin;
        out = stdout;
        }
    else if (argc-argIndex == 2)
        {
        nfp = OpenFile(argv[argIndex],"r");
        in = OpenFile(argv[argIndex+1],"r");
        out = stdout;
        }
    else if (argc-argIndex == 3)
        {
        nfp = OpenFile(argv[argIndex],"r");
        in = OpenFile(argv[argIndex+1],"r");
        out = OpenFile(argv[argIndex+2],"w");
        }
    else
        {
        printf("usage: %s\n [-h] NOTES_FILE [INPUT_RRA [OUTPUT_RRA]\n",argv[0]);
        exit(-1);
        }

    h = newRRAHeader();
    
    readRRAHeader(in,h,0);

    SampleRate = h->sampleRate;

    if (h->channels != 1)
        Fatal("%s requires a mono audio file\n",argv[0]);

    writeRRAHeader(out,h,"modifiedBy: correct",0);

    if (Report) ReportFP = OpenFile("changes.rpt","w");

    readLegalNotes(nfp);

    LongestCycle = SampleRate / 100 * OctavesAllowed;
    ShortestCycle = SampleRate / 1000;

    MinFactor = 1 / (1 + (ST - 1) * Reach * Bias);
    MaxFactor = 1 + (ST - 1) * Reach * Bias;

    WindowSize = (CycleCount + 1) * LongestCycle;

    if (AbsoluteThreshold == 0)
        AbsoluteThreshold = (int) (pow(2,h->bitsPerSample - 1) / 200);
    if (RelativeThreshold == 0.0)
        RelativeThreshold = 0.20;

    //printf("MinFactor is %f\n",MinFactor);
    //printf("MaxFactor is %f\n",MaxFactor);
    //printf("AbsoluteThreshold is %d\n",AbsoluteThreshold);
    //getchar();

    if ((Window = malloc(sizeof(int) * WindowSize)) == 0)
        Fatal("correct: out of memory\n");

    processData(in,out,h);

    fclose(out);
    fclose(in);
    fclose(nfp);

    if (Report)
        {
        fprintf(ReportFP,"%d samples read\n",SamplesRead);
        fprintf(ReportFP,"%d samples written\n",SamplesWritten);
        fprintf(ReportFP,"%.2f%% samples repitched\n",
            100.0 * SamplesRepitched / (double) SamplesRead);
        fprintf(ReportFP,"%d samples (%.3f seconds) %s\n",
            SampleDelta,SampleDelta / (double) SampleRate,
            SampleDelta < 0? "long" : "short");
        fclose(ReportFP);
        assert(SamplesStored == 0);
        }

    return 0;
    }

static void
processData(FILE *in,FILE *out,RRA *h)
    {
    int i,cycle;
    double factor;
    int note;
    //double factor;

    /* fill the initial window */

    for (i = 0; i < WindowSize; ++i)
        {
        Window[WindowEnd]
            = readRRAAmplitude(in,out,h->bitsPerSample,outputComment);
        if (!feof(in))
            {
            ++SamplesRead;
            ++SamplesStored;
            WindowEnd = (WindowEnd+1) % WindowSize;
            }
        }

    //printf("%d samples read\n",SamplesRead);
    //printf("%d samples written\n",SamplesWritten);
    //printf("%d samples stored\n",SamplesStored);
    //printf("%d sample delta\n",SampleDelta);
    //printf("%d window size\n",WindowSize);

    assert(SamplesWritten + SamplesStored + SampleDelta == SamplesRead);

    //getchar();

    /* write out data until an upward zero crossing */
    
    advance(in,out,h);

    if (feof(in)) return;

    //printf("first is at time %d\n",SamplesRead);
    //getchar();

    while (SamplesWritten + SampleDelta != SamplesRead)
        {
        assert(SamplesWritten + SamplesStored + SampleDelta == SamplesRead);
        assert(peek(0) == 0 && peek(1) > 0);

        cycle = findCycleLength(&factor,&note);
        if (cycle == 0)
            {
            if (Report)
                fprintf(ReportFP,"no good cycle found, advancing\n");
            advance(in,out,h);
            }
        else if (factor == 0)
            {
            if (Report)
                fprintf(ReportFP,"no good factor found, advancing\n");
            advance(in,out,h);
            }
        else
            {
            repitch(out,cycle,factor,note);
            //printf("skipping %d samples\n",cycle);
            skip(cycle,in,out,h);
            }
        //printf("%d samples read\n",SamplesRead);
        //printf("%d samples written\n",SamplesWritten);
        //printf("%d samples stored\n",SamplesStored);
        //printf("%d sample delta\n",SampleDelta);
        //printf("sum: %d\n",SamplesWritten+SampleDelta+SamplesStored);
        //getchar();
        }
    }

static int
findCycleLength(double *factor,int *n)
    {
    int end;
    int peak = 0;
    int bestCycle = 0;
    int bestNote = 0;
    double bestFactor = 0;
    double error;
    double smallestError = -1;

    /*
    for (j = 0; j < WindowSize; ++j)
        fprintf(stderr,"%d: %d\n",j,Window[j]);
    */

    end = ShortestCycle;
    if (Report) fprintf(ReportFP,"shortest Cycle is %d samples\n",end);
    
    end = 0;
    peak = 0;
    while (1)
        {
        end = crossing(end+1,&peak);
        //fprintf(ReportFP,"found a crossing, %d samples, peak is %d\n",end,peak);
        if (peak < AbsoluteThreshold || end == 0 || end >= LongestCycle) break;
        if (Report)
            fprintf(ReportFP,"found a cycle, %d samples\n",end);

        *factor = findFactor(end,n);
        if (*factor == 0) continue;
        error = correlate(end);
        if (*n == LastNote) error /= LastNoteBias;
        if (Report)
            fprintf(ReportFP,"    for cycle length %d, error is %f\n",end,error);
        if (smallestError < 0 || error < smallestError)
            {
            bestFactor = *factor;
            bestNote = *n;
            bestCycle = end;
            if (Report)
                fprintf(ReportFP,"        improvement: cycle is %d (%f < %f)\n",
                    bestCycle,error,smallestError);
            smallestError = error;
            }
        //else
            //printf("not an improvement\n");
        //getchar();
        }

    if (Report)
        fprintf(ReportFP,"best cycle is %d\n",bestCycle);
    *n = bestNote;
    *factor = bestFactor;
    return bestCycle;
    }


static int
peek(int offset)
    {
    return Window[(WindowStart + offset) % WindowSize];
    }

static int 
replace(FILE *in,FILE *out,RRA *h)
    {
    int amp;
    fprintf(out,"%d\n",Window[WindowStart]);
    ++SamplesWritten;
    WindowStart = (WindowStart + 1) % WindowSize;
    --SamplesStored;
    amp = readRRAAmplitude(in,out,h->bitsPerSample,outputComment);
    if (!feof(in))
        {
        ++SamplesRead;
        ++SamplesStored;
        Window[WindowEnd] = amp;
        WindowEnd = (WindowEnd + 1) % WindowSize;
        return 1;
        }
    else
        {
        return 0;
        }
    }

static void
advance(FILE *in,FILE *out,RRA *h)
    {
    int i;
    int start = SamplesWritten;
    int more = replace(in,out,h);
    int thresholdReached = 0;
    int peak = 0;

    //printf("advancing\n");
    while (more && !(thresholdReached && peek(0) == 0 && peek(1) > 0))
        {
        if (abs(peek(0)) > peak) peak = abs(peek(0));
        if (thresholdReached == 0 && peak > AbsoluteThreshold)
            thresholdReached = 1;
        //printf("while advancing, first amp is %d\n",peek(0));
        //printf("while advancing, second amp is %d\n",peek(1));
        //printf("while advancing, thresholdReached is %d\n",thresholdReached);
        more = replace(in,out,h);
        }

    //printf("after advance, first amp is %d\n",peek(0));
    //printf("after advance, second amp is %d\n",peek(1));
    //printf("more: %d\n",more);
    //printf("thresholdReached: %d\n",thresholdReached);

    if (!more)
        {
        //printf("dumping the window, stored = %d\n",SamplesStored);
        int count = SamplesStored;
        for (i = 0; i < count; ++i)
            replace(in,out,h);
        //printf("done dumping the window, stored = %d\n",SamplesStored);
        }

    if (Report)
        fprintf(ReportFP,"no repitching of cycle with length %d, peak was %d\n",
            SamplesWritten - start,peak);
    }

static void
skip(int count,FILE *in,FILE *out,RRA *h)
    {
    int i,amp;
    for (i = 0; i < count; ++i)
        {
        WindowStart = (WindowStart + 1) % WindowSize;
        --SamplesStored;
        amp = readRRAAmplitude(in,out,h->bitsPerSample,outputComment);
        if (!feof(in))
            {
            ++SamplesRead;
            ++SamplesStored;
            Window[WindowEnd] = amp;
            WindowEnd = (WindowEnd + 1) % WindowSize;
            }
        }
    //printf("after repitching/skipping, first amp is %d\n",peek(0));
    if (feof(in))
        {
        int count = SamplesStored;
        //printf("dumping the window, stored = %d\n",SamplesStored);
        for (i = 0; i < count; ++i)
            replace(in,out,h);
        //printf("done dumping the window, stored = %d\n",SamplesStored);
        }
    }

static int
crossing(int start,int *peak)
    {
    int i;
    for (i = start; i < WindowSize-1; ++i)
        {
        if (abs(peek(i+1)) > *peak) *peak = abs(peek(i+1));
        if (peek(i) == 0 && peek(i+1) > 0)
            return i;
        }
    return 0;
    }

static double
findFactor(int cycle,int *n)
    {
    if (LegalNoteCount > 0)
        return findLegalFactor(cycle,n);
    else
        return findAnyFactor(cycle,n);
    }
       
static double
findLegalFactor(int cycle,int *n)
    {
    int i,j;
    double freq = SampleRate / (double) cycle;

    i = 0;
    while (i < LegalNoteCount)
        {
        int index = LegalNoteIndices[i];
        for (j = 1; j < OctavesAllowed + 1; ++j)
            {
            double factor = freqs[index] / (freq*j);
            if (factor > MinFactor && factor < MaxFactor)
                {
                if (Report)
                    {
                    if (j > 1) fprintf(ReportFP,
                        "found an octave multiple at %d\n",j);
                    fprintf(ReportFP,"    closest note to cycle %d is %s, "
                        "with factor %.4f\n",
                        cycle,notes[index],factor);
                    }
                *n = index;
                return factor;
                }
            }
        ++i;
        }
    if (Report)
        fprintf(ReportFP,"    no close note found\n");
    return 0;
    }

static double
findAnyFactor(int cycle,int *n)
    {
    int i;
    int best;
    double freq;
    double diff;
    double closest;
    double factor;

    /* find the closest note */

    best = -1;
    closest = -1;
    freq = SampleRate / (double) cycle;
    for (i = 0; i < sizeof(freqs) / sizeof(double); ++i)
        {
        diff = fabs(freqs[i] - freq);
        double factor = freqs[i] / freq;
        if (factor > MinFactor && factor < MaxFactor)
            {
            if (closest < 0 || diff < closest)
                {
                closest = diff;
                best = i;
                }
            }
        }


    if (best < 0)
        return 0;
    else
        {
        factor = freqs[best] / freq;
        *n = best;
        if (Report)
            fprintf(ReportFP,"possible repitch: cycle %d, "
                "from %f cps to %f cps(%s), factor is %f\n",
                cycle,freq,freqs[best],notes[best],factor);
        return factor;
        }
    }

static double
correlate(int end)
    {
    int i,j;
    double sum;

    sum = 0;
    for (i = 1; i <= CycleCount; ++i)
        {
        for (j = 0; j < end; ++j)
            sum += pow(peek(i*end + j) - peek(j),2);
        }

    return pow(sum / (CycleCount * (1.0 * end)),0.5);
    }

#define GAIN 1.00003
static void
repitch(FILE *out,int cycle,double factor,int note)
    {
    int i;
    int steps;
    double step,spot;

    //if (LastNote >= 0) fprintf(ReportFP,"last note was %s\n",notes[LastNote]);
    //fprintf(ReportFP,"factor was %f\n",factor);
    if (LastNote == 0)
        {
        LastNote = note;
        LastNoteCount = Lag;
        }
    else if (note == LastNote && LastNoteCount > 0)
        {
        factor = 1;
        --LastNoteCount;
        }
    else if (note != LastNote)
        {
        LastNote = note;
        LastNoteCount = Lag;
        factor = 1;
        }
    //fprintf(ReportFP,"factor now is %f\n",factor);

    steps =(int) (cycle / factor + 0.5);
    step = cycle / (double) steps;

    if (Report)
        fprintf(ReportFP,"repitching to %3s, from %d samples to %d samples, "
            "factor is %f\n",notes[note],cycle,steps,factor);

    fprintf(out,"%d\n",0);
    ++SamplesWritten;

    spot = step;
    for (i = 1; i < steps; ++i)
        {
        fprintf(out,"%d\n",(int) interpolate(spot));
        ++SamplesWritten;
        spot += step;
        }

    if (factor != 1.000)
        SamplesRepitched += cycle;
    SampleDelta += (cycle - steps);
    assert((int)(spot+0.5) == cycle);
    }

static int
interpolate(double spot)
    {
    int index = (int) spot;
    int low = peek(index);
    int high = peek(index + 1);
    return (int)(low + (spot - index) * (high - low));
    }
      
static void
readLegalNotes(FILE *fp)
    {
    char *note;

    fscanf(fp,"%ms",&note);
    LegalNoteCount = 0;
    while (!feof(fp))
        {
        //fprintf(stderr,"read note: %s\n",note);
        LegalNotes[LegalNoteCount++] = note;
        fscanf(fp,"%ms",&note);
        }

    int i,j;
    for (i = 0; i < LegalNoteCount; ++i)
        {
        int found = 0;
        for (j = 0; !found && j < sizeof(freqs) / sizeof(double); ++j)
            {
            //fprintf(stderr,"comparing %s with %s\n",LegalNotes[i],notes[j]);
            if (strcmp(LegalNotes[i],notes[j]) == 0)
                {
                LegalNoteIndices[i] = j;
                //printf("legal note %s, %f cps, %d samples\n",
                //    notes[j],freqs[j],(int)(SampleRate/freqs[j]));
                found = 1;
                }
            }
        if (!found)
            Fatal("note %s is not a legal note\n",LegalNotes[i]);
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
                break;
            case 'b':
                Bias = atof(arg);
                argUsed = 1;
                break;
            case 'l':
                Lag = atoi(arg);
                argUsed = 1;
                break;
            case 'o':
                OctavesAllowed = atoi(arg);
                argUsed = 1;
                break;
            case 'r':
                Reach = atof(arg);
                argUsed = 1;
                break;
            case 'R':
                Report = 1;
                break;
            case 'h':
printf("pitch-a-fix usage:\n");
printf("  pitch-a-fix NOTES_FILE [INFILE [OUTFILE]]\n");
printf("pitch-a-fix options:\n");
printf("  -b N.N set the bias to N.N\n");
printf("         a bias above one will preferentially choose higher notes\n");
printf("         a below one bias will preferentially choose lower notes\n");
printf("         default is %f\n",Bias);
printf("  -l N   set the lag to N\n");
printf("         A lag of 3 means pitch correction will start on the fourth\n");
printf("         cycle in a row of a particular legal note\n");
printf("         smaller lag (and larger reach) sounds like Autotune\n");
printf("         default is %d\n",Lag);
printf("  -o N   set the number of allowed octaves to N\n");
printf("         a value greater than one will mean that cycles with\n");
printf("         frequencies that are an integral multiple (up to N) of\n");
printf("         legal notes will be considered for pitch correction\n");
printf("         default is %d\n",OctavesAllowed);
printf("  -R     generate a report (pitch-a-fix.rpt)\n");
printf("  -r N.N set the pitch correction reach to N.N semitones\n");
printf("         a setting of 1 means cycles more than one\n");
printf("         semitone away will not be pitch corrected\n");
printf("         larger reach (and smaller lag) sounds like Autotune\n");
printf("         default is %f\n",Reach);
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

