/* 
 * File:   ports.h
 * Author: Jeff Hubbard
 * 
 * Separate the LADSPA/OSC ports into their own header file so Qt MOC will stop complaining
 *
 * Created on April 8, 2012, 2:27 PM
 */

#ifndef PORTS_H
#define	PORTS_H

//Delimits the file string sent with configure().  Also used in the file saving format
#define LMS_FILES_STRING_DELIMITER '|'
//When used in place of "|", it tells the sampler to load the sample even if it's already been loaded once.
#define LMS_FILES_STRING_RELOAD_DELIMITER '>'
/* defines used for the file saving format */

/*These indicate that the file paths are either relative to the directory the saved file is in(useful for distributed files), or absolute(useful for saving one's work on a local machine).*/
#define LMS_FILES_ATTRIBUTE_ABSOLUTE_PATH "<<><<|{Path:Absolute}|>><>>"
#define LMS_FILES_ATTRIBUTE_RELATIVE_PATH "<<><<|{Path:Relative}|>><>>"
//This indicates the beginning of the files section
#define LMS_FILE_FILES_TAG "<<><<|{Begin Files}|>><>>"
//Indicates the beginning of the controls section.  Must be followed by a controls identifier before attempting to load any control values
#define LMS_FILE_CONTROLS_TAG "<<><<|{Begin Controls}|>><>>"
/*This indicates the beginning of the controls section for Euphoria, version 1 of Euphoria's control ports. 
 If you implement this file format in another application, you must come up with your own unique app.  If you 
 then change the control ports by adding/remove controls or changing their range or order, you should increment 
 your version number.  The idea is, you can't set one sampler's control's from another's, nor would it even be
 a good idea to try.  You can, however, merge the control sections from multiple files supporting this format,
 as a particular version of a particular sampler should only try to load controls from it's own control section,
 and only if it's a compatible version.  All others should be ignored.  Example of a multi-instrument file:
 
 <<><<|{Path:Relative}|>><>>
 0|samples/sample1.wav
 ...
 <<><<|{Begin Controls}|>><>>
 <<><<|{controls_identifier|app=lms_euphoria|version=1}|>><>>
 2:100
 3:50
 ...
 <<><<|{Begin Controls}|>><>>
 <<><<|{controls_identifier|app=super_duper_sampler|version=6}|>><>>
 2:780
 3:5000
 ...
 */
#define LMS_FILE_CONTROLS_TAG_EUP_V1 "<<><<|{controls_identifier|app=lms_euphoria|version=1}|>><>>"
//This separates port/value pairs
#define LMS_FILE_PORT_VALUE_SEPARATOR ":"

/*Provide an arbitrary maximum number of samples the user can load*/
#define LMS_MAX_SAMPLE_COUNT 32

//Total number of LFOs, ADSRs, other envelopes, etc...  Used for the PolyFX mod matrix
#define LMS_MODULATOR_COUNT 4
//How many modular PolyFX
#define LMS_MODULAR_POLYFX_COUNT 4
//How many ports per PolyFX, 3 knobs and a combobox
#define LMS_PORTS_PER_MOD_EFFECT 4
//How many knobs per PolyFX, 3 knobs
#define LMS_CONTROLS_PER_MOD_EFFECT 3
//How many groups of effects.  This will become useful when each sample has an "effects group" choice
#define LMS_EFFECTS_GROUPS_COUNT 1

#define Sampler_Stereo_LABEL "Euphoria"

#define Sampler_OUTPUT_LEFT 0
#define Sampler_OUTPUT_RIGHT 1


/*LMS_FIRST_CONTROL_PORT is the first port used for controls such as knobs.  All control ports must be numbered continuously,
 as they are iterated through*/
