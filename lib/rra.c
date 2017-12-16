
/* rra.c - decode an RRA file
 *
 * written by John C. Lusth
 *
 * WARNING: barely tested!
 *
 */

#define DEFAULT_SAMPLE_RATE 44100
#define DEFAULT_CHANNELS 1
#define DEFAULT_BPS 16
#define DEFAULT_SAMPLES 26460000 // 10 minutes at cd quality

#define _USE_MATH_DEFINES

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <math.h>
#include "rra.h"
#include "util.h"

#define PI  M_PI    /* pi to machine precision, defined in math.h */
#define TWOPI   (2.0*PI)

int LineNumber = 1;

static void defaultTagWriter(FILE *,RRA *);
static void defaultFreePair(char *,void *);
static RRA_TAG *defaultCloneTag(RRA_TAG *);

static void readData(FILE *,RRA *);

static char *readAttribute(FILE *);
static char *readValue(FILE *);
static char *readToken(FILE *,int);
static char *readString(FILE *);
static void fft(double *,int,int);

void updateRRAHeader(FILE *, int );

static void skipWhiteSpace(FILE *,FILE *,void (*)(FILE *,FILE *));

RRA *
newRRA(int sr,int bps,int channels,int size)
    {
    RRA *w = (RRA *) New(sizeof(RRA));

    w->sampleRate = sr;
    w->bitsPerSample = bps;
    w->channels = channels;
    w->skip = 0;
    w->samples = size;
    w->items = 0;
    w->extra1 = 0;
    w->extra2 = 0;

    createRRAData(w);

    return w;
    }

RRA *
newRRAHeader()
    {
    RRA *w = (RRA *) New(sizeof(RRA));

    w->channels = DEFAULT_CHANNELS;
    w->sampleRate = DEFAULT_SAMPLE_RATE;
    w->bitsPerSample = DEFAULT_BPS;
    w->skip = 0;
    w->samples = DEFAULT_SAMPLES;

    w->data = 0;
    w->items = 0;

    return w;
    }

void
createRRAData(RRA *h)
    {
    int i;

    if (h->samples == 0) return;

    //printf("creating space for %d samples\n",h->samples * h->channels);
    //printf("size of int: %d\n",sizeof(int));
    //printf("size of int * samples: %d\n",h->samples * sizeof(int));
    h->data = (int **) New(sizeof(int *) * h->channels);

    for (i = 0; i < h->channels; ++i)
        h->data[i] = (int *) New(sizeof(int) * h->samples);
    }
        
void
clearRRAData(RRA *h)
    {
    int i,j;

    if (h->samples == 0) return;

    for (i = 0; i < h->channels; ++i)
        for (j = 0; j < h->samples; ++j)
            h->data[i][j] = 0;
    }

RRA_TAG *
newRRA_TAG(char *attribute,void *value,RRA_TAG *next)
    {
    RRA_TAG *t = (RRA_TAG *) New(sizeof(RRA_TAG));

    t->attribute = attribute;
    t->value = value;
    t->next = next;

    return t;
    }

void
pushTag(RRA *w,RRA_TAG *t)
    {
    t->next = w->items;
    w->items = t;
    }

void
pushPair(RRA *w,char *attribute,void *value)
    {
    w->items = newRRA_TAG(attribute,value,w->items);
    }

void
freeRRA(RRA *w,void (*handler)(char *,void *))
    {
    int i;
    RRA_TAG *t;

    if (handler == 0) handler = defaultFreePair;

    /* free all data */

    if (w->data)
        {
        for (i = 0; i < w->channels; ++i)
            free(w->data[i]);

        free(w->data);
        }

    /* free all tags */

    t = w->items;
    while (t != 0)
        {
        RRA_TAG *next = t->next;
        handler(t->attribute,t->value);
        free(t);
        t = next;
        }

    free(w);
    }

RRA *
cloneRRA(RRA *v,RRA_TAG *(*handler)(RRA_TAG *))
    {
    int i,j;
    RRA_TAG *t;
    RRA *w;

    if (handler == 0) handler = defaultCloneTag;

    w = newRRA(v->sampleRate,v->bitsPerSample,v->channels,v->samples);
    w->skip = v->skip;

    /* transfer over the data */

    for (i = 0; i < v->channels; ++i)
        for (j = 0; j < v->samples; ++j)
            w->data[i][j] = v->data[i][j];

    /* transfer over the tags */

    t = v->items;
    while (t != 0)
        {
        pushTag(w,handler(t));
        t = t->next;
        }

    w->maxAmp = v->maxAmp;

    return w;
    }

