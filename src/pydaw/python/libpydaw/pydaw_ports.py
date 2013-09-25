# -*- coding: utf-8 -*-
"""
This file is part of the PyDAW project, Copyright PyDAW Team

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
"""

#Euphoria
EUPHORIA_FILES_STRING_DELIMITER = '~' #'|'
#When used in place of "|", it tells the sampler to load the sample even if it's already been loaded once.
EUPHORIA_FILES_STRING_RELOAD_DELIMITER = '>'
EUPHORIA_MAX_SAMPLE_COUNT  =  32
#+1 to LMS_MAX_SAMPLE_COUNT, the highest index is for the preview sample
EUPHORIA_TOTAL_SAMPLE_COUNT  =  33
#Total number of LFOs, ADSRs, other envelopes, etc... Used for the PolyFX mod matrix
EUPHORIA_MODULATOR_COUNT  =  4
#How many modular PolyFX
EUPHORIA_MODULAR_POLYFX_COUNT  =  4
#How many ports per PolyFX, 3 knobs and a combobox
EUPHORIA_PORTS_PER_MOD_EFFECT  =  4
#How many knobs per PolyFX, 3 knobs
EUPHORIA_CONTROLS_PER_MOD_EFFECT  =  3
#How many groups of effects. This will become useful when each sample has an "effects group" choice
#EDIT: This may or may not ever come to fruition with my  strategy. Delete self and re-arrange everywhere it's used...
EUPHORIA_EFFECTS_GROUPS_COUNT  =  1
#The number of mono_fx groups
EUPHORIA_MONO_FX_GROUPS_COUNT  =  EUPHORIA_MAX_SAMPLE_COUNT
EUPHORIA_MONO_FX_COUNT  =  4
EUPHORIA_LABEL  =  "Euphoria"
EUPHORIA_OUTPUT_LEFT  =  0
EUPHORIA_OUTPUT_RIGHT  =  1
EUPHORIA_FIRST_CONTROL_PORT  =  2
EUPHORIA_SELECTED_SAMPLE  =  2
EUPHORIA_ATTACK  =  3
EUPHORIA_DECAY  =  4
EUPHORIA_SUSTAIN  =  5
EUPHORIA_RELEASE  =  6
EUPHORIA_FILTER_ATTACK  =  7
EUPHORIA_FILTER_DECAY  =  8
EUPHORIA_FILTER_SUSTAIN  =  9
EUPHORIA_FILTER_RELEASE  =  10
EUPHORIA_NOISE_AMP  =  11
EUPHORIA_MASTER_VOLUME  =  12
EUPHORIA_MASTER_GLIDE  =  13
EUPHORIA_MASTER_PITCHBEND_AMT  =  14
EUPHORIA_PITCH_ENV_TIME  =  15
EUPHORIA_LFO_FREQ  =  16
EUPHORIA_LFO_TYPE  =  17
#From Modulex
EUPHORIA_FX0_KNOB0  =  18
EUPHORIA_FX0_KNOB1  =  19
EUPHORIA_FX0_KNOB2  =  20
EUPHORIA_FX0_COMBOBOX  =  21
EUPHORIA_FX1_KNOB0  =  22
EUPHORIA_FX1_KNOB1  =  23
EUPHORIA_FX1_KNOB2  =  24
EUPHORIA_FX1_COMBOBOX  =  25
EUPHORIA_FX2_KNOB0  =  26
EUPHORIA_FX2_KNOB1  =  27
EUPHORIA_FX2_KNOB2  =  28
EUPHORIA_FX2_COMBOBOX  =  29
EUPHORIA_FX3_KNOB0  =  30
EUPHORIA_FX3_KNOB1  =  31
EUPHORIA_FX3_KNOB2  =  32
EUPHORIA_FX3_COMBOBOX  =  33
#PolyFX Mod Matrix
EUPHORIA_PFXMATRIX_FIRST_PORT  =  34
EUPHORIA_PFXMATRIX_GRP0DST0SRC0CTRL0  =  34
EUPHORIA_PFXMATRIX_GRP0DST0SRC0CTRL1  =  35
EUPHORIA_PFXMATRIX_GRP0DST0SRC0CTRL2  =  36
EUPHORIA_PFXMATRIX_GRP0DST0SRC1CTRL0  =  37
EUPHORIA_PFXMATRIX_GRP0DST0SRC1CTRL1  =  38
EUPHORIA_PFXMATRIX_GRP0DST0SRC1CTRL2  =  39
EUPHORIA_PFXMATRIX_GRP0DST0SRC2CTRL0  =  40
EUPHORIA_PFXMATRIX_GRP0DST0SRC2CTRL1  =  41
EUPHORIA_PFXMATRIX_GRP0DST0SRC2CTRL2  =  42
EUPHORIA_PFXMATRIX_GRP0DST0SRC3CTRL0  =  43
EUPHORIA_PFXMATRIX_GRP0DST0SRC3CTRL1  =  44
EUPHORIA_PFXMATRIX_GRP0DST0SRC3CTRL2  =  45
EUPHORIA_PFXMATRIX_GRP0DST1SRC0CTRL0  =  46
EUPHORIA_PFXMATRIX_GRP0DST1SRC0CTRL1  =  47
EUPHORIA_PFXMATRIX_GRP0DST1SRC0CTRL2  =  48
EUPHORIA_PFXMATRIX_GRP0DST1SRC1CTRL0  =  49
EUPHORIA_PFXMATRIX_GRP0DST1SRC1CTRL1  =  50
EUPHORIA_PFXMATRIX_GRP0DST1SRC1CTRL2  =  51
EUPHORIA_PFXMATRIX_GRP0DST1SRC2CTRL0  =  52
EUPHORIA_PFXMATRIX_GRP0DST1SRC2CTRL1  =  53
EUPHORIA_PFXMATRIX_GRP0DST1SRC2CTRL2  =  54
EUPHORIA_PFXMATRIX_GRP0DST1SRC3CTRL0  =  55
EUPHORIA_PFXMATRIX_GRP0DST1SRC3CTRL1  =  56
EUPHORIA_PFXMATRIX_GRP0DST1SRC3CTRL2  =  57
EUPHORIA_PFXMATRIX_GRP0DST2SRC0CTRL0  =  58
EUPHORIA_PFXMATRIX_GRP0DST2SRC0CTRL1  =  59
EUPHORIA_PFXMATRIX_GRP0DST2SRC0CTRL2  =  60
EUPHORIA_PFXMATRIX_GRP0DST2SRC1CTRL0  =  61
EUPHORIA_PFXMATRIX_GRP0DST2SRC1CTRL1  =  62
EUPHORIA_PFXMATRIX_GRP0DST2SRC1CTRL2  =  63
EUPHORIA_PFXMATRIX_GRP0DST2SRC2CTRL0  =  64
EUPHORIA_PFXMATRIX_GRP0DST2SRC2CTRL1  =  65
EUPHORIA_PFXMATRIX_GRP0DST2SRC2CTRL2  =  66
EUPHORIA_PFXMATRIX_GRP0DST2SRC3CTRL0  =  67
EUPHORIA_PFXMATRIX_GRP0DST2SRC3CTRL1  =  68
EUPHORIA_PFXMATRIX_GRP0DST2SRC3CTRL2  =  69
EUPHORIA_PFXMATRIX_GRP0DST3SRC0CTRL0  =  70
EUPHORIA_PFXMATRIX_GRP0DST3SRC0CTRL1  =  71
EUPHORIA_PFXMATRIX_GRP0DST3SRC0CTRL2  =  72
EUPHORIA_PFXMATRIX_GRP0DST3SRC1CTRL0  =  73
EUPHORIA_PFXMATRIX_GRP0DST3SRC1CTRL1  =  74
EUPHORIA_PFXMATRIX_GRP0DST3SRC1CTRL2  =  75
EUPHORIA_PFXMATRIX_GRP0DST3SRC2CTRL0  =  76
EUPHORIA_PFXMATRIX_GRP0DST3SRC2CTRL1  =  77
EUPHORIA_PFXMATRIX_GRP0DST3SRC2CTRL2  =  78
EUPHORIA_PFXMATRIX_GRP0DST3SRC3CTRL0  =  79
EUPHORIA_PFXMATRIX_GRP0DST3SRC3CTRL1  =  80
EUPHORIA_PFXMATRIX_GRP0DST3SRC3CTRL2  =  81
#End PolyFX Mod Matrix
EUPHORIA_NOISE_TYPE  =  82
EUPHORIA_LFO_PITCH  =  83
"""This is the last control port, + 1, for zero-based iteration"""
EUPHORIA_LAST_REGULAR_CONTROL_PORT  =  84
"""The first port to use when enumerating the ports for mod_matrix controls. All of the mod_matrix ports should be sequential,
* any additional ports should prepend self port number"""
EUPHORIA_FIRST_SAMPLE_TABLE_PORT  =  84
"""The range of ports for sample pitch"""
EUPHORIA_SAMPLE_PITCH_PORT_RANGE_MIN  =  EUPHORIA_FIRST_SAMPLE_TABLE_PORT
EUPHORIA_SAMPLE_PITCH_PORT_RANGE_MAX  =  (EUPHORIA_SAMPLE_PITCH_PORT_RANGE_MIN + EUPHORIA_MAX_SAMPLE_COUNT)
"""The range of ports for the low note to play a sample on"""
EUPHORIA_PLAY_PITCH_LOW_PORT_RANGE_MIN  =  EUPHORIA_SAMPLE_PITCH_PORT_RANGE_MAX
EUPHORIA_PLAY_PITCH_LOW_PORT_RANGE_MAX  =  (EUPHORIA_PLAY_PITCH_LOW_PORT_RANGE_MIN + EUPHORIA_MAX_SAMPLE_COUNT)
"""The range of ports for the high note to play a sample on"""
EUPHORIA_PLAY_PITCH_HIGH_PORT_RANGE_MIN  =  EUPHORIA_PLAY_PITCH_LOW_PORT_RANGE_MAX
EUPHORIA_PLAY_PITCH_HIGH_PORT_RANGE_MAX  =  (EUPHORIA_PLAY_PITCH_HIGH_PORT_RANGE_MIN + EUPHORIA_MAX_SAMPLE_COUNT)
"""The range of ports for sample volume"""
EUPHORIA_SAMPLE_VOLUME_PORT_RANGE_MIN  =  EUPHORIA_PLAY_PITCH_HIGH_PORT_RANGE_MAX
EUPHORIA_SAMPLE_VOLUME_PORT_RANGE_MAX  =  (EUPHORIA_SAMPLE_VOLUME_PORT_RANGE_MIN + EUPHORIA_MAX_SAMPLE_COUNT)
EUPHORIA_SAMPLE_START_PORT_RANGE_MIN  =  EUPHORIA_SAMPLE_VOLUME_PORT_RANGE_MAX
EUPHORIA_SAMPLE_START_PORT_RANGE_MAX  =  (EUPHORIA_SAMPLE_START_PORT_RANGE_MIN + EUPHORIA_MAX_SAMPLE_COUNT)
EUPHORIA_SAMPLE_END_PORT_RANGE_MIN  =  EUPHORIA_SAMPLE_START_PORT_RANGE_MAX
EUPHORIA_SAMPLE_END_PORT_RANGE_MAX  =  (EUPHORIA_SAMPLE_END_PORT_RANGE_MIN + EUPHORIA_MAX_SAMPLE_COUNT)
EUPHORIA_SAMPLE_VEL_SENS_PORT_RANGE_MIN  =  EUPHORIA_SAMPLE_END_PORT_RANGE_MAX
EUPHORIA_SAMPLE_VEL_SENS_PORT_RANGE_MAX  =  (EUPHORIA_SAMPLE_VEL_SENS_PORT_RANGE_MIN + EUPHORIA_MAX_SAMPLE_COUNT)
EUPHORIA_SAMPLE_VEL_LOW_PORT_RANGE_MIN  =  EUPHORIA_SAMPLE_VEL_SENS_PORT_RANGE_MAX
EUPHORIA_SAMPLE_VEL_LOW_PORT_RANGE_MAX  =  (EUPHORIA_SAMPLE_VEL_LOW_PORT_RANGE_MIN + EUPHORIA_MAX_SAMPLE_COUNT)
EUPHORIA_SAMPLE_VEL_HIGH_PORT_RANGE_MIN  =  EUPHORIA_SAMPLE_VEL_LOW_PORT_RANGE_MAX
EUPHORIA_SAMPLE_VEL_HIGH_PORT_RANGE_MAX  =  (EUPHORIA_SAMPLE_VEL_HIGH_PORT_RANGE_MIN + EUPHORIA_MAX_SAMPLE_COUNT)
EUPHORIA_PITCH_PORT_RANGE_MIN  =  EUPHORIA_SAMPLE_VEL_HIGH_PORT_RANGE_MAX
EUPHORIA_PITCH_PORT_RANGE_MAX  =  (EUPHORIA_PITCH_PORT_RANGE_MIN + EUPHORIA_MAX_SAMPLE_COUNT)
EUPHORIA_TUNE_PORT_RANGE_MIN  =  EUPHORIA_PITCH_PORT_RANGE_MAX
EUPHORIA_TUNE_PORT_RANGE_MAX  =  (EUPHORIA_TUNE_PORT_RANGE_MIN + EUPHORIA_MAX_SAMPLE_COUNT)
EUPHORIA_SAMPLE_INTERPOLATION_MODE_PORT_RANGE_MIN  =  EUPHORIA_TUNE_PORT_RANGE_MAX
EUPHORIA_SAMPLE_INTERPOLATION_MODE_PORT_RANGE_MAX  =  (EUPHORIA_SAMPLE_INTERPOLATION_MODE_PORT_RANGE_MIN + EUPHORIA_MAX_SAMPLE_COUNT)
EUPHORIA_SAMPLE_LOOP_START_PORT_RANGE_MIN  =  EUPHORIA_SAMPLE_INTERPOLATION_MODE_PORT_RANGE_MAX
EUPHORIA_SAMPLE_LOOP_START_PORT_RANGE_MAX  =  (EUPHORIA_SAMPLE_LOOP_START_PORT_RANGE_MIN + EUPHORIA_MAX_SAMPLE_COUNT)
EUPHORIA_SAMPLE_LOOP_END_PORT_RANGE_MIN  =  EUPHORIA_SAMPLE_LOOP_START_PORT_RANGE_MAX
EUPHORIA_SAMPLE_LOOP_END_PORT_RANGE_MAX  =  (EUPHORIA_SAMPLE_LOOP_END_PORT_RANGE_MIN + EUPHORIA_MAX_SAMPLE_COUNT)
EUPHORIA_SAMPLE_LOOP_MODE_PORT_RANGE_MIN  =  EUPHORIA_SAMPLE_LOOP_END_PORT_RANGE_MAX
EUPHORIA_SAMPLE_LOOP_MODE_PORT_RANGE_MAX  =  (EUPHORIA_SAMPLE_LOOP_MODE_PORT_RANGE_MIN + EUPHORIA_MAX_SAMPLE_COUNT)
#MonoFX0
EUPHORIA_MONO_FX0_KNOB0_PORT_RANGE_MIN  =  EUPHORIA_SAMPLE_LOOP_MODE_PORT_RANGE_MAX
EUPHORIA_MONO_FX0_KNOB0_PORT_RANGE_MAX  =  (EUPHORIA_MONO_FX0_KNOB0_PORT_RANGE_MIN + EUPHORIA_MONO_FX_GROUPS_COUNT)
EUPHORIA_MONO_FX0_KNOB1_PORT_RANGE_MIN  =  EUPHORIA_MONO_FX0_KNOB0_PORT_RANGE_MAX
EUPHORIA_MONO_FX0_KNOB1_PORT_RANGE_MAX  =  (EUPHORIA_MONO_FX0_KNOB1_PORT_RANGE_MIN + EUPHORIA_MONO_FX_GROUPS_COUNT)
EUPHORIA_MONO_FX0_KNOB2_PORT_RANGE_MIN  =  EUPHORIA_MONO_FX0_KNOB1_PORT_RANGE_MAX
EUPHORIA_MONO_FX0_KNOB2_PORT_RANGE_MAX  =  (EUPHORIA_MONO_FX0_KNOB2_PORT_RANGE_MIN + EUPHORIA_MONO_FX_GROUPS_COUNT)
EUPHORIA_MONO_FX0_COMBOBOX_PORT_RANGE_MIN  =  EUPHORIA_MONO_FX0_KNOB2_PORT_RANGE_MAX
EUPHORIA_MONO_FX0_COMBOBOX_PORT_RANGE_MAX  =  (EUPHORIA_MONO_FX0_COMBOBOX_PORT_RANGE_MIN + EUPHORIA_MONO_FX_GROUPS_COUNT)
#MonoFX1
EUPHORIA_MONO_FX1_KNOB0_PORT_RANGE_MIN  =  EUPHORIA_MONO_FX0_COMBOBOX_PORT_RANGE_MAX
EUPHORIA_MONO_FX1_KNOB0_PORT_RANGE_MAX  =  (EUPHORIA_MONO_FX1_KNOB0_PORT_RANGE_MIN + EUPHORIA_MONO_FX_GROUPS_COUNT)
EUPHORIA_MONO_FX1_KNOB1_PORT_RANGE_MIN  =  EUPHORIA_MONO_FX1_KNOB0_PORT_RANGE_MAX
EUPHORIA_MONO_FX1_KNOB1_PORT_RANGE_MAX  =  (EUPHORIA_MONO_FX1_KNOB1_PORT_RANGE_MIN + EUPHORIA_MONO_FX_GROUPS_COUNT)
EUPHORIA_MONO_FX1_KNOB2_PORT_RANGE_MIN  =  EUPHORIA_MONO_FX1_KNOB1_PORT_RANGE_MAX
EUPHORIA_MONO_FX1_KNOB2_PORT_RANGE_MAX  =  (EUPHORIA_MONO_FX1_KNOB2_PORT_RANGE_MIN + EUPHORIA_MONO_FX_GROUPS_COUNT)
EUPHORIA_MONO_FX1_COMBOBOX_PORT_RANGE_MIN  =  EUPHORIA_MONO_FX1_KNOB2_PORT_RANGE_MAX
EUPHORIA_MONO_FX1_COMBOBOX_PORT_RANGE_MAX  =  (EUPHORIA_MONO_FX1_COMBOBOX_PORT_RANGE_MIN + EUPHORIA_MONO_FX_GROUPS_COUNT)
#MonoFX2
EUPHORIA_MONO_FX2_KNOB0_PORT_RANGE_MIN  =  EUPHORIA_MONO_FX1_COMBOBOX_PORT_RANGE_MAX
EUPHORIA_MONO_FX2_KNOB0_PORT_RANGE_MAX  =  (EUPHORIA_MONO_FX2_KNOB0_PORT_RANGE_MIN + EUPHORIA_MONO_FX_GROUPS_COUNT)
EUPHORIA_MONO_FX2_KNOB1_PORT_RANGE_MIN  =  EUPHORIA_MONO_FX2_KNOB0_PORT_RANGE_MAX
EUPHORIA_MONO_FX2_KNOB1_PORT_RANGE_MAX  =  (EUPHORIA_MONO_FX2_KNOB1_PORT_RANGE_MIN + EUPHORIA_MONO_FX_GROUPS_COUNT)
EUPHORIA_MONO_FX2_KNOB2_PORT_RANGE_MIN  =  EUPHORIA_MONO_FX2_KNOB1_PORT_RANGE_MAX
EUPHORIA_MONO_FX2_KNOB2_PORT_RANGE_MAX  =  (EUPHORIA_MONO_FX2_KNOB2_PORT_RANGE_MIN + EUPHORIA_MONO_FX_GROUPS_COUNT)
EUPHORIA_MONO_FX2_COMBOBOX_PORT_RANGE_MIN  =  EUPHORIA_MONO_FX2_KNOB2_PORT_RANGE_MAX
EUPHORIA_MONO_FX2_COMBOBOX_PORT_RANGE_MAX  =  (EUPHORIA_MONO_FX2_COMBOBOX_PORT_RANGE_MIN + EUPHORIA_MONO_FX_GROUPS_COUNT)
#MonoFX3
EUPHORIA_MONO_FX3_KNOB0_PORT_RANGE_MIN  =  EUPHORIA_MONO_FX2_COMBOBOX_PORT_RANGE_MAX
EUPHORIA_MONO_FX3_KNOB0_PORT_RANGE_MAX  =  (EUPHORIA_MONO_FX3_KNOB0_PORT_RANGE_MIN + EUPHORIA_MONO_FX_GROUPS_COUNT)
EUPHORIA_MONO_FX3_KNOB1_PORT_RANGE_MIN  =  EUPHORIA_MONO_FX3_KNOB0_PORT_RANGE_MAX
EUPHORIA_MONO_FX3_KNOB1_PORT_RANGE_MAX  =  (EUPHORIA_MONO_FX3_KNOB1_PORT_RANGE_MIN + EUPHORIA_MONO_FX_GROUPS_COUNT)
EUPHORIA_MONO_FX3_KNOB2_PORT_RANGE_MIN  =  EUPHORIA_MONO_FX3_KNOB1_PORT_RANGE_MAX
EUPHORIA_MONO_FX3_KNOB2_PORT_RANGE_MAX  =  (EUPHORIA_MONO_FX3_KNOB2_PORT_RANGE_MIN + EUPHORIA_MONO_FX_GROUPS_COUNT)
EUPHORIA_MONO_FX3_COMBOBOX_PORT_RANGE_MIN  =  EUPHORIA_MONO_FX3_KNOB2_PORT_RANGE_MAX
EUPHORIA_MONO_FX3_COMBOBOX_PORT_RANGE_MAX  =  (EUPHORIA_MONO_FX3_COMBOBOX_PORT_RANGE_MIN + EUPHORIA_MONO_FX_GROUPS_COUNT)
#Sample FX Group
EUPHORIA_SAMPLE_MONO_FX_GROUP_PORT_RANGE_MIN  =  EUPHORIA_MONO_FX3_COMBOBOX_PORT_RANGE_MAX
EUPHORIA_SAMPLE_MONO_FX_GROUP_PORT_RANGE_MAX  =  (EUPHORIA_SAMPLE_MONO_FX_GROUP_PORT_RANGE_MIN + EUPHORIA_MAX_SAMPLE_COUNT)
EUPHORIA_PORT_COUNT  =  EUPHORIA_SAMPLE_MONO_FX_GROUP_PORT_RANGE_MAX

