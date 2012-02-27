/* 
 * File:   synth.h
 * Author: Jeff Hubbard
 *
 * Created on February 26, 2012, 8:48 PM
 */

#ifndef SYNTH_H
#define	SYNTH_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "ladspa.h"
#include "libmodsynth.h"
    
//Comment this out when you compile a stable release version on the plugin, you don't want it printing debug output unless you're debugging
//#define LMS_DEBUG_MODE_QT
/*This is used for things like naming the preset file, etc...*/
#define LMS_PLUGIN_NAME "ray-v"

    /*Comment these out when compiling a stable, production-ready plugin.  
 The debugging code wastes a lot of CPU, and end users don't really need to see it*/
//#define LMS_DEBUG_NOTE
//#define LMS_DEBUG_MAIN_LOOP

/*Then you can print debug information like this:
#ifdef LMS_DEBUG_NOTE
printf("debug information");
#endif
*/
    
    
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
    
    float sv_pitch_bend_value;
    float sv_last_note;  //For glide
} LTS;




#ifdef	__cplusplus
}
#endif

#endif	/* SYNTH_H */