void
displayRRA(RRA *w)
    {
    printf("channels: %d\n",w->channels);
    printf("sampleRate: %d\n",w->sampleRate);
    printf("bitsPerSample: %d\n",w->bitsPerSample);
    printf("skip: %d\n",w->skip);
    printf("samples: %d\n",w->samples);
    printf("maxAmp: %d\n",w->maxAmp);
    }

RRA *
readRRA(FILE *in,void (*handler)(RRA *,char *,void *))
    {
    RRA *w;

    w = newRRAHeader();

    readRRAHeader(in,w,handler);

    /* derived values */

    w->maxAmp = (int) pow(2,w->bitsPerSample - 1);

    readData(in,w);

    return w;
    }

void
readRRAHeader(FILE *in,RRA *w,void (*handler)(RRA *,char *,void *))
    {
    int ch;
    char *attribute,*value;

    if (handler == 0) handler = pushPair;

    attribute = readAttribute(in);

    if (attribute == 0)
        Fatal("not an RRA file: first attribute should be RRAUDIO\n");

    /* check if raw data file -- if so, just return */
    if (isdigit(*attribute))
        {
        ungetc(*attribute,in);
        return;
        }

    if (strcmp(attribute,"RRAUDIO") != 0)
        Fatal("not an RRA file: first attribute is %s (should be RRAUDIO)\n",
            attribute);

    attribute = readAttribute(in);
    while (attribute != 0)
        {
        skipWhiteSpace(in,0,discardComment);

        ch = fgetc(in);
        if (ch == EOF)
            Fatal("premature end of file\n");
        else if (ch != ':')
            Fatal("line %d, expected a colon after attribute %s\n",
                LineNumber,attribute);

        value = readValue(in);

        if (strcmp(attribute,"samples") == 0)
            {
            w->samples = atoi(value);
            if (w->samples < 0)
                Fatal("attribute samples must have a non-zero value\n");
            }
        else if (strcmp(attribute,"skip") == 0)
            {
            w->skip = atoi(value);
            if (w->skip < 0)
                Fatal("attribute skip must have a non-zero value\n");
            }
        else if (strcmp(attribute,"sampleRate") == 0)
            {
            w->sampleRate = atoi(value);
            if (w->sampleRate < 1)
                Fatal("attribute sampleRate must have a non-zero value\n");
            }
        else if (strcmp(attribute,"bitsPerSample") == 0)
            {
            w->bitsPerSample = atoi(value);
            if (w->bitsPerSample < 1)
                Fatal("attribute bitsPerSample must have a non-zero value\n");
            }
        else if (strcmp(attribute,"channels") == 0)
            {
            w->channels = atoi(value);
            if (w->channels < 1)
                Fatal("attribute channels must have a non-zero value\n");
            }
        else
            handler(w,attribute,value);

        attribute = readAttribute(in);
        }
    }

void *
getAttributeRRA(RRA *h,char *attribute)
    {
    RRA_TAG *t;

    t  = h->items;
    while (t != 0)
        {
        if (strcmp(attribute,t->attribute) == 0) 
            return t->value;
        t = t->next;
        }

    return 0;
    }

void *
setAttributeRRA(RRA *h,char *attribute,void *value)
    {
    RRA_TAG *t;

    t  = h->items;
    while (t != 0)
        {
        if (strcmp(attribute,t->attribute) == 0) 
            {
            void *temp = t->value;
            t->value = value;
            return temp;
            }
        t = t->next;
        }

    return 0;
    }

static char *
readAttribute(FILE *in)
    {
    int ch;
    char *t;

    skipWhiteSpace(in,0,discardComment);

    ch = fgetc(in);

    if (ch == EOF)
        t = 0;
    else if (isdigit(ch))
        {
        char buffer[2];
        buffer[0] = ch;
        buffer[1] = '\0';
        return strdup(buffer);
        }
    else
        {
        t = readToken(in,ch);
        if (strcmp(t,"%%") == 0) 
            t = 0;
        }

    return t;
    }

