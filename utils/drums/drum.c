/*
 *  This is a RRA drum filter
 *  
 *  modified by Madhav Rao
 *
 *  WARNING: barely tested
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/songlib.h"
#include "/usr/local/include/songlib/wave.h"
#include "/usr/local/include/songlib/rra.h"
#include "drum.h"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "rradrum";
char *PROGRAM_VERSION = "0.01";

static int drumset;

static int processOptions(int, char **);

static int interplay,interplayInstance=0;
static double drumNote;
static int pitch = KICK;
static double scalingFactor = 1.0;

int checkScalingFactor(char ch)
    {
    if(ch == 's')
        {
        return 0;
        }

    return 1;

    }

int determineScalingFactor(FILE *in)    
    {
    int count=0, ch;
    char s[512]="";
    int first=1;
    int decimal=0;
    int floater=0;
    int exponent=0;

    ch = fgetc(in);

    while(isdigit(ch)||(first && ch
    == '-') || (!decimal && ch ==
    '.') || (!exponent && (ch == 'E' || ch == 'e')))
        {
        //fprintf(stderr,"%c",ch);
        first =0;
        if(ch == '.')
            {
            decimal = 1;
            floater = 1;
            }
        if(ch=='E' || ch == 'e')            
            {
            decimal = 1;
            floater = 1;
            exponent = 1;
            first = 1;
            }
        s[count++]=ch;      
        if(count >= sizeof(s)-1)
            {
            fprintf(stderr,"Count =%d\n",count);
            fprintf(stderr,"RRA FORMAT error\n");
            }
        ch = fgetc(in);
        }
    s[count]='\0';
    //fprintf(stderr,"The scalingFactor is %s \n", s);

    if(floater)
        {
        //scalingFactor = (int)(atof(s));
        scalingFactor = atof(s);
        }
    else
        {
        scalingFactor = (float)atoi(s);
        }

    return 1;           

    }

int checkDrumNotes(char ch)
    {


    if (ch == 'T' || ch == 'S'
    || ch == 'I' || ch == 'Q' ||
    ch == 'H' || ch == 'W')
        {
        switch(ch)
            {
            case 'T' :
                fprintf(stderr,"Should play Thirty second note \n");
                drumNote = 0.125;
                break;
            case 'S' :
                fprintf(stderr,"Should play Sixteenth note \n");
                drumNote = 0.25;
                break;
            case 'I' :
                fprintf(stderr,"Should play Eighth note \n");
                drumNote = 0.5;
                break;
            case 'Q' :
                fprintf(stderr,"Should play Quater note \n");
                drumNote = 1; 
                break;
            case 'H' :
                fprintf(stderr,"Should  play Half note \n");
                drumNote = 2; 
                break;
            case 'W' :
                fprintf(stderr,"Should play Whole note \n");
                drumNote = 4; 
                break;

            /* write something for else
            cases */
            default :
                fprintf(stderr,"Should play Quater note by default \n");
                drumNote = 1; 
                break;
            }
        }
    return 1;
    }

int checkDrumType(char ch)
    {
    if (ch == 'b' || ch == 'B'
    || ch == 's' || ch == 'S' ||
    ch == 'T' || ch == 't' || ch == 'o' || ch == 'O')
        {
        switch(ch)
            {
            case 'b' :
                fprintf(stderr,"Playing Base Drum \n");
                pitch = KICK;
                break;
            case 's' :
                fprintf(stderr,"Playing Snare Drum \n");
                pitch = SNARE;
                break;
            case 't' :
                fprintf(stderr,"Playing TomTom Drum \n");
                pitch = TOM_HIGH;
                break;
            case 'o' :
                fprintf(stderr,"Drum off \n");
                pitch = 0;  /* Will define a Macro  for pitch 0 */ 
                break;
                
            default:
                fprintf(stderr,"Playing all drum instruments \n");
                pitch = KICK;
                break;
                
            }
        }
    return 1;
    }


int checkPattern(FILE *in)
    {

    int count=0, ch;
    char s[512]="";
    int first=1;
    int decimal=0;
    int floater=0;
    int exponent=0;

    ch = fgetc(in);

    while(isdigit(ch)||(first && ch
    == '-') || (!decimal && ch ==
    '.') || (!exponent && (ch == 'E' || ch == 'e')))
        {
        first =0;
        if(ch == '.')
            {
            decimal = 1;
            floater = 1;
            }
        if(ch=='E' || ch == 'e')            
            {
            decimal = 1;
            floater = 1;
            exponent = 1;
            first = 1;
            }

        s[count++]=ch;      
        if(count >= sizeof(s)-1)
            {
            fprintf(stderr,"Count =%d\n",count);
            fprintf(stderr,"RRA FORMAT error\n");
            }
        ch = fgetc(in);
        }
    s[count]='\0';

    fprintf(stderr,"The drum pattern is %s \n", s);

    if(floater)
        {
        interplay = (int)(atof(s));
        }
    else
        {
        interplay = atoi(s);
        }

    return 0;
    
    }

