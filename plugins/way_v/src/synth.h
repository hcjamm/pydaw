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
    
#define WAYV_OUTPUT0  0
#define WAYV_OUTPUT1  1
    
#define WAYV_FIRST_CONTROL_PORT 2
#define WAYV_ATTACK  2
#define WAYV_DECAY   3
#define WAYV_SUSTAIN 4
#define WAYV_RELEASE 5
#define WAYV_NOISE_AMP 6
#define WAYV_OSC1_TYPE 7
#define WAYV_OSC1_PITCH 8
#define WAYV_OSC1_TUNE 9
#define WAYV_OSC1_VOLUME 10
#define WAYV_OSC2_TYPE 11
#define WAYV_OSC2_PITCH 12
#define WAYV_OSC2_TUNE 13
#define WAYV_OSC2_VOLUME 14
#define WAYV_MASTER_VOLUME 15
#define WAYV_MASTER_UNISON_VOICES 16
#define WAYV_MASTER_UNISON_SPREAD 17
#define WAYV_MASTER_GLIDE 18
#define WAYV_MASTER_PITCHBEND_AMT 19
#define WAYV_LAST_CONTROL_PORT 19
#define WAYV_COUNT 20 /* must be 1 + highest value above CHANGE THIS IF YOU ADD OR TAKE AWAY ANYTHING*/
    
#define WAYV_PROGRAM_CHANGE 21  //Not used as a real port
#define WAYV_POLYPHONY   16

typedef struct {
    LADSPA_Data *output0;
    LADSPA_Data *output1;
    LADSPA_Data *tune;
    LADSPA_Data *attack;
    LADSPA_Data *decay;
    LADSPA_Data *sustain;
    LADSPA_Data *release;    
    LADSPA_Data pitch;
        
    LADSPA_Data *osc1pitch;
    LADSPA_Data *osc1tune;
    LADSPA_Data *osc1type;
    LADSPA_Data *osc1vol;
    
    LADSPA_Data *osc2pitch;
    LADSPA_Data *osc2tune;
    LADSPA_Data *osc2type;
    LADSPA_Data *osc2vol;
    
    LADSPA_Data *master_vol;
    
    LADSPA_Data *noise_amp;
        
    LADSPA_Data *master_uni_voice;
    LADSPA_Data *master_uni_spread;
    LADSPA_Data *master_glide;
    LADSPA_Data *master_pb_amt;
    
    LADSPA_Data *program;
    
    t_ccm_midi_cc_map * midi_cc_map;
    
    t_rayv_poly_voice * data[WAYV_POLYPHONY];
    t_voc_voices * voices;
    
    long         sampleNo;
    
    float fs;    
    t_rayv_mono_modules * mono_modules;
        
    int event_pos;
    int i_run_poly_voice;
    int i_iterator;    
    
    float sv_pitch_bend_value;
    float sv_last_note;  //For glide
} t_wayv;




#ifdef	__cplusplus
}
#endif

#endif	/* RAY_VSYNTH_H */