static char *
readValue(FILE *in)
    {
    int ch;
    char *t;

    skipWhiteSpace(in,0,discardComment);

    ch = fgetc(in);

    if (ch == '"')
        {
        t = readString(in);
        }
    else
        {
        t = readToken(in,ch);
        if (strcmp(t,"%%") == 0)
            Fatal("missing value in attribute/value pair");
        }

    return t;
    }

static void
readData(FILE *in,RRA *w)
    {
    int amp;
    int spot;

    w->samples = w->sampleRate * 10;

    createRRAData(w);

    spot = 0;

    amp = readRRAAmplitude(in,0,w->bitsPerSample,discardComment);
    while (!feof(in))
        {
        if (spot / w->channels == w->samples) growRRA(w);
        w->data[spot % w->channels][spot / w->channels] = amp;
        amp = readRRAAmplitude(in,0,w->bitsPerSample,discardComment);
        ++spot;
        }

    if (spot % w->channels != 0) Warning("channel %d is short\n", spot % w->channels);

    /* remove the excess space in the arrays */

    shrinkRRA(w,spot / w->channels);
    }

int
readRRAAmplitude(FILE *in,FILE *out,int bps,void (*handler)(FILE *, FILE *))
    {
    int ch;
    char s[512] = "";
    int count;
    int first;
    int decimal;
    int floater;
    int exponent;
    static int storedBPS = -1;
    static double storedMax;

    if (storedBPS != bps)
        {
        storedBPS = bps;
        storedMax = pow(2,bps - 1) - 1;
        }

    if (feof(in)) return -1;

    first = 1;
    decimal = 0;
    floater = 0;
    exponent = 0;

    skipWhiteSpace(in,out,handler);

    count = 0;
    ch = fgetc(in);
    while (isdigit(ch) || (first && ch == '-') || (!decimal && ch == '.')
        || (!exponent && (ch == 'E' || ch == 'e')))
        {
        first = 0;
        if (ch == '.')
            {
            decimal = 1;
            floater = 1;
            }
        if (ch == 'E' || ch == 'e')
            {
            decimal = 1;
            floater = 1;
            exponent = 1;
            first = 1;
            }
        s[count++] = ch;
        if (count >= sizeof(s) - 1)
            Fatal("RRA FORMAT ERROR, line %d: number is too large\n",
                LineNumber);
        ch = fgetc(in);
        }

    s[count] = '\0';

    ungetc(ch, in);

    //printf("number is %s\n", s);

    if (strcmp(s,"-") == 0)
        Fatal("RRA FORMAT ERROR, line %d: lonely dash found\n", LineNumber);
    else if (strcmp(s,".") == 0)
        Fatal("RRA FORMAT ERROR, line %d: lonely dot found\n", LineNumber);

    if (floater)
        {
        //printf("!floater is %s (a fraction of 2^%d)\n",s,bps); 
        //printf("!%d\n", (int) (atof(s) * storedMax));
        return (int) (atof(s) * storedMax);
        }
    else
        return atoi(s);
    }

int
readRRAAmplitudeFD(int in,int bps,int *size)
    {
    char ch;
    char s[512] = "";
    int count;
    int first;
    int decimal;
    int floater;
    int exponent;
    static int storedBPS = -1;
    static double storedMax;

    if (storedBPS != bps)
        {
        storedBPS = bps;
        storedMax = pow(2,bps - 1) - 1;
        }

    first = 1;
    decimal = 0;
    floater = 0;
    exponent = 0;

    //skip white space
    *size = 1;
    read(in,&ch,1);
    while (isspace(ch))
        {
        read(in,&ch,1);
        ++*size;
        }

    //printf("first char is %c\n",ch);
    
    count = 0;
    while (isdigit(ch) || (first && ch == '-') || (!decimal && ch == '.')
        || (!exponent && (ch == 'E' || ch == 'e')))
        {
        first = 0;
        if (ch == '.')
            {
            decimal = 1;
            floater = 1;
            }
        if (ch == 'E' || ch == 'e')
            {
            decimal = 1;
            floater = 1;
            exponent = 1;
            first = 1;
            }
        s[count++] = ch;
        if (count >= sizeof(s) - 1)
            Fatal("RRA FORMAT ERROR, line %d: number is too large\n",
                LineNumber);
        ++*size;
        read(in,&ch,1);
        }

    s[count] = '\0';

    //printf("number is %s\n", s);

    if (strcmp(s,"-") == 0)
        Fatal("RRA FORMAT ERROR, line %d: lonely dash found\n", LineNumber);
    else if (strcmp(s,".") == 0)
        Fatal("RRA FORMAT ERROR, line %d: lonely dot found\n", LineNumber);

    if (floater)
        {
        //printf("!floater is %s (a fraction of 2^%d)\n",s,bps); 
        //printf("!%d\n", (int) (atof(s) * storedMax));
        return (int) (atof(s) * storedMax);
        }
    else
        return atoi(s);
    }

