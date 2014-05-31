/*
This file is part of the PyDAW project, Copyright PyDAW Team

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef WAYV_SYNTH_H
#define	WAYV_SYNTH_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "../../include/pydaw_plugin.h"
#include "libmodsynth.h"
#include "../../libmodsynth/lib/voice.h"

//How many modular PolyFX
#define WAYV_MODULAR_POLYFX_COUNT 4
//How many ports per PolyFX, 3 knobs and a combobox
#define WAYV_PORTS_PER_MOD_EFFECT 4
//How many knobs per PolyFX, 3 knobs
#define WAYV_CONTROLS_PER_MOD_EFFECT 3
//TODO:  Delete thjs
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

#define WAVV_PFXMATRIX_GRP0DST0SRC4CTRL0  133
#define WAVV_PFXMATRIX_GRP0DST0SRC4CTRL1  134
#define WAVV_PFXMATRIX_GRP0DST0SRC4CTRL2  135
#define WAVV_PFXMATRIX_GRP0DST1SRC4CTRL0  136
#define WAVV_PFXMATRIX_GRP0DST1SRC4CTRL1  137
#define WAVV_PFXMATRIX_GRP0DST1SRC4CTRL2  138
#define WAVV_PFXMATRIX_GRP0DST2SRC4CTRL0  139
#define WAVV_PFXMATRIX_GRP0DST2SRC4CTRL1  140
#define WAVV_PFXMATRIX_GRP0DST2SRC4CTRL2  141
#define WAVV_PFXMATRIX_GRP0DST3SRC4CTRL0  142
#define WAVV_PFXMATRIX_GRP0DST3SRC4CTRL1  143
#define WAVV_PFXMATRIX_GRP0DST3SRC4CTRL2  144

#define WAVV_PFXMATRIX_GRP0DST0SRC5CTRL0  145
#define WAVV_PFXMATRIX_GRP0DST0SRC5CTRL1  146
#define WAVV_PFXMATRIX_GRP0DST0SRC5CTRL2  147
#define WAVV_PFXMATRIX_GRP0DST1SRC5CTRL0  148
#define WAVV_PFXMATRIX_GRP0DST1SRC5CTRL1  149
#define WAVV_PFXMATRIX_GRP0DST1SRC5CTRL2  150
#define WAVV_PFXMATRIX_GRP0DST2SRC5CTRL0  151
#define WAVV_PFXMATRIX_GRP0DST2SRC5CTRL1  152
#define WAVV_PFXMATRIX_GRP0DST2SRC5CTRL2  153
#define WAVV_PFXMATRIX_GRP0DST3SRC5CTRL0  154
#define WAVV_PFXMATRIX_GRP0DST3SRC5CTRL1  155
#define WAVV_PFXMATRIX_GRP0DST3SRC5CTRL2  156

#define WAYV_PERC_ENV_TIME1 157
#define WAYV_PERC_ENV_PITCH1 158
#define WAYV_PERC_ENV_TIME2 159
#define WAYV_PERC_ENV_PITCH2 160
#define WAYV_PERC_ENV_ON 161
#define WAYV_RAMP_CURVE 162
#define WAYV_MONO_MODE 163

#define WAYV_OSC4_TYPE 164
#define WAYV_OSC4_PITCH 165
#define WAYV_OSC4_TUNE 166
#define WAYV_OSC4_VOLUME 167
#define WAYV_OSC4_UNISON_VOICES 168
#define WAYV_OSC4_UNISON_SPREAD 169
#define WAYV_OSC1_FM4 170
#define WAYV_OSC2_FM4 171
#define WAYV_OSC3_FM4 172
#define WAYV_OSC4_FM1 173
#define WAYV_OSC4_FM2 174
#define WAYV_OSC4_FM3 175
#define WAYV_OSC4_FM4 176
#define WAYV_ATTACK4  177
#define WAYV_DECAY4   178
#define WAYV_SUSTAIN4 179
#define WAYV_RELEASE4 180
#define WAYV_ADSR4_CHECKBOX 181

#define WAYV_FM_MACRO1 182
#define WAYV_FM_MACRO1_OSC1_FM1 183
#define WAYV_FM_MACRO1_OSC1_FM2 184
#define WAYV_FM_MACRO1_OSC1_FM3 185
#define WAYV_FM_MACRO1_OSC1_FM4 186
#define WAYV_FM_MACRO1_OSC2_FM1 187
#define WAYV_FM_MACRO1_OSC2_FM2 188
#define WAYV_FM_MACRO1_OSC2_FM3 189
#define WAYV_FM_MACRO1_OSC2_FM4 190
#define WAYV_FM_MACRO1_OSC3_FM1 191
#define WAYV_FM_MACRO1_OSC3_FM2 192
#define WAYV_FM_MACRO1_OSC3_FM3 193
#define WAYV_FM_MACRO1_OSC3_FM4 194
#define WAYV_FM_MACRO1_OSC4_FM1 195
#define WAYV_FM_MACRO1_OSC4_FM2 196
#define WAYV_FM_MACRO1_OSC4_FM3 197
#define WAYV_FM_MACRO1_OSC4_FM4 198

#define WAYV_FM_MACRO2 199
#define WAYV_FM_MACRO2_OSC1_FM1 200
#define WAYV_FM_MACRO2_OSC1_FM2 201
#define WAYV_FM_MACRO2_OSC1_FM3 202
#define WAYV_FM_MACRO2_OSC1_FM4 203
#define WAYV_FM_MACRO2_OSC2_FM1 204
#define WAYV_FM_MACRO2_OSC2_FM2 205
#define WAYV_FM_MACRO2_OSC2_FM3 206
#define WAYV_FM_MACRO2_OSC2_FM4 207
#define WAYV_FM_MACRO2_OSC3_FM1 208
#define WAYV_FM_MACRO2_OSC3_FM2 209
#define WAYV_FM_MACRO2_OSC3_FM3 210
#define WAYV_FM_MACRO2_OSC3_FM4 211
#define WAYV_FM_MACRO2_OSC4_FM1 212
#define WAYV_FM_MACRO2_OSC4_FM2 213
#define WAYV_FM_MACRO2_OSC4_FM3 214
#define WAYV_FM_MACRO2_OSC4_FM4 215

#define WAYV_LFO_PHASE 216

#define WAYV_FM_MACRO1_OSC1_VOL 217
#define WAYV_FM_MACRO1_OSC2_VOL 218
#define WAYV_FM_MACRO1_OSC3_VOL 219
#define WAYV_FM_MACRO1_OSC4_VOL 220

#define WAYV_FM_MACRO2_OSC1_VOL 221
#define WAYV_FM_MACRO2_OSC2_VOL 222
#define WAYV_FM_MACRO2_OSC3_VOL 223
#define WAYV_FM_MACRO2_OSC4_VOL 224
#define WAYV_LFO_PITCH_FINE 225
#define WAYV_ADSR_PREFX 226

#define WAYV_ADSR1_DELAY 227
#define WAYV_ADSR2_DELAY 228
#define WAYV_ADSR3_DELAY 229
#define WAYV_ADSR4_DELAY 230

#define WAYV_ADSR1_HOLD 231
#define WAYV_ADSR2_HOLD 232
#define WAYV_ADSR3_HOLD 233
#define WAYV_ADSR4_HOLD 234

/* must be 1 + highest value above
 * CHANGE THIS IF YOU ADD OR TAKE AWAY ANYTHING*/
