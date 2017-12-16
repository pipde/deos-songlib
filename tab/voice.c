/*
 * Defines the standard tab voice processor
 */

#include "voice.h"

#define ACCENT_MULTIPLIER 2.0

// TODO: Allow amplitudes to be read as "p", "mf", "ff", etc.
// Also check these amplitudes... not sure if they are good
#define AMPLITUDE_MULTIPLIER_PPP 1/6.0
#define AMPLITUDE_MULTIPLIER_PP 1/4.0
#define AMPLITUDE_MULTIPLIER_P 1/2.0
#define AMPLITUDE_MULTIPLIER_MP 1/1.5
#define AMPLITUDE_MULTIPLIER_MF 1.0
#define AMPLITUDE_MULTIPLIER_F 2.0
#define AMPLITUDE_MULTIPLIER_FF 4.0
#define AMPLITUDE_MULTIPLIER_FFF 6.0

/* Voice parameters */

typedef struct voice_note
    {
    char name;
    int offset;
    bool accent;
	int delay;
    } VoiceNote;
    
typedef struct voice_instrument
    {
    double pitchMultiplier;
    double amplitudeMultiplier;
    double timingDeviation;
    } VoiceInstrument;
    
typedef struct voice_definition
    {
    int id;
    char *instrumentPath;
    char *instrumentBaseName;
    int songlibInstrument;
    char *name;
    VoiceNote **notes;
    int noteCount;
    double amplitudeMultiplier;
    VoiceInstrument **instruments;
    int instrumentCount;
    } VoiceDefinition;
    
static VoiceNote *
newVoiceNote()
    {
    VoiceNote *vn = (VoiceNote *)New(sizeof(VoiceNote));
    vn->name = -1;
    vn->offset = -1;
    vn->accent = false;
	vn->delay = 0;
    return vn;
    }
    
static VoiceInstrument *
newVoiceInstrument()
    {
    VoiceInstrument *vi = (VoiceInstrument *)New(sizeof(VoiceInstrument));
    vi->pitchMultiplier = 0.0;
    vi->amplitudeMultiplier = 0.0;
    vi->timingDeviation = 0.0;
    return vi;
    }
    
static VoiceDefinition *
newVoiceDefinition()
    {
    VoiceDefinition *v = (VoiceDefinition *)New(sizeof(VoiceDefinition));
    v->id = 0;
    v->instrumentPath = 0;
    v->instrumentBaseName = 0;
    v->songlibInstrument = -1;
    v->name = 0;
    v->notes = 0;
    v->noteCount = 0;
    v->amplitudeMultiplier = 0; 
    v->instruments = 0;
    v->instrumentCount = 0;
    return v;
    }
    
static void
addNoteToVoice(VoiceNote *note, VoiceDefinition *voice)
    {
    voice->notes = (VoiceNote **)ReNew(voice->notes, sizeof(VoiceNote *) * (voice->noteCount + 1));
    voice->notes[voice->noteCount++] = note;
    }
    
static void
addInstrumentToVoice(VoiceInstrument *instrument, VoiceDefinition *voice)
    {
    voice->instruments = (VoiceInstrument **)ReNew(voice->instruments, sizeof(VoiceInstrument *) * (voice->instrumentCount + 1));
    voice->instruments[voice->instrumentCount++] = instrument;
    }
    
static VoiceDefinition **voices = 0;
static int voiceCount = 0;

static void
addVoice(VoiceDefinition *voice)
    {
    voices = (VoiceDefinition **)ReNew(voices, sizeof(VoiceDefinition *) * (voiceCount + 1));
    voices[voiceCount++] = voice;
    }

static VoiceNote *
findVoiceNote(char noteName, VoiceDefinition *voice)
    {
    int i;
    /* First, try finding explicitly defined notes */
    for (i = 0; i < voice->noteCount; i++)
        {
        if (voice->notes[i]->name == noteName) return voice->notes[i];
        }
    
    /* TODO: add a parameter baseOffset, so that '0' through '9' are built automatically */
    /* This will be most useful for guitar tabs, with individual voices representing strings E, A, D, G, B, E*/
    return 0;
    }

