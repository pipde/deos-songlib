#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "songlib.h"
#include "key.h"

static int key = C;

static int mode = IONIAN;

static int lastVoicing = -1;

#define MODE_SIZE 7
#define MODE_COUNT 10
static const int modes[MODE_COUNT][MODE_SIZE] =
    {
    //  W  W  H  W  W  W  H
    { 0, 2, 4, 5, 7, 9, 11,}, // IONIAN, MAJOR

    //  W  H  W  W  W  H  W
    { 0, 2, 3, 5, 7, 9, 10,}, // DORIAN_MODE

    //  H  W  W  W  H  W  W
    { 0, 1, 3, 5, 7, 8, 10,}, // PHRYGIAN_MODE

    //  W  W  W  H  W  W  H
    { 0, 2, 4, 6, 7, 9, 11,}, // LYDIAN_MODE

    //  W  W  H  W  W  H  W
    { 0, 2, 4, 5, 7, 9, 10,}, // MIXOLYDIAN_MODE

    // W  H  W  W  H  W   W
    { 0, 2, 3, 5, 7, 8, 10,}, // AEOLIAN, MINOR, NATURAL_MINOR

    //  H  W  W  H  W  W  W
    { 0, 1, 3, 5, 6, 8, 10,}, // LOCRIAN_MODE

    //  W  H  W  W  W  W  H
    { 0, 2, 3, 5, 7, 9, 11,}, // MELODIC_MINOR

    //  W  H  W  W  H  WH  H
    { 0, 2, 3, 5, 7, 8, 11,}, // HARMONIC_MINOR

    //  W  H  H  W  H  H  W
    { 0, 2, 4, 6, 8, 10, 12,}, // STRANGE
    };

#define MAX_PATTERNS 32
char *patterns[MAX_PATTERNS];
double patternAmplitudes[MAX_PATTERNS]
    = {1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,
       1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,
       1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,
       1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0 };

int
setKey(int pitch)
    {
    int old = key;
    key = (pitch + 12) % 12;
    return old;
    }

int
getKey()
    {
    return key;
    }

int
degree(int note)
    {
    //assert(note >= 1 && note <= MODE_SIZE);

    return getKey()+modes[mode][note-1];
    }

int
setMode(int m)
    {
    int old;
    
    //assert(m >= 0 && m < MODE_COUNT);

    old = mode;
    mode = m;
    return old;
    }

int
getMode()
    {
    return mode;
    }

/*
 * getInterval returns the number of semitones
 * representing the interval between base and offset
 *
 * one based counting for base and offset, so an offset
 * of one is simply the base note.
 *
 */

int
getInterval(int base,int delta)
    {
    int spot1,spot2,spot1Shift,spot2Shift;

    //printf("getInterval: base is %d, delta is %d\n",base,delta);

    spot1 = base;
    spot1Shift = 0;
    spot2 = base + delta;
    spot2Shift = 0;

    while (spot1 > 7)
        {
        spot1 -= 7;
        spot1Shift += 12;
        }
    while (spot1 < 1)
        {
        spot1 += 7;
        spot1Shift -= 12;
        }
    while (spot2 > 7)
        {
        spot2 -= 7;
        spot2Shift += 12;
        }
    while (spot2 < 1)
        {
        spot2 += 7;
        spot2Shift -= 12;
        }

    /* subtract off one to get to zero-based counting */

    spot1 -= 1;
    spot2 -= 1;

    //printf("spot1 now is %d\n",spot1);
    //printf("spot2 now is %d\n",spot2);
    //printf("modes[mode][spot1] = %d\n",modes[mode][spot1]);
    //printf("modes[mode][spot2] = %d\n",modes[mode][spot2]);

    return modes[mode][spot2] + spot2Shift - (modes[mode][spot1] + spot1Shift);
    }

