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
#define WAYV_OSC3_TYPE 113
#define WAYV_OSC3_PITCH 114
#define WAYV_OSC3_TUNE 115
#define WAYV_OSC3_VOLUME 116
#define WAYV_OSC3_UNISON_VOICES 117
#define WAYV_OSC3_UNISON_SPREAD 118
#define WAYV_OSC1_FM1 119
#define WAYV_OSC1_FM2 120
#define WAYV_OSC1_FM3 121
#define WAYV_OSC2_FM1 122
#define WAYV_OSC2_FM2 123
#define WAYV_OSC2_FM3 124
#define WAYV_OSC3_FM1 125 
#define WAYV_OSC3_FM2 126
#define WAYV_OSC3_FM3 127
#define WAYV_ATTACK3  128
#define WAYV_DECAY3   129
#define WAYV_SUSTAIN3 130
#define WAYV_RELEASE3 131
#define WAYV_ADSR3_CHECKBOX 132
    
#define WAYV_LAST_CONTROL_PORT 132
#define WAYV_COUNT 133 /* must be 1 + highest value above CHANGE THIS IF YOU ADD OR TAKE AWAY ANYTHING*/
    
#define WAYV_POLYPHONY   16

typedef struct {
    PYFX_Data *output0;
    PYFX_Data *output1;
    //PYFX_Data *tune;
    PYFX_Data *attack_main;
    PYFX_Data *decay_main;
    PYFX_Data *sustain_main;
    PYFX_Data *release_main;    
    
    PYFX_Data *attack1;
    PYFX_Data *decay1;
    PYFX_Data *sustain1;
    PYFX_Data *release1;    
    
    PYFX_Data *attack2;
    PYFX_Data *decay2;
    PYFX_Data *sustain2;
    PYFX_Data *release2;    
    
    PYFX_Data *attack3;
    PYFX_Data *decay3;
    PYFX_Data *sustain3;
    PYFX_Data *release3;    
    
    //PYFX_Data pitch;
        
    PYFX_Data *osc1pitch;
    PYFX_Data *osc1tune;
    PYFX_Data *osc1type;
    PYFX_Data *osc1vol;
    
    PYFX_Data *osc2pitch;
    PYFX_Data *osc2tune;
    PYFX_Data *osc2type;
    PYFX_Data *osc2vol;
    
    PYFX_Data *osc3pitch;
    PYFX_Data *osc3tune;
    PYFX_Data *osc3type;
    PYFX_Data *osc3vol;
    
    PYFX_Data *osc1fm1;
    PYFX_Data *osc1fm2;
    PYFX_Data *osc1fm3;
    
    PYFX_Data *osc2fm1;
    PYFX_Data *osc2fm2;
    PYFX_Data *osc2fm3;
    
    PYFX_Data *osc3fm1;
    PYFX_Data *osc3fm2;
    PYFX_Data *osc3fm3;
    
    PYFX_Data *master_vol;
    
    PYFX_Data *attack;
    PYFX_Data *decay;
    PYFX_Data *sustain;
    PYFX_Data *release;
        
    PYFX_Data *attack_f;
    PYFX_Data *decay_f;
    PYFX_Data *sustain_f;
    PYFX_Data *release_f;
    
    PYFX_Data *noise_amp;
    PYFX_Data *noise_type;
        
    PYFX_Data *osc1_uni_voice;
    PYFX_Data *osc1_uni_spread;
    PYFX_Data *osc2_uni_voice;
    PYFX_Data *osc2_uni_spread;
    PYFX_Data *osc3_uni_voice;
    PYFX_Data *osc3_uni_spread;
    
    
    PYFX_Data *master_glide;
    PYFX_Data *master_pb_amt;
        
    PYFX_Data *pitch_env_time;
    PYFX_Data *pitch_env_amt;
    
    PYFX_Data *lfo_freq;
    PYFX_Data *lfo_type;
    
    PYFX_Data * adsr1_checked;
    PYFX_Data * adsr2_checked;
    PYFX_Data * adsr3_checked;
    
    PYFX_Data *lfo_amp;
    PYFX_Data *lfo_pitch;
    PYFX_Data *lfo_amount;
        
       
    //Corresponds to the actual knobs on the effects themselves, not the mod matrix
    PYFX_Data *pfx_mod_knob[WAYV_EFFECTS_GROUPS_COUNT][WAYV_MODULAR_POLYFX_COUNT][WAYV_CONTROLS_PER_MOD_EFFECT];
    
    PYFX_Data *fx_combobox[WAYV_EFFECTS_GROUPS_COUNT][WAYV_MODULAR_POLYFX_COUNT];
        
    //PolyFX Mod Matrix
    //Corresponds to the mod matrix spinboxes
    PYFX_Data *polyfx_mod_matrix[WAYV_EFFECTS_GROUPS_COUNT][WAYV_MODULAR_POLYFX_COUNT][WAYV_MODULATOR_COUNT][WAYV_CONTROLS_PER_MOD_EFFECT];
    
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

