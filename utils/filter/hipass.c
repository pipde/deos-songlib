#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/songlib.h"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "fir";
char *PROGRAM_VERSION = "0.01";

/* filter designer at http://cisco.cscs.wmin.ac.uk/filter_design.html */

static int processOptions(int,char **);

double attenuation = 1;
double coefficient = 0.5;

#define MAX  32768.0
#define NZEROS 4
#define NPOLES 4
//#define GAIN   1.018789902e+00 //100 Hz
#define GAIN   1.037934013e+00 //200 Hz
static float xv[NZEROS+1], yv[NPOLES+1];

static void
filterloop(FILE *in,FILE *out,RRA *h)
    {
    int i;
    int amp;
    for (i = 0; i < h->samples; ++i)
        {
        xv[0] = xv[1];xv[1] = xv[2]; xv[2] = xv[3]; xv[3] = xv[4]; 
        amp = readRRAAmplitude(in,out,h->bitsPerSample,outputComment);
        xv[4] = amp / MAX / GAIN;
        yv[0] = yv[1]; yv[1] = yv[2]; yv[2] = yv[3]; yv[3] = yv[4]; 
        //100 Hz
        //yv[4] =   (xv[0] + xv[4]) - 4 * (xv[1] + xv[3]) + 6 * xv[2]
        //      + ( -0.9634534511 * yv[0]) + (  3.8896837378 * yv[1])
        //      + ( -5.8889997443 * yv[2]) + (  3.9627694171 * yv[3]);
        //200 Hz
        yv[4] =   (xv[0] + xv[4]) - 4 * (xv[1] + xv[3]) + 6 * xv[2]
               + ( -0.9282404974 * yv[0]) + (  3.7820790085 * yv[1])
            + ( -5.7793788971 * yv[2]) + (  3.9255397507 * yv[3]);
                                                                                        fprintf(out,"%d\n",(int) (yv[4] * MAX));
        }
    }

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
        fprintf(stderr,"usage: hipass -h -a<N.M> -w<N.M> -d<N.M> -l<N.M> "
            "[<input rra file> [<output rra file>]]\n");
        exit(-1);
        }

    h = newRRAHeader();
    
    readRRAHeader(in,h,0);

    if (h->channels != 1)
        Fatal("hipass requires a mono RRA file\n");

    writeRRAHeader(out,h,"modifiedBy: hipass",0);

    filterloop(in,out,h);

    fclose(in);
    fclose(out);

    return 0;
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
            case 'a':
                attenuation = atof(arg);
                argUsed = 1;
                break;
            case 'c':
                coefficient = atof(arg);
                argUsed = 1;
                break;
            case 'h':
                printf("hipass options:\n");
                printf("  -a N     set alpha to N\n");
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

