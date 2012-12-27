/* 
 * File:   ports.h
 * Author: Jeff Hubbard
 * 
 * Separate the LADSPA/OSC ports into their own header file so Qt MOC will stop complaining
 *
 * Created on April 8, 2012, 2:27 PM
 */

#ifndef EUPHORIA_PORTS_H
#define	EUPHORIA_PORTS_H

//Delimits the file string sent with configure().  Also used in the file saving format
#define EUPHORIA_FILES_STRING_DELIMITER '|'
//When used in place of "|", it tells the sampler to load the sample even if it's already been loaded once.
#define EUPHORIA_FILES_STRING_RELOAD_DELIMITER '>'
/* defines used for the file saving format */

/*These indicate that the file paths are either relative to the directory the saved file is in(useful for distributed files), or absolute(useful for saving one's work on a local machine).*/
#define EUPHORIA_FILES_ATTRIBUTE_ABSOLUTE_PATH "<<><<|{Path:Absolute}|>><>>"
#define EUPHORIA_FILES_ATTRIBUTE_RELATIVE_PATH "<<><<|{Path:Relative}|>><>>"
//This indicates the beginning of the files section
#define EUPHORIA_FILE_FILES_TAG "<<><<|{Begin Files}|>><>>"
//Indicates the beginning of the controls section.  Must be followed by a controls identifier before attempting to load any control values
#define EUPHORIA_FILE_CONTROLS_TAG "<<><<|{Begin Controls}|>><>>"
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
#define EUPHORIA_FILE_CONTROLS_TAG_EUP_V1 "<<><<|{controls_identifier|app=lms_euphoria|version=1}|>><>>"
//This separates port/value pairs
#define EUPHORIA_FILE_PORT_VALUE_SEPARATOR ":"

/*Provide an arbitrary maximum number of samples the user can load*/
#define EUPHORIA_MAX_SAMPLE_COUNT 32
//+1 to LMS_MAX_SAMPLE_COUNT, the highest index is for the preview sample
#define EUPHORIA_TOTAL_SAMPLE_COUNT 33

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
//The number of mono_fx groups
#define EUPHORIA_MONO_FX_GROUPS_COUNT EUPHORIA_MAX_SAMPLE_COUNT
#define EUPHORIA_MONO_FX_COUNT 4

#define EUPHORIA_LABEL "Euphoria"

#define EUPHORIA_OUTPUT_LEFT 0
#define EUPHORIA_OUTPUT_RIGHT 1


/*LMS_FIRST_CONTROL_PORT is the first port used for controls such as knobs.  All control ports must be numbered continuously,
 as they are iterated through*/
#define EUPHORIA_FIRST_CONTROL_PORT 2   
#define EUPHORIA_SELECTED_SAMPLE 2
#define EUPHORIA_ATTACK  3
#define EUPHORIA_DECAY   4
#define EUPHORIA_SUSTAIN 5
#define EUPHORIA_RELEASE 6
#define EUPHORIA_FILTER_ATTACK  7
#define EUPHORIA_FILTER_DECAY   8
#define EUPHORIA_FILTER_SUSTAIN 9
#define EUPHORIA_FILTER_RELEASE 10
#define EUPHORIA_NOISE_AMP 11
#define EUPHORIA_MASTER_VOLUME 12
#define EUPHORIA_MASTER_GLIDE 13
#define EUPHORIA_MASTER_PITCHBEND_AMT 14
#define EUPHORIA_PITCH_ENV_TIME 15
#define EUPHORIA_LFO_FREQ 16
#define EUPHORIA_LFO_TYPE 17
//From Modulex
#define EUPHORIA_FX0_KNOB0  18
#define EUPHORIA_FX0_KNOB1 19
#define EUPHORIA_FX0_KNOB2  20
#define EUPHORIA_FX0_COMBOBOX 21
#define EUPHORIA_FX1_KNOB0  22
#define EUPHORIA_FX1_KNOB1  23
#define EUPHORIA_FX1_KNOB2  24
#define EUPHORIA_FX1_COMBOBOX 25
#define EUPHORIA_FX2_KNOB0  26
#define EUPHORIA_FX2_KNOB1  27
#define EUPHORIA_FX2_KNOB2  28
#define EUPHORIA_FX2_COMBOBOX 29
#define EUPHORIA_FX3_KNOB0  30
#define EUPHORIA_FX3_KNOB1  31
#define EUPHORIA_FX3_KNOB2  32
#define EUPHORIA_FX3_COMBOBOX 33
//PolyFX Mod Matrix
#define EUPHORIA_PFXMATRIX_FIRST_PORT 34

