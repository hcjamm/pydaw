/* -*- c-basic-offset: 4 -*-  vi:set ts=8 sts=4 sw=4: */

/* synth.c

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

   
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

/*GUI Step 9:  Add ports to the main synthesizer file that the GUI can talk to */
#define LMS_OUTPUT  0
#define LMS_ATTACK  1
#define LMS_DECAY   2
#define LMS_SUSTAIN 3
#define LMS_RELEASE 4
#define LMS_TIMBRE  5
#define LMS_RES  6
#define LMS_DIST 7
#define LMS_FILTER_ATTACK  8
#define LMS_FILTER_DECAY   9
#define LMS_FILTER_SUSTAIN 10
#define LMS_FILTER_RELEASE 11
#define LMS_NOISE_AMP 12
#define LMS_FILTER_ENV_AMT 13
#define LMS_DIST_WET 14
#define LMS_OSC1_TYPE 15
#define LMS_OSC1_PITCH 16
#define LMS_OSC1_TUNE 17
#define LMS_OSC1_VOLUME 18
#define LMS_OSC2_TYPE 19
#define LMS_OSC2_PITCH 20
#define LMS_OSC2_TUNE 21
#define LMS_OSC2_VOLUME 22
#define LMS_MASTER_VOLUME 23
#define LMS_COUNT 24 /* must be 1 + highest value above CHANGE THIS IF YOU ADD ANYTHING*/

#define POLYPHONY   8  //maximum voices played at one time
#define MIDI_NOTES  128  //Maximum MIDI note.  You probably don't want to change this
#define STEP_SIZE   16

//#define GLOBAL_GAIN 0.25f   //TODO:  Get rid of this

long int lrintf (float x);

static LADSPA_Descriptor *ltsLDescriptor = NULL;
static DSSI_Descriptor *ltsDDescriptor = NULL;

typedef enum {
    off = 0,    
    running
} note_state;

typedef struct {
    int     note;
    float   amp;
    float note_f;
    float osc_inc1;
    float osc_inc2;
    float osc1_linamp;
    float osc2_linamp;
    float hz;
    poly_voice * _voice;    
    note_state n_state;
} voice_data;


/*GUI Step 10:  Add a variable for each control in the synth_vals type*/
typedef struct {    
    LADSPA_Data attack;
    LADSPA_Data decay;
    LADSPA_Data sustain;
    LADSPA_Data release;
    LADSPA_Data timbre;
    LADSPA_Data res;
    LADSPA_Data dist;
    LADSPA_Data pitch;    
    
    LADSPA_Data attack_f;
    LADSPA_Data decay_f;
    LADSPA_Data sustain_f;
    LADSPA_Data release_f;
    
    LADSPA_Data osc1pitch;
    LADSPA_Data osc1tune;
    LADSPA_Data osc1type;
    LADSPA_Data osc1vol;
    
    LADSPA_Data osc2pitch;
    LADSPA_Data osc2tune;
    LADSPA_Data osc2type;
    LADSPA_Data osc2vol;
    
    LADSPA_Data filter_env_amt;
    LADSPA_Data dist_wet;
    LADSPA_Data master_vol;    
    
    LADSPA_Data noise_amp;
} synth_vals;

/*GUI Step 11:  Add a variable for each control in the LTS type*/
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
    
    LADSPA_Data *attack_f;
    LADSPA_Data *decay_f;
    LADSPA_Data *sustain_f;
    LADSPA_Data *release_f;
    
    LADSPA_Data *osc1pitch;
    LADSPA_Data *osc1tune;
    LADSPA_Data *osc1type;
    LADSPA_Data *osc1vol;
    
    LADSPA_Data *osc2pitch;
    LADSPA_Data *osc2tune;
    LADSPA_Data *osc2type;
    LADSPA_Data *osc2vol;
    
    LADSPA_Data *filter_env_amt;
    LADSPA_Data *dist_wet;
    LADSPA_Data *master_vol;
    
    LADSPA_Data *noise_amp;
    
    voice_data data[POLYPHONY];
    int note2voice[MIDI_NOTES];    
    float fs;
    /*LibModSynth additions*/
    float pitch_bend_amount;
} LTS;


