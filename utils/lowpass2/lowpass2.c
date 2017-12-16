#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/rra.h"
#include "/usr/local/include/songlib/wave.h"
#include "/usr/local/include/songlib/filters.h"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "lowpass";
char *PROGRAM_VERSION = "0.01";

static int processOptions(int, char **);
static void processData(FILE *,FILE *,RRA *);
static void computeCoefficients(void);
static void computeCoefficients2(void);

static double AmpFactor = 1.00;
static double Freq = 8000;
static double Quality = 0.7071;
static double Gain = 6;
static double Style = LOWPASS;
static char *FType = "cutoff";

static double a0,a1,a2,a2,b1,b2;

/* vocal mastering 
 *
 * high pass filter with ~100 Hz corner
 * notch filter at ~300 Hz low Q
 * notch filter at ~3000 Hz
 * high shelf filter at ~9000 Hz 2 dB
 */

int
main(int argc,char **argv)
    {
    int argIndex = 1;

    FILE *in,*out;
    RRA *h;

    if (strcmp(argv[0],"rralowpass2") == 0)
        {
        Freq = 8000;
        Style = LOWPASS;
        FType = "cutoff";
        }
    else if (strcmp(argv[0],"rrahighpass2") == 0)
        {
        Freq = 100;
        Style = HIGHPASS;
        FType = "cutoff";
        }
    else if (strcmp(argv[0],"rrabandpass2") == 0)
        {
        Freq = 800;
        Style = BANDPASS;
        FType = "central";
        }
    else if (strcmp(argv[0],"rranotch2") == 0)
        {
        Freq = 2500;
        Quality = 10;
        Style = NOTCH;
        FType = "central";
        }
    else if (strcmp(argv[0],"rrapeak2") == 0)
        {
        Freq = 440;
        Style = PEAK;
        FType = "central";
        }
    else if (strcmp(argv[0],"rralowshelf2") == 0)
        {
        Freq = 350;
        Style = LOWSHELF;
        Gain = -3;                  //default is cut
        FType = "shoulder";
        }
    else if (strcmp(argv[0],"rrahighshelf2") == 0)
        {
        Freq = 8000;
        Style = HIGHSHELF;
        FType = "shoulder";
        }

    argIndex = processOptions(argc,argv);

    computeCoefficients(); //get rid of this?
    computeCoefficients2();

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
        printf("usage: %s [<input rra file> [<output wave file>]]\n",argv[0]);
        exit(-1);
        }

    h = newRRAHeader();
    
    readRRAHeader(in,h,0);

    if (h->channels > 1)
        Fatal("lowpass works on mono files, use slowpass for stero files\n");

    writeRRAHeader(out,h,"modifiedBy: lowpass",0);

    processData(in,out,h);

    fclose(in);
    fclose(out);

    return 0;
    }

