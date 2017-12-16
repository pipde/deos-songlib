#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "/usr/local/include/songlib/util.h"
#include "tabchecker.h"

/******** Globals ********/

char *PROGRAM_NAME = "Songlib Tab Processor";
char *PROGRAM_VERSION = "0.9";

char *outputProgramName = 0;
char *outputVersion = "1.0";
char *programCmdLine = 0;
bool useStdIn = false, useStdOut = false, useStdOutForMakefile = false, skipMakefile = false;

/* Files listed in the following array will be included as #include <tab/[attribute].h> 
   instead of using the local #include "[attribute].h" syntax */
#define STD_ATTRIBUTE_COUNT 3
char stdAttributes[STD_ATTRIBUTE_COUNT][20] =
	{
		"voice",
		"tempo",
		"amplitude"
	};
	
#define MAX_LENGTH_ATTRIBUTE_INCLUDES 1024

/******** Prototypes ********/

static char *getBaseFilename(char *);
static void printVersion();
static void printHelp();
static int processOptions(int,char **);
static void outputMakefile(Composition *, FILE *, char *);
static void outputSonglibFile(Composition *, FILE *, char *, char *, char *);
static char *getNoteLengthString(double);
static void writeEscapedJsonString(FILE *, char *);

/******** Functions ********/

int 
main(int argc, char *argv[])
	{
    ASTTab *tab;
    Composition *composition;
    FILE *in;
    FILE *out = 0, *makefileOut = 0;
    char fileNameBuffer[256];
    char *baseName;
    int argIndex = 1;

	programCmdLine = argv[0];
    argIndex = processOptions(argc,argv);
    
    if (argc-argIndex == 0 && useStdIn)
        {
        baseName = "song";
        in = stdin;
        }
    else if (argc-argIndex == 1 && !useStdIn)
        {
  		baseName = getBaseFilename(argv[argIndex]);
        in = OpenFile(argv[argIndex],"r");
        }
    else
        {
        printHelp();
        exit(-1);
        }
    
    /* outputProgramName can be overriden via command line argument */
    if (outputProgramName == 0) outputProgramName = baseName;

    if (useStdOutForMakefile)
    	{
    	/* Print makefile to stdout, and exit */
    	
    	tab = parseTab(in);
		if (useStdIn) fclose(in);
    	composition = newCompositionFromTab(tab);
		outputMakefile(composition, stdout, baseName);
    	}
    else if (useStdOut)
        {
        /* Print songlib program to stdout, and exit */
        
        out = stdout;
    	tab = parseTab(in);
		if (useStdIn) fclose(in);
    	composition = newCompositionFromTab(tab);
    	outputSonglibFile(composition, out, baseName, outputProgramName, outputVersion);
        }
    else
        {
        /* Print songlib output and makefile to files, and display feedback via stdout */
        
		printf("Processing tab '%s':\n", baseName);
		
		printf("Parsing tab into syntax tree...\n");
    	tab = parseTab(in);
		if (useStdIn) fclose(in);
		
		/* Uncomment the following lines to get a printout of the tab syntax tree */
		//printf("Printing syntax tree...\n");
		//printASTTab(tab, stdout);
		
    	printf("Transforming syntax tree into composition...\n");
    	composition = newCompositionFromTab(tab);
        sprintf(fileNameBuffer, "%s.c", baseName);  
        
	    printf("Outputting %s...\n", fileNameBuffer);
        out = fopen(fileNameBuffer, "w");
    	outputSonglibFile(composition, out, baseName, outputProgramName, outputVersion);
	    fclose(out);
	    
	    if (!skipMakefile)
	    	{
			printf("Outputting makefile...\n");
		    makefileOut = fopen("makefile", "w");
			outputMakefile(composition, makefileOut, baseName);
			fclose(makefileOut);
			}
			
		printf("Done with tab '%s'.\n\n", baseName);
		}
    
    return 0;
	}
	
	
