/* 
 * File:   synth.h
 * Author: Jeff Hubbard
 *
 */

#ifndef WAYV_SYNTH_H
#define	WAYV_SYNTH_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "../../include/pydaw3/pydaw_plugin.h"
#include "libmodsynth.h"
#include "../../libmodsynth/lib/voice.h"
    
//Total number of LFOs, ADSRs, other envelopes, etc...  Used for the PolyFX mod matrix
#define WAYV_MODULATOR_COUNT 4
//How many modular PolyFX
#define WAYV_MODULAR_POLYFX_COUNT 4
//How many ports per PolyFX, 3 knobs and a combobox
#define WAYV_PORTS_PER_MOD_EFFECT 4
//How many knobs per PolyFX, 3 knobs
#define WAYV_CONTROLS_PER_MOD_EFFECT 3
//How many groups of effects.  This will become useful when each sample has an "effects group" choice  
//EDIT:  This may or may not ever come to fruition with my new strategy.  Delete this and re-arrange everywhere it's used...
#define WAYV_EFFECTS_GROUPS_COUNT 1

    
#define WAYV_OUTPUT0  0
#define WAYV_OUTPUT1  1
    
#define WAYV_FIRST_CONTROL_PORT 2
#define WAYV_ATTACK_MAIN  2
#define WAYV_DECAY_MAIN   3
#define WAYV_SUSTAIN_MAIN 4
#define WAYV_RELEASE_MAIN 5
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
#define WAYV_OSC1_UNISON_VOICES 16
#define WAYV_OSC1_UNISON_SPREAD 17
#define WAYV_MASTER_GLIDE 18
#define WAYV_MASTER_PITCHBEND_AMT 19
#define WAYV_ATTACK1  20
#define WAYV_DECAY1   21
#define WAYV_SUSTAIN1 22
#define WAYV_RELEASE1 23
#define WAYV_ATTACK2  24
#define WAYV_DECAY2   25
#define WAYV_SUSTAIN2 26
#define WAYV_RELEASE2 27
    
    
    
#define WAYV_ATTACK_PFX1  28
#define WAYV_DECAY_PFX1   29
#define WAYV_SUSTAIN_PFX1 30
#define WAYV_RELEASE_PFX1 31
#define WAYV_ATTACK_PFX2  32
#define WAYV_DECAY_PFX2   33
#define WAYV_SUSTAIN_PFX2 34
#define WAYV_RELEASE_PFX2 35
#define LMS_NOISE_TYPE 36
#define WAYV_RAMP_ENV_TIME 37
#define WAYV_LFO_FREQ 38
#define WAYV_LFO_TYPE 39
//From Modulex
#define WAYV_FX0_KNOB0  40
#define WAYV_FX0_KNOB1 41
#define WAYV_FX0_KNOB2  42
#define WAYV_FX0_COMBOBOX 43
#define WAYV_FX1_KNOB0  44
#define WAYV_FX1_KNOB1  45
#define WAYV_FX1_KNOB2  46
#define WAYV_FX1_COMBOBOX 47
#define WAYV_FX2_KNOB0  48
#define WAYV_FX2_KNOB1  49
#define WAYV_FX2_KNOB2  50
#define WAYV_FX2_COMBOBOX 51
#define WAYV_FX3_KNOB0  52
#define WAYV_FX3_KNOB1  53
#define WAYV_FX3_KNOB2  54
#define WAYV_FX3_COMBOBOX 55
//PolyFX Mod Matrix
#define WAVV_PFXMATRIX_FIRST_PORT 56

