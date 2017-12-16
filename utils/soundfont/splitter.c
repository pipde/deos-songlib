#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/rra.h"

#define MIDI 0
#define OCTAVE 1
#define NOTE 2

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "splitter";
char *PROGRAM_VERSION = "0.01";

static int style = OCTAVE;

static int processOptions(int,char **);

char *noteNames[12] = { "c","c#","d","d#","e","f","f#","g","g#","a","a#","b" };
int   noteOffsets[7] = {  0,  2,  4,  5,  7,  9, 11  };

char buffer[1024];
char *prefix = "note_";

int notes = 12;

int
main(int argc,char **argv)
    {
    int argIndex = 1;

    int i;
    int notesWritten,samples;
    int start;
    double duration;
    FILE *in,*out;
    RRA *h;

    argIndex = processOptions(argc,argv);

    start = 0;
    duration = 1;

    if (argc-argIndex == 0)
	{
	in = stdin;
	}
    else if (argc-argIndex == 1)
        {
	in = OpenFile(argv[argIndex],"r");
	}
    else if (argc-argIndex == 2)
        {
	in = OpenFile(argv[argIndex],"r");
	start = atoi(argv[argIndex+1]);
	}
    else if (argc-argIndex == 3)
        {
	in = OpenFile(argv[argIndex],"r");
	start = atoi(argv[argIndex+1]);
	duration = atof(argv[argIndex+2]);
	}
    else
	{
	printf("usage: splitter [-h] [<rrafile> [<startnote> [<duration>]]]\n");
	exit(-1);
	}

    h = newRRAHeader();
    
    readRRAHeader(in,h,0);

    if (h->channels > 1)
        Fatal("splitter requires a mono rra file\n");

    samples = duration * h->sampleRate;

    notesWritten = 0;

    while (notesWritten * samples < h->samples)
        {
	int midinote = start * 12 + noteOffsets[notesWritten % 7] +
	    (notesWritten / 7) * 12;
	char *notename = noteNames[midinote % notes];
	int octavename = midinote / notes;

	if (style == MIDI)
	    sprintf(buffer,"%s%d.rra",prefix,midinote);
	else if (style == NOTE)
	    sprintf(buffer,"%s%s%d.rra",prefix,notename,octavename);
	else
	    sprintf(buffer,"%s%d%s.rra",prefix,octavename,notename);

	printf("creating %s...\n",buffer);

	out = OpenFile(buffer,"w");

	fprintf(out,"RRAUDIO\n");
	fprintf(out,"sampleRate: %d\n",h->sampleRate);
	fprintf(out,"bitsPerSample: %d\n",h->bitsPerSample);
	fprintf(out,"channels: %d\n",h->channels);
	fprintf(out,"samples: %d\n",samples);
	fprintf(out,"%%%%\n");

	for (i = 0; i < samples; ++i)
	    fprintf(out,"%d\n", readRRAAmplitude(in,out,h->bitsPerSample,outputComment));

	fclose(out);

	++notesWritten;
	}

    fclose(in);

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
	    case 'm':
		style = MIDI;
		break;
	    case 'o':
		style = OCTAVE;
		break;
	    case 'n':
		style = NOTE;
		break;
	    case 'p':
		prefix = arg;
		if (strlen(prefix) >= sizeof(buffer) - 10)
		    Fatal("splitter: prefix is too big\n");
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

