#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "/usr/local/include/songlib/songlib.h"
#include "/usr/local/include/songlib/rra.h"
#include "/usr/local/include/songlib/debug.h"

char *PROGRAM_NAME = "mixer";
char *PROGRAM_VERSION = "0.01";

#define MAX_RRAS 20

/* a quick hack
 */

int
main(int argc,char **argv)
    {
    FILE *fp;
    int i,j,k;
    int inRRACount;
    int channels,samples,bitsPerSample,sampleRate,skip;
    int max;
    double scale;
    RRA *output,*rras[MAX_RRAS];
    char *outFileName;

    // should have two inputs and one output at least

    if (argc < 4)
	{
	puts("mixer inputFile1 inputFile2 [more input files ...] outputFile");
	exit(-1);
	}

    inRRACount = argc - 2; // 1 program name, 1 output file

    outFileName = argv[argc - 1];

    printf("out file: %s\n", outFileName);

    /* read in rras, keeping track of the highest sampleCount */

    samples = channels = sampleRate = bitsPerSample = skip = 0;

    for (i = 0; i < inRRACount; ++i)
	{
	printf("mixing: %s", argv[i + 1]);

	if (i >= MAX_RRAS)
	    {
	    printf(" skipping (too many input files)\n");
	    continue;
	    }

	fp = OpenFile(argv[i + 1],"r");
	rras[i] = readRRA(fp,0);
	fclose(fp);

	printf(" (%d,%d,%d,%d)\n",rras[i]->sampleRate,
	    rras[i]->bitsPerSample,rras[i]->channels,rras[i]->samples);

	if (rras[i] == NULL)
	    {
	    fprintf(stderr, "file %s does not exist!\n", argv[i + 1]);
	    exit(-1);
	    }
	if (rras[i]->samples > samples)
	    {
	    samples = rras[i]->samples;
	    }
	if (rras[i]->channels > channels)
	    {
	    channels = rras[i]->channels;
	    }
	if (rras[i]->skip > skip)
	    {
	    skip = rras[i]->skip;
	    }
	sampleRate = rras[i]->sampleRate;
	bitsPerSample = rras[i]->bitsPerSample;
	}

    /* create our output wav that is as long as the longest input */

    printf("\n");

    printf("output sample rate: %d\n",sampleRate);
    printf("output bits per sample: %d\n",bitsPerSample);
    printf("output channels: %d\n",channels);
    printf("output samples per channel: %d\n",samples);
    printf("output skip: %d\n",skip);

    output = newRRA(sampleRate,bitsPerSample,channels,samples);
    output->skip = skip;

    /* mix in each input wav and keep track of the highest peek */

    max = 0;
    for (i = 0; i < inRRACount; ++i)
	{
	if (i >= MAX_RRAS) continue;
	for (j = 0; j < rras[i]->channels; ++j)
	    {
	    int *in = rras[i]->data[j];
	    int *out = output->data[j];
	    for (k = 0; k < rras[i]->samples; ++k)
		{
		out[k] += in[k];
		if (abs(out[k]) > max) max = abs(out[k]);
		}
	    }
	}

    printf("\n");
    printf("max amplitude: %d\n", max);
    printf("max allowable amplitude: %d\n",
        (int) (pow(2,output->bitsPerSample - 1) - 0.5));

    /* use the peak to calculate a reasonable scale factor */

    scale = (pow(2,output->bitsPerSample - 1) - 1) / max;

    /* fudge factor? */

    scale *= 0.99;

    printf("scaling samples by %g\n", scale);

    /* scale samples */

    for (i = 0; i < output->channels; ++i)
	{
	int *out = output->data[i];
        for (j = 0; j < output->samples; ++j)
	    out[j] = out[j] * scale;
	}

    printf("\n");
    printf("writing out mix...");

    // write the samples out to the last file specified on command line

    fp = fopen(outFileName,"w");
    writeRRA(fp,output,"modifiedBy: mixer",0, bitsPerSample == 8? 127:0);
    printf("finished.\n");
    fclose(fp);

    return 0;
    }
