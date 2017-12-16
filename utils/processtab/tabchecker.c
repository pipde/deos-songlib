#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "/usr/local/include/songlib/util.h"
#include "tabchecker.h"

/******** Constants ********/

/* composition duration is in whole notes, but songlib note lengths are in quarter notes */
#define SONGLIB_NOTELENGTH_MULTIPLIER 4

/******** Local typedefs ********/

typedef struct split_string
    {
    char *first;
    char *second;
    } SplitString;
    
typedef struct double_array
    {
    double *values;
    int length;
    } DoubleArray;
    
typedef struct string_array
    {
    char **values;
    int length;
    } StringArray;
    
typedef struct ast_attribute_list ASTAttributeList;

struct ast_attribute_list
    {
    ASTAttribute *attribute;
    ASTAttributeList *next;
    };

/******** Function prototypes ********/

static StringArray *newStringArray(int);
static DoubleArray *newDoubleArray(int);
static ASTAttributeList *newASTAttributeList();
static ASTAttributeList *addAttributeToList(ASTAttribute *, ASTAttributeList *);
static void buildMeasureFromASTEvents(Measure *, char *, char *);
static DoubleArray *parseMeasureEventDurations(char *, int);
static DoubleArray *parseQuarterNoteEventDurations(char *);
static DoubleArray *parseFirstEighthNoteEventDurations(char *);
static DoubleArray *parseSecondEighthNoteEventDurations(char *);
static DoubleArray *parseGenericTimingEvents(char *, double);
static StringArray *splitTimingEvents(char *, char *);
static char *substring(const char *, size_t, size_t);
static SplitString *split(char *, char);
static DoubleArray *concatDoubleArrays(DoubleArray *, DoubleArray *);
static StringArray *cons_strarr(char *, StringArray *);
static char *cons_str(char, char *);
static char *charRange(char, char);
static void addPartToComposition(Part *, Composition *);
static void addMeasureToPart(Measure *, Part *);
static void addNoteInstanceToMeasure(NoteInstance *, Measure *);
static void addSectionToComposition(Section *, Composition *);
static void addAttributeToSection(ASTAttribute *, Section *);
static void addAttributeNameToComposition(char *, Composition *);

/******** New functions ********/

Composition *
newComposition()
    {
    Composition *c = (Composition *) New(sizeof(Composition));
    
    c->parts = 0;
    c->partCount = 0;
    c->sections = 0;
    c->sectionCount = 0;
    
    return c;
    }

Part *
newPart()
    {
    Part *p = (Part *) New(sizeof(Part));
    
    p->measures = 0;
    p->measureCount = 0;
    p->voiceName = 0;
    p->composition = 0;

    return p;
    }
    
Measure *
newMeasure()
    {
    Measure *m = (Measure *) New(sizeof(Measure));
    
    m->noteInstances = 0;
    m->noteInstanceCount = 0;
    m->beatsInMeasure = 0;
    m->beatLength = 0;
    m->part = 0;
    
    return m;
    }
    
NoteInstance *
newNoteInstance()
    {
    NoteInstance *ni = (NoteInstance *) New(sizeof(NoteInstance));
    
    ni->noteName = 0;
    ni->duration = 0;
    
    return ni;
    }

Section *
newSection()
    {
    Section *s = (Section *) New(sizeof(Section));
    
    s->attributes = 0;
    s->attributeCount = 0;
    s->measureNumber = 0;
    s->composition = 0;

    return s;
    }
    
/******** Attribute list ********/

static ASTAttributeList *
newASTAttributeList()
    {
    ASTAttributeList *aal = (ASTAttributeList *) New(sizeof(ASTAttributeList));
    
    aal->attribute = 0;
    aal->next = 0;

    return aal;
    }
    
static ASTAttributeList *
addAttributeToList(ASTAttribute *attribute, ASTAttributeList *attributeList)
    {
    ASTAttributeList *head, *curr, *newAal = newASTAttributeList();    
    newAal->attribute = attribute;
    
    if (attributeList == 0) return newAal;
    
    head = curr = attributeList;
    while (curr != 0 && curr->next != 0)
        {
        curr = curr->next;
        }
    curr->next = newAal;
    return head;
    }
    
/******** Composition builder ********/

