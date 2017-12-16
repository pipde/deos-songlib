/*
 * Defines the standard tab tempo processor
 */

#include "tempo.h"

static double originalTempo;


typedef struct tempo
    {
    int section;
    double tempo;
    } Tempo;
        
static Tempo *
newTempo()
    {
    Tempo *t = (Tempo *)New(sizeof(Tempo));
    t->section = 0;
    t->tempo = -1;
    return t;
    }
    
static Tempo **tempos;
static int tempoCount = 0;

static double
findTempo(int section)
    {
    int i;
    for (i = 0; i < tempoCount; i++)
        {
        if (tempos[i]->section == section) return tempos[i]->tempo;
        }
    return -1;
    }

static void
addTempo(Tempo *tempo)
    {
    tempos = (Tempo **)ReNew(tempos, sizeof(Tempo *) * (tempoCount + 1));
    tempos[tempoCount++] = tempo;
    }

/* Processor setup */

void
setupProcessor_tempo(int parameterIndex, int sectionNumber, json_value *parameter)
    {    
    Tempo *tempo = newTempo();
    
    /* store original tempo */
    originalTempo = getTempo();
    
    tempo->section = sectionNumber;
    tempo->tempo = get_json_double(parameter, "", 0);
    
    addTempo(tempo);
    }

/* Pre- and post- processing */

void
preProcess_tempo(int parameterIndex, Part *part)
    {
    setTempo(originalTempo);
    }

void
postProcess_tempo(int parameterIndex, Part *part)
    {
    }
    
/* Section processing */
    
static void
processSection(int sectionNumber)
    {
    double tempo = findTempo(sectionNumber);
    if (tempo >= 0) 
    	{
		setTempo(tempo);
    	}
    }
 
/* Note rendering */
    
void
preRenderNote_tempo(int parameterIndex, Part *part, int noteNumber)
    {
    int i;
    for (i = 0; i < part->sectionCount; i++)
    	{
    	if (part->sections[i]->noteNumber == noteNumber)
    		{
    		processSection(i);
    		}
    	}
    }
    
void
renderNote_tempo(int parameterIndex, Part *part, int noteNumber)
	{
	}
