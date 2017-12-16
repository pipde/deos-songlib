/* The following line must be defined before including math.h to */
/* correctly define M_PI */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "/usr/local/include/songlib/songlib.h"
#include "/usr/local/include/songlib/rra.h"
#include "/usr/local/include/songlib/util.h"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "fft";
char *PROGRAM_VERSION = "0.01";

int Shift = 0;

static int processOptions(int, char **);

int
main(int argc,char **argv)
    {
    int argIndex = 1;
    int i;
    FILE *in,*out;
    int length;
    double *data;

    RRA *h;

    argIndex = processOptions(argc,argv);

    if (argc-argIndex == 0)
        {
        in = stdin;
        out = stdout;
        }
    else if (argc-argIndex == 1)
        {
        in = OpenFile(argv[argIndex],"r");
        out = stdout;
        }
    else if (argc-argIndex == 2)
        {
        in = OpenFile(argv[argIndex],"r");
        out = OpenFile(argv[argIndex+1],"w");
        }
    else
        {
        printf("usage: fft -s<N> "
            "[<input rra file> [<output rra file>]]\n");
        exit(-1);
        }

    h = readRRA(in,0);

    length = 0;
    data = rra2fft(h,0,&length);

    //printf("first 10 data points:\n");
    //for (i = 0; i < 10; ++i)
        //printf("%f\n",data[i]);

    if (Shift > 0)
        {
        for (i = length - 1; i >= 0; --i)
            {
            if (i - Shift >= 0)
                {
                data[2*i + 1] = data[2*(i - Shift) + 1];
                data[2*i + 2] = data[2*(i - Shift) + 2];
                }
            else
                {
                //printf("setting bucket %d to zero\n",2*i+1);
                data[2*i + 1] = 0;
                data[2*i + 2] = 0;
                }
            }
        }
    else if (Shift < 0)
        {
        for (i = 0; i < length; ++i)
            {
            if (i - Shift < length)
                {
                data[2*i + 1] = data[2*(i - Shift) + 1];
                data[2*i + 2] = data[2*(i - Shift) + 2];
                }
            else
                {
                //printf("setting bucket %d to zero\n",2*i+1);
                data[2*i + 1] = 0;
                data[2*i + 2] = 0;
                }
            }
        }

    //printf("first 10 data points after shifting:\n");
    //for (i = 0; i < 10; ++i)
        //printf("%f\n",data[i]);

    fft2rra(h,0,data,length);

    writeRRA(out,h,"modifiedBy: fft",0,0);

    fclose(in);
    fclose(out);

    return 0;
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

    while (argIndex < argc && *argv[argIndex] == '-') {

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
             *         PrintActions = 1;
             *         break;
             */
            case 's':
                Shift = atoi(arg);
                argUsed = 1;
                break;
            case 'v':
                printf("%s version %s\n", PROGRAM_NAME, PROGRAM_VERSION);
                exit(0);
                break;
            case 'h':
                printf("fft:\n");
                printf("    a sample filter using a Fourier Transform --\n");
                printf("    the input RRA is transformed and then the");
                printf("    transform is inverted and written on the output\n");
                printf("\n");
                printf("fft options:\n");
                printf("  -s N   shift the spectrum up or down N buckets\n");
                printf("         downward (negative) shifts will lower the pitch\n");
                printf("         upward (postive) shifts will raise the pitch\n");
                printf("         default shift is %d\n",Shift);
                printf("  -h     help\n");
                printf("  -v     display the fft version number\n");
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

#ifdef JUNK
/********************************************************
* The following is a test routine that generates a ramp *
* with 10 elements, finds their FFT, and then finds the *
* original sequence using inverse FFT                   *
********************************************************/

int
main(int argc, char * argv[])
    {
    int i;
    int Nx;
    int NFFT;
    double *x;
    double *X;

    /* generate a ramp with 10 numbers */
    Nx = 10;
    printf("Nx = %d\n", Nx);
    x = (double *) malloc(Nx * sizeof(double));
    for(i=0; i<Nx; ++i)
        {
        x[i] = i;
        }

    /* calculate NFFT as the next higher power of 2 >= Nx */
    NFFT = (int)pow(2.0, ceil(log((double)Nx)/log(2.0)));
    printf("NFFT = %d\n", NFFT);

    /* allocate memory for NFFT complex numbers (note the +1) */
    X = (double *) malloc((2*NFFT+1) * sizeof(double));

    /* Storing x(n) in a complex array to make it work with four1. 
    This is needed even though x(n) is purely real in this case. */
    for(i=0; i<Nx; ++i)
        {
        X[2*i+1] = x[i];
        X[2*i+2] = 0.0;
        }
    /* pad the remainder of the array with zeros (0 + 0 j) */
    for(i=Nx; i<NFFT; ++i)
        {
        X[2*i+1] = 0.0;
        X[2*i+2] = 0.0;
        }

    printf("\nInput complex sequence (padded to next highest power of 2):\n");
    for(i=0; i<NFFT; ++i)
        {
        printf("x[%d] = (%.2f + j %.2f)\n", i, X[2*i+1], X[2*i+2]);
        }

    /* calculate FFT */
    four1(X, NFFT, 1);

    printf("\nFFT:\n");
    for(i=0; i<NFFT; ++i)
        {
        printf("X[%d] = (%.2f + j %.2f)\n", i, X[2*i+1], X[2*i+2]);
        }

    /* calculate IFFT */
    four1(X, NFFT, -1);

    /* normalize the IFFT */
    for(i=0; i<NFFT; ++i)
        {
        X[2*i+1] /= NFFT;
        X[2*i+2] /= NFFT;
        }

    printf("\nComplex sequence reconstructed by IFFT:\n");
    for(i=0; i<NFFT; ++i)
        {
        printf("x[%d] = (%.2f + j %.2f)\n", i, X[2*i+1], X[2*i+2]);
        }

    getchar();
    }

/*

Nx = 10
NFFT = 16

Input complex sequence (padded to next highest power of 2):
x[0] = (0.00 + j 0.00)
x[1] = (1.00 + j 0.00)
x[2] = (2.00 + j 0.00)
x[3] = (3.00 + j 0.00)
x[4] = (4.00 + j 0.00)
x[5] = (5.00 + j 0.00)
x[6] = (6.00 + j 0.00)
x[7] = (7.00 + j 0.00)
x[8] = (8.00 + j 0.00)
x[9] = (9.00 + j 0.00)
x[10] = (0.00 + j 0.00)
x[11] = (0.00 + j 0.00)
x[12] = (0.00 + j 0.00)
x[13] = (0.00 + j 0.00)
x[14] = (0.00 + j 0.00)
x[15] = (0.00 + j 0.00)

FFT:
X[0] = (45.00 + j 0.00)
X[1] = (-25.45 + j 16.67)
X[2] = (10.36 + j -3.29)
X[3] = (-9.06 + j -2.33)
X[4] = (4.00 + j 5.00)
X[5] = (-1.28 + j -5.64)
X[6] = (-2.36 + j 4.71)
X[7] = (3.80 + j -2.65)
X[8] = (-5.00 + j 0.00)
X[9] = (3.80 + j 2.65)
X[10] = (-2.36 + j -4.71)
X[11] = (-1.28 + j 5.64)
X[12] = (4.00 + j -5.00)
X[13] = (-9.06 + j 2.33)
X[14] = (10.36 + j 3.29)
X[15] = (-25.45 + j -16.67)

Complex sequence reconstructed by IFFT:
x[0] = (0.00 + j -0.00)
x[1] = (1.00 + j -0.00)
x[2] = (2.00 + j 0.00)
x[3] = (3.00 + j -0.00)
x[4] = (4.00 + j -0.00)
x[5] = (5.00 + j 0.00)
x[6] = (6.00 + j -0.00)
x[7] = (7.00 + j -0.00)
x[8] = (8.00 + j 0.00)
x[9] = (9.00 + j 0.00)
x[10] = (0.00 + j -0.00)
x[11] = (0.00 + j -0.00)
x[12] = (0.00 + j 0.00)
x[13] = (-0.00 + j -0.00)
x[14] = (0.00 + j 0.00)
x[15] = (0.00 + j 0.00)

*/
#endif
