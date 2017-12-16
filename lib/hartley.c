#include <stdlib.h>
#include <math.h>
#include "rra.h"
#include "hartley.h"

//This just returns the value of one frequency of the frequency
//domain with an unoptimized Hartley transform.
//Note that freq is not in terms of Hz.  Hz = sampleRate / freq

static double
getFreq(RRA *in, double freq)
    {
	int i;
	double N = (double)in->samples;
	double sum = 0;

	for(i = 0; i < in->samples; i++)
        {
		sum = sum + 
              ((double)in->data[0][i] * (cos((2*M_PI*freq*(double)i)/N) + 
              sin((2*M_PI*freq*(double)i)/N)));
        }
	
	return sum / N;
    }

//Prints out a file with human readable correspondence 
// to frequency data in terms of Hertz.
//Each line is of the form: INDEX HERTZ VALUE

void
printFDist(RRF *freq, RRA *time, FILE *out)
    {
	int i;
	
	for(i = 0; i < freq->samples; i++)
        {
		fprintf(out, "%d %lf %lf\n", i, 
			((double)i * time->sampleRate) /
                (double)time->samples, freq->data[0][i]);
        }
    }

//Extends the length of input RRA to the next power of two,
//then zeros in the extra data.

RRA
*padToPowerTwo(RRA *in)
    {
	//double lgIn;
	int i,power;
	RRA *out;

	//lgIn = log(in->samples) / log(2);
	power = log(in->samples) / log(2);

	if (in - power != 0)
        {
		power = power + 1;
 		out = newRRA(in->sampleRate,in->bitsPerSample,in->channels,
            pow(2, power));
		for(i = 0; i < in->samples; ++i)
			out->data[0][i] = in->data[0][i];
		for( ; i < out->samples; ++i)
			out->data[0][i] = 0;
        return out;
        }
	else 
		return in;
    }

//Returns an interleaved RRA * starting at start.
//e.g. to get every even indexed sample: getIntRRA(in, 0);
//and every odd: getIntRRA(in, 1);

static RRA
*getIntRRA(RRA *in, int start)
    {
	int i, j;
	RRA *out = newRRA(in->sampleRate,in->bitsPerSample,in->channels,
        in->samples / 2);

	j = 0;
	for (i = start; i < in->samples; i = i + 2)
        {
		out->data[0][j] = in->data[0][i];
		j++;
        }

	return out;
}

//a and b should be of the same length
static RRF
*combineFDists(RRF *a,RRF *b)
    {
	int i;
	double N = (double)(a->samples + b->samples);
	RRF *out = (RRF *)malloc(sizeof(RRF));

	out->samples = a->samples + b->samples;
	out->channels = a->channels;
	out->data = (double **)calloc(out->channels, sizeof(double *));	
	
	for(i = 0; i < a->channels; i++)
		out->data[i] = (double *)calloc(out->samples, sizeof(double));
	
	for(i = 0; i < a->samples; i++)
        {
		out->data[0][i] =
            .5 *
                (a->data[0][i] +
                (b->data[0][i]*cos((2*M_PI*(double)i)/N)) + 
                (b->data[0][((int)N - i) % b->samples]*sin((2*M_PI*(double)i)/N)));
        }
	
	for(i = 0; i < a->samples; i++)
        {	
		out->data[0][i+a->samples] =
            .5 * 
            (a->data[0][i] -
            (b->data[0][i]*cos((2*M_PI*(double)i)/N)) - 
            (b->data[0][((int)N - i) % b->samples]*sin((2*M_PI*(double)i)/N)));
        }

	return out;
    }

//To run the radix-2 Coolie-Tukey optimization, the length
//of the input RRA must be baselength * 2^n.  Skew
//appropriately, then deskew OR zero in the remaining samples
//until the next power of two.

static RRF
*CooleyTukey(RRA *in, int baseLength)
    {
	RRF *a,*b,*c;
	int i;
	
	if(in->samples <= baseLength)
        {
		c = (RRF *)malloc(sizeof(RRF));
		c->samples = in->samples;
		c->channels = in->channels;
		c->data = (double **)calloc(c->channels, sizeof(double *));
	
		for(i = 0; i < in->channels; i++)
            {
            c->data[i] = (double *)calloc(c->samples, sizeof(double));
            }

		for(i = 0; i < c->samples; i++)
            {	
			c->data[0][i] = getFreq(in, (double)i);
            }

		return c;
        }
	else
        {
		a = CooleyTukey(getIntRRA(in, 0), baseLength);
		b = CooleyTukey(getIntRRA(in, 1), baseLength);
		c = combineFDists(a, b);
		free(a);
		free(b);
		return c;
        }
    }

RRF
*Hartley(RRA *in)
    {
	return CooleyTukey(in, 1);
    }

RRA *
getRRAWindow(RRA *in, int start, int width)
    {
	int i;
	RRA *out = newRRA(in->sampleRate, in->bitsPerSample, in->channels, width);

	for(i = 0; i < width; i++)
        {
		out->data[0][i] = in->data[0][i + start];
        }

	return out;
    }
