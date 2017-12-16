/* Wave.h - structure to hold a decoded canonical WAVE file 
 *
 * written by John C. Lusth
 *
 * WARNING: barely tested!
 *
 */
#include <stdint.h>

#ifndef __WAVE_INCLUDED__
#define __WAVE_INCLUDED__

#define MAX_CHANNELS 5

typedef struct
    {
    char *ID;
    char *Format;
    char *Subchunk1ID;
    char *Subchunk2ID;
    long ChunkSize;
    long Subchunk1Size;
    int AudioFormat;
    int NumChannels;
    int SampleRate;
    int ByteRate;
    int BlockAlign;
    int BitsPerSample;
    long Subchunk2Size;
    int *channels[MAX_CHANNELS];

    /* derived values */

    int sampleCount;
    int sampleWidth;
    int maxAmp;

    FILE* in;
    char* inName;
    FILE* out;
    char *outName;

    double frequency;
    double amplitude;

    double diminishFactor;
    double diminishBottom;
    int diminishStart;

    } WAVE;

extern WAVE *newWave(void);
extern WAVE *cloneWave(WAVE *);
extern WAVE *readWave(const char *);
extern WAVE *readWaveFP(FILE *);
extern void readWaveHeader(WAVE *,FILE *);
extern void displayWave(WAVE *);
extern WAVE *readWaveMono(const char *);
extern WAVE *readWaveMonoFP(FILE *);
//extern WAVE *readFractal(const char *);
extern void writeWave(WAVE *,const char *);
extern void setWaveSize(WAVE *,int);
extern void freeWave(WAVE *w);
extern FILE *openWaveHeader(const char *,int,int,int,int);
extern void writeWaveHeader(FILE *,int,int,int,int);
extern int *newIntArray(int);
extern int *growIntArray(int *);
extern int *copyIntArray(int *);
extern void freeIntArray(int *);
extern double *newDoubleArray(int);
extern double *growDoubleArray(double *);
extern double *copyDoubleArray(double *);
extern void freeDoubleArray(double *);

#define LENGTH(a) ((int) ((a)[-1]))

static inline void writeByteInt(FILE *out, const int number, const int count)
    {
    fwrite(&number, sizeof(uint8_t), count, out);
    }

static inline int readByteInt(FILE *in,int count)
    {
    uint32_t z = 0;
    fread(&z, sizeof(uint8_t), count, in);
    return (int) z;
    }
#endif