static void runLTS(LADSPA_Handle instance, unsigned long sample_count,
		  snd_seq_event_t * events, unsigned long EventCount);

static void run_voice(LTS *p, synth_vals *vals, voice_data *d,
		      LADSPA_Data *out, unsigned int count);

int pick_voice(const voice_data *data, int);

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
    
    /*GUI Step 12:  Add the ports from step 9 to the connectPortLTS event handler*/
    
    switch (port) {
    case LMS_OUTPUT:
	plugin->output = data;
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
    case LMS_FILTER_ATTACK:
	plugin->attack_f = data;
	break;
    case LMS_FILTER_DECAY:
	plugin->decay_f = data;
	break;
    case LMS_FILTER_SUSTAIN:
	plugin->sustain_f = data;
	break;
    case LMS_FILTER_RELEASE:
	plugin->release_f = data;
	break;
    case LMS_NOISE_AMP:
        plugin->noise_amp = data;
        break;
    case LMS_DIST_WET:
        plugin->dist_wet = data;
        break;
    case LMS_FILTER_ENV_AMT:
        plugin->filter_env_amt = data;
        break;
    case LMS_MASTER_VOLUME:
        plugin->master_vol = data;
        break;
    case LMS_OSC1_PITCH:
        plugin->osc1pitch = data;
        break;
    case LMS_OSC1_TUNE:
        plugin->osc1tune = data;
        break;
    case LMS_OSC1_TYPE:
        plugin->osc1type = data;
        break;
    case LMS_OSC1_VOLUME:
        plugin->osc1vol = data;
        break;
    case LMS_OSC2_PITCH:
        plugin->osc2pitch = data;
        break;
    case LMS_OSC2_TUNE:
        plugin->osc2tune = data;
        break;
    case LMS_OSC2_TYPE:
        plugin->osc2type = data;
        break;
    case LMS_OSC2_VOLUME:
        plugin->osc2vol = data;
        break;    
    }
}

static LADSPA_Handle instantiateLTS(const LADSPA_Descriptor * descriptor,
				   unsigned long s_rate)
{
    LTS *plugin_data = (LTS *) malloc(sizeof(LTS));
    
    plugin_data->fs = s_rate;
    
    /*LibModSynth additions*/
    _init_lms(s_rate);  //initialize any static variables
    _mono_init();  //initialize all monophonic modules
    /*End LibModSynth additions*/
    
    return (LADSPA_Handle) plugin_data;
}

