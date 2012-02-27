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
/*Comment these out when compiling a stable, production-ready plugin.  
 The debugging code wastes a lot of CPU, and end users don't really need to see it*/
//#define LMS_DEBUG_NOTE
//#define LMS_DEBUG_MAIN_LOOP

/*Then you can print debug information like this:
#ifdef LMS_DEBUG_MODE
printf("debug information");
#endif
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
#include "../libmodsynth/lib/amp.h"
#include "../libmodsynth/modules/filter/svf.h"


#define LMS_OUTPUT  0
/*GUI Step 11:  Add ports to the main synthesizer file that the GUI can talk to */
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
#define LMS_MASTER_UNISON_VOICES 24
#define LMS_MASTER_UNISON_SPREAD 25
#define LMS_MASTER_GLIDE 26
#define LMS_MASTER_PITCHBEND_AMT 27
#define LMS_PITCH_ENV_TIME 28
#define LMS_PITCH_ENV_AMT 29
#define LMS_PROGRAM_CHANGE 30
#define LMS_COUNT 31 /* must be 1 + highest value above CHANGE THIS IF YOU ADD OR TAKE AWAY ANYTHING*/


#define POLYPHONY   8  //maximum voices played at one time
#define MIDI_NOTES  128  //Maximum MIDI note.  You probably don't want to change this
#define STEP_SIZE   16


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
    float osc1_linamp;
    float osc2_linamp;
    float noise_linamp;
    float hz;
    t_poly_voice * p_voice;    
    note_state n_state;
    int i_voice;  //for the runVoice function to iterate the current block
#ifdef LMS_DEBUG_MAIN_LOOP
    int debug_counter;
#endif
} voice_data;

#ifdef LMS_DEBUG_MAIN_LOOP

int debug_interval = 176400;

void dump_debug_voice_data(voice_data*);

/*Any changes to voice_data require this to be changed*/
void dump_debug_voice_data(voice_data * a_data)
{
    printf("\n\nRunning dump_debug_voice_data\n");
    printf("amp == %f\n", a_data->amp);
    printf("hz == %f\n", a_data->hz);
    printf("n_state == %i\n", a_data->n_state);
    printf("noise_linamp == %f\n", a_data->noise_linamp);
    printf("n_state == %i\n", a_data->note);
    printf("note_f == %f\n", a_data->note_f);
    printf("osc1_linamp == %f\n", a_data->osc1_linamp);    
    printf("osc2_linamp == %f\n", a_data->osc2_linamp);    
}

#endif



/*GUI Step 12:  Add a variable for each control in the synth_vals type*/
typedef struct {    
    /*The variables below this line correspond to GUI controls*/
    LADSPA_Data attack;
    LADSPA_Data decay;
    LADSPA_Data sustain;
    LADSPA_Data release;    
    LADSPA_Data dist;
    LADSPA_Data dist_wet;
    
    LADSPA_Data attack_f;
    LADSPA_Data decay_f;
    LADSPA_Data sustain_f;
    LADSPA_Data release_f;
    LADSPA_Data filter_env_amt;
    LADSPA_Data timbre;
    LADSPA_Data res;
    
    LADSPA_Data osc1pitch;
    LADSPA_Data osc1tune;
    LADSPA_Data osc1type;
    LADSPA_Data osc1vol;
    
    LADSPA_Data osc2pitch;
    LADSPA_Data osc2tune;
    LADSPA_Data osc2type;
    LADSPA_Data osc2vol;
    
    LADSPA_Data master_vol;       
    LADSPA_Data master_uni_voice;
    LADSPA_Data master_uni_spread;
    LADSPA_Data master_glide;
    LADSPA_Data master_pb_amt;
    
    LADSPA_Data pitch_env_amt;
    LADSPA_Data pitch_env_time;
    
    LADSPA_Data noise_amp;
    
    /*The variables below this line do NOT correspond to GUI controls*/
#ifdef LMS_DEBUG_MAIN_LOOP
    int debug_counter;
#endif
} synth_vals;

#ifdef LMS_DEBUG_MAIN_LOOP

void dump_debug_synth_vals(synth_vals*);

/*Any changes to voice_data require this to be changed*/
void dump_debug_synth_vals(synth_vals * a_data)
{
    printf("\n\nRunning dump_debug_synth_vals\n");
    printf("attack == %f\n", a_data->attack);
    printf("attack_f == %f\n", a_data->attack_f);
    printf("decay == %f\n", a_data->decay);
    printf("decay_f == %f\n", a_data->decay_f);
    printf("dist == %f\n", a_data->dist);
    printf("dist_wet == %f\n", a_data->dist_wet);
    printf("filter_env_amt == %f\n", a_data->filter_env_amt);    
    printf("master_glide == %f\n", a_data->master_glide);    
    printf("master_pb_amt == %f\n", a_data->master_pb_amt);
    printf("master_uni_spread == %f\n", a_data->master_uni_spread);
    printf("master_uni_voice == %f\n", a_data->master_uni_voice);
    printf("master_vol == %f\n", a_data->master_vol);
    printf("noise_amp == %f\n", a_data->noise_amp);
    printf("osc1pitch == %f\n", a_data->osc1pitch);
    printf("osc1tune == %f\n", a_data->osc1tune);    
    printf("osc1type == %f\n", a_data->osc1type);   
    printf("osc1vol == %f\n", a_data->osc1vol);    
    printf("osc2pitch == %f\n", a_data->osc2pitch);
    printf("osc2tune == %f\n", a_data->osc2tune);
    printf("osc2type == %f\n", a_data->osc2type);
    printf("osc2vol == %f\n", a_data->osc2vol);
    printf("pitch_env_amt == %f\n", a_data->pitch_env_amt);
    printf("pitch_env_time == %f\n", a_data->pitch_env_time);    
    printf("release == %f\n", a_data->release);
    printf("release_f == %f\n", a_data->release_f);
    printf("res == %f\n", a_data->res);    
    printf("sustain == %f\n", a_data->sustain);    
    printf("sustain_f == %f\n", a_data->sustain_f);
    printf("timbre == %f\n", a_data->timbre);  
}