#define WAYV_COUNT 235

#define WAYV_POLYPHONY   16
#define WAYV_POLYPHONY_THRESH 12

typedef struct
{
    PYFX_Data *output0;
    PYFX_Data *output1;
    PYFX_Data *attack_main;
    PYFX_Data *decay_main;
    PYFX_Data *sustain_main;
    PYFX_Data *release_main;

    PYFX_Data *attack[4];
    PYFX_Data *decay[4];
    PYFX_Data *sustain[4];
    PYFX_Data *release[4];

    PYFX_Data * adsr_checked[4];

    PYFX_Data *adsr_fm_delay[4];
    PYFX_Data *adsr_fm_hold[4];

    //PYFX_Data pitch;

    PYFX_Data *osc_pitch[4];
    PYFX_Data *osc_tune[4];
    PYFX_Data *osc_vol[4];
    PYFX_Data *osc_type[4];

    PYFX_Data *osc_fm[4][4];

    PYFX_Data *master_vol;

    PYFX_Data *pfx_attack;
    PYFX_Data *pfx_decay;
    PYFX_Data *pfx_sustain;
    PYFX_Data *pfx_release;

    PYFX_Data *pfx_attack_f;
    PYFX_Data *pfx_decay_f;
    PYFX_Data *pfx_sustain_f;
    PYFX_Data *pfx_release_f;

    PYFX_Data *noise_amp;
    PYFX_Data *noise_type;

    PYFX_Data *osc_uni_voice[4];
    PYFX_Data *osc_uni_spread[4];

    PYFX_Data *master_glide;
    PYFX_Data *master_pb_amt;

    PYFX_Data *pitch_env_time;
    PYFX_Data *pitch_env_amt;
    PYFX_Data *ramp_curve;

    PYFX_Data *lfo_freq;
    PYFX_Data *lfo_type;
    PYFX_Data *lfo_phase;

    PYFX_Data *lfo_amp;
    PYFX_Data *lfo_pitch;
    PYFX_Data *lfo_pitch_fine;
    PYFX_Data *lfo_amount;

    PYFX_Data *perc_env_time1;
    PYFX_Data *perc_env_pitch1;
    PYFX_Data *perc_env_time2;
    PYFX_Data *perc_env_pitch2;
    PYFX_Data *perc_env_on;
    PYFX_Data *adsr_prefx;

    PYFX_Data *fm_macro[2];
    PYFX_Data *fm_macro_values[2][4][4];
    PYFX_Data *amp_macro_values[2][4];

    PYFX_Data *mono_mode;

    //Corresponds to the actual knobs on the effects themselves,
    //not the mod matrix
    PYFX_Data *pfx_mod_knob[WAYV_EFFECTS_GROUPS_COUNT]
            [WAYV_MODULAR_POLYFX_COUNT][WAYV_CONTROLS_PER_MOD_EFFECT];

    PYFX_Data *fx_combobox[WAYV_EFFECTS_GROUPS_COUNT]
            [WAYV_MODULAR_POLYFX_COUNT];

    //PolyFX Mod Matrix
    //Corresponds to the mod matrix spinboxes
    PYFX_Data *polyfx_mod_matrix[WAYV_EFFECTS_GROUPS_COUNT]
            [WAYV_MODULAR_POLYFX_COUNT][WAYV_MODULATOR_COUNT]
            [WAYV_CONTROLS_PER_MOD_EFFECT];

    int active_polyfx[WAYV_POLYPHONY][WAYV_MODULAR_POLYFX_COUNT];
    int active_polyfx_count[WAYV_POLYPHONY];

    //The index of the control to mod, currently 0-2
    int polyfx_mod_ctrl_indexes[WAYV_POLYPHONY][WAYV_MODULAR_POLYFX_COUNT]
    [(WAYV_CONTROLS_PER_MOD_EFFECT * WAYV_MODULATOR_COUNT)];

    //How many polyfx_mod_ptrs to iterate through for the current note
    int polyfx_mod_counts[WAYV_POLYPHONY][WAYV_MODULAR_POLYFX_COUNT];

    //The index of the modulation source(LFO, ADSR, etc...) to multiply by
    int polyfx_mod_src_index[WAYV_POLYPHONY][WAYV_MODULAR_POLYFX_COUNT]
    [(WAYV_CONTROLS_PER_MOD_EFFECT * WAYV_MODULATOR_COUNT)];

    //The value of the mod_matrix knob, multiplied by .01
    float polyfx_mod_matrix_values[WAYV_POLYPHONY][WAYV_MODULAR_POLYFX_COUNT]
    [(WAYV_CONTROLS_PER_MOD_EFFECT * WAYV_MODULATOR_COUNT)];

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

    int midi_event_types[200];
    int midi_event_ticks[200];
    float midi_event_values[200];
    int midi_event_ports[200];
    int midi_event_count;

    float * port_table;
} t_wayv;




#ifdef	__cplusplus
}
#endif

#endif	/* WAYV_SYNTH_H */

