/* -*- c-basic-offset: 4 -*-  vi:set ts=8 sts=4 sw=4: */

/* less_trivial_synth.c

   DSSI Soft Synth Interface
   Constructed by Chris Cannam, Steve Harris and Sean Bolton

   This is an example DSSI synth plugin written by Steve Harris.

   This example file is in the public domain.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <stdint.h>

#include <math.h>
#include <stdio.h>

#include "dssi.h"
#include "ladspa.h"

#include "libmodsynth.h"
#include "libmodsynth/lib/amp.h"

#define LMS_OUTPUT  0
#define LMS_FREQ    1
#define LMS_ATTACK  2
#define LMS_DECAY   3
#define LMS_SUSTAIN 4
#define LMS_RELEASE 5
#define LMS_TIMBRE  6
#define LMS_RES  7
#define LMS_DIST 8
#define LMS_COUNT   9 /* must be 1 + highest value above */

//#define POLYPHONY   74
#define POLYPHONY   8
#define MIDI_NOTES  128
#define STEP_SIZE   16

#define GLOBAL_GAIN 0.25f

#define TABLE_MODULUS 1024
#define TABLE_SIZE    (TABLE_MODULUS + 1)
#define TABLE_MASK    (TABLE_MODULUS - 1)

#define FP_IN(x) (x.part.in & TABLE_MASK)
#define FP_FR(x) ((float)x.part.fr * 0.0000152587890625f)
#define FP_OMEGA(w) ((double)TABLE_MODULUS * 65536.0 * (w));

#define LERP(f,a,b) ((a) + (f) * ((b) - (a)))

long int lrintf (float x);

static LADSPA_Descriptor *ltsLDescriptor = NULL;
static DSSI_Descriptor *ltsDDescriptor = NULL;

//static float *table[2];

typedef enum {
    inactive = 0,
    attack,
    decay,
    sustain,
    release
} state_t;

typedef enum {
    off = 0,
    note_on,
    running,            
    note_off,
    releasing
} note_state;


typedef union {
    uint32_t all;
    struct {
#ifdef WORDS_BIGENDIAN
	uint16_t in;
	uint16_t fr;
#else
	uint16_t fr;
	uint16_t in;
#endif
    } part;
} fixp;

typedef struct {
    state_t state;
    note_state n_state;
    int     note;
    float   amp;
    float   env;
    float   env_d;
    fixp    phase;
    int     counter;
    int     next_event;
    /*LibModSynth additions*/
    float note_f;
    float osc_inc;
    float hz;
    poly_voice * _voice;
} voice_data;

typedef struct {
    LADSPA_Data tune;
    LADSPA_Data attack;
    LADSPA_Data decay;
    LADSPA_Data sustain;
    LADSPA_Data release;
    LADSPA_Data timbre;
    LADSPA_Data res;
    LADSPA_Data dist;
    LADSPA_Data pitch;    
} synth_vals;

typedef struct {
    LADSPA_Data *output;
    LADSPA_Data *tune;
    LADSPA_Data *attack;
    LADSPA_Data *decay;
    LADSPA_Data *sustain;
    LADSPA_Data *release;
    LADSPA_Data *timbre;
    LADSPA_Data *res;
    LADSPA_Data *dist;
    LADSPA_Data pitch;
    voice_data data[POLYPHONY];
    int note2voice[MIDI_NOTES];
    fixp omega[MIDI_NOTES];
    float fs;
    LADSPA_Data previous_timbre;
    /*LibModSynth additions*/
    float pitch_bend_amount;
} LTS;

static void runLTS(LADSPA_Handle instance, unsigned long sample_count,
		  snd_seq_event_t * events, unsigned long EventCount);

static void run_voice(LTS *p, synth_vals *vals, voice_data *d,
		      LADSPA_Data *out, unsigned int count);

int pick_voice(const voice_data *data);

const LADSPA_Descriptor *ladspa_descriptor(unsigned long index)
{
    switch (index) {
    case 0:
	return ltsLDescriptor;
    default:
	return NULL;
    }
}