#define EUPHORIA_PFXMATRIX_GRP0DST0SRC0CTRL0  34
#define EUPHORIA_PFXMATRIX_GRP0DST0SRC0CTRL1  35
#define EUPHORIA_PFXMATRIX_GRP0DST0SRC0CTRL2  36
#define EUPHORIA_PFXMATRIX_GRP0DST0SRC1CTRL0  37
#define EUPHORIA_PFXMATRIX_GRP0DST0SRC1CTRL1  38
#define EUPHORIA_PFXMATRIX_GRP0DST0SRC1CTRL2  39
#define EUPHORIA_PFXMATRIX_GRP0DST0SRC2CTRL0  40
#define EUPHORIA_PFXMATRIX_GRP0DST0SRC2CTRL1  41
#define EUPHORIA_PFXMATRIX_GRP0DST0SRC2CTRL2  42
#define EUPHORIA_PFXMATRIX_GRP0DST0SRC3CTRL0  43
#define EUPHORIA_PFXMATRIX_GRP0DST0SRC3CTRL1  44
#define EUPHORIA_PFXMATRIX_GRP0DST0SRC3CTRL2  45
#define EUPHORIA_PFXMATRIX_GRP0DST1SRC0CTRL0  46
#define EUPHORIA_PFXMATRIX_GRP0DST1SRC0CTRL1  47
#define EUPHORIA_PFXMATRIX_GRP0DST1SRC0CTRL2  48
#define EUPHORIA_PFXMATRIX_GRP0DST1SRC1CTRL0  49
#define EUPHORIA_PFXMATRIX_GRP0DST1SRC1CTRL1  50
#define EUPHORIA_PFXMATRIX_GRP0DST1SRC1CTRL2  51
#define EUPHORIA_PFXMATRIX_GRP0DST1SRC2CTRL0  52
#define EUPHORIA_PFXMATRIX_GRP0DST1SRC2CTRL1  53
#define EUPHORIA_PFXMATRIX_GRP0DST1SRC2CTRL2  54
#define EUPHORIA_PFXMATRIX_GRP0DST1SRC3CTRL0  55
#define EUPHORIA_PFXMATRIX_GRP0DST1SRC3CTRL1  56
#define EUPHORIA_PFXMATRIX_GRP0DST1SRC3CTRL2  57
#define EUPHORIA_PFXMATRIX_GRP0DST2SRC0CTRL0  58
#define EUPHORIA_PFXMATRIX_GRP0DST2SRC0CTRL1  59
#define EUPHORIA_PFXMATRIX_GRP0DST2SRC0CTRL2  60
#define EUPHORIA_PFXMATRIX_GRP0DST2SRC1CTRL0  61
#define EUPHORIA_PFXMATRIX_GRP0DST2SRC1CTRL1  62
#define EUPHORIA_PFXMATRIX_GRP0DST2SRC1CTRL2  63
#define EUPHORIA_PFXMATRIX_GRP0DST2SRC2CTRL0  64
#define EUPHORIA_PFXMATRIX_GRP0DST2SRC2CTRL1  65
#define EUPHORIA_PFXMATRIX_GRP0DST2SRC2CTRL2  66
#define EUPHORIA_PFXMATRIX_GRP0DST2SRC3CTRL0  67
#define EUPHORIA_PFXMATRIX_GRP0DST2SRC3CTRL1  68
#define EUPHORIA_PFXMATRIX_GRP0DST2SRC3CTRL2  69
#define EUPHORIA_PFXMATRIX_GRP0DST3SRC0CTRL0  70
#define EUPHORIA_PFXMATRIX_GRP0DST3SRC0CTRL1  71
#define EUPHORIA_PFXMATRIX_GRP0DST3SRC0CTRL2  72
#define EUPHORIA_PFXMATRIX_GRP0DST3SRC1CTRL0  73
#define EUPHORIA_PFXMATRIX_GRP0DST3SRC1CTRL1  74
#define EUPHORIA_PFXMATRIX_GRP0DST3SRC1CTRL2  75
#define EUPHORIA_PFXMATRIX_GRP0DST3SRC2CTRL0  76
#define EUPHORIA_PFXMATRIX_GRP0DST3SRC2CTRL1  77
#define EUPHORIA_PFXMATRIX_GRP0DST3SRC2CTRL2  78
#define EUPHORIA_PFXMATRIX_GRP0DST3SRC3CTRL0  79
#define EUPHORIA_PFXMATRIX_GRP0DST3SRC3CTRL1  80
#define EUPHORIA_PFXMATRIX_GRP0DST3SRC3CTRL2  81

