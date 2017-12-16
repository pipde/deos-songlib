/*
 * zero
 *
 * make zero crossings hit zero exactly
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/songlib.h"
#include "/usr/local/include/songlib/rra.h"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "zero";
char *PROGRAM_VERSION = "0.01";

#define SIZE 10
#define SIZE2 (2*SIZE+1)

static int Window[SIZE2];
static int Remaining = 1; /* 1 allows the initial read to happen */
static int Spot = -1;     /* -1 indicates Window needs to be filled */

static double Amplify = 1;
static double Factor = 0.5;

static int processOptions(int,char **);
static void adjustPrevious(int);
static void adjustCurrent(int);
static int readAmplitude(FILE *,FILE *,RRA *);
static void writeAmplitude(FILE *,RRA *);

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
        out = stdout;
        }
    else if (argc-argIndex == 1)
        {
        in = OpenFile(argv[argIndex],"r");
        out = stdout;
        }
    else if (argc-argIndex == 2)
        {
        in = OpenFile(argv[argIndex],"r");
        out = OpenFile(argv[argIndex+1],"w");
        }
    else
        {
        printf("usage: %s -fN [<input rra file> [<output rra file>]]\n",
            argv[0]);
        exit(-1);
        }

    h = newRRAHeader();
    
    readRRAHeader(in,h,0);

    if (h->channels != 1)
        Fatal("%s requires a mono audio file\n",argv[0]);

    writeRRAHeader(out,h,"modifiedBy: zero",0);

    int curr = 0;
    int prev;
    int i = 0;
    while (Remaining > 0)
        {
        prev = curr;
        curr = readAmplitude(in,out,h);
        if (prev < 0 && curr > 0)
            {
            //fprintf(stderr,"crossing up at %d\n",i);
            if (abs(prev) < curr)
                adjustPrevious(-prev);
            else
                adjustCurrent(curr);
            curr = 0;
            }
        writeAmplitude(out,h);
        ++i;
        }

    fclose(in);
    fclose(out);

    return 0;
    }

static int
readAmplitude(FILE *in,FILE *out,RRA *h)
    {
    int i,amp;

    if (Spot == -1)
        {
        Remaining = SIZE;
        /* fill the window */
        for (i = 0; i < SIZE; ++i)
            Window[i] = readRRAAmplitude(in,out,h->bitsPerSample,outputComment);
        }

    ++Spot;
    amp = Window[Spot%SIZE2];
    //printf("reading index %d (%d): %d\n",Spot,Spot%SIZE2,amp);

    Window[(Spot+SIZE)%SIZE2] = 
            readRRAAmplitude(in,out,h->bitsPerSample,outputComment);
    //printf("placing %d at index %d (%d)\n",
    //    Window[(Spot+SIZE)%SIZE2],Spot+SIZE,(Spot+SIZE)%SIZE2);
    //getchar();

    if (feof(in)) --Remaining;
       
    return amp;
    }

static void
writeAmplitude(FILE *out,RRA *h)
    {
    if (Spot >= SIZE)
        {
        int index = (Spot-SIZE) % SIZE2;
        //fprintf(out,"writing index %d (%d): ",Spot-SIZE,(Spot-SIZE)%SIZE2);
        fprintf(out,"%d\n",(int)(Amplify * Window[index]));
        }
    }

