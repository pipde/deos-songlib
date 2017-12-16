#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <stdarg.h>
#include <string.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/songlib.h"
#include "/usr/local/include/songlib/rra.h"
#include "/usr/local/include/songlib/wave.h"

//#define WIDTH  441
//#define WIDTH  44100
#define FADE_IN_FACTOR 0.97 
#define FADE_OUT_FACTOR 0.99 

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */
char *PROGRAM_NAME = "rraextract";
char *PROGRAM_VERSION = "0.02";

static void extract(FILE *,FILE *, char **);
static void batch(int,char **);
static int processOptions(int, char **);
static int rraFilesExtract(int **, int , int , int); 

int channelExtract = 0;
int stereoFlag = 0;
double thresholdMeasTime = 0.5;
double skipTime = 0.5;
double windowTime = 0.1;

int
main(int argc,char **argv)
    {
    int argIndex = 1;

    argIndex = processOptions(argc,argv);

    #if 0
    if (argc-argIndex == 0)
        {
	extract(stdin,stdout, argv);
	}
    #endif	    
    if (argc-argIndex == 1)
        {
        extract(OpenFile(argv[argIndex],"r"),stdout,argv);
        }
    else if (argc-argIndex >= 2 && strstr(argv[argIndex+1],".wav") != 0)
        {
        batch(argc-argIndex,argv+argIndex);
        }
    else if (argc-argIndex == 2)
        {
        extract(OpenFile(argv[argIndex],"r"),OpenFile(argv[argIndex+1],"w"),argv);
        }
    else
        {
        printf("usage: rraextract -c[Channel No] -s[stereo output] -i[ignore/skip time in seconds] -t[threshold measure time in sec] -w[window time in sec] [<input wav file>]\n");
        exit(-1);
        }

    return 0;
    }

static void
extract(FILE *in,FILE *out, char **argv)
    {
    int i,amp, j;
    WAVE *w;
    RRA *h;
    int **buffer;

    w = newWave();
    readWaveHeader(w,in);

    h = newRRAHeader();

    h->sampleRate = w->SampleRate;
    h->samples = w->sampleCount;
    h->bitsPerSample = w->BitsPerSample;
    h->channels = w->NumChannels;
    h->data = w->channels;
    h->maxAmp = w->maxAmp;

    if (h->sampleRate == 0) h->sampleRate = 44100;
    
    buffer = (int **) New(sizeof(int *) * h->channels);

    for(i = 0; i < h->channels; i++)
	buffer[i] = (int*)New(sizeof(int) * h->samples);

    for (i = 0; !feof(in) && (h->samples == 0 || i < h->samples); ++i)
        {
	if(h->channels > 1)   
	    {
	    j = 0;
	    for(;j < h->channels; j++)
            {
            amp = readByteInt(in,h->bitsPerSample/8);
            if (amp >= w->maxAmp) amp = -2 * h->maxAmp + amp;
            buffer[j][i] = amp;    
            }
	    }
	else
	    buffer[0][i] = amp;
	}

    rraFilesExtract(buffer,h->samples,h->channels,h->sampleRate);
    
    free(buffer);
    fclose(in);
    fclose(out);
    }

/*
 * Calculates the maximum of moving window
 * of WIDTH samples.
 *
 */
    
static void 
maxAlgo(double **maxWindowBuf, int ** buffer, int channels, int size)    
    {
    int i,j,l; 
    double max;
    int width;

    width = secondsToSamples(windowTime);

    for(i=0; i<channels; i++)
        {
        for(j=0; j<size; j++)
            {
            max = 0;
            //for(l=j; l<j+WIDTH; l++) 
            for(l=j; l<j+width; l++) 
                {
                if(l>size-1) break;

                if (max<fabs(buffer[i][l])) 
                    {
                    max = fabs(buffer[i][l]);  
                    maxWindowBuf[i][j] = max;
                    }
                }
            }
        }
    }

