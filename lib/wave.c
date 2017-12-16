
/* Wave - decode a canonical WAVE file
 *
 * written by John C. Lusth
 *
 * WARNING: barely tested!
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "wave.h"
#include "util.h"

#define NO_ALLOCATE 0
#define ALLOCATE 1

static inline char *readByteString(FILE *,long,int);
static inline void writeByteString(FILE *,char *,int);
static int isID(FILE *,char *id);
static void skip(FILE *,long);

WAVE *
newWave()
    {
    int i;
    WAVE *w = (WAVE *) New(sizeof(WAVE));

    w->ID = 0;
    w->Format = 0;
    w->Subchunk1ID = 0;
    w->Subchunk2ID = 0;
    w->ChunkSize = 0;
    w->Subchunk1Size = 0;
    w->AudioFormat = 0;
    w->NumChannels = 0;
    w->SampleRate = 0;
    w->ByteRate = 0;
    w->BlockAlign = 0;
    w->BitsPerSample = 0;
    w->Subchunk2Size = 0;
    for (i = 0; i < MAX_CHANNELS; ++i)
        w->channels[i] = 0;

    /* derived values */

    w->sampleCount = 0;
    w->sampleWidth = 0;
    w->maxAmp = 0;

    w->inName = 0;
    w->outName = 0;

    w->frequency = 440.0;
    w->amplitude = 1.0;

    w->diminishFactor = 1.0;
    w->diminishBottom = 0.0;
    w->diminishStart = 0;

    return w;
    }

void
freeWave(WAVE *w)
    {
    int i;

    free(w->ID);
    free(w->Format);
    free(w->Subchunk1ID);
    free(w->Subchunk2ID);
    free(w->inName);
    free(w->outName);
    for (i = 0; i < w->NumChannels; ++i)
        freeIntArray(w->channels[i]);

    free(w);
    }

WAVE *
cloneWave(WAVE *v)
    {
    int i;
    WAVE *w = newWave();

    w->ID = v->ID;
    w->Format = v->Format;
    w->Subchunk1ID = v->Subchunk1ID;
    w->Subchunk2ID = v->Subchunk2ID;
    w->ChunkSize = v->ChunkSize;
    w->Subchunk1Size = v->Subchunk1Size;
    w->AudioFormat = v->AudioFormat;
    w->NumChannels = v->NumChannels;
    w->SampleRate = v->SampleRate;
    w->ByteRate = v->ByteRate;
    w->BlockAlign = v->BlockAlign;
    w->BitsPerSample = v->BitsPerSample;
    w->Subchunk2Size = v->Subchunk2Size;
    for (i = 0; i < MAX_CHANNELS; ++i)
        w->channels[i] = v->channels[i];
    w->sampleCount = v->sampleCount;
    w->sampleWidth = v->sampleWidth;
    w->maxAmp = v->maxAmp;

    return w;
    }

void
displayWave(WAVE *w)
    {
    printf("ID: %s\n",w->ID);
    printf("ChunkSize: %ld\n",w->ChunkSize);
    printf("Format: %s\n",w->Format);
    printf("Subchunk1ID: %s\n",w->Subchunk1ID);
    printf("Subchunk1Size: %ld\n",w->Subchunk1Size);
    printf("AudioFormat: %d\n",w->AudioFormat);
    printf("NumChannels: %d\n",w->NumChannels);
    printf("SampleRate: %d\n",w->SampleRate);
    printf("ByteRate: %d\n",w->ByteRate);
    printf("BlockAlign: %d\n",w->BlockAlign);
    printf("BitsPerSample: %d\n",w->BitsPerSample);
    printf("Subchunk2ID: %s\n",w->Subchunk2ID);
    printf("Subchunk2Size: %ld\n",w->Subchunk2Size);
    }

/*
static int
stringIs(char *str,char *upper,char *lower)
    {
    return strcmp(str,upper) == 0 || strcmp(str,lower) == 0;
    }
*/

