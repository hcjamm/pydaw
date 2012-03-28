/* -*- c-basic-offset: 4 -*-  vi:set ts=8 sts=4 sw=4: */

/* trivial_sampler.c

   DSSI Soft Synth Interface
   Constructed by Chris Cannam, Steve Harris and Sean Bolton

   A straightforward DSSI plugin sampler.

   This example file is in the public domain.
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
#define Sampler_BALANCE 6

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

//#define Sampler_Stereo_LABEL "stereo_sampler"
#define Sampler_Stereo_LABEL "Euphoria"


typedef struct {
    LADSPA_Data *output[2];
    LADSPA_Data *retune;
    LADSPA_Data *basePitch;
    LADSPA_Data *sustain;
    LADSPA_Data *release;
    LADSPA_Data *balance;
    int          channels;
    float       *sampleData[2];
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