static void 
thresholdMeas(int **buffer, int channels, int size, double *thres)
    {
    double initialSamples, skipSamples, max = 0;
    int j,i;

    initialSamples = secondsToSamples(thresholdMeasTime);
    skipSamples = secondsToSamples(skipTime);
    fprintf(stderr,"thresholdMeasTime: %3.2f \n",thresholdMeasTime);
    fprintf(stderr,"thresholdMeasSamples: %3.2f \n",initialSamples);
    fprintf(stderr,"skipTime: %3.2f \n",skipTime);
    fprintf(stderr,"skipSamples: %3.2f \n",skipSamples);
    fprintf(stderr,"sampleRate: %d \n",getSampleRate());

    for (j=0; j<channels; j++)
        {
        for (i=skipSamples; i< initialSamples+skipSamples; i++)
            if(max < abs(buffer[j][i])) 
		{
		max = abs(buffer[j][i]);
		//fprintf(stderr,"max %3.2f found at: %d \n",max,i);
		}


        thres[j] = max;    
        max = 0;
        }
    }

static int
determineFadeSamples(int val, double factor)
    {
    double diminishedAmp = factor;
    double sustainVal = 1;
    int l = 0;

    //fprintf(stderr,"factor = %f \n",factor);

    for(; fabs(sustainVal) > 0.001; ++l)
        {
        sustainVal =  val* diminishedAmp;
        diminishedAmp *= factor;
        if((int)sustainVal == 0) 
            {
            l++;
            break;
            }
        }
    return l;
    }


static void
fillFadeValues(int startVal, double *buf, int size, double factor)
    {
    double diminishedAmp = factor;
    double sustainVal = 1;

    for(; size>0; --size)
        {
        sustainVal = startVal * diminishedAmp;
        buf[size-1] = sustainVal;
        diminishedAmp *= factor;
        }
    }

static int 
rraFilesExtract(int **buffer,int size, int channels, int sampleRate)    
    {
    int i = 0,j,k = 0,fileNo = 0, l = 0, m = 0, n = 0,p =0;
    int flag = 0, flagInit = 0, fadeNoSamples;
    double **maxBuf;
    char fileName[10] = "";
    FILE *out;
    RRA *h;
    double thresholdChannels[channels], skipSamples, initialSamples;
    double sustainVal = 1, diminishedAmp;
    double *initialFadeIn, *initialFadeOut;

    h = newRRAHeader();
    h->sampleRate = sampleRate;
    h->channels = channels;

    maxBuf = (double **)New(sizeof(double *)* channels);
    for(n = 0; n < channels; n++)
	maxBuf[n] = (double*)New(sizeof(double) * size);
    
    /* 
     * Determining maximum of moving window samples.
     *
     */

    maxAlgo(maxBuf,buffer,channels,size);
    
    /*
     * Determining threshold using intial samples of silence
     *
     */

    initialSamples = secondsToSamples(thresholdMeasTime);
    skipSamples = secondsToSamples(skipTime);
    thresholdMeas(buffer,channels,size, thresholdChannels);
    fprintf(stderr,"Absolute threshold: %f \n ",thresholdChannels[1]);

    k = skipSamples + initialSamples;

    i = 0;
    for (j = (skipSamples + initialSamples) * (i + 1) ; j < size; j++) 
	{
	if(fabs(maxBuf[channelExtract][j]) >= 2*thresholdChannels[channelExtract] && (!flagInit))
	    {
	    flagInit = 1;
	    k = j;
	    fprintf(stderr,"Opening file at %3.2f second \n",samplesToSeconds(k));
	    fadeNoSamples = determineFadeSamples(abs(buffer[channelExtract][j]),FADE_IN_FACTOR);
	    initialFadeIn = (double *)New(sizeof(double)*fadeNoSamples);
	    fillFadeValues(abs(buffer[channelExtract][j]),initialFadeIn,fadeNoSamples,FADE_IN_FACTOR);
	    }
	if((fabs(maxBuf[channelExtract][j]) > 3 * thresholdChannels[channelExtract]) && (!flag)) flag = 1;
	if(fabs(maxBuf[channelExtract][j]) < 2 * thresholdChannels[channelExtract] && flag && flagInit)
	    {

	    /*
	     * RRA file is generated...
	     *
	     */

	    /*
	     *  Fade out is slow, hence
	     *  diminishedAmp of 0.99 is
	     *  considered.
	     *
	     */

	    diminishedAmp = FADE_OUT_FACTOR;
	    sustainVal = 1;
	    l = determineFadeSamples(abs(buffer[channelExtract][j-1]), FADE_OUT_FACTOR);
	    initialFadeOut = (double *)New(sizeof(double)*l);
	    fillFadeValues(abs(buffer[channelExtract][j-1]),initialFadeOut,l,FADE_OUT_FACTOR);
	    sprintf(fileName,"%d",fileNo);
	    strncat(fileName,".rra",4);
	    fprintf(stderr,"opening %s ... \n",fileName);
	    out = fopen(fileName,"w");
	    h->samples = j - k  + l + fadeNoSamples;
	    h->channels = 1;
	    if(stereoFlag) h->channels = 2;
	    fileNo++;
	    writeRRAHeader(out,h,"modifiedBy: rraextract",0);

	    /*
	     * Putting Fade in samples
	     *
	     */

	    for(; k<j; k++)
		{
		if(fabs(maxBuf[channelExtract][k]) >= thresholdChannels[channelExtract])
		    {
		    for(m = 0;m<fadeNoSamples;++m)
			{
			if(buffer[channelExtract][k] > 0) /* If positive */
			    {
			    //for( p = 0; p < h->channels; p++)
			    for( p = 0; p < channels; p++)
				{
				if(p == channelExtract)
				    fprintf(out,"%d\n",(int)initialFadeIn[m]);
				else
				    if(stereoFlag) fprintf(out,"0\n");
				}
			    }
			else  /* If negative */
			    {
			    for( p = 0; p < channels; p++)
				{
				if(p == channelExtract)
				    fprintf(out,"%d\n",(int)(-1*initialFadeIn[m]));
				else
				    if(stereoFlag) fprintf(out,"0\n");
				}
			    }
			}
		    break;    
		    }
		}

	    /*
	     *
	     * Fill in the real
	     * samples...
	     * 
	     *
	     */
	    for(; k<j; k++)
		{
		fprintf(out,"%d\n",buffer[channelExtract][k]);
		for(p = 0; p < channels; p++)
		    {
		    if((p != channelExtract) && (stereoFlag))
			fprintf(out,"%d\n",buffer[p][k]);
		    }
		}

	    /*
	     * Fill fade out samples...
	     *
	     */
	    for(m=l-1; m >= 0; --m)
		{
		if(buffer[channelExtract][j-1] > 0)
		    {
		    for( p = 0; p < channels; p++)
			{
			if(p == channelExtract)
			    fprintf(out,"%d\n",(int)initialFadeOut[m]);
			else
			    if(stereoFlag) fprintf(out,"0\n");
			}
		    }
		else
		    {
		    for( p = 0; p < channels; p++)
			{
			if(p == channelExtract)
			    fprintf(out,"%d\n",(int)(-1*initialFadeOut[m]));
			else
			    if(stereoFlag) fprintf(out,"0\n");
			}
		    }
		}

	    fprintf(stderr,"closing %s ... \n",fileName);
	    fprintf(stderr,"closing file at %3.2f seconds \n",samplesToSeconds(j));

	    fclose(out);
	    flag = 0;
	    flagInit = 0;
	    free(initialFadeIn);
	    free(initialFadeOut);
	    continue;
	    }
	}
    free(maxBuf);	    
    return 1;
    }