static void activateLTS(LADSPA_Handle instance)
{
    LTS *plugin_data = (LTS *) instance;
    unsigned int i;

    for (i=0; i<POLYPHONY; i++) {
        plugin_data->data[i].n_state = off;
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

    /*GUI Step 13:  Set the values from synth_vals in RunLTS*/
    vals.attack = *(plugin_data->attack);
    vals.decay = *(plugin_data->decay); 
    vals.sustain = *(plugin_data->sustain);
    vals.release = *(plugin_data->release);
    vals.timbre = *(plugin_data->timbre);
    vals.pitch = 0; 
    vals.res = *(plugin_data->res);
    vals.dist = *(plugin_data->dist);

    vals.attack_f = *(plugin_data->attack_f);
    vals.decay_f = *(plugin_data->decay_f); 
    vals.sustain_f = *(plugin_data->sustain_f);
    vals.release_f = *(plugin_data->release_f);
    
    vals.noise_amp = *(plugin_data->noise_amp);
    
    vals.dist_wet = *(plugin_data->dist_wet);    
    vals.filter_env_amt = *(plugin_data->filter_env_amt);
    vals.master_vol = *(plugin_data->master_vol);
    
    vals.osc1pitch = *(plugin_data->osc1pitch);
    vals.osc1tune = *(plugin_data->osc1tune);
    vals.osc1type = *(plugin_data->osc1type);
    vals.osc1vol = *(plugin_data->osc1vol);
    
    vals.osc2pitch = *(plugin_data->osc2pitch);
    vals.osc2tune = *(plugin_data->osc2tune);
    vals.osc2type = *(plugin_data->osc2type);
    vals.osc2vol = *(plugin_data->osc2vol);
    
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
		    const int voice = pick_voice(data, n.note);
                    
		    plugin_data->note2voice[n.note] = voice;
		    data[voice].note = n.note;
		    data[voice].amp = _db_to_linear((n.velocity * 0.157480315) - 20 + (vals.master_vol)); //-20db to 0db, + master volume (0 to -60)
		    
                    
                    /*LibModSynth additions*/
                    data[voice].note_f = (float)n.note;
                    data[voice].hz = _pit_midi_note_to_hz(data[voice].note_f);
                    
                    data[voice].osc_inc1  = (_pit_midi_note_to_hz((data[voice].note_f) + (vals.osc1pitch) + (vals.osc1tune))) * _sr_recip;
                    data[voice].osc_inc2  = (_pit_midi_note_to_hz((data[voice].note_f) + (vals.osc2pitch) + (vals.osc2tune))) * _sr_recip;
                    
                    data[voice].osc1_linamp = _db_to_linear(vals.osc1vol);
                    data[voice].osc2_linamp = _db_to_linear(vals.osc2vol);
                    
                    data[voice].n_state = running;
                    
                    
                    /*Here is where we perform any actions that should ONLY happen at note_on, you can save a lot of CPU by
                     placing things here that don't need to be modulated as a note is playing*/
                    
                    /*Retrigger ADSR envelopes*/
                    _adsr_retrigger(data[voice]._voice->_adsr_amp);
                    _adsr_retrigger(data[voice]._voice->_adsr_filter);
                    
                    _adsr_set_adsr_db(data[voice]._voice->_adsr_amp, (vals.attack), (vals.decay), (vals.sustain), (vals.release));
                    _adsr_set_adsr(data[voice]._voice->_adsr_filter, (vals.attack_f), (vals.decay_f), (vals.sustain_f), (vals.release_f));
                    
                    _clp_set_in_gain(data[voice]._voice->_clipper1, vals.dist);
    
                    _svf_set_res(data[voice]._voice->_svf_filter, vals.res);  
                    
                    data[voice]._voice->_noise_amp = _db_to_linear((vals.noise_amp));
                    
                    _axf_set_xfade(data[voice]._voice->_dist_dry_wet, vals.dist_wet);
                    
                    switch((int)(vals.osc1type))
                    {
                        case 0:
                            data[voice]._voice->_osc1_type = _get_saw;
                            break;
                        case 1:
                            data[voice]._voice->_osc1_type = _get_square;
                            break;
                        case 2:
                            data[voice]._voice->_osc1_type = _get_triangle;
                            break;
                        case 3:
                            data[voice]._voice->_osc1_type = _get_sine;
                            break;
                        case 4:
                            printf("invalid osc1type\n%f\n", vals.osc1type);
                            data[voice]._voice->_osc1_type = _get_saw;
                            break;    
                    }
                    
                    
                    switch((int)(vals.osc2type))
                    {
                        case 0:
                            data[voice]._voice->_osc2_type = _get_saw;
                            break;
                        case 1:
                            data[voice]._voice->_osc2_type = _get_square;
                            break;
                        case 2:
                            data[voice]._voice->_osc2_type = _get_triangle;
                            break;
                        case 3:
                            data[voice]._voice->_osc2_type = _get_sine;
                            break;
                        case 4:
                            printf("invalid osc2type\n%f\n", vals.osc2type);
                            data[voice]._voice->_osc2_type = _get_saw;
                            break;    
                    }
                    
                    
                    printf("note_on\n%f\n", (vals.master_vol));
		} 
                /*0 velocity, essentially the same as note-off?*/
                else 
                {
		    const int voice = plugin_data->note2voice[n.note];

                    /*LibModSynth additions*/
                    //data[voice].n_state = note_off;
                    
                    _poly_note_off(data[voice]._voice);
                    
                    printf("note_off\n");
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
                if(data[voice].n_state != off)
                {                    
                    /*LibModSynth additions*/                    
                    _poly_note_off(data[voice]._voice);
                                        
                    printf("note_off\n");
		}
	    } 
            /*Pitch-bend sequencer event, modify the voices pitch*/
            else if (events[event_pos].type == SND_SEQ_EVENT_PITCHBEND) 
            {
		vals.pitch = 0.000061035 //TODO:  Make this a "pitchbend amount" variable, and double-check that it is a 15 bit message
                        * events[event_pos].data.control.value;
                printf("%f", vals.pitch);
		    /*powf(2.0f, (float)(events[event_pos].data.control.value)
			 * 0.0001220703125f * 0.166666666f);*/
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
        
    /*TODO:  create a loop here that corresponds to mono effects not processed per-voice*/
        
    }
    
}

