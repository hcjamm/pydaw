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
    
//Total number of LFOs, ADSRs, other envelopes, etc...  Used for the PolyFX mod matrix
#define EUPHORIA_MODULATOR_COUNT 4
//How many modular PolyFX
#define EUPHORIA_MODULAR_POLYFX_COUNT 4
//How many ports per PolyFX, 3 knobs and a combobox
#define EUPHORIA_PORTS_PER_MOD_EFFECT 4
//How many knobs per PolyFX, 3 knobs
#define EUPHORIA_CONTROLS_PER_MOD_EFFECT 3
//How many groups of effects.  This will become useful when each sample has an "effects group" choice  
//EDIT:  This may or may not ever come to fruition with my new strategy.  Delete this and re-arrange everywhere it's used...
#define EUPHORIA_EFFECTS_GROUPS_COUNT 1

    
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
#define WAYV_ATTACK1  20
#define WAYV_DECAY1   21
#define WAYV_SUSTAIN1 22
#define WAYV_RELEASE1 23
#define WAYV_ATTACK2  24
#define WAYV_DECAY2   25
#define WAYV_SUSTAIN2 26
#define WAYV_RELEASE2 27
    
    
    
#define EUPHORIA_ATTACK  28
#define EUPHORIA_DECAY   29
#define EUPHORIA_SUSTAIN 30
#define EUPHORIA_RELEASE 31
#define EUPHORIA_FILTER_ATTACK  32
#define EUPHORIA_FILTER_DECAY   33
#define EUPHORIA_FILTER_SUSTAIN 34
#define EUPHORIA_FILTER_RELEASE 35
#define EUPHORIA_NOISE_AMP 36
#define EUPHORIA_MASTER_VOLUME 37
#define EUPHORIA_MASTER_GLIDE 38
#define EUPHORIA_MASTER_PITCHBEND_AMT 39
#define EUPHORIA_PITCH_ENV_TIME 40
#define EUPHORIA_LFO_FREQ 41
#define EUPHORIA_LFO_TYPE 42
//From Modulex
#define EUPHORIA_FX0_KNOB0  43
#define EUPHORIA_FX0_KNOB1 44
#define EUPHORIA_FX0_KNOB2  45
#define EUPHORIA_FX0_COMBOBOX 46
#define EUPHORIA_FX1_KNOB0  47
#define EUPHORIA_FX1_KNOB1  48
#define EUPHORIA_FX1_KNOB2  49
#define LMS_FX1_COMBOBOX 50
#define LMS_FX2_KNOB0  51
#define LMS_FX2_KNOB1  52
#define LMS_FX2_KNOB2  53
#define LMS_FX2_COMBOBOX 54
#define LMS_FX3_KNOB0  55
#define LMS_FX3_KNOB1  56
#define LMS_FX3_KNOB2  57
#define LMS_FX3_COMBOBOX 58
//PolyFX Mod Matrix
#define LMS_PFXMATRIX_FIRST_PORT 59

#define LMS_PFXMATRIX_GRP0DST0SRC0CTRL0  59
#define LMS_PFXMATRIX_GRP0DST0SRC0CTRL1  60
#define LMS_PFXMATRIX_GRP0DST0SRC0CTRL2  61
#define LMS_PFXMATRIX_GRP0DST0SRC1CTRL0  62
#define LMS_PFXMATRIX_GRP0DST0SRC1CTRL1  63
#define LMS_PFXMATRIX_GRP0DST0SRC1CTRL2  64
#define LMS_PFXMATRIX_GRP0DST0SRC2CTRL0  65
#define LMS_PFXMATRIX_GRP0DST0SRC2CTRL1  66
#define LMS_PFXMATRIX_GRP0DST0SRC2CTRL2  67
#define LMS_PFXMATRIX_GRP0DST0SRC3CTRL0  68
#define LMS_PFXMATRIX_GRP0DST0SRC3CTRL1  69
#define LMS_PFXMATRIX_GRP0DST0SRC3CTRL2  70
#define LMS_PFXMATRIX_GRP0DST1SRC0CTRL0  71
#define LMS_PFXMATRIX_GRP0DST1SRC0CTRL1  72
#define LMS_PFXMATRIX_GRP0DST1SRC0CTRL2  73
#define LMS_PFXMATRIX_GRP0DST1SRC1CTRL0  74
#define LMS_PFXMATRIX_GRP0DST1SRC1CTRL1  75
#define LMS_PFXMATRIX_GRP0DST1SRC1CTRL2  76
#define LMS_PFXMATRIX_GRP0DST1SRC2CTRL0  77
#define LMS_PFXMATRIX_GRP0DST1SRC2CTRL1  78
#define LMS_PFXMATRIX_GRP0DST1SRC2CTRL2  79
#define LMS_PFXMATRIX_GRP0DST1SRC3CTRL0  80
#define LMS_PFXMATRIX_GRP0DST1SRC3CTRL1  81
#define LMS_PFXMATRIX_GRP0DST1SRC3CTRL2  82
#define LMS_PFXMATRIX_GRP0DST2SRC0CTRL0  83
#define LMS_PFXMATRIX_GRP0DST2SRC0CTRL1  84
#define LMS_PFXMATRIX_GRP0DST2SRC0CTRL2  85
#define LMS_PFXMATRIX_GRP0DST2SRC1CTRL0  86
#define LMS_PFXMATRIX_GRP0DST2SRC1CTRL1  87
#define LMS_PFXMATRIX_GRP0DST2SRC1CTRL2  88
#define LMS_PFXMATRIX_GRP0DST2SRC2CTRL0  89
#define LMS_PFXMATRIX_GRP0DST2SRC2CTRL1  90
#define LMS_PFXMATRIX_GRP0DST2SRC2CTRL2  91
#define LMS_PFXMATRIX_GRP0DST2SRC3CTRL0  92
#define LMS_PFXMATRIX_GRP0DST2SRC3CTRL1  93
#define LMS_PFXMATRIX_GRP0DST2SRC3CTRL2  94
#define LMS_PFXMATRIX_GRP0DST3SRC0CTRL0  95
#define LMS_PFXMATRIX_GRP0DST3SRC0CTRL1  96
#define LMS_PFXMATRIX_GRP0DST3SRC0CTRL2  97
#define LMS_PFXMATRIX_GRP0DST3SRC1CTRL0  98
#define LMS_PFXMATRIX_GRP0DST3SRC1CTRL1  99
#define LMS_PFXMATRIX_GRP0DST3SRC1CTRL2  100
#define LMS_PFXMATRIX_GRP0DST3SRC2CTRL0  101
#define LMS_PFXMATRIX_GRP0DST3SRC2CTRL1  102
#define LMS_PFXMATRIX_GRP0DST3SRC2CTRL2  103
#define LMS_PFXMATRIX_GRP0DST3SRC3CTRL0  104
#define LMS_PFXMATRIX_GRP0DST3SRC3CTRL1  105
#define LMS_PFXMATRIX_GRP0DST3SRC3CTRL2  106