static void
batch(int argc,char **argv)
    {
    int i;
    char buffer[512];
    char *base;
    char *suffix = ".rra";
    FILE *in,*out;

    for (i = 0; i < argc; ++i)
        {
        fprintf(stderr,"converting %s to RRA\n",argv[i]);
        in = OpenFile(argv[i],"r");
        base = strtok(argv[i],".");
        if (strlen(base) > sizeof(buffer) - (strlen(suffix) + 2))
            Fatal("file name too long\n");
        sprintf(buffer,"%s%s",base,suffix);
        out = OpenFile(buffer,"w");
        extract(in,out,argv);
        }
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

	fprintf(stderr,"%s \n",argv[argIndex]);

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
	    case 'c':
		channelExtract = atoi(&argv[argIndex][3]);
		argUsed = 1;
		printf("%s channel %d\n",PROGRAM_NAME, channelExtract);
		break;

	    case 's':
		printf("%s: Output: stereo channel\n",PROGRAM_NAME); 
		stereoFlag = 1;
		break;

	    case 't':
		thresholdMeasTime = atof(&argv[argIndex][3]);
		argUsed = 1;
		printf("Threshold measure time: %3.2f seconds \n",thresholdMeasTime); 
		break;

	    case 'i': /* Ignore time */
		skipTime = atof(&argv[argIndex][3]);
		argUsed = 1;
		printf("Ignore/skip time: %3.2f seconds \n",skipTime); 
		break;

	    case 'w': /* Ignore time */
		windowTime = atof(&argv[argIndex][3]);
		argUsed = 1;
		printf("window Time: %3.2f seconds \n",windowTime); 
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

