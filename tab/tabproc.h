#ifndef INCLUDED_TABPROC_H
#define INCLUDED_TABPROC_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "/usr/local/include/songlib/songlib.h"
#include "/usr/local/include/songlib/util.h"
#include "json.h"

#define WHOLE_NOTE 1.0
#define HALF_NOTE WHOLE_NOTE/2
#define QUARTER_NOTE HALF_NOTE/2
#define EIGHTH_NOTE QUARTER_NOTE/2
#define SIXTEENTH_NOTE EIGHTH_NOTE/2
#define THIRTYSECOND_NOTE SIXTEENTH_NOTE/2

#define DOTTED_WHOLE_NOTE 1.5
#define DOTTED_HALF_NOTE DOTTED_WHOLE_NOTE/2
#define DOTTED_QUARTER_NOTE DOTTED_HALF_NOTE/2
#define DOTTED_EIGHTH_NOTE DOTTED_QUARTER_NOTE/2
#define DOTTED_SIXTEENTH_NOTE DOTTED_EIGHTH_NOTE/2
#define DOTTED_THIRTYSECOND_NOTE DOTTED_SIXTEENTH_NOTE/2

#define TRIPLET_WHOLE_NOTE 2.0/3
#define TRIPLET_HALF_NOTE TRIPLET_WHOLE_NOTE/2
#define TRIPLET_QUARTER_NOTE TRIPLET_HALF_NOTE/2
#define TRIPLET_EIGHTH_NOTE TRIPLET_QUARTER_NOTE/2
#define TRIPLET_SIXTEENTH_NOTE TRIPLET_EIGHTH_NOTE/2
#define TRIPLET_THIRTYSECOND_NOTE TRIPLET_SIXTEENTH_NOTE/2

#define REST 0

/**** Type definitions ****/

typedef struct note
    {
    double duration;
    char name;
    } Note;
    
typedef struct section
	{
	int noteNumber;
	} Section;

typedef struct part
    {
    char *voiceName;
    Note **notes;
    int noteCount;
    Section **sections;
    int sectionCount;
    } Part;
    
extern Part *part(char *, int, int);
extern Note *note(double, char);
extern Section *section(int);

/**** JSON manipulation function prototypes ****/

extern json_value *get_json_object(json_value *, ...);
// return value is length of array
extern int get_json_array(json_value *, json_value ***, ...);
extern int get_json_integer(json_value *, ...);
extern double get_json_double(json_value *, ...);
extern char *get_json_string(json_value *, ...);
extern bool get_json_boolean(json_value *, ...);

/**** Misc function prototypes ****/

extern void AssertFatal(bool, char *, ...);
extern void AssertWarning(bool, char *, ...);

#endif	/* INCLUDED_TABPROC_H */