//End PolyFX Mod Matrix

#define EUPHORIA_NOISE_TYPE 82

/*This is the last control port, + 1, for zero-based iteration*/
#define EUPHORIA_LAST_REGULAR_CONTROL_PORT 83

/*The first port to use when enumerating the ports for mod_matrix controls.  All of the mod_matrix ports should be sequential, 
 * any additional ports should prepend this port number*/
#define EUPHORIA_FIRST_SAMPLE_TABLE_PORT 83

/*The range of ports for sample pitch*/
#define EUPHORIA_SAMPLE_PITCH_PORT_RANGE_MIN     EUPHORIA_FIRST_SAMPLE_TABLE_PORT
#define EUPHORIA_SAMPLE_PITCH_PORT_RANGE_MAX     (EUPHORIA_SAMPLE_PITCH_PORT_RANGE_MIN + EUPHORIA_MAX_SAMPLE_COUNT)

/*The range of ports for the low note to play a sample on*/
#define EUPHORIA_PLAY_PITCH_LOW_PORT_RANGE_MIN   EUPHORIA_SAMPLE_PITCH_PORT_RANGE_MAX
#define EUPHORIA_PLAY_PITCH_LOW_PORT_RANGE_MAX   (EUPHORIA_PLAY_PITCH_LOW_PORT_RANGE_MIN + EUPHORIA_MAX_SAMPLE_COUNT)

/*The range of ports for the high note to play a sample on*/
#define EUPHORIA_PLAY_PITCH_HIGH_PORT_RANGE_MIN  EUPHORIA_PLAY_PITCH_LOW_PORT_RANGE_MAX
#define EUPHORIA_PLAY_PITCH_HIGH_PORT_RANGE_MAX  (EUPHORIA_PLAY_PITCH_HIGH_PORT_RANGE_MIN + EUPHORIA_MAX_SAMPLE_COUNT)

/*The range of ports for sample volume*/
#define LMS_SAMPLE_VOLUME_PORT_RANGE_MIN    EUPHORIA_PLAY_PITCH_HIGH_PORT_RANGE_MAX
#define EUPHORIA_SAMPLE_VOLUME_PORT_RANGE_MAX    (LMS_SAMPLE_VOLUME_PORT_RANGE_MIN + EUPHORIA_MAX_SAMPLE_COUNT)

#define EUPHORIA_SAMPLE_START_PORT_RANGE_MIN    EUPHORIA_SAMPLE_VOLUME_PORT_RANGE_MAX
#define EUPHORIA_SAMPLE_START_PORT_RANGE_MAX    (EUPHORIA_SAMPLE_START_PORT_RANGE_MIN + EUPHORIA_MAX_SAMPLE_COUNT)