int
findPreviousRRAAmplitudeFD(int fd)
    {
    char ch;
    int comment;
    off_t offset;

    //find beginning of the line
    comment = 0;
    read(fd,&ch,1);
    while (ch != '\n')
        {
        //printf("ch is <%c>\n",ch);
        if (ch == '!') comment = 1;
        offset = lseek(fd,-2,SEEK_CUR);
        read(fd,&ch,1);
        }

    if (comment)
        {
        lseek(fd,-1,SEEK_CUR);
        return findPreviousRRAAmplitudeFD(fd);
        }

    //find beginning of token
    read(fd,&ch,1);
    while (isspace(ch))
        {
        read(fd,&ch,1);
        }

    offset = lseek(fd,-1,SEEK_CUR);

    return offset;
    }

void
growRRA(RRA *w)
    {
    shrinkRRA(w,w->samples * 2);
    }

void
shrinkRRA(RRA *w,int size)
    {
    int i;

    w->samples = size;
    for (i = 0; i < w->channels; ++i)
        w->data[i] = (int *) ReNew(w->data[i],sizeof(int) * size);
    }

void
writeRRA(FILE *out,RRA *w,char *tag, void (*handler)(FILE *,RRA *), int offset)
    {
    int i,j;
    int samples;
    
    
    /* save the original samples because we need to reduce then */
    /* number written by 'skip' */

    samples = w->samples;

    w->samples -= w->skip;

    if (w->samples < 0) w->samples = 0;

    writeRRAHeader(out,w,tag,handler);

    for (i = 0; i < samples; ++i)
        for (j = 0; j < w->channels; ++j)
            if (i >= w->skip)
                fprintf(out,"%d\n",w->data[j][i] + offset);
    }

static int determineDigits(int x)    
    {
    int count = 1;

    if(x / 10) count = count + determineDigits(x / 10);
    
    return count;
    }

void
writeRRAHeader(FILE *out,RRA *w,char *tag, void (*handler)(FILE *,RRA *))
    {
    int digits,i;
    int prevSkip=-1,prevBPS=-1,prevChannels=-1,prevSamples=-1,prevSR=-1;
    if (handler == 0) handler = defaultTagWriter;

    fprintf(out,"RRAUDIO\n");

    handler(out,w);

    if (w->channels != DEFAULT_CHANNELS && w->channels != prevChannels)
        {
        fprintf(out,"channels: %d\n",w->channels);
        prevChannels = w->channels;
        }
    if (w->sampleRate != DEFAULT_SAMPLE_RATE && w->sampleRate != prevSR)
        {
        fprintf(out,"sampleRate: %d\n",w->sampleRate);
        prevSR = w->sampleRate;
        }
    if (w->bitsPerSample != DEFAULT_BPS && w->bitsPerSample != prevBPS)
        {
        fprintf(out,"bitsPerSample: %d\n",w->bitsPerSample);
        prevBPS = w->bitsPerSample;
        }
    if (w->skip != 0 && w->skip != prevSkip)
        {
        fprintf(out,"skip: %d\n",w->skip);
        prevSkip = w->skip;
        }

    if (w->samples != 0 && w->samples != prevSamples)
        {
        //fprintf(stderr,"samples: 000000000000000\n");
        fprintf(out,"samples: ");
        digits = determineDigits(w->samples); 
        for(i = 0; i<15-digits;i++)
            fprintf(out,"0");
        //fseek(out,-digits-1,SEEK_CUR);
        fprintf(out,"%d\n",w->samples);
        prevSamples = w->samples;
        }

    fprintf(out,"%s\n",tag);

    fprintf(out,"%%%%\n");
    }


