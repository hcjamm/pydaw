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

#define LMS_FILES_STRING_DELIMITER "|"
/* defines used for the file saving format */
//This indicates the beginning of the controls section
#define LMS_FILE_FILES_TAG "<<><<|{Begin Files}|>><>>"
#define LMS_FILE_CONTROLS_TAG "<<><<|{Begin Controls}|>><>>"
//This separates port/value pairs
#define LMS_FILE_PORT_VALUE_SEPARATOR ":"

/**/
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
#define LMS_TIMBRE  7
#define LMS_RES  8
#define LMS_DIST 9
#define LMS_FILTER_ATTACK  10
#define LMS_FILTER_DECAY   11
#define LMS_FILTER_SUSTAIN 12
#define LMS_FILTER_RELEASE 13
#define LMS_NOISE_AMP 14
#define LMS_FILTER_ENV_AMT 15
#define LMS_DIST_WET 16
#define LMS_MASTER_VOLUME 17
#define LMS_MASTER_UNISON_VOICES 18
#define LMS_MASTER_UNISON_SPREAD 19
#define LMS_MASTER_GLIDE 20
#define LMS_MASTER_PITCHBEND_AMT 21
#define LMS_PITCH_ENV_TIME 22
#define LMS_PITCH_ENV_AMT 23
#define LMS_LFO_FREQ 24
#define LMS_LFO_TYPE 25
#define LMS_LFO_AMP 26
#define LMS_LFO_PITCH 27
#define LMS_LFO_FILTER 28
#define LMS_FILTER_TYPE 29
/*This is the last control port*/
#define LMS_LAST_CONTROL_PORT 29  //TODO:  Is this obsolete now with the below?
#define LMS_COUNT 30 /* must be 1 + highest value above CHANGE THIS IF YOU ADD OR TAKE AWAY ANYTHING*/

/*Provide an arbitrary maximum number of samples the user can load*/
#define LMS_MAX_SAMPLE_COUNT 32

/*The first port to use when enumerating the ports for mod_matrix controls.  All of the mod_matrix ports should be sequential, 
 * any additional ports should prepend this port number*/
#define LMS_FIRST_MOD_MATRIX_PORT 30

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

#define Sampler_Stereo_COUNT                LMS_SAMPLE_VOLUME_PORT_RANGE_MAX

#endif	/* PORTS_H */