#Modulex

MODULEX_INPUT0  =  0
MODULEX_INPUT1  =  1
MODULEX_OUTPUT0  =  2
MODULEX_OUTPUT1  =  3
MODULEX_FIRST_CONTROL_PORT  =  4
MODULEX_FX0_KNOB0  =  4
MODULEX_FX0_KNOB1  =  5
MODULEX_FX0_KNOB2  =  6
MODULEX_FX0_COMBOBOX  =  7
MODULEX_FX1_KNOB0  =  8
MODULEX_FX1_KNOB1  =  9
MODULEX_FX1_KNOB2  =  10
MODULEX_FX1_COMBOBOX  =  11
MODULEX_FX2_KNOB0  =  12
MODULEX_FX2_KNOB1  =  13
MODULEX_FX2_KNOB2  =  14
MODULEX_FX2_COMBOBOX  =  15
MODULEX_FX3_KNOB0  =  16
MODULEX_FX3_KNOB1  =  17
MODULEX_FX3_KNOB2  =  18
MODULEX_FX3_COMBOBOX  =  19
MODULEX_FX4_KNOB0  =  20
MODULEX_FX4_KNOB1  =  21
MODULEX_FX4_KNOB2  =  22
MODULEX_FX4_COMBOBOX  =  23
MODULEX_FX5_KNOB0  =  24
MODULEX_FX5_KNOB1  =  25
MODULEX_FX5_KNOB2  =  26
MODULEX_FX5_COMBOBOX  =  27
MODULEX_FX6_KNOB0  =  28
MODULEX_FX6_KNOB1  =  29
MODULEX_FX6_KNOB2  =  30
MODULEX_FX6_COMBOBOX  =  31
MODULEX_FX7_KNOB0  =  32
MODULEX_FX7_KNOB1  =  33
MODULEX_FX7_KNOB2  =  34
MODULEX_FX7_COMBOBOX  =  35
MODULEX_DELAY_TIME  =  36
MODULEX_FEEDBACK  =  37
MODULEX_DRY  =  38
MODULEX_WET  =  39
MODULEX_DUCK  =  40
MODULEX_CUTOFF  =  41
MODULEX_STEREO  =  42
MODULEX_VOL_SLIDER  =  43
MODULEX_REVERB_TIME  =  44
MODULEX_REVERB_WET  =  45
MODULEX_REVERB_COLOR  =  46
"""This is the last control port"""
MODULEX_LAST_CONTROL_PORT  =  46
MODULEX_COUNT  =  47 #must be 1 + highest value above CHANGE THIS IF YOU ADD OR TAKE AWAY ANYTHING