#define WAVV_PFXMATRIX_GRP0DST0SRC0CTRL0  56
#define WAVV_PFXMATRIX_GRP0DST0SRC0CTRL1  57
#define WAVV_PFXMATRIX_GRP0DST0SRC0CTRL2  58
#define WAVV_PFXMATRIX_GRP0DST0SRC1CTRL0  59
#define WAVV_PFXMATRIX_GRP0DST0SRC1CTRL1  60
#define WAVV_PFXMATRIX_GRP0DST0SRC1CTRL2  61
#define WAVV_PFXMATRIX_GRP0DST0SRC2CTRL0  62
#define WAVV_PFXMATRIX_GRP0DST0SRC2CTRL1  63
#define WAVV_PFXMATRIX_GRP0DST0SRC2CTRL2  64
#define WAVV_PFXMATRIX_GRP0DST0SRC3CTRL0  65
#define WAVV_PFXMATRIX_GRP0DST0SRC3CTRL1  66
#define WAVV_PFXMATRIX_GRP0DST0SRC3CTRL2  67
#define WAVV_PFXMATRIX_GRP0DST1SRC0CTRL0  68
#define WAVV_PFXMATRIX_GRP0DST1SRC0CTRL1  69
#define WAVV_PFXMATRIX_GRP0DST1SRC0CTRL2  70
#define WAVV_PFXMATRIX_GRP0DST1SRC1CTRL0  71
#define WAVV_PFXMATRIX_GRP0DST1SRC1CTRL1  72
#define WAVV_PFXMATRIX_GRP0DST1SRC1CTRL2  73
#define WAVV_PFXMATRIX_GRP0DST1SRC2CTRL0  74
#define WAVV_PFXMATRIX_GRP0DST1SRC2CTRL1  75
#define WAVV_PFXMATRIX_GRP0DST1SRC2CTRL2  76
#define WAVV_PFXMATRIX_GRP0DST1SRC3CTRL0  77
#define WAVV_PFXMATRIX_GRP0DST1SRC3CTRL1  78
#define WAVV_PFXMATRIX_GRP0DST1SRC3CTRL2  79
#define WAVV_PFXMATRIX_GRP0DST2SRC0CTRL0  80
#define WAVV_PFXMATRIX_GRP0DST2SRC0CTRL1  81
#define WAVV_PFXMATRIX_GRP0DST2SRC0CTRL2  82
#define WAVV_PFXMATRIX_GRP0DST2SRC1CTRL0  83
#define WAVV_PFXMATRIX_GRP0DST2SRC1CTRL1  84
#define WAVV_PFXMATRIX_GRP0DST2SRC1CTRL2  85
#define WAVV_PFXMATRIX_GRP0DST2SRC2CTRL0  86
#define WAVV_PFXMATRIX_GRP0DST2SRC2CTRL1  87
#define WAVV_PFXMATRIX_GRP0DST2SRC2CTRL2  88
#define WAVV_PFXMATRIX_GRP0DST2SRC3CTRL0  89
#define WAVV_PFXMATRIX_GRP0DST2SRC3CTRL1  90
#define WAVV_PFXMATRIX_GRP0DST2SRC3CTRL2  91
#define WAVV_PFXMATRIX_GRP0DST3SRC0CTRL0  92
#define WAVV_PFXMATRIX_GRP0DST3SRC0CTRL1  93
#define WAVV_PFXMATRIX_GRP0DST3SRC0CTRL2  94
#define WAVV_PFXMATRIX_GRP0DST3SRC1CTRL0  95
#define WAVV_PFXMATRIX_GRP0DST3SRC1CTRL1  96
#define WAVV_PFXMATRIX_GRP0DST3SRC1CTRL2  97
#define WAVV_PFXMATRIX_GRP0DST3SRC2CTRL0  98
#define WAVV_PFXMATRIX_GRP0DST3SRC2CTRL1  99
#define WAVV_PFXMATRIX_GRP0DST3SRC2CTRL2  100
#define WAVV_PFXMATRIX_GRP0DST3SRC3CTRL0  101
#define WAVV_PFXMATRIX_GRP0DST3SRC3CTRL1  102
#define WAVV_PFXMATRIX_GRP0DST3SRC3CTRL2  103

//End PolyFX Mod Matrix

#define WAYV_ADSR1_CHECKBOX 105
#define WAYV_ADSR2_CHECKBOX 106
#define WAYV_LFO_AMP 107
#define WAYV_LFO_PITCH 108
#define WAYV_PITCH_ENV_AMT 109
#define WAYV_OSC2_UNISON_VOICES 110
#define WAYV_OSC2_UNISON_SPREAD 111
#define WAYV_LFO_AMOUNT 112
    
#define WAYV_LAST_CONTROL_PORT 112
#define WAYV_COUNT 113 /* must be 1 + highest value above CHANGE THIS IF YOU ADD OR TAKE AWAY ANYTHING*/
    
#define WAYV_POLYPHONY   16

