/* generic filter for resampling
 *
 *identifyz in.rra
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

//#define WRITE_ERROR 1

#define OFFSET  0.15
#define WINDOW  2000
#define SPAN    4

#define STEP 1.0594630944

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "identifyz";
char *PROGRAM_VERSION = "0.01";

static double initialOffset = OFFSET;
static int Pitch = 0;
static int Note = 0;
static int Frequency = 0;
static double MinFrequency = 50;
static int smoothings = 20;

static int *window;
static int *zeros;
static int windowSize;

static void smooth(void);
static int processOptions(int, char **);

double freqs[] =
    {
    13.75,
    13.75 * STEP,
    13.75 * STEP*STEP,
    13.75 * STEP*STEP*STEP,
    13.75 * STEP*STEP*STEP*STEP,
    13.75 * STEP*STEP*STEP*STEP*STEP,
    13.75 * STEP*STEP*STEP*STEP*STEP*STEP,
    13.75 * STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    13.75 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    13.75 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    13.75 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    13.75 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    27.5,
    27.5 * STEP,
    27.5 * STEP*STEP,
    27.5 * STEP*STEP*STEP,
    27.5 * STEP*STEP*STEP*STEP,
    27.5 * STEP*STEP*STEP*STEP*STEP,
    27.5 * STEP*STEP*STEP*STEP*STEP*STEP,
    27.5 * STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    27.5 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    27.5 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    27.5 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    27.5 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    55,
    55 * STEP,
    55 * STEP*STEP,
    55 * STEP*STEP*STEP,
    55 * STEP*STEP*STEP*STEP,
    55 * STEP*STEP*STEP*STEP*STEP,
    55 * STEP*STEP*STEP*STEP*STEP*STEP,
    55 * STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    55 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    55 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    55 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    55 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    110,
    110 * STEP,
    110 * STEP*STEP,
    110 * STEP*STEP*STEP,
    110 * STEP*STEP*STEP*STEP,
    110 * STEP*STEP*STEP*STEP*STEP,
    110 * STEP*STEP*STEP*STEP*STEP*STEP,
    110 * STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    110 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    110 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    110 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    110 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    220,
    220 * STEP,
    220 * STEP*STEP,
    220 * STEP*STEP*STEP,
    220 * STEP*STEP*STEP*STEP,
    220 * STEP*STEP*STEP*STEP*STEP,
    220 * STEP*STEP*STEP*STEP*STEP*STEP,
    220 * STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    220 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    220 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    220 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    220 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    440,
    440 * STEP,
    440 * STEP*STEP,
    440 * STEP*STEP*STEP,
    440 * STEP*STEP*STEP*STEP,
    440 * STEP*STEP*STEP*STEP*STEP,
    440 * STEP*STEP*STEP*STEP*STEP*STEP,
    440 * STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    440 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    440 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    440 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    440 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    880,
    880 * STEP,
    880 * STEP*STEP,
    880 * STEP*STEP*STEP,
    880 * STEP*STEP*STEP*STEP,
    880 * STEP*STEP*STEP*STEP*STEP,
    880 * STEP*STEP*STEP*STEP*STEP*STEP,
    880 * STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    880 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    880 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    880 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    880 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    1760,
    1760 * STEP,
    1760 * STEP*STEP,
    1760 * STEP*STEP*STEP,
    1760 * STEP*STEP*STEP*STEP,
    1760 * STEP*STEP*STEP*STEP*STEP,
    1760 * STEP*STEP*STEP*STEP*STEP*STEP,
    1760 * STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    1760 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    1760 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    1760 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    1760 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    3520,
    3520 * STEP,
    3520 * STEP*STEP,
    3520 * STEP*STEP*STEP,
    3520 * STEP*STEP*STEP*STEP,
    3520 * STEP*STEP*STEP*STEP*STEP,
    3520 * STEP*STEP*STEP*STEP*STEP*STEP,
    3520 * STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    3520 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    3520 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    3520 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
    3520 * STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP*STEP,
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

    int i,index;
    FILE *in,*out;
    char *source = 0;
    RRA *h;
    static int skip;

    argIndex = processOptions(argc,argv);

    if (argc-argIndex == 0)
        {
        in = stdin;
        out = stdout;
        }
    else if (argc-argIndex == 1)
        {
        source = argv[argIndex];
        in = OpenFile(source,"r");
        out = stdout;
        }
    else if (argc-argIndex == 2)
        {
        source = argv[argIndex];
        in = OpenFile(source,"r");
        out = OpenFile(argv[argIndex+1],"w");
        }
    else
        {
        printf("usage: %s [<input rra file> [<output wave file>]]\n",argv[0]);
        exit(-1);
        }

    h = newRRAHeader();
    
    readRRAHeader(in,h,0);

    if (h->channels != 1)
        Fatal("%s requires a mono audio file\n",argv[0]);

    /* allocate window and zeros */
    windowSize = (h->sampleRate / MinFrequency) * 10;
    window = malloc(sizeof(int) * windowSize);
    zeros = malloc(sizeof(int) * windowSize);
    if (window == 0 || zeros == 0) Fatal("out of memory\n");

    /* skip over the first part */

    skip = initialOffset * h->sampleRate;
    printf("skip is %d\n",skip);
    for (i = 0; i < skip; ++i)
        (void) readRRAAmplitude(in,out,h->bitsPerSample,outputComment);
        printf("skipping %d\n",readRRAAmplitude(in,out,h->bitsPerSample,outputComment));
    //printf("skipping %d\n",readRRAAmplitude(in,out,h->bitsPerSample,outputComment));

    /* read in a segment */

    printf("windowSize is %d\n",windowSize);
    for (i = 0; i < windowSize && !feof(in); ++i)
        {
        window[i] = readRRAAmplitude(in,out,h->bitsPerSample,outputComment);
        //printf("window[%d] is %d\n",i,window[i]);
        }

    printf("smoothing...\n");
    /* heavily smooth the data */
    for (i = 0; i < smoothings; ++i)
        smooth();

    FILE *fp = fopen("smoothed.rra","w");
    fprintf(fp,"RRAUDIO\n");
    fprintf(fp,"%%%%\n");
    for (i = 0; i < windowSize; ++i)
        fprintf(fp,"%d\n",window[i]);
    fclose(fp);

    printf("finding first upward zero crossing\n");
    /* find the first upward zero crossing */
    for (i = 100; i < windowSize-1; ++i)
        if (window[i] < 0 && window[i+1] >= 0)
            break;

    printf("zero at %d\n",i);
    printf("finding remaining upward zero crossings\n");
    /* find remaining upward zero crossings, recording length and count */
    int count = 0;
    int last = i;
    for (++i; i < windowSize-200; ++i)
        {
        if (window[i] < 0 && window[i+1] >= 0)
            {
            zeros[count++] = i - last;
            printf("zeros at %d = %d\n",count-1,zeros[count-1]);
            last = i;
            }
        }

    printf("computing average and score\n");
    /* compute the average and stddev */
    double total = 0.0;
    for (i = 0; i < count; ++i)
        total += zeros[i];
    printf("count is %d\n",count);
    printf("total is %f\n",total);
    double avg = total / count;
    total = 0.0;
    for (i = 0; i < count; ++i)
        total += pow(zeros[i]-avg,2);
    double stddev = pow(total / count,0.5);
    double freq = h->sampleRate / avg;

    printf("avg is %f\n",avg);

    index = -1;
    for (i = 0; index < 0 && i < 120; ++i)
        {
        if (freq >= freqs[i] && freq < freqs[i+1])
            {
            if (freq - freqs[i] >= freqs[i+1] - freq)
                index = i+1;
            else
                index = i;
            }
        }

    printf("index is %d\n",index);
    if (Pitch && source != 0)
        printf("rratune %f %s %s.repitch\n",
            freqs[index] / freq,source,source);
    else if (Pitch)
        printf("rratune %f\n", freqs[index] / freq);
    else if (Note)
        printf("%s\n", notes[index]);
    else if (Frequency)
        printf("%f\n", freq);
    else
        printf("closest note to %f is %s (freq = %f) -- error is %f\n",
            freq,notes[index],freqs[index],stddev);
            
    fclose(in);
    fclose(out);

    return 0;
    }

static void
smooth()
    {
    int i;
    for (i = 4; i < windowSize-4; ++i)
        {
        window[i] = (
            32*window[i-4] + 24*window[i-3] + 8*window[i-2] + 4*window[i-1] +
            window[i] +
            4*window[i+3] + 2*window[i+2]
            ) / 75;
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
            case 'p':
                Pitch = 1;
                break;
            case 'f':
                Frequency = 1;
                break;
            case 'n':
                Note = 1;
                break;
            case 'o':
                initialOffset = atof(arg);
                argUsed = 1;
                break;
            case 'h':
                printf("identifyz options:\n");
                printf("  -f     only report the detected frequency\n");
                printf("  -p     generate a command to repitch the note\n");
                printf("  -n     only report the nearest note\n");
                printf("  -o N   start analyzing N seconds into the note\n");
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

