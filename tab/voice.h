#ifndef INCLUDED_VOICE_H
#define INCLUDED_VOICE_H

/*
 * Defines the standard tab voice processor
 */

#include "tabproc.h"

extern void setupProcessor_voice(int, int, json_value *);

/* Pre- and post- processing */

extern void preProcess_voice(int, Part *);
extern void postProcess_voice(int, Part *);
    
/* Note rendering */
    
extern void preRenderNote_voice(int, Part *, int);
extern void renderNote_voice(int, Part *, int);

#endif /* INCLUDED_VOICE_H */