#define LMS_FIRST_CONTROL_PORT 2   
#define Sampler_SELECTED_SAMPLE 2
#define LMS_ATTACK  3
#define LMS_DECAY   4
#define LMS_SUSTAIN 5
#define LMS_RELEASE 6
#define LMS_FILTER_ATTACK  7
#define LMS_FILTER_DECAY   8
#define LMS_FILTER_SUSTAIN 9
#define LMS_FILTER_RELEASE 10
#define LMS_NOISE_AMP 11
#define LMS_MASTER_VOLUME 12
#define LMS_MASTER_GLIDE 13
#define LMS_MASTER_PITCHBEND_AMT 14
#define LMS_PITCH_ENV_TIME 15
#define LMS_PITCH_ENV_AMT 16
#define LMS_LFO_FREQ 17
#define LMS_LFO_TYPE 18
//From Modulex
#define LMS_FX0_KNOB0  19
#define LMS_FX0_KNOB1 20
#define LMS_FX0_KNOB2  21
#define LMS_FX0_COMBOBOX 22
#define LMS_FX1_KNOB0  23
#define LMS_FX1_KNOB1  24
#define LMS_FX1_KNOB2  25
#define LMS_FX1_COMBOBOX 26
#define LMS_FX2_KNOB0  27
#define LMS_FX2_KNOB1  28
#define LMS_FX2_KNOB2  29
#define LMS_FX2_COMBOBOX 30
#define LMS_FX3_KNOB0  31
#define LMS_FX3_KNOB1  32
#define LMS_FX3_KNOB2  33
#define LMS_FX3_COMBOBOX 34
//PolyFX Mod Matrix
#define LMS_PFXMATRIX_GRP0DST0SRC0CTRL0  35
#define LMS_PFXMATRIX_GRP0DST0SRC0CTRL1  36
#define LMS_PFXMATRIX_GRP0DST0SRC0CTRL2  37
#define LMS_PFXMATRIX_GRP0DST0SRC1CTRL0  38
#define LMS_PFXMATRIX_GRP0DST0SRC1CTRL1  39
#define LMS_PFXMATRIX_GRP0DST0SRC1CTRL2  40
#define LMS_PFXMATRIX_GRP0DST0SRC2CTRL0  41
#define LMS_PFXMATRIX_GRP0DST0SRC2CTRL1  42
#define LMS_PFXMATRIX_GRP0DST0SRC2CTRL2  43
#define LMS_PFXMATRIX_GRP0DST0SRC3CTRL0  44
#define LMS_PFXMATRIX_GRP0DST0SRC3CTRL1  45
#define LMS_PFXMATRIX_GRP0DST0SRC3CTRL2  46
#define LMS_PFXMATRIX_GRP0DST1SRC0CTRL0  47
#define LMS_PFXMATRIX_GRP0DST1SRC0CTRL1  48
#define LMS_PFXMATRIX_GRP0DST1SRC0CTRL2  49
#define LMS_PFXMATRIX_GRP0DST1SRC1CTRL0  50
#define LMS_PFXMATRIX_GRP0DST1SRC1CTRL1  51
#define LMS_PFXMATRIX_GRP0DST1SRC1CTRL2  52
#define LMS_PFXMATRIX_GRP0DST1SRC2CTRL0  53
#define LMS_PFXMATRIX_GRP0DST1SRC2CTRL1  54
#define LMS_PFXMATRIX_GRP0DST1SRC2CTRL2  55
#define LMS_PFXMATRIX_GRP0DST1SRC3CTRL0  56
#define LMS_PFXMATRIX_GRP0DST1SRC3CTRL1  57
#define LMS_PFXMATRIX_GRP0DST1SRC3CTRL2  58
#define LMS_PFXMATRIX_GRP0DST2SRC0CTRL0  59
#define LMS_PFXMATRIX_GRP0DST2SRC0CTRL1  60
#define LMS_PFXMATRIX_GRP0DST2SRC0CTRL2  61
#define LMS_PFXMATRIX_GRP0DST2SRC1CTRL0  62
#define LMS_PFXMATRIX_GRP0DST2SRC1CTRL1  63
#define LMS_PFXMATRIX_GRP0DST2SRC1CTRL2  64
#define LMS_PFXMATRIX_GRP0DST2SRC2CTRL0  65
#define LMS_PFXMATRIX_GRP0DST2SRC2CTRL1  66
#define LMS_PFXMATRIX_GRP0DST2SRC2CTRL2  67
#define LMS_PFXMATRIX_GRP0DST2SRC3CTRL0  68
#define LMS_PFXMATRIX_GRP0DST2SRC3CTRL1  69
#define LMS_PFXMATRIX_GRP0DST2SRC3CTRL2  70
#define LMS_PFXMATRIX_GRP0DST3SRC0CTRL0  71
#define LMS_PFXMATRIX_GRP0DST3SRC0CTRL1  72
#define LMS_PFXMATRIX_GRP0DST3SRC0CTRL2  73
#define LMS_PFXMATRIX_GRP0DST3SRC1CTRL0  74
#define LMS_PFXMATRIX_GRP0DST3SRC1CTRL1  75
#define LMS_PFXMATRIX_GRP0DST3SRC1CTRL2  76
#define LMS_PFXMATRIX_GRP0DST3SRC2CTRL0  77
#define LMS_PFXMATRIX_GRP0DST3SRC2CTRL1  78
#define LMS_PFXMATRIX_GRP0DST3SRC2CTRL2  79
#define LMS_PFXMATRIX_GRP0DST3SRC3CTRL0  80
#define LMS_PFXMATRIX_GRP0DST3SRC3CTRL1  81
#define LMS_PFXMATRIX_GRP0DST3SRC3CTRL2  82