#endif




/*GUI Step 13:  Add a variable for each control in the LTS type*/
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
    
    
    LADSPA_Data *master_uni_voice;
    LADSPA_Data *master_uni_spread;
    LADSPA_Data *master_glide;
    LADSPA_Data *master_pb_amt;
    
    LADSPA_Data *pitch_env_amt;
    LADSPA_Data *pitch_env_time;
    
    LADSPA_Data *program;
    
    voice_data data[POLYPHONY];
    int note2voice[MIDI_NOTES];    
    float fs;    
    t_mono_modules * mono_modules;
    synth_vals vals;
    
    int pos;
    int count;
    int event_pos;
    int voice;
    int i_buffer_clear;    
} LTS;


static float sv_pitch_bend_value = 0;
static float sv_last_note = 60;  //For glide
/*For preventing references to values like last_note that will be null, change to 1 once the first note plays*/


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
    
    /*GUI Step 14:  Add the ports from step 9 to the connectPortLTS event handler*/
    
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
    case LMS_MASTER_UNISON_VOICES:
        plugin->master_uni_voice = data;
        break;
    case LMS_MASTER_UNISON_SPREAD:
        plugin->master_uni_spread = data;        
        break;
    case LMS_MASTER_GLIDE:
        plugin->master_glide = data;
        break;
    case LMS_MASTER_PITCHBEND_AMT:
        plugin->master_pb_amt = data;
        break;
    case LMS_PITCH_ENV_AMT:
        plugin->pitch_env_amt = data;
        break;
    case LMS_PITCH_ENV_TIME:
        plugin->pitch_env_time = data;
        break;
    case LMS_PROGRAM_CHANGE:
        plugin->program = data;
        break;
    }
}

static LADSPA_Handle instantiateLTS(const LADSPA_Descriptor * descriptor,
				   unsigned long s_rate)
{
    LTS *plugin_data = (LTS *) malloc(sizeof(LTS));
    
    plugin_data->fs = s_rate;
    
    /*LibModSynth additions*/
    v_init_lms(s_rate);  //initialize any static variables    
    /*End LibModSynth additions*/
    
    return (LADSPA_Handle) plugin_data;
}

