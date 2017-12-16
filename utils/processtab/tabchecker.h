#ifndef INCLUDED_TABCHECKER_H
#define INCLUDED_TABCHECKER_H

#include <stdio.h>
#include "tabparser.h"

/******** Constants ********/

/******** Struct prototypes ********/

typedef struct composition Composition;
typedef struct part Part;
typedef struct measure Measure;
typedef struct note_instance NoteInstance;
typedef struct voice Voice;
typedef struct note_class NoteClass;
typedef struct section Section;

/******** Composition Structs ********/

struct composition
    {
    Part **parts;
    int partCount;
    Section **sections;
    int sectionCount;
    char **attributeNames;
    int attributeNameCount;
    };
    
struct part
    {
    Measure **measures;
    int measureCount;
    char *voiceName;
    Composition *composition;
    };
    
struct measure
    {
    NoteInstance **noteInstances;
    int noteInstanceCount;
    int beatsInMeasure;
    int beatLength;
    Part *part;
    };
    
struct note_instance
    {
    char noteName; /* rest = 0 (null) */
    double duration;
    //bool accent;
    };
    
struct section
    {
    ASTAttribute **attributes;
    int attributeCount;
    int measureNumber;
    Composition *composition;
    };
    

/******** Composition API ********/

extern Composition *newComposition();
extern Part *newPart();
extern Measure *newMeasure();
extern NoteInstance *newNoteInstance();
extern Voice *newVoice();
extern Section *newSection();

extern Composition *newCompositionFromTab(ASTTab *);

#endif	/* INCLUDED_TABCHECKER_H */