#define EUPHORIA_SAMPLE_END_PORT_RANGE_MIN    EUPHORIA_SAMPLE_START_PORT_RANGE_MAX
#define EUPHORIA_SAMPLE_END_PORT_RANGE_MAX    (EUPHORIA_SAMPLE_END_PORT_RANGE_MIN + EUPHORIA_MAX_SAMPLE_COUNT)

#define EUPHORIA_SAMPLE_VEL_SENS_PORT_RANGE_MIN    EUPHORIA_SAMPLE_END_PORT_RANGE_MAX
#define EUPHORIA_SAMPLE_VEL_SENS_PORT_RANGE_MAX    (EUPHORIA_SAMPLE_VEL_SENS_PORT_RANGE_MIN + EUPHORIA_MAX_SAMPLE_COUNT)

#define EUPHORIA_SAMPLE_VEL_LOW_PORT_RANGE_MIN    EUPHORIA_SAMPLE_VEL_SENS_PORT_RANGE_MAX
#define EUPHORIA_SAMPLE_VEL_LOW_PORT_RANGE_MAX    (EUPHORIA_SAMPLE_VEL_LOW_PORT_RANGE_MIN + EUPHORIA_MAX_SAMPLE_COUNT)

#define EUPHORIA_SAMPLE_VEL_HIGH_PORT_RANGE_MIN    EUPHORIA_SAMPLE_VEL_LOW_PORT_RANGE_MAX
#define EUPHORIA_SAMPLE_VEL_HIGH_PORT_RANGE_MAX    (EUPHORIA_SAMPLE_VEL_HIGH_PORT_RANGE_MIN + EUPHORIA_MAX_SAMPLE_COUNT)

#define EUPHORIA_PITCH_PORT_RANGE_MIN    EUPHORIA_SAMPLE_VEL_HIGH_PORT_RANGE_MAX
#define EUPHORIA_PITCH_PORT_RANGE_MAX    (EUPHORIA_PITCH_PORT_RANGE_MIN + EUPHORIA_MAX_SAMPLE_COUNT)

#define EUPHORIA_TUNE_PORT_RANGE_MIN    EUPHORIA_PITCH_PORT_RANGE_MAX
#define EUPHORIA_TUNE_PORT_RANGE_MAX    (EUPHORIA_TUNE_PORT_RANGE_MIN + EUPHORIA_MAX_SAMPLE_COUNT)

#define EUPHORIA_SAMPLE_INTERPOLATION_MODE_PORT_RANGE_MIN    EUPHORIA_TUNE_PORT_RANGE_MAX
#define EUPHORIA_SAMPLE_INTERPOLATION_MODE_PORT_RANGE_MAX    (EUPHORIA_SAMPLE_INTERPOLATION_MODE_PORT_RANGE_MIN + EUPHORIA_MAX_SAMPLE_COUNT)

#define EUPHORIA_SAMPLE_LOOP_START_PORT_RANGE_MIN    EUPHORIA_SAMPLE_INTERPOLATION_MODE_PORT_RANGE_MAX
#define EUPHORIA_SAMPLE_LOOP_START_PORT_RANGE_MAX    (EUPHORIA_SAMPLE_LOOP_START_PORT_RANGE_MIN + EUPHORIA_MAX_SAMPLE_COUNT)

#define EUPHORIA_SAMPLE_LOOP_END_PORT_RANGE_MIN    EUPHORIA_SAMPLE_LOOP_START_PORT_RANGE_MAX
#define EUPHORIA_SAMPLE_LOOP_END_PORT_RANGE_MAX    (EUPHORIA_SAMPLE_LOOP_END_PORT_RANGE_MIN + EUPHORIA_MAX_SAMPLE_COUNT)

#define EUPHORIA_SAMPLE_LOOP_MODE_PORT_RANGE_MIN    EUPHORIA_SAMPLE_LOOP_END_PORT_RANGE_MAX
#define EUPHORIA_SAMPLE_LOOP_MODE_PORT_RANGE_MAX    (EUPHORIA_SAMPLE_LOOP_MODE_PORT_RANGE_MIN + EUPHORIA_MAX_SAMPLE_COUNT)