static void activateLTS(LADSPA_Handle instance)
{
    LTS *plugin_data = (LTS *) instance;
    unsigned int i;

    for (i=0; i<POLYPHONY; i++) {
        plugin_data->data[i].n_state = off;
        plugin_data->data[i].note = i;
        plugin_data->data[i].note_f = i;
        plugin_data->data[i].p_voice = g_poly_init();
    }
    for (i=0; i<MIDI_NOTES; i++) {
	plugin_data->note2voice[i] = 0;
    }
    plugin_data->pitch = 1.0f;
    
    plugin_data->mono_modules = v_mono_init();  //initialize all monophonic modules
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
    voice_data *data = plugin_data->data;
    
    plugin_data->pos = 0;
    plugin_data->count= 0;
    plugin_data->event_pos = 0;
    plugin_data->voice = 0;
    
    /*GUI Step 15:  Set the values from synth_vals in RunLTS*/
    plugin_data->vals.attack = *(plugin_data->attack) * .01;
    plugin_data->vals.decay = *(plugin_data->decay) * .01; 
    plugin_data->vals.sustain = *(plugin_data->sustain);
    plugin_data->vals.release = *(plugin_data->release) * .01;
    plugin_data->vals.timbre = *(plugin_data->timbre);
    
    plugin_data->vals.res = *(plugin_data->res);
    plugin_data->vals.dist = *(plugin_data->dist);

    plugin_data->vals.attack_f = *(plugin_data->attack_f) * .01;
    plugin_data->vals.decay_f = *(plugin_data->decay_f) * .01; 
    plugin_data->vals.sustain_f = *(plugin_data->sustain_f) * .01;
    plugin_data->vals.release_f = *(plugin_data->release_f) * .01;
    
    plugin_data->vals.noise_amp = *(plugin_data->noise_amp);
    
    plugin_data->vals.dist_wet = *(plugin_data->dist_wet) * .01;    
    plugin_data->vals.filter_env_amt = *(plugin_data->filter_env_amt);
    plugin_data->vals.master_vol = *(plugin_data->master_vol);
    
    plugin_data->vals.osc1pitch = *(plugin_data->osc1pitch);
    plugin_data->vals.osc1tune = *(plugin_data->osc1tune) * .01;
    plugin_data->vals.osc1type = *(plugin_data->osc1type);
    plugin_data->vals.osc1vol = *(plugin_data->osc1vol);
    
    plugin_data->vals.osc2pitch = *(plugin_data->osc2pitch);
    plugin_data->vals.osc2tune = *(plugin_data->osc2tune) * .01;
    plugin_data->vals.osc2type = *(plugin_data->osc2type);
    plugin_data->vals.osc2vol = *(plugin_data->osc2vol);
    
    plugin_data->vals.master_uni_voice = *(plugin_data->master_uni_voice);
    plugin_data->vals.master_uni_spread = *(plugin_data->master_uni_spread) * .01;
    plugin_data->vals.master_glide = *(plugin_data->master_glide) * .01;
    plugin_data->vals.master_pb_amt = *(plugin_data->master_pb_amt);
    
    plugin_data->vals.pitch_env_amt = *(plugin_data->pitch_env_amt);
    plugin_data->vals.pitch_env_time = *(plugin_data->pitch_env_time) * .01;
    
    /*Events is an array of snd_seq_event_t objects, 
     event_count is the number of events,
     and sample_count is the block size          
     */
    while ((plugin_data->pos) < sample_count) 
    {	        
        v_smr_iir_run(plugin_data->mono_modules->filter_smoother, (plugin_data->vals.timbre));
        v_smr_iir_run(plugin_data->mono_modules->pitchbend_smoother, sv_pitch_bend_value);
        
	while ((plugin_data->event_pos) < event_count)
        {
#ifdef LMS_DEBUG_NOTE
            printf("Event firing\n");
#endif                      
            /*Note on event*/
	    if (events[(plugin_data->event_pos)].type == SND_SEQ_EVENT_NOTEON) 
            {
		snd_seq_ev_note_t n = events[(plugin_data->event_pos)].data.note;

		if (n.velocity > 0) 
                {                    
                    
#ifdef LMS_DEBUG_NOTE
                        printf("note_on note# %i\n", n.note);
                        
#endif                    
                    
		    const int voice = pick_voice(data, n.note);
                    
		    plugin_data->note2voice[n.note] = voice;
		    data[voice].note = n.note;
		    data[voice].amp = f_db_to_linear_fast((n.velocity * 0.094488) - 12 + (plugin_data->vals.master_vol)); //-20db to 0db, + master volume (0 to -60)
                    v_svf_velocity_mod(data[voice].p_voice->svf_filter, n.velocity);
                    
                    /*LibModSynth additions*/
                    
#ifdef LMS_DEBUG_MAIN_LOOP
                    data[voice].debug_counter = 0;                    
#endif
                    data[voice].note_f = (float)n.note;
                    data[voice].hz = f_pit_midi_note_to_hz(data[voice].note_f);
                    
                    
                    data[voice].p_voice->target_pitch = (data[voice].note_f);
                    
                    //data[voice].p_voice->real_pitch = sv_last_note;
                                        
                    v_sml_set_smoother_glide(data[voice].p_voice->glide_smoother, (data[voice].p_voice->target_pitch), sv_last_note,
                            (plugin_data->vals.master_glide));
                                        
                    /*These are the values to multiply the oscillators by, DO NOT use the one's in vals*/
                    data[voice].osc1_linamp = f_db_to_linear_fast(plugin_data->vals.osc1vol);
                    data[voice].osc2_linamp = f_db_to_linear_fast(plugin_data->vals.osc2vol);
                    data[voice].noise_linamp = f_db_to_linear_fast(plugin_data->vals.noise_amp);
                    
                    data[voice].n_state = running;
                                        
                    /*Here is where we perform any actions that should ONLY happen at note_on, you can save a lot of CPU by
                     placing things here that don't need to be modulated as a note is playing*/
                    
                    /*Retrigger ADSR envelopes*/
                    v_adsr_retrigger(data[voice].p_voice->adsr_amp);
                    v_adsr_retrigger(data[voice].p_voice->adsr_filter);
                    
                    v_adsr_set_adsr_db(data[voice].p_voice->adsr_amp, (plugin_data->vals.attack), (plugin_data->vals.decay), (plugin_data->vals.sustain), (plugin_data->vals.release));
                    v_adsr_set_adsr(data[voice].p_voice->adsr_filter, (plugin_data->vals.attack_f), (plugin_data->vals.decay_f), (plugin_data->vals.sustain_f), (plugin_data->vals.release_f));
                    
                    /*Retrigger the pitch envelope*/
                    v_rmp_retrigger((data[voice].p_voice->pitch_env), (plugin_data->vals.pitch_env_time));  
                    
                    v_clp_set_in_gain(data[voice].p_voice->clipper1, plugin_data->vals.dist);
    
                    v_svf_set_res(data[voice].p_voice->svf_filter, plugin_data->vals.res);  
                    
                    data[voice].p_voice->noise_amp = f_db_to_linear((plugin_data->vals.noise_amp));
                    
                    v_axf_set_xfade(data[voice].p_voice->dist_dry_wet, plugin_data->vals.dist_wet);       
                    
                    /*Set the oscillator type(saw, square, etc...)*/
                    v_osc_set_simple_osc_unison_type(data[voice].p_voice->osc_unison1, (int)(plugin_data->vals.osc1type));
                    v_osc_set_simple_osc_unison_type(data[voice].p_voice->osc_unison2, (int)(plugin_data->vals.osc2type));   
                    
                    /*Set the number of unison voices*/
                    v_osc_set_uni_voice_count(data[voice].p_voice->osc_unison1, plugin_data->vals.master_uni_voice);
                    v_osc_set_uni_voice_count(data[voice].p_voice->osc_unison2, plugin_data->vals.master_uni_voice);
                    
                                        
                    /*Set the last_note property, so the next note can glide from it if glide is turned on*/
                    sv_last_note = (data[voice].note_f);
		} 
                /*0 velocity, the same as note-off*/
                else 
                {
                    snd_seq_ev_note_t n = events[(plugin_data->event_pos)].data.note;
                    
		    const int voice = plugin_data->note2voice[n.note];

                    /*LibModSynth additions*/
                    if(data[voice].n_state != off)
                    {                  
                        v_poly_note_off(data[voice].p_voice);

#ifdef LMS_DEBUG_NOTE
                        printf("note_off zero velocity note# %i\n", n.note);
#endif
                    }                    
		}
	    } 
            /*Note-off event*/
            else if (events[(plugin_data->event_pos)].type == SND_SEQ_EVENT_NOTEOFF) 
            {
		snd_seq_ev_note_t n = events[(plugin_data->event_pos)].data.note;
                
                /*POLYPHONY is directly correlated to the actual index of the voice
                 in plugin_data->note2voice.*/
		const int voice = plugin_data->note2voice[n.note];

                /*Inactivate the voice if it's not already inactive*/		
                if(data[voice].n_state != off)
                {                    
                    /*LibModSynth additions*/                    
                    v_poly_note_off(data[voice].p_voice);
#ifdef LMS_DEBUG_NOTE                                        
                    printf("note_off note off event note# %i\n", n.note);
#endif
		}
	    } 
            /*Pitch-bend sequencer event, modify the voices pitch*/
            else if (events[(plugin_data->event_pos)].type == SND_SEQ_EVENT_PITCHBEND) 
            {
		sv_pitch_bend_value = 0.00012207   //0.000061035 
                        * events[(plugin_data->event_pos)].data.control.value * (plugin_data->vals.master_pb_amt);
#ifdef LMS_DEBUG_NOTE                
                printf("_pitchbend_value is %f\n", sv_pitch_bend_value);		
#endif                
	    }
	    plugin_data->event_pos = (plugin_data->event_pos) + 1;
	}

        /*TODO:  WTF does this mean?*/
	plugin_data->count = (sample_count - (plugin_data->pos)) > STEP_SIZE ? STEP_SIZE :	sample_count - (plugin_data->pos);
	
        /*Clear the output buffer*/
        plugin_data->i_buffer_clear = 0;
        
        while((plugin_data->i_buffer_clear)<(plugin_data->count))
        {
	    output[((plugin_data->pos) + (plugin_data->i_buffer_clear))] = 0.0f;
            plugin_data->i_buffer_clear = (plugin_data->i_buffer_clear) + 1;
	}
        
        plugin_data->voice = 0; 
	while ((plugin_data->voice) < POLYPHONY) 
        {
	    //if (data[voice].state != inactive) 
            if(data[(plugin_data->voice)].n_state != off)
            {
		run_voice(plugin_data, //The LTS class containing global synth data
                        &(plugin_data->vals), //monophonic values for the the synth's controls
                        &data[(plugin_data->voice)], //The amp, envelope, state, etc... of the voice
                        output + (plugin_data->pos), //output is the block array, I think + pos advances the index???
			  (plugin_data->count) //has to do with iterating through stepsize, but I'm not sure how
                        );
	    }
            
            plugin_data->voice = (plugin_data->voice) + 1; 
	}
        
        plugin_data->pos = (plugin_data->pos) + STEP_SIZE;
    /*TODO:  create a loop here that corresponds to mono effects not processed per-voice*/
                
    }
    
}