static char *
getBaseFilename(char *path)
    {
    char *fileName = StringDup(path);
    char *slash = strrchr(fileName, '/');
    char *backSlash = strrchr(fileName, '\\');
    char *baseName = slash > backSlash ? slash : backSlash;
    char *ext;
    
    // if baseName points to a slash or backslash, increment the pointer
    if (baseName > 0) baseName++;
    else baseName = fileName;
    
    ext = strrchr(baseName, '.');
    
    if (ext > 0) ext[0] = '\0';
    
    baseName = StringDup(baseName);
    free(fileName);
    return baseName;
    }

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
            case 'h':
            	printHelp();
                exit(0);
                break;
            case 'i':
                useStdIn = true;
                break;
            case 'm':
            	if (useStdOut)
            		{
			        printf("option -m cannot be used with -o\n");
			        printHelp();
			        exit(-1);
			        break;
            		}
            	if (skipMakefile)
            		{
			        printf("option -s cannot be used with -m\n");
			        printHelp();
			        exit(-1);
			        break;
            		}
                useStdOutForMakefile = true;
                break;
            case 'n':
                outputProgramName = StringDup(arg);
                argUsed = 1;
                break;
            case 'o':
            	if (useStdOutForMakefile)
            		{
			        printf("option -m cannot be used with -o\n");
			        printHelp();
			        exit(-1);
			        break;
            		}
                useStdOut = true;
                break;
            case 's':
            	if (useStdOutForMakefile)
            		{
			        printf("option -s cannot be used with -m\n");
			        printHelp();
			        exit(-1);
			        break;
            		}
                skipMakefile = true;
                break;
            case 'v':
		        printVersion();
                exit(0);
                break;
            case 'V':
                outputVersion = StringDup(arg);
                argUsed = 1;
                break;
            default:
	            printf("option %s not understood\n", argv[argIndex]);
	            printHelp();
	            exit(-1);
            }

        if (separateArg && argUsed)
            ++argIndex;

        ++argIndex;
        }

    return argIndex;
}

static void
printVersion()
	{
    printf("%s, version %s.\n", PROGRAM_NAME, PROGRAM_VERSION);
    
	}
	
static void
printHelp()
	{
	printf("Usage: %s [OPTION]... [FILE]\n", programCmdLine);
	printVersion();
    printf("\n");
    printf("FILE argument is mandatory, or must be omitted with option -i.\n");
    printf("  -h           print this help information\n");
    printf("  -i           input from stdin (output default name is 'song.c')\n");
    printf("  -m           output makefile to stdout (cannot be used with -o or -s)\n");
    printf("  -n NAME      set program name for the output\n");
    printf("  -o           output songlib program to stdout\n");
    printf("  -s           skip makefile output (cannot be used with -m)\n");
    printf("  -v           output version information and exit\n");
    printf("  -V VERSION   set program version for the output\n");
	}

/******* output and formatting ********/
static void 
outputMakefile(Composition *c, FILE *out, char *baseName)
	{
	int i, j;
    bool usedStdAttribute;
	char cfiles[MAX_LENGTH_ATTRIBUTE_INCLUDES] = "";
	char includes[MAX_LENGTH_ATTRIBUTE_INCLUDES] = "";	
    
    /* determine attribute processors to add to makefile */
    for (i = 0; i < c->attributeNameCount; i++)
        {
        usedStdAttribute = false;
        for (j = 0; j < STD_ATTRIBUTE_COUNT; j++)
        	{
        	if (strcmp(c->attributeNames[i], stdAttributes[j]) == 0)
        		{
        		usedStdAttribute = true;
        		break;
        		}
	        }
        if (!usedStdAttribute)
        	{
        	sprintf(cfiles, "%s.c ", c->attributeNames[i]);
        	sprintf(includes, "%s.h ", c->attributeNames[i]);
        	}
        }
        
    fprintf(out, "# This file was generated by %s %s.\n", PROGRAM_NAME, PROGRAM_VERSION);
    fprintf(out, "\n");
    fprintf(out, "BASENAME = %s\n", baseName);
    fprintf(out, "CC = gcc\n");
    fprintf(out, "DEBUG = -DMIN_DEBUG_LEVEL=3 -DMAX_DEBUG_LEVEL=6 -g #-p\n");
    fprintf(out, "CFLAGS = -Wall -pg $(DEBUG)\n");
    fprintf(out, "\n");
    fprintf(out, "BIN = $(BASENAME)\n");
    fprintf(out, "TAB = $(BASENAME).tab\n");
    fprintf(out, "MAIN = $(BASENAME).c\n");
    fprintf(out, "CFILES = $(MAIN) %s\n", cfiles);
    fprintf(out, "INCLUDES = %s\n", includes);
    fprintf(out, "RRAS = $(BASENAME).rra\n");
    fprintf(out, "\n");
    fprintf(out, ".PHONY : mix play clean\n");
    fprintf(out, "\n");
    fprintf(out, "play : mix\n");
    fprintf(out, "\trplay $(RRAS)\n");
    fprintf(out, "\n");
    fprintf(out, "mix : $(RRAS)\n");
    fprintf(out, "\n");
    fprintf(out, "%%.rra : $(BIN)\n");
    fprintf(out, "\t./$<\n");
    fprintf(out, "\n");
    fprintf(out, "$(BIN) : $(CFILES) $(INCLUDES)\n");
    fprintf(out, "\t$(CC) $(CFILES) -o $(BIN) -L/usr/local/lib -lsong -ltab -lm $(CFLAGS)\n");
    fprintf(out, "\n");
    fprintf(out, "clean :\n");
    fprintf(out, "\trm -f $(RRAS) $(BIN) gmon.out\n");
    fprintf(out, "\n");
    fprintf(out, "really-clean : clean\n");
    fprintf(out, "\trm -f $(MAIN) makefile\n");
    fprintf(out, "\n");
	}