off_t
skipRRAHeaderFD(int fd)
    {
    off_t index;
    char buffer[10];
    //find start
    lseek(fd,0,SEEK_SET);
    read(fd,buffer,sizeof(buffer));
    buffer[7] = '\0';
    if (strcmp(buffer,"RRAUDIO") != 0) return -1;

    index = 0;
    lseek(fd,0,SEEK_SET);
    read(fd,buffer,2);
    while (buffer[0] != '%' || buffer[1] != '%')
        {
        ++index;
        lseek(fd,index,SEEK_SET);
        read(fd,buffer,2);
        }

    return index + 2;
    }

/******** DEFAULT HANDLERS *****************************/

static void
defaultTagWriter(FILE *out,RRA *w)
    {
    RRA_TAG *t;
    char *value;

    t = w->items;
    while (t != 0)
        {
        int i;
        int spaces;

        value = (char *) t->value;

        spaces = 0;
        for (i = 0; i < strlen(value); ++i)
            if (isspace(value[i]))
                {
                spaces = 1;
                break;
                }

        fprintf(out,"%s: %s%s%s\n",
            t->attribute,
            spaces? "\"" : "",
            value,
            spaces? "\"" : ""
            );
        t = t->next;
        }
    }

static void
defaultFreePair(char *attribute,void *value)
    {
    free(attribute);
    free(value);
    }

static RRA_TAG *
defaultCloneTag(RRA_TAG *t)
    {
    return newRRA_TAG(StringDup(t->attribute),StringDup(t->value),0);
    }

/******** PARSING SUPPORT FUNCTIONS ********************/

static void
skipWhiteSpace(FILE *in,FILE *out,void (*handler)(FILE *,FILE *))
    {
    int ch;

    while ((ch = fgetc(in)) && ch != EOF && (isspace(ch) || ch == '!'))
        {
        if (ch == '\n')
            {
            ++LineNumber;
            }
        else if (ch == '!')
            {
            handler(in,out);
            ++LineNumber;
            }
        }

    ungetc(ch,in);
    }

void
outputComment(FILE *in, FILE *out)
    {
    int ch;

    fprintf(out,"!");

    while ((ch = fgetc(in)) && ch != EOF && ch != '\n')
        {
        fprintf(out,"%c",ch);
        continue;
        }
    fprintf(out,"\n");
    }

void
discardComment(FILE *in, FILE *out)
    {
    int ch;

    while ((ch = fgetc(in)) && ch != EOF && ch != '\n')
        continue;
    }

static char *
readString(FILE *in)
    {
    int ch;
    int index;
    char buffer[4096];
    int backslashed;

    index = 0;

    backslashed = 0;
    while ((ch = fgetc(in)) && ch != EOF)
        {
        if (!backslashed && ch == '\"')
            break;

        if (!backslashed && ch == '\\')
            backslashed = 1;
        else
            backslashed = 0;

        if (ch == '\n') ++LineNumber;

        if (index == sizeof(buffer) - 1)
            {
            buffer[sizeof(buffer)-1] = '\0';
            Fatal("line %d, string too long: %s\n",
                    LineNumber,buffer);
            }
        buffer[index++] = ch;
        }

    buffer[index] = '\0';

    if (ch != '\"')
        {
        buffer[sizeof(buffer)-1] = '\0';
        Fatal("line %d,unterminated string: %s\n",LineNumber,buffer);
        }

    return StringDup(buffer);
    }

static char *
readToken(FILE *in,int ch)
    {
    int index;
    char buffer[1024];

    buffer[0] = ch;

    ch = fgetc(in);

    index = 1;
    while (ch != EOF && !isspace(ch) && ch != '\"' && ch != ':')
        {
        if (index == sizeof(buffer) - 1)
            {
            buffer[sizeof(buffer)-1] = '\0';
            Fatal("file %s,line %s, token too long: %s\n",buffer);
            }
        buffer[index++] = ch;
        ch = fgetc(in);
        }

    buffer[index] = '\0';

    ungetc(ch,in);

    return StringDup(buffer);
    }