static void run_voice(LTS *p, synth_vals *vals, voice_data *d, LADSPA_Data *out, unsigned int count)
{   
    
    //int f_i = 0;
    d->i_voice = 0;
    
    /*Process an audio block*/
    while((d->i_voice)<count) {
	
        /*Here is where we periodically dump debug information if debugging is enabled*/
#ifdef LMS_DEBUG_MAIN_LOOP
        d->debug_counter = (d->debug_counter) + 1;
        
        int is_debug_printing = 0;
        
        if((d->debug_counter) >= debug_interval)
        {
            d->debug_counter = 0;
            dump_debug_voice_data(d);
            dump_debug_synth_vals(vals);
            dump_debug_t_poly_voice(d->p_voice);
            is_debug_printing = 1;
        }
#endif
                
        /*Call everything defined in libmodsynth.h in the order it should be called in*/
        d->p_voice->current_sample = 0;
        
        /*Run the glide module*/        
        
        v_sml_run_glide(d->p_voice->glide_smoother, (d->p_voice->target_pitch));
       
        f_rmp_run_ramp(d->p_voice->pitch_env, (vals->pitch_env_amt));

#ifdef LMS_DEBUG_MAIN_LOOP
        if(is_debug_printing == 1)
                printf("d->p_voice->pitch_env->output_multiplied == %f\n", (d->p_voice->pitch_env->output_multiplied));
#endif        
        
        v_osc_set_unison_pitch(d->p_voice->osc_unison1, vals->master_uni_spread,   
                ((d->p_voice->glide_smoother->last_value) + (d->p_voice->pitch_env->output_multiplied) 
                + (vals->osc1pitch) + (vals->osc1tune) + (p->mono_modules->pitchbend_smoother->output)));

        
        v_osc_set_unison_pitch(d->p_voice->osc_unison2, vals->master_uni_spread, 
                ((d->p_voice->glide_smoother->last_value) + (d->p_voice->pitch_env->output_multiplied) 
                + (vals->osc2pitch) + (vals->osc2tune) +  + (p->mono_modules->pitchbend_smoother->output)));

#ifdef LMS_DEBUG_MAIN_LOOP
        if((is_debug_printing == 1) || ((d->p_voice->current_sample) > 1000)  || ((d->p_voice->current_sample) < -1000))
                printf("Before oscillators, d->p_voice->current_sample == %f\n", (d->p_voice->current_sample));
#endif        
        
        /*Run any oscillators, etc...*/
        d->p_voice->current_sample += f_osc_run_unison_osc(d->p_voice->osc_unison1) * (d->osc1_linamp);
#ifdef LMS_DEBUG_MAIN_LOOP
        if((is_debug_printing == 1) || ((d->p_voice->current_sample) > 1000)  || ((d->p_voice->current_sample) < -1000))
                printf("After osc1, d->p_voice->current_sample == %f\n", (d->p_voice->current_sample));
#endif        
        
        d->p_voice->current_sample += f_osc_run_unison_osc(d->p_voice->osc_unison2) * (d->osc2_linamp);
#ifdef LMS_DEBUG_MAIN_LOOP
        if((is_debug_printing == 1) || ((d->p_voice->current_sample) > 1000)  || ((d->p_voice->current_sample) < -1000))
                printf("After osc2, d->p_voice->current_sample == %f\n", (d->p_voice->current_sample));
#endif        
        
        d->p_voice->current_sample += (f_run_white_noise(d->p_voice->white_noise1) * (d->noise_linamp)); //white noise

#ifdef LMS_DEBUG_MAIN_LOOP
        if((is_debug_printing == 1) || ((d->p_voice->current_sample) > 1000)  || ((d->p_voice->current_sample) < -1000))
                printf("After white noise, d->p_voice->current_sample == %f\n", (d->p_voice->current_sample));
#endif                
        /*Run any processing of the initial result(s)*/      
        
        v_adsr_run(d->p_voice->adsr_amp);        
        
#ifdef LMS_DEBUG_MAIN_LOOP
        if(is_debug_printing == 1)
                printf("d->p_voice->adsr_amp->output == %f\n", (d->p_voice->adsr_amp->output));
#endif        
        
        v_adsr_run(d->p_voice->adsr_filter);

#ifdef LMS_DEBUG_MAIN_LOOP
        if(is_debug_printing == 1)
                printf("d->p_voice->adsr_filter->output == %f\n", (d->p_voice->adsr_filter->output));
#endif        
        
        v_svf_set_cutoff_base(d->p_voice->svf_filter,  (p->mono_modules->filter_smoother->output));//vals->timbre);
        //Run v_svf_add_cutoff_mod once for every input source
        v_svf_add_cutoff_mod(d->p_voice->svf_filter, ((d->p_voice->adsr_filter->output) * (vals->filter_env_amt)));        
        //calculate the cutoff
        v_svf_set_cutoff(d->p_voice->svf_filter);
        
        d->p_voice->filter_output = d->p_voice->svf_function(d->p_voice->svf_filter, (d->p_voice->current_sample));
        
#ifdef LMS_DEBUG_MAIN_LOOP
        if(is_debug_printing == 1)
                printf("output after svf == %f\n", (d->p_voice->current_sample));
#endif  
        
        /*Crossfade between the filter, and the filter run through the distortion unit*/
        d->p_voice->current_sample = f_axf_run_xfade((d->p_voice->dist_dry_wet), (d->p_voice->filter_output), 
                f_clp_clip(d->p_voice->clipper1, (d->p_voice->filter_output)));

#ifdef LMS_DEBUG_MAIN_LOOP
        if(is_debug_printing == 1)
                printf("output after clipper == %f\n", (d->p_voice->current_sample));
#endif  
        
        /*Run the envelope and assign to the output buffer*/
        out[(d->i_voice)] += (d->p_voice->current_sample) * (f_linear_to_db_linear((d->p_voice->adsr_amp->output))) * (d->amp) ; 
                
        d->i_voice = (d->i_voice) + 1;
        /*End LibModSynth modifications*/
    }

    
    /*If the main ADSR envelope has reached it's release stage, kill the voice.
     However, you don't have to necessarily have to kill the voice, but you will waste a lot of CPU if you don't*/
    if(d->p_voice->adsr_amp->stage == 4)
    {
        d->n_state = off;        
    }
        
}

