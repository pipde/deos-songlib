#ifndef __RRA_INCLUDED__
#define __RRA_INCLUDED__

/* rra.h - structure to hold a decoded RRA file 
 *
 * written by John C. Lusth
 *
 * WARNING: barely tested!
 *
 */

#include <stdio.h>
#include <stdint.h>

typedef struct rra_tag_object
    {
    char *attribute;
    void *value;
    struct rra_tag_object *next;
    } RRA_TAG;

typedef struct rra_object
    {
    int channels;
    int sampleRate;
    int bitsPerSample;
    int samples;
    int skip;
    int **data;

    /* derived values */

    int maxAmp;

    RRA_TAG *items;

    void *extra1;
    void *extra2;
    } RRA;

extern int readRRAAmplitude(FILE *, FILE *,int,void (*)(FILE *, FILE *));
extern int readRRAAmplitudeFD(int,int,int *);
extern int findPreviousRRAAmplitudeFD(int);
extern void outputComment(FILE *,FILE *);
extern void discardComment(FILE *,FILE *);

extern RRA *newRRAHeader(void);
extern RRA *newRRA(int,int,int,int);
extern void createRRAData(RRA *);
extern void clearRRAData(RRA *);
extern RRA *readRRA(FILE *,void (*)(RRA *,char *,void *));
extern void readRRAHeader(FILE *,RRA *,void (*)(RRA *,char *,void *));
extern void writeRRA(FILE *,RRA *,char *, void (*)(FILE *,RRA *), int offset);
extern void writeRRAHeader(FILE *,RRA *,char *, void (*)(FILE *,RRA *));
extern void freeRRA(RRA *,void (*)(char *,void *));
extern void growRRA(RRA *);
extern void shrinkRRA(RRA *,int);
extern RRA *cloneRRA(RRA *,RRA_TAG *(*)(RRA_TAG *));

extern void *getAttributeRRA(RRA *,char *);
extern void *setAttributeRRA(RRA *,char *,void *);
extern RRA_TAG *newRRA_TAG(char *,void *,RRA_TAG *);
extern void pushTag(RRA *,RRA_TAG *);
extern void pushPair(RRA *,char *,void *);
extern void updateRRAHeader(FILE *, int);

extern void fft2rra(RRA *,int,double *,int);
extern double *rra2fft(RRA *,int,int *);

/* FILE DESCRIPTOR-BASED FUNCTION */

extern off_t skipRRAHeaderFD(int);

#endif	/* INCLUDED_RRA_H */