void
updateRRAHeader(FILE *fp, int samples)
    {
    char *attribute;
    int digits;
    long tmp = ftell(fp);
    fseek(fp,0L,SEEK_SET);
    fprintf(stderr,"fp = %ld \n",ftell(fp));

    attribute = readAttribute(fp);
    fprintf(stderr,"attribute : %s\n",attribute);

    if(!strcmp(attribute,"RRAUDIO"))
        fprintf(stderr,"Its an RRA file \n");
    

    attribute = readAttribute(fp);
    fprintf(stderr,"attribute : %s\n",attribute);
    while(attribute != 0)
        {
        if(strcmp(attribute,"samples") == 0)
            {
            fprintf(fp,": 000000000000000\n");
            digits = determineDigits(samples);
            fseek(fp,-digits-1,SEEK_CUR);
            fprintf(fp,"%d\n",samples);
            fseek(fp,-1,SEEK_CUR);
            attribute = readAttribute(fp);
            fprintf(stderr,"attribute : %s\n",attribute);
            if(!strcmp(attribute,"samples"))
                continue;
            break;
            }
        fprintf(stderr,"attribute : %s\n",attribute);
        attribute = readAttribute(fp);
        }
    fseek(fp,tmp,SEEK_SET);         
    }

double *
rra2fft(RRA *h,int channel,int *length)
    {
    int i;
    double *data = 0;
    double max;

    if (channel >= h->channels)
        Fatal("not enough channels (channel zero is the first channel)\n");

    *length = (int) pow(2,ceil(log((double)h->samples)/log(2.0)));
    //printf("h->channels is %d\n",h->channels);
    //printf("samples is %d\n",h->samples);
    //printf("fftLength is %d\n",*length);

    /* fft uses one-based counting! */

    data = (double *) New(2*((*length)+1) * sizeof(double));

    /* fill the data array data here */

    max = pow(2,h->bitsPerSample - 1);
    //printf("max is %f\n",max);

    for (i = 0; i < h->samples && i < *length; ++i)
        {
        data[i * 2 + 1] = h->data[channel][i] / max;
        data[i * 2 + 2] = 0.0;
        }

    /* fill the remainder of the data array with zeros */

    for ( ; i < *length; ++i)
        {
        data[i * 2 + 1] = 0.0;
        data[i * 2 + 2] = 0.0;
        }

    /* call the fft */

    fft(data,*length,1);

    return data;
    }

void
fft2rra(RRA *h,int channel,double *data,int length)
    {
    int i;
    double max;

    /* call the inverse fft */

    fft(data,length,-1);

    /* normalize the result */

    for(i = 0; i < length; ++i)
        {
        data[2*i+1] /= length;
        data[2*i+2] /= length;
        }

    /* now place back into RRA object */

    max = pow(2,h->bitsPerSample - 1);

    for (i = 0; i < length; i += 2)
        {
        h->data[channel][i / 2] = (int) (max * data[i + 1]);
        }
    }

static void
fft(double *data,int nn,int isign)
    {
    int n, mmax, m, j, istep, i;
    double wtemp, wr, wpr, wpi, wi, theta;
    double tempr, tempi;
    
    n = nn << 1;
    j = 1;
    for (i = 1; i < n; i += 2)
        {
        if (j > i)
            {
            tempr = data[j];     data[j] = data[i];     data[i] = tempr;
            tempr = data[j+1]; data[j+1] = data[i+1]; data[i+1] = tempr;
            }
        m = n >> 1;
        while (m >= 2 && j > m)
            {
            j -= m;
            m >>= 1;
            }
        j += m;
        }
    mmax = 2;
    while (n > mmax)
        {
        istep = 2*mmax;
        theta = TWOPI/(isign*mmax);
        wtemp = sin(0.5*theta);
        wpr = -2.0*wtemp*wtemp;
        wpi = sin(theta);
        wr = 1.0;
        wi = 0.0;
        for (m = 1; m < mmax; m += 2)
            {
            for (i = m; i <= n; i += istep)
                {
                j =i + mmax;
                tempr = wr*data[j]   - wi*data[j+1];
                tempi = wr*data[j+1] + wi*data[j];
                data[j]   = data[i]   - tempr;
                data[j+1] = data[i+1] - tempi;
                data[i] += tempr;
                data[i+1] += tempi;
                }
            wr = (wtemp = wr)*wpr - wi*wpi + wr;
            wi = wi*wpr + wtemp*wpi + wi;
            }
        mmax = istep;
        }
    }

