/*
 * This is a RRA CROSSOVER channel
 * filter
 * 
 * modified by Madhav Rao
 *
 * WARNING: barely tested
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/rra.h"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "crossover";
char *PROGRAM_VERSION = "0.01";

static int inrun = 10;
static int outrun = 50;

static void process(RRA *);
static int processOptions(int,char **);

static FILE *in,*out;
/* by default cross over samples 
is set to 1000 */
static int crossoverSamples = 88100; 
static int crossoverFlag = 0;
static int channel1 = 0, channel2 = 1;

int
main(int argc,char **argv)
    {
    int argIndex = 1;

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
        printf("usage: fader -n<N> [<input rra file> [<output rra file>]]\n");
        exit(-1);
        }

    h = newRRAHeader();
    
    readRRAHeader(in,h,0);

    process(h);

    fclose(in);
    fclose(out);

    return 0;
    }

static void
process(RRA *h)
    {
    int i, j; 
    int crossoverComplete = 1; 
    int buffer[h->channels], start, temp;
    double factor;
    double delta;

    /*
     * Need to pass the channel nos to
     * crossover as commandline
     * arguments
     *
     */

    writeRRAHeader(out,h,"modifiedBy: crossover",0);

    start = 0;
    factor = 1;
    temp = 0;
    for (i = 0; !feof(in); ++i)
        {
        for (j = 0; j < h->channels; ++j)
            buffer[j] = readRRAAmplitude(in,out,h->bitsPerSample,outputComment);
        
        if(crossoverFlag > temp && crossoverComplete) 
            {
            crossoverComplete = 0;
            temp = crossoverFlag;
            }

        if(crossoverFlag && !crossoverComplete)
            {
            start++; /* counting
            crossoverSamples */

            //factor = 0.99*factor;
            //factor = ((crossoverSamples - 1) / crossoverSamples) *factor;
            delta = 1.0 / crossoverSamples; 
            factor = factor - delta;
            if(factor < 0)  factor = 0;

            if(start < crossoverSamples)
                {
                if (crossoverFlag % 2) /* Odd */
                    {
                    for(j = 0; j < h->channels; ++j)
                        {
                        if(j == channel1)
                            fprintf(out,"%d\n",(int)(buffer[channel1]*factor + buffer[channel2]*(1-factor)));
                        else if(j == channel2)      
                            fprintf(out,"%d\n",(int)(buffer[channel1]*(1-factor) + buffer[channel2]*factor));
                        else                
                            fprintf(out,"%d\n",(int)(buffer[j]));
                        }
                    }
                else /* Even */
                    {
                    for(j = 0; j < h->channels; ++j)
                        {
                        if(j == channel1)
                            fprintf(out,"%d\n",(int)(buffer[channel2]*factor + buffer[channel1]*(1-factor)));
                        else if(j == channel2)      
                            fprintf(out,"%d\n",(int)(buffer[channel2]*(1-factor) + buffer[channel1]*factor));
                        else                
                            fprintf(out,"%d\n",(int)(buffer[j]));
                        }
                    }
                }
            }
        
        if(start > crossoverSamples) 
            {
            crossoverComplete = 1;
            factor = 1;
            start = 0;
            }
        
        if(crossoverComplete )    
            {
            if(crossoverFlag % 2)
                {
                /* completed try swapping */

                /*
                 * swap the read amplitudes
                 * outputs to different channels.
                 *
                 */
                for(j = 0; j < h->channels; ++j)
                    {
                    if(j == channel1)
                        fprintf(out,"%d\n",(int)(buffer[channel2]));
                    else if(j == channel2)          
                        fprintf(out,"%d\n",(int)(buffer[channel1]));
                    else                    
                        fprintf(out,"%d\n",(int)(buffer[j]));
                    }
                //for (j = h->channels-1 ; j >= 0 ; --j)
                //    fprintf(out,"%d\n",(int)buffer[j]);

                }
            else
                {
                for (j = 0; j < h->channels; ++j)
                    fprintf(out,"%d\n",(int)buffer[j]);

                }
            }               
        }
            
    if (i != h->samples + 1)
        fprintf(stderr,"WARNING: %d samples found, %d expected\n",i,h->samples);
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
            case 'i':
                inrun = atoi(arg);
                fprintf(stderr,"inrun is %d\n",inrun);
                argUsed = 1;
                break;
            case 'o':
                outrun = atoi(arg);
                fprintf(stderr,"outrun is %d\n",outrun);
                argUsed = 1;
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