const DSSI_Descriptor *dssi_descriptor(unsigned long index)
{
    switch (index) {
    case 0:
	return ltsDDescriptor;
    default:
	return NULL;
    }
}

static void cleanupLTS(LADSPA_Handle instance)
{
    free(instance);
}

static void connectPortLTS(LADSPA_Handle instance, unsigned long port,
			  LADSPA_Data * data)
{
    LTS *plugin;

    plugin = (LTS *) instance;
    switch (port) {
    case LMS_OUTPUT:
	plugin->output = data;
	break;
    case LMS_FREQ:
	plugin->tune = data;
	break;
    case LMS_ATTACK:
	plugin->attack = data;
	break;
    case LMS_DECAY:
	plugin->decay = data;
	break;
    case LMS_SUSTAIN:
	plugin->sustain = data;
	break;
    case LMS_RELEASE:
	plugin->release = data;
	break;
    case LMS_TIMBRE:
	plugin->timbre = data;              
	break;
    case LMS_RES:
	plugin->res = data;              
	break;
    case LMS_DIST:
	plugin->dist = data;              
	break;
    }
}

static LADSPA_Handle instantiateLTS(const LADSPA_Descriptor * descriptor,
				   unsigned long s_rate)
{
    unsigned int i;

    LTS *plugin_data = (LTS *) malloc(sizeof(LTS));

    plugin_data->fs = s_rate;
    plugin_data->previous_timbre = 0.5f;
    
    
    /*LibModSynth additions*/
    _init_lms(s_rate);  //initialize any static variables
    _mono_init();  //initialize all monophonic modules
    /*End LibModSynth additions*/
    
    
    for (i=0; i<MIDI_NOTES; i++) {
	plugin_data->omega[i].all =
		FP_OMEGA(pow(2.0, (i-69.0) / 12.0) / (double)s_rate);
    }
    
    return (LADSPA_Handle) plugin_data;
}

static void activateLTS(LADSPA_Handle instance)
{
    LTS *plugin_data = (LTS *) instance;
    unsigned int i;

    for (i=0; i<POLYPHONY; i++) {
	plugin_data->data[i].state = inactive;
        plugin_data->data[i]._voice = _poly_init();
    }
    for (i=0; i<MIDI_NOTES; i++) {
	plugin_data->note2voice[i] = 0;
    }
    plugin_data->pitch = 1.0f;
}

static void runLTSWrapper(LADSPA_Handle instance,
			 unsigned long sample_count)
{
    runLTS(instance, sample_count, NULL, 0);
}

