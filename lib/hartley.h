#ifndef __HARTLEY_INCLUDED__
#define __HARTLEY_INCLUDED__

typedef struct{
int samples;
int channels;
double **data;
}RRF;

void printFDist(RRF *d, RRA *h, FILE *out);
RRA *padToPowerTwo(RRA *in);
RRF *hartley(RRA *in);
RRA *getRRAWindow(RRA *in, int start, int width);
#endif