static void
adjustPrevious(int amt)
    {
    int i;
    int delta;

    //adjust previous values upwards
    
    //printf("adjusting previous index %d (%d)\n",Spot-1,(Spot-1+SIZE2)%SIZE2);

    Window[(Spot-1+SIZE2)%SIZE2] = 0;
    
    /* adjust the previous values */
    i = 1;
    delta = (int)(amt * Factor);
    //printf("delta is %d\n",delta);
    while (abs(delta) > 0)
        {
        if (i >= SIZE) break;
        if (Spot-i-1 < 0) break;
        //printf("delta is %d\n",delta);
        if (Window[(Spot-i-1+SIZE2)%SIZE2] == 0) break;
        //printf("changing index %d from %d ",
        //    Spot-i-1,Window[(Spot-i-1+SIZE2)%SIZE2]);
        Window[(Spot-i-1+SIZE2)%SIZE2] += delta;
        if (Window[(Spot-i-1+SIZE2)%SIZE2] == 0)
            Window[(Spot-i-1+SIZE2)%SIZE2] = 1;
        //printf("to %d\n",Window[(Spot-i-1+SIZE2)%SIZE2]);
        //getchar();
        ++i;
        delta = (int)(delta * Factor);
        }

    /* adjust the future values */
    i = 1;
    delta = (int)(amt * Factor);
    while (abs(delta) > 0)
        {
        //printf("delta is %d\n",delta);
        if (i >= SIZE) break;
        if (Window[(Spot+i-1)%SIZE2] == 0) break;
        //printf("changing index %d from %d ",
        //    Spot+i-1,Window[(Spot+i-1)%SIZE2]);
        Window[(Spot+i-1)%SIZE2] += delta;
        if (Window[(Spot+i-1)%SIZE2] == 0)
            Window[(Spot+i-1)%SIZE2] = 1;
        //printf("to %d\n",Window[(Spot+i-1)%SIZE2]);
        //getchar();
        ++i;
        delta = (int)(delta * Factor);
        }
    }

static void
adjustCurrent(int amt)
    {
    int i,delta;

    //adjust current value down

    //printf("adjusting current index %d\n",Spot);

    Window[Spot%SIZE2] = 0;
    
    /* adjust the previous values */
    i = 1;
    delta = (int)(amt * Factor);
    while (abs(delta) > 0)
        {
        //printf("delta is %d\n",delta);
        if (i >= SIZE) break;
        if (Spot-i < 0) break;
        if (Window[(Spot-i+SIZE2)%SIZE2] == 0) break;
        //printf("changing index %d (%d) from %d ",
        //    Spot-i,(Spot-i+SIZE2)%SIZE2,
        //    Window[(Spot-i+SIZE2)%SIZE2]);
        Window[(Spot-i+SIZE2)%SIZE2] -= delta;
        if (Window[(Spot-i+SIZE2)%SIZE2] == 0)
            Window[(Spot-i+SIZE2)%SIZE2] = -1;
        //printf("to %d\n",Window[(Spot-i+SIZE2)%SIZE2]);
        ++i;
        delta = (int)(delta * Factor);
        }

    /* adjust the future values */
    i = 1;
    delta = (int)(amt * Factor);
    while (abs(delta) > 0)
        {
        //printf("delta is %d\n",delta);
        if (i >= SIZE) break;
        if (Window[(Spot+i)%SIZE2] == 0) break;
        //printf("changing index %d (%d) from %d ",
        //    Spot+i,(Spot+i)%SIZE2,
        //    Window[(Spot+i)%SIZE2]);
        Window[(Spot+i)%SIZE2] -= delta;
        if (Window[(Spot+i)%SIZE2] == 0)
            Window[(Spot+i)%SIZE2] = -1;
        //printf("to %d\n",Window[(Spot+i)%SIZE2]);
        ++i;
        delta = (int)(delta * Factor);
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
                Amplify = atof(arg);
                argUsed = 1;
                break;
            case 'f':
                Factor = atof(arg);
                argUsed = 1;
                break;
            case 'v':
                printf("%s version %s\n", PROGRAM_NAME, PROGRAM_VERSION);
                exit(0);
                break;
            case 'h':
                printf("zero:\n");
                printf("    forces zero crossings to be exactly at zero -\n");
                printf("    nearby values are adjusted to smooth out the\n");
                printf("    dislocation\n");
                printf("\n");
                printf("zero options:\n");
                printf("  -a N     set amplification factor to N\n");
                printf("           amplification happens after compression\n");
                printf("           default value is %f\n",Amplify);
                printf("\n");
                printf("  -f N.N   set diminish factor to N.N\n");
                printf("           lower factor means more aggressive\n");
                printf("           movement of nearby values\n");
                printf("\n");
                printf("  -v       display this program's version number\n");
                printf("\n");
                printf("  -h       help\n");
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

