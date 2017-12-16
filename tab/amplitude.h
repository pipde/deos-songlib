#ifndef INCLUDED_AMPLITUDE_H
#define INCLUDED_AMPLITUDE_H

/*
 * Defines the standard tab amplitude processor
 */

#include "tabproc.h"

extern void setupProcessor_amplitude(int, int, json_value *);

/* Pre- and post- processing */

extern void preProcess_amplitude(int, Part *);
extern void postProcess_amplitude(int, Part *);
    
/* Note rendering */
    
extern void preRenderNote_amplitude(int, Part *, int);
extern void renderNote_amplitude(int, Part *, int);

#endif /* INCLUDED_AMPLITUDE_H */
