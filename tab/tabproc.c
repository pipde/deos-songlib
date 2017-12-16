#include "tabproc.h"

Note *
note(double duration, char name)
    {
    Note *n = (Note *)New(sizeof(Note));
    n->duration = duration;
    n->name = name;
    return n;
    }

Section *
section(int noteNumber)
    {
    Section *s = (Section *)New(sizeof(Section));
    s->noteNumber = noteNumber;
    return s;
    }
    
Part *
part(char *voiceName, int noteCount, int sectionCount)
    {
    Part *p = (Part *)New(sizeof(Part));
    p->voiceName = voiceName;
    p->notes = (Note **)New(sizeof(Note*) * noteCount);
    p->noteCount = noteCount;
    p->sections = (Section **)New(sizeof(Section *) * sectionCount);
    p->sectionCount = sectionCount;
    return p;
    }

/**** JSON manipulation functions ****/

static char *
decode_json_type(json_type type)
    {
    switch (type)
        {
        case json_none:
            return "json_none";
        case json_object:
            return "json_object";
        case json_array:
            return "json_array";
        case json_integer:
            return "json_integer";
        case json_double:
            return "json_double";
        case json_string:
            return "json_string";
        case json_boolean:
            return "json_boolean";
        case json_null:
            return "json_null";
        default:
            return "[unknown]";
        }
    }

/* args requires null terminator */

static json_value *
get_json_value(json_value *jsonVal, va_list args)
    {
    char *name, *next;
    int i;
    bool currentChanged;
    json_value *current = jsonVal;
    
    /* Expect at least one name supplied as a variadic parameter */
    name = va_arg(args, char *);
    AssertFatal(name != 0, "get_json_value requires at least one variadic parameter of type 'char *'\n");
    
    for (i = 0; i < current->u.object.length; i++)
        {
        if (strcmp(current->u.object.values[i].name, name) == 0)
            {
            current = current->u.object.values[i].value;
            currentChanged = true;
            break;
            }
        }
    if (!currentChanged)
        {
        // Not found
        return 0;
        }
        
    /* Loop through objects until we get to the end of the variadic parameters */
    while ((next = va_arg(args, char *)) != 0)
        {
        currentChanged = false;
        for (i = 0; i < current->u.object.length; i++)
            {
            if (strcmp(current->u.object.values[i].name, name) == 0)
                {
                current = current->u.object.values[i].value;
                currentChanged = true;
                break;
                }
            }
        if (!currentChanged)
            {
            // Not found
            return 0;
            }
        name = next;
        }
    return current;
    }
    
#define get_json_parent(priorArg) \
    va_list args; \
    json_value *parent; \
    va_start(args, priorArg); \
    parent = get_json_value(jsonVal, args); \
    va_end(args);

json_value *
get_json_object(json_value *jsonVal, ...)
    {
    get_json_parent(jsonVal);
    if (parent == 0) return 0;
    AssertFatal(parent->type == json_object, 
        "get_json_object found value of type '%s', expected 'json_object'\n", 
        decode_json_type(parent->type));
    return parent;
    }
    
/* return value is length of array */
/* parameter 'returnArray' is returned via reference */
int
get_json_array(json_value *jsonVal, json_value ***returnArray, ...)
    {
    get_json_parent(returnArray);
    if (parent == 0) 
        {
        returnArray = 0;
        return 0;
        }
    AssertFatal(parent->type == json_array, 
        "get_json_array found value of type '%s', expected 'json_array'\n", 
        decode_json_type(parent->type));
    *returnArray = parent->u.array.values;
    return parent->u.array.length;
    }
    
int
get_json_integer(json_value *jsonVal, ...)
    {
    get_json_parent(jsonVal);
    if (parent == 0) return 0;
    AssertFatal(parent->type == json_integer || parent->type == json_double || parent->type == json_boolean, 
        "get_json_integer found value of type '%s', expected 'json_integer', 'json_double', or 'json_boolean'\n",
        decode_json_type(parent->type));
    switch (parent->type)
        {
        case json_double:
            return parent->u.dbl;
        case json_boolean:
            return parent->u.boolean;
        default:
            return parent->u.integer;
        }
    }
    
double
get_json_double(json_value *jsonVal, ...)
    {
    get_json_parent(jsonVal);
    if (parent == 0) return 0;
    AssertFatal(parent->type == json_double || parent->type == json_integer, 
        "get_json_double found value of type '%s', expected 'json_double' or 'json_integer'\n",
        decode_json_type(parent->type));
    switch (parent->type)
        {
        case json_integer:
            return parent->u.integer;
        default:
            return parent->u.dbl;
        }
    }
    
char *
get_json_string(json_value *jsonVal, ...)
    {
    get_json_parent(jsonVal);
    if (parent == 0) return 0;
    AssertFatal(parent->type == json_string, 
        "get_json_string found value of type '%s', expected 'json_string'\n", 
        decode_json_type(parent->type));
    return parent->u.string.ptr;
    }
    
bool
get_json_boolean(json_value *jsonVal, ...)
    {
    get_json_parent(jsonVal);
    if (parent == 0) return false;
    AssertFatal(parent->type == json_boolean || parent->type == json_integer, 
        "get_json_boolean found value of type '%s', expected 'json_boolean' or 'json_integer'\n",
        decode_json_type(parent->type));
    switch (parent->type)
        {
        case json_integer:
            return parent->u.integer;
        default:
            return parent->u.boolean;
        }
    }

/**** Misc functions ****/
    
void
AssertFatal(bool value, char *message, ...)
    {
    va_list args;
    if (!value)
        {
        va_start(args, message);
        fprintf(stderr, "fatal: ");
        vfprintf(stderr, message, args);
        va_end(args);
        exit(-1);
        }
    }

void
AssertWarning(bool value, char *message, ...)
    {
    va_list args;
    if (!value)
        {
        va_start(args, message);
        fprintf(stderr, "warning: ");
        vfprintf(stderr, message, args);
        va_end(args);
        }
    }