static void 
outputSonglibFile(Composition *c, FILE *out, char *baseName, char *programName, char *version)
    {  
    int i, j, k, noteCount = 0;
    Part *part;
    Section *section;
    Measure *measure;
    NoteInstance *noteInstance;
    ASTAttribute *attribute;
    char *noteLengthString, *voiceName;
    bool usedStdAttribute;
    
    if (c == 0 || c->partCount == 0)
        {
        Fatal("expected composition with at least one part\n");
        return;
        }
    
    /* includes and defines */
    
    indent(out); fprintf(out, "/*\n");
    indent(out); fprintf(out, "      This file was generated by %s %s.\n", PROGRAM_NAME, PROGRAM_VERSION);
    indent(out); fprintf(out, " */\n");
    indent(out); fprintf(out, "\n");
    indent(out); fprintf(out, "#include <stdio.h>\n");
    indent(out); fprintf(out, "#include <songlib/util.h>\n");
    indent(out); fprintf(out, "#include <songlib/songlib.h>\n");
    indent(out); fprintf(out, "#include <tab/tabproc.h>\n");
    indent(out); fprintf(out, "\n");
    indent(out); fprintf(out, "/* attribute includes */\n");
    
    for (i = 0; i < c->attributeNameCount; i++)
        {
        usedStdAttribute = false;
        for (j = 0; j < STD_ATTRIBUTE_COUNT; j++)
        	{
        	if (strcmp(c->attributeNames[i], stdAttributes[j]) == 0)
        		{
	        	indent(out); fprintf(out, "#include <tab/%s.h>\n", c->attributeNames[i]);
        		usedStdAttribute = true;
        		break;
        		}
	        }
        if (!usedStdAttribute)
        	{
        	indent(out); fprintf(out, "#include \"%s.h\"\n", c->attributeNames[i]);
        	}
        }

    indent(out); fprintf(out, "\n");     
    indent(out); fprintf(out, "char *PROGRAM_NAME = \"%s\";\n", programName);
    indent(out); fprintf(out, "char *PROGRAM_VERSION = \"%s\";\n", version);    
    indent(out); fprintf(out, "\n");
    indent(out); fprintf(out, "/**** Part definitions ****/\n");
    indent(out); fprintf(out, "\n");
    indent(out); fprintf(out, "#define PART_COUNT %i\n", c->partCount);
    indent(out); fprintf(out, "\n");
    for (i = 0; i < c->partCount; i++)
        {
        part = c->parts[i];
        indent(out); fprintf(out, "#define PART_%s %d\n", part->voiceName, i);
        }
    indent(out); fprintf(out, "\n");
    indent(out); fprintf(out, "Part **parts = 0;\n");    
    indent(out); fprintf(out, "\n");    
    indent(out); fprintf(out, "void\n");    
    indent(out); fprintf(out, "initParts()\n");
    indentLevel++;
    indent(out); fprintf(out, "{\n");
    indent(out); fprintf(out, "int i;\n");
    indent(out); fprintf(out, "parts = (Part **)New(sizeof(Part *) * %i);\n", c->partCount);
   
    /* loop through all parts */
    for(i = 0; i < c->partCount; i++)
        {
        part = c->parts[i];
        voiceName = part->voiceName;
        noteCount = 0;
        for(j = 0; j < part->measureCount;j++)
            {
            measure = part->measures[j];
            noteCount += measure->noteInstanceCount;
            }
        indent(out); fprintf(out, "\n");
        indent(out); fprintf(out, "parts[PART_%s] = part(\"%s\", %i, %i);\n", voiceName, voiceName, noteCount, c->sectionCount);
        indent(out); fprintf(out, "i = 0;\n");
        
        for(j = 0; j < part->measureCount;j++)
            {
            measure = part->measures[j];
            
            /* If measure corresponds to section, write section code */
            for (k = 0; k < c->sectionCount; k++)
            	{
            	if (c->sections[k]->measureNumber == j)
		        	{
					indent(out); fprintf(out, "/* Section %i (part %s) */\n", k, voiceName);
					indent(out); fprintf(out, "parts[PART_%s]->sections[%i] = section(i);\n", voiceName, k);
		        	}
            	}
            	
            /* Write measure code */
            indent(out); fprintf(out, "/* Measure %i (part %s) */\n", j, voiceName);
            
            for(k = 0; k < measure->noteInstanceCount;k++)
                {                
                noteInstance = measure->noteInstances[k];
                noteLengthString = getNoteLengthString(noteInstance->duration);
                
                if (noteInstance->noteName == 0) // REST
                    {
                    indent(out); fprintf(out, "parts[PART_%s]->notes[i++] = note(%s, REST);\n", 
                                    voiceName, noteLengthString);
                    }
                else // NOTE
                    {
                    indent(out); fprintf(out, "parts[PART_%s]->notes[i++] = note(%s, '%c');\n", 
                                    voiceName, noteLengthString, noteInstance->noteName);
                    }
                }
            }
        }
        
    indent(out); fprintf(out, "}\n");
    indentLevel--;
    indent(out); fprintf(out, "\n");
    
    /* main function definition */
    
    indent(out); fprintf(out, "/**** Main ****/\n");
    
    indent(out); fprintf(out, "int\n");
    indent(out); fprintf(out, "main()\n");
    indentLevel++;    
    indent(out); fprintf(out, "{\n");
    indent(out); fprintf(out, "initParts();\n");
    indent(out); fprintf(out, "int location, noteNumber, partIndex;\n");
    indent(out); fprintf(out, "Part *part;\n");
        
    for (i = 0; i < c->attributeNameCount; i++)
        {
        for (j = 0; j < c->sectionCount;j++)
            {
            section = c->sections[j];            
            for (k = 0; k < section->attributeCount; k++)
                {
                attribute = section->attributes[k];
                if (strcmp(c->attributeNames[i], attribute->name) != 0) continue;
                
                    
                indent(out); fprintf(out, "setupProcessor_%s(\n", attribute->name);
                indentLevel++;
                indent(out); fprintf(out, "%i, // attribute index\n", attribute->index);
                indent(out); fprintf(out, "%i, // section number\n", j);
                indent(out); fprintf(out, "json_parse(\n");
                indentLevel++;
                writeEscapedJsonString(out, attribute->value);
                indent(out); fprintf(out, ")\n");
                indentLevel--;
                indent(out); fprintf(out, ");\n");
                indentLevel--;
                }
            }
        }
        
    indent(out); fprintf(out, "songInit();\n");
    indent(out); fprintf(out, "\n");

    indent(out); fprintf(out, "setStride(0.05);\n");
    indent(out); fprintf(out, "setSustain(0.99995);\n");
    indent(out); fprintf(out, "setPrimaryEmphasis(1);\n");
    indent(out); fprintf(out, "setSecondaryEmphasis(1);\n");
    indent(out); fprintf(out, "setTempo(80);\n");
    indent(out); fprintf(out, "setTime(4,4);\n");
    indent(out); fprintf(out, "setUseRandomSampling(true);\n");
    indent(out); fprintf(out, "\n");
    indent(out); fprintf(out, "openOutput(\"%s.rra\",0,0);\n", baseName);
    indent(out); fprintf(out, "\n");
    indent(out); fprintf(out, "/* set beginning location for each part */\n");
    indent(out); fprintf(out, "\n"); 
    indent(out); fprintf(out, "location = getLocation();\n");
    indent(out); fprintf(out, "\n"); 
    indent(out); fprintf(out, "/* process each part */\n");
    indent(out); fprintf(out, "\n");         

    indent(out); fprintf(out, "for (partIndex = 0; partIndex < PART_COUNT; partIndex++)\n");
    indentLevel++;
    indent(out); fprintf(out, "{\n");
    indent(out); fprintf(out, "setLocation(location);\n");
    indent(out); fprintf(out, "part = parts[partIndex];\n");
    indent(out); fprintf(out, "\n");
    indent(out); fprintf(out, "/* run preprocessors */\n");
        
    for (j = 0; j < c->sectionCount;j++)
        {
        section = c->sections[j];            
        for (k = 0; k < section->attributeCount; k++)
            {
            attribute = section->attributes[k];      
            indent(out); fprintf(out, "preProcess_%s(%i, part);\n", attribute->name, attribute->index);
            }
        }
        
    indent(out); fprintf(out, "\n"); 
    indent(out); fprintf(out, "for (noteNumber = 0; noteNumber < part->noteCount; noteNumber++)\n");
    indentLevel++;
    indent(out); fprintf(out, "{\n"); 
    indent(out); fprintf(out, "/* pre-render notes */\n");
    
    for (j = 0; j < c->sectionCount;j++)
        {
        section = c->sections[j];            
        for (k = 0; k < section->attributeCount; k++)
            {
            attribute = section->attributes[k];      
            indent(out); fprintf(out, "preRenderNote_%s(%i, part, noteNumber);\n", 
                        attribute->name, attribute->index);
            }
        }
    indent(out); fprintf(out, "\n");
    indent(out); fprintf(out, "/* render notes */\n");
    
    for (j = 0; j < c->sectionCount;j++)
        {
        section = c->sections[j];            
        for (k = 0; k < section->attributeCount; k++)
            {
            attribute = section->attributes[k];      
            indent(out); fprintf(out, "renderNote_%s(%i, part, noteNumber);\n", 
                        attribute->name, attribute->index);
            }
        }
    indent(out); fprintf(out, "}\n");
    indentLevel--;
    indent(out); fprintf(out, "\n"); 
    indent(out); fprintf(out, "/* run postprocessors */\n");
        
    for (j = 0; j < c->sectionCount;j++)
        {
        section = c->sections[j];            
        for (k = 0; k < section->attributeCount; k++)
            {
            attribute = section->attributes[k];      
            indent(out); fprintf(out, "postProcess_%s(%i, part);\n", attribute->name, attribute->index);
            }
        }
        
    indent(out); fprintf(out, "}\n");
    indentLevel--;
    
    indent(out); fprintf(out, "\n");
    indent(out); fprintf(out, "closeOutput();\n");
    indent(out); fprintf(out, "\n");
    indent(out); fprintf(out, "return 0;\n");
    indent(out); fprintf(out, "}\n");
    indentLevel--;
    }

