/*
 * splitter
 *
 * split a wave file into equal portions
 *
 * written by John C. Lusth, 2006
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/wave.h"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "splitter";
char *PROGRAM_VERSION = "0.01";

char *notes[] = 
    {
    "c-1", "c#-1", "d-1", "d#-1", "e-1", "f-1", "f#-1",
        "g-1", "g#-1", "a-1", "a#-1", "b-1",
    "c0", "c#0", "d0", "d#0", "e0", "f0", "f#0", "g0", "g#0", "a0", "a#0", "b0",
    "c1", "c#1", "d1", "d#1", "e1", "f1", "f#1", "g1", "g#1", "a1", "a#1", "b1",
    "c2", "c#2", "d2", "d#2", "e2", "f2", "f#2", "g2", "g#2", "a2", "a#2", "b2",
    "c3", "c#3", "d3", "d#3", "e3", "f3", "f#3", "g3", "g#3", "a3", "a#3", "b3",
    "c4", "c#4", "d4", "d#4", "e4", "f4", "f#4", "g4", "g#4", "a4", "a#4", "b4",
    "c5", "c#5", "d5", "d#5", "e5", "f5", "f#5", "g5", "g#5", "a5", "a#5", "b5",
    "c6", "c#6", "d6", "d#6", "e6", "f6", "f#6", "g6", "g#6", "a6", "a#6", "b6",
    "c7", "c#7", "d7", "d#7", "e7", "f7", "f#7", "g7", "g#7", "a7", "a#7", "b7",
    "c8", "c#8", "d8", "d#8", "e8", "f8", "f#8", "g8", "g#8", "a8", "a#8", "b8",
    };

int noteCount;

int
main(int argc,char **argv)
    {
    WAVE *w,*n;
    int width;
    int base;
    int noteCount;
    int delta;
    char *prefix,*baseNote;
    int notesExtracted;
    int offset;
    double duration;
    char buffer[512];

    if (argc < 4 || argc > 6)
        Fatal("usage: splitter <wav file> <note duration> " 
                "<base note number> [<note prefix> [<semitone delta>]]\n");

    noteCount = sizeof(notes) / sizeof(char *);
    printf("note count is %d\n",noteCount);
    w = readWaveMono(argv[1]);
    printf("total samples is %d\n",w->sampleCount);

    duration = atof(argv[2]);
    width = (int) (duration * w->SampleRate);
    base = atoi(argv[3]);
    if (base < 0 || base >= noteCount)
        Fatal("base note %d is out of range\n", base);
    baseNote = notes[base];
    printf("base note is %s\n", notes[base]);
    printf("should extract %d notes\n", w->sampleCount / width);

    delta = 1;
    prefix = "note_";

    if (argc == 6)
        {
        delta = atoi(argv[5]);
        prefix = argv[4];
        }

    if (argc == 5)
        prefix = argv[4];

    notesExtracted = 0;
    offset = 0;
    while (offset < w->sampleCount)
        {
        int i;
        int count;
        n = cloneWave(w);
        n->channels[0] = newIntArray(width);
        count = 0;
        for (i = 0; i < width && i + offset < w->sampleCount; ++i)
            {
            n->channels[0][i] = w->channels[0][i+offset];
            ++count;
            }
        setWaveSize(n,count);
        printf("extracting %s%s.wav...\n",prefix,baseNote);
        sprintf(buffer,"%s%s.wav",prefix,baseNote);
        writeWave(n,buffer);
        free(n);
        ++notesExtracted;
        baseNote = notes[base+notesExtracted*delta];
        offset = (int) (notesExtracted * duration * w->SampleRate);
        }

    return 0;
    }