/*This returns MIDI CCs for the different knobs
 TODO:  Try it with non-hex numbers*/
/*GUI Step 16:  Assign the LADSPA ports defined in step 9 to MIDI CCs in getControllerLTS*/
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
        return DSSI_CC(0x29);  //41  
    case LMS_OSC2_PITCH:
        return DSSI_CC(0x21);  //33
    case LMS_OSC2_TUNE:
        return DSSI_CC(0x22);  //34
    case LMS_OSC2_VOLUME:
        return DSSI_CC(0x23);  //35            
    case LMS_MASTER_VOLUME:        
        return DSSI_CC(0x24);  //36        
    case LMS_MASTER_UNISON_VOICES:        
        return DSSI_CC(0x25);  //37        
    case LMS_MASTER_UNISON_SPREAD:        
        return DSSI_CC(0x26);  //38        
    case LMS_MASTER_GLIDE:        
        return DSSI_CC(0x27);  //39        
    case LMS_MASTER_PITCHBEND_AMT:        
        return DSSI_CC(0x28);  //40
    case LMS_PITCH_ENV_AMT:
        return DSSI_CC(0x2a); //42
    case LMS_PITCH_ENV_TIME:
        return DSSI_CC(0x2b); //43     
    case LMS_PROGRAM_CHANGE:
        return DSSI_CC(0x20);  //32  -  Bank Select fine (this may be the wrong use of that CC)
    }

    return DSSI_NONE;
}