static void run_voice(LTS *p, synth_vals *vals, voice_data *d, LADSPA_Data *out, unsigned int count)
{   
    /*Put anything here that is not internally smoothed and only needs to be checked once per block, per voice*/
    
    
    
    /*End LibModSynth additions*/    
    
    unsigned int i;

    /*Process an audio block*/
    for (i=0; i<count; i++) {
	
	//d->env += d->env_d; 
                
        /*Begin LibModSynth modifications, calling everything defined in
         libmodsynth.h in the order it should be called in*/
        
        /*Run any oscillators, etc...*/
        _run_osc(d->_voice->_osc_core1, d->osc_inc1);
        _run_osc(d->_voice->_osc_core2, d->osc_inc2);
        
        //float _result = d->_voice->_osc1_type(d->_voice->_osc_core1);
        //_result = d->_voice->_osc1_type(d->_voice->_osc_core1);
        
        float _result = (d->_voice->_osc1_type(d->_voice->_osc_core1) * (d->osc1_linamp)) +   //osc1
        (d->_voice->_osc2_type(d->_voice->_osc_core2) * (d->osc2_linamp)) +  //osc2
        (_run_w_noise(d->_voice->_w_noise) * (d->_voice->_noise_amp)); //white noise
        
        /*
        float _result = (_get_saw(d->_voice->_osc_core1) * (d->osc1_linamp)) +   //osc1
        (_get_saw(d->_voice->_osc_core2) * (d->osc2_linamp)) +  //osc2
        (_run_w_noise(d->_voice->_w_noise) * (d->_voice->_noise_amp)); //white noise
        */
        
        /*Run any processing of the initial result(s)*/      
        
        _adsr_run(d->_voice->_adsr_amp);        
        _adsr_run(d->_voice->_adsr_filter);
        
        _svf_set_cutoff(d->_voice->_svf_filter, ((vals->timbre) + ((d->_voice->_adsr_filter->output) * (vals->filter_env_amt))) );
                        
        _svf_set_input_value(d->_voice->_svf_filter, _result); //run it through the filter
                
        _result = _axf_run_xfade(d->_voice->_dist_dry_wet, (d->_voice->_svf_filter->_lp), 
                _clp_clip(d->_voice->_clipper1, d->_voice->_svf_filter->_lp)); //run the lowpass filter output through a hard-clipper, mixed by the dry/wet knob
        
        //_result = (d->_voice->_svf_filter->_lp);
        
        /*Run the envelope and assign to the output buffer*/
        out[i] += _result *  (d->_voice->_adsr_amp->output) * (d->amp) ; 
                
        
        /*End LibModSynth modifications*/
    }

    
    /*If the main ADSR envelope has reached it's release stage, kill the voice.
     However, you don't have to necessarily have to kill the voice, but you will waste a lot of CPU if you don't*/
    if(d->_voice->_adsr_amp->stage == 4)
    {
        d->n_state = off;        
    }
        
}

