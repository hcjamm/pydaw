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
#define LMS_LFO_AMP 19
#define LMS_LFO_PITCH 20
#define LMS_LFO_FILTER 21
//From Modulex
#define LMS_FX0_KNOB0  22
#define LMS_FX0_KNOB1 23
#define LMS_FX0_KNOB2  24
#define LMS_FX0_COMBOBOX 25
#define LMS_FX1_KNOB0  26
#define LMS_FX1_KNOB1  27
#define LMS_FX1_KNOB2  28
#define LMS_FX1_COMBOBOX 29
#define LMS_FX2_KNOB0  30
#define LMS_FX2_KNOB1  31
#define LMS_FX2_KNOB2  32
#define LMS_FX2_COMBOBOX 33
#define LMS_FX3_KNOB0  34
#define LMS_FX3_KNOB1  35
#define LMS_FX3_KNOB2  36
#define LMS_FX3_COMBOBOX 37
//PolyFX Mod Matrix

//End PolyFX Mod Matrix
/*This is the last control port*/
#define LMS_LAST_CONTROL_PORT 37  //TODO:  Is this obsolete now with the below?
#define LMS_COUNT 38 /* must be 1 + highest value above CHANGE THIS IF YOU ADD OR TAKE AWAY ANYTHING*/

/*The first port to use when enumerating the ports for mod_matrix controls.  All of the mod_matrix ports should be sequential, 
 * any additional ports should prepend this port number*/
#define LMS_FIRST_MOD_MATRIX_PORT 38

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