static void
writeEscapedJsonString(FILE *out, char *jsonString)
    {
    int i = 0;
    char ch;
    
    indent(out); fprintf(out, "\""); 
    while ((ch = jsonString[i++]) != '\0')
        {
        switch (ch) 
            {
            case '"': 
                fprintf(out, "\\\""); 
                break;
            case '\\': 
                fprintf(out, "\\\\"); 
                break;
/*
            case '/': 
                fprintf(out, "\\/");             
                break;
*/
            case '\b': 
                fprintf(out, "\\b"); 
                break;
            case '\f': 
                fprintf(out, "\\f"); 
                break;
            case '\t': 
                fprintf(out, "\\t"); 
                break;
            case '\r': 
                fprintf(out, "\\r"); 
                break;
            case '\n': 
                /* break line in output code one line break in input */
                fprintf(out, "\\n\"\n");
                indent(out); fprintf(out, "\""); 
                break;
            default: 
                fprintf(out, "%c", ch); 
                break;
            }
        }
    fprintf(out, "\"\n"); 
    }
    
static char *
getNoteLengthString(double duration)
    {
   	char *retVal;
   	
   	if (duration == 1.0/32) return "THIRTYSECOND_NOTE";
    if (duration == 1.0/16) return "SIXTEENTH_NOTE";
    if (duration == 1.0/8) return "EIGHTH_NOTE";
    if (duration == 1.0/4) return "QUARTER_NOTE";
    if (duration == 1.0/2) return "HALF_NOTE";
    if (duration == 1.0) return "WHOLE_NOTE";
    
   	if (duration == 1.0/32*1.5) return "DOTTED_THIRTYSECOND_NOTE";
    if (duration == 1.0/16*1.5) return "DOTTED_SIXTEENTH_NOTE";
    if (duration == 1.0/8*1.5) return "DOTTED_EIGHTH_NOTE";
    if (duration == 1.0/4*1.5) return "DOTTED_QUARTER_NOTE";
    if (duration == 1.0/2*1.5) return "DOTTED_HALF_NOTE";
    if (duration == 1.0*1.5) return "DOTTED_WHOLE_NOTE";
    
   	if (duration == 1.0/32/1.5) return "TRIPLET_THIRTYSECOND_NOTE";
    if (duration == 1.0/16/1.5) return "TRIPLET_SIXTEENTH_NOTE";
    if (duration == 1.0/8/1.5) return "TRIPLET_EIGHTH_NOTE";
    if (duration == 1.0/4/1.5) return "TRIPLET_QUARTER_NOTE";
    if (duration == 1.0/2/1.5) return "TRIPLET_HALF_NOTE";
    if (duration == 1.0/1.5) return "TRIPLET_WHOLE_NOTE";

    // TODO: this could be expanded to support returning "1.0/64" or "T/2" for sixty-fourths, etc.
    
    retVal = (char *) New(sizeof(char) * 80);
    snprintf(retVal, 79, "%f", duration);
    return retVal;
    }