/*This returns MIDI CCs for the different knobs
 TODO:  Try it with non-hex numbers*/
/*GUI Step 15:  Assign the LADSPA ports defined in step 9 to MIDI CCs in getControllerLTS*/
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
    case LMS_DIST:
        return DSSI_CC(0x14);  //20            
    case LMS_FILTER_ATTACK:
        return DSSI_CC(0x15);  //21
    case LMS_FILTER_DECAY:
        return DSSI_CC(0x16);  //22
    case LMS_FILTER_SUSTAIN:
        return DSSI_CC(0x17);  //23
    case LMS_FILTER_RELEASE:
        return DSSI_CC(0x18);  //24
    case LMS_NOISE_AMP:        
        return DSSI_CC(0x19);  //25
    case LMS_FILTER_ENV_AMT:
        return DSSI_CC(0x1a);  //26
    case LMS_DIST_WET:
        return DSSI_CC(0x1b);  //27            
    case LMS_OSC1_TYPE:
        return DSSI_CC(0x1c);  //28
    case LMS_OSC1_PITCH:
        return DSSI_CC(0x1d);  //29
    case LMS_OSC1_TUNE:
        return DSSI_CC(0x1e);  //30
    case LMS_OSC1_VOLUME:
        return DSSI_CC(0x1f);  //31
    case LMS_OSC2_TYPE:
        return DSSI_CC(0x20);  //32
    case LMS_OSC2_PITCH:
        return DSSI_CC(0x21);  //33
    case LMS_OSC2_TUNE:
        return DSSI_CC(0x22);  //34
    case LMS_OSC2_VOLUME:
        return DSSI_CC(0x23);  //35            
    case LMS_MASTER_VOLUME:        
        return DSSI_CC(0x24);  //36
        
    }

    return DSSI_NONE;
}

/* Original comment:  find the voice that is least relevant (low note priority)*/
/*libmodsynth comment:  *data is an array of voices in an LMS struct,
 iterate through them for a free voice, or if one cannot be found,
 pick the highest voice*/
