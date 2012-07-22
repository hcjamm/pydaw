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

//#define Sampler_RELEASE_MIN 0.001f
#define Sampler_RELEASE_MIN 0.01f
#define Sampler_RELEASE_MAX 2.0f

#define Sampler_NOTES 100
#define Sampler_NOTES_m1 99
#define Sampler_FRAMES_MAX 1048576
//Pad the end of samples with zeroes to ensure you don't get artifacts from samples that have no silence at the end
#define Sampler_Sample_Padding 100

typedef struct {
    LADSPA_Data *output[2];    
    LADSPA_Data *basePitch[LMS_MAX_SAMPLE_COUNT];
    LADSPA_Data *low_note[LMS_MAX_SAMPLE_COUNT];
    LADSPA_Data *high_note[LMS_MAX_SAMPLE_COUNT];
    LADSPA_Data *sample_vol[LMS_MAX_SAMPLE_COUNT];     //in decibels    
    LADSPA_Data *sampleStarts[LMS_MAX_SAMPLE_COUNT];
    LADSPA_Data *sampleEnds[LMS_MAX_SAMPLE_COUNT];
    LADSPA_Data *sample_vel_sens[LMS_MAX_SAMPLE_COUNT];
    LADSPA_Data *sample_vel_low[LMS_MAX_SAMPLE_COUNT];
    LADSPA_Data *sample_vel_high[LMS_MAX_SAMPLE_COUNT];
    LADSPA_Data *sample_pitch[LMS_MAX_SAMPLE_COUNT];
    LADSPA_Data *sample_tune[LMS_MAX_SAMPLE_COUNT];
    LADSPA_Data *sample_interpolation_mode[LMS_MAX_SAMPLE_COUNT];
    
    LADSPA_Data *selected_sample;
        
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
    LADSPA_Data *noise_type;
    
    LADSPA_Data *master_glide;
    LADSPA_Data *master_pb_amt;
    
    LADSPA_Data *pitch_env_time;
    
    LADSPA_Data *lfo_freq;
    LADSPA_Data *lfo_type;
        
    LADSPA_Data *global_midi_octaves_offset;
        
    //Corresponds to the actual knobs on the effects themselves, not the mod matrix
    LADSPA_Data *pfx_mod_knob[LMS_EFFECTS_GROUPS_COUNT][LMS_MODULAR_POLYFX_COUNT][LMS_CONTROLS_PER_MOD_EFFECT];
    
    LADSPA_Data *fx_combobox[LMS_EFFECTS_GROUPS_COUNT][LMS_MODULAR_POLYFX_COUNT];
        
    //PolyFX Mod Matrix
    //Corresponds to the mod matrix spinboxes
    LADSPA_Data *polyfx_mod_matrix[LMS_EFFECTS_GROUPS_COUNT][LMS_MODULAR_POLYFX_COUNT][LMS_MODULATOR_COUNT][LMS_CONTROLS_PER_MOD_EFFECT];
    
    //End from PolyFX Mod Matrix
    
    
    int         i_selected_sample;
    int          channels;
    int         sample_channels[LMS_TOTAL_SAMPLE_COUNT];
    float       sample_last_interpolated_value[LMS_MAX_SAMPLE_COUNT];
    float       *sampleData[2][LMS_TOTAL_SAMPLE_COUNT];
    size_t       sampleCount[LMS_TOTAL_SAMPLE_COUNT];        
    float       sampleStartPos[LMS_MAX_SAMPLE_COUNT];         
    float       sampleEndPos[LMS_MAX_SAMPLE_COUNT];
    float       sample_amp[LMS_MAX_SAMPLE_COUNT];     //linear, for multiplying
    /*TODO: Initialize these at startup*/
    int         sample_indexes[Sampler_NOTES][LMS_MAX_SAMPLE_COUNT];  //Sample indexes for each note to play
    int         sample_indexes_count[Sampler_NOTES]; //The count of sample indexes to iterate through
    float vel_sens_output[Sampler_NOTES][LMS_MAX_SAMPLE_COUNT];
    
    float adjusted_base_pitch[LMS_MAX_SAMPLE_COUNT];
    
    //For sample preview:
    int preview_sample_array_index;
    int preview_sample_max_length;  //Used to set the maximum time to preview a sample to an arbitrary number of samples
    
    /*TODO:  Deprecate these 2?*/
    int loaded_samples[LMS_MAX_SAMPLE_COUNT];
    int loaded_samples_count;
    int i_loaded_samples;
    /*Used as a boolean when determining if a sample has already been loaded*/
    int sample_is_loaded;    
    /*The index of the current sample being played*/
    int current_sample;
    
    int          sampleRate;
    float fs;    //From Ray-V
    float ratio; //Used per-sample;  If voices are ever multithreaded, this will need to be widened...
    float sample_rate_ratios[LMS_MAX_SAMPLE_COUNT];
    long         ons[Sampler_NOTES];
    long         offs[Sampler_NOTES];
    int         velocities[Sampler_NOTES];    
    t_int_frac_read_head * sample_read_heads[Sampler_NOTES][LMS_MAX_SAMPLE_COUNT];
    long         sampleNo;
    char        *projectDir;
    char*       sample_paths[LMS_TOTAL_SAMPLE_COUNT];    
    char*       sample_files;
    
    //PolyFX modulation streams    
    int polyfx_mod_ctrl_indexes[Sampler_NOTES][LMS_MODULAR_POLYFX_COUNT][(LMS_CONTROLS_PER_MOD_EFFECT * LMS_MODULATOR_COUNT)]; //The index of the control to mod, currently 0-2
    int polyfx_mod_counts[Sampler_NOTES][LMS_MODULAR_POLYFX_COUNT];  //How many polyfx_mod_ptrs to iterate through for the current note
    int polyfx_mod_src_index[Sampler_NOTES][LMS_MODULAR_POLYFX_COUNT][(LMS_CONTROLS_PER_MOD_EFFECT * LMS_MODULATOR_COUNT)];  //The index of the modulation source(LFO, ADSR, etc...) to multiply by
    float polyfx_mod_matrix_values[Sampler_NOTES][LMS_MODULAR_POLYFX_COUNT][(LMS_CONTROLS_PER_MOD_EFFECT * LMS_MODULATOR_COUNT)];  //The value of the mod_matrix knob, multiplied by .01
    
    //Active PolyFX to process
    int active_polyfx[Sampler_NOTES][LMS_MODULAR_POLYFX_COUNT];
    int active_polyfx_count[Sampler_NOTES];
    
    pthread_mutex_t mutex;
    t_mono_modules * mono_modules;
    t_amp * amp_ptr;
    t_pit_pitch_core * smp_pit_core;
    t_pit_ratio * smp_pit_ratio;
    t_ccm_midi_cc_map * midi_cc_map;    
    t_poly_voice * data[Sampler_NOTES];
    
    //iterators for iterating through their respective array dimensions
    int i_fx_grps;
    int i_dst;
    int i_src;    
    int i_ctrl;
    
    float amp;  //linear amplitude, from the master volume knob
    
    t_lin_interpolater * lin_interpolator;
    
    float sv_pitch_bend_value;
    float sv_last_note;  //For glide
} Sampler __attribute__((aligned(16)));


    
#endif
