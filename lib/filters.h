/****************** FILTERS **********************/

#ifndef __FILTERS_INCLUDED__
#define __FILTERS_INCLUDED__

#define LOWPASS 0
#define HIGHPASS 1
#define BANDPASS 2
#define NOTCH 3
#define PEAK 4
#define LOWSHELF 5
#define HIGHSHELF 6

extern void lowPass(int *,int,int,double,double);
extern void highPass(int *, int,int,double,double);
extern void ampLinear(int *,int,double,double,double, double);
extern void ampExponential(int *,int,double,double,double, double);
extern void reverb(int *,int,int,double *,double *,int);
extern void distort1(int *,int,double);
extern void distort2(int *,int,double);
extern void distort3(int *,int,double,double);
extern void amplify(int *,int,double);
extern void attackLinear(int *,int,double,double);
extern void attackExponential(int *,int,double,double);
extern void diminishLinear(int *,int,int,double);
extern void diminishExponential(int *,int,int,double);

extern int SampleRate;

typedef struct delay
    {
    int index;
    int length;
    int *store;
    } delay;

extern delay *newDelay(int);
extern int getDelay(delay *);
extern int updateDelay(delay *,int);
extern int getVarDelay(delay *,double);

typedef struct fof
    {
    double freq;
    double a0;
    double b1;
    delay *sdelay;
    } fof;

extern fof *newFOF(double,int);
extern int getFOF(fof *,int);
extern int updateFOF(fof *,int);

typedef struct sof
    {
    double freq;
    double a0;
    double b1;
    double b2;
    delay *sdelay;
    } sof;

extern sof *newSOF(double,int);
extern int getSOF(sof *,int);
extern int updateSOF(sof *,int);

typedef struct biquad
    {
    double a0;
    double a1;
    double a2;
    double b1;
    double b2;
    delay *ldelay;
    delay *rdelay;
    } biquad;

extern biquad *newBiquad(double,double,double,double,double);
extern int getBiquad(biquad *b,int);
extern int updateBiquad(biquad *b,int);

typedef struct modbiquad
    {
    double wet;
    double dry;
    biquad *bq;
    } modbiquad;

extern modbiquad *newModBiquad(double,double,double,double,double,double,double);
extern int getModBiquad(modbiquad *b,int);
extern int setModBiquad(modbiquad *b,int);

typedef struct ddl
    {
    double wet;
    double dry;
    double fb;
    int length;
    delay *sdelay;
    } ddl;

extern ddl *newDDL(int,double,double);
extern int getDDL(ddl *,int);
extern int updateDDL(ddl *,int);

typedef struct lfo
    {
    double freq;
    double start;
    double step;
    } lfo;

extern lfo *newLFO(double);
extern double getLFO(lfo *);
extern double updateLFO(lfo *);

typedef struct mde
    {
    delay *sdelay;
    lfo *osc;
    double dry;
    double wet;
    double fb;
    int lo;
    int hi;
    } mde;

extern mde *newMDE(double,double,int,double,int,int);
extern int getMDE(mde *,int);
extern int updateMDE(mde *,int);

typedef struct combf
    {
    double fb;
    int length;
    delay *sdelay;
    } combf;

extern combf *newCombF(int,double);
extern int getCombF(combf *,int);
extern int updateCombF(combf *,int);

typedef struct lpc
    {
    double fb;
    int length;
    delay *idelay;
    delay *odelay;
    double gc;
    double glp;
    } lpc;

extern lpc *newLPC(int,double,double);
extern int getLPC(lpc *,int);
extern int updateLPC(lpc *,int);

typedef struct dap
    {
    double fb;
    int length;
    delay *xdelay;
    delay *ydelay;
    double g;
    } dap;

extern dap *newDAP(int,double);
extern int getDAP(dap *,int);
extern int updateDAP(dap *,int);

#endif