//MonoFX0
#define EUPHORIA_MONO_FX0_KNOB0_PORT_RANGE_MIN    EUPHORIA_SAMPLE_LOOP_MODE_PORT_RANGE_MAX
#define EUPHORIA_MONO_FX0_KNOB0_PORT_RANGE_MAX    (EUPHORIA_MONO_FX0_KNOB0_PORT_RANGE_MIN + EUPHORIA_MONO_FX_GROUPS_COUNT)

#define EUPHORIA_MONO_FX0_KNOB1_PORT_RANGE_MIN    EUPHORIA_MONO_FX0_KNOB0_PORT_RANGE_MAX
#define EUPHORIA_MONO_FX0_KNOB1_PORT_RANGE_MAX    (EUPHORIA_MONO_FX0_KNOB1_PORT_RANGE_MIN + EUPHORIA_MONO_FX_GROUPS_COUNT)

#define EUPHORIA_MONO_FX0_KNOB2_PORT_RANGE_MIN    EUPHORIA_MONO_FX0_KNOB1_PORT_RANGE_MAX
#define EUPHORIA_MONO_FX0_KNOB2_PORT_RANGE_MAX    (EUPHORIA_MONO_FX0_KNOB2_PORT_RANGE_MIN + EUPHORIA_MONO_FX_GROUPS_COUNT)

#define EUPHORIA_MONO_FX0_COMBOBOX_PORT_RANGE_MIN    EUPHORIA_MONO_FX0_KNOB2_PORT_RANGE_MAX
#define EUPHORIA_MONO_FX0_COMBOBOX_PORT_RANGE_MAX    (EUPHORIA_MONO_FX0_COMBOBOX_PORT_RANGE_MIN + EUPHORIA_MONO_FX_GROUPS_COUNT)
//MonoFX1
#define EUPHORIA_MONO_FX1_KNOB0_PORT_RANGE_MIN    EUPHORIA_MONO_FX0_COMBOBOX_PORT_RANGE_MAX
#define EUPHORIA_MONO_FX1_KNOB0_PORT_RANGE_MAX    (EUPHORIA_MONO_FX1_KNOB0_PORT_RANGE_MIN + EUPHORIA_MONO_FX_GROUPS_COUNT)

#define EUPHORIA_MONO_FX1_KNOB1_PORT_RANGE_MIN    EUPHORIA_MONO_FX1_KNOB0_PORT_RANGE_MAX
#define EUPHORIA_MONO_FX1_KNOB1_PORT_RANGE_MAX    (EUPHORIA_MONO_FX1_KNOB1_PORT_RANGE_MIN + EUPHORIA_MONO_FX_GROUPS_COUNT)

#define EUPHORIA_MONO_FX1_KNOB2_PORT_RANGE_MIN    EUPHORIA_MONO_FX1_KNOB1_PORT_RANGE_MAX
#define EUPHORIA_MONO_FX1_KNOB2_PORT_RANGE_MAX    (EUPHORIA_MONO_FX1_KNOB2_PORT_RANGE_MIN + EUPHORIA_MONO_FX_GROUPS_COUNT)

#define EUPHORIA_MONO_FX1_COMBOBOX_PORT_RANGE_MIN    EUPHORIA_MONO_FX1_KNOB2_PORT_RANGE_MAX
#define EUPHORIA_MONO_FX1_COMBOBOX_PORT_RANGE_MAX    (EUPHORIA_MONO_FX1_COMBOBOX_PORT_RANGE_MIN + EUPHORIA_MONO_FX_GROUPS_COUNT)
//MonoFX2
#define EUPHORIA_MONO_FX2_KNOB0_PORT_RANGE_MIN    EUPHORIA_MONO_FX1_COMBOBOX_PORT_RANGE_MAX
#define EUPHORIA_MONO_FX2_KNOB0_PORT_RANGE_MAX    (EUPHORIA_MONO_FX2_KNOB0_PORT_RANGE_MIN + EUPHORIA_MONO_FX_GROUPS_COUNT)

