#include "util.h"

#ifndef __KEY_INCLUDED__
#define __KEY_INCLUDED__

// MODES - do not change these constants or key.c will break

#define IONIAN          0
#define DORIAN          1
#define PHRYGIAN        2
#define LYDIAN          3
#define MIXOLYDIAN      4
#define AEOLIAN         5
#define LOCRIAN         6
#define MELODIC_MINOR   7
#define HARMONIC_MINOR  8
#define STRANGE         9

#define MAJOR           IONIAN
#define MINOR           AEOLIAN
#define NATURAL_MINOR   AEOLIAN
#define NATURAL         AEOLIAN
#define MELODIC         MELODIC_MINOR
#define HARMONIC        HARMONIC_MINOR

extern int setKey(int);
extern int getKey();
extern int setMode(int);
extern int getMode();
extern int degree(int);

extern int getInterval(int,int);
extern int third(int);
extern int fifth(int);
extern int seventh(int);

extern void flatten(void (*)(double,int,int),double,int,int);
extern void sharpen(void (*)(double,int,int),double,int,int);

extern void cchord(int,double,int,int,char *);

extern void c(int,double,int,int);
extern void ic(int,int,double,int,int);
extern void cdis(int,double,int,int);
extern void cM(int,double,int,int);
extern void cm(int,double,int,int);
extern void cd(int,double,int,int);
extern void ca(int,double,int,int);
extern void cf(int,double,int,int);
extern void cs(int,double,int,int);
extern void cpower(int,double,int,int);
extern void cpower2(int,double,int,int);
extern void csus2(int,double,int,int);
extern void csus4(int,double,int,int);
extern void cquart(int,double,int,int);

extern void c7(int,double,int,int);
extern void cV7(int,double,int,int);
extern void cM7(int,double,int,int);
extern void cm7(int,double,int,int);
extern void cd7(int,double,int,int);
extern void ca7(int,double,int,int);
extern void cD7(int,double,int,int);
extern void c7f(int,double,int,int);
extern void c7s(int,double,int,int);
extern void ic7(int,int,double,int,int);
extern void icD7(int,int,double,int,int);

extern void c9(int,double,int,int);
extern void c9f(int, double,int,int);
extern void c9s(int, double,int,int);

extern void c11(int,double,int,int);
extern void c11f(int, double,int,int);
extern void c11s(int, double,int,int);

extern void c13(int,double,int,int);
extern void c13f(int, double,int,int);
extern void c13s(int, double,int,int);

extern void n(int,double,int,int,int);
extern void b(int,double,int,int,char *,...);

extern void maj(double,int,int,int);
extern void min(double,int,int,int);
extern void aug(double,int,int,int);
extern void dim(double,int,int,int);

extern int getcnote(int,int);

extern void i3(double beats,int instrument,int octave,int pitch);
extern void i3p(double beats,int instrument,int octave,int pitch);
extern void i4(double beats,int instrument,int octave,int pitch);
extern void i4p(double beats,int instrument,int octave,int pitch);
extern void i5(double beats,int instrument,int octave,int pitch);
extern void i5p(double beats,int instrument,int octave,int pitch);
extern void i6(double beats,int instrument,int octave,int pitch);
extern void i6p(double beats,int instrument,int octave,int pitch);
extern void i7(double beats,int instrument,int octave,int pitch);
extern void i7p(double beats,int instrument,int octave,int pitch);
extern void i8(double beats,int instrument,int octave,int pitch);
extern void i8p(double beats,int instrument,int octave,int pitch);
extern void i9(double beats,int instrument,int octave,int pitch);
extern void i9p(double beats,int instrument,int octave,int pitch);
extern void i37(double beats,int instrument,int octave,int pitch);
extern void i37p(double beats,int instrument,int octave,int pitch);
extern void i38(double beats,int instrument,int octave,int pitch);
extern void i38p(double beats,int instrument,int octave,int pitch);
extern void i47(double beats,int instrument,int octave,int pitch);
extern void i47p(double beats,int instrument,int octave,int pitch);
extern void i49(double beats,int instrument,int octave,int pitch);
extern void i49p(double beats,int instrument,int octave,int pitch);
extern void i57(double beats,int instrument,int octave,int pitch);
extern void i57p(double beats,int instrument,int octave,int pitch);
extern void i59(double beats,int instrument,int octave,int pitch);
extern void i59p(double beats,int instrument,int octave,int pitch);

extern void setBrokenAmplitudes(double,...);
#endif