static void
computeCoefficients2(void)
    {
    /* see http://http://www.earlevel.com/main/2011/01/02/biquad-formulas/ */
    double V,K,norm,Sqrt2,Sqrt2V,K2,VK2;
    V = pow(10,fabs(Gain)/20.0);
    K = tan(M_PI * Freq / SampleRate);
    Sqrt2 = pow(2.0,0.5);
    Sqrt2V = pow(2.0*V,0.5);
    K2 = K * K;
    VK2 = V * K * K;
    if (Style == LOWPASS)
        {
        norm = 1.0 / (1.0 + K / Quality + K2);
        a0 = K2 * norm;
        a1 = 2.0 * a0;
        a2 = a0;
        b1 = 2.0 * (K2 - 1) * norm;
        b2 = (1.0 - K / Quality + K2) * norm;
        }
    else if (Style == HIGHPASS) //OK
        {
        norm = 1.0 / (1.0 + K / Quality + K2);
        a0 = 1.0 * norm;
        a1 = -2.0 * a0;
        a2 = a0;
        b1 = 2.0 * (K2 - 1) * norm;
        b2 = (1.0 - K / Quality + K2) * norm;
        }
    else if (Style == BANDPASS) //OK
        {
        norm = 1.0 / (1.0 + K / Quality + K2);
        a0 = K / Quality * norm;
        a1 = 0.0;
        a2 = -a0;
        b1 = 2.0 * (K2 - 1) * norm;
        b2 = (1.0 - K / Quality + K2) * norm;
        }
    else if (Style == NOTCH) //OK
        {
        norm = 1.0 / (1.0 + K / Quality + K2);
        a0 = (1.0 + K2) * norm;
        a1 = 2.0 * (K2 - 1.0) * norm;
        a2 = a0;
        b1 = a1;
        b2 = (1.0 - K / Quality + K2) * norm;
        }
    else if (Style == PEAK && Gain >= 0) //OK
        {
        norm = 1.0 / (1.0 + 1.0 / Quality * K + K2);
        a0 = (1.0 + V / Quality * K + K2) * norm;
        a1 = 2.0 * (K2 - 1.0) * norm;
        a2 = (1.0 - V / Quality * K + K2) * norm;
        b1 = a1;
        b2 = (1.0 - 1.0 / Quality * K + K2) * norm;
        }
    else if (Style == PEAK) //Gain < 0 //OK
        {
        norm = 1.0 / (1.0 + V / Quality * K + K2);
        a0 = (1.0 + 1.0 / Quality * K + K2) * norm;
        a1 = 2.0 * (K2 - 1.0) * norm;
        a2 = (1.0 - 1.0 / Quality * K + K2) * norm;
        b1 = a1;
        b2 = (1.0 - V / Quality * K + K2) * norm;
        }
    else if (Style == LOWSHELF && Gain >= 0) //OK
        {
        norm = 1.0 / (1.0 + Sqrt2 * K + K2);
        a0 = (1.0 + Sqrt2V * K + VK2) * norm;
        a1 = 2.0 * (VK2 - 1.0) * norm;
        a2 = (1.0 - Sqrt2V * K + VK2) * norm;
        b1 = 2.0 * (K2 - 1.0) * norm;
        b2 = (1.0 - Sqrt2 * K + K2) * norm;
        }
    else if (Style == LOWSHELF) //Gain < 0) //OK
        {
        norm = 1.0 / (1.0 + Sqrt2V * K + VK2);
        a0 = (1.0 + Sqrt2 * K + K2) * norm;
        a1 = 2.0 * (K2 - 1.0) * norm;
        a2 = (1.0 - Sqrt2 * K + K2) * norm;
        b1 = 2.0 * (VK2 - 1.0) * norm;
        b2 = (1.0 - Sqrt2V * K + VK2) * norm;
        }
    else if (Style == HIGHSHELF && Gain >= 0) //OK
        {
        norm = 1.0 / (1.0 + Sqrt2 * K + K2);
        a0 = (V + Sqrt2V * K + K2) * norm;
        a1 = 2.0 * (K2 - V) * norm;
        a2 = (V - Sqrt2V * K + K2) * norm;
        b1 = 2.0 * (K2 - 1.0) * norm;
        b2 = (1.0 - Sqrt2 * K + K2) * norm;
        }
    else if (Style == HIGHSHELF) //Gain < 0 //OK
        {
        norm = 1.0 / (V + Sqrt2V * K + K2);
        a0 = (1.0 + Sqrt2 * K + K2) * norm;
        a1 = 2.0 * (K2 - 1.0) * norm;
        a2 = (1.0 - Sqrt2 * K + K2) * norm;
        b1 = 2.0 * (K2 - V) * norm;
        b2 = (V - Sqrt2V * K + K2) * norm;
        }
    else
        Fatal("unknown filter style: %d\n",Style);

    fprintf(stderr,"a0 = %f\n",a0);
    fprintf(stderr,"a1 = %f\n",a1);
    fprintf(stderr,"a2 = %f\n",a2);
    fprintf(stderr,"b1 = %f\n",b1);
    fprintf(stderr,"b2 = %f\n",b2);
    }