static VoiceDefinition *
findVoice(char *name)
    {
    int i;
    for (i = 0; i < voiceCount; i++)
        {
        if (strcmp(voices[i]->name, name) == 0) return voices[i];
        }
    return 0;
    }

/* Processor setup */
    
void
setupProcessor_voice(int parameterIndex, int sectionNumber, json_value *parameter)
    {
    int noteCount, i, instrumentCount;
    VoiceDefinition *voice = newVoiceDefinition();
    VoiceNote *note;
    VoiceInstrument *instrument;
    double voiceTimingDeviation, amplitudeDeviation, pitchDeviation, randNum;
    char *nameStr;
    json_value *jsonNote, **jsonNotes = (json_value **)New(0);
        
    voice->id = parameterIndex;
    
    /* Voice name is required */
    voice->name = get_json_string(parameter, "name", 0);
    AssertFatal(voice->name != 0, "voice must have a valid name\n");
    
    /* Instrument path is required */
    voice->instrumentPath = get_json_string(parameter, "instrumentPath", 0);
    AssertFatal(voice->instrumentPath != 0, "voice '%s' must have a valid instrument path\n", voice->name);
    
    /* Instrument base name is required */
    voice->instrumentBaseName = get_json_string(parameter, "instrumentBaseName", 0);
    AssertFatal(voice->instrumentBaseName != 0, "voice '%s' must have a valid instrument base name\n", voice->name);
    
    voice->songlibInstrument = readScale(voice->instrumentPath, voice->instrumentBaseName);
    
    /* Amplitude multiplier is optional */
    voice->amplitudeMultiplier = get_json_double(parameter, "amplitudeMultiplier", 0);
        
    /* Pitch deviation is optional, measured in positive semitones */
    pitchDeviation = get_json_double(parameter, "pitchDeviation", 0);
    AssertFatal(pitchDeviation >= 0.0, "voice '%s' pitchDeviation must be greater than or equal to 0\n", voice->name);
    
    /* Amplitude deviation is optional (default 1.0), measured as a multiplier over the base amplitude */
    amplitudeDeviation = get_json_double(parameter, "amplitudeDeviation", 0);
    if (amplitudeDeviation == 0.0) amplitudeDeviation = 1.0;
    AssertFatal(amplitudeDeviation >= 1.0, "voice '%s' amplitudeDeviation must be greater than or equal to 1\n", voice->name);
    
    /* Timing deviation is optional */
    voiceTimingDeviation = get_json_double(parameter, "timingDeviation", 0);
    
    /* Instrument count is optional */
    instrumentCount = get_json_integer(parameter, "instrumentCount", 0);
    if (instrumentCount < 1) instrumentCount = 1;
    for (i = 0; i < instrumentCount; i++)
        {
        instrument = newVoiceInstrument();
        
        /* permanently set pitch multiplier for this voice instrument */
        randNum = randomRange(0, pitchDeviation);
        instrument->pitchMultiplier = pow(2.0, randNum/12.0);
//        fprintf(stderr, "pitch multiplier: %f\n", instrument->pitchMultiplier);
        
        /* permanently set amplitude multiplier for this voice instrument */
        randNum = randomRange(0, amplitudeDeviation - 1.0);
        instrument->amplitudeMultiplier = (randNum >= 0.0) ? (randNum + 1.0) : 1.0/(-1.0 * (randNum - 1.0));
//        fprintf(stderr, "amplitude multiplier: %f\n", instrument->amplitudeMultiplier);
        
        /* permanently set timing deviation for this voice instrument */
        instrument->timingDeviation = randomRange(0, voiceTimingDeviation);
//        fprintf(stderr, "timing deviation: %f\n", instrument->timingDeviation);
        
        addInstrumentToVoice(instrument, voice);
        }
    
    noteCount = get_json_array(parameter, &jsonNotes, "notes", 0);
    for (i = 0; i < noteCount; i++)
        {
        jsonNote = jsonNotes[i];
        note = newVoiceNote();
        
        /* Note names are required, and must be length 1 (type char) */
        nameStr = get_json_string(jsonNote, "name", 0);
        AssertFatal(nameStr != 0, "voice '%s' note must have a valid name\n", voice->name);
        AssertFatal(strlen(nameStr) == 1, "voice '%s' note '%s' name does not have length 1\n", voice->name, nameStr);
        note->name = nameStr[0];
        
        /* Offset is optional, but must be positive or zero (default) */
        note->offset = get_json_integer(jsonNote, "offset", 0);
        AssertFatal(note->offset >= 0, "voice '%s' offset is less than zero\n", voice->name);
        
        /* Accent and delay are optional */
        note->accent = get_json_boolean(jsonNote, "accent", 0);
		note->delay = get_json_integer(jsonNote, "delay", 0);
        addNoteToVoice(note, voice);
        }
        
    addVoice(voice);
    }

