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

#ifndef EUPHORIA_SYNTH_H
#define EUPHORIA_SYNTH_H

#include <ladspa.h>
#include "ports.h"
#include "../../libmodsynth/lib/amp.h"
#include "../../libmodsynth/lib/pitch_core.h"
#include "../../libmodsynth/lib/cc_map.h"
#include "libmodsynth.h"
#include "../../libmodsynth/lib/interpolate-linear.h"

#define EUPHORIA_NOTES 100
#define EUPHORIA_NOTES_m1 99
#define EUPHORIA_FRAMES_MAX 1048576
//Pad the end of samples with zeroes to ensure you don't get artifacts from samples that have no silence at the end
#define EUPHORIA_Sample_Padding 100

//How many buffers in between slow indexing operations.  Buffer == users soundcard latency settings, ie: 512 samples
#define EUPHORIA_SLOW_INDEX_COUNT 64

typedef struct {
    LADSPA_Data *output[2];    
    LADSPA_Data *basePitch[EUPHORIA_MAX_SAMPLE_COUNT];
    LADSPA_Data *low_note[EUPHORIA_MAX_SAMPLE_COUNT];
    LADSPA_Data *high_note[EUPHORIA_MAX_SAMPLE_COUNT];
    LADSPA_Data *sample_vol[EUPHORIA_MAX_SAMPLE_COUNT];     //in decibels    
    LADSPA_Data *sampleStarts[EUPHORIA_MAX_SAMPLE_COUNT];
    LADSPA_Data *sampleEnds[EUPHORIA_MAX_SAMPLE_COUNT];
    LADSPA_Data *sampleLoopStarts[EUPHORIA_MAX_SAMPLE_COUNT];
    LADSPA_Data *sampleLoopEnds[EUPHORIA_MAX_SAMPLE_COUNT];
    LADSPA_Data *sampleLoopModes[EUPHORIA_MAX_SAMPLE_COUNT];
    LADSPA_Data *sample_vel_sens[EUPHORIA_MAX_SAMPLE_COUNT];
    LADSPA_Data *sample_vel_low[EUPHORIA_MAX_SAMPLE_COUNT];
    LADSPA_Data *sample_vel_high[EUPHORIA_MAX_SAMPLE_COUNT];
    LADSPA_Data *sample_pitch[EUPHORIA_MAX_SAMPLE_COUNT];
    LADSPA_Data *sample_tune[EUPHORIA_MAX_SAMPLE_COUNT];
    LADSPA_Data *sample_interpolation_mode[EUPHORIA_MAX_SAMPLE_COUNT];
    
    LADSPA_Data *mfx_knobs[EUPHORIA_MONO_FX_GROUPS_COUNT][EUPHORIA_MONO_FX_COUNT][EUPHORIA_CONTROLS_PER_MOD_EFFECT];
    LADSPA_Data *mfx_comboboxes[EUPHORIA_MONO_FX_GROUPS_COUNT][EUPHORIA_MONO_FX_COUNT];
    
    //The MonoFX group selected for each sample
    LADSPA_Data *sample_mfx_groups[EUPHORIA_MAX_SAMPLE_COUNT];
    
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
    LADSPA_Data *pfx_mod_knob[EUPHORIA_EFFECTS_GROUPS_COUNT][EUPHORIA_MODULAR_POLYFX_COUNT][EUPHORIA_CONTROLS_PER_MOD_EFFECT];
    
    LADSPA_Data *fx_combobox[EUPHORIA_EFFECTS_GROUPS_COUNT][EUPHORIA_MODULAR_POLYFX_COUNT];
        
    //PolyFX Mod Matrix
    //Corresponds to the mod matrix spinboxes
    LADSPA_Data *polyfx_mod_matrix[EUPHORIA_EFFECTS_GROUPS_COUNT][EUPHORIA_MODULAR_POLYFX_COUNT][EUPHORIA_MODULATOR_COUNT][EUPHORIA_CONTROLS_PER_MOD_EFFECT];
    
    //End from PolyFX Mod Matrix
    
    
    int         i_selected_sample;
    int          channels;
    int         sample_channels[EUPHORIA_TOTAL_SAMPLE_COUNT];
    float       sample_last_interpolated_value[EUPHORIA_MAX_SAMPLE_COUNT];
    float       *sampleData[2][EUPHORIA_TOTAL_SAMPLE_COUNT];
    size_t       sampleCount[EUPHORIA_TOTAL_SAMPLE_COUNT];        
    float       sampleStartPos[EUPHORIA_MAX_SAMPLE_COUNT];         
    float       sampleEndPos[EUPHORIA_MAX_SAMPLE_COUNT];
    float       sampleLoopStartPos[EUPHORIA_MAX_SAMPLE_COUNT];   //There is no sampleLoopEndPos because the regular sample end is re-used for this purpose
    float       sample_amp[EUPHORIA_MAX_SAMPLE_COUNT];     //linear, for multiplying
    /*TODO: Initialize these at startup*/
    int         sample_indexes[EUPHORIA_NOTES][EUPHORIA_MAX_SAMPLE_COUNT];  //Sample indexes for each note to play
    int         sample_indexes_count[EUPHORIA_NOTES]; //The count of sample indexes to iterate through
    float vel_sens_output[EUPHORIA_NOTES][EUPHORIA_MAX_SAMPLE_COUNT];
    
    int sample_mfx_groups_index[EUPHORIA_MAX_SAMPLE_COUNT];  //Cast to int during note_on
    
    //These 2 calculate which channels are assigned to a sample and should be processed
    int monofx_channel_index[EUPHORIA_MONO_FX_GROUPS_COUNT];
    int monofx_channel_index_count;
        
    int monofx_index_contained;  //Used as a boolean
    
    //These 2 calculate which effects are enabled and should be processed.  Not yet implemented
    /*
    int monofx_effect_index[LMS_MONO_FX_GROUPS_COUNT][LMS_MONO_FX_COUNT];
    int monofx_effect_index_count[LMS_MONO_FX_GROUPS_COUNT]; 
    */
        
    float adjusted_base_pitch[EUPHORIA_MAX_SAMPLE_COUNT];
    
    //For sample preview:
    int preview_sample_array_index;
    int preview_sample_max_length;  //Used to set the maximum time to preview a sample to an arbitrary number of samples
    
    /*TODO:  Deprecate these 2?*/
    int loaded_samples[EUPHORIA_MAX_SAMPLE_COUNT];
    int loaded_samples_count;
    int i_loaded_samples;
    /*Used as a boolean when determining if a sample has already been loaded*/
    int sample_is_loaded;    
    /*The index of the current sample being played*/
    int current_sample;
    
    int          sampleRate;
    float fs;    //From Ray-V
    float ratio; //Used per-sample;  If voices are ever multithreaded, this will need to be widened...
    float sample_rate_ratios[EUPHORIA_MAX_SAMPLE_COUNT];
    long         ons[EUPHORIA_NOTES];
    long         offs[EUPHORIA_NOTES];
    int         velocities[EUPHORIA_NOTES];    
    t_int_frac_read_head * sample_read_heads[EUPHORIA_NOTES][EUPHORIA_MAX_SAMPLE_COUNT];
    long         sampleNo;
    //char        *projectDir;
    char*       sample_paths[EUPHORIA_TOTAL_SAMPLE_COUNT];    
    char*       sample_files;
    
    float sample[EUPHORIA_CHANNEL_COUNT];
    
    //PolyFX modulation streams    
    int polyfx_mod_ctrl_indexes[EUPHORIA_NOTES][EUPHORIA_MODULAR_POLYFX_COUNT][(EUPHORIA_CONTROLS_PER_MOD_EFFECT * EUPHORIA_MODULATOR_COUNT)]; //The index of the control to mod, currently 0-2
    int polyfx_mod_counts[EUPHORIA_NOTES][EUPHORIA_MODULAR_POLYFX_COUNT];  //How many polyfx_mod_ptrs to iterate through for the current note
    int polyfx_mod_src_index[EUPHORIA_NOTES][EUPHORIA_MODULAR_POLYFX_COUNT][(EUPHORIA_CONTROLS_PER_MOD_EFFECT * EUPHORIA_MODULATOR_COUNT)];  //The index of the modulation source(LFO, ADSR, etc...) to multiply by
    float polyfx_mod_matrix_values[EUPHORIA_NOTES][EUPHORIA_MODULAR_POLYFX_COUNT][(EUPHORIA_CONTROLS_PER_MOD_EFFECT * EUPHORIA_MODULATOR_COUNT)];  //The value of the mod_matrix knob, multiplied by .01
    
    //Active PolyFX to process
    int active_polyfx[EUPHORIA_NOTES][EUPHORIA_MODULAR_POLYFX_COUNT];
    int active_polyfx_count[EUPHORIA_NOTES];
    
    pthread_mutex_t mutex;
    t_euphoria_mono_modules * mono_modules;
    t_amp * amp_ptr;
    t_pit_pitch_core * smp_pit_core;
    t_pit_ratio * smp_pit_ratio;
    t_ccm_midi_cc_map * midi_cc_map;    
    t_euphoria_poly_voice * data[EUPHORIA_NOTES];
    
    long pos_plus_i;  //To avoid redundantly calculating this
    
    //These are used for storing the mono FX buffers from the polyphonic voices.
    //4096 was chosen because AFAIK that's the largest size you can use in qjackctl
    float mono_fx_buffers[EUPHORIA_MONO_FX_GROUPS_COUNT][2][4096];
    
    int i_slow_index;  //For indexing operations that don't need to track realtime events closely
        
    //iterators for iterating through their respective array dimensions
    int i_fx_grps;
    int i_dst;
    int i_src;    
    int i_ctrl;
    
    float amp;  //linear amplitude, from the master volume knob
    
    t_lin_interpolater * lin_interpolator;
    
    float sv_pitch_bend_value;
    float sv_last_note;  //For glide
} t_euphoria __attribute__((aligned(16)));


    
#endif