#define EUPHORIA_MONO_FX2_KNOB1_PORT_RANGE_MIN    EUPHORIA_MONO_FX2_KNOB0_PORT_RANGE_MAX
#define EUPHORIA_MONO_FX2_KNOB1_PORT_RANGE_MAX    (EUPHORIA_MONO_FX2_KNOB1_PORT_RANGE_MIN + EUPHORIA_MONO_FX_GROUPS_COUNT)

#define EUPHORIA_MONO_FX2_KNOB2_PORT_RANGE_MIN    EUPHORIA_MONO_FX2_KNOB1_PORT_RANGE_MAX
#define EUPHORIA_MONO_FX2_KNOB2_PORT_RANGE_MAX    (EUPHORIA_MONO_FX2_KNOB2_PORT_RANGE_MIN + EUPHORIA_MONO_FX_GROUPS_COUNT)

#define EUPHORIA_MONO_FX2_COMBOBOX_PORT_RANGE_MIN    EUPHORIA_MONO_FX2_KNOB2_PORT_RANGE_MAX
#define EUPHORIA_MONO_FX2_COMBOBOX_PORT_RANGE_MAX    (EUPHORIA_MONO_FX2_COMBOBOX_PORT_RANGE_MIN + EUPHORIA_MONO_FX_GROUPS_COUNT)
//MonoFX3
#define EUPHORIA_MONO_FX3_KNOB0_PORT_RANGE_MIN    EUPHORIA_MONO_FX2_COMBOBOX_PORT_RANGE_MAX
#define EUPHORIA_MONO_FX3_KNOB0_PORT_RANGE_MAX    (EUPHORIA_MONO_FX3_KNOB0_PORT_RANGE_MIN + EUPHORIA_MONO_FX_GROUPS_COUNT)

#define EUPHORIA_MONO_FX3_KNOB1_PORT_RANGE_MIN    EUPHORIA_MONO_FX3_KNOB0_PORT_RANGE_MAX
#define EUPHORIA_MONO_FX3_KNOB1_PORT_RANGE_MAX    (EUPHORIA_MONO_FX3_KNOB1_PORT_RANGE_MIN + EUPHORIA_MONO_FX_GROUPS_COUNT)

#define EUPHORIA_MONO_FX3_KNOB2_PORT_RANGE_MIN    EUPHORIA_MONO_FX3_KNOB1_PORT_RANGE_MAX
#define EUPHORIA_MONO_FX3_KNOB2_PORT_RANGE_MAX    (EUPHORIA_MONO_FX3_KNOB2_PORT_RANGE_MIN + EUPHORIA_MONO_FX_GROUPS_COUNT)

#define EUPHORIA_MONO_FX3_COMBOBOX_PORT_RANGE_MIN    EUPHORIA_MONO_FX3_KNOB2_PORT_RANGE_MAX
#define EUPHORIA_MONO_FX3_COMBOBOX_PORT_RANGE_MAX    (EUPHORIA_MONO_FX3_COMBOBOX_PORT_RANGE_MIN + EUPHORIA_MONO_FX_GROUPS_COUNT)
//Sample FX Group
#define EUPHORIA_SAMPLE_MONO_FX_GROUP_PORT_RANGE_MIN    EUPHORIA_MONO_FX3_COMBOBOX_PORT_RANGE_MAX
#define EUPHORIA_SAMPLE_MONO_FX_GROUP_PORT_RANGE_MAX    (EUPHORIA_SAMPLE_MONO_FX_GROUP_PORT_RANGE_MIN + EUPHORIA_MAX_SAMPLE_COUNT)

#define EUPHORIA_PORT_COUNT                EUPHORIA_SAMPLE_MONO_FX_GROUP_PORT_RANGE_MAX

#endif	/* PORTS_H */