void createInterplayData(RRADRUM
*h,int size,int channels)
    {
    int i;

    if(size == 0) return ;

    h->data = (int **) New(sizeof(int *) * channels);

    for(i=0;i<channels;i++)
        {
        h->data[i]=(int *)New(sizeof(int) * size);
        }
    }

int clearInterplayData(RRADRUM *h, int size,
int channels)    
    {
    int i,j;

    if(size == 0) return 0;

    for(i=0;i<channels;i++)
        for(j=0;j<size;j++)     
            h->data[i][j]=0;
    return 0;           
    }

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


static int checkDrumOff(int ch, FILE *in)    
    {
    int count=0;

    char s[10]="";
    
    s[count++] = ch;

    ch = fgetc(in);

    while(ch != '\n')
        {
        
        s[count++] = ch;

        ch = fgetc(in);

        }
    s[count] = '\0';

    return (strcmp(s, "off"));          

    }

/*
 * New drumHandler based on tokens is
 * developed
 *
 */
static void drumHandlerNew(FILE *in, FILE *out)
    {
    int ch,count = 0;
    char s[512]="";
    char delims[] = " ";
    char *result = NULL;

    ch = fgetc(in);

    if(ch == 'd')
        {
        interplayInstance=1;

        while(ch != '\n')
            {
            s[count++] = ch;

            ch = fgetc(in);
            }

        s[count] = '\0';    

        result = strtok( s, delims );

        while( result != NULL ) 
            {
        
            if(!strcmp(result,"pattern"))
                {
                result = strtok( NULL, delims );

                fprintf(stderr,"The pattern is %d \n", atoi(result));

                interplay = atoi(result);
                }
         
            if(!strcmp(result,"scalingFactor"))
                {
                result = strtok( NULL, " " );

                fprintf(stderr,"scalingFactor %f \n",atof(result));

                scalingFactor = atof(result);
                }

             if(!strcmp(result,"snare") || !strcmp(result,"base") || !strcmp(result,"tomtom") )   
                {
                fprintf(stderr,"Drum type is %s \n ", result);

                checkDrumType(result[0]);
                }
        
             if(!strcmp(result,"Quarter") || !strcmp(result,"Half") || !strcmp(result,"Eighth") )   
                {
                fprintf(stderr,"note Length is %s \n ", result);
                /*
                 * Need to develop the
                 * function to update
                 * noteLength....
                 *
                 */
                checkDrumNotes(result[0]);
                }

            if(!strcmp(result,"off"))
                {
                fprintf(stderr,"drum Off \n");
                pitch  = 0;
                }

            result = strtok( NULL, delims );

            }
        }
    else
        {
        fprintf(stderr,"This is not meant for rradrum filter\n");

        fprintf(out,"!");
        fprintf(out,"%c",ch);

        writeCommentOut(in,out);
        }
    }

static void drumHandler(FILE *in, FILE *out)
    {
    int ch ;
    ch = fgetc(in);
    if(ch == 'd') /*  drum pattern */
        {
        interplayInstance=1;
        //fprintf(stderr,"%c ",ch);
        while(!isspace(ch))
            {
            ch = fgetc(in);
            //fprintf(stderr,"%c ",ch);
            }
            
        ch = fgetc(in);
        //fprintf(stderr,"%c ",ch);

        while(!(isdigit(ch)))
            {

            checkDrumType(ch);

            if(pitch)
                {
                ch = fgetc(in);

                //fprintf(stderr,"%c ",ch);
                ch = fgetc(in);

                //fprintf(stderr,"%c ",ch);

                checkDrumNotes(ch);

                ch = fgetc(in);
            
                //checkScalingFactor(ch);

                //fprintf(stderr,"%c ",ch);
                ch = fgetc(in);

                }
            else
                {
                break;
                }

            }
        
        if(pitch)
            {
            ungetc(ch,in); 

            checkPattern(in);   

            ch = fgetc(in);

            if(!checkScalingFactor(ch))  
                {
                ch = fgetc(in);

                determineScalingFactor(in);

                fprintf(stderr,"The scaling factor is %f \n",scalingFactor);
                }
            }

        else
            {
            if(!checkDrumOff(ch, in)) /* Assuring that off is provided as comment */
                {
                pitch = 0;
                }
            }

        }
    else
        {
        fprintf(stderr,"This is not meant for rradrum filter\n");

        fprintf(out,"!");
        fprintf(out,"%c",ch);

        writeCommentOut(in,out);
        }
    }


