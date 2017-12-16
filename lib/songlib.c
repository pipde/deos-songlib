#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include "songlib.h"
#include <time.h>
#include <dirent.h>
#include <limits.h>

/* change LIBRARY_NAME and LIBRARY_VERSION appropriately */

#define LIBRARY_NAME "songlib";
#define LIBRARY_VERSION "1.47";

#define FADE_LIMIT 0.0001
#define SUSTAIN_DEFAULT 0.999

#define MIN(a,b) ((a) < (b)?(a) : (b))
#define MAX(a,b) ((a) > (b)?(a) : (b))
#define sign(x) ((x) < 0 ? -1 : 1)

#define OUTPUT_LENGTH 100000
#define NOTE_NAME_LENGTH 1024


int interp1(int *,int,double); //linear interpolation
int interp2(int *,int,double); //cosine interpolation
int interp3(int *,int,double); //cubic interpolation

static int interpType = 2;

int (*interp)(int *,int,double);

static int ramp = LINEAR;

static int sampleCounter = 0;
static int beatsPerMeasure = 4;
static int noteValue = 4;
static double drawRamp = 0.20;
static double drawAmpFactor = 2.0;
static double skipAttack = 0;

double strideSlop = 0;
double slopTiming = 0;
double slopAmplitude = 0;
double slopPitch = 0;

double T = 0.125;                 //thirty-secondth note
double S = 0.250;                 //sixteenth note
double I = 0.500;                 //eighth note
double Q = 1.000;                 //quarter note
double H = 2.000;                 //half note
double W = 4.000;                 //whole note

double Td = 0.125 * 1.5;          //dotted thirty-secondth note
double Sd = 0.250 * 1.5;          //dotted sixteenth note
double Id = 0.500 * 1.5;          //dotted eighth note
double Qd = 1.000 * 1.5;          //dotted quarter note
double Hd = 2.000 * 1.5;          //dotted half note
double Wd = 4.000 * 1.5;          //dotted whole note

double Tt = 0.125 * 2.0 / 3.0;    //triplet thirty-secondth note
double St = 0.250 * 2.0 / 3.0;    //triplet sixteenth note
double It = 0.500 * 2.0 / 3.0;    //triplet eighth note
double Qt = 1.000 * 2.0 / 3.0;    //triplet quarter note
double Ht = 2.000 * 2.0 / 3.0;    //triplet half note
double Wt = 4.000 * 2.0 / 3.0;    //triplet whole note

double TD = 0.125 * 4.0 / 3.0;    //downbeat thirty-secondth note
double SD = 0.250 * 4.0 / 3.0;    //downbeat sixteenth note
double ID = 0.500 * 4.0 / 3.0;    //downbeat eighth note
double QD = 1.000 * 4.0 / 3.0;    //downbeat quarter note
double HD = 2.000 * 4.0 / 3.0;    //downbeat half note
double WD = 4.000 * 4.0 / 3.0;    //downbeat whole note

double TU = 0.125 * 2.0 / 3.0;    //upbeat thirty-secondth note
double SU = 0.250 * 2.0 / 3.0;    //upbeat sixteenth note
double IU = 0.500 * 2.0 / 3.0;    //upbeat eighth note
double QU = 1.000 * 2.0 / 3.0;    //upbeat quarter note
double HU = 2.000 * 2.0 / 3.0;    //upbeat half note
double WU = 4.000 * 2.0 / 3.0;    //upbeat whole note

char *NoteNames[NOTES] =
    {"c","c#","d","d#","e","f","f#","g","g#","a","a#","b",};