int pick_voice(const voice_data *data, int _current_note)
{
    unsigned int i;
    int highest_note = 0;
    int highest_note_voice = 0;
    
    /*Look for the voice being played by the current note.
     It's more musical to kill the same note than to let it play twice,
     guitars, pianos, etc... work that way.  It also helps to prevent hung notes*/    
    for (i=0; i<POLYPHONY; i++) {
	if (data[i].note == _current_note) {
            printf("pick_voice found current_note\n");
	    return i;
	}
    }
    
    /* Look for an inactive voice */
    for (i=0; i<POLYPHONY; i++) {
	if (data[i].n_state == off) {
            printf("pick_voice found inactive voice %i\n", i);
	    return i;
	}
    }

    /* otherwise find for the highest note and replace that */
    for (i=0; i<POLYPHONY; i++) {
	if (data[i].note > highest_note) {
	    highest_note = data[i].note;
	    highest_note_voice = i;
            printf("pick_voice found highest voice %i\n", i);
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
    char **port_names;
    LADSPA_PortDescriptor *port_descriptors;
    LADSPA_PortRangeHint *port_range_hints;

    ltsLDescriptor =
	(LADSPA_Descriptor *) malloc(sizeof(LADSPA_Descriptor));
    if (ltsLDescriptor) {
	ltsLDescriptor->UniqueID = 24;  //TODO:  Find out what this means
	ltsLDescriptor->Label = "LTS";  //Changing this breaks the plugin, it compiles, but hangs when trying to run.  TODO:  investigate
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

        /*GUI Step 14:  Define the LADSPA ports for the plugin in the class constructor*/
        
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
	port_range_hints[LMS_SUSTAIN].LowerBound = -60.0f;
	port_range_hints[LMS_SUSTAIN].UpperBound = 0.0f;

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
	port_range_hints[LMS_TIMBRE].LowerBound =  39;//0.0f;
	port_range_hints[LMS_TIMBRE].UpperBound =  136;//1.0f;
        
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
        
        
        
        
	/* Parameters for attack_f */
	port_descriptors[LMS_FILTER_ATTACK] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_FILTER_ATTACK] = "Attack time (s) filter";
	port_range_hints[LMS_FILTER_ATTACK].HintDescriptor =
			LADSPA_HINT_DEFAULT_LOW |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_FILTER_ATTACK].LowerBound = 0.01f;
	port_range_hints[LMS_FILTER_ATTACK].UpperBound = 1.0f;

	/* Parameters for decay_f */
	port_descriptors[LMS_FILTER_DECAY] = port_descriptors[LMS_ATTACK];
	port_names[LMS_FILTER_DECAY] = "Decay time (s) filter";
	port_range_hints[LMS_FILTER_DECAY].HintDescriptor =
			port_range_hints[LMS_ATTACK].HintDescriptor;
	port_range_hints[LMS_FILTER_DECAY].LowerBound =
			port_range_hints[LMS_ATTACK].LowerBound;
	port_range_hints[LMS_FILTER_DECAY].UpperBound =
			port_range_hints[LMS_ATTACK].UpperBound;

	/* Parameters for sustain_f */
	port_descriptors[LMS_FILTER_SUSTAIN] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_FILTER_SUSTAIN] = "Sustain level (%) filter";
	port_range_hints[LMS_FILTER_SUSTAIN].HintDescriptor =
			LADSPA_HINT_DEFAULT_HIGH |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_FILTER_SUSTAIN].LowerBound = 0.0f;
	port_range_hints[LMS_FILTER_SUSTAIN].UpperBound = 1.0f;
        
	/* Parameters for release_f */
	port_descriptors[LMS_FILTER_RELEASE] = port_descriptors[LMS_ATTACK];
	port_names[LMS_FILTER_RELEASE] = "Release time (s) filter";
	port_range_hints[LMS_FILTER_RELEASE].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_LOGARITHMIC |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_FILTER_RELEASE].LowerBound =
			port_range_hints[LMS_ATTACK].LowerBound;
	port_range_hints[LMS_FILTER_RELEASE].UpperBound =
			port_range_hints[LMS_ATTACK].UpperBound * 4.0f;

        
        /*Parameters for noise_amp*/        
	port_descriptors[LMS_NOISE_AMP] = port_descriptors[LMS_ATTACK];
	port_names[LMS_NOISE_AMP] = "Dist";
	port_range_hints[LMS_NOISE_AMP].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_NOISE_AMP].LowerBound =  -60;
	port_range_hints[LMS_NOISE_AMP].UpperBound =  0;
        
        
        
        /*Parameters for filter env amt*/        
	port_descriptors[LMS_FILTER_ENV_AMT] = port_descriptors[LMS_ATTACK];
	port_names[LMS_FILTER_ENV_AMT] = "Filter Env Amt";
	port_range_hints[LMS_FILTER_ENV_AMT].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_FILTER_ENV_AMT].LowerBound =  -36;
	port_range_hints[LMS_FILTER_ENV_AMT].UpperBound =  36;
        
        /*Parameters for dist wet*/        
	port_descriptors[LMS_DIST_WET] = port_descriptors[LMS_ATTACK];
	port_names[LMS_DIST_WET] = "Filter Env Amt";
	port_range_hints[LMS_DIST_WET].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_DIST_WET].LowerBound =  0.0f;
	port_range_hints[LMS_DIST_WET].UpperBound =  1.0f;
        
        
        /*Parameters for osc1type*/        
	port_descriptors[LMS_OSC1_TYPE] = port_descriptors[LMS_ATTACK];
	port_names[LMS_OSC1_TYPE] = "Filter Env Amt";
	port_range_hints[LMS_OSC1_TYPE].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_OSC1_TYPE].LowerBound =  0;
	port_range_hints[LMS_OSC1_TYPE].UpperBound =  4;
        
        
        /*Parameters for osc1pitch*/        
	port_descriptors[LMS_OSC1_PITCH] = port_descriptors[LMS_ATTACK];
	port_names[LMS_OSC1_PITCH] = "Filter Env Amt";
	port_range_hints[LMS_OSC1_PITCH].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_OSC1_PITCH].LowerBound =  -12;
	port_range_hints[LMS_OSC1_PITCH].UpperBound =  12;
        
        
        /*Parameters for osc1tune*/        
	port_descriptors[LMS_OSC1_TUNE] = port_descriptors[LMS_ATTACK];
	port_names[LMS_OSC1_TUNE] = "Filter Env Amt";
	port_range_hints[LMS_OSC1_TUNE].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_OSC1_TUNE].LowerBound =  -1.0f;
	port_range_hints[LMS_OSC1_TUNE].UpperBound =  1.0f;
        
        
        /*Parameters for osc1vol*/        
	port_descriptors[LMS_OSC1_VOLUME] = port_descriptors[LMS_ATTACK];
	port_names[LMS_OSC1_VOLUME] = "Filter Env Amt";
	port_range_hints[LMS_OSC1_VOLUME].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_OSC1_VOLUME].LowerBound =  -60;
	port_range_hints[LMS_OSC1_VOLUME].UpperBound =  0;
        
        
        
        /*Parameters for osc2type*/        
	port_descriptors[LMS_OSC2_TYPE] = port_descriptors[LMS_ATTACK];
	port_names[LMS_OSC2_TYPE] = "Filter Env Amt";
	port_range_hints[LMS_OSC2_TYPE].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_OSC2_TYPE].LowerBound =  0;
	port_range_hints[LMS_OSC2_TYPE].UpperBound =  4;
        
        
        /*Parameters for osc2pitch*/        
	port_descriptors[LMS_OSC2_PITCH] = port_descriptors[LMS_ATTACK];
	port_names[LMS_OSC1_PITCH] = "Filter Env Amt";
	port_range_hints[LMS_OSC2_PITCH].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_OSC2_PITCH].LowerBound =  -12;
	port_range_hints[LMS_OSC2_PITCH].UpperBound =  12;
        
        
        /*Parameters for osc2tune*/        
	port_descriptors[LMS_OSC2_TUNE] = port_descriptors[LMS_ATTACK];
	port_names[LMS_OSC2_TUNE] = "Filter Env Amt";
	port_range_hints[LMS_OSC2_TUNE].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_OSC2_TUNE].LowerBound =  -1.0f;
	port_range_hints[LMS_OSC2_TUNE].UpperBound =  1.0f;
        
        
        /*Parameters for osc2vol*/        
	port_descriptors[LMS_OSC2_VOLUME] = port_descriptors[LMS_ATTACK];
	port_names[LMS_OSC2_VOLUME] = "Filter Env Amt";
	port_range_hints[LMS_OSC2_VOLUME].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_OSC2_VOLUME].LowerBound =  -60;
	port_range_hints[LMS_OSC2_VOLUME].UpperBound =  0;
        
        
        /*Parameters for master vol*/        
	port_descriptors[LMS_MASTER_VOLUME] = port_descriptors[LMS_ATTACK];
	port_names[LMS_MASTER_VOLUME] = "Filter Env Amt";
	port_range_hints[LMS_MASTER_VOLUME].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_MASTER_VOLUME].LowerBound =  -60;
	port_range_hints[LMS_MASTER_VOLUME].UpperBound =  0;
        
        
        
        
        /*Here is where the functions in synth.c get pointed to for the host to call*/
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
	ltsDDescriptor->configure = NULL;  //TODO:  I think this is where the host can set plugin state, etc...
	ltsDDescriptor->get_program = NULL;  //TODO:  This is where program change is read, plugin state retrieved, etc...
	ltsDDescriptor->get_midi_controller_for_port = getControllerLTS;
	ltsDDescriptor->select_program = NULL;  //TODO:  This is how the host can select programs, not sure how it differs from a MIDI program change
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