//End PolyFX Mod Matrix
/*This is the last control port*/
#define LMS_LAST_CONTROL_PORT 82  //TODO:  Is this obsolete now with the below?
#define LMS_COUNT 83 /* must be 1 + highest value above CHANGE THIS IF YOU ADD OR TAKE AWAY ANYTHING*/

/*The first port to use when enumerating the ports for mod_matrix controls.  All of the mod_matrix ports should be sequential, 
 * any additional ports should prepend this port number*/
#define LMS_FIRST_MOD_MATRIX_PORT 83

/*The range of ports for sample pitch*/
#define LMS_SAMPLE_PITCH_PORT_RANGE_MIN     LMS_FIRST_MOD_MATRIX_PORT
#define LMS_SAMPLE_PITCH_PORT_RANGE_MAX     (LMS_SAMPLE_PITCH_PORT_RANGE_MIN + LMS_MAX_SAMPLE_COUNT)

/*The range of ports for the low note to play a sample on*/
#define LMS_PLAY_PITCH_LOW_PORT_RANGE_MIN   LMS_SAMPLE_PITCH_PORT_RANGE_MAX
#define LMS_PLAY_PITCH_LOW_PORT_RANGE_MAX   (LMS_PLAY_PITCH_LOW_PORT_RANGE_MIN + LMS_MAX_SAMPLE_COUNT)

/*The range of ports for the high note to play a sample on*/
#define LMS_PLAY_PITCH_HIGH_PORT_RANGE_MIN  LMS_PLAY_PITCH_LOW_PORT_RANGE_MAX
#define LMS_PLAY_PITCH_HIGH_PORT_RANGE_MAX  (LMS_PLAY_PITCH_HIGH_PORT_RANGE_MIN + LMS_MAX_SAMPLE_COUNT)

/*The range of ports for sample volume*/
#define LMS_SAMPLE_VOLUME_PORT_RANGE_MIN    LMS_PLAY_PITCH_HIGH_PORT_RANGE_MAX
#define LMS_SAMPLE_VOLUME_PORT_RANGE_MAX    (LMS_SAMPLE_VOLUME_PORT_RANGE_MIN + LMS_MAX_SAMPLE_COUNT)

#define LMS_SAMPLE_START_PORT_RANGE_MIN    LMS_SAMPLE_VOLUME_PORT_RANGE_MAX
#define LMS_SAMPLE_START_PORT_RANGE_MAX    (LMS_SAMPLE_START_PORT_RANGE_MIN + LMS_MAX_SAMPLE_COUNT)

#define LMS_SAMPLE_END_PORT_RANGE_MIN    LMS_SAMPLE_START_PORT_RANGE_MAX
#define LMS_SAMPLE_END_PORT_RANGE_MAX    (LMS_SAMPLE_END_PORT_RANGE_MIN + LMS_MAX_SAMPLE_COUNT)

#define LMS_SAMPLE_VEL_SENS_PORT_RANGE_MIN    LMS_SAMPLE_END_PORT_RANGE_MAX
#define LMS_SAMPLE_VEL_SENS_PORT_RANGE_MAX    (LMS_SAMPLE_VEL_SENS_PORT_RANGE_MIN + LMS_MAX_SAMPLE_COUNT)

#define LMS_SAMPLE_VEL_LOW_PORT_RANGE_MIN    LMS_SAMPLE_VEL_SENS_PORT_RANGE_MAX
#define LMS_SAMPLE_VEL_LOW_PORT_RANGE_MAX    (LMS_SAMPLE_VEL_LOW_PORT_RANGE_MIN + LMS_MAX_SAMPLE_COUNT)

#define LMS_SAMPLE_VEL_HIGH_PORT_RANGE_MIN    LMS_SAMPLE_VEL_LOW_PORT_RANGE_MAX
#define LMS_SAMPLE_VEL_HIGH_PORT_RANGE_MAX    (LMS_SAMPLE_VEL_HIGH_PORT_RANGE_MIN + LMS_MAX_SAMPLE_COUNT)

#define Sampler_Stereo_COUNT                LMS_SAMPLE_VEL_HIGH_PORT_RANGE_MAX

#endif	/* PORTS_H */

