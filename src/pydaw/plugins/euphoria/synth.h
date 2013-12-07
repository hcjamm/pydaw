/* -*- c-basic-offset: 4 -*-  vi:set ts=8 sts=4 sw=4: */
/*
This file is part of the PyDAW project, Copyright PyDAW Team

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

#include "../../include/pydaw_plugin.h"
#include "ports.h"
#include "../../libmodsynth/lib/amp.h"
#include "../../libmodsynth/lib/voice.h"
#include "../../libmodsynth/lib/pitch_core.h"
#include "libmodsynth.h"
#include "../../libmodsynth/lib/interpolate-linear.h"
#include "../../libmodsynth/lib/interpolate-cubic.h"

#define EUPHORIA_POLYPHONY 20
#define EUPHORIA_FRAMES_MAX 16777216
//Pad the end of samples with zeroes to ensure you don't get artifacts from samples that have no silence at the end
#define EUPHORIA_Sample_Padding 100

//How many buffers in between slow indexing operations.  Buffer == users soundcard latency settings, ie: 512 samples
#define EUPHORIA_SLOW_INDEX_COUNT 64

typedef struct {
    PYFX_Data *output[2];
    PYFX_Data *basePitch[EUPHORIA_MAX_SAMPLE_COUNT];
    PYFX_Data *low_note[EUPHORIA_MAX_SAMPLE_COUNT];
    PYFX_Data *high_note[EUPHORIA_MAX_SAMPLE_COUNT];
    PYFX_Data *sample_vol[EUPHORIA_MAX_SAMPLE_COUNT];     //in decibels
    PYFX_Data *sampleStarts[EUPHORIA_MAX_SAMPLE_COUNT];
    PYFX_Data *sampleEnds[EUPHORIA_MAX_SAMPLE_COUNT];
    PYFX_Data *sampleLoopStarts[EUPHORIA_MAX_SAMPLE_COUNT];
    PYFX_Data *sampleLoopEnds[EUPHORIA_MAX_SAMPLE_COUNT];
    PYFX_Data *sampleLoopModes[EUPHORIA_MAX_SAMPLE_COUNT];
    PYFX_Data *sampleFadeInEnds[EUPHORIA_MAX_SAMPLE_COUNT];
    PYFX_Data *sampleFadeOutStarts[EUPHORIA_MAX_SAMPLE_COUNT];
    PYFX_Data *sample_vel_sens[EUPHORIA_MAX_SAMPLE_COUNT];
    PYFX_Data *sample_vel_low[EUPHORIA_MAX_SAMPLE_COUNT];
    PYFX_Data *sample_vel_high[EUPHORIA_MAX_SAMPLE_COUNT];
    PYFX_Data *sample_pitch[EUPHORIA_MAX_SAMPLE_COUNT];
    PYFX_Data *sample_tune[EUPHORIA_MAX_SAMPLE_COUNT];
    PYFX_Data *sample_interpolation_mode[EUPHORIA_MAX_SAMPLE_COUNT];

    PYFX_Data *mfx_knobs[EUPHORIA_MONO_FX_GROUPS_COUNT][EUPHORIA_MONO_FX_COUNT][EUPHORIA_CONTROLS_PER_MOD_EFFECT];
    PYFX_Data *mfx_comboboxes[EUPHORIA_MONO_FX_GROUPS_COUNT][EUPHORIA_MONO_FX_COUNT];

    //The MonoFX group selected for each sample
    PYFX_Data *sample_mfx_groups[EUPHORIA_MAX_SAMPLE_COUNT];

    PYFX_Data *selected_sample;

    PYFX_Data *attack;
    PYFX_Data *decay;
    PYFX_Data *sustain;
    PYFX_Data *release;

    PYFX_Data *attack_f;
    PYFX_Data *decay_f;
    PYFX_Data *sustain_f;
    PYFX_Data *release_f;

    PYFX_Data *master_vol;

    PYFX_Data *noise_amp[EUPHORIA_MAX_SAMPLE_COUNT];
    PYFX_Data *noise_type[EUPHORIA_MAX_SAMPLE_COUNT];

    PYFX_Data *master_glide;
    PYFX_Data *master_pb_amt;

    PYFX_Data *pitch_env_time;

    PYFX_Data *lfo_freq;
    PYFX_Data *lfo_type;
    PYFX_Data *lfo_pitch;

    PYFX_Data *global_midi_octaves_offset;

    //Corresponds to the actual knobs on the effects themselves, not the mod matrix
    PYFX_Data *pfx_mod_knob[EUPHORIA_EFFECTS_GROUPS_COUNT][EUPHORIA_MODULAR_POLYFX_COUNT][EUPHORIA_CONTROLS_PER_MOD_EFFECT];

    PYFX_Data *fx_combobox[EUPHORIA_EFFECTS_GROUPS_COUNT][EUPHORIA_MODULAR_POLYFX_COUNT];

    //PolyFX Mod Matrix
    //Corresponds to the mod matrix spinboxes
    PYFX_Data *polyfx_mod_matrix[EUPHORIA_EFFECTS_GROUPS_COUNT][EUPHORIA_MODULAR_POLYFX_COUNT][EUPHORIA_MODULATOR_COUNT][EUPHORIA_CONTROLS_PER_MOD_EFFECT];

    //End from PolyFX Mod Matrix


    int         i_selected_sample;
    int          channels;
    float       sample_last_interpolated_value[EUPHORIA_MAX_SAMPLE_COUNT];
    t_wav_pool_item * wavpool_items[EUPHORIA_MAX_SAMPLE_COUNT];
    float       sampleStartPos[EUPHORIA_MAX_SAMPLE_COUNT];
    float       sampleEndPos[EUPHORIA_MAX_SAMPLE_COUNT];
    float       sampleLoopStartPos[EUPHORIA_MAX_SAMPLE_COUNT];   //There is no sampleLoopEndPos because the regular sample end is re-used for this purpose
    float       sample_amp[EUPHORIA_MAX_SAMPLE_COUNT];     //linear, for multiplying
    int         sample_indexes[EUPHORIA_POLYPHONY][EUPHORIA_MAX_SAMPLE_COUNT];  //Sample indexes for each note to play
    int         sample_indexes_count[EUPHORIA_POLYPHONY]; //The count of sample indexes to iterate through
    float vel_sens_output[EUPHORIA_POLYPHONY][EUPHORIA_MAX_SAMPLE_COUNT];

    int sample_mfx_groups_index[EUPHORIA_MAX_SAMPLE_COUNT];  //Cast to int during note_on

    //These 2 calculate which channels are assigned to a sample and should be processed
    int monofx_channel_index[EUPHORIA_MONO_FX_GROUPS_COUNT];
    int monofx_channel_index_count;
    //Tracks which indexes are in use
    int monofx_channel_index_tracker[EUPHORIA_MONO_FX_GROUPS_COUNT];

    float adjusted_base_pitch[EUPHORIA_MAX_SAMPLE_COUNT];

    t_lin_interpolater * linear_interpolator;

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
    float ratio; //Used per-sample;

    t_voc_voices * voices;
    int         velocities[EUPHORIA_POLYPHONY];
    t_int_frac_read_head * sample_read_heads[EUPHORIA_POLYPHONY][EUPHORIA_MAX_SAMPLE_COUNT];
    long         sampleNo;

    float sample[EUPHORIA_CHANNEL_COUNT];

    //PolyFX modulation streams
    int polyfx_mod_ctrl_indexes[EUPHORIA_POLYPHONY][EUPHORIA_MODULAR_POLYFX_COUNT][(EUPHORIA_CONTROLS_PER_MOD_EFFECT * EUPHORIA_MODULATOR_COUNT)]; //The index of the control to mod, currently 0-2
    int polyfx_mod_counts[EUPHORIA_POLYPHONY][EUPHORIA_MODULAR_POLYFX_COUNT];  //How many polyfx_mod_ptrs to iterate through for the current note
    int polyfx_mod_src_index[EUPHORIA_POLYPHONY][EUPHORIA_MODULAR_POLYFX_COUNT][(EUPHORIA_CONTROLS_PER_MOD_EFFECT * EUPHORIA_MODULATOR_COUNT)];  //The index of the modulation source(LFO, ADSR, etc...) to multiply by
    float polyfx_mod_matrix_values[EUPHORIA_POLYPHONY][EUPHORIA_MODULAR_POLYFX_COUNT][(EUPHORIA_CONTROLS_PER_MOD_EFFECT * EUPHORIA_MODULATOR_COUNT)];  //The value of the mod_matrix knob, multiplied by .01

    //Active PolyFX to process
    int active_polyfx[EUPHORIA_POLYPHONY][EUPHORIA_MODULAR_POLYFX_COUNT];
    int active_polyfx_count[EUPHORIA_POLYPHONY];

    //pthread_mutex_t mutex;
    t_euphoria_mono_modules * mono_modules;
    t_amp * amp_ptr;
    t_pit_pitch_core * smp_pit_core;
    t_pit_ratio * smp_pit_ratio;
    t_euphoria_poly_voice * data[EUPHORIA_POLYPHONY];

    //These are used for storing the mono FX buffers from the polyphonic voices.
    float mono_fx_buffers[EUPHORIA_MONO_FX_GROUPS_COUNT][2];

    int i_slow_index;  //For indexing operations that don't need to track realtime events closely

    //iterators for iterating through their respective array dimensions
    int i_fx_grps;
    int i_dst;
    int i_src;
    int i_ctrl;

    float amp;  //linear amplitude, from the master volume knob

    t_cubic_interpolater * cubic_interpolator;

    float sv_pitch_bend_value;
    float sv_last_note;  //For glide

    int midi_event_types[200];
    int midi_event_ticks[200];
    float midi_event_values[200];
    int midi_event_ports[200];
    int midi_event_count;
    float * port_table;
} t_euphoria __attribute__((aligned(16)));



#endif
