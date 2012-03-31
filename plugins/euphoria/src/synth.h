/* -*- c-basic-offset: 4 -*-  vi:set ts=8 sts=4 sw=4: */

/* trivial_sampler.c
   
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
#define Sampler_Stereo_COUNT 7

#define Sampler_BASE_PITCH_MIN 0
// not 127, as we want 120/2 = 60 as the default:
#define Sampler_BASE_PITCH_MAX 120

//#define Sampler_RELEASE_MIN 0.001f
#define Sampler_RELEASE_MIN 0.01f
#define Sampler_RELEASE_MAX 2.0f

#define Sampler_NOTES 128
#define Sampler_FRAMES_MAX 1048576

/*Provide an arbitrary maximum number of samples the user can load*/
#define LMS_MAX_SAMPLE_COUNT 32
/*Temporary placeholder, to be removed*/
//#define LMS_ZERO_INDEX 0

//#define Sampler_Stereo_LABEL "stereo_sampler"
#define Sampler_Stereo_LABEL "Euphoria"


typedef struct {
    LADSPA_Data *output[2];
    LADSPA_Data *retune;
    LADSPA_Data *basePitch;
    LADSPA_Data *sustain;
    LADSPA_Data *release;
    //LADSPA_Data *balance;
    LADSPA_Data *selected_sample;
    int         i_selected_sample;
    int          channels;
    float       *sampleData[2][LMS_MAX_SAMPLE_COUNT];
    size_t       sampleCount;
    int          sampleRate;
    long         ons[Sampler_NOTES];
    long         offs[Sampler_NOTES];
    char         velocities[Sampler_NOTES];
    long         sampleNo;
    char        *projectDir;
    pthread_mutex_t mutex;    
} Sampler;

#endif