typedef struct {
    LADSPA_Data *output0;
    LADSPA_Data *output1;
    //LADSPA_Data *tune;
    LADSPA_Data *attack_main;
    LADSPA_Data *decay_main;
    LADSPA_Data *sustain_main;
    LADSPA_Data *release_main;    
    
    LADSPA_Data *attack1;
    LADSPA_Data *decay1;
    LADSPA_Data *sustain1;
    LADSPA_Data *release1;    
    
    LADSPA_Data *attack2;
    LADSPA_Data *decay2;
    LADSPA_Data *sustain2;
    LADSPA_Data *release2;    
    
    //LADSPA_Data pitch;
        
    LADSPA_Data *osc1pitch;
    LADSPA_Data *osc1tune;
    LADSPA_Data *osc1type;
    LADSPA_Data *osc1vol;
    
    LADSPA_Data *osc2pitch;
    LADSPA_Data *osc2tune;
    LADSPA_Data *osc2type;
    LADSPA_Data *osc2vol;
    
    LADSPA_Data *master_vol;
    
    LADSPA_Data *attack;
    LADSPA_Data *decay;
    LADSPA_Data *sustain;
    LADSPA_Data *release;
        
    LADSPA_Data *attack_f;
    LADSPA_Data *decay_f;
    LADSPA_Data *sustain_f;
    LADSPA_Data *release_f;
    
    LADSPA_Data *noise_amp;
    LADSPA_Data *noise_type;
        
    LADSPA_Data *osc1_uni_voice;
    LADSPA_Data *osc1_uni_spread;
    LADSPA_Data *osc2_uni_voice;
    LADSPA_Data *osc2_uni_spread;
    
    
    LADSPA_Data *master_glide;
    LADSPA_Data *master_pb_amt;
        
    LADSPA_Data *pitch_env_time;
    LADSPA_Data *pitch_env_amt;
    
    LADSPA_Data *lfo_freq;
    LADSPA_Data *lfo_type;
    
    LADSPA_Data * adsr1_checked;
    LADSPA_Data * adsr2_checked;
    
    LADSPA_Data *lfo_amp;
    LADSPA_Data *lfo_pitch;
    LADSPA_Data *lfo_amount;
        
       
    //Corresponds to the actual knobs on the effects themselves, not the mod matrix
    LADSPA_Data *pfx_mod_knob[WAYV_EFFECTS_GROUPS_COUNT][WAYV_MODULAR_POLYFX_COUNT][WAYV_CONTROLS_PER_MOD_EFFECT];
    
    LADSPA_Data *fx_combobox[WAYV_EFFECTS_GROUPS_COUNT][WAYV_MODULAR_POLYFX_COUNT];
        
    //PolyFX Mod Matrix
    //Corresponds to the mod matrix spinboxes
    LADSPA_Data *polyfx_mod_matrix[WAYV_EFFECTS_GROUPS_COUNT][WAYV_MODULAR_POLYFX_COUNT][WAYV_MODULATOR_COUNT][WAYV_CONTROLS_PER_MOD_EFFECT];
    
    int active_polyfx[WAYV_POLYPHONY][WAYV_MODULAR_POLYFX_COUNT];
    int active_polyfx_count[WAYV_POLYPHONY];
        
    int polyfx_mod_ctrl_indexes[WAYV_POLYPHONY][WAYV_MODULAR_POLYFX_COUNT][(WAYV_CONTROLS_PER_MOD_EFFECT * WAYV_MODULATOR_COUNT)]; //The index of the control to mod, currently 0-2
    int polyfx_mod_counts[WAYV_POLYPHONY][WAYV_MODULAR_POLYFX_COUNT];  //How many polyfx_mod_ptrs to iterate through for the current note
    int polyfx_mod_src_index[WAYV_POLYPHONY][WAYV_MODULAR_POLYFX_COUNT][(WAYV_CONTROLS_PER_MOD_EFFECT * WAYV_MODULATOR_COUNT)];  //The index of the modulation source(LFO, ADSR, etc...) to multiply by
    float polyfx_mod_matrix_values[WAYV_POLYPHONY][WAYV_MODULAR_POLYFX_COUNT][(WAYV_CONTROLS_PER_MOD_EFFECT * WAYV_MODULATOR_COUNT)];  //The value of the mod_matrix knob, multiplied by .01
        
    t_wayv_poly_voice * data[WAYV_POLYPHONY];
    t_voc_voices * voices;
    
    long         sampleNo;
    
    float fs;    
    t_wayv_mono_modules * mono_modules;
        
    int event_pos;
    int i_run_poly_voice;
    int i_iterator;    
    
    float sv_pitch_bend_value;
    float sv_last_note;  //For glide
    
    int i_fx_grps;
    int i_dst;
    int i_src;    
    int i_ctrl;
} t_wayv;




#ifdef	__cplusplus
}
#endif

#endif	/* WAYV_SYNTH_H */