/* Pre- and post- processing */

void
preProcess_voice(int parameterIndex, Part *part)
    {
    }

void
postProcess_voice(int parameterIndex, Part *part)
    {
    }
    
/* Note rendering */
    
void
preRenderNote_voice(int parameterIndex, Part *part, int noteNumber)
	{
	}
	
void
renderNote_voice(int parameterIndex, Part *part, int noteNumber)
    {
    Note *note = part->notes[noteNumber];
    VoiceNote *voiceNote;
    RRA *rra, *resampled;
    int delay, i,
        startLocation = getLocation();
    double startAmplitude, tdiff,
        amplitudeMultiplier = 1, accentMultiplier = 1, 
        beats = note->duration * getNoteValue();
    VoiceDefinition *voice = findVoice(part->voiceName);    

    AssertFatal(voice != 0, "voice not defined: '%s'\n", part->voiceName);
        
    /* Only render this note if the supplied parameter index matches the supplied part */
    if (parameterIndex != voice->id) return; 
    
    /* Rests are easy and frequent, process them first */
    if (note->name == REST)
        {
        rest(beats);
        return;
        }
    
    /* Find the note */
    voiceNote = findVoiceNote(note->name, voice);
    AssertFatal(voiceNote != 0, "note not defined: '%c'\n", note->name);
    
    /* If the note has a delay, move backwards (-) or forwards (+) based on that delay */
	delay = voiceNote->delay;		
    
    /* Set amplitude by adjusting current amplitude for accent and amplitudeMultiplier options */
    startAmplitude = getAmplitude();
    if (voiceNote->accent) accentMultiplier = ACCENT_MULTIPLIER;
    if (voice->amplitudeMultiplier > 0) 
        {
        amplitudeMultiplier = voice->amplitudeMultiplier;
        }    
    
    //printf("[renderNote_voice] note offset %i at %f for %f beats\n", offset, getLocation(), beats);
    for (i = 0; i < voice->instrumentCount; i++)
        {
        setLocation(startLocation + delay);
    
        /* set amplitude based on voice and instrument amplitude multipliers, accent, and instrument count */
        setAmplitude(startAmplitude * amplitudeMultiplier * accentMultiplier * voice->instruments[i]->amplitudeMultiplier / voice->instrumentCount);
    
        /* get specific timing offset for this note instance */
        tdiff = randomRange(0, voice->instruments[i]->timingDeviation);
        
        /* Offset timing before playing note */
        forwards(tdiff);

        /* resample note */
        rra = getNumberedNote(voice->songlibInstrument, voiceNote->offset);
        resampled = resample(rra, voice->instruments[i]->pitchMultiplier);

        /* play note and free resampled RRA */
        rplay(beats, resampled);
        freeRRA(resampled, 0);
        }
    
    /* Reset amplitude and location (adjusted for size of note) */
    setAmplitude(startAmplitude);
    setLocation(startLocation + beatsToSamples(beats));
    }
