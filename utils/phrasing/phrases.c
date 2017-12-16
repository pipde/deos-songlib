#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/wave.h"
#include "scanner.h"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "extract";
char *PROGRAM_VERSION = "0.01";

#define BANK_CAPACITY 5
char *banks[BANK_CAPACITY];
int banksize = 0;
#define PHRASE_CAPACITY 512
int takes[PHRASE_CAPACITY];
char *comments[PHRASE_CAPACITY];
int TimingCount;
double Timings[1024];
int Silent = 0;

#define Mode "rra"

static void readTimings(char *);
static void prelude(FILE *out,char *);
static void middle(FILE *out);
static void coda(FILE *out);
static int processOptions(int, char **);

int
main(int argc,char **argv)
    {
    int i;
    int argIndex = 1;
    int phrase;
    FILE *out,*fp;
    char buffer[512];

    argIndex = processOptions(argc,argv);

    if (argc-argIndex < 1)
        Fatal("usage: %s BASENAME_1 BASENAME_2 ...\n",argv[0]);

    out = OpenFile("vocals-temp.c","w");

    banksize = argc - argIndex;
    if (banksize > BANK_CAPACITY)
        Fatal("a maximum of %d takes are allowed\n",BANK_CAPACITY);

    for (i = 0; i < banksize; ++i)
         banks[i] = argv[i+argIndex];

    prelude(out,banks[0]);

    phrase = 0;
    printf("banksize is %d\n",banksize);
    sprintf(buffer,"%s%d.%s",banks[0],phrase,Mode);
    fp = fopen(buffer,"r");
    if (fp == 0)
        {
        printf("file %s%d.%s not found -- phrases must begin at note 0\n",
            banks[0],phrase,Mode);
        exit(-1);
        }
    while (phrase < PHRASE_CAPACITY)
        {
        fp = fopen(buffer,"r");
        if (fp == 0 && phrase == 0)
            printf("file %s%d.%s not found -- phrases must begin at note 0\n",
                banks[0],phrase,Mode);
        else if (fp == 0)
            printf("phrase %d not found...\n",phrase);
        else
            {
            //printf("phrase %d exists...\n",phrase);
            for (i = 0; i < banksize; ++i)
                {
                printf("playing take %d (%s%d.wav)\n",i+1,banks[i],phrase);
                if (!Silent)
                    {
                    sprintf(buffer,"rplay %s%d.%s",banks[i],phrase,Mode);
                    system(buffer);
                    }
                }
            }
        int good = 0;
        char *command;
        printf("> ");
        command = readToken(stdin);
        if (*command == 'r')
            {
            printf("repeating...\n");
            good = -1;
            phrase -= 1;
            }
        else if (*command == 'b')
            {
            if (phrase == 0)
                printf("already at the first phrase...\n");
            else
                {
                printf("backing up...\n");
                good = -1;
                phrase -= 2;
                }
            }
        else if (*command == 's' || *command == 'f')
            {
            printf("skipping this phrase...\n");
            good = -1;
            }
        else if (isdigit(*command))
            {
            int take = atoi(command);
            if (take > 0 && take <= banksize)
                {
                printf("selecting take %d\n",take);
                /* clear the newline */
                readLine(stdin);
                printf("enter a comment: ");
                comments[phrase] = readLine(stdin);
                takes[phrase] = take;
                good = 1;
                }
            }
        else if (*command == 'q')
            break;

        if (good == 0)
            {
            printf("command %s not understood\n",command);
            printf("commands are:\n");
            printf("   q    quit\n");
            printf("   r    repeat the last phrase\n");
            printf("   b    go back to the previous phrase\n");
            printf("   s    skip this phrase\n");
            printf("   1    choose the first take\n");
            printf("   2    choose the second take\n");
            printf("   ...\n");
            }
        ++phrase;
        sprintf(buffer,"%s%d.%s",banks[0],phrase,Mode);
        fp = fopen(buffer,"r");
        }

    readTimings(banks[0]);
    middle(out);
    coda(out);

    fclose(out);

    return 0;
    }

static void
readTimings(char *take)
    {
    double seconds;
    FILE *fp;
    char buffer[512];

    snprintf(buffer,sizeof(buffer),"%stimings",take);
    printf("looking for timing information in %s\n",buffer);

    TimingCount = 0;
    if ((fp = fopen(buffer,"r")) == 0) return;
    fscanf(fp,"%lf",&seconds);
    while (!feof(fp))
        {
        Timings[TimingCount] = seconds;
        fscanf(fp,"%lf",&seconds);
        ++TimingCount;
        }
    }

static void
prelude(FILE *out,char *take)
    {
    int i;
    fprintf(out,"#include <stdio.h>\n");
    fprintf(out,"#include <stdlib.h>\n");
    fprintf(out,"#include <stdarg.h>\n");
    fprintf(out,"#include \"/usr/local/include/songlib/util.h\"\n");
    fprintf(out,"#include \"/usr/local/include/songlib/songlib.h\"\n");

    fprintf(out,"\n");
    fprintf(out,"/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */\n");
    fprintf(out,"char *PROGRAM_NAME = \"vocals\";\n");
    fprintf(out,"char *PROGRAM_VERSION = \"0.01\";\n");
    fprintf(out,"\n");
    fprintf(out,"double TimingOffset = 0.0;\n");
    fprintf(out,"\n");
    fprintf(out,"int\n");
    fprintf(out,"main()\n");
    fprintf(out,"    {\n");
    for (i = 0; i < banksize; ++i)
        fprintf(out,"    int take%d = readScale(\"raw-vocals/\",\"%s\");\n",
            i+1,banks[i]);
    fprintf(out,"\n");
    fprintf(out,"    songInit();\n");
    fprintf(out,"\n");
    fprintf(out,"    setAmplitude(1.0);\n");
    fprintf(out,"\n");
    fprintf(out,"    openOutput(\"vocals.rra\",0,0);\n");
    fprintf(out,"\n");
    }

static void
middle(FILE *out)
    {
    int i;

    for (i = 0; i < PHRASE_CAPACITY; ++i)
        {
        if (comments[i] == 0) continue;
            
        if (strlen(comments[i]) > 1)
            fprintf(out,"    //%s\n",comments[i]);
        if (i < TimingCount)
            fprintf(out,"    moveTo(0,%f+TimingOffset);\n",Timings[i]);
        else
            fprintf(out,"    moveTo(0,SSS);\n");

        fprintf(out,"    nplay(0,take%d,%d);\n",takes[i],i);
        fprintf(out,"\n");
        }
    }

static void
coda(FILE *out)
    {
    fprintf(out,"    closeOutput();\n");
    fprintf(out,"\n");
    fprintf(out,"    return 0;\n");
    fprintf(out,"    }\n");
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
                printf("%s\n",arg);
                break;
            case 's':
                Silent = 1;
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

