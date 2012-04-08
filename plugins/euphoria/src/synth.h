/* -*- c-basic-offset: 4 -*-  vi:set ts=8 sts=4 sw=4: */

/* synth.h
   
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef _TRIVIAL_SAMPLER_H_
#define _TRIVIAL_SAMPLER_H_

#include <ladspa.h>

#define Sampler_OUTPUT_LEFT 0
#define Sampler_RETUNE 1
#define Sampler_BASE_PITCH 2
#define Sampler_SUSTAIN 3
#define Sampler_RELEASE 4

#define Sampler_OUTPUT_RIGHT 5
//#define Sampler_BALANCE 6
#define Sampler_SELECTED_SAMPLE 6

/*Provide an arbitrary maximum number of samples the user can load*/
#define LMS_MAX_SAMPLE_COUNT 32

/*The first port to use when enumerating the ports for mod_matrix controls.  All of the mod_matrix ports should be sequential, 
 * any additional ports should prepend this port number*/
#define LMS_FIRST_MOD_MATRIX_PORT 7

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

#define Sampler_BASE_PITCH_MIN 0
// not 127, as we want 120/2 = 60 as the default:
#define Sampler_BASE_PITCH_MAX 120

//#define Sampler_RELEASE_MIN 0.001f
#define Sampler_RELEASE_MIN 0.01f
#define Sampler_RELEASE_MAX 2.0f

#define Sampler_NOTES 128
#define Sampler_FRAMES_MAX 1048576

#define Sampler_Stereo_LABEL "Euphoria"


typedef struct {
    LADSPA_Data *output[2];
    LADSPA_Data *retune;
    LADSPA_Data *basePitch[LMS_MAX_SAMPLE_COUNT];
    LADSPA_Data *low_note[LMS_MAX_SAMPLE_COUNT];
    LADSPA_Data *high_note[LMS_MAX_SAMPLE_COUNT];
    LADSPA_Data *sample_vol[LMS_MAX_SAMPLE_COUNT];
    LADSPA_Data *sustain;
    LADSPA_Data *release;
    //LADSPA_Data *balance;
    LADSPA_Data *selected_sample;
    
    int         i_selected_sample;
    int          channels;
    float       *sampleData[2][LMS_MAX_SAMPLE_COUNT];
    size_t       sampleCount[LMS_MAX_SAMPLE_COUNT];
    
    int loaded_samples[LMS_MAX_SAMPLE_COUNT];
    int loaded_samples_count;
    int i_loaded_samples;
    /*Used as a boolean when determining if a sample has already been loaded*/
    int sample_is_loaded;    
    /*The current sample being played*/
    int current_sample;
    
    int          sampleRate;
    long         ons[Sampler_NOTES];
    long         offs[Sampler_NOTES];
    char         velocities[Sampler_NOTES];
    long         sampleNo;
    char        *projectDir;
    pthread_mutex_t mutex;    
} Sampler;

#endif