static void
computeCoefficients(void)
    {
    double theta,d,beta,delta;
    theta = 2 * M_PI * Freq / SampleRate;
    d = 1 / (double) Quality;
    beta = 0.5 * (1 - (d / 2) * sin(theta)) / (1 + (d / 2) * sin(theta));
    delta = (0.5 + beta) * cos(theta);
    b1 = -2 * delta;
    b2 = 2 * beta;
    if (Style == LOWPASS)
        {
        a0 = (0.5 + beta - delta) / 2;
        a1 = (0.5 + beta - delta);
        a2 = (0.5 + beta - delta) / 2;
        }
    else if (Style == HIGHPASS)
        {
        a0 =  (0.5 + beta + delta) / 2;
        a1 = -(0.5 + beta + delta);
        a2 =  (0.5 + beta + delta) / 2;
        }
    else if (Style == NOTCH)
        {
        a0 =  (0.5 + beta + delta) / 2;
        a1 = -(0.5 + beta + delta);
        a2 =  (0.5 + beta + delta) / 2;
        b1 = a1;
        }
    /*
    double theta,alpha,A0,A1,A2,B0,B1,B2;
    theta = 2 * M_PI * Freq / SampleRate;
    alpha = sin(theta) / (2 * Quality);
    A0 = 1 + alpha;
    A1 = -2 * cos(theta);
    A2 = 1 - alpha;
    if (Style == LOWPASS)
        {
        B0 = (1 - cos(theta)) / 2;
        B1 = 1 - cos(theta);
        B2 = (1 - cos(theta)) / 2;
        }
    else if (Style == HIGHPASS)
        {
        B0 = (1 + cos(theta)) / 2;
        B1 = 1 + cos(theta);
        B2 = (1 + cos(theta)) / 2;
        }
    a0 = B0/A0;
    a1 = B1/A0;
    a2 = B2/A0;
    b1 = A1/A0;
    b2 = A2/A0;
    */
    fprintf(stderr,"a0 = %f\n",a0);
    fprintf(stderr,"a1 = %f\n",a1);
    fprintf(stderr,"a2 = %f\n",a2);
    fprintf(stderr,"b1 = %f\n",b1);
    fprintf(stderr,"b2 = %f\n",b2);
    //a0 = 0.2513790015131591;
    //a1 = 0.5027580030263182;
    //a2 = 0.2513790015131591;
    //b1 = -0.17124071441396285;
    //b2 = 0.1767567204665992;
    }

static void
processData(FILE *in,FILE *out,RRA *h)
    {
    int amp;

    biquad *m = newBiquad(a0,a1,a2,b1,b2);

    amp = readRRAAmplitude(in,out,h->bitsPerSample,outputComment);
    while (!feof(in))
        {
        int d = updateBiquad(m,amp);
        fprintf(out,"%d\n",(int)(d * AmpFactor));
        //getchar();
        amp = readRRAAmplitude(in,out,h->bitsPerSample,outputComment);
        }
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
            case 'v':
                printf("%s version %s\n", argv[0], PROGRAM_VERSION);
                exit(0);
                break;
            case 'a':
                AmpFactor = atof(arg);
                argUsed = 1;
                break;
            case 'f':
                Freq = atof(arg);
                argUsed = 1;
                break;
            case 'g':
                Gain = atof(arg);
                argUsed = 1;
                break;
            case 'q':
                Quality = atof(arg);
                argUsed = 1;
                break;
            case 'h':
                printf("%s options:\n",argv[0]);
                printf("  -a N   set amplitude factor to N\n");
                printf("         values > 1 increase loudness\n");
                printf("         values < 1 decrease loudness\n");
                printf("         default is %f\n",AmpFactor);
                printf("  -f N.N set %s frequency to N.N Hertz\n",FType);
                printf("         default is %f Hz\n",Freq);
            if (Style == PEAK || Style == LOWSHELF || Style == HIGHSHELF)
                {
                printf("  -g N.N set the gain to N.N\n");
                printf("         positive gains are boosts, negative are cuts\n");
                printf("         default is %f dB\n",Gain);
                }
                printf("  -q N.N set the quality factor to N.N\n");
            if (Style == NOTCH)
                printf("         the higher the quality, the shallower and narrower the notch\n");
            else
                printf("         the higher the quality, the more gain at the %s\n",FType);
                printf("         default is %f\n",Quality);
                printf("  -v     display the version number\n");
                printf("  -h     help\n");
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