static int
tweakTranslate(int chordnum,int note,char x)
    {
    switch (x)
        {
        case 'x': return 0;
        case 'y': return 0;
        case 'X': return 12;
        case 'Y': return -12;
        case 's': return 1;
        case 'S': return 2;
        case 'u': return 3;
        case 'U': return 4;
        case 'b': return -1;
        case 'B': return -2;
        case 'd': return -3;
        case 'D': return -4;
        case 'n': return getInterval(chordnum+note,1);
        case 'N': return getInterval(chordnum+note,1) + 12;
        case 'm': return getInterval(chordnum+note,2);
        case 'M': return getInterval(chordnum+note,2) + 12;
        case 'l': return getInterval(chordnum+note,3);
        case 'L': return getInterval(chordnum+note,3) + 12;
        case 'p': return getInterval(chordnum+note,-1);
        case 'P': return getInterval(chordnum+note,-1) - 12;
        case 'q': return getInterval(chordnum+note,-2);
        case 'Q': return getInterval(chordnum+note,-2) - 12;
        case 'r': return getInterval(chordnum+note,-3);
        case 'R': return getInterval(chordnum+note,-3) - 12;
        default:
            Fatal("could not translate tweak %c, "
                "should be one of \"xXyYnNmMlLpPqQrRsSuUbBdD\"\n",
                x);
        return 0;
        }
    }

void
cchord(int chordNumber,double beats,int inst,int octave,char *tweaks)
    {
    int i;
    int additionals[7];
    int offset;
    int actual = 0;
    int count = strlen(tweaks);

    //printf("pitch count is %d\n",count);
    //printf("key is %d\n",getKey());
    //printf("chordNumber is %d\n",chordNumber);

    /* compute the number of semitones to get to the root node of */
    /* of the given chordNumber */

    offset = getInterval(1,chordNumber-1);

    //printf("offset is %d\n",offset);

    actual = 0;

    //printf("cchord: looking at tweaks %s\n",tweaks);
    for (i = 0; i < count && i < sizeof(additionals)/sizeof(int); ++i)
        {
        //printf("looking at tweak %d (%c)\n",i,tweaks[i]);
        if (tweaks[i] != '-')
            {
            int interval = getInterval(chordNumber,2*i);
            //printf("the untweaked note is %d semitones higher\n",interval);
            int tweak = tweakTranslate(chordNumber,2*i,tweaks[i]);
            //printf("it will be tweaked %d semitones\n",tweak);
            additionals[actual++] = interval+tweak;
            //printf("    additionals[%d] was %d\n",actual-1,interval);
            //printf("    additionals[%d] is %d\n",actual-1,additionals[actual-1]);
            }
        }

    //rpintf("total number of notes selected: %d\n",actual);
    if (actual > 0)
        achord(beats,inst,octave,getKey()+offset,additionals,actual);
    else
        {
        fprintf(stderr,"!empty note tab, resting %f beats!\n",beats);
        rest(beats);
        }
    }

int
getcnote(int chordNumber,int index)
    {
    //return getKey() + getInterval(1,chordNumber+index);
    //printf("getKey returns %d\n",getKey());
    //printf("requested chord is %d\n",chordNumber);
    //printf("requested chord starts %d semitones higher\n",getInterval(1,chordNumber-1));
    return getKey() + getInterval(1,chordNumber-1) + getInterval(chordNumber,index-1);
    }

static void
cadjust(int chordNumber,double beats,int inst,int octave,int shift,
    void (*f)(int,double,int,int))
    {
    int oldKey;
    int newKey;

    newKey = getKey() + shift;
    if (newKey > 11)
        {
        newKey -= 12;
        octave += 1;
        }
    else if (newKey < 0)
        {
        newKey += 12;
        octave -= 1;
        }
    oldKey = setKey(newKey);
    f(chordNumber,beats,inst,octave);
    setKey(oldKey);
    }