Composition *
newCompositionFromTab(ASTTab *astTab)
    {
    Composition *c = newComposition();
    Part *part;
    Measure *measure;
    Section *section = 0;
    int i, j, k, partIndex, measureCount = 0;
    ASTCommand *astCommand;
    ASTStave *astStave;
    ASTCourse *astCourse;
    char *astEvents, *astTimingEvents;
    ASTAttributeList *attributeList = 0, *originalList = 0;
    bool attributeAdded;
        
    /* First pass checks all the stave courses and creates parts */
    
    for (i = 0; i < astTab->commandCount; i++)
        {
        astCommand = astTab->commands[i];
        if (astCommand->commandType == StaveCommand)
            {
            astStave = astCommand->un.stave;
            for (j = 0; j < astStave->courseCount; j++)
                {
                astCourse = astStave->courses[j];
                part = 0;
                /* Add measure to each part */
                for (partIndex = 0; partIndex < c->partCount; partIndex++)
                    {                
                    if (strcmp(astCourse->name, c->parts[partIndex]->voiceName) == 0) 
                        {
                        part = c->parts[partIndex];
                        break;
                        }
                    }
                    
                if (part == 0)
                    {
                    /* Set up part */
                    part = newPart();
                    part->voiceName = StringDup(astCourse->name);
                    addPartToComposition(part, c);
                    }
                }
            }
        }
    
    /* Second pass reads attributes and staves and populates sections, measures, and note instances */
    
    for (i = 0; i < astTab->commandCount; i++)
        {
        astCommand = astTab->commands[i];
        /* attribute command */
        if (astCommand->commandType == AttributeCommand) 
            {
            attributeList = addAttributeToList(astCommand->un.attribute, attributeList);
                        
            /* add attribute name to composition if it hasn't already been added */
            attributeAdded = false;
            for (j = 0; j < c->attributeNameCount; j++)
                {
                if (strcmp(astCommand->un.attribute->name, c->attributeNames[j]) == 0) attributeAdded = true;
                }
            if (!attributeAdded)
                {
                addAttributeNameToComposition(astCommand->un.attribute->name, c);
                }
           }
        /* stave command */
        else if (astCommand->commandType == StaveCommand)
            {
            astStave = astCommand->un.stave;
            
            /* Begin a section at each new stave */
            section = newSection();
            section->measureNumber = measureCount;
            addSectionToComposition(section, c);
            
            /* Add current attributes to section, removing them from the list */
            while (attributeList != 0)
                {
                addAttributeToSection(attributeList->attribute, section);
                originalList = attributeList;
                attributeList = attributeList->next;
                free(originalList); 
                originalList = 0;
                }
            
            /* Add measures to each part */
            for (partIndex = 0; partIndex < c->partCount; partIndex++)
                {                
                part = c->parts[partIndex];
                
                for (j = 0; j < astStave->timingCourse->measureCount; j++)
                    {
                    measure = newMeasure();
                    measure->beatsInMeasure = 4; /* TODO: replace with attribute */
                    measure->beatLength = 4;     /* TODO: replace with attribute */
                    addMeasureToPart(measure, part);
                    
                    astTimingEvents = astStave->timingCourse->measures[j]->events;
                    
                    /* find course in this stave that matches the part, then populate noteInstances */
                    for (k = 0; k < astStave->courseCount; k++)
                        {
                        astCourse = astStave->courses[k];
                        if (strcmp(astCourse->name, part->voiceName) != 0) continue;
                        
                        /* build note instances based on timing events */            
                        astEvents = astCourse->measures[j]->events;
                        buildMeasureFromASTEvents(measure, astEvents, astTimingEvents);
                        }
                    
                    /* course for this part not found; populate with rests */
                    if (measure->noteInstances == 0) buildMeasureFromASTEvents(measure, 0, astTimingEvents);
                    }
                }
            
            /* Advance measure count */
            measureCount += astStave->timingCourse->measureCount;
            }
        /* comment command */
        else if (astCommand->commandType == CommentCommand) 
            {
            /* skip */
            }
        /* unknown command */
        else
            {
            Fatal("AstCommand of type '%i' found; aborting\n", astCommand->commandType);
            return 0;
            }
        }
    
    return c;
    }
    
static DoubleArray *
newDoubleArray(int size)
    {
    DoubleArray *da = (DoubleArray *) New(sizeof(DoubleArray));
    int i;
    
    da->values = (double *) New(sizeof(double) * size);
    da->length = size;
    for (i=0; i<size; i++)
        {
        da->values[i] = 0;
        }
    return da;
    }
    
