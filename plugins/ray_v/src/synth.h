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
#include "../../libmodsynth/lib/voice.h"
#include "../../libmodsynth/lib/cc_map.h"
    
/*Comment these out when compiling a stable, production-ready plugin.  
 The debugging code wastes a lot of CPU, and end users don't really need to see it*/
//#define LMS_DEBUG_NOTE
//#define LMS_DEBUG_MAIN_LOOP
//#define LMS_DEBUG_MODE_QT


/*Then you can print debug information like this:
#ifdef LMS_DEBUG_NOTE
printf("debug information");
#endif
*/
    
#define LMS_OUTPUT0  0
#define LMS_OUTPUT1  36
/*Add ports to the main synthesizer file that the GUI can talk to */
    
/*LMS_FIRST_CONTROL_PORT is the first port used for controls such as knobs.  All control ports must be numbered continuously,
 as they are iterated through*/
#define LMS_FIRST_CONTROL_PORT 1    
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
#define LMS_LFO_FREQ 31
#define LMS_LFO_TYPE 32
#define LMS_LFO_AMP 33
#define LMS_LFO_PITCH 34
#define LMS_LFO_FILTER 35
/*This is the last control port*/
#define LMS_LAST_CONTROL_PORT 35
#define LMS_COUNT 37 /* must be 1 + highest value above CHANGE THIS IF YOU ADD OR TAKE AWAY ANYTHING*/


#define POLYPHONY   8  //maximum voices played at one time
#define STEP_SIZE   16
    
    
/*Add a variable for each control in the synth_vals type*/
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
    
    LADSPA_Data lfo_freq;
    LADSPA_Data lfo_type;
    LADSPA_Data lfo_amp;
    LADSPA_Data lfo_pitch;
    LADSPA_Data lfo_filter;
    
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




/*Add a variable for each control in the LMS type*/
typedef struct {
    LADSPA_Data *output0;
    LADSPA_Data *output1;
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
    
    LADSPA_Data *lfo_freq;
    LADSPA_Data *lfo_type;
    LADSPA_Data *lfo_amp;
    LADSPA_Data *lfo_pitch;
    LADSPA_Data *lfo_filter;
    
    LADSPA_Data *program;
    
    t_ccm_midi_cc_map * midi_cc_map;
    
    t_poly_voice * data[POLYPHONY];
    t_voc_voices * voices;
    
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
} LMS;




#ifdef	__cplusplus
}
#endif

#endif	/* SYNTH_H */