static void
nadjust(int chordNumber,double beats,int inst,int octave,int shift,int index)
    {
    int oldKey;
    int newKey;

    newKey = degree(chordNumber) + shift;
    if (newKey > 11)
        {
        newKey -= 12;
        octave += 1;
        }
    else if (newKey < 0)
        {
        newKey += 12;
        octave -= 1;
        }
    oldKey = setKey(newKey);
    n(chordNumber,beats,inst,octave,index);
    setKey(oldKey);
    }

void
c(int chordNumber,double beats,int inst,int octave)
    {
    cchord(chordNumber,beats,inst,octave,"xxx");

    lastVoicing = chordNumber;
    }

void
ic(int chordNumber,int inverted,double beats,int inst,int octave)
    {
    int i;
    char pitches[] = "xxx";
    for (i = 0; i < inverted && i < 3; ++i)
        pitches[i] = 'X';
    cchord(chordNumber,beats,inst,octave,pitches);
    }

void
cM(int chordNumber,double beats,int inst,int octave)
    {
    maj(beats,inst,octave,degree(chordNumber));
    }

void
cm(int chordNumber,double beats,int inst,int octave)
    {
    min(beats,inst,octave,degree(chordNumber));
    }

void
cd(int chordNumber,double beats,int inst,int octave)
    {
    dim(beats,inst,octave,degree(chordNumber));
    }

void
ca(int chordNumber,double beats,int inst,int octave)
    {
    aug(beats,inst,octave,degree(chordNumber));
    }

void
cf(int chordNumber,double beats,int inst,int octave)
    {
    cadjust(chordNumber,beats,inst,octave,-1,c);
    }

void
cs(int chordNumber,double beats,int inst,int octave)
    {
    cadjust(chordNumber,beats,inst,octave,1,c);
    }

void
cpower(int chordNumber,double beats,int inst,int octave)
    {
    cchord(chordNumber,beats,inst,octave,"x-x");
    backwards(beats);
    n(chordNumber,beats,inst,octave+1,0);
    }

void
cpower2(int chordNumber,double beats,int inst,int octave)
    {
    cchord(chordNumber,beats,inst,octave,"xxx");
    backwards(beats);
    n(chordNumber,beats,inst,octave+1,0);
    }

void
csus2(int chordNumber,double beats,int inst,int octave)
    {
    chord(beats,inst,octave,degree(chordNumber),+2,+7,(int) 0);
    }

void
csus4(int chordNumber,double beats,int inst,int octave)
    {
    chord(beats,inst,octave,degree(chordNumber),+5,+7,(int) 0);
    }

void
cquart(int chordNumber,double beats,int inst,int octave)
    {
    chord(beats,inst,octave,degree(chordNumber),+5,+10,(int) 0);
    }

void
c7(int chordNumber,double beats,int inst,int octave)
    {
    cchord(chordNumber,beats,inst,octave,"xxxx");
    }

void
cM7(int chordNumber,double beats,int inst,int octave)
    {
    chord(beats,inst,octave,degree(chordNumber),+4,+7,+11,IX);
    }

void
cm7(int chordNumber,double beats,int inst,int octave)
    {
    cchord(chordNumber,beats,inst,octave,"xbxb");
    }

void
cd7(int chordNumber,double beats,int inst,int octave)
    {
    chord(beats,inst,octave,degree(chordNumber),+3,+6,+9,(int) 0);
    cchord(chordNumber,beats,inst,octave,"xbbB");
    }

void
ca7(int chordNumber,double beats,int inst,int octave)
    {
    chord(beats,inst,octave,degree(chordNumber),+4,+8,+10,(int) 0);
    }

void
cD7(int chordNumber,double beats,int inst,int octave)
    {
    chord(beats,inst,octave,degree(chordNumber),+4,+7,+10,(int) 0);
    }