void
readWaveHeader(WAVE *w,FILE *in)
    {
    w->ID = readByteString(in,4,ALLOCATE);
    //fprintf(stderr,"id is %s\n",w->ID);

    w->ChunkSize = readByteInt(in,4);
    //fprintf(stderr,"chunk size is %ld\n",w->ChunkSize);

    w->Format = readByteString(in,4,ALLOCATE);
    //fprintf(stderr,"format is %s\n",w->Format);

    while (!feof(in))
        {
        if (isID(in,"FMT "))
            {
            skip(in,4);
            break;
            }
        if (isID(in,"fmt "))
            {
            skip(in,4);
            break;
            }
        skip(in,1);
        }
            
    w->Subchunk1Size = readByteInt(in,4);
    //fprintf(stderr,"subchunk 1 size is %ld\n",w->Subchunk1Size);

    w->AudioFormat = readByteInt(in,2);
    //fprintf(stderr,"audio format is %d\n",w->AudioFormat);

    w->NumChannels = readByteInt(in,2);
    //fprintf(stderr,"# channels is %d\n",w->NumChannels);

    if (w->NumChannels == 0) w->NumChannels = 1;

    w->SampleRate = readByteInt(in,4);
    //fprintf(stderr,"sample rate is %d\n",w->SampleRate);

    w->ByteRate = readByteInt(in,4);
    //fprintf(stderr,"byte rate is %d\n",w->ByteRate);

    w->BlockAlign = readByteInt(in,2);
    //fprintf(stderr,"block align is %d\n",w->BlockAlign);

    w->BitsPerSample = readByteInt(in,2);
    //fprintf(stderr,"bits per sample is %d\n",w->BitsPerSample);

    if (w->BitsPerSample == 0) w->BitsPerSample = 16;

    while (!feof(in))
        {
        if (isID(in,"DATA"))
            {
            skip(in,4);
            break;
            }
        if (isID(in,"data"))
            {
            skip(in,4);
            break;
            }
        skip(in,1);
        }

    w->Subchunk2Size = readByteInt(in,4);
    //fprintf(stderr,"subchunk 2 size is %ld\n",w->Subchunk2Size);
            
    /* derived values */

    w->sampleCount = w->Subchunk2Size / (w->BitsPerSample / 8)
        / w->NumChannels;

    //fprintf(stderr,"sample count is %d\n",w->sampleCount);

    w->sampleWidth = w->BitsPerSample / 8;
    w->maxAmp = (int) pow(2,w->BitsPerSample - 1);
    }



WAVE *
readWave(const char *fileName)
    {
    WAVE *w;

    w = readWaveFP(OpenFile(fileName,"r"));
    w->inName = StringDup(fileName);

    fprintf(stderr,"file name is %s\n",w->inName);

    return w;
    }

WAVE *
readWaveFP(FILE *in)
    {
    int i,j;
    WAVE *w;

    w = newWave();

    readWaveHeader(w,in);

    if (w->AudioFormat != 1)
        fprintf(stderr,"WARNING: Audio format is not PCM");

    //fprintf(stderr,"allocating %d samples for each of %d channels\n",
            //w->sampleCount,w->NumChannels);

    for (j = 0; j < w->NumChannels; ++j)
        w->channels[j] = (int *) newIntArray(w->sampleCount);

    for (i = 0; !feof(in) && i < w->sampleCount; ++i)
        for (j = 0; j < w->NumChannels; ++j)
            {
            int amp;
            amp = readByteInt(in,w->sampleWidth);
            if (amp >= w->maxAmp)
                amp = -2 * w->maxAmp + amp;
            //if (i < 1000) printf("read sample %d: %d\n", i, amp);
            w->channels[j][i] = amp;
            }

    //fprintf(stderr,"i is %d\n",i);
    if (i != w->sampleCount)
        {
        fprintf(stderr,"sample count mismatch, setting sample count to %d\n",
            i);
        setWaveSize(w,i);
        }

    fclose(in);

    //fprintf(stderr,"samples allocated\n");

    return w;
    }

