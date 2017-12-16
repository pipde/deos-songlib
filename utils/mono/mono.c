#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/rra.h"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "mono";
char *PROGRAM_VERSION = "0.01";

static int Channel = -1;
static int trackChannel = -1;
static double AmpFactor = 1;

static int processOptions(int, char **);

void writeCommentOut(FILE *in, FILE *out)
    {
    
    int ch;

    while((ch = fgetc(in)) && (ch != EOF) && ch != '\n')
        {
        
        fprintf(out,"%c",ch);

        continue;
        }
    
    fprintf(out,"\n");
    }

static void commentHandler(FILE *in, FILE *out)
    {
    int ch, count=0;
    char s[512] = "";
    char *token = NULL;

    ch = fgetc(in);

    if(ch == 'm')
        {
        fprintf(stderr,"mono comment found \n");
        
        while(ch != '\n')
            {
            s[count++] = ch;
            ch = fgetc(in);
            }

        s[count] = '\0';

        token = strtok(s," ");

        token  =strtok(NULL , " ");

        Channel = atoi(token);

        fprintf(stderr,"Channel selected is %d\n",Channel);

        if (trackChannel < Channel)
            {
            Fatal("mono: bad channel: %d\n",Channel);
            Channel = -1;
            }
        }
    else
        {
        ungetc(ch,out);
        outputComment(in,out);
        }
    }

int
main(int argc,char **argv)
    {
    int argIndex = 1;

    int i,j,channels;
    char *inFile;
    FILE *in,*out;
    RRA *track;

    argIndex = processOptions(argc,argv);

    if (argc-argIndex == 0)
        {
        in = stdin;
        out = stdout;
        inFile = "stdin";
        }
    else if (argc-argIndex == 1)
        {
        in = OpenFile(argv[argIndex],"r");
        out = stdout;
        inFile = argv[argIndex];
        }
    else if (argc-argIndex == 2)
        {
        in = OpenFile(argv[argIndex],"r");
        out = OpenFile(argv[argIndex+1],"w");
        inFile = argv[argIndex];
        }
    else
        {
        printf("usage: rramono [-aNN.N] [-dNN.N] [-l] [-r] "
            "[<input wave> [<output wave>]]\n");
        exit(-1);
        }

    track = newRRAHeader();
    readRRAHeader(in,track,0);

    if (track->channels == 1)
        {
        fprintf(stderr, "%s is already a mono track\n", inFile);
        Channel = -1;
        }

    if (Channel >= track->channels)
        Fatal("mono: bad channel: %d\n",Channel);

    channels = track->channels;
    trackChannel = track->channels;
    track->channels = 1;
    setAttributeRRA(track,"channels","1");

    writeRRAHeader(out,track,"modifiedBy: mono",0);

    for (i = 0; !feof(in); ++i)
        {
        int total = 0;
        for (j = 0; j < channels; ++j)
            {

            int amp =
                readRRAAmplitude(in,out,track->bitsPerSample,commentHandler);

            if (j == Channel || Channel == -1)
                total += amp;
            }
        fprintf(out,"%d\n",(int)(total * AmpFactor));
        }

    if (i != track->samples)
        fprintf(stderr,"WARNING: %d samples found, %d expected\n",
            i,track->samples);

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
            case 'a':
                AmpFactor = atof(arg);
                argUsed = 1;
                break;
            case 't':
            case 'c':
                Channel = atoi(arg);
                argUsed = 1;
                break;
            case 'h':
                printf("mono options:\n");
                printf("  -a N   set amplitude factor to N\n");
                printf("         values > 1 increase loudness\n");
                printf("         values < 1 decrease loudness\n");
                printf("         default is %f\n",AmpFactor);
                printf("  -c N   set channel to select to N\n");
                printf("         setting channel to -1 merges all channels\n");
                printf("         default is %d\n",Channel);
                printf("  -t N   same as -c\n");
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

