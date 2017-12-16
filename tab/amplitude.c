/*
 * Defines the standard tab amplitude processor
 */

#include "amplitude.h"

static double originalAmplitude;


typedef struct amplitude
    {
    int section;
    double amplitude;
    } Amplitude;
        
static Amplitude *
newAmplitude()
    {
    Amplitude *t = (Amplitude *)New(sizeof(Amplitude));
    t->section = 0;
    t->amplitude = -1;
    return t;
    }
    
static Amplitude **amplitudes;
static int amplitudeCount = 0;

static double
findAmplitude(int section)
    {
    int i;
    for (i = 0; i < amplitudeCount; i++)
        {
        if (amplitudes[i]->section == section) return amplitudes[i]->amplitude;
        }
    return -1;
    }

static void
addAmplitude(Amplitude *amplitude)
    {
    amplitudes = (Amplitude **)ReNew(amplitudes, sizeof(Amplitude *) * (amplitudeCount + 1));
    amplitudes[amplitudeCount++] = amplitude;
    }

/* Processor setup */

void
setupProcessor_amplitude(int parameterIndex, int sectionNumber, json_value *parameter)
    {    
    Amplitude *amplitude = newAmplitude();
    
    /* store original amplitude */
    originalAmplitude = getAmplitude();
    
    amplitude->section = sectionNumber;
    amplitude->amplitude = get_json_double(parameter, "", 0);
    
    addAmplitude(amplitude);
    }

/* Pre- and post- processing */

void
preProcess_amplitude(int parameterIndex, Part *part)
    {
    setAmplitude(originalAmplitude);
    }

void
postProcess_amplitude(int parameterIndex, Part *part)
    {
    }
    
/* Section processing */
    
static void
processSection(int sectionNumber)
    {
    double amplitude = findAmplitude(sectionNumber);
    if (amplitude >= 0) 
    	{
    	setAmplitude(amplitude);
    	}
    }
    
/* Note rendering */
    
void
preRenderNote_amplitude(int parameterIndex, Part *part, int noteNumber)
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
renderNote_amplitude(int parameterIndex, Part *part, int noteNumber)
	{
	}