WAVE *
readWaveMono(const char *fileName)
    {
    WAVE *w;
    w = readWaveMonoFP(OpenFile(fileName,"r"));
    w->inName = StringDup(fileName);

    fprintf(stderr,"file name is %s\n",w->inName);

    return w;
    }

WAVE *
readWaveMonoFP(FILE *in)
    {
    int i,j;
    WAVE *w;

    if (in == 0) return 0;

    w = newWave();

    readWaveHeader(w,in);

    if (w->AudioFormat != 1)
        Fatal("PCM only (no compression)");

    fprintf(stderr,"allocating %d samples\n",w->sampleCount);

    w->channels[0] = (int *) newIntArray(w->sampleCount);

    for (i = 0; !feof(in) && i < w->sampleCount; ++i)
        {
        int total = 0;
        for (j = 0; j < w->NumChannels; ++j)
            {
            int amp;
            amp = readByteInt(in,w->sampleWidth);
            if (amp >= w->maxAmp)
                amp = -2 * w->maxAmp + amp;
            total += amp;
            }
        w->channels[0][i] = total;
        }

    if (i != w->sampleCount)
        setWaveSize(w,i);

    if (w->NumChannels != 1)
        {
        fprintf(stderr,"WAVE WARNING: converting to mono\n");
        w->NumChannels = 1;
        setWaveSize(w,w->sampleCount);
        }

    fclose(in);

    fprintf(stderr,"samples allocated\n");

    return w;
    }

FILE *
openWaveHeader(const char *fileName,int rate,int channels,int bps,int samples)
    {
    FILE *out = OpenFile(fileName,"w");
    writeWaveHeader(out,rate,channels,bps,samples);
    return out;
    }

void
writeWaveHeader(FILE *out,int rate,int channels,int bps,int samples)
    {
    writeByteString(out,"RIFF",4);

    writeByteInt(out,36 + samples * channels * (bps / 8),4);

    writeByteString(out,"WAVE",4);

    writeByteString(out,"fmt ",4);

    writeByteInt(out,16,4); //PCM subchunk

    writeByteInt(out,1,2); //PCM (no compression)

    writeByteInt(out,channels,2);  //1 for mono, 2 for stereo

    writeByteInt(out,rate,4); //sample rate

    writeByteInt(out,rate * channels * (bps / 8),4); //byte rate

    writeByteInt(out,channels * (bps / 8),2); //block align

    writeByteInt(out,bps,2); //bits per sample

    writeByteString(out,"data",4);

    writeByteInt(out,samples * channels * (bps / 8),4); //subchunk 2 size

    fprintf(stderr,"header written\n");
    }

void
writeWave(WAVE *w,const char *fileName)
    {
    int i,j;
    FILE *out = OpenFile(fileName,"w");

    free(w->outName);
    w->outName = StringDup(fileName);

    writeByteString(out,w->ID,4);

    writeByteInt(out,w->ChunkSize,4);

    writeByteString(out,w->Format,4);

    writeByteString(out,w->Subchunk1ID,4);

    writeByteInt(out,w->Subchunk1Size,4);

    writeByteInt(out,w->AudioFormat,2);

    if (w->AudioFormat != 1)
        Fatal("PCM only (no compression)");

    writeByteInt(out,w->NumChannels,2);

    writeByteInt(out,w->SampleRate,4);

    writeByteInt(out,w->ByteRate,4);

    writeByteInt(out,w->BlockAlign,2);

    writeByteInt(out,w->BitsPerSample,2);

    writeByteString(out,w->Subchunk2ID,4);

    /* find the largest channel and make its length the sample count */

    w->sampleCount = 0;
    for (i = 0; i < w->NumChannels; ++i)
        if (w->sampleCount < LENGTH(w->channels[i]))
            w->sampleCount = LENGTH(w->channels[i]);

    writeByteInt(out,w->sampleCount * (w->BitsPerSample / 8) * w->NumChannels,4);

    for (i = 0; i < w->sampleCount; ++i)
        for (j = 0; j < w->NumChannels; ++j)
            {
            int amp;
            //if (i % 1000 == 0)
            //    printf("%d samples of %d written\n",i,w->sampleCount);
            if (i >= LENGTH(w->channels[j]))
                amp = 0;
            else
                {
                amp = w->channels[j][i];
                if (amp < 0)
                    amp = amp + 2 * w->maxAmp;
                }
            //if (i < 50) printf("write sample %d: %d\n", i, amp);
            writeByteInt(out,amp,w->sampleWidth);
            }

    fclose(out);
    }