static void runLTS(LADSPA_Handle instance, unsigned long sample_count,
		  snd_seq_event_t *events, unsigned long event_count)
{
    LTS *plugin_data = (LTS *) instance;
    
    LADSPA_Data *const output = plugin_data->output;
    synth_vals vals;
    voice_data *data = plugin_data->data;
    unsigned long i;
    unsigned long pos;
    unsigned long count;
    unsigned long event_pos;
    unsigned long voice;

    vals.tune = *(plugin_data->tune);
    vals.attack = *(plugin_data->attack) * plugin_data->fs;
    vals.decay = *(plugin_data->decay) * plugin_data->fs;
    vals.sustain = *(plugin_data->sustain) * 0.01f;
    vals.release = *(plugin_data->release) * plugin_data->fs;
    vals.timbre = *(plugin_data->timbre);
    vals.pitch = plugin_data->pitch;
    vals.res = *(plugin_data->res);
    vals.dist = *(plugin_data->dist);

    /*Events is an array of snd_seq_event_t objects, 
     event_count is the number of events,
     and sample_count is the block size          
     */
    for (pos = 0, event_pos = 0; pos < sample_count; pos += STEP_SIZE) 
    {	        
        /**/
	while (event_pos < event_count && pos >= events[event_pos].time.tick) 
        {
            /*Note on event*/
	    if (events[event_pos].type == SND_SEQ_EVENT_NOTEON) 
            {
		snd_seq_ev_note_t n = events[event_pos].data.note;

		if (n.velocity > 0) 
                {
		    const int voice = pick_voice(data);

		    plugin_data->note2voice[n.note] = voice;
		    data[voice].note = n.note;
		    data[voice].amp = _db_to_linear((n.velocity * 0.157480315) - 20) *  GLOBAL_GAIN; //-20db to 0db
		    data[voice].state = attack;                    
		    data[voice].env = 0.0;
		    data[voice].env_d = 1.0f / vals.attack;
		    data[voice].phase.all = 0;
		    data[voice].counter = 0;
		    data[voice].next_event = vals.attack;
                    
                    /*LibModSynth additions*/
                    data[voice].note_f = (float)n.note;
                    data[voice].hz = _pit_midi_note_to_hz(data[voice].note_f);
                    data[voice].osc_inc  = data[voice].hz * _sr_recip;
                    data[voice].n_state = note_on;
		} 
                /*0 velocity, essentially the same as note-off?*/
                else 
                {
		    const int voice = plugin_data->note2voice[n.note];

		    data[voice].state = release;
		    data[voice].env_d = -vals.sustain / vals.release;
		    data[voice].counter = 0;
		    data[voice].next_event = vals.release;
                    
                    /*LibModSynth additions*/
                    data[voice].n_state = note_off;
		}
	    } 
            /*Note-off event*/
            else if (events[event_pos].type == SND_SEQ_EVENT_NOTEOFF) 
            {
		snd_seq_ev_note_t n = events[event_pos].data.note;
                
                /*POLYPHONY is directly correlated to the actual index of the voice
                 in plugin_data->note2voice.*/
		const int voice = plugin_data->note2voice[n.note];

                /*Inactivate the voice if it's not already inactive*/
		if (data[voice].state != inactive) 
                {
		    data[voice].state = release;
		    data[voice].env_d = -data[voice].env / vals.release;
		    data[voice].counter = 0;
		    data[voice].next_event = vals.release;
                    
                    /*LibModSynth additions*/
                    data[voice].n_state = note_off;
		}
	    } 
            /*Pitch-bend sequencer event, modify the voices pitch*/
            else if (events[event_pos].type == SND_SEQ_EVENT_PITCHBEND) 
            {
                /*TODO:  integrate this part into the LibModSynth
                 system of doing things*/
		vals.pitch =
		    powf(2.0f, (float)(events[event_pos].data.control.value)
			 * 0.0001220703125f * 0.166666666f);
		plugin_data->pitch = vals.pitch;
	    }
	    event_pos++;
	}

	count = (sample_count - pos) > STEP_SIZE ? STEP_SIZE :
		sample_count - pos;
	
        /*I think this is just clearing the output buffer???*/
        for (i=0; i<count; i++) 
        {
	    output[pos + i] = 0.0f;
	}
        
	for (voice = 0; voice < POLYPHONY; voice++) 
        {
	    //if (data[voice].state != inactive) 
            if(data[voice].n_state != off)
            {
		run_voice(plugin_data, //The LTS class containing global synth data
                        &vals, //monophonic values for the the synth's controls
                        &data[voice], //The amp, envelope, state, etc... of the voice
                        output + pos, //output is the block array, I think + pos advances the index???
			  count //has to do with iterating through stepsize, but I'm not sure how
                        );
	    }
	}
    }
    
    /*TODO:  create a loop here that corresponds to mono effects not processed per-voice*/
    
    //plugin_data->previous_timbre = vals.timbre;
}

