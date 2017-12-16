#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/songlib.h"

char *PROGRAM_NAME = "scale";
char *PROGRAM_VERSION = "0.01";

static char *notes[] = {"c","c#","d","d#","e","f","f#","g","g#","a","a#","b"};

static int processOptions(int,char **);

void
note(int inst,int octave,int pitch)
    {
    char buffer[512];

    sprintf(buffer,"%s%d.rra",notes[pitch],octave);
    openOutput(buffer,0,0);
    play(H,inst,octave,pitch);
    closeOutput();
    }

int
main(int argc,char **argv)
    {
    int argIndex;
    int inst,octave;

    argIndex = processOptions(argc,argv);

    if (argc - argIndex != 2)
        Fatal("usage: scale <instrument_prefix> <octave>\n");

    octave = atoi(argv[2]);

    printf("octave %d...\n",octave);

    songInit();

    inst = readScale(argv[1],"");
    setAmplitude(0.2);

    note(inst,octave,C);
    note(inst,octave,Cs);
    note(inst,octave,D);
    note(inst,octave,Ds);
    note(inst,octave,E);
    note(inst,octave,F);
    note(inst,octave,Fs);
    note(inst,octave,G);
    note(inst,octave,Gs);
    note(inst,octave,A);
    note(inst,octave,As);
    note(inst,octave,B);
    note(inst,octave+1,C);

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
            default:
                fprintf(stderr,"option %s not understood\n",argv[argIndex]);
                exit(-2);
            }

        if (separateArg && argUsed)
            ++argIndex;

        ++argIndex;
        }

    return argIndex;
    }