void
setWaveSize(WAVE *w,int samples)
    {
    int bytes = samples * w->NumChannels * w->BitsPerSample / 8;

    w->ChunkSize = 36+bytes;
    w->Subchunk2Size = bytes;
    w->sampleCount = samples;
    }

static void
skip(FILE *in,long count)
    {
    //fprintf(stderr,"skipping %ld bytes\n",count);
    fseek(in,count,SEEK_CUR);
    }

static int
isID(FILE *in,char *id)
    {
    char ch;
    int matching = 1;
    int count = 0;
    while (matching && *id)
        {
        fread(&ch,sizeof(char),1,in);
        if (ch != *id) matching = 0;
        //fprintf(stderr,"comparing %c (%d) with %c\n",ch,ch,*id);
        ++count;
        ++id;
        }
    skip(in,-count);
    //if (matching) 
    //    fprintf(stderr,"match!\n");
    //else
    //    fprintf(stderr,"no match.\n");
    return matching;
    }

static char *
readByteString(FILE *in,long count,int mode)
    {
    if (mode == ALLOCATE)
        {
        char *result = (char *) New(sizeof(char) * (count + 1));

        fread(result, sizeof(char), count, in);
        result[count] = '\0';
        return result;
        }
    else
        {
        fseek(in,count+1,SEEK_CUR);
        return 0;
        }
    }

static inline void
writeByteString(FILE *out,char *s, int count)
    {
    fwrite(s, sizeof(char), count, out);
    }

int *
newIntArray(int size)
    {
    int *a = (int *) New(sizeof(int) * (size+1));

    //fprintf(stderr,"newIntArray: allocating %d integers at location %p\n",size,a);
    a[0] = size;
    return a + 1;
    }

double *
newDoubleArray(int size)
    {
    double *a = (double *) New(sizeof(double) * (size+1));

    fprintf(stderr,"newDoubleArray: allocating %d reals at location %p\n",size,a);

    a[0] = size + 0.01;
    return a + 1;
    }

int *
growIntArray(int *a)
    {
    int i;
    int *b = newIntArray(LENGTH(a) * 2);

    for (i = 0; i < LENGTH(a); ++i)
        b[i] = a[i];

    return b;
    }

double *
growDoubleArray(double *a)
    {
    int i;
    double *b = newDoubleArray(LENGTH(a) * 2);

    for (i = 0; i < LENGTH(a); ++i)
        b[i] = a[i];

    return b;
    }

int *
copyIntArray(int *a)
    {
    int i;
    int *b = newIntArray(LENGTH(a));

    for (i = 0; i < LENGTH(a); ++i)
        b[i] = a[i];

    return b;
    }

double *
copyDoubleArray(double *a)
    {
    int i;
    double *b = newDoubleArray((int) LENGTH(a));

    for (i = 0; i < (int) LENGTH(a); ++i)
        b[i] = a[i];

    return b;
    }


void
freeIntArray(int *a)
    {
    fprintf(stderr,"freeing location %p with length %d\n",a-1,a[-1]);
    a[-1] = 0;
    free(a-1);
    }

void
freeDoubleArray(double *a)
    {
    fprintf(stderr,"freeing location %p with length %d\n",a-1,(int)a[-1]);
    a[-1] = 0.0;
    free(a-1);
    }