static StringArray *
newStringArray(int size)
    {
    StringArray *sa = (StringArray *) New(sizeof(StringArray));
    int i;
    
    sa->values = (char **) New(sizeof(char *) * size);
    sa->length = size;
    for (i=0; i<size; i++)
        {
        sa->values[i] = 0;
        }
    return sa;
    }
    
/******** Misc functions ********/
    
static void
buildMeasureFromASTEvents(Measure *measure, char *astEvents, char *astTimingEvents)
    {
    int i;
    DoubleArray *eventDurations = parseMeasureEventDurations(astTimingEvents, (int)measure->beatsInMeasure);
    NoteInstance *ni;
    double currentDuration;
    
    for (i = 0; i < eventDurations->length; i++)
        {
        ni = newNoteInstance();
        /* ni->noteName == 0 is rest */
        ni->noteName = astEvents == 0 ? 0 : ((astEvents[i] == '-' || astEvents[i] == ' ') ? 0 : astEvents[i]);
        
        /* spaces ' ' hold the note length.  Loop through them, adding to the current duration. */
        currentDuration = eventDurations->values[i]; 
        for (; i < (eventDurations->length - 1) && (astEvents == 0 || astEvents[i+1] == ' '); i++)
            {
            currentDuration += eventDurations->values[i+1];
            }
        
        ni->duration = currentDuration;
        addNoteInstanceToMeasure(ni, measure);
        }
    }
    
static DoubleArray *
parseMeasureEventDurations(char *measureTimingEvents, int measureLengthInBeats)
    {
//printf("[parseMeasureEventDurations]\n");
    StringArray *timingEventsByQuarterNote;
    DoubleArray *retVal = 0;
    int i;
    
//printf("[%s]\n", measureTimingEvents);
    
    /* attempt to split into quarter notes */
    timingEventsByQuarterNote = splitTimingEvents(measureTimingEvents, charRange('1', measureLengthInBeats + '0'));
    
    /* parse event durations inside first half of beat */
    for (i = 0; i < timingEventsByQuarterNote->length; i++)
        {
//printf("[parseMeasureEventDurations] before concat %i: %s\n", i, timingEventsByQuarterNote->values[i]);
        retVal = concatDoubleArrays(retVal, parseQuarterNoteEventDurations(timingEventsByQuarterNote->values[i]));
//printf("[parseMeasureEventDurations] after concat %i: %s\n", i, timingEventsByQuarterNote->values[i]);
        }    
        
    return retVal;
    }
    
static DoubleArray *
parseQuarterNoteEventDurations(char *quarterNoteTimingEvents)
    {
//printf("[parseQuarterNoteEventDurations]\n");
    StringArray *timingEventsByEighthNote;
    DoubleArray *retVal = 0;
    
//printf("\t[%s]\n", quarterNoteTimingEvents);
    
    /* attempt to split into eighth notes */
    timingEventsByEighthNote = splitTimingEvents(quarterNoteTimingEvents, cons_str(quarterNoteTimingEvents[0], "&"));

    /* unable to split; return */
    if (timingEventsByEighthNote == 0 || timingEventsByEighthNote->length != 2) 
        {
//printf("[parseQuarterNoteEventDurations] unable to split '%s' into eighth notes\n", quarterNoteTimingEvents);
        return parseGenericTimingEvents(quarterNoteTimingEvents, 1.0/4);
        }
        
        
//printf("[parseQuarterNoteEventDurations] first concat: %s\n", timingEventsByEighthNote->values[0]);
    retVal = concatDoubleArrays(retVal, parseFirstEighthNoteEventDurations(timingEventsByEighthNote->values[0]));
//printf("[parseQuarterNoteEventDurations] second concat: %s\n", timingEventsByEighthNote->values[1]);
    retVal = concatDoubleArrays(retVal, parseSecondEighthNoteEventDurations(timingEventsByEighthNote->values[1]));
//printf("[parseQuarterNoteEventDurations] after concat\n");
    
    return retVal;
    }
    
