#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/rra.h"
#include "/usr/local/include/songlib/wave.h"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "reverse";
char *PROGRAM_VERSION = "0.01";

static int processOptions(int,char **);

int
main(int argc,char **argv)
    {
    int argIndex = 1;

    int amp,size;
    int fd;
    FILE *in,*out;
    off_t offset,start;
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
        printf("usage: reverse [<input rra file> [<output rra file>]]\n");
        exit(-1);
        }

    fd = fileno(in);
    h = newRRAHeader();
    readRRAHeader(in,h,0);

    writeRRAHeader(out,h,"modifiedBy: rrareverse",0);

    //find start
        
    start = skipRRAHeaderFD(fd);

    offset = lseek(fd,-2,SEEK_END);
    while (offset > start)
        {
        findPreviousRRAAmplitudeFD(fd);
        amp = readRRAAmplitudeFD(fd,h->bitsPerSample,&size);
        fprintf(out,"%d\n", amp);
        offset = lseek(fd,-(size + 2),SEEK_CUR);
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
	    case 'h':
            printf("reverse options: none\n");
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