char *NoteNAMES[NOTES] =
    {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B",};

#define CHORD_LENGTH 512
int lastChord[CHORD_LENGTH];
int lastChordLength = 0;

int style = PLAIN;
double swingBeat = 0.500;
int swingDown = 1;

int TempoChange = 0;
double TempoDelta = 0;

double softLoud = 2;
double slowFast = 2;

int SampleRate = 44100;

/***************** PRIVATE VARIABLES **********************/

static double skip = 0;

static char *outputName;
static RRA *output;

static int closed = 2;

static double amplitude = 1.0;
static int InstrumentCount = 0;

static double beatsPerMinute = 132;
static int bitsPerSample = 16;
static double location = 0;
static int sustainedLocation = 0;
static int measuredLocation = 0;
static int crescendoStart = -1;
static double crescendoFactor = 1;
static int crescendoMode = LINEAR;
static int maxLocation = 0;
static int samplesWritten = 0;
static double previousMeasures = 0;
static double spaces = 0;
static double sustain = SUSTAIN_DEFAULT;
static double attack = 1;
double stride = 0;
int strideMode = 0;
static int strumDirection = 1;

static double primaryEmphasis = 1.8;
static double secondaryEmphasis = 1.3;

static bool useRandomSampling = true;

static int voiceLeading = 0;

typedef struct sample_set_object
    {
    RRA **samples;
    int count;
    } SAMPLE_SET;

/*************** PRIVATE METHOD SIGNATURES *****************/

static SAMPLE_SET *newSampleSetForRRA(RRA *);
static SAMPLE_SET *newSampleSet();
static SAMPLE_SET *addSampleToSampleSet(RRA *, SAMPLE_SET *);
static RRA *getSample(SAMPLE_SET *,bool);
static void readClosestNote(int,int);
static SAMPLE_SET *readSampleSetName(int,int,const char *);
static SAMPLE_SET *readSampleSet(int,int);
static RRA *readNote(int,int);
static int findClosestNote(int,int);
static void ensureFadeIn(RRA *);
static void growOutput(void);
static SAMPLE_SET *resampleSet(SAMPLE_SET *,double);
static void useClosestChord(int *,int);

static void sort(int *,int);

/************** PRIVATE DATA STRUCTURES ********************/ 

static int originals[INSTRUMENTS][OCTAVES * NOTES];
static SAMPLE_SET *scale[INSTRUMENTS][OCTAVES * NOTES];

static char *InstrumentDir[INSTRUMENTS];
static char *InstrumentBase[INSTRUMENTS];
static int pitchShift[INSTRUMENTS];

/********************Added new structure *******************/
typedef struct commentRRA
    {
    double location;
    char comment[512];
    struct commentRRA *next;
    }COMMENT;

static COMMENT *commentBuffer;
static COMMENT *tail = NULL;
static COMMENT *head = NULL;

/*****************************************************/
/************** PUBLIC METHODS ***********************/
/*****************************************************/

/************* REQUIRED FUNCTIONS ********************/

/* function songInit
 *
 * clears out originals so it is possible to tell whether a
 * note has been read in or not.
 *
 * REQUIRED
 *
 * This needs to be changed to accept SampleRate and bitsPerSample
 *
 */

void
songInit()
    {
    memset(originals, 0, sizeof(originals));
    memset(scale, 0, sizeof(scale));

    interp = interp2;
    }

/* function openOutput
 *
 * this function creates the RRA object that notes will
 * be played into - the function closeOutput will write
 * all this data out
 *
 * REQUIRED
 *
 * This needs to be changed to remove SampleRate and bitsPerSample
 *
 */

void
openOutput(char *fn,int sr,int bps)
    {
    if (closed == 0)
        Fatal("attempt to open output wave when "
                "previous has not yet been closed\n");

    if (sr > 0) SampleRate = sr;
    if (bps > 0) bitsPerSample = bps;

    fprintf(stderr,"sampleRate is %d\n",SampleRate);
    fprintf(stderr,"bits per sample is %d\n",bitsPerSample);

    outputName = fn;
    output = newRRA(SampleRate,bitsPerSample,1,OUTPUT_LENGTH);

    clearRRAData(output);

    location = 0;
    maxLocation = 0;

    closed = 0;
    }

/*************** PUBLIC FUNCTIONS ********************/

/* function readScale
 *
 * doesn't actually read the scale but sets up the structure
 * for reading notes on an as-needed basis
 *
 */

int
readScale(char *dirName,char *baseName)
    {
    int inst = InstrumentCount;

    ++InstrumentCount;

    if (InstrumentCount >= INSTRUMENTS)
        Fatal("readScale: too many instruments, the max is %d\n",INSTRUMENTS);

    InstrumentDir[inst] = strdup(dirName);
    InstrumentBase[inst] = strdup(baseName);

    pitchShift[inst] = 0;

    return inst;
    }

/* function readStrings
 *
 * sets up an array where each index holds a string
 * of the instrument with index 0 being muted sounds for
 * each string
 *
 */

int *
readStrings(char *dirName, char *baseName, int strings)
    {
    int i = 0;
    int *result = malloc((strings+1)*sizeof(int));
    if (result == 0)
        {
        fprintf(stderr, "Out of memory\n");
        exit(1);
        }
    for (i = 0; i < (strings + 1); ++i)
        {
        char newBase[512];
        snprintf(newBase, sizeof(newBase), "%s%d_", baseName, i);
        result[i] = readScale(dirName, newBase);
        }
    return result;
    }

/************ BASIC NOTE PLAYING ROUTINES ************/

/* function play
 *
 * the workhorse function of songlib - really a wrapper for dplay
 *
 */

void
_play(double beats,int inst,int octave,int pitch)
    {
    dTrace("entering\n");

    nplay(beats,inst,octave * NOTES + pitch);
    }

PLAYER play = _play;

PLAYER
setPlay(PLAYER f)
    {
    static PLAYER old = (PLAYER) 0;
    if (f == 0)
        {
        PLAYER temp = play;
        play = old;
        return temp;
        }
    else
        {
        old = play;
        play = f;
        return old;
        }
    }

void
drone(double beats,int inst,int octave,int pitch,double length)
    {
    rdrone(beats,getNote(inst,octave,pitch),length);
    }

void
splay(double beats,int inst,int octave,int pitch)
    {
    dTrace("entering\n");

    snplay(beats,inst,octave * NOTES + pitch);
    }

void
_nplay(double beats,int inst,int offset)
    {
    RRA *h;
 
    dTrace("entering\n");

    // if style is SWING,
    //swing beats are pulled longer and shorter, alternately

    if (style == SWING && swingBeat == beats)
        {
        if (swingDown) beats = beats * 4 / 3;
        else beats = beats *2 / 3;
        swingDown = !swingDown;
        }

    h = getNumberedNote(inst,offset);
    dplay(beats,h->data[0],h->samples,h->bitsPerSample);
    }

NPLAYER nplay = _nplay;

NPLAYER
setNPlay(NPLAYER f)
    {
    static NPLAYER old = (NPLAYER) 0;
    if (f == 0)
        {
        NPLAYER temp = nplay;
        nplay = old;
        return temp;
        }
    else
        {
        old = nplay;
        nplay = f;
        return old;
        }
    }

void
ndrone(double beats,int inst,int offset,double length)
    {
    rdrone(beats,getNumberedNote(inst,offset),length);
    }

void
rdrone(double beats,RRA *h,double length)
    {
    dTrace("entering\n");

    while (beats > 1.5 * length)
        {
        dplay(1.5 * length,h->data[0],h->samples,h->bitsPerSample);
        //attack = 1 - sustain;
        backwards(length / 2);
        beats = beats - length;
        }

    dplay(beats,h->data[0],h->samples,h->bitsPerSample);
    }

void
snplay(double beats,int inst,int offset)
    {
    RRA *h;
    int spot;

    dTrace("entering\n");

    h = getNumberedNote(inst,offset);

    spot = getLocation();
    dplay(samplesToBeats(h->samples),h->data[0],h->samples,h->bitsPerSample);
    setLocation(spot + beatsToSamples(beats));
    }

/* function rplay
 *
 * an alternative form of play
 *
 */

void
rplay(double beats,RRA *h)
    {
    dplay(beats,h->data[0],h->samples,h->bitsPerSample);
    }

/* function dplay
 *
 * this function is the heart of songlib - it has two basic tasks:
 *
 * the first is to write the data from the given array to the
 * output for the specified number of beats.
 *
 * the second is to fade out the note data to avoid discontinuites
 * when the next note is played
 *
 */

void
dplay(double beats,int *data,int length, int bps)
    {
    static double beatSlop = 0.0;

    double where;
    double front;
    double middle;

    int skip;
    register double samples = beatsToSamples(beats);
    register int noteSamples = (int) samples;
    register int dataLen;
    register int len;
    register double diminishedAmplitude,increasedAmplitude;
    register int i;
    register double fadeLimit;
    register double bpsFactor;
    double startOfFade;
    double emphasis;

    dTrace("entering\n");
    //printf("playing note at %f\n",location - spaces);

    if (beats == 0) 
        {
        oneshot(data,length,bps);
        return;
        }

    samples = beatsToSamples(beats);
    noteSamples = (int) samples;
    where = samplesToMeasures(location - spaces);

    front = (int) (where + 0.011);

    if (beatsPerMeasure % 2 == 0)
        middle = 0.5;
    else
        middle = (beatsPerMeasure + 1.0) / (2 * beatsPerMeasure);

    //printf("where: %f, front: %f, middle: %f, front + middle: %f\n",
        //where,front,middle,front+middle);
    //getchar();

    if (where > front - 0.01 && where < front + 0.01)
        {
        emphasis = primaryEmphasis;
        //printf("primary emphasis at measure %f\n",where);
        }
    else if (where > front + middle - 0.01 && where < front + middle + 0.01)
        {
        emphasis = secondaryEmphasis;
        //printf("secondary emphasis at measure %f\n",where);
        }
    else
        {
        emphasis = 1.0;
        }

    beatSlop += samples - noteSamples;
    while (beatSlop > 1.0)
        {
        //printf("dplay: adding extra sample\n");
        ++noteSamples;
        beatSlop -= 1.0;
        }

    bpsFactor = pow(2,bitsPerSample) / pow(2,bps);

    skip = beatsToSamples(skipAttack);

    len = MIN(length, noteSamples - skip);
    dataLen = length;

    while (location + noteSamples >= output->samples)
        growOutput();

    /* write out all the data we have up to len (len <= noteSamples) */

    increasedAmplitude = 0;
    for (i = 0; i < len; ++i)
        {
        output->data[0][(int)location] +=
            data[i+skip] * amplitude * bpsFactor * emphasis * increasedAmplitude;
        location += 1;
        if (increasedAmplitude >= 1)
            increasedAmplitude = 1;
        else
            increasedAmplitude += attack;
        }

    /* if we run out of data, replicate the last sample to make up */
    /* the shortfall */
    
    if (len < noteSamples)
        {
        register const int lastSample = data[length-1] * amplitude * emphasis;

        diminishedAmplitude = amplitude * emphasis;
        for (; i < noteSamples; ++i)
            {
            output->data[0][(int)location] += lastSample
                * bpsFactor *diminishedAmplitude;
            location += 1;
            diminishedAmplitude *= sustain;
            }
        }

    /* write out fade/sustained portion */

    startOfFade = location;
    diminishedAmplitude = amplitude * emphasis;
    fadeLimit = FADE_LIMIT / MAX(crescendoFactor,1.0);

    for (; i < dataLen && diminishedAmplitude > fadeLimit; ++i)
        {
        //added the next line (probably should compute the
        //number of samples needed in advance)
        if (location >= output->samples) growOutput();
        output->data[0][(int)location] += 
            data[i] * diminishedAmplitude * bpsFactor;
        location += 1;
        diminishedAmplitude *= sustain;
        }

    if (location > maxLocation) maxLocation = location;

    sustainedLocation = location;
    location = startOfFade;
    //printf("ending note at %f\n",location - spaces);
    //printf("    %f samples\n",samples);

    if (TempoChange) setTempo(getTempo() + beats * TempoDelta);
    }

void
oneshot(int *data,int length, int bps)
    {
    int i;
    double  bpsFactor;

    dTrace("entering\n");
    //printf("playing oneshot note at %f\n",location - spaces);

    bpsFactor = pow(2,bitsPerSample) / pow(2,bps);

    if (length <= 0) return;

    while (location + length >= output->samples)
        growOutput();

    /* write out all the data */

    for (i = 0; i < length; ++i)
        {
        output->data[0][(int)location] +=
            data[i] * amplitude * bpsFactor;
        location += 1;
        }

    if (location > maxLocation) maxLocation = location;
    }

/* function setSeed
 *
 * seeds the random number generator 
 *
 */

void
setSeed(long seed)
    {
    srand((unsigned) seed);
    }

/* function sort
 *
 * sort a set of numbered notes
 *
 */

static void
sort(int *notes,int size)
    {
    int i,j;
    for (i = 0; i < size - 1; ++i)
        {
        int min = i;
        for (j = i+1; j < size; ++j)
            if (notes[j] < notes[min])
                min = j;
        int temp = notes[i];
        notes[i] = notes[min];
        notes[min] = temp;
        }
    }

        
/* function chord
 *
 * play a set of notes simultaneously
 *
 * notes are given as octave/pitch pair plus offsets
 *
 */

void
chord(double beats,int inst,int octave,int pitch,...)
    {
    int offset;
    int noteCount = 0;
    int notes[CHORD_LENGTH];
    va_list ap;

    dTrace("entering\n");;
    /* store the first absolute note */

    notes[noteCount++] = octave * 12 + pitch;

    /* extract the following pitch deltas */

    va_start(ap, pitch);

    offset = va_arg(ap,int);
    while (offset != 0)
        {
        if (offset < - OCTAVES * NOTES || offset >= OCTAVES * NOTES)
            Fatal("chord: bad offset: %d (missing zero terminator?)\n",offset);
        if (noteCount >= sizeof(notes) / sizeof(int))
            Fatal("chord: too many notes\n");
        notes[noteCount++] = notes[0]+offset;
        offset = va_arg(ap,int);
        }
    va_end(ap);

    anchord(beats,inst,notes,noteCount);
    }

/* function achord
 *
 * play a set of notes simultaneously
 *
 * notes are given as a base note octave and pitch and
 * a set of pitch offsets stored in an array
 *
 */

void
achord(double beats,int inst,int octave,int pitch,int *offsets,int length)
    {
    int i;
    int notes[CHORD_LENGTH];

    for (i = 0; i < length; ++i)
        {
        if (i >= sizeof(notes) / sizeof(int))
            Fatal("achord: too many notes\n");
        notes[i] = octave * 12 + pitch + offsets[i];
        }

    anchord(beats,inst,notes,length);
    }

/* function anchord
 *
 * play a set of notes simultaneously
 *
 * notes are given as a base note octave and pitch and
 * a set of pitch offsets stored in an array
 *
 */

void
anchord(double beats,int inst,int *notes,int length)
    {
    int i;
    int spot;
    int oldTempoChange = TempoChange;

    TempoChange = 0;

    if (style == SWING && swingBeat == beats)
        {
        if (swingDown) beats = beats * 4 / 3;
        else beats = beats * 2 / 3;
        swingDown = !swingDown;
        }

    if (voiceLeading) useClosestChord(notes,length);

    //sort the notes from highest to lowest
    //sort(notes,length); // probably should not sort nodes

    spot = getLocation();

    if (strumDirection == STRUM_DOWN)
        {
        if (strideMode == RAGGED)
            {
            for (i = 0; i < length; ++i)
                {
                if (i == length-1) TempoChange = oldTempoChange;
                nplay(beats,inst,notes[i]);
                backwards(beats - stride);
                }
            }
        else if (strideMode == JUSTIFIED)
            {
            for (i = 0; i < length; ++i)
                {
                double b = beats - (stride * i);
                if (i == length-1) TempoChange = oldTempoChange;
                nplay(b,inst,notes[i]);
                backwards(b - stride);
                }

            }
        }
    else if (strumDirection == STRUM_UP)
        {
        if (strideMode == RAGGED)
            {
            for (i = length - 1; i >= 0; --i)
                {
                if (i == 0) TempoChange = oldTempoChange;
                nplay(beats,inst,notes[i]);
                backwards(beats - stride);
                }
            }
        else if (strideMode == JUSTIFIED)
            {
            for (i = length - 1; i >= 0; --i)
                {
                double b = beats - (stride * i);
                if (i == 0) TempoChange = oldTempoChange;
                nplay(b,inst,notes[i]);
                backwards(b - stride);
                }
            }
        }

    //printf("spot was %d\n",spot);
    setLocation(spot+beatsToSamples(beats));
    //printf("spot now is %d\n",getLocation());
    }

/* function rchord
 *
 * play a set of notes simultaneously
 *
 * notes are given as RRA objects
 *
 */

void
rchord(double beats,...)
    {
    int noteCount;
    RRA *note;
    va_list ap;

    if (style == SWING && swingBeat == beats)
        {
        if (swingDown) beats = beats * 4 / 3;
        else beats = beats * 2 / 3;
        swingDown = !swingDown;
        }

    /* extract the arguments */

    va_start(ap, beats);

    noteCount = 0;
    note = va_arg(ap,RRA *);
    while (note != 0)
        {
        if (noteCount > 0) backwards(beats-stride);

        dplay(beats,note->data[0],note->samples,note->bitsPerSample);

        note = va_arg(ap,RRA *);

        ++noteCount;
        }

    va_end(ap);

    backwards((noteCount - 1) * stride);
    }

static int
scoreLeading(int *prev,int *curr,int *prop,int length,int offset,int goUp)
    {
    int i;
    int total = 0;

    if (goUp)
        {
        for (i = 0; i < length; ++i)
            {
            if (i + offset >= length)
                {
                total += abs(prev[i] - (curr[i+offset-length]+12));
                prop[i] = curr[i+offset-length]+12;
                }
            else
                {
                total += abs(prev[i] - curr[i+offset]);
                prop[i] = curr[i+offset];
                }
            }
        }
    else
        {
        for (i = length-1; i >= 0; --i)
            {
            if (i - offset < 0)
                {
                total += abs(prev[i] - (curr[i-offset+length]-12));
                prop[i] = curr[i-offset+length]-12;
                }
            else
                {
                total += abs(prev[i] - curr[i-offset]);
                prop[i] = curr[i-offset];
                }
            }
        }
    return total;
    }

static void
useClosestChord(int *notes,int length)
    {
    int i,j;
    int drop,score,bestScore;
    int proposed[CHORD_LENGTH];
    int bestChord[CHORD_LENGTH];

    /*
     * if this is the first chord played, then just store the chord
     * if the length of the current chord is different then the length
     * of the previous chord, start over
     */

    for (i = 0; i < length; ++i)
        bestChord[i] = notes[i];

    if (lastChordLength == length)
        {
        /* 
         * choose best inversion for each note
         * if the lowest note of the previous chord is lower than the 
         *    root of the current chord, notes can only go down an octave
         * if the lowest note is higher, though, notes can only go up
         */

        /* assume, at the start, the current cord is the best chord */

        //printf("last chord: ");
        //for (i = 0; i < length; ++i) printf("%d ",lastChord[i]);
        //printf("\n");
        //printf("new chord: ");
        //for (i = 0; i < length; ++i) printf("%d ",notes[i]);
        //printf("\n");

        bestScore = INT_MAX;

        drop = lastChord[0] - notes[0];
        //printf("drop is %d\n",drop);

        for (i = 0; drop != 0 && i < length; ++i)
            {
            score = scoreLeading(lastChord,notes,proposed,length,i,drop > 0);
            //printf("proposed chord (%d): ",score);
            //for (j = 0; j < length; ++j) printf("%d ",proposed[j]);
            //printf("\n");
            if (score < bestScore)
                {
                bestScore = score;
                for (j = 0; j < length; ++j)
                    bestChord[j] = proposed[j];
                }
            }
        //printf("better chord: ");
        //for (i = 0; i < length; ++i) printf("%d ",bestChord[i]);
        //printf("\n");
        //getchar();
        }


    /* store this chord */
    for (i = 0; i < length; ++i)
        lastChord[i] = notes[i] = bestChord[i];
    lastChordLength = length;
    sort(lastChord,length);
    }

/* function draw
 *
 * a note playing function which changes the pitch of a note
 * while it's being played (i.e., the note is not restarted 
 * upon subsequent pitch changes
 *
 * The given octave/pitch pair is the note to be resampled.
 * The subsequent octave/pitch/beat triples are the
 * notes to be played.
 *
 */

void
adraw(double length,int inst,int count,int* octaves,int *pitches,double *lengths)
    {
    int i,j;
    int index;
    int octave,pitch;
    RRA *note = 0,*resampled = 0;
    double beats;
    double factor,oldfactor,*factors;
    int baseNote,factorCount;

    dTrace("entering\n");;
    //printf("count is %d\n",count);

    index = 0;
    factor = 1.0;

    /* extract the arguments */

    for (i = 0; i < count; ++i)
        {
        double delta;
        int steps;

        octave = octaves[i];
        pitch = pitches[i];
        beats = lengths[i];

        //printf("[%d,%d,%f]\n",octave,pitch,beats);

        if (i == 0)
            {
            baseNote = octave * NOTES + pitch;
            if (baseNote < 0 || baseNote >= OCTAVES * NOTES)
                Fatal("adraw: bad octave/pitch pair: "
                    "%d,%d\n",octave,pitch);
            note = getNumberedNote(inst,baseNote);
            //printf("%d samples total\n",note->samples);
            factors = (double *) New(sizeof(double) * note->samples);
            factor = 1;
            oldfactor = 1;
            //printf("resampling factor starting at %f\n",factor);
            }
        else
            {
            oldfactor = factor;
            factor = pow(SEMITONE,(octave * NOTES + pitch) - baseNote);
            //printf("resampling factor changing from %f to %f\n",oldfactor,factor);
            }

        factorCount = (int) (beatsToSamples(beats) * factor);
        //printf("factorCount is %d\n",factorCount);
        if (factorCount == 0 && drawRamp <= 0.0)
            steps = 1;
        else
            steps = SampleRate * beatsToSeconds(drawRamp);
        if (steps < 1) steps = 1;
        delta = (factor - oldfactor) / steps;
        //printf("delta is %f, factorCount is %d, steps is %d\n",
        //    delta,factorCount,steps);

        for (j = 0; j < factorCount; ++j)
            {
            factors [index++] = oldfactor;
            if (j < steps) oldfactor += delta;
            }
        }

    /* fill out the rest of the array with the last value of factor */

    for ( ; index < note->samples; ++index)
        factors[index] = oldfactor;

    resampled = aresample(note,factors,index);

    assert(note->samples == index);
    
    dplay(length,resampled->data[0],resampled->samples,note->bitsPerSample);

    freeRRA(resampled,0);
    free(factors);
    }

void
ddraw(double beats,int inst1,int inst2,int octave,int pitch,double length,...)
    {
    int i;
    int octaves[CHORD_LENGTH],pitches[CHORD_LENGTH];
    double lengths[CHORD_LENGTH];

    dTrace("entering\n");;

    octaves[0] = octave;
    pitches[0] = pitch;
    lengths[0] = length;

    /* extract the remaining arguments */

    va_list ap;

    va_start(ap, length);

    octave = va_arg(ap,int);
    i = 1;
    while (octave != (int) 0 && i < sizeof(octaves)/sizeof(int))
        {
        pitch = va_arg(ap,int);
        length = va_arg(ap,double);

        octaves[i] = octave;
        pitches[i] = pitch;
        lengths[i] = length;

        ++i;

        octave = va_arg(ap,int);
        }

    va_end(ap);

    adraw(beats,inst1,i,octaves,pitches,lengths);

    backwards(beats);

    softer(2);
    play(length,inst2,octave,pitch);

    /* extract the remaining arguments */

    va_start(ap, length);

    octave = va_arg(ap,int);
    while (octave != (int) 0)
        {
        pitch = va_arg(ap,int);
        length = va_arg(ap,double);

        play(length,inst2,octave,pitch);
        octave = va_arg(ap,int);
        }

    va_end(ap);

    louder(2);
    }

void
draw(double beats,int inst,int octave,int pitch,double length,...)
    {
    int i;
    int octaves[CHORD_LENGTH],pitches[CHORD_LENGTH];
    double lengths[CHORD_LENGTH];

    dTrace("entering\n");;

    octaves[0] = octave;
    pitches[0] = pitch;
    lengths[0] = length;

    /* extract the remaining arguments */

    va_list ap;

    va_start(ap, length);

    octave = va_arg(ap,int);
    i = 1;
    while (octave != (int) 0 && i < sizeof(octaves)/sizeof(int))
        {
        pitch = va_arg(ap,int);
        length = va_arg(ap,double);

        octaves[i] = octave;
        pitches[i] = pitch;
        lengths[i] = length;

        ++i;

        octave = va_arg(ap,int);
        }

    va_end(ap);

    adraw(beats,inst,i,octaves,pitches,lengths);
    }

void
ndraw(double beats,int inst,int note,double length,...)
    {
    int i;
    int octaves[CHORD_LENGTH],pitches[CHORD_LENGTH];
    double lengths[CHORD_LENGTH];

    dTrace("entering\n");;

    octaves[0] = note / NOTES;
    pitches[0] = note % NOTES;
    lengths[0] = length;

    /* extract the remaining arguments */

    va_list ap;

    va_start(ap, length);

    note = va_arg(ap,int);
    i = 1;
    while (note != (int) 0 && i < sizeof(octaves)/sizeof(int))
        {
        length = va_arg(ap,double);

        octaves[i] = note / NOTES;
        pitches[i] = note % NOTES;
        lengths[i] = length;

        ++i;

        note = va_arg(ap,int);
        }

    va_end(ap);

    adraw(beats,inst,i,octaves,pitches,lengths);
    }

RRA *
aresample(RRA *note,double *factors,int factorLength)
    {
    int UP = 0,DOWN = 1,OFF = 2;
    int i;
    int amping = OFF;
    double maxAmp = drawAmpFactor;
    double t;
    double amp;
    double oldfactor;
    RRA *h;

    h = newRRA(SampleRate,note->bitsPerSample,1,note->samples);

    maxAmp /= 0.8;
    i = 0;
    t = 0;
    amp = 1;
    oldfactor = factors[0];
    while (t < note->samples)
        {
        if (i >= h->samples) growRRA(h);

        h->data[0][i] = interp(note->data[0],note->samples,t); // * amp;

        if (t > factorLength - 1)
            t += factors[factorLength - 1];
        else
            {
            t += factors[(int) t];
            if ((int) t < factorLength && factors[(int) t] != oldfactor)
                {
                //printf("factor change from %.16f to %.16f at %f\n",oldfactor,
                    //factors[(int) t],t);
                if (amping == OFF)
                    {
                    maxAmp = maxAmp * 0.8;
                    amping = UP;
                    //printf("changing to UP at %f\n",t);
                    }
                //printf("   amping is %d, amp is %f\n",amping,amp);
                oldfactor = factors[(int) t];
                }
            }
        ++i;

        if (amping == UP)
            {
            amp *= 1.01;
            //printf("amping up %f at %f\n",amp,t);
            }

        if (amping == UP && amp >= maxAmp)
            {
            amping = DOWN;
            //printf("changing to DOWN at %f\n",t);
            }

        if (amping == DOWN)
            {
            amp *= 0.9999;
            //printf("amping down %f at %f\n",amp,t);
            }

        if (amp < 1)
            {
            amping = OFF;
            //printf("changing to OFF at %f\n",t);
            }

        if (amping == OFF) amp = 1;
        }

    //printf("factor length is %d\n",factorLength);

    /* remove the excess capacity */

    shrinkRRA(h,i);

    dDebug("leaving\n");

    return h;
    }

/* function resolve,resovleAbsolute
*
* a two note version of draw
*
* the first note resolves into the second after 'duration'
*
 * this gives a different sound than playing the first note
 * and then playing the second via 'play'
 *
 */

void
resolve(int inst,int soctave,int spitch,double sbeats,
        int eoctave,int epitch,double ebeats)
    {
    draw(inst,
        eoctave,epitch,0.0,
        soctave,spitch,sbeats,
        eoctave,epitch,ebeats,
        (int) 0
        );
    }

void
nresolve(int inst,int startNote,double sbeats,int endNote,double ebeats)
    {
    //printf("in nresolve...\n");
    ndraw(inst,
        endNote,0.0,
        startNote,sbeats,
        endNote,ebeats,
        (int) 0
        );
    }

void
dtrill(double beats,int inst1,int inst2,int octave,int pitch,int offset,
    double initial,double tlength,int count)
    {
    int i,j;
    int octaves[CHORD_LENGTH];
    int pitches[CHORD_LENGTH];
    double lengths[CHORD_LENGTH];

    octaves[0] = octave;
    pitches[0] = pitch;
    lengths[0] = initial;

    j = 1;
    for (i = 0; j < sizeof(octaves)/sizeof(int) && i < count; ++i)
        {
        octaves[j] = octave;
        pitches[j] = pitch + offset;
        lengths[j] = tlength;
        ++j;
        octaves[j] = octave;
        pitches[j] = pitch;
        lengths[j] = tlength;
        ++j;
        }

    /*
    for (i = 0; i < 2*count+1; ++i)
        printf("[%d,%d,%f]\n",octaves[i],pitches[i],lengths[i]);
    */

    adraw(beats,inst1,2*count+1,octaves,pitches,lengths);

    backwards(beats);

    softer(2);
    play(initial,inst2,octave,pitch);

    for (i = 0; i < count-1; ++i)
        {
        play(tlength,inst2,octave,pitch+offset);
        play(tlength,inst2,octave,pitch);
        }

    play(tlength,inst2,octave,pitch+offset);
    play(beats-initial-2*tlength*(count-1)-tlength,inst2,octave,pitch);

    louder(2);
    }

/* function trill
 *
 * note plays for 'start' beats and then oscillates 'count' times
 * around +range and -range
 *
 * each oscillation lasts 2 * 'delta'
 *
 */

void
trill(double beats,int instrument,int octave,int pitch,int offset,
    double initial,double tlength,int count)
    {
    int i,j;
    int octaves[CHORD_LENGTH];
    int pitches[CHORD_LENGTH];
    double lengths[CHORD_LENGTH];

    octaves[0] = octave;
    pitches[0] = pitch;
    lengths[0] = initial;

    j = 1;
    for (i = 0; j < sizeof(octaves)/sizeof(int) && i < count; ++i)
        {
        octaves[j] = octave;
        pitches[j] = pitch + offset;
        lengths[j] = tlength;
        ++j;
        octaves[j] = octave;
        pitches[j] = pitch;
        lengths[j] = tlength;
        ++j;
        }

    /*
    for (i = 0; i < 2*count+1; ++i)
        printf("[%d,%d,%f]\n",octaves[i],pitches[i],lengths[i]);
    */

    adraw(beats,instrument,2*count+1,octaves,pitches,lengths);
    }

/* function ntrill
 *
 * note plays for 'start' beats and then oscillates 'count' times
 * between 'down' and 'up'
 *
 * each oscillation lasts 2 * 'delta'
 *
 * a wrapper for trill
 *
 */

void
ntrill(double beats,int instrument,int baseNote,int offset,
    double initial,double tlength,int count)
    {
    trill(beats,instrument,baseNote / NOTES,baseNote % NOTES,offset,initial,
        tlength,count);
    }

void
bend(double beats,int inst,int octave,int pitch,int offset,
    double bendLength)
    {
    setDrawRamp(getDrawRamp() * bendLength);
    resolve(beats,inst,octave,pitch+offset,0,octave,pitch);
    //rbend(beats,getNote(inst,octave,pitch),start,startLength,bendLength);
    setDrawRamp(-1); // reset to previous draw ramp
    }

void
nbend(double beats,int inst,int note,double start,
    double startLength, double bendLength)
    {
    rbend(beats,getNumberedNote(inst,note),start,startLength,bendLength);
    }

void
rbend(double beats,RRA *h,double start,
    double startLength,double bendLength)
    {
    dbend(beats,h->data[0],h->samples,start,startLength,bendLength,h->bitsPerSample);
    }

void
dbend(double beats,int *data,int length,double start,
    double startLength,double bendLength,int bps)
    {
    int i;
    int* bentNote;
    int maxSamples,startSamples,bendSamples;
    double t;
    double adjust,delta;
    
    maxSamples = 4 * length;

    bentNote = (int *) New(sizeof(int) * maxSamples);

    /* resample */

    bendSamples = beatsToSamples(bendLength);
    startSamples = beatsToSamples(startLength);
    delta = (1.0 - start) / bendSamples;

    i = 0;
    t = 0;
    adjust = start;
    while (i < maxSamples && i < length)
        {
        bentNote[i] = interp(data,length,t);

        if (i >= startSamples + bendSamples)
            adjust = 1.0;
        else if (i > startSamples)
            adjust += delta;

        /* update the new time */

        t += adjust;
        ++i;
        }

    dplay(beats,bentNote,i,bps);

    free(bentNote);
    }

void
silence(double beats)
    {
    rest(beats);
    }

void
rest(double beats)
    {
    if (style == SWING && swingBeat == beats)
        {
        if (swingDown) beats = beats * 4 / 3;
        else beats = beats *2 / 3;
        swingDown = !swingDown;
        }

    forwards(beats);
    }

void
space(double beats)
    {
    spaces += beatsToSamples(beats);
    rest(beats);
    }

/********************* EFFECTS ***********************/

int 
getVoiceLeading()
    {
    return voiceLeading;
    }

int
setVoiceLeading(int mode)
    {
    int old = voiceLeading;
    voiceLeading = mode;
    return old;
    }
    
int
getCrescendoMode()
    {
    return crescendoMode;
    }

int
setCrescendoMode(int mode)
    {
    int old = crescendoMode;
    crescendoMode = mode;
    return old;
    }

void
beginCrescendo(double factor)
    {
    if (crescendoStart > 0) Fatal("Rewound before crescendo");
    crescendoStart = location;
    crescendoFactor = factor;
    }

void
endCrescendo()
    {
    int i = 0;
    double amp,delta;

    if (crescendoStart < 0 || crescendoStart == sustainedLocation)
        Fatal ("Rewound before crescendo");

    if (crescendoMode == EXPONENTIAL)
        delta = pow(crescendoFactor,1.0 / (sustainedLocation - crescendoStart));
    else
        delta = (crescendoFactor - 1.0) / (sustainedLocation - crescendoStart);

    amp = getAmplitude();

    for (i = crescendoStart; i < sustainedLocation; ++i)
        {
        output->data[0][i] *= amp;
        if (crescendoMode == EXPONENTIAL)
            amp *= delta;
        else
            amp += delta;
        }

    crescendoStart = -1;
    amplitude = getAmplitude() * crescendoFactor;
    crescendoFactor = 1;
    }

void
beginRitardando(double td)
    {
    TempoChange = 1;
    TempoDelta = -td;
    setTempo(getTempo() - td);
    }

void
endRitardando(void)
    {
    TempoChange = 0;
    }

void
beginAccelerando(double td)
    {
    TempoChange = 1;
    TempoDelta = td;
    setTempo(getTempo() + td);
    }

void
endAccelerando(void)
    {
    TempoChange = 0;
    }

/*************** PUBLIC INFORMATION FUNCTIONS  ********************/

char *
getVersion()
    {
    return LIBRARY_VERSION;
    }

int
getSampleRate()
    {
    return SampleRate;
    }

int
getSamplesGenerated()
    {
    return maxLocation;
    }

int
getSamplesWritten()
    {
    return samplesWritten;
    }

int
getBitsPerSample()
    {
    return bitsPerSample;
    }

int
getBytesPerSample()
    {
    return bitsPerSample / 8;
    }

int
getBeatsPerMeasure()
    {
    return beatsPerMeasure;
    }

int
getNoteValue()
    {
    return noteValue;
    }

double
getSkipAttack()
    {
    return skipAttack;
    }

int
getInterp()
    {
    return interpType;
    }

int
getRamp()
    {
    return ramp;
    }

/**************** PUBLIC CONTROL FUNCTIONS ********************/

double 
setSkipAttack(double skip)
    {
    double old = skipAttack;
    skipAttack = skip;
    return old;
    }

int
setInterp(int mode)
    {
    int old;

    old = interpType;
    interpType = mode;

    switch (mode)
        {
        case 1:
            interp = interp1;
            break;
        case 2:
            interp = interp2;
            break;
        case 3:
            interp = interp3;
            break;
        default:
            interp = interp2;
            interpType = 2;
        }

    return old;
    }

int
setRamp(int mode)
    {
    int temp;

    temp = ramp;
    ramp = mode;
    return temp;
    }

int
setSampleRate(int x)
    {
    int old = SampleRate;
    SampleRate = x;
    return old;
    }

void
setTime(int bpm,int note)
    {
    previousMeasures += samplesToMeasures(location - spaces - measuredLocation);
    measuredLocation = location;

    beatsPerMeasure = bpm;
    noteValue = note;

    T  = note / 32.0;
    S  = T * 2.0;
    I  = S * 2.0;
    Q  = I * 2.0;
    H  = Q * 2.0;
    W  = H * 2.0;

    Td = T * 1.5;
    Sd = S * 1.5;
    Id = I * 1.5;
    Qd = Q * 1.5;
    Hd = H * 1.5;
    Wd = W * 1.5;

    Tt = T / 3.0 * 2.0;
    St = S / 3.0 * 2.0;
    It = I / 3.0 * 2.0;
    Qt = Q / 3.0 * 2.0;
    Ht = H / 3.0 * 2.0;
    Wt = W / 3.0 * 2.0;

    TD = St * 2;
    SD = It * 2;
    ID = Qt * 2;
    QD = Ht * 2;
    HD = Wt * 2;
    WD = 2 * Wt * 2;

    TU = St;
    SU = It;
    IU = Qt;
    QU = Ht;
    HU = Wt;
    WU = 2 * Wt;
    }

int
setPitchShift(int inst,int n)
    {
    int old = pitchShift[inst];
    pitchShift[inst] = n;
    return old;
    }

int
getPitchShift(int inst)
    {
    return pitchShift[inst];
    }

double
setSkipBeats(double b)
    {
    double temp = samplesToBeats(skip);
    skip = beatsToSamples(b);
    return temp;
    }

double
getSkipBeats(void)
    {
    return samplesToBeats(skip);
    }

double
setSkipSeconds(double t)
    {
    double temp = samplesToSeconds(skip);
    skip = secondsToSamples(t);
    return temp;
    }

double
getSkipSeconds()
    {
    return samplesToSeconds(skip);
    }

double
setSkipMeasures(double t)
    {
    double temp = samplesToMeasures(skip);
    skip = measuresToSamples(t);
    return temp;
    }

double
getSkipMeasures()
    {
    return measuresToSamples(skip);
    }

double
setSustain(double amt)
    {
    double temp = sustain;
    sustain = amt;
    return temp;
    }

double
getSustain()
    {
    return sustain;
    }

double
setAttack(double amt)
    {
    double temp = attack;
    attack = amt;
    return temp;
    }

double
getAttack()
    {
    return attack;
    }


double
setStride(double amt)
    {
    double temp = stride;
    stride = amt;
    return temp;
    }

double
getStride()
    {
    return stride;
    }

int
setStrideMode(int mode)
    {
    int temp = strideMode;
    strideMode = mode;
    return temp;
    }

int
getStrideMode()
    {
    return strideMode;
    }

double
setStrideSlop(double s)
    {
    double temp = strideSlop;
    strideSlop = s;
    return temp;
    }

int
setStrumDirection(int dir)
    {
    int temp = strumDirection;
    strumDirection = dir;
    return temp;
    }

int getStrumDirection()
    {
    return strumDirection;
    }

double
setAmplitude(double amp)
    {
    double old = amplitude;
    amplitude = amp;
    return old;
    }

double changeAmplitude(double delta)
    {
    double old = amplitude;
    amplitude += delta;
    return old;
    }

static double AmplitudeFactors[512];
static int AmplitudeFactorTop = 0;

static void
pushAmplitudeFactor(double f)
    {
    if (AmplitudeFactorTop < sizeof(AmplitudeFactors) / sizeof(double) - 1)
        AmplitudeFactors[AmplitudeFactorTop++] = f;
    }

static double
popAmplitudeFactor(void)
    {
    if (AmplitudeFactorTop > 0)
        return AmplitudeFactors[--AmplitudeFactorTop];
    Fatal("too many calls to louder/softer with 0.0 amplitude factor\n");
    return 0.0;
    }

void
louder(double factor)
    {
    if (factor == 0.0)
        {
        setAmplitude(getAmplitude() / softLoud);
        softLoud = popAmplitudeFactor();
        }
    else
        {
        setAmplitude(getAmplitude() * factor);
        softLoud = factor;
        pushAmplitudeFactor(factor);
        }
    }

void
softer(double factor)
    {
    if (factor == 0.0)
        {
        setAmplitude(getAmplitude() * softLoud);
        softLoud = popAmplitudeFactor();
        }
    else
        {
        setAmplitude(getAmplitude() / factor);
        softLoud = factor;
        pushAmplitudeFactor(factor);
        }
    }

void
faster(double factor)
    {
    if (factor == 0.0) factor = slowFast;
    setTempo(getTempo() * factor);
    slowFast = factor;
    }

void
slower(double factor)
    {
    if (factor == 0.0) factor = slowFast;
    setTempo(getTempo() / factor);
    slowFast = factor;
    }

double
getAmplitude()
    {
    return amplitude;
    }

double
setTempo(double amount)
    {
    double temp = beatsPerMinute;
    beatsPerMinute = amount;
    return temp;
    }

double
getTempo()
    {
    return beatsPerMinute;
    }


double
setDrawRamp(double amount)
    {
    static double old = 0;
    if (amount < 0)
        {
        drawRamp = old;
        }
    else
        {
        old = drawRamp;
        drawRamp = amount;
        }
    return old;
    }

double
getDrawRamp()
    {
    return drawRamp;
    }

double
setDrawAmpFactor(double amount)
    {
    static double old = 0;
    if (amount < 0)
        {
        drawAmpFactor = old;
        }
    else
        {
        old = drawAmpFactor;
        drawAmpFactor = amount;
        }
    return old;
    }

double
getDrawAmpFactor()
    {
    return drawAmpFactor;
    }

double
setSlopTiming(double s)
    {
    double temp = slopTiming;
    slopTiming = s;
    return temp;
    }

double
setSlopAmplitude(double s)
    {
    double temp = slopAmplitude;
    slopAmplitude = s;
    return temp;
    }

double
setSlopPitch(double s)
    {
    double temp = slopPitch;
    slopPitch = s;
    return temp;
    }

double
getPrimaryEmphasis()
    {
    return primaryEmphasis;
    }
  

double
setPrimaryEmphasis(double factor)
    {
    double temp = primaryEmphasis;
    primaryEmphasis = factor;
    return temp;
    }
  
double
getSecondaryEmphasis()
    {
    return secondaryEmphasis;
    }
  

double
setSecondaryEmphasis(double factor)
    {
    double temp = secondaryEmphasis;
    secondaryEmphasis = factor;
    return temp;
    }
    
    
bool 
getUseRandomSampling()
    {
    return useRandomSampling;    
    }
    
bool 
setUseRandomSampling(bool value)
    {
    bool temp = useRandomSampling;
    useRandomSampling = value;
    return temp;
    }

void
setStyle(char *mode,double info)
    {
    if (strcasecmp(mode,"SWING") == 0)
        {
        style = SWING;
        swingBeat = info;
        swingDown = 1;
        }
    else if (strcasecmp(mode,"PLAIN") == 0)
        style = PLAIN;
    else
        Fatal("Style %s not understood\n",mode);
    }

    
/****************** SAMPLE SET FUNCTIONS ***********************/
SAMPLE_SET *
newSampleSetForRRA(RRA *h)
    {
    return addSampleToSampleSet(h, newSampleSet());
    }
     
SAMPLE_SET *
newSampleSet()
    {
    SAMPLE_SET *ss = (SAMPLE_SET *) New(sizeof(SAMPLE_SET));
    ss->samples = 0;
    ss->count = 0;
    return ss;
    }
    
    
static SAMPLE_SET *
addSampleToSampleSet(RRA *h, SAMPLE_SET *ss)
    {
    int i;
    RRA **oldSamples = ss->samples;
    ss->samples = (RRA **) New(sizeof(RRA *) * (ss->count+1));
    for (i=0;i<ss->count;++i)
        {
        ss->samples[i] = oldSamples[i];
        }
    ss->samples[ss->count] = h;
    ++(ss->count);
    return ss;
    }

static RRA *
getSample(SAMPLE_SET *ss,bool random)
{
    if (!ss) return 0;
    
    // Get random sample if requested and if multiple available
    if (random && ss->count > 1) return ss->samples[rand()%(ss->count)];
    
    // Otherwise, if samples exist, use the first
    if (ss->count > 0) return ss->samples[0];

    // No samples found
    return 0;
}

/************* PUBLIC NOTE RETRIEVAL FUNCTIONS *****************/

/* function getNote
 *
 * a function for retrieving the specified note by either
 * reading it in or resampling a nearby note
 *
 */

RRA *
getNote(int inst,int octave,int pitch)
    {
    return getNumberedNote(inst,octave * NOTES + pitch);
    }

RRA *
setNote(int inst,int octave,int pitch,RRA *h)
    {
    return setNumberedNote(inst,octave * NOTES + pitch,h);
    }

/* function getNumberedNote
 *
 * a rather complicated routine for retrieving a specified note
 *
 * the routine makes four attempts at finding a note
 *
 * first, it checks the scale array to see if it already
 * has been read in
 *
 * then it attempts to load the note exactly as specified
 *
 * if the attempt fails, it assumes the note is from a single
 * octave note collection (an octave value of 3 is used in
 * this case) and attempts to read in the note
 *
 * if that fails, it assumes the note is from a single
 * note collection (octave is 3 and pitch is 0) and attempts
 * to read in the note
 *
 * if that fails, the function finds the closest note
 * and resamples it
 *
 */

static SAMPLE_SET *
setNumberedSampleSet(int inst, int baseNote,SAMPLE_SET *ss)
    {
    SAMPLE_SET *temp = scale[inst][baseNote];
    scale[inst][baseNote] = ss;
    return temp;
    }
    
RRA *
setNumberedNote(int inst, int baseNote,RRA *h)
    {
    SAMPLE_SET *ss = newSampleSetForRRA(h);
    SAMPLE_SET *temp = setNumberedSampleSet(inst, baseNote, ss);
    return temp->samples[0];
    }

int
isOriginalNote(int inst,int octave,int note)
    {
    return isOriginalNumberedNote(inst,octave * 12 + note);
    }

int
isOriginalNumberedNote(int inst,int baseNote)
    {
    (void) getNumberedNote(inst,baseNote);
    return originals[inst][baseNote] == 1;
    }

static SAMPLE_SET *
getNumberedSampleSet(int inst, int baseNote)
    {
    int spot;
    double factor;
    SAMPLE_SET *ss = 0;

    if (baseNote < 0 || baseNote >= OCTAVES * NOTES)
        Fatal("pitch out of range, inst %d, note %d\n",inst,baseNote);
       
    /* see if the note exists */

    if (scale[inst][baseNote] != 0) return scale[inst][baseNote];

    /* it doesn't, so force a read of the closest note */

    readClosestNote(inst,baseNote);

    /* see if the note exists now */

    if (scale[inst][baseNote] != 0) return scale[inst][baseNote];

    /* it doesn't, so find the closest note and pitch shift it */

    spot = findClosestNote(inst,baseNote);

    factor = pow(SEMITONE,baseNote-spot);

    dDebug("factor was %g\n", factor);

    ss = scale[inst][spot];

    /*
    printf("generating %s%s%d (using %s%s%d)\n",
            InstrumentBase[inst],
            NoteNames[baseNote % NOTES],baseNote / NOTES,
            InstrumentBase[inst],
            NoteNames[spot%NOTES],spot/NOTES
            );
    */

    originals[inst][baseNote] = 0;
    
    ss = resampleSet(ss,factor);
    //printf ("    SUCCESS!\n");
    setNumberedSampleSet(inst, baseNote, ss);

    return ss;
    }
    
    
RRA *
getNumberedNote(int inst, int baseNote)
    {
    return getSample(getNumberedSampleSet(inst, baseNote),useRandomSampling);
    }

/*****************************************************/
/*************** PRIVATE METHODS *********************/
/*****************************************************/

/********** PRIVATE NOTE RETRIEVAL ROUTINES **********/

static void
readClosestNote(int inst,int baseNote)
    {
    int i;

    for (i = 0; i < OCTAVES * NOTES; ++i)
        {
        int up = baseNote + i;
        int down = baseNote - i;

        if (up < OCTAVES * NOTES)
            {
            dDebug("looking for note %d\n",up);
            if (originals[inst][up] == 1) return;
            if (readNote(inst,up) != 0) return;
            }

        if (i != 0 && down >= 0)
            {
            dDebug("looking for note %d\n",down);
            if (originals[inst][down] == 1) return;
            if (readNote(inst,down) != 0) return;
            }
        }

    Fatal("No notes found for instrument %d\n"
          "(%s%s)\n",inst,InstrumentDir[inst],InstrumentBase[inst]);
    }

/****** Compare end of string ******/
static int
endsWith(const char *str, const char *suffix)
{
    if (!str || !suffix)
        return 0;
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix >  lenstr)
        return 0;
    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

    
SAMPLE_SET *
readSampleSetName(int inst,int baseNote,const char *name)
    {
    RRA *h = 0;
    SAMPLE_SET *ss = 0;
    FILE *fp = 0;
    DIR *dp = 0;
    struct dirent *ep = 0;     
    char fileNameBuffer[NOTE_NAME_LENGTH];
    char dirNameBuffer[NOTE_NAME_LENGTH];
    
    sprintf(fileNameBuffer, "%s%s.rra", 
        InstrumentDir[inst], name
        );

    dDebug("attempting to read %s\n",fileNameBuffer);
    fp = fopen(fileNameBuffer,"r");

    if (fp)
        {
        h = readRRA(fp,0);
        if (h->samples == 0) Fatal("note %s is empty!\n",fileNameBuffer);
        fclose(fp);
        ensureFadeIn(h);
        ss = newSampleSetForRRA(h);
        dDebug("successfully read base sample %s\n",fileNameBuffer);
        }
        
    sprintf(dirNameBuffer, "%s%s/", 
        InstrumentDir[inst], name
        );
    
    dDebug("attempting to read samples in directory %s\n",dirNameBuffer);
    
    dp = opendir(dirNameBuffer);
    if (dp != NULL)
        {            
        while ((ep = readdir(dp)))
            {
            sprintf(fileNameBuffer, "%s%s/%s", 
                InstrumentDir[inst], name, ep->d_name
                );
                
            // Ensure file is of regular type, and that it ends with .rra
            if (ep->d_type != DT_REG || !endsWith(fileNameBuffer, ".rra"))
                {
                dDebug("skipping entry %s\n",fileNameBuffer);
                continue;
                }
        
            dDebug("attempting to read %s\n",fileNameBuffer);
            fp = fopen(fileNameBuffer,"r");

            if (fp)
                {
                h = readRRA(fp,0);
                if (!h->samples) Fatal("sample %s is empty!\n",fileNameBuffer);
                fclose(fp);
                ensureFadeIn(h);
                if (!ss) ss = newSampleSet();
                addSampleToSampleSet(h,ss);
                dDebug("successfully read directory sample %s\n",fileNameBuffer);
                }
            }
                        
        (void)closedir(dp);
        }
        
    if (!ss) 
        {
        dDebug("no samples found for %s\n",name);
        return 0;
        }
        
    scale[inst][baseNote] = ss;
    originals[inst][baseNote] = 1;
    dDebug("successfully read %d samples for note %s\n",ss->count,name);
        
    return ss;
    } 

static SAMPLE_SET *
readSampleSet(int inst,int baseNote)
    {
    SAMPLE_SET *ss = 0;
    char buffer[NOTE_NAME_LENGTH];

    /* try pitch octave name */

    sprintf(buffer, "%s%s%d",
        InstrumentBase[inst], NoteNames[baseNote % NOTES], baseNote / NOTES
        );
    
    ss = readSampleSetName(inst,baseNote,buffer);
    if (ss != 0) return ss;

    /* try Pitch octave name */

    sprintf(buffer, "%s%s%d",
        InstrumentBase[inst], NoteNAMES[baseNote % NOTES], baseNote / NOTES
        );
    
    ss = readSampleSetName(inst,baseNote,buffer);
    if (ss != 0) return ss;

    /* try baseNote name */

    sprintf(buffer, "%s%d",
        InstrumentBase[inst], baseNote
        );
    
    ss = readSampleSetName(inst,baseNote,buffer);
    if (ss != 0) return ss;

    /* try octave pitch name */

    sprintf(buffer, "%s%d%s",
        InstrumentBase[inst], baseNote / NOTES, NoteNames[baseNote % NOTES]
        );
    
    ss = readSampleSetName(inst,baseNote,buffer);
    if (ss != 0) return ss;

    /* try octave Pitch name */

    sprintf(buffer, "%s%d%s",
        InstrumentBase[inst], baseNote / NOTES, NoteNAMES[baseNote % NOTES]
        );
    
    ss = readSampleSetName(inst,baseNote,buffer);
    if (ss != 0) return ss;

    return 0;
    }

static RRA *
readNote(int inst,int baseNote)
    {
    return getSample(readSampleSet(inst,baseNote),useRandomSampling);
    }

/* function findClosestNote
 *
 * This function finds the closest original (not resampled) note
 * and returns its index
 *
 */

static int
findClosestNote(int inst, int baseNote)
    {
    int i;

    //printf("findClosestNote: instrument %d, octave: %d, note: %s\n",
        //inst,baseNote / NOTES,NoteNames[baseNote % NOTES]);

    for (i = 0; i < OCTAVES * NOTES; ++i)
        {
        int up = baseNote + i;
        int down = baseNote - i;

        if (up < OCTAVES * NOTES && originals[inst][up] == 1)
            return up;

        if (down >= 0 && originals[inst][down] == 1)
            return down;
        }

    Fatal("INTERNAL ERROR: findClosestNote failed\n");

    return 0;
    }

void
replaceNote(int inst1,int octave1,int note1,int inst2,int octave2,int note2)
    {
    nreplaceNote(inst1,octave1 * 12 + note1,inst2,octave2 * 12 + note2);
    }

void
nreplaceNote(int inst1,int pitch1,int inst2,int pitch2)
    {
    printf("!replacing note %d in instrument %d\n",pitch1,inst1);
    scale[inst1][pitch1] = scale[inst2][pitch2];
    }


/*************** PRIVATE MUTATORS ********************/

/* function growOutput
 *
 * a wrapper for growRRA that ensures the newly added capacit
 * is zeroed out
 *
 */

static void
growOutput()
    {
    int i;
    int samples = output->samples;

    growRRA(output);

    for (i = samples; i < output->samples; ++i)
        output->data[0][i] = 0;
    }

/* function ensureFadeIn
 *
 * make sure a note begins with zeros
 *
 */

static void
ensureFadeIn(RRA *h)
    {
    int i;
    double factor = 0.00001;
    int multiplier = 3;

    /* quickly ramp amplitudes up from zero */
    /* a multiplier of 3 means a 10 sample fade in */

    for (i = 0; i < h->samples && factor < 0; ++i)
        {
        if (h->data[0][i] == 0) return;
        h->data[0][i] = h->data[0][i] * factor;
        factor *= multiplier;
        }
    }

/********* INTERPOLATION ROUTINES *********************************/

int
interp1(int *data,int length,double t)
    {
    int lo = (int) t;
    int hi = lo + 1;
    double fraction;
    double result;

    if (lo >= length-1) return data[length-1];

    fraction = t - lo;

    result = data[lo] + fraction * (data[hi] - data[lo]);

    return (int) result;
    }

int
interp3(int *data,int length,double t)
    {
    int x;
    double mu,mu2;
    int y0,y1,y2,y3;
    int a0,a1,a2,a3;

    if (t >= length) return data[length - 1];

    x = (int) t;
    mu = t - x;
    mu2 = mu * mu;

    y1 = data[x];
    if (x == 0)
        {
        y2 = data[x + 1];
        y3 = data[x + 2];
        y0 = y1 + y1 - y2;
        }
    else if (x == length - 2)
        {
        y0 = data[x - 1];
        y2 = data[x + 1];
        y3 = y2 + y2 - y1;
        }
    else if (x == length - 1)
        {
        y0 = data[x - 1];
        y2 = y1 + y1 - y0;
        y3 = y2 + y2 - y1;
        }
    else
        {
        y0 = data[x - 1];
        y2 = data[x + 1];
        y3 = data[x + 2];
        }
        
    a0 = y3 - y2 - y0 + y1;
    a1 = y0 - y1 - a0;
    a2 = y2 - y0;
    a3 = y1;

    return (int) (a0*mu*mu2+a1*mu2+a2*mu+a3);
    }

int
interp2(int *data,int length,double t)
    {
    int x;
    double mu;
    double mu2;

    if (t >= length) return data[length - 1];

    x = (int) t;
    mu = t - x;
    mu2 = (1-cos(mu*3.14159))/2;
    return(data[x]*(1-mu2)+data[x+1]*mu2);
    }

static SAMPLE_SET *
resampleSet(SAMPLE_SET *ss,double factor)
    {
    int i;
    SAMPLE_SET *newss = newSampleSet();
    newss->count = ss->count;
    newss->samples = (RRA **) New(sizeof(RRA*) * ss->count);

    /* transfer over the samples */

    for (i = 0; i < ss->count; ++i)
        newss->samples[i] = resample(ss->samples[i], factor);

    return newss;
    }

RRA *
resample(RRA *note,double factor)
    {
    int i;
    double t;
    RRA *h;

    //printf("in resample...\n");

    h = newRRA(note->sampleRate,note->bitsPerSample,1,note->samples);

    dDebug("dresample: samples in original note is %d\n",length);
    dDebug("dresample: factor is %f\n",factor);
    //printf("samples in original note is %d\n",note->samples);
    //printf("factor is %f\n",factor);

    i = 0;
    t = 0;
    while (t < note->samples)
        {
        if (i >= h->samples)
            {
            //printf("%d samples\n",h->samples);
            //printf("t is %f\n",t);
            growRRA(h);
            }

        h->data[0][i] = interp(note->data[0],note->samples,t);
        t += factor;
        ++i;
        }

    /* remove the excess capacity in the rra object */

    shrinkRRA(h,i);

    dDebug("leaving\n");

    return h;
    }
    
/************** CONVERSION ROUTINES *************************/

double
beatsToSamples(double beats)
    {
    double s = beats * 60 / beatsPerMinute * SampleRate;
    return s;
    }

double
samplesToBeats(double samples)
    {
    return samples
           / (double) SampleRate
           / 60.0
           * beatsPerMinute;
    }

double
secondsToSamples(double t)
    {
    return t * SampleRate;
    }

double
millisecondsToSamples(double t)
    {
    return (t / 1000.0) * SampleRate;
    }

double
samplesToSeconds(double samples)
    {
    return ((double) samples) / SampleRate;
    }

double
samplesToMeasures(double samples)
    {
    return samplesToBeats(samples)/ beatsPerMeasure;
    }

double
measuresToSamples(double measures)
    {
    return beatsToSamples(measures * beatsPerMeasure);
    }

double
measuresToBeats(double measures)
    {
    return measures * beatsPerMeasure;
    }

double
beatsToMeasures(double beats)
    {
    return beats / beatsPerMeasure;
    }

double
secondsToBeats(double seconds)
    {
    return seconds * beatsPerMinute / 60;
    }

double
beatsToSeconds(double beats)
    {
    return beats * 60 / beatsPerMinute;
    }

void
startCounting()
    {
    sampleCounter = location;
    }

double
stopCounting()
    {
    return samplesToBeats(location - sampleCounter);
    }

void
measure(const char *file,const char *function,const int line)
    {
    static int last = 0;

    if (file == 0)
        {
        last = location;
        //printf("location is %d\n",last);
        //printf("last is %f\n",samplesToBeats(last));
        //getchar();
        }
    else
        {
        int span = location - last;
        //printf("span is %d\n",span);
        printf("!%s: %s: %4d: %f measures (%f total)\n",
                file,function,line,samplesToMeasures(span),
                samplesToMeasures(location - spaces - measuredLocation)
		    + previousMeasures);
        }
    }

/************** MOVEMENT ROUTINES ********************************/

double
backwards(double beats)
    {
    return forwards(-beats);
    }

double
moveTo(double minutes,double seconds)
    {
    return samplesToSeconds(setLocation(secondsToSamples(minutes*60+seconds)));
    }

int
setLocation(int future)
    {
    int old = location;

    location = future;

    if (location > maxLocation)
        {
        while (location >= output->samples)
            growOutput();
        maxLocation = location;
        }

    if (location < 0)
        {
        //printf("attempt to rewind to a negative index, "
        //       "setting index to zero\n");
        location = 0;
        }

    return old;
    }

int
getLocation()
    {
    return location;
    }

double
forwards(double beats)
    {
    int old = location;

    setLocation(location + beatsToSamples(beats));

    return samplesToBeats(old);
    }


/*
 * The following set of functions
 *
 * is used to incorporate Drum comments
 *
 * in the RRA programmable file
 *
 * Warning: Barely tested
 *
 */
void
addComment(char *drumComment)
    {
    COMMENT *temp, *temp1;
    int commentSamples = getLocation();
    commentBuffer = (COMMENT *)New(sizeof(COMMENT));

    if(tail == NULL)
        {
        head = commentBuffer;
        tail = commentBuffer;
        commentBuffer->location = commentSamples;
        strcpy(commentBuffer->comment,drumComment);
        return;
        }

    if(commentSamples > tail->location) 
	{
	tail->next = commentBuffer;
	commentBuffer->location = commentSamples;
	strcpy(commentBuffer->comment,drumComment);
	//fprintf(stderr,"%s at %d \n",commentBuffer->comment, commentBuffer->location);
	commentBuffer->next = NULL;
	tail = commentBuffer;
	}
    else /* insert in between */
	{
	/* check from head to current
	 * tail, the most suitable
	 * location to place the comment */

	temp1 = (COMMENT *)New(sizeof(COMMENT));
	temp1->location = 0;

	temp = head;
	while(temp != tail)
	    {
	    if(temp->location > commentSamples)
		{
		temp1->next = temp->next;		
		temp->next = commentBuffer;
		commentBuffer->location = commentSamples;
		strcpy(commentBuffer->comment,drumComment);
		fprintf(stderr,"%s at %f \n",commentBuffer->comment, commentBuffer->location);
		commentBuffer->next = temp1->next;
		break;
		}
	    temp = temp->next;	    
	    }
	}
    }

void 
closeComments()    
    {
    COMMENT *temp;
    temp = head;

    while(temp)
	{
	head = temp->next;
	free(temp);
	temp = head;
	}
    }

/* 
 * Returns location to put
 * comment to the RRA file
 *
 */
int 
readComment()
    {
    static COMMENT *temp ;
    int samples;

    temp = head;

    if(temp)
	{
	samples = temp->location;
	temp = temp->next;
	fprintf(stderr,"samples:: %d \n",samples);
	}
    else
	{
	samples = 0;
	}
	
    return samples;
    }


/* function closeOutput
 *
 * this function writes out the sound data that has been accumulated
 * in the output object
 *
 * REQUIRED
 *
 */

void 
closeOutput()
    {
    FILE *fp;
    int samples,i,commentLocation,offset = 0;
    COMMENT *temp;

    if (closed == 1) return;
    
    if(commentBuffer != NULL)
        temp = head;

    if (maxLocation == 0)
        fprintf(stderr,"RRA file is empty\n");

    shrinkRRA(output,maxLocation);

    if (closed == 2)
        Fatal("new output RRA file has not been opened\n");

    if (outputName == 0)
        fp = stdout;
    else
        fp = OpenFile(outputName,"w");

    output->skip = (int) skip;

    //writeRRA(fp,output,"createdBy: songlib",0);

    samples = output->samples;

    if(output->samples < 0) output->samples = 0;

    writeRRAHeader(fp,output,"createdBy: songlib",0);

    if(commentBuffer != NULL)
        commentLocation = temp->location; 
    //fprintf(stderr,"commentLocation = %d\n",commentLocation);

    offset = (bitsPerSample == 8 ? 127:0);

    for(i = 0; i < samples; ++i)
        {
        while(commentLocation && (i == commentLocation) && commentBuffer != NULL)
            {
            fprintf(fp,"%s\n",temp->comment);
            temp = temp->next;
            //fprintf(stderr,"commentLocation = %d\n",commentLocation);
            if(temp)
                commentLocation = temp->location;
            else
                break;
            }
        fprintf(fp,"%d\n",output->data[0][i] + offset);
        }

    fclose(fp);

    freeRRA(output,0);
    
    if (commentBuffer != NULL) closeComments();

    samplesWritten = maxLocation;

    closed = 1;

    //return 0;
    }
    
/* function closeOutputSelection
 *
 * this function writes out the sound data that has been accumulated
 * in the output object for the given selection. Start and duration
 * are given in samples.
 *
 * SUBSTITUTIONAL (in place of closeOutput)
 *
 */

void 
closeOutputSelection(double start,double duration)
    {
    FILE *fp;
    int i,commentLocation,offset = 0;
    COMMENT *temp;
    
    if (closed == 1) return;

    if(commentBuffer != NULL)
        temp = head;

    if (maxLocation == 0)
        fprintf(stderr,"RRA file is empty\n");

    shrinkRRA(output,maxLocation);

    if (closed == 2)
        Fatal("new output RRA file has not been opened\n");

    if (outputName == 0)
        fp = stdout;
    else
        fp = OpenFile(outputName,"w");

    output->skip = 0;

    //writeRRA(fp,output,"createdBy: songlib",0);

    double samples = output->samples;
    output->samples = duration;
    writeRRAHeader(fp,output,"createdBy: songlib",0);
    output->samples = samples;

    if(commentBuffer != NULL)
        commentLocation = temp->location; 
    //fprintf(stderr,"commentLocation = %d\n",commentLocation);

    offset = (bitsPerSample == 8 ? 127:0);

    for(i = (int) start; i < (int) (start + duration) && i < output->samples; ++i)
        {
        while(commentLocation && (i == commentLocation) && commentBuffer != NULL)
            {
            fprintf(fp,"%s\n",temp->comment);
            temp = temp->next;
            //fprintf(stderr,"commentLocation = %d\n",commentLocation);
            if(temp)
                commentLocation = temp->location;
            else
                break;
            }
        fprintf(fp,"%d\n",output->data[0][i] + offset);
        }

    fclose(fp);

    freeRRA(output,0);
    
    if (commentBuffer != NULL) closeComments();

    samplesWritten = maxLocation;

    closed = 1;

    //return 0;
    }