static void run_voice(LTS *p, synth_vals *vals, voice_data *d, LADSPA_Data *out, unsigned int count)
{
    /*TODO:  CHECK THE PER-VOICE/BLOCK PROCESSING ASPECT, SEE IF ANYTHING NEEDS TO BE MADE PER-VOICE, THAT
     PROBABLY ACCOUNTS FOR THE GLITCHES.  ALL MODULES SHOULD BE PLACED IN ARRAYS THAT CORRESPOND TO VOICES.*/
    
    /*Begin LibModSynth additions*/
    if((d->n_state) == note_on)
        {
            d->n_state = running;
            
        /*This is where you retrigger any envelopes, etc... on a note-on event*/    
            _adsr_retrigger(d->_voice->_adsr_filter);
        }
        else if((d->n_state) == note_off)
        {
            d->n_state = releasing;
            
            /*This is where you signal a note_off event to any modules that should receive it*/
            _adsr_release(d->_voice->_adsr_filter);
        }
    
    /*Put anything here that is not internally smoothed and only needs to be checked once per block*/
    
    _clp_set_in_gain(d->_voice->_clipper1, vals->dist);
    
    _svf_set_res(d->_voice->_svf_filter, vals->res);  
    
    /*End LibModSynth additions*/    
    
    unsigned int i;

    /*Process an audio block*/
    for (i=0; i<count; i++) {
	
	d->env += d->env_d; 
                
        /*Begin LibModSynth modifications, calling everything defined in
         libmodsynth.h in the order it should be called in*/
        
        /*Run any oscillators, etc...*/
        _run_osc(d->_voice->_osc_core_test, d->osc_inc);
        
        float _result = _get_saw(d->_voice->_osc_core_test); //Get a saw oscillator
        
        /*Run any processing of the initial result(s)*/      
        
        _adsr_run(d->_voice->_adsr_filter);
        
        /*TODO:  Add a note_on stage to the beginning of the voice data*/
        _svf_set_cutoff(d->_voice->_svf_filter, ((vals->timbre) + ((d->_voice->_adsr_filter->output) * 60)) );
        //_svf_set_cutoff(_svf_filter, vals->timbre); //this must be run every clock cycle because the cutoff is smoothed internally
                
        _svf_set_input_value(d->_voice->_svf_filter, _result); //run it through the filter
                
        _result = _clp_clip(d->_voice->_clipper1, d->_voice->_svf_filter->_lp); //run the lowpass filter output through a hard-clipper
        
        //_result = (_svf_filter->_lp);
        
        /*Run the envelope and assign to the output buffer*/
        out[i] += _result * (d->env) ; // * (d->amp);
                
        
        /*End LibModSynth modifications*/
    }

    /*Run the envelope.  TODO:  Structure this differently to fit better with
     LibModSynth*/
    d->counter += count;
    if (d->counter >= d->next_event) {
	switch (d->state) {
	case inactive:
	    break;
            
	case attack:
	    d->state = decay;
	    d->env_d = (vals->sustain - 1.0f) / vals->decay;
	    d->counter = 0;
	    d->next_event = vals->decay;
	    break;

	case decay:
	    d->state = sustain;
	    d->env_d = 0.0f;
	    d->counter = 0;
	    d->next_event = INT_MAX;
	    break;

	case sustain:
	    d->counter = 0;
	    break;

	case release:
	    d->state = inactive;
            d->n_state = inactive;
	    break;

	default:
	    d->state = inactive;
            d->n_state = inactive;
	    break;
	}
    }
}

/*This returns MIDI CCs for the different knobs
 TODO:  Try it with non-hex numbers*/
int getControllerLTS(LADSPA_Handle instance, unsigned long port)
{
    switch (port) {
    case LMS_ATTACK:
        return DSSI_CC(0x49);  //73
    case LMS_DECAY:
        return DSSI_CC(0x4b);  //75
    case LMS_SUSTAIN:
        return DSSI_CC(0x4f);  //79
    case LMS_RELEASE:
        return DSSI_CC(0x48);  //72
    case LMS_TIMBRE:
        return DSSI_CC(0x01);  //1
    }

    return DSSI_NONE;
}

/* Original comment:  find the voice that is least relevant (low note priority)*/
/*libmodsynth comment:  *data is an array of voices in an LTS struct,
 iterate through them for a free voice, or if one cannot be found,
 pick the highest voice*/
