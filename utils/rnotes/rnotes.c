#include <stdio.h>
#include <stdlib.h>

#define MINOR 0
#define MAJOR 1

static char *PROGRAM_NAME = "rnotes";
static char *PROGRAM_VERSION = "0.01";
static int mode = MAJOR;

static char *major[] = {"C","D","E","F","G","A","B"};
static char *minor[] = {"C","Cs","D","Ds","E","F","Fs","G","Gs","A","As","B"};

static int
randomInt(int low,int high)
    {
    return low + (high - low) * (rand() / (RAND_MAX * 1.0));
    }

static char *
randomNote()
    {
    if (mode == MAJOR)
        return major[randomInt(0,(sizeof(major)/sizeof(char *)))];
    else
        return minor[randomInt(0,(sizeof(minor)/sizeof(char *)))];
    }

static void
printHeader()
    {
    printf("#include <stdio.h>\n");
    printf("#include <stdlib.h>\n");
    printf("#include <stdarg.h>\n");
    printf("#include \"util.h\"\n");
    printf("#include \"songlib.h\"\n");
    printf("\n");
    printf("#define dir \"/usr/local/share/samples/mandolin/platinum/\"\n");
    printf("#define base \"soft_sus_vib_\"\n");
    printf("\n");
    printf("/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */\n");
    printf("\n");
    printf("char *PROGRAM_NAME = \"rnote\";\n");
    printf("char *PROGRAM_VERSION = \"0.01\";\n");
    printf("\n");
    printf("int instrument;\n");
    }

static void
printVerse()
    {
    printf("static void\n");
    printf("verse()\n");
    printf("    {\n");
    printf("    play(%d,instrument,3,%s);\n",randomInt(1,5),randomNote());
    printf("    play(%d,instrument,3,%s);\n",randomInt(1,5),randomNote());
    printf("    play(%d,instrument,3,%s);\n",randomInt(1,5),randomNote());
    printf("    play(%d,instrument,3,%s);\n",randomInt(1,5),randomNote());
    printf("    play(%d,instrument,3,%s);\n",randomInt(1,5),randomNote());
    printf("    play(%d,instrument,3,%s);\n",randomInt(1,5),randomNote());
    printf("    play(%d,instrument,3,%s);\n",randomInt(1,5),randomNote());
    printf("    play(%d,instrument,3,%s);\n",randomInt(1,5),randomNote());
    printf("    }\n");
    }

static void
printMain()
    {
    printf("int\n");
    printf("main()\n");
    printf("    {\n");
    printf("    songInit();\n");
    printf("\n");
    printf("    instrument = readScale(dir,base);\n");
    printf("\n");
    printf("    setTempo(184);\n");
    printf("    openOutput(\"a.rra\",44100,16);\n");
    printf("\n");
    printf("    verse();\n");
    printf("\n");
    printf("    closeOutput();\n");
    printf("\n");
    printf("    return 0;\n");
    printf("    }\n");
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
            case 'm':
                if (atoi(arg) == 0)
                    mode = MINOR;
                else
                    mode = MAJOR;
                argUsed = 1;
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

int
main(int argc,char **argv)
    {
    int argIndex = processOptions(argc,argv);
    fprintf(stderr,"argc is %d, argIndex is %d\n",argc,argIndex);
    fprintf(stderr,"mode is %d\n",mode);
    if (argIndex == argc - 1) srand(atoi(argv[argIndex]));
    printHeader();
    printVerse();
    printMain();
    return 0;
    }