//End PolyFX Mod Matrix

//#define LMS_GLOBAL_MIDI_OCTAVES_OFFSET 82
#define LMS_NOISE_TYPE 107

    
    
    
#define WAYV_LAST_CONTROL_PORT 107
#define WAYV_COUNT 108 /* must be 1 + highest value above CHANGE THIS IF YOU ADD OR TAKE AWAY ANYTHING*/
    
#define WAYV_PROGRAM_CHANGE 109  //Not used as a real port
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
        
    LADSPA_Data *master_uni_voice;
    LADSPA_Data *master_uni_spread;
    LADSPA_Data *master_glide;
    LADSPA_Data *master_pb_amt;
        
    LADSPA_Data *pitch_env_time;
    
    LADSPA_Data *lfo_freq;
    LADSPA_Data *lfo_type;
    
    LADSPA_Data *program;
    
    
    
    
    //Corresponds to the actual knobs on the effects themselves, not the mod matrix
    LADSPA_Data *pfx_mod_knob[EUPHORIA_EFFECTS_GROUPS_COUNT][EUPHORIA_MODULAR_POLYFX_COUNT][EUPHORIA_CONTROLS_PER_MOD_EFFECT];
    
    LADSPA_Data *fx_combobox[EUPHORIA_EFFECTS_GROUPS_COUNT][EUPHORIA_MODULAR_POLYFX_COUNT];
        
    //PolyFX Mod Matrix
    //Corresponds to the mod matrix spinboxes
    LADSPA_Data *polyfx_mod_matrix[EUPHORIA_EFFECTS_GROUPS_COUNT][EUPHORIA_MODULAR_POLYFX_COUNT][EUPHORIA_MODULATOR_COUNT][EUPHORIA_CONTROLS_PER_MOD_EFFECT];
    
    int active_polyfx[WAYV_POLYPHONY][EUPHORIA_MODULAR_POLYFX_COUNT];
    int active_polyfx_count[WAYV_POLYPHONY];
        
    int polyfx_mod_ctrl_indexes[WAYV_POLYPHONY][EUPHORIA_MODULAR_POLYFX_COUNT][(EUPHORIA_CONTROLS_PER_MOD_EFFECT * EUPHORIA_MODULATOR_COUNT)]; //The index of the control to mod, currently 0-2
    int polyfx_mod_counts[WAYV_POLYPHONY][EUPHORIA_MODULAR_POLYFX_COUNT];  //How many polyfx_mod_ptrs to iterate through for the current note
    int polyfx_mod_src_index[WAYV_POLYPHONY][EUPHORIA_MODULAR_POLYFX_COUNT][(EUPHORIA_CONTROLS_PER_MOD_EFFECT * EUPHORIA_MODULATOR_COUNT)];  //The index of the modulation source(LFO, ADSR, etc...) to multiply by
    float polyfx_mod_matrix_values[WAYV_POLYPHONY][EUPHORIA_MODULAR_POLYFX_COUNT][(EUPHORIA_CONTROLS_PER_MOD_EFFECT * EUPHORIA_MODULATOR_COUNT)];  //The value of the mod_matrix knob, multiplied by .01
    
    
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
    
    int i_fx_grps;
    int i_dst;
    int i_src;    
    int i_ctrl;
} t_wayv;




#ifdef	__cplusplus
}
#endif

#endif	/* RAY_VSYNTH_H */