#Way-V


#Total number of LFOs, ADSRs, other envelopes, etc... Used for the PolyFX mod matrix
WAYV_MODULATOR_COUNT  =  4
#How many modular PolyFX
WAYV_MODULAR_POLYFX_COUNT  =  4
#How many ports per PolyFX, 3 knobs and a combobox
WAYV_PORTS_PER_MOD_EFFECT  =  4
#How many knobs per PolyFX, 3 knobs
WAYV_CONTROLS_PER_MOD_EFFECT  =  3
WAYV_EFFECTS_GROUPS_COUNT  =  1
WAYV_OUTPUT0  =  0
WAYV_OUTPUT1  =  1
WAYV_FIRST_CONTROL_PORT  =  2
WAYV_ATTACK_MAIN  =  2
WAYV_DECAY_MAIN  =  3
WAYV_SUSTAIN_MAIN  =  4
WAYV_RELEASE_MAIN  =  5
WAYV_NOISE_AMP  =  6
WAYV_OSC1_TYPE  =  7
WAYV_OSC1_PITCH  =  8
WAYV_OSC1_TUNE  =  9
WAYV_OSC1_VOLUME  =  10
WAYV_OSC2_TYPE  =  11
WAYV_OSC2_PITCH  =  12
WAYV_OSC2_TUNE  =  13
WAYV_OSC2_VOLUME  =  14
WAYV_MASTER_VOLUME  =  15
WAYV_OSC1_UNISON_VOICES  =  16
WAYV_OSC1_UNISON_SPREAD  =  17
WAYV_MASTER_GLIDE  =  18
WAYV_MASTER_PITCHBEND_AMT  =  19
WAYV_ATTACK1  =  20
WAYV_DECAY1  =  21
WAYV_SUSTAIN1  =  22
WAYV_RELEASE1  =  23
WAYV_ATTACK2  =  24
WAYV_DECAY2  =  25
WAYV_SUSTAIN2  =  26
WAYV_RELEASE2  =  27
WAYV_ATTACK_PFX1  =  28
WAYV_DECAY_PFX1  =  29
WAYV_SUSTAIN_PFX1  =  30
WAYV_RELEASE_PFX1  =  31
WAYV_ATTACK_PFX2  =  32
WAYV_DECAY_PFX2  =  33
WAYV_SUSTAIN_PFX2  =  34
WAYV_RELEASE_PFX2  =  35
LMS_NOISE_TYPE  =  36
WAYV_RAMP_ENV_TIME  =  37
WAYV_LFO_FREQ  =  38
WAYV_LFO_TYPE  =  39
WAYV_FX0_KNOB0  =  40
WAYV_FX0_KNOB1  =  41
WAYV_FX0_KNOB2  =  42
WAYV_FX0_COMBOBOX  =  43
WAYV_FX1_KNOB0  =  44
WAYV_FX1_KNOB1  =  45
WAYV_FX1_KNOB2  =  46
WAYV_FX1_COMBOBOX  =  47
WAYV_FX2_KNOB0  =  48
WAYV_FX2_KNOB1  =  49
WAYV_FX2_KNOB2  =  50
WAYV_FX2_COMBOBOX  =  51
WAYV_FX3_KNOB0  =  52
WAYV_FX3_KNOB1  =  53
WAYV_FX3_KNOB2  =  54
WAYV_FX3_COMBOBOX  =  55
#PolyFX Mod Matrix
WAVV_PFXMATRIX_FIRST_PORT  =  56
WAVV_PFXMATRIX_GRP0DST0SRC0CTRL0  =  56
WAVV_PFXMATRIX_GRP0DST0SRC0CTRL1  =  57
WAVV_PFXMATRIX_GRP0DST0SRC0CTRL2  =  58
WAVV_PFXMATRIX_GRP0DST0SRC1CTRL0  =  59
WAVV_PFXMATRIX_GRP0DST0SRC1CTRL1  =  60
WAVV_PFXMATRIX_GRP0DST0SRC1CTRL2  =  61
WAVV_PFXMATRIX_GRP0DST0SRC2CTRL0  =  62
WAVV_PFXMATRIX_GRP0DST0SRC2CTRL1  =  63
WAVV_PFXMATRIX_GRP0DST0SRC2CTRL2  =  64
WAVV_PFXMATRIX_GRP0DST0SRC3CTRL0  =  65
WAVV_PFXMATRIX_GRP0DST0SRC3CTRL1  =  66
WAVV_PFXMATRIX_GRP0DST0SRC3CTRL2  =  67
WAVV_PFXMATRIX_GRP0DST1SRC0CTRL0  =  68
WAVV_PFXMATRIX_GRP0DST1SRC0CTRL1  =  69
WAVV_PFXMATRIX_GRP0DST1SRC0CTRL2  =  70
WAVV_PFXMATRIX_GRP0DST1SRC1CTRL0  =  71
WAVV_PFXMATRIX_GRP0DST1SRC1CTRL1  =  72
WAVV_PFXMATRIX_GRP0DST1SRC1CTRL2  =  73
WAVV_PFXMATRIX_GRP0DST1SRC2CTRL0  =  74
WAVV_PFXMATRIX_GRP0DST1SRC2CTRL1  =  75
WAVV_PFXMATRIX_GRP0DST1SRC2CTRL2  =  76
WAVV_PFXMATRIX_GRP0DST1SRC3CTRL0  =  77
WAVV_PFXMATRIX_GRP0DST1SRC3CTRL1  =  78
WAVV_PFXMATRIX_GRP0DST1SRC3CTRL2  =  79
WAVV_PFXMATRIX_GRP0DST2SRC0CTRL0  =  80
WAVV_PFXMATRIX_GRP0DST2SRC0CTRL1  =  81
WAVV_PFXMATRIX_GRP0DST2SRC0CTRL2  =  82
WAVV_PFXMATRIX_GRP0DST2SRC1CTRL0  =  83
WAVV_PFXMATRIX_GRP0DST2SRC1CTRL1  =  84
WAVV_PFXMATRIX_GRP0DST2SRC1CTRL2  =  85
WAVV_PFXMATRIX_GRP0DST2SRC2CTRL0  =  86
WAVV_PFXMATRIX_GRP0DST2SRC2CTRL1  =  87
WAVV_PFXMATRIX_GRP0DST2SRC2CTRL2  =  88
WAVV_PFXMATRIX_GRP0DST2SRC3CTRL0  =  89
WAVV_PFXMATRIX_GRP0DST2SRC3CTRL1  =  90
WAVV_PFXMATRIX_GRP0DST2SRC3CTRL2  =  91
WAVV_PFXMATRIX_GRP0DST3SRC0CTRL0  =  92
WAVV_PFXMATRIX_GRP0DST3SRC0CTRL1  =  93
WAVV_PFXMATRIX_GRP0DST3SRC0CTRL2  =  94
WAVV_PFXMATRIX_GRP0DST3SRC1CTRL0  =  95
WAVV_PFXMATRIX_GRP0DST3SRC1CTRL1  =  96
WAVV_PFXMATRIX_GRP0DST3SRC1CTRL2  =  97
WAVV_PFXMATRIX_GRP0DST3SRC2CTRL0  =  98
WAVV_PFXMATRIX_GRP0DST3SRC2CTRL1  =  99
WAVV_PFXMATRIX_GRP0DST3SRC2CTRL2  =  100
WAVV_PFXMATRIX_GRP0DST3SRC3CTRL0  =  101
WAVV_PFXMATRIX_GRP0DST3SRC3CTRL1  =  102
WAVV_PFXMATRIX_GRP0DST3SRC3CTRL2  =  103
#End PolyFX Mod Matrix
WAYV_ADSR1_CHECKBOX  =  105
WAYV_ADSR2_CHECKBOX  =  106
WAYV_LFO_AMP  =  107
WAYV_LFO_PITCH  =  108
WAYV_PITCH_ENV_AMT  =  109
WAYV_OSC2_UNISON_VOICES  =  110
WAYV_OSC2_UNISON_SPREAD  =  111
WAYV_LFO_AMOUNT  =  112
WAYV_OSC3_TYPE  =  113
WAYV_OSC3_PITCH  =  114
WAYV_OSC3_TUNE  =  115
WAYV_OSC3_VOLUME  =  116
WAYV_OSC3_UNISON_VOICES  =  117
WAYV_OSC3_UNISON_SPREAD  =  118
WAYV_OSC1_FM1  =  119
WAYV_OSC1_FM2  =  120
WAYV_OSC1_FM3  =  121
WAYV_OSC2_FM1  =  122
WAYV_OSC2_FM2  =  123
WAYV_OSC2_FM3  =  124
WAYV_OSC3_FM1  =  125
WAYV_OSC3_FM2  =  126
WAYV_OSC3_FM3  =  127
WAYV_ATTACK3  =  128
WAYV_DECAY3  =  129
WAYV_SUSTAIN3  =  130
WAYV_RELEASE3  =  131
WAYV_ADSR3_CHECKBOX  =  132
WAYV_LAST_CONTROL_PORT  =  132
WAYV_COUNT  =  133 #must be 1 + highest value above CHANGE THIS IF YOU ADD OR TAKE AWAY ANYTHING
WAYV_POLYPHONY  =  16


