/* 
 * File:   synth.h
 * Author: Jeff Hubbard
 *
 */

#ifndef RAYV_SYNTH_H
#define	RAYV_SYNTH_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "ladspa.h"
#include "libmodsynth.h"
#include "../../libmodsynth/lib/voice.h"
#include "../../libmodsynth/lib/cc_map.h"
    
#define LMS_OUTPUT0  0
#define LMS_OUTPUT1  1
    
#define LMS_FIRST_CONTROL_PORT 2
#define LMS_ATTACK  2
#define LMS_DECAY   3
#define LMS_SUSTAIN 4
#define LMS_RELEASE 5
#define LMS_TIMBRE  6
#define LMS_RES  7
#define LMS_DIST 8
#define LMS_FILTER_ATTACK  9
#define LMS_FILTER_DECAY   10
#define LMS_FILTER_SUSTAIN 11
#define LMS_FILTER_RELEASE 12
#define LMS_NOISE_AMP 13
#define LMS_FILTER_ENV_AMT 14
#define LMS_DIST_WET 15
#define LMS_OSC1_TYPE 16
#define LMS_OSC1_PITCH 17
#define LMS_OSC1_TUNE 18
#define LMS_OSC1_VOLUME 19
#define LMS_OSC2_TYPE 20
#define LMS_OSC2_PITCH 21
#define LMS_OSC2_TUNE 22
#define LMS_OSC2_VOLUME 23
#define LMS_MASTER_VOLUME 24
#define LMS_MASTER_UNISON_VOICES 25
#define LMS_MASTER_UNISON_SPREAD 26
#define LMS_MASTER_GLIDE 27
#define LMS_MASTER_PITCHBEND_AMT 28
#define LMS_PITCH_ENV_TIME 29
#define LMS_PITCH_ENV_AMT 30
#define LMS_LFO_FREQ 31
#define LMS_LFO_TYPE 32
#define LMS_LFO_AMP 33
#define LMS_LFO_PITCH 34
#define LMS_LFO_FILTER 35
#define LMS_LAST_CONTROL_PORT 35
#define LMS_COUNT 36 /* must be 1 + highest value above CHANGE THIS IF YOU ADD OR TAKE AWAY ANYTHING*/

    
#define LMS_PROGRAM_CHANGE 37  //Not used as a real port
#define RAYV_POLYPHONY   16

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
    
    t_rayv_poly_voice * data[RAYV_POLYPHONY];
    t_voc_voices * voices;
    
    //long         ons[VOICES_MAX_MIDI_NOTE_NUMBER];
    //long         offs[VOICES_MAX_MIDI_NOTE_NUMBER];
    long         sampleNo;
    
    float fs;    
    t_rayv_mono_modules * mono_modules;
        
    int event_pos;
    int i_run_poly_voice;
    int i_iterator;    
    
    float sv_pitch_bend_value;
    float sv_last_note;  //For glide
} t_rayv;




#ifdef	__cplusplus
}
#endif

#endif	/* RAY_VSYNTH_H */

