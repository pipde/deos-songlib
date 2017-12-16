#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/rra.h"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "amplify";
char *PROGRAM_VERSION = "0.01";

static double attenuation = 1.5;
static int smart = 0;

static int processOptions(int,char **);

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
    int ch, count = 0;
    char s[512] = "";

    char *token = NULL;

    ch = fgetc(in);

    if(ch == 'a')
        {

        while(ch != '\n')
            {
            s[count++] = ch;
            ch = fgetc(in);
            }

        s[count] = '\0';

        token = strtok(s," ");

        fprintf(stderr,"%s\n",token);

        token = strtok(NULL, " ");

        fprintf(stderr,"%s\n",token);

        attenuation = (float)atoi(token);

        }
    else
        {
        ungetc(ch,in);
        outputComment(in,out);
        }

    }

int
main(int argc,char **argv)
    {
    int argIndex = 1;

    int i,j;
    int count;
    int max,amp;
    char *source,ampname[1024];
    FILE *in,*out,*fp;
    RRA *h;

    if (strcmp(argv[0],"softer") == 0) attenuation = 0.75;
    if (strcmp(argv[0],"louder") == 0) attenuation = 1.25;

    argIndex = processOptions(argc,argv);

    if (argc-argIndex == 0)
        {
        in = stdin;
        source = "stdin";
        out = stdout;
        }
    else if (argc-argIndex == 1)
        {
        in = OpenFile(argv[argIndex],"r");
        source = argv[argIndex];
        out = stdout;
        }
    else if (argc-argIndex == 2)
        {
        in = OpenFile(argv[argIndex],"r");
        source = argv[argIndex];
        out = OpenFile(argv[argIndex+1],"w");
        }
    else
        {
        printf("usage: amplify -aN [<input rra file> [<output rra file>]]\n");
        exit(-1);
        }

    h = newRRAHeader();
    
    if (smart)
        {
        sprintf(ampname,".amplitude_%s",source);

        if (strlen(source) + strlen(",amplitude_") + 1 >= sizeof(ampname))
            Fatal("amplify: file name is too long\n");

        fp = fopen(ampname,"r");
        if (fp != 0)
            {
            fscanf(fp," %d",&max);
            attenuation = (pow(2,h->bitsPerSample - 1) - 1) / max;
            fprintf(stderr,"amplify: old maximum is %d\n",max);
            fprintf(stderr,"amplify: smart attenuation is %f\n",attenuation);
            fclose(fp);
            }
        }

    readRRAHeader(in,h,0);
    writeRRAHeader(out,h,"modifiedBy: amp",0);

    max = 0;
    count = 0;
    for (i = 0; !feof(in); ++i)
        for (j = 0; j < h->channels; ++j)
            {
            if (amp > max) max = amp;
            if (-amp > max) max = -amp;
            amp = readRRAAmplitude(in,out,h->bitsPerSample,commentHandler);
            fprintf(out,"%d\n",(int) (amp * attenuation));
            ++count;
            }

    if (i != h->samples + 1)
        fprintf(stderr,"WARNING: %d samples found, %d expected\n",i,h->samples);

    fclose(in);
    fclose(out);

    #if 0
    fp = OpenFile(ampname,"w");
    fprintf(fp,"%d\n",max);
    fclose(fp);
    #endif

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
            case 'a':
                attenuation = atof(arg);
                argUsed = 1;
                break;
            case 's':
                smart = 0;
                break;
            case 'S':
                smart = 1;
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