int getDrumNotes(RRA **h,int drumset) 
    {
    
    //*h = getNumberedNote(drumset,KICK);
    *h = getNumberedNote(drumset,pitch);

    return 0;
    }       


int getMeasureDrumData(RRA *hDrum,int drumset,RRADRUM *interplayData)
    {
    int k,offset=0;
    
    int sampleRate = getSampleRate();
    int bitsPerSample = getBitsPerSample();
    int beats = getBeatsPerMeasure();
    double samples=beatsToSamples((double)beats);
    int samplesQ = (int)samples/4;

    if(!pitch)
        {
        memset(&interplayData->data[0][0],(int)0,(int)(samplesQ * 4));  
        return 0;

        }

    for(k=8;k>0;k=k>>1)
        {
        if(k & interplay)
            {
            fprintf(stderr,"drumNote = %f \n",drumNote);
            hDrum = newRRA(sampleRate,bitsPerSample,1,(int)(samplesQ * drumNote)); 
            getDrumNotes(&hDrum,drumset);
            memcpy(&interplayData->data[0][(int)(offset*samplesQ*drumNote)],&hDrum->data[0][0],(int)(samplesQ * drumNote));     

            if(drumNote == 0.5) /*  Eight'th note */ 
                {
                /*
                 * Add zero's to another set of 
                 * Eight'th noteLength 
                 *
                 */
                memset(&interplayData->data[0][(int)((offset+1)*samplesQ*drumNote)],(int)0,sizeof(int)*(int)(samplesQ * drumNote));     
                }
            else if(drumNote == 2.0) /* Half Note*/          
                {
                k = k >> 1;
                }
            }
        else
            {
            if(drumNote  != 2.0) /* Half Note */
                memset(&interplayData->data[0][offset*samplesQ],(int)0,sizeof(int)*samplesQ);   
            }
        offset++;
        }
    return 0;       
    }       

int
main(int argc,char **argv)
    {
    int argIndex = 1;
    int
    i,amp,lineNumber=0,x=0; 
    FILE *in, *out;
    RRA *h,*track,*hDrum;
    int
    beats,samplesQ;
    double samples;
    RRADRUM interplayData;

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
        printf("usage: rradrum [<input rra file> [<output rra file>]]\n");
        exit(-1);
        }
    
    songInit();
    drumset = readScale("/usr/local/share/samples/drums/","hera_");
    setTempo(250);
    setTime(4,4);
    setStride(0.05);
    setSustain(0.99995);
    setAmplitude(1.5);

    beats = getBeatsPerMeasure();
    samples=beatsToSamples((double)beats);
    samplesQ = (int)samples/4;

    h = newRRAHeader();

    track = newRRAHeader();

    readRRAHeader(in,h,0);

    track->channels = h->channels + 1;
    track->samples = h->samples;
    track->bitsPerSample=h->bitsPerSample;
    track->sampleRate=h->sampleRate;

    /* writing into output file */
    writeRRAHeader(out,track,"modifiedBy: rradrum",0);

    /* Only Quarter note samples */
    createInterplayData(&interplayData,samplesQ*4,1);

    for (i = 0; !feof(in) && i < h->samples * h->channels; ++i)
        {

        //amp = readRRAAmplitude(in,h->bitsPerSample,drumHandler);
        //amp = readRRAAmplitudeNew(in,out,h->bitsPerSample,drumHandler);
        amp = readRRAAmplitudeNew(in,out,h->bitsPerSample,drumHandlerNew);

        fprintf(out,"%d\n",amp);

        if(interplayInstance && 
        i-lineNumber >= samplesQ*4)
            {
            x=0;
            //fprintf(stderr,"The interplay data is changed \n");
            }

        if(interplay && pitch)
            {
            if(!lineNumber ||
            (i-lineNumber > samplesQ*4
            && interplayInstance))
                {
                x=0;
                fprintf(stderr,"New hDrum data \n ");

                getMeasureDrumData(hDrum,drumset,&interplayData);

                lineNumber=i;

                }

            interplayInstance=0;
                
            if((i>=lineNumber) && (i-lineNumber < samplesQ*4)&&(x<samplesQ*4)) 
                {
                fprintf(out,"%d\n", (int)(interplayData.data[0][x++] * scalingFactor));
                }
           else 
                {

                if(x == samplesQ*4)     
                    {
                    x=0;
                    }

                fprintf(out,"%d\n",(int)(interplayData.data[0][x++] * scalingFactor));
                //fprintf(out,"0\n");
                }
            }
        else
            {
            fprintf(out,"0\n");
            continue;
            }
        }
        
    fclose(in);

    fclose(out);

    clearInterplayData(&interplayData,4*samplesQ,1);

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
            default:
                Fatal("option %s not understood\n",argv[argIndex]);
            }

        if (separateArg && argUsed)
            ++argIndex;

        ++argIndex;
        }

    return argIndex;
    }