int pick_voice(const voice_data *data)
{
    unsigned int i;
    int highest_note = 0;
    int highest_note_voice = 0;

    /* Look for an inactive voice */
    for (i=0; i<POLYPHONY; i++) {
	if (data[i].state == inactive) {
	    return i;
	}
    }

    /* otherwise find for the highest note and replace that */
    for (i=0; i<POLYPHONY; i++) {
	if (data[i].note > highest_note) {
	    highest_note = data[i].note;
	    highest_note_voice = i;
	}
    }

    return highest_note_voice;
}

/*Here we define how all of the LADSPA and DSSI header stuff is setup,
 we also define the ports and the GUI.*/
#ifdef __GNUC__
__attribute__((constructor)) void init()
#else
void _init()
#endif
{
    unsigned int i;
    char **port_names;
    //float *sin_table;
    LADSPA_PortDescriptor *port_descriptors;
    LADSPA_PortRangeHint *port_range_hints;

    /*sin_table = malloc(sizeof(float) * TABLE_SIZE);
    for (i=0; i<TABLE_SIZE; i++) {
	sin_table[i] = sin(2.0 * M_PI * (double)i / (double)TABLE_MODULUS);
    }*/
//    table[0] = sin_table;
//    table[1] = saw_table;

    ltsLDescriptor =
	(LADSPA_Descriptor *) malloc(sizeof(LADSPA_Descriptor));
    if (ltsLDescriptor) {
	ltsLDescriptor->UniqueID = 24;
	ltsLDescriptor->Label = "LTS";
	ltsLDescriptor->Properties = 0;
	ltsLDescriptor->Name = "LibModSynth example synth";
	ltsLDescriptor->Maker = "Jeff Hubbard <libmodsynth.sourceforge.net>";
	ltsLDescriptor->Copyright = "GNU GPL v3";
	ltsLDescriptor->PortCount = LMS_COUNT;

	port_descriptors = (LADSPA_PortDescriptor *)
				calloc(ltsLDescriptor->PortCount, sizeof
						(LADSPA_PortDescriptor));
	ltsLDescriptor->PortDescriptors =
	    (const LADSPA_PortDescriptor *) port_descriptors;

	port_range_hints = (LADSPA_PortRangeHint *)
				calloc(ltsLDescriptor->PortCount, sizeof
						(LADSPA_PortRangeHint));
	ltsLDescriptor->PortRangeHints =
	    (const LADSPA_PortRangeHint *) port_range_hints;

	port_names = (char **) calloc(ltsLDescriptor->PortCount, sizeof(char *));
	ltsLDescriptor->PortNames = (const char **) port_names;

	/* Parameters for output */
	port_descriptors[LMS_OUTPUT] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
	port_names[LMS_OUTPUT] = "Output";
	port_range_hints[LMS_OUTPUT].HintDescriptor = 0;

	/* Parameters for tune */
	port_descriptors[LMS_FREQ] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_FREQ] = "A tuning (Hz)";
	port_range_hints[LMS_FREQ].HintDescriptor = LADSPA_HINT_DEFAULT_440 |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_FREQ].LowerBound = 410;
	port_range_hints[LMS_FREQ].UpperBound = 460;

	/* Parameters for attack */
	port_descriptors[LMS_ATTACK] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_ATTACK] = "Attack time (s)";
	port_range_hints[LMS_ATTACK].HintDescriptor =
			LADSPA_HINT_DEFAULT_LOW |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_ATTACK].LowerBound = 0.01f;
	port_range_hints[LMS_ATTACK].UpperBound = 1.0f;

	/* Parameters for decay */
	port_descriptors[LMS_DECAY] = port_descriptors[LMS_ATTACK];
	port_names[LMS_DECAY] = "Decay time (s)";
	port_range_hints[LMS_DECAY].HintDescriptor =
			port_range_hints[LMS_ATTACK].HintDescriptor;
	port_range_hints[LMS_DECAY].LowerBound =
			port_range_hints[LMS_ATTACK].LowerBound;
	port_range_hints[LMS_DECAY].UpperBound =
			port_range_hints[LMS_ATTACK].UpperBound;

	/* Parameters for sustain */
	port_descriptors[LMS_SUSTAIN] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_SUSTAIN] = "Sustain level (%)";
	port_range_hints[LMS_SUSTAIN].HintDescriptor =
			LADSPA_HINT_DEFAULT_HIGH |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_SUSTAIN].LowerBound = 0.0f;
	port_range_hints[LMS_SUSTAIN].UpperBound = 100.0f;

	/* Parameters for release */
	port_descriptors[LMS_RELEASE] = port_descriptors[LMS_ATTACK];
	port_names[LMS_RELEASE] = "Release time (s)";
	port_range_hints[LMS_RELEASE].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_LOGARITHMIC |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_RELEASE].LowerBound =
			port_range_hints[LMS_ATTACK].LowerBound;
	port_range_hints[LMS_RELEASE].UpperBound =
			port_range_hints[LMS_ATTACK].UpperBound * 4.0f;

	/* Parameters for timbre */
	port_descriptors[LMS_TIMBRE] = port_descriptors[LMS_ATTACK];
	port_names[LMS_TIMBRE] = "Timbre";
	port_range_hints[LMS_TIMBRE].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_TIMBRE].LowerBound =  30;//0.0f;
	port_range_hints[LMS_TIMBRE].UpperBound =  120;//1.0f;
        
        /* Parameters for res */
	port_descriptors[LMS_RES] = port_descriptors[LMS_ATTACK];
	port_names[LMS_RES] = "Res";
	port_range_hints[LMS_RES].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_RES].LowerBound =  -50;
	port_range_hints[LMS_RES].UpperBound =  0;
        
        
        /* Parameters for dist */
	port_descriptors[LMS_DIST] = port_descriptors[LMS_ATTACK];
	port_names[LMS_DIST] = "Dist";
	port_range_hints[LMS_DIST].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_DIST].LowerBound =  -6;
	port_range_hints[LMS_DIST].UpperBound =  36;
        

	ltsLDescriptor->activate = activateLTS;
	ltsLDescriptor->cleanup = cleanupLTS;
	ltsLDescriptor->connect_port = connectPortLTS;
	ltsLDescriptor->deactivate = NULL;
	ltsLDescriptor->instantiate = instantiateLTS;
	ltsLDescriptor->run = runLTSWrapper;
	ltsLDescriptor->run_adding = NULL;
	ltsLDescriptor->set_run_adding_gain = NULL;
    }

    ltsDDescriptor = (DSSI_Descriptor *) malloc(sizeof(DSSI_Descriptor));
    if (ltsDDescriptor) {
	ltsDDescriptor->DSSI_API_Version = 1;
	ltsDDescriptor->LADSPA_Plugin = ltsLDescriptor;
	ltsDDescriptor->configure = NULL;
	ltsDDescriptor->get_program = NULL;
	ltsDDescriptor->get_midi_controller_for_port = getControllerLTS;
	ltsDDescriptor->select_program = NULL;
	ltsDDescriptor->run_synth = runLTS;
	ltsDDescriptor->run_synth_adding = NULL;
	ltsDDescriptor->run_multiple_synths = NULL;
	ltsDDescriptor->run_multiple_synths_adding = NULL;
    }
}

#ifdef __GNUC__
__attribute__((destructor)) void fini()
#else
void _fini()
#endif
{
    if (ltsLDescriptor) {
	free((LADSPA_PortDescriptor *) ltsLDescriptor->PortDescriptors);
	free((char **) ltsLDescriptor->PortNames);
	free((LADSPA_PortRangeHint *) ltsLDescriptor->PortRangeHints);
	free(ltsLDescriptor);
    }
    if (ltsDDescriptor) {
	free(ltsDDescriptor);
    }
}
