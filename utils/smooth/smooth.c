/* generic smooth for resampling
 *
 * smooth -s2.0 in.rra out.rra   # raise an octave
 * smooth -s1.059 in.rra out.rra # raise one semitone
 * smooth -s0.5 in.rra out.rra   # lower one octave
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <assert.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/wave.h"
#include "/usr/local/include/songlib/rra.h"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "smooth";
char *PROGRAM_VERSION = "0.01";

static void processData(FILE *in,FILE *,RRA *);
static int processOptions(int, char **);

int
main(int argc,char **argv)
    {
    int argIndex = 1;

    FILE *in,*out;
    RRA *r;

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
        printf("usage: smooth [<input rra file> [<output wave file>]]\n");
        exit(-1);
        }

    r = newRRAHeader();

    readRRAHeader(in,r,0);
    
    if (r->channels != 1)
        Fatal("smooth only works on mono audio files\n");

    writeRRAHeader(out,r,"modifiedBy: smooth",0);

    processData(in,out,r);

    fclose(in);
    fclose(out);

    return 0;
    }

static void
processData(FILE *in,FILE *out,RRA *r)
    {
    int prevprev,prev,amp;

    prevprev = prev = amp =
        readRRAAmplitude(in,out,r->sampleRate,outputComment);

    while (!feof(in))
        {
        fprintf(out,"%d\n",(prevprev + prev + amp) / 3);
        prevprev = prev;
        prev = amp;
        amp = readRRAAmplitude(in,out,r->sampleRate,outputComment);
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
            default:
                Fatal("option %s not understood\n",argv[argIndex]);
            }

        if (separateArg && argUsed)
            ++argIndex;

        ++argIndex;
        }

    return argIndex;
}