/* Original comment:  find the voice that is least relevant (low note priority)*/
/*libmodsynth comment:  *data is an array of voices in an LMS struct,
 iterate through them for a free voice, or if one cannot be found,
 pick the highest voice*/
int pick_voice(const voice_data *data, int a_current_note)
{
    unsigned int f_i;
    int highest_note = 0;
    int highest_note_voice = 0;
    
    /*Look for the voice being played by the current note.
     It's more musical to kill the same note than to let it play twice,
     guitars, pianos, etc... work that way.  It also helps to prevent hung notes*/    
    for (f_i=0; f_i<POLYPHONY; f_i++) {
	if (data[f_i].note == a_current_note) {
#ifdef LMS_DEBUG_NOTE            
            printf("pick_voice found current_note:  %i\n", f_i);
#endif
            v_adsr_set_fast_release(data->p_voice->adsr_amp);
            break;
	    //return f_i; 
	}
    }
    
    /* Look for an inactive voice */
    for (f_i=0; f_i<POLYPHONY; f_i++) {
	if (data[f_i].n_state == off) {
#ifdef LMS_DEBUG_NOTE            
            printf("pick_voice found inactive voice:  %i\n", f_i);
#endif
	    return f_i;
	}
    }

    /* otherwise find for the highest note and replace that */
    for (f_i=0; f_i<POLYPHONY; f_i++) {
	if (data[f_i].note > highest_note) {
	    highest_note = data[f_i].note;
	    highest_note_voice = f_i;
	}
    }

#ifdef LMS_DEBUG_NOTE            
            printf("pick_voice found highest voice:  %i\n", highest_note_voice);
#endif
            
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
	//ltsLDescriptor->UniqueID = 24;  //Default, don't use this
        ltsLDescriptor->UniqueID = 1337721;  //Arbitrary number I made up, somewhat near the upper end of allowable UIDs
	ltsLDescriptor->Label = "LMS";  //Changing this breaks the plugin, it compiles, but hangs when trying to run.  TODO:  investigate
	ltsLDescriptor->Properties = 0;
	ltsLDescriptor->Name = "Ray-V (Powered by LibModSynth)";
	ltsLDescriptor->Maker = "Jeff Hubbard <jhubbard651@users.sf.net>";
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
	port_range_hints[LMS_ATTACK].LowerBound = 1; 
	port_range_hints[LMS_ATTACK].UpperBound = 100; 

	/* Parameters for decay */
	port_descriptors[LMS_DECAY] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_DECAY] = "Decay time (s)";
	port_range_hints[LMS_DECAY].HintDescriptor =
			port_range_hints[LMS_ATTACK].HintDescriptor;
	port_range_hints[LMS_DECAY].LowerBound = 1; 
	port_range_hints[LMS_DECAY].UpperBound = 100; 

	/* Parameters for sustain */
	port_descriptors[LMS_SUSTAIN] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_SUSTAIN] = "Sustain level (%)";
	port_range_hints[LMS_SUSTAIN].HintDescriptor =
			LADSPA_HINT_DEFAULT_HIGH |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_SUSTAIN].LowerBound = -60;
	port_range_hints[LMS_SUSTAIN].UpperBound = 0;

	/* Parameters for release */
	port_descriptors[LMS_RELEASE] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_RELEASE] = "Release time (s)";
	port_range_hints[LMS_RELEASE].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_LOGARITHMIC |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_RELEASE].LowerBound = 1; 
	port_range_hints[LMS_RELEASE].UpperBound = 400; 

	/* Parameters for timbre */
	port_descriptors[LMS_TIMBRE] = port_descriptors[LMS_ATTACK];
	port_names[LMS_TIMBRE] = "Timbre";
	port_range_hints[LMS_TIMBRE].HintDescriptor =
			LADSPA_HINT_DEFAULT_HIGH |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_TIMBRE].LowerBound =  20;
	port_range_hints[LMS_TIMBRE].UpperBound =  124;
        
        /* Parameters for res */
	port_descriptors[LMS_RES] = port_descriptors[LMS_ATTACK];
	port_names[LMS_RES] = "Res";
	port_range_hints[LMS_RES].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_RES].LowerBound =  -30;
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
	port_range_hints[LMS_FILTER_ATTACK].LowerBound = 1; 
	port_range_hints[LMS_FILTER_ATTACK].UpperBound = 100; 

	/* Parameters for decay_f */
	port_descriptors[LMS_FILTER_DECAY] = port_descriptors[LMS_ATTACK];
	port_names[LMS_FILTER_DECAY] = "Decay time (s) filter";
	port_range_hints[LMS_FILTER_DECAY].HintDescriptor =
			port_range_hints[LMS_ATTACK].HintDescriptor;
	port_range_hints[LMS_FILTER_DECAY].LowerBound = 1;
	port_range_hints[LMS_FILTER_DECAY].UpperBound = 100;

	/* Parameters for sustain_f */
	port_descriptors[LMS_FILTER_SUSTAIN] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_FILTER_SUSTAIN] = "Sustain level (%) filter";
	port_range_hints[LMS_FILTER_SUSTAIN].HintDescriptor =
			LADSPA_HINT_DEFAULT_HIGH |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_FILTER_SUSTAIN].LowerBound = 0; 
	port_range_hints[LMS_FILTER_SUSTAIN].UpperBound = 100; 
        
	/* Parameters for release_f */
	port_descriptors[LMS_FILTER_RELEASE] = port_descriptors[LMS_ATTACK];
	port_names[LMS_FILTER_RELEASE] = "Release time (s) filter";
	port_range_hints[LMS_FILTER_RELEASE].HintDescriptor =
			LADSPA_HINT_DEFAULT_LOW  |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_FILTER_RELEASE].LowerBound = 1; 
	port_range_hints[LMS_FILTER_RELEASE].UpperBound = 400; 

        
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
	port_names[LMS_DIST_WET] = "Dist Wet";
	port_range_hints[LMS_DIST_WET].HintDescriptor =
			//LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_DIST_WET].LowerBound =  0; 
	port_range_hints[LMS_DIST_WET].UpperBound =  100;
        
        
        /*Parameters for osc1type*/        
	port_descriptors[LMS_OSC1_TYPE] = port_descriptors[LMS_ATTACK];
	port_names[LMS_OSC1_TYPE] = "Osc 1 Type";
	port_range_hints[LMS_OSC1_TYPE].HintDescriptor =
			//LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_OSC1_TYPE].LowerBound =  0;
	port_range_hints[LMS_OSC1_TYPE].UpperBound =  5;
        
        
        /*Parameters for osc1pitch*/        
	port_descriptors[LMS_OSC1_PITCH] = port_descriptors[LMS_ATTACK];
	port_names[LMS_OSC1_PITCH] = "Osc 1 Pitch";
	port_range_hints[LMS_OSC1_PITCH].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_OSC1_PITCH].LowerBound =  -12;
	port_range_hints[LMS_OSC1_PITCH].UpperBound =  12;
        
        
        /*Parameters for osc1tune*/        
	port_descriptors[LMS_OSC1_TUNE] = port_descriptors[LMS_ATTACK];
	port_names[LMS_OSC1_TUNE] = "Osc 1 Tune";
	port_range_hints[LMS_OSC1_TUNE].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_OSC1_TUNE].LowerBound = -100;
	port_range_hints[LMS_OSC1_TUNE].UpperBound =  100;
        
        
        /*Parameters for osc1vol*/        
	port_descriptors[LMS_OSC1_VOLUME] = port_descriptors[LMS_ATTACK];
	port_names[LMS_OSC1_VOLUME] = "Osc 1 Vol";
	port_range_hints[LMS_OSC1_VOLUME].HintDescriptor =
			//LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_OSC1_VOLUME].LowerBound =  -60;
	port_range_hints[LMS_OSC1_VOLUME].UpperBound =  0;
        
        
        
        /*Parameters for osc2type*/        
	port_descriptors[LMS_OSC2_TYPE] = port_descriptors[LMS_ATTACK];
	port_names[LMS_OSC2_TYPE] = "Osc 2 Type";
	port_range_hints[LMS_OSC2_TYPE].HintDescriptor =
			LADSPA_HINT_DEFAULT_MAXIMUM |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_OSC2_TYPE].LowerBound =  0;
	port_range_hints[LMS_OSC2_TYPE].UpperBound =  4;
        
        
        /*Parameters for osc2pitch*/        
	port_descriptors[LMS_OSC2_PITCH] = port_descriptors[LMS_ATTACK];
	port_names[LMS_OSC2_PITCH] = "Osc 2 Pitch";
	port_range_hints[LMS_OSC2_PITCH].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_OSC2_PITCH].LowerBound =  -12;
	port_range_hints[LMS_OSC2_PITCH].UpperBound =  12;
        
        
        /*Parameters for osc2tune*/        
	port_descriptors[LMS_OSC2_TUNE] = port_descriptors[LMS_ATTACK];
	port_names[LMS_OSC2_TUNE] = "Osc 2 Tune";
	port_range_hints[LMS_OSC2_TUNE].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_OSC2_TUNE].LowerBound = -100;
	port_range_hints[LMS_OSC2_TUNE].UpperBound = 100; 
        
        
        /*Parameters for osc2vol*/        
	port_descriptors[LMS_OSC2_VOLUME] = port_descriptors[LMS_ATTACK];
	port_names[LMS_OSC2_VOLUME] = "Osc 2 Vol";
	port_range_hints[LMS_OSC2_VOLUME].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW |  LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_OSC2_VOLUME].LowerBound =  -60;
	port_range_hints[LMS_OSC2_VOLUME].UpperBound =  0;
        
        
        /*Parameters for master vol*/        
	port_descriptors[LMS_MASTER_VOLUME] = port_descriptors[LMS_ATTACK];
	port_names[LMS_MASTER_VOLUME] = "Master Vol";
	port_range_hints[LMS_MASTER_VOLUME].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_MASTER_VOLUME].LowerBound =  -60;
	port_range_hints[LMS_MASTER_VOLUME].UpperBound =  12;
        
        
        
        /*Parameters for master unison voices*/        
	port_descriptors[LMS_MASTER_UNISON_VOICES] = port_descriptors[LMS_ATTACK];
	port_names[LMS_MASTER_UNISON_VOICES] = "Master Unison";
	port_range_hints[LMS_MASTER_UNISON_VOICES].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_MASTER_UNISON_VOICES].LowerBound =  1;
	port_range_hints[LMS_MASTER_UNISON_VOICES].UpperBound =  7;
        
        
        /*Parameters for master unison spread*/        
	port_descriptors[LMS_MASTER_UNISON_SPREAD] = port_descriptors[LMS_ATTACK];
	port_names[LMS_MASTER_UNISON_SPREAD] = "Master Unison Spread";
	port_range_hints[LMS_MASTER_UNISON_SPREAD].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_MASTER_UNISON_SPREAD].LowerBound =  0;
	port_range_hints[LMS_MASTER_UNISON_SPREAD].UpperBound =  100;
        
        
        /*Parameters for master glide*/        
	port_descriptors[LMS_MASTER_GLIDE] = port_descriptors[LMS_ATTACK];
	port_names[LMS_MASTER_GLIDE] = "Master Glide";
	port_range_hints[LMS_MASTER_GLIDE].HintDescriptor =
			LADSPA_HINT_DEFAULT_MINIMUM |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_MASTER_GLIDE].LowerBound =  0;
	port_range_hints[LMS_MASTER_GLIDE].UpperBound =  200;
        
        
        /*Parameters for master pitchbend amt*/        
	port_descriptors[LMS_MASTER_PITCHBEND_AMT] = port_descriptors[LMS_ATTACK];
	port_names[LMS_MASTER_PITCHBEND_AMT] = "Pitchbend Amt";
	port_range_hints[LMS_MASTER_PITCHBEND_AMT].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_MASTER_PITCHBEND_AMT].LowerBound =  1;
	port_range_hints[LMS_MASTER_PITCHBEND_AMT].UpperBound =  36;
        
        
        /*Parameters for pitch env amt*/        
	port_descriptors[LMS_PITCH_ENV_AMT] = port_descriptors[LMS_ATTACK];
	port_names[LMS_PITCH_ENV_AMT] = "Pitch Env Amt";
	port_range_hints[LMS_PITCH_ENV_AMT].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PITCH_ENV_AMT].LowerBound =  -36;
	port_range_hints[LMS_PITCH_ENV_AMT].UpperBound =   36;
        
        
        /*Parameters for pitch env time*/        
	port_descriptors[LMS_PITCH_ENV_TIME] = port_descriptors[LMS_ATTACK];
	port_names[LMS_PITCH_ENV_TIME] = "Pitch Env Time";
	port_range_hints[LMS_PITCH_ENV_TIME].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PITCH_ENV_TIME].LowerBound = 0; 
	port_range_hints[LMS_PITCH_ENV_TIME].UpperBound = 200;
                
        
        /*Parameters for program change*/        
	port_descriptors[LMS_PROGRAM_CHANGE] = port_descriptors[LMS_ATTACK];
	port_names[LMS_PROGRAM_CHANGE] = "Program Change";
	port_range_hints[LMS_PROGRAM_CHANGE].HintDescriptor =
			LADSPA_HINT_DEFAULT_MINIMUM | LADSPA_HINT_BOUNDED_BELOW | 
                        LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PROGRAM_CHANGE].LowerBound = 0; 
	port_range_hints[LMS_PROGRAM_CHANGE].UpperBound = 127;  // > 127 loads the first preset
        
        /*Step 17:  Add LADSPA ports*/
        
        
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