#Ray-V

RAYV_OUTPUT0  =  0
RAYV_OUTPUT1  =  1
RAYV_FIRST_CONTROL_PORT  =  2
RAYV_ATTACK  =  2
RAYV_DECAY  =  3
RAYV_SUSTAIN  =  4
RAYV_RELEASE  =  5
RAYV_TIMBRE  =  6
RAYV_RES  =  7
RAYV_DIST  =  8
RAYV_FILTER_ATTACK  =  9
RAYV_FILTER_DECAY  =  10
RAYV_FILTER_SUSTAIN  =  11
RAYV_FILTER_RELEASE  =  12
RAYV_NOISE_AMP  =  13
RAYV_FILTER_ENV_AMT  =  14
RAYV_DIST_WET  =  15
RAYV_OSC1_TYPE  =  16
RAYV_OSC1_PITCH  =  17
RAYV_OSC1_TUNE  =  18
RAYV_OSC1_VOLUME  =  19
RAYV_OSC2_TYPE  =  20
RAYV_OSC2_PITCH  =  21
RAYV_OSC2_TUNE  =  22
RAYV_OSC2_VOLUME  =  23
RAYV_MASTER_VOLUME  =  24
RAYV_MASTER_UNISON_VOICES  =  25
RAYV_MASTER_UNISON_SPREAD  =  26
RAYV_MASTER_GLIDE  =  27
RAYV_MASTER_PITCHBEND_AMT  =  28
RAYV_PITCH_ENV_TIME  =  29
RAYV_PITCH_ENV_AMT  =  30
RAYV_LFO_FREQ  =  31
RAYV_LFO_TYPE  =  32
RAYV_LFO_AMP  =  33
RAYV_LFO_PITCH  =  34
RAYV_LFO_FILTER  =  35
RAYV_OSC_HARD_SYNC  =  36
RAYV_LAST_CONTROL_PORT  =  36
RAYV_POLYPHONY  =  16
RAYV_COUNT  =  37 #must be 1 + highest value above CHANGE THIS IF YOU ADD OR TAKE AWAY ANYTHING