static DoubleArray *
parseFirstEighthNoteEventDurations(char *eighthNoteTimingEvents)
    {
//printf("[parseFirstEighthNoteEventDurations]\n");
    StringArray *timingEventsBySixteenthNote;
    DoubleArray *retVal = 0;
    
//printf("\t\t[%s]\n", eighthNoteTimingEvents);
    
    /* attempt to split into sixteenth notes */
    timingEventsBySixteenthNote = splitTimingEvents(eighthNoteTimingEvents, cons_str(eighthNoteTimingEvents[0], "e"));

    /* unable to split; return */
    if (timingEventsBySixteenthNote == 0 || timingEventsBySixteenthNote->length != 2) 
        {
//printf("[parseFirstEighthNoteEventDurations] unable to split '%s' into sixteenth notes\n", eighthNoteTimingEvents);
        return parseGenericTimingEvents(eighthNoteTimingEvents, 1.0/8);
        }
        
//printf("[parseFirstEighthNoteEventDurations] first concat: %s\n", timingEventsBySixteenthNote->values[0]);
    retVal = concatDoubleArrays(retVal, parseGenericTimingEvents(timingEventsBySixteenthNote->values[0], 1.0/16));
//printf("[parseFirstEighthNoteEventDurations] second concat: %s\n", timingEventsBySixteenthNote->values[1]);
    retVal = concatDoubleArrays(retVal, parseGenericTimingEvents(timingEventsBySixteenthNote->values[1], 1.0/16));
//printf("[parseFirstEighthNoteEventDurations] after concat\n");
    
    return retVal;
    }
    
static DoubleArray *
parseSecondEighthNoteEventDurations(char *eighthNoteTimingEvents)
    {
//printf("[parseSecondEighthNoteEventDurations]\n");
    StringArray *timingEventsBySixteenthNote;
    DoubleArray *retVal = 0;
    
//printf("\t\t[%s]\n", eighthNoteTimingEvents);
    
    /* attempt to split into sixteenth notes */
    timingEventsBySixteenthNote = splitTimingEvents(eighthNoteTimingEvents, "&a");

    /* unable to split; return */
    if (timingEventsBySixteenthNote == 0 || timingEventsBySixteenthNote->length != 2) 
        {
//printf("[parseSecondEighthNoteEventDurations] unable to split '%s' into sixteenth notes\n", eighthNoteTimingEvents);
        return parseGenericTimingEvents(eighthNoteTimingEvents, 1.0/8);
        }
        
//printf("[parseSecondEighthNoteEventDurations] first concat: %s\n", timingEventsBySixteenthNote->values[0]);
    retVal = concatDoubleArrays(retVal, parseGenericTimingEvents(timingEventsBySixteenthNote->values[0], 1.0/16));
//printf("[parseSecondEighthNoteEventDurations] second concat: %s\n", timingEventsBySixteenthNote->values[1]);
    retVal = concatDoubleArrays(retVal, parseGenericTimingEvents(timingEventsBySixteenthNote->values[1], 1.0/16));
//printf("[parseSecondEighthNoteEventDurations] after concat\n");
    
    return retVal;
    }
    
static DoubleArray *
parseGenericTimingEvents(char *timingEvents, double totalValue)
    {
//printf("[parseGenericTimingEvents]\n");
    int length = strlen(timingEvents);
    DoubleArray *retVal = newDoubleArray(length);
    int i;
    char ch;
    
    for (i = 0; i < length; i++)
        {
        ch = timingEvents[i];
        /* expect only spaces and dots after first event */
        if (ch != '.' && i != 0)
            {
            Fatal("Timing '.' expected; found '%c'\n", timingEvents[i]);
            return 0;
            }
        retVal->values[i] = totalValue / length;
//printf("[parseGenericTimingEvents] Converted '%c' into duration: %f\n", timingEvents[i], retVal->values[i]);
        }
//printf("[parseGenericTimingEvents] retVal length: %i\n", retVal->length);
    return retVal;
    }    
    