void
ic7(int chordNumber,int inverted,double beats,int inst,int octave)
    {
    int i;
    char pitches[] = "xxxx";
    for (i = 0; i < inverted; ++i)
        pitches[i] = 'X';
    cchord(chordNumber,beats,inst,octave,pitches);
    }

void
icD7(int chordNumber,int inverted,double beats,int inst,int octave)
    {
    switch (inverted)
        {
        case 1:
            chord(beats,inst,octave+1,degree(chordNumber),-8,-5,-2,IX);
            break;
        case 2:
            chord(beats,inst,octave+1,degree(chordNumber),4,-5,-2,IX);
            break;
        default:
            chord(beats,inst,octave,degree(chordNumber),+4,+7,+10,(int) 0);
            break;
        }
    }

void
c7f(int chordNumber,double beats,int inst,int octave)
    {
    cadjust(chordNumber,beats,inst,octave,-1,c7);
    }

void
c7s(int chordNumber,double beats,int inst,int octave)
    {
    cadjust(chordNumber,beats,inst,octave,1,c7);
    }

void
c9(int chordNumber,double beats,int inst,int octave)
    {
    cchord(chordNumber,beats,inst,octave,"xx-xx");
    }

void
c9f(int chordNumber,double beats,int inst,int octave)
    {
    cadjust(chordNumber,beats,inst,octave,-1,c9);
    }

void
c9s(int chordNumber,double beats,int inst,int octave)
    {
    cadjust(chordNumber,beats,inst,octave,1,c9);
    }

void
c11(int chordNumber,double beats,int inst,int octave)
    {
    cchord(chordNumber,beats,inst,octave,"xx-x-x");
    }

void
c11f(int chordNumber,double beats,int inst,int octave)
    {
    cadjust(chordNumber,beats,inst,octave,-1,c11);
    }

void
c11s(int chordNumber,double beats,int inst,int octave)
    {
    cadjust(chordNumber,beats,inst,octave,1,c11);
    }

void
c13(int chordNumber,double beats,int inst,int octave)
    {
    cchord(chordNumber,beats,inst,octave,"xx-x--x");
    }

void
c13f(int chordNumber,double beats,int inst,int octave)
    {
    cadjust(chordNumber,beats,inst,octave,-1,c13);
    }

void
c13s(int chordNumber,double beats,int inst,int octave)
    {
    cadjust(chordNumber,beats,inst,octave,1,c13);
    }

void
setBrokenAmplitudes(double first,...)
    {
    int count;
    double amp;
    va_list ap;

    patternAmplitudes[0] = first;
    count = 1;
    va_start(ap, first);
    amp = va_arg(ap,double);
    while (amp != DX)
        {
        if (count >= MAX_PATTERNS)
            Fatal("b: only %d amplitudes allowed (missing null terminator?)\n",
               MAX_PATTERNS);
        patternAmplitudes[count++] = amp;
        amp = va_arg(ap,double);
        }
    }

void
b(int chordNumber,double beats,int inst,int octave,char *first,...)
    {
    char *pattern;
    int i,count;
    va_list ap;

    patterns[0] = first;
    count = 1;
    //printf("!pattern is %s\n",first);

    /* extract the arguments */

    //printf("b...\n");
    va_start(ap, first);
    pattern = va_arg(ap,char *);
    while (pattern != SX)
        {
        //printf("!pattern is %s\n",pattern);
        if (count >= MAX_PATTERNS)
            Fatal("b: only %d patterns allowed (missing null terminator?)\n",
                MAX_PATTERNS);
        patterns[count++] = pattern;
        pattern = va_arg(ap,char *);
        }
    
    //printf("%d patterns found\n",count);

    if (count == 0)
        {
        rest(beats);
        return;
        }

    double old = getAmplitude();
    for (i = 0; i < count; ++i)
        {
        setAmplitude(old * patternAmplitudes[i]);
        cchord(chordNumber,beats/count,inst,octave,patterns[i]);
        }
    setAmplitude(old);
    }

