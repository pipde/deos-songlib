#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/rra.h"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "display";
char *PROGRAM_VERSION = "0.01";

static int Size = 5;
static int NoDisplay = 0;
static int Width = 73;
static int maxValue = 0;
static int minValue = 0;
static int clips = 0;
static double max;
static double min;

static void display(int,int);
static void displayChar(char,int);

static int processOptions(int,char **);


int
main(int argc,char **argv)
    {
    int argIndex = 1;

    int i,j;
    int count;
    int total;
    int amp;
    int avg;
    int clipPositive,clipNegative;
    FILE *in,*out;
    RRA *h;

    argIndex = processOptions(argc,argv);

    out = stdout;

    if (argc-argIndex == 0)
        in = stdin;
    else if (argc-argIndex == 1)
        in = OpenFile(argv[argIndex],"r");
    else
        {
        printf("usage: display [-wN] [-sN] [<input rra file>]\n");
        exit(-1);
        }

    h = newRRAHeader();
    
    readRRAHeader(in,h,0);
    writeRRAHeader(out,h,"modifiedBy: display",0);

    max = pow(2,h->bitsPerSample - 1);
    min = - (max - 1);

    count = 0;
    total = 0;
    clipPositive = 0;
    clipNegative = 0;
    for (i = 0; !feof(in); ++i)
        for (j = 0; j < h->channels; ++j)
            {
            amp = readRRAAmplitude(in,out,h->bitsPerSample,outputComment);
            if (amp >= max) { clipPositive = 1; ++clips; }
            if (amp <= min) { clipNegative = 1; ++clips; }
            if (amp > maxValue) maxValue = amp;
            if (amp < minValue) minValue = amp;
            total += amp;
            ++count;
            if (count % Size == 0)
                {
                avg = total / Size;
                if (clipPositive)
                    display(i,max);
                else if (clipNegative)
                    display(i,min);
                else
                    display(i,avg);
                total = 0;
                count = 0;
                clipPositive = 0;
                clipNegative = 0;
                }
            }

    if (i != h->samples + 1)
        fprintf(stderr,"WARNING: %d samples found, %d expected\n",i,h->samples);

    printf("maximum value: %d\n",maxValue);
    printf("minimum value: %d\n",minValue);
    printf("clips: %d\n",clips);

    fclose(in);
    fclose(out);

    return 0;
    }

static void
display(int index,int amp)
    {
    int span,gap;
    if (NoDisplay) return;
    printf("%7d:",index);
    if (amp <= min)
        {
        displayChar('X',Width / 2 - 1);
        displayChar('|',1);
        }
    else if (amp < 0)
        {
        span = (int) ((Width / 2 - 1) * amp / min);
        gap = Width / 2 - 1 - span;
        displayChar(' ',gap);
        displayChar('o',span);
        displayChar('|',1);
        }
    else if (amp == 0)
        {
        displayChar(' ',Width / 2 - 1);
        displayChar('|',1);
        }
    else if (amp >= max)
        {
        displayChar(' ',Width / 2 - 1);
        displayChar('|',1);
        displayChar('X',Width / 2);
        }
    else
        {
        displayChar(' ',Width / 2 - 1);
        displayChar('|',1);
        displayChar('o',(int) (Width / 2 * amp / max));
        }
    displayChar('\n',1);
    }

static void
displayChar(char ch,int count)
    {
    while (count > 0)
        {
        printf("%c",ch);
        --count;
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
            case 'w':
                Width = atoi(arg);
                argUsed = 1;
                break;
            case 'n':
                NoDisplay = 1;
                break;
            case 's':
                Size = atoi(arg);
                argUsed = 1;
                break;
            case 'h':
                printf("display options:\n");
                printf("  -w N   set display width to N\n");
                printf("         default is %d characters\n",Width);
                printf("  -s N   set averaging size to N samples\n");
                printf("         default is %d values are averaged\n",Size);
                printf("  -h     help\n");
                exit(0);
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