static StringArray *
splitTimingEvents(char *timingEvents, char *expectedEvents)
    {
//printf("[splitTimingEvents] 1 (timingEvents: '%s', expectedEvents '%s'\n", timingEvents, expectedEvents);
    SplitString *splitString;
    StringArray *sa;
    int expectedEventsCount = strlen(expectedEvents);
    
    /* if there are no expected events, error */
    if (expectedEventsCount == 0)
        {
        Fatal("No expected timing events were supplied\n");
        return 0;
        }
        
    /* the first event should be the same as the first expected event */
    if (timingEvents[0] != expectedEvents[0])
        {
        Fatal("Timing event '%c' expected; found '%c'\n", expectedEvents[0], timingEvents[0]);
        return 0;
        }
        
    /* if there is only one expected event, then timingEvents represents all of the event */
    if (expectedEventsCount == 1)
        {
        sa = newStringArray(1);
        sa->values[0] = StringDup(timingEvents);
        return sa;
        }
    
//printf("[splitTimingEvents] 2\n");
    /* otherwise, split */
    splitString = split(timingEvents, expectedEvents[1]);
    
//printf("[splitTimingEvents] 3\n");
    /* the expected event should be found, otherwise error */
    if (splitString->second == 0)
        {
        //Warning("Timing event '%c' not found\n", expectedEvents[1]);
        return 0;
        }
        
//printf("[splitTimingEvents] 4\n");
    /* otherwise recurse and concatenate the first */
    return cons_strarr(splitString->first, splitTimingEvents(splitString->second, substring(expectedEvents, 1, expectedEventsCount - 1)));
    }

static char *
substring(const char* str, size_t begin, size_t len)
    {
    int strLength;
    int realLength = len;
    
//printf("[substring] 1\n");
    if (str == 0)
        {
        return 0;
        }
        
    strLength = strlen(str);
//printf("[substring] 2\n");
        
    if (len == 0 || strLength == 0 || strLength < begin)
        {
        return StringDup("");
        }
        
//printf("[substring] 3\n");
    if (strLength < (begin+len))
        {
        realLength = strLength - begin;
        }

//printf("[substring] 4\n");
    return strndup(str + begin, realLength);
    } 
 
static SplitString *
split(char *str, char ch)
    {
    SplitString *ss = (SplitString *) New(sizeof(SplitString));
    size_t memloc = (size_t)strchr(str, ch);
    size_t loc;
    
    if (memloc == 0)
        {
        ss->first = StringDup(str);
        ss->second = 0;
        }
    else
        {
        loc = memloc - (size_t)str;
        ss->first = substring(str, 0, loc);
        ss->second = substring(str, loc, strlen(str) - loc);
        }
    return ss;
    }
    
static DoubleArray *
concatDoubleArrays(DoubleArray *first, DoubleArray *second)
    {
//printf("[concatDoubleArrays]\n");
    DoubleArray *newArray;
    int i, firstLength = 0, secondLength = 0;
    
    if (first == 0 && second == 0) return 0;
    
//printf("[concatDoubleArrays] first: {%zu}\n", (size_t)first);
//printf("[concatDoubleArrays] second: {%zu}\n", (size_t)second);
    
    if (first == 0) firstLength = 0;
    else firstLength = first->length;

    if (second == 0) secondLength = 0;
    else secondLength = second->length;
        
//printf("[concatDoubleArrays] firstLength: %i\n", firstLength);
//printf("[concatDoubleArrays] secondLength: %i\n", secondLength);
    
    newArray = newDoubleArray(firstLength + secondLength);
        
    for (i = 0; i < firstLength; i++)
        {
        newArray->values[i] = first->values[i];
        }
    for (i = 0; i < secondLength; i++)
        {
        newArray->values[firstLength + i] = second->values[i];
        }
//printf("[concatDoubleArrays] newArray: {%zu}\n", (size_t)newArray);
//printf("[concatDoubleArrays] newArray->length: %i\n", newArray->length);
    return newArray;
    }
    
static StringArray *
cons_strarr(char *car, StringArray *cdr)
    {
    int i;
    
//printf("[cons_strarr] 1\n");
//printf("[cons_strarr] cdr: {%zu}\n", (size_t)cdr);
    StringArray *newArray = newStringArray(cdr->length + 1);
//printf("[cons_strarr] 2\n");
    newArray->values[0] = car;
    
//printf("[cons_strarr] 3\n");
    for (i = 1; i < newArray->length; i++)
        {
//printf("[cons_strarr] 4 (iteration %i)\n", i);
        newArray->values[i] = cdr->values[i-1];
        }
//printf("[cons_strarr] 5\n");
    return newArray;
    }
    
