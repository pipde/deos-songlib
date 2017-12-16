#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/rra.h"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "tempo";
char *PROGRAM_VERSION = "0.01";

//static double base = 10;
static double tempo = 1.0;

static int processOptions(int,char **);


static void writeCommentOut(FILE *in, FILE *out)
    {
    
    int ch;

    while((ch = fgetc(in)) && (ch != EOF) && (ch != '\n'))
	{
	fprintf(out,"%c",ch);

	continue;
	}

    fprintf(out,"\n");	    
    }


/*
 *
 * Added comment Handler for rratempo
 * using tempo value
 *
 */

static void tempoHandler(FILE *in, FILE *out)
    {
    
    int ch, count=0;
    char s[512] = "";
    ch = fgetc(in);
    char *token = NULL;

    if(ch == 't') /* tempo comment */
	{

	while(ch != '\n')
	    {
	    s[count++] = ch;
	    ch = fgetc(in);

	    }
	
	s[count] = '\0';

	token = strtok(s, " ");

	fprintf(stderr,"%s \n", token);

	token = strtok(NULL," ");

	fprintf(stderr,"%s\n",token);

	if(!strcmp(token,"OFF") | !(strcmp(token,"off")))
	    {
	    tempo = 1;
	    }
	else
	    {
	    tempo = atof(token);
	    }
	}
    else
	{

	fprintf(stderr,"This is not meant for rratempo filter \n");

	fprintf(out,"!");

	fprintf(out,"%c",ch);

	writeCommentOut(in,out);
	}
    
    }

int
main(int argc,char **argv)
    {
    int argIndex = 1;

    int i, amp1=0, amp11=0, amp2=0, amp22=0;
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
	printf("usage: tempo -fN [<input rra file> [<output rra file>]]\n");
	exit(-1);
	}

    h = newRRAHeader();
    
    readRRAHeader(in,h,0);
    writeRRAHeader(out,h,"modifiedBy: tempo",0);

    //limit = (int) pow(2,base);
    int count = 0;

    for (i = 0; i < h->samples; ++i)
        {
        //for (j = 0; j < h->channels; ++j)
	    //{
	    //int amp = readRRAAmplitude(in,h->bitsPerSample,0);
	    amp1 = readRRAAmplitudeNew(in,out,h->bitsPerSample,tempoHandler);
        if(h->channels > 1) 
            amp2 = readRRAAmplitudeNew(in,out,h->bitsPerSample,tempoHandler);
	    if(tempo > 1)
            {
            if(count++ < tempo-1) 
                continue;
            else
                count = 0;
            fprintf(out,"%d\n",amp1);
            if(h->channels > 1) 
                fprintf(out,"%d\n",amp2);
            }
        else if(tempo < 1)
            {
            fprintf(out,"%d\n",(int)(amp1+amp11)/2);
            if(h->channels > 1) 
                fprintf(out,"%d\n",(int)(amp2+amp22)/2);
            fprintf(out,"%d\n",amp1);
            if(h->channels > 1) 
                fprintf(out,"%d\n",amp2);
            amp11 = amp1;
            amp22 = amp2;
            }

        
	    }

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
	     *	       PrintActions = 1;
	     *         break;
	     */
	    case 'b':
		tempo = atof(arg);
		argUsed = 1;
		break;
	    case 'v':
	        printf("%s version %s\n", PROGRAM_NAME, PROGRAM_VERSION);
		exit(0);
		break;
	    case 'h':
		printf("tempo options:\n");
		printf("  -b N     set log base to N\n");
		printf("           good values range from >1 upwards\n");
		printf("           lower values mean higher compression\n");
		printf("  -h       help\n");
		exit(0);
		break;
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

