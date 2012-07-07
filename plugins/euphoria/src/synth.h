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
#include "ports.h"
#include "../../libmodsynth/lib/amp.h"
#include "../../libmodsynth/lib/pitch_core.h"
#include "../../libmodsynth/lib/cc_map.h"
#include "libmodsynth.h"
#include "../../libmodsynth/lib/interpolate-linear.h"

#define Sampler_BASE_PITCH_MIN 0
// not 127, as we want 120/2 = 60 as the default:
#define Sampler_BASE_PITCH_MAX 120

//#define Sampler_RELEASE_MIN 0.001f
#define Sampler_RELEASE_MIN 0.01f
#define Sampler_RELEASE_MAX 2.0f

#define Sampler_NOTES 128
#define Sampler_FRAMES_MAX 1048576

typedef struct {
    LADSPA_Data *output[2];
    LADSPA_Data *retune;
    LADSPA_Data *basePitch[LMS_MAX_SAMPLE_COUNT];
    LADSPA_Data *low_note[LMS_MAX_SAMPLE_COUNT];
    LADSPA_Data *high_note[LMS_MAX_SAMPLE_COUNT];
    LADSPA_Data *sample_vol[LMS_MAX_SAMPLE_COUNT];     //in decibels    
    LADSPA_Data *sampleStarts[LMS_MAX_SAMPLE_COUNT];
    LADSPA_Data *sampleEnds[LMS_MAX_SAMPLE_COUNT];
    LADSPA_Data *sample_vel_sens[LMS_MAX_SAMPLE_COUNT];
    LADSPA_Data *sample_vel_low[LMS_MAX_SAMPLE_COUNT];
    LADSPA_Data *sample_vel_high[LMS_MAX_SAMPLE_COUNT];
    LADSPA_Data *selected_sample;
    
    //From Ray-V
    LADSPA_Data *attack;
    LADSPA_Data *decay;
    LADSPA_Data *sustain;
    LADSPA_Data *release;
    LADSPA_Data pitch;
    
    LADSPA_Data *attack_f;
    LADSPA_Data *decay_f;
    LADSPA_Data *sustain_f;
    LADSPA_Data *release_f;
    
    LADSPA_Data *master_vol;
    
    LADSPA_Data *noise_amp;    
    
    LADSPA_Data *master_glide;
    LADSPA_Data *master_pb_amt;
    
    LADSPA_Data *pitch_env_amt;
    LADSPA_Data *pitch_env_time;
    
    LADSPA_Data *lfo_freq;
    LADSPA_Data *lfo_type;
    
    //End from Ray-V
    
    //From Modulex
    
    //Corresponds to the mod matrix knobs
    LADSPA_Data *pfx_mod_knob[LMS_EFFECTS_GROUPS_COUNT][LMS_MODULAR_POLYFX_COUNT][LMS_CONTROLS_PER_MOD_EFFECT];
    
    LADSPA_Data *fx_combobox[LMS_EFFECTS_GROUPS_COUNT][LMS_MODULAR_POLYFX_COUNT];
        
    //End from Modulex
    
    //PolyFX Mod Matrix
    //Corresponds to the actual knobs on the effects themselves, not the mod matrix
    LADSPA_Data *polyfx_mod_matrix[LMS_EFFECTS_GROUPS_COUNT][LMS_MODULAR_POLYFX_COUNT][LMS_MODULATOR_COUNT][LMS_CONTROLS_PER_MOD_EFFECT];
    
    //End from PolyFX Mod Matrix
    
    
    int         i_selected_sample;
    int          channels;
    float       *sampleData[2][LMS_MAX_SAMPLE_COUNT];
    size_t       sampleCount[LMS_MAX_SAMPLE_COUNT];        
    float       sampleStartPos[LMS_MAX_SAMPLE_COUNT];         
    float       sampleEndPos[LMS_MAX_SAMPLE_COUNT];
    float       sample_amp[LMS_MAX_SAMPLE_COUNT];     //linear, for multiplying
    /*TODO: Initialize these at startup*/
    int         sample_indexes[Sampler_NOTES][LMS_MAX_SAMPLE_COUNT];  //Sample indexes for each note to play
    int         sample_indexes_count[Sampler_NOTES]; //The count of sample indexes to iterate through
    
    /*TODO:  Deprecate these 2?*/
    int loaded_samples[LMS_MAX_SAMPLE_COUNT];
    int loaded_samples_count;
    int i_loaded_samples;
    /*Used as a boolean when determining if a sample has already been loaded*/
    int sample_is_loaded;    
    /*The current sample being played*/
    int current_sample;
    
    int          sampleRate;
    float fs;    //From Ray-V
    long         ons[Sampler_NOTES];
    long         offs[Sampler_NOTES];
    int         velocities[Sampler_NOTES];
    float       sample_position[Sampler_NOTES][LMS_MAX_SAMPLE_COUNT];
    long         sampleNo;
    char        *projectDir;
    char*       sample_paths[LMS_MAX_SAMPLE_COUNT];    
    char*       sample_files;
    pthread_mutex_t mutex;
    t_mono_modules * mono_modules;
    t_amp * amp_ptr;
    t_pit_pitch_core * smp_pit_core[LMS_MAX_SAMPLE_COUNT];
    t_pit_ratio * smp_pit_ratio[LMS_MAX_SAMPLE_COUNT];
    t_ccm_midi_cc_map * midi_cc_map;    
    t_poly_voice * data[Sampler_NOTES];
    
    //iterators for iterating through their respective array dimensions
    int i_fx_grps;
    int i_src;
    int i_dst;
    int i_ctrl;
    
    float amp;  //linear amplitude, from the master volume knob
    
    t_lin_interpolater * lin_interpolator;
    
    float sv_pitch_bend_value;
    float sv_last_note;  //For glide
} Sampler;

#endif