static char *
cons_str(char car, char *cdr)
    {
    int i, newLength = strlen(cdr) + 2;
    
    char *newString = (char *)New(sizeof(char) * newLength);
    newString[0] = car;
    
    for (i = 1; i < newLength; i++)
        {
        newString[i] = cdr[i-1];
        }
	newString[newLength - 1] = '\0';
    return newString;
    }
    
static char *
charRange(char first, char last)
    {
//printf("[charRange] parameters, first: '%c', last: '%c'\n", first, last);
    char *range;
    char ch;
    if (first > last)
        {
        Fatal("in charRange: first '%c' greater than last '%c'\n", first, last);
        return 0;
        }
//printf("[charRange] range size: %i\n", last-first+2);
    range = (char *) New(sizeof(char) * (last-first+2)); /* range is inclusive AND must contain \0 */
    for (ch = first; ch <= last; ch++)
        {
//printf("[charRange] ch: '%c', index: %i\n", ch, ch-first);
        range[ch-first] = ch;
        }
//printf("[charRange] ch character 0, index: %i\n", last+1);
    range[last-first+1] = '\0';
//printf("[charRange] return value: '%s'\n", range);
    return range;
    }
    
/* unlike strcat, first is unchanged and the return value is a new string in memory */
/*
static char *
concatString(char *first, char *second)
    {
    int totalLength;
    char *retVal;
    
    if (first == 0) return StringDup(second);
    if (second == 0) return StringDup(first);
    
    totalLength = strlen(first) + strlen(second);
    retVal = (char *) New((sizeof(char) * totalLength) + 1);
    strcpy(retVal, first);
    strcat(retVal, second);
    return retVal;
    }
*/
/******** Growth functions ********/

static void
addPartToComposition(Part *p, Composition *c)
    {    
    int i;
    Part **oldParts = c->parts;
    c->parts = (Part **) New(sizeof(Part *) * (c->partCount+1));
    for (i=0;i<c->partCount;++i)
        {
        c->parts[i] = oldParts[i];
        }
    c->parts[c->partCount] = p;
    ++(c->partCount);
    
    p->composition = c;
    }
    
static void
addMeasureToPart(Measure *m, Part *p)
    {    
    int i;
    Measure **oldMeasures = p->measures;
    p->measures = (Measure **) New(sizeof(Measure *) * (p->measureCount+1));
    for (i=0;i<p->measureCount;++i)
        {
        p->measures[i] = oldMeasures[i];
        }
    p->measures[p->measureCount] = m;
    ++(p->measureCount);
    
    m->part = p;
    }
    
static void
addNoteInstanceToMeasure(NoteInstance *ni, Measure *m)
    {    
    int i;
    NoteInstance **oldNoteInstances = m->noteInstances;
    m->noteInstances = (NoteInstance **) New(sizeof(NoteInstance *) * (m->noteInstanceCount+1));
    for (i=0;i<m->noteInstanceCount;++i)
        {
        m->noteInstances[i] = oldNoteInstances[i];
        }
    m->noteInstances[m->noteInstanceCount] = ni;
    ++(m->noteInstanceCount);
    }

static void
addSectionToComposition(Section *s, Composition *c)
    {    
    int i;
    Section **oldSections = c->sections;
    c->sections = (Section **) New(sizeof(Section *) * (c->sectionCount+1));
    for (i=0;i<c->sectionCount;++i)
        {
        c->sections[i] = oldSections[i];
        }
    c->sections[c->sectionCount] = s;
    ++(c->sectionCount);
    
    s->composition = c;
    }

static void
addAttributeToSection(ASTAttribute *a, Section *s)
    {    
    int i;
    ASTAttribute **oldAttributes = s->attributes;
    s->attributes = (ASTAttribute **) New(sizeof(ASTAttribute *) * (s->attributeCount+1));
    for (i=0;i<s->attributeCount;++i)
        {
        s->attributes[i] = oldAttributes[i];
        }
    s->attributes[s->attributeCount] = a;
    ++(s->attributeCount);
    }
    
static void
addAttributeNameToComposition(char *an, Composition *c)
    {    
    int i;
    char **oldAttributeNames = c->attributeNames;
    c->attributeNames = (char **) New(sizeof(char *) * (c->attributeNameCount+1));
    for (i=0;i<c->attributeNameCount;++i)
        {
        c->attributeNames[i] = oldAttributeNames[i];
        }
    c->attributeNames[c->attributeNameCount] = an;
    ++(c->attributeNameCount);
    }