void
n(int chordNumber,double beats,int inst,int octave,int index)
    {
    play(beats,inst,octave,getcnote(chordNumber,index));
    }

void
nf(int chordNumber,double beats,int inst,int octave,int index)
    {
    nadjust(chordNumber,beats,inst,octave,-1,index);
    }

void
ns(int chordNumber,double beats,int inst,int octave,int index)
    {
    nadjust(chordNumber,beats,inst,octave,+1,index);
    }

void
maj(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,4,7,(int) 0);
    }

void
min(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,3,7,(int) 0);
    }

void
aug(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,4,8,(int) 0);
    }

void
dim(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,3,6,(int) 0);
    }

void
sus4(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,5,7,(int) 0);
    }

void
sus2(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,2,7,(int) 0);
    }

void
sus7(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,5,7,10,(int) 0);
    }

void
sus9(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,5,7,10,14,(int) 0);
    }

void
sus11(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,5,7,10,14,17,(int) 0);
    }

void
sus13(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,5,7,10,14,17,21,(int) 0);
    }


void
maj6(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,4,7,9,(int) 0);
    }

void
min6(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,3,7,9,(int) 0);
    }

void
hdim7(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,3,6,10,(int) 0);
    }

void
dim7(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,3,6,9,(int) 0);
    }

void
aug7(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,4,8,11,(int) 0);
    }

void
maj9(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,4,7,11,14,(int) 0);
    }

void
min9(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,3,7,10,14,(int) 0);
    }

void
dom9(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,4,7,10,14,(int) 0);
    }

void
aug9(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,4,8,11,14,(int) 0);
    }

void
dim9(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,3,6,9,14,(int) 0);
    }

void
maj11(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,4,7,11,14,17,(int) 0);
    }

void
min11(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,3,7,10,14,17,(int) 0);
    }

void
dom11(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,4,7,10,14,17,(int) 0);
    }

void
aug11(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,4,8,11,14,17,(int) 0);
    }

void
dim11(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,3,6,9,14,17,(int) 0);
    }

void
maj13(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,4,7,11,14,17,21,(int) 0);
    }

void
dom13(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,4,7,10,14,17,21,(int) 0);
    }

void
min13(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,3,7,10,14,17,21,(int) 0);
    }

void
aug13(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,4,8,11,14,17,21,(int) 0);
    }

void
dim13(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,3,6,9,14,17,21,(int) 0);
    }

void
i3(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,3,(int) 0);
    }

void
i3p(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,3,12,(int) 0);
    }

void
i4(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,4,(int) 0);
    }

void
i4p(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,4,12,(int) 0);
    }

void
i5(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,5,(int) 0);
    }

void
i5p(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,5,12,(int) 0);
    }

void
i6(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,6,(int) 0);
    }

void
i6p(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,6,12,(int) 0);
    }

void
i7(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,7,(int) 0);
    }

void
i7p(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,7,12,(int) 0);
    }

void
i8(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,8,(int) 0);
    }

void
i8p(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,8,12,(int) 0);
    }

void
i9(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,9,(int) 0);
    }

void
i9p(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,9,12,(int) 0);
    }

void
i37(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,3,7,(int) 0);
    }

void
i37p(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,3,7,12,(int) 0);
    }

void
i38(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,3,8,(int) 0);
    }

void
i38p(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,3,8,12,(int) 0);
    }

void
i47(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,4,7,(int) 0);
    }

void
i47p(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,4,7,12,(int) 0);
    }

void
i49(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,4,9,(int) 0);
    }

void
i49p(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,4,9,12,(int) 0);
    }

void
i57(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,5,7,(int) 0);
    }

void
i57p(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,5,7,12,(int) 0);
    }

void
i59(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,5,9,(int) 0);
    }

void
i59p(double beats,int instrument,int octave,int pitch)
    {
    chord(beats,instrument,octave,pitch,5,9,12,(int) 0);
    }
