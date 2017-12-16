#include <stdio.h>


#ifndef __UTIL_INCLUDED__
#define __UTIL_INCLUDED__

extern int ProcessOptions(int,char **);      /* command line option handler */
extern void Fatal(const char *,...);
extern void Warning(const char *,...);
extern FILE *OpenFile(const char *,const char *);
extern void Pause(void);
extern void *New(int);
extern void *ReNew(void *,int);
extern char *StringDup(const char *);
extern double randomRange(double,double);
extern int randomInteger(int,int);

extern int Pausing;
#endif
