#ifndef INCLUDED_TEMPO_H
#define INCLUDED_TEMPO_H

/*
 * Defines the standard tab tempo processor
 */

#include "tabproc.h"

extern void setupProcessor_tempo(int, int, json_value *);

/* Pre- and post- processing */

extern void preProcess_tempo(int, Part *);
extern void postProcess_tempo(int, Part *);
    
/* Note rendering */
    
extern void preRenderNote_tempo(int, Part *, int);
extern void renderNote_tempo(int, Part *, int);

#endif /* INCLUDED_TEMPO_H */
