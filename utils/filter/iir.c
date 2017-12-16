#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <assert.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/songlib.h"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "iir";
char *PROGRAM_VERSION = "0.01";

/* 
 * filter designers at:
 *     http://cisco.cscs.wmin.ac.uk/filter_design.html
 *     http://www-users.cs.york.ac.uk/~fisher/mkfilter/trad.html
 *
 */

static void readCoefficients(char *);
static double iirFilter(int,int);
static double getInput(int,int);
static double getOutput(int,int);
static int processOptions(int,char **);

#define MAX_TAPS 1024
double feedforward[MAX_TAPS];
double feedback[MAX_TAPS];
double totalWeight;
double gain = 1;
int feedforwardTaps;
int feedbackTaps;
char *coefficientFile = "coefficients";
double attenuation = 1;
double **input;
double **output;

int
main(int argc,char **argv)
    {
    int argIndex = 1;

    int i,j;
    double max;
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
        fprintf(stderr,"usage: flanger -h -a<N.M> -w<N.M> -d<N.M> -l<N.M> "
            "[<input rra file> [<output rra file>]]\n");
        exit(-1);
        }

    readCoefficients(coefficientFile);
    fprintf(stderr,"feedforward taps: %d\n",feedforwardTaps);
    fprintf(stderr,"feedback taps: %d\n",feedbackTaps);
    fprintf(stderr,"filter attenuation is %f\n",attenuation);

    h = newRRAHeader();
    
    readRRAHeader(in,h,0);

    writeRRAHeader(out,h,"modifiedBy: iir",0);

    max = pow(2,h->bitsPerSample - 1) - 1;

    input = (double **) New(sizeof(double *) * h->channels);
    output = (double **) New(sizeof(double *) * h->channels);

    for (i = 0; i < h->channels; ++i)
        {
        input[i] = (double *) New(sizeof(double) * h->samples);
        output[i] = (double *) New(sizeof(double) * h->samples);
        }

    for ( i = 0; i < h->samples; ++i)
        for (j = 0; j < h->channels; ++j)
            {
            int amp = readRRAAmplitude(in,out,h->bitsPerSample,outputComment);
            input[j][i] = (amp / max) / gain;
            output[j][i] = iirFilter(j,i);
            //fprintf(out,"original: %d\n", amp);
            //fprintf(out,"original: %f\n", input[j][i]);
            fprintf(out,"%f\n", attenuation * output[j][i]);
            }

    fclose(in);
    fclose(out);

    return 0;
    }

static double
iirFilter(int channel,int index)
    {
    int i;
    double total;

    //printf("initial: %f\n",getInput(channel,index));
    total = 0; 
    
    for (i = 0; i < feedforwardTaps; i++) 
        {
        total += feedforward[feedforwardTaps-i-1] * getInput(channel,index-i);
        //printf("    feedforward total now is %f\n",total);
        }
    
    for (i = 0; i < feedbackTaps; i++) 
        {
        total += feedback[feedbackTaps-i-1] * getOutput(channel,index-i-1);
        //printf("    feedback total now is %f\n",total);
        }

    //getchar();
    return total;
    }

static double
getInput(int channel,int index)
    {
    if (index < 0)
        return input[channel][0];
    else
        return input[channel][index];
    }

static double
getOutput(int channel,int index)
    {
    if (index < 0)
        return output[channel][0];
    else
        return output[channel][index];
    }

static void
readCoefficients(char *fileName)
    {
    FILE *fp;
    int ch;
    double coeff;

    fprintf(stderr,"reading coefficients: %s\n",fileName);

    fp = OpenFile(fileName,"r");

    totalWeight = 0;
    feedforwardTaps = 0;

    //first number in file is gain

    fscanf(fp," %lf",&gain);

    //read feedforward coefficients

    fscanf(fp," %lf",&coeff);
    while (!feof(fp))
        {
        if (feedforwardTaps >= MAX_TAPS)
            Fatal("iir: too many coefficients!\n");
        feedforward[feedforwardTaps] = coeff;
        totalWeight += coeff;
        ++feedforwardTaps;
        while ((ch = fgetc(fp)) && isspace(ch))
            continue;
        if (ch == '%') break;
        ungetc(ch,fp);
        fscanf(fp," %lf",&coeff);
        }

    fprintf(stderr,"total feedforward coefficient weight: %f\n",totalWeight);

    totalWeight = 0;
    feedbackTaps = 0;

    //read feedback coefficients

    fscanf(fp," %lf",&coeff);
    while (!feof(fp))
        {
        if (feedbackTaps >= MAX_TAPS)
            Fatal("iir: too many coefficients!\n");
        feedback[feedbackTaps] = coeff;
        totalWeight += coeff;
        ++feedbackTaps;
        fscanf(fp," %lf",&coeff);
        }

    fprintf(stderr,"total feedback coefficient weight: %f\n",totalWeight);
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
            case 'f':
                coefficientFile = arg;
                argUsed = 1;
                break;
            case 'h':
                printf("iir options:\n");
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

