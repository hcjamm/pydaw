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

#define _BSD_SOURCE    1
#define _SVID_SOURCE   1
#define _ISOC99_SOURCE 1

#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <math.h>

#include "../../include/pydaw3/pydaw_plugin.h"

#include <sndfile.h>
#include <pthread.h>

#include "synth.h"
#include "meta.h"
#include "../../libmodsynth/lib/lms_math.h"

static void v_run_lms_euphoria(PYFX_Handle instance, int sample_count,
		       t_pydaw_seq_event *events, int EventCount);

static inline void v_euphoria_slow_index(t_euphoria*);

const PYFX_Descriptor *euphoria_PYFX_descriptor(int index);
const PYINST_Descriptor *euphoria_PYINST_descriptor(int index);

static void cleanupSampler(PYFX_Handle instance)
{
    t_euphoria *plugin = (t_euphoria *)instance;
    free(plugin);
}

static void connectPortSampler(PYFX_Handle instance, int port,
			       PYFX_Data * data)
{
    t_euphoria *plugin;
    plugin = (t_euphoria *) instance;
    
    if(port < EUPHORIA_LAST_REGULAR_CONTROL_PORT)
    {
        switch (port) {
        case EUPHORIA_OUTPUT_LEFT:
            plugin->output[0] = data;
            break;
        case EUPHORIA_OUTPUT_RIGHT:
            plugin->output[1] = data;
            break;
        case EUPHORIA_SELECTED_SAMPLE:
            plugin->selected_sample = data;
            break;
        case EUPHORIA_ATTACK:
            plugin->attack = data;
            break;
        case EUPHORIA_DECAY:
            plugin->decay = data;
            break;
        case EUPHORIA_SUSTAIN:
            plugin->sustain = data;
            break;
        case EUPHORIA_RELEASE:
            plugin->release = data;
            break;
        case EUPHORIA_FILTER_ATTACK:
            plugin->attack_f = data;
            break;
        case EUPHORIA_FILTER_DECAY:
            plugin->decay_f = data;
            break;
        case EUPHORIA_FILTER_SUSTAIN:
            plugin->sustain_f = data;
            break;
        case EUPHORIA_FILTER_RELEASE:
            plugin->release_f = data;
            break;
        case EUPHORIA_NOISE_AMP:
            plugin->noise_amp = data;
            break;
        case EUPHORIA_MASTER_VOLUME:
            plugin->master_vol = data;
            break;
        case EUPHORIA_MASTER_GLIDE:
            plugin->master_glide = data;
            break;
        case EUPHORIA_MASTER_PITCHBEND_AMT:
            plugin->master_pb_amt = data;
            break;
        case EUPHORIA_PITCH_ENV_TIME:
            plugin->pitch_env_time = data;
            break;
        case EUPHORIA_LFO_FREQ:
            plugin->lfo_freq = data;
            break;
        case EUPHORIA_LFO_TYPE:
            plugin->lfo_type = data;
            break;
            
        case EUPHORIA_FX0_KNOB0: plugin->pfx_mod_knob[0][0][0] = data; break;
        case EUPHORIA_FX0_KNOB1: plugin->pfx_mod_knob[0][0][1] = data; break;
        case EUPHORIA_FX0_KNOB2: plugin->pfx_mod_knob[0][0][2] = data; break;
        case EUPHORIA_FX1_KNOB0: plugin->pfx_mod_knob[0][1][0] = data; break;
        case EUPHORIA_FX1_KNOB1: plugin->pfx_mod_knob[0][1][1] = data; break;
        case EUPHORIA_FX1_KNOB2: plugin->pfx_mod_knob[0][1][2] = data; break;
        case EUPHORIA_FX2_KNOB0: plugin->pfx_mod_knob[0][2][0] = data; break;
        case EUPHORIA_FX2_KNOB1: plugin->pfx_mod_knob[0][2][1] = data; break;
        case EUPHORIA_FX2_KNOB2: plugin->pfx_mod_knob[0][2][2] = data; break;
        case EUPHORIA_FX3_KNOB0: plugin->pfx_mod_knob[0][3][0] = data; break;
        case EUPHORIA_FX3_KNOB1: plugin->pfx_mod_knob[0][3][1] = data; break;
        case EUPHORIA_FX3_KNOB2: plugin->pfx_mod_knob[0][3][2] = data; break;
            
        case EUPHORIA_FX0_COMBOBOX: plugin->fx_combobox[0][0] = data; break;    
        case EUPHORIA_FX1_COMBOBOX: plugin->fx_combobox[0][1] = data; break;    
        case EUPHORIA_FX2_COMBOBOX: plugin->fx_combobox[0][2] = data; break;    
        case EUPHORIA_FX3_COMBOBOX: plugin->fx_combobox[0][3] = data; break;    
        //End from Modulex
        /*PolyFX mod matrix port connections*/
        case EUPHORIA_PFXMATRIX_GRP0DST0SRC0CTRL0: plugin->polyfx_mod_matrix[0][0][0][0] = data; break;
        case EUPHORIA_PFXMATRIX_GRP0DST0SRC0CTRL1: plugin->polyfx_mod_matrix[0][0][0][1] = data; break;
        case EUPHORIA_PFXMATRIX_GRP0DST0SRC0CTRL2: plugin->polyfx_mod_matrix[0][0][0][2] = data; break;
        case EUPHORIA_PFXMATRIX_GRP0DST0SRC1CTRL0: plugin->polyfx_mod_matrix[0][0][1][0] = data; break;
        case EUPHORIA_PFXMATRIX_GRP0DST0SRC1CTRL1: plugin->polyfx_mod_matrix[0][0][1][1] = data; break;
        case EUPHORIA_PFXMATRIX_GRP0DST0SRC1CTRL2: plugin->polyfx_mod_matrix[0][0][1][2] = data; break;
        case EUPHORIA_PFXMATRIX_GRP0DST0SRC2CTRL0: plugin->polyfx_mod_matrix[0][0][2][0] = data; break;
        case EUPHORIA_PFXMATRIX_GRP0DST0SRC2CTRL1: plugin->polyfx_mod_matrix[0][0][2][1] = data; break;
        case EUPHORIA_PFXMATRIX_GRP0DST0SRC2CTRL2: plugin->polyfx_mod_matrix[0][0][2][2] = data; break;
        case EUPHORIA_PFXMATRIX_GRP0DST0SRC3CTRL0: plugin->polyfx_mod_matrix[0][0][3][0] = data; break;
        case EUPHORIA_PFXMATRIX_GRP0DST0SRC3CTRL1: plugin->polyfx_mod_matrix[0][0][3][1] = data; break;
        case EUPHORIA_PFXMATRIX_GRP0DST0SRC3CTRL2: plugin->polyfx_mod_matrix[0][0][3][2] = data; break;
        case EUPHORIA_PFXMATRIX_GRP0DST1SRC0CTRL0: plugin->polyfx_mod_matrix[0][1][0][0] = data; break;
        case EUPHORIA_PFXMATRIX_GRP0DST1SRC0CTRL1: plugin->polyfx_mod_matrix[0][1][0][1] = data; break;
        case EUPHORIA_PFXMATRIX_GRP0DST1SRC0CTRL2: plugin->polyfx_mod_matrix[0][1][0][2] = data; break;
        case EUPHORIA_PFXMATRIX_GRP0DST1SRC1CTRL0: plugin->polyfx_mod_matrix[0][1][1][0] = data; break;
        case EUPHORIA_PFXMATRIX_GRP0DST1SRC1CTRL1: plugin->polyfx_mod_matrix[0][1][1][1] = data; break;
        case EUPHORIA_PFXMATRIX_GRP0DST1SRC1CTRL2: plugin->polyfx_mod_matrix[0][1][1][2] = data; break;
        case EUPHORIA_PFXMATRIX_GRP0DST1SRC2CTRL0: plugin->polyfx_mod_matrix[0][1][2][0] = data; break;
        case EUPHORIA_PFXMATRIX_GRP0DST1SRC2CTRL1: plugin->polyfx_mod_matrix[0][1][2][1] = data; break;
        case EUPHORIA_PFXMATRIX_GRP0DST1SRC2CTRL2: plugin->polyfx_mod_matrix[0][1][2][2] = data; break;
        case EUPHORIA_PFXMATRIX_GRP0DST1SRC3CTRL0: plugin->polyfx_mod_matrix[0][1][3][0] = data; break;
        case EUPHORIA_PFXMATRIX_GRP0DST1SRC3CTRL1: plugin->polyfx_mod_matrix[0][1][3][1] = data; break;
        case EUPHORIA_PFXMATRIX_GRP0DST1SRC3CTRL2: plugin->polyfx_mod_matrix[0][1][3][2] = data; break;
        case EUPHORIA_PFXMATRIX_GRP0DST2SRC0CTRL0: plugin->polyfx_mod_matrix[0][2][0][0] = data; break;
        case EUPHORIA_PFXMATRIX_GRP0DST2SRC0CTRL1: plugin->polyfx_mod_matrix[0][2][0][1] = data; break;
        case EUPHORIA_PFXMATRIX_GRP0DST2SRC0CTRL2: plugin->polyfx_mod_matrix[0][2][0][2] = data; break;
        case EUPHORIA_PFXMATRIX_GRP0DST2SRC1CTRL0: plugin->polyfx_mod_matrix[0][2][1][0] = data; break;
        case EUPHORIA_PFXMATRIX_GRP0DST2SRC1CTRL1: plugin->polyfx_mod_matrix[0][2][1][1] = data; break;
        case EUPHORIA_PFXMATRIX_GRP0DST2SRC1CTRL2: plugin->polyfx_mod_matrix[0][2][1][2] = data; break;
        case EUPHORIA_PFXMATRIX_GRP0DST2SRC2CTRL0: plugin->polyfx_mod_matrix[0][2][2][0] = data; break;
        case EUPHORIA_PFXMATRIX_GRP0DST2SRC2CTRL1: plugin->polyfx_mod_matrix[0][2][2][1] = data; break;
        case EUPHORIA_PFXMATRIX_GRP0DST2SRC2CTRL2: plugin->polyfx_mod_matrix[0][2][2][2] = data; break;
        case EUPHORIA_PFXMATRIX_GRP0DST2SRC3CTRL0: plugin->polyfx_mod_matrix[0][2][3][0] = data; break;
        case EUPHORIA_PFXMATRIX_GRP0DST2SRC3CTRL1: plugin->polyfx_mod_matrix[0][2][3][1] = data; break;
        case EUPHORIA_PFXMATRIX_GRP0DST2SRC3CTRL2: plugin->polyfx_mod_matrix[0][2][3][2] = data; break;
        case EUPHORIA_PFXMATRIX_GRP0DST3SRC0CTRL0: plugin->polyfx_mod_matrix[0][3][0][0] = data; break;
        case EUPHORIA_PFXMATRIX_GRP0DST3SRC0CTRL1: plugin->polyfx_mod_matrix[0][3][0][1] = data; break;
        case EUPHORIA_PFXMATRIX_GRP0DST3SRC0CTRL2: plugin->polyfx_mod_matrix[0][3][0][2] = data; break;
        case EUPHORIA_PFXMATRIX_GRP0DST3SRC1CTRL0: plugin->polyfx_mod_matrix[0][3][1][0] = data; break;
        case EUPHORIA_PFXMATRIX_GRP0DST3SRC1CTRL1: plugin->polyfx_mod_matrix[0][3][1][1] = data; break;
        case EUPHORIA_PFXMATRIX_GRP0DST3SRC1CTRL2: plugin->polyfx_mod_matrix[0][3][1][2] = data; break;
        case EUPHORIA_PFXMATRIX_GRP0DST3SRC2CTRL0: plugin->polyfx_mod_matrix[0][3][2][0] = data; break;
        case EUPHORIA_PFXMATRIX_GRP0DST3SRC2CTRL1: plugin->polyfx_mod_matrix[0][3][2][1] = data; break;
        case EUPHORIA_PFXMATRIX_GRP0DST3SRC2CTRL2: plugin->polyfx_mod_matrix[0][3][2][2] = data; break;
        case EUPHORIA_PFXMATRIX_GRP0DST3SRC3CTRL0: plugin->polyfx_mod_matrix[0][3][3][0] = data; break;
        case EUPHORIA_PFXMATRIX_GRP0DST3SRC3CTRL1: plugin->polyfx_mod_matrix[0][3][3][1] = data; break;
        case EUPHORIA_PFXMATRIX_GRP0DST3SRC3CTRL2: plugin->polyfx_mod_matrix[0][3][3][2] = data; break;

        //End PolyFX mod matrix
        
        //case LMS_GLOBAL_MIDI_CHANNEL: plugin->global_midi_channel = data; break;
        //case LMS_GLOBAL_MIDI_OCTAVES_OFFSET: plugin->global_midi_octaves_offset = data; break;
        case EUPHORIA_NOISE_TYPE: plugin->noise_type = data; break;
        case EUPHORIA_LFO_PITCH: plugin->lfo_pitch = data; break;
        default:
            break;
        }
    }
    else if((port >= EUPHORIA_SAMPLE_PITCH_PORT_RANGE_MIN) && (port < EUPHORIA_SAMPLE_PITCH_PORT_RANGE_MAX))
    {
        plugin->basePitch[(port - EUPHORIA_SAMPLE_PITCH_PORT_RANGE_MIN)] = data;
    }
    else if((port >= EUPHORIA_PLAY_PITCH_LOW_PORT_RANGE_MIN) && (port < EUPHORIA_PLAY_PITCH_LOW_PORT_RANGE_MAX))
    {
        plugin->low_note[(port - EUPHORIA_PLAY_PITCH_LOW_PORT_RANGE_MIN)] = data;
    }    
    else if((port >= EUPHORIA_PLAY_PITCH_HIGH_PORT_RANGE_MIN) && (port < EUPHORIA_PLAY_PITCH_HIGH_PORT_RANGE_MAX))
    {
        plugin->high_note[(port - EUPHORIA_PLAY_PITCH_HIGH_PORT_RANGE_MIN)] = data;
    }
    else if((port >= LMS_SAMPLE_VOLUME_PORT_RANGE_MIN) && (port < EUPHORIA_SAMPLE_VOLUME_PORT_RANGE_MAX))
    {
        plugin->sample_vol[(port - LMS_SAMPLE_VOLUME_PORT_RANGE_MIN)] = data;
    }
    else if((port >= EUPHORIA_SAMPLE_START_PORT_RANGE_MIN) && (port < EUPHORIA_SAMPLE_START_PORT_RANGE_MAX))
    {
        plugin->sampleStarts[(port - EUPHORIA_SAMPLE_START_PORT_RANGE_MIN)] = data;
    }
    else if((port >= EUPHORIA_SAMPLE_END_PORT_RANGE_MIN) && (port < EUPHORIA_SAMPLE_END_PORT_RANGE_MAX))
    {
        plugin->sampleEnds[(port - EUPHORIA_SAMPLE_END_PORT_RANGE_MIN)] = data;
    }
    else if((port >= EUPHORIA_SAMPLE_VEL_SENS_PORT_RANGE_MIN) && (port < EUPHORIA_SAMPLE_VEL_SENS_PORT_RANGE_MAX))
    {
        plugin->sample_vel_sens[(port - EUPHORIA_SAMPLE_VEL_SENS_PORT_RANGE_MIN)] = data;
    }    
    else if((port >= EUPHORIA_SAMPLE_VEL_LOW_PORT_RANGE_MIN) && (port < EUPHORIA_SAMPLE_VEL_LOW_PORT_RANGE_MAX))
    {
        plugin->sample_vel_low[(port - EUPHORIA_SAMPLE_VEL_LOW_PORT_RANGE_MIN)] = data;
    }
    else if((port >= EUPHORIA_SAMPLE_VEL_HIGH_PORT_RANGE_MIN) && (port < EUPHORIA_SAMPLE_VEL_HIGH_PORT_RANGE_MAX))
    {
        plugin->sample_vel_high[(port - EUPHORIA_SAMPLE_VEL_HIGH_PORT_RANGE_MIN)] = data;
    }
    //new
    else if((port >= EUPHORIA_PITCH_PORT_RANGE_MIN) && (port < EUPHORIA_PITCH_PORT_RANGE_MAX))
    {
        plugin->sample_pitch[(port - EUPHORIA_PITCH_PORT_RANGE_MIN)] = data;
    }    
    else if((port >= EUPHORIA_TUNE_PORT_RANGE_MIN) && (port < EUPHORIA_TUNE_PORT_RANGE_MAX))
    {
        plugin->sample_tune[(port - EUPHORIA_TUNE_PORT_RANGE_MIN)] = data;
    }
    else if((port >= EUPHORIA_SAMPLE_INTERPOLATION_MODE_PORT_RANGE_MIN) && (port < EUPHORIA_SAMPLE_INTERPOLATION_MODE_PORT_RANGE_MAX))
    {
        plugin->sample_interpolation_mode[(port - EUPHORIA_SAMPLE_INTERPOLATION_MODE_PORT_RANGE_MIN)] = data;
    }
    
    else if((port >= EUPHORIA_SAMPLE_LOOP_START_PORT_RANGE_MIN) && (port < EUPHORIA_SAMPLE_LOOP_START_PORT_RANGE_MAX))
    {
        plugin->sampleLoopStarts[(port - EUPHORIA_SAMPLE_LOOP_START_PORT_RANGE_MIN)] = data;
    }
    else if((port >= EUPHORIA_SAMPLE_LOOP_END_PORT_RANGE_MIN) && (port < EUPHORIA_SAMPLE_LOOP_END_PORT_RANGE_MAX))
    {
        plugin->sampleLoopEnds[(port - EUPHORIA_SAMPLE_LOOP_END_PORT_RANGE_MIN)] = data;
    }
    else if((port >= EUPHORIA_SAMPLE_LOOP_MODE_PORT_RANGE_MIN) && (port < EUPHORIA_SAMPLE_LOOP_MODE_PORT_RANGE_MAX))
    {
        plugin->sampleLoopModes[(port - EUPHORIA_SAMPLE_LOOP_MODE_PORT_RANGE_MIN)] = data;
    }
    
    //MonoFX0
    else if((port >= EUPHORIA_MONO_FX0_KNOB0_PORT_RANGE_MIN) && (port < EUPHORIA_MONO_FX0_KNOB0_PORT_RANGE_MAX))
    {
        plugin->mfx_knobs[(port - EUPHORIA_MONO_FX0_KNOB0_PORT_RANGE_MIN)][0][0] = data;
    }
    else if((port >= EUPHORIA_MONO_FX0_KNOB1_PORT_RANGE_MIN) && (port < EUPHORIA_MONO_FX0_KNOB1_PORT_RANGE_MAX))
    {
        plugin->mfx_knobs[(port - EUPHORIA_MONO_FX0_KNOB1_PORT_RANGE_MIN)][0][1] = data;
    }
    else if((port >= EUPHORIA_MONO_FX0_KNOB2_PORT_RANGE_MIN) && (port < EUPHORIA_MONO_FX0_KNOB2_PORT_RANGE_MAX))
    {
        plugin->mfx_knobs[(port - EUPHORIA_MONO_FX0_KNOB2_PORT_RANGE_MIN)][0][2] = data;
    }
    else if((port >= EUPHORIA_MONO_FX0_COMBOBOX_PORT_RANGE_MIN) && (port < EUPHORIA_MONO_FX0_COMBOBOX_PORT_RANGE_MAX))
    {
        plugin->mfx_comboboxes[(port - EUPHORIA_MONO_FX0_COMBOBOX_PORT_RANGE_MIN)][0] = data;
    }
    //MonoFX1
    else if((port >= EUPHORIA_MONO_FX1_KNOB0_PORT_RANGE_MIN) && (port < EUPHORIA_MONO_FX1_KNOB0_PORT_RANGE_MAX))
    {
        plugin->mfx_knobs[(port - EUPHORIA_MONO_FX1_KNOB0_PORT_RANGE_MIN)][1][0] = data;
    }
    else if((port >= EUPHORIA_MONO_FX1_KNOB1_PORT_RANGE_MIN) && (port < EUPHORIA_MONO_FX1_KNOB1_PORT_RANGE_MAX))
    {
        plugin->mfx_knobs[(port - EUPHORIA_MONO_FX1_KNOB1_PORT_RANGE_MIN)][1][1] = data;
    }
    else if((port >= EUPHORIA_MONO_FX1_KNOB2_PORT_RANGE_MIN) && (port < EUPHORIA_MONO_FX1_KNOB2_PORT_RANGE_MAX))
    {
        plugin->mfx_knobs[(port - EUPHORIA_MONO_FX1_KNOB2_PORT_RANGE_MIN)][1][2] = data;
    }
    else if((port >= EUPHORIA_MONO_FX1_COMBOBOX_PORT_RANGE_MIN) && (port < EUPHORIA_MONO_FX1_COMBOBOX_PORT_RANGE_MAX))
    {
        plugin->mfx_comboboxes[(port - EUPHORIA_MONO_FX1_COMBOBOX_PORT_RANGE_MIN)][1] = data;
    }
    //MonoFX2
    else if((port >= EUPHORIA_MONO_FX2_KNOB0_PORT_RANGE_MIN) && (port < EUPHORIA_MONO_FX2_KNOB0_PORT_RANGE_MAX))
    {
        plugin->mfx_knobs[(port - EUPHORIA_MONO_FX2_KNOB0_PORT_RANGE_MIN)][2][0] = data;
    }
    else if((port >= EUPHORIA_MONO_FX2_KNOB1_PORT_RANGE_MIN) && (port < EUPHORIA_MONO_FX2_KNOB1_PORT_RANGE_MAX))
    {
        plugin->mfx_knobs[(port - EUPHORIA_MONO_FX2_KNOB1_PORT_RANGE_MIN)][2][1] = data;
    }
    else if((port >= EUPHORIA_MONO_FX2_KNOB2_PORT_RANGE_MIN) && (port < EUPHORIA_MONO_FX2_KNOB2_PORT_RANGE_MAX))
    {
        plugin->mfx_knobs[(port - EUPHORIA_MONO_FX2_KNOB2_PORT_RANGE_MIN)][2][2] = data;
    }
    else if((port >= EUPHORIA_MONO_FX2_COMBOBOX_PORT_RANGE_MIN) && (port < EUPHORIA_MONO_FX2_COMBOBOX_PORT_RANGE_MAX))
    {
        plugin->mfx_comboboxes[(port - EUPHORIA_MONO_FX2_COMBOBOX_PORT_RANGE_MIN)][2] = data;
    }
    //MonoFX3
    else if((port >= EUPHORIA_MONO_FX3_KNOB0_PORT_RANGE_MIN) && (port < EUPHORIA_MONO_FX3_KNOB0_PORT_RANGE_MAX))
    {
        plugin->mfx_knobs[(port - EUPHORIA_MONO_FX3_KNOB0_PORT_RANGE_MIN)][3][0] = data;
    }
    else if((port >= EUPHORIA_MONO_FX3_KNOB1_PORT_RANGE_MIN) && (port < EUPHORIA_MONO_FX3_KNOB1_PORT_RANGE_MAX))
    {
        plugin->mfx_knobs[(port - EUPHORIA_MONO_FX3_KNOB1_PORT_RANGE_MIN)][3][1] = data;
    }
    else if((port >= EUPHORIA_MONO_FX3_KNOB2_PORT_RANGE_MIN) && (port < EUPHORIA_MONO_FX3_KNOB2_PORT_RANGE_MAX))
    {
        plugin->mfx_knobs[(port - EUPHORIA_MONO_FX3_KNOB2_PORT_RANGE_MIN)][3][2] = data;
    }
    else if((port >= EUPHORIA_MONO_FX3_COMBOBOX_PORT_RANGE_MIN) && (port < EUPHORIA_MONO_FX3_COMBOBOX_PORT_RANGE_MAX))
    {
        plugin->mfx_comboboxes[(port - EUPHORIA_MONO_FX3_COMBOBOX_PORT_RANGE_MIN)][3] = data;
    }
    
    else if((port >= EUPHORIA_SAMPLE_MONO_FX_GROUP_PORT_RANGE_MIN) && (port < EUPHORIA_SAMPLE_MONO_FX_GROUP_PORT_RANGE_MAX))
    {
        plugin->sample_mfx_groups[(port - EUPHORIA_SAMPLE_MONO_FX_GROUP_PORT_RANGE_MIN)] = data;
    }    
}

static PYFX_Handle instantiateSampler(const PYFX_Descriptor * descriptor,
					int s_rate)
{
    t_euphoria *plugin_data; // = (Sampler *) malloc(sizeof(Sampler));
    
    if(posix_memalign((void**)&plugin_data, 16, sizeof(t_euphoria)) != 0)
    {     
        return NULL;
    }
    
    pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

    plugin_data->voices = g_voc_get_voices(EUPHORIA_POLYPHONY);
    
    plugin_data->i_selected_sample = 0;
    plugin_data->current_sample = 0;
    plugin_data->loaded_samples_count = 0;
    plugin_data->cubic_interpolator = g_cubic_get();
    plugin_data->linear_interpolator = g_lin_get();
    plugin_data->amp = 1.0f;
    plugin_data->i_slow_index = 0;
    
    plugin_data->preview_sample_array_index = 0;
    plugin_data->sampleCount[EUPHORIA_MAX_SAMPLE_COUNT] = 0;  //To prevent a SEGFAULT on the first call of the main loop
    plugin_data->sample_paths[EUPHORIA_MAX_SAMPLE_COUNT] = (char*)malloc(sizeof(char) * 1024);
    plugin_data->sampleData[0][EUPHORIA_MAX_SAMPLE_COUNT] = NULL;
    plugin_data->sampleData[1][EUPHORIA_MAX_SAMPLE_COUNT] = NULL;
    plugin_data->sample_paths[EUPHORIA_MAX_SAMPLE_COUNT][0] = '\0'; //This is the preview file path
    plugin_data->sample_files = (char*)malloc(sizeof(char) * 10000);
    plugin_data->preview_sample_max_length = s_rate * 5.0f;  //Sets the maximum time to preview a sample to 5 seconds, lest a user unwittlingly tries to preview a 2 hour long sample.
    plugin_data->preview_length = 0.0f;
    
    plugin_data->smp_pit_core = g_pit_get();
    plugin_data->smp_pit_ratio = g_pit_ratio();
        
    plugin_data->sample_rate_ratios[EUPHORIA_MAX_SAMPLE_COUNT] = 1.0f;
    
    int f_i = 0;
    while(f_i < EUPHORIA_MAX_SAMPLE_COUNT)
    {
        plugin_data->sampleStarts[f_i] = 0;
        plugin_data->sampleEnds[f_i] = 0;
        plugin_data->sampleCount[f_i] = 0;
        plugin_data->basePitch[f_i] = 0;
        plugin_data->low_note[f_i] = 0;
        plugin_data->high_note[f_i] = 0;
        plugin_data->sample_vol[f_i] = 0;
        plugin_data->sample_amp[f_i] = 1.0f;
        plugin_data->sampleEndPos[f_i] = 0.0f;
        plugin_data->sample_last_interpolated_value[f_i] = 0.0f;
        
        plugin_data->sample_paths[f_i] = (char*)malloc(sizeof(char) * 1024);
        plugin_data->sample_paths[f_i][0] = '\0';
        
        plugin_data->sampleData[0][f_i] = NULL;
        plugin_data->sampleData[1][f_i] = NULL;
        
        plugin_data->adjusted_base_pitch[f_i] = 60.0f;
        plugin_data->sample_rate_ratios[f_i] = 1.0f;
        
        f_i++;
    }
    
    f_i = 0;
    int f_i2;
    
    while(f_i < EUPHORIA_POLYPHONY)
    {        
        plugin_data->sample_indexes_count[f_i] = 0;
        plugin_data->data[f_i] = g_euphoria_poly_init(s_rate);
        
        f_i2 = 0;
        while(f_i2 < EUPHORIA_MAX_SAMPLE_COUNT)
        {
            plugin_data->sample_read_heads[f_i][f_i2] = g_ifh_get();
            plugin_data->sample_indexes[f_i][f_i2] = 0;
            plugin_data->vel_sens_output[f_i][f_i2] = 0.0f;
            f_i2++;
        }
        
        f_i++;
    }
    
    for(f_i = 0; f_i < EUPHORIA_MONO_FX_GROUPS_COUNT; f_i++)
    {
        plugin_data->monofx_channel_index[f_i] = 0;
        
        for(f_i2 = 0; f_i2 < 4096; f_i2++)
        {
            plugin_data->mono_fx_buffers[f_i][0][f_i2] = 0.0f;
            plugin_data->mono_fx_buffers[f_i][1][f_i2] = 0.0f;
        }
    }
    
    plugin_data->sampleRate = s_rate;
    //plugin_data->projectDir = 0;    
    plugin_data->sv_pitch_bend_value = 0.0f;
    plugin_data->sv_last_note = 36.0f;
    plugin_data->channels = 2;
    plugin_data->amp_ptr = g_amp_get();    
    memcpy(&plugin_data->mutex, &m, sizeof(pthread_mutex_t));    
    plugin_data->mono_modules = g_euphoria_mono_init(s_rate);    
    plugin_data->fs = s_rate;
    
    return (PYFX_Handle) plugin_data;
}

static void v_euphoria_activate(PYFX_Handle instance)
{
    t_euphoria *plugin_data = (t_euphoria *) instance;
    int i;

    pthread_mutex_lock(&plugin_data->mutex);

    plugin_data->sampleNo = 0;

    for (i = 0; i < EUPHORIA_POLYPHONY; i++) 
    {
	plugin_data->velocities[i] = 0;
    }
    
    v_euphoria_slow_index(plugin_data);

    pthread_mutex_unlock(&plugin_data->mutex);
}


//For the per-sample interpolation modes
typedef int (*fp_calculate_ratio)(t_euphoria *__restrict plugin_data, int n);
typedef void (*fp_run_sampler_interpolation)(t_euphoria *__restrict plugin_data, int n, int ch);

static fp_calculate_ratio ratio_function_ptrs[EUPHORIA_MAX_SAMPLE_COUNT];
static fp_run_sampler_interpolation interpolation_modes[EUPHORIA_MAX_SAMPLE_COUNT];

static inline int check_sample_bounds(t_euphoria *__restrict plugin_data, int n)
{    
    if ((plugin_data->sample_read_heads[n][(plugin_data->current_sample)]->whole_number) >=  plugin_data->sampleEndPos[(plugin_data->current_sample)])
    {
        if(((int)(*(plugin_data->sampleLoopModes[(plugin_data->current_sample)]))) > 0)
        {
            //TODO:  write a special function that either maintains the fraction, or
            //else wraps the negative interpolation back to where it was before the loop happened, to avoid clicks and pops
            v_ifh_retrigger(plugin_data->sample_read_heads[n][(plugin_data->current_sample)], 
                    (EUPHORIA_SINC_INTERPOLATION_POINTS_DIV2 + (plugin_data->sampleLoopStartPos[(plugin_data->current_sample)])));// 0.0f;
   
            return 0;
        }
        else
        {
            return 1;
        }
    }
    else
    {
        return 0;
    }
}

static int calculate_ratio_sinc(t_euphoria *__restrict plugin_data, int n)
{
    plugin_data->ratio =
    f_pit_midi_note_to_ratio_fast(plugin_data->adjusted_base_pitch[(plugin_data->current_sample)],
            ((plugin_data->data[n]->base_pitch) //+ (plugin_data->data[n]->lfo_pitch_output)
            ),
            plugin_data->smp_pit_core, plugin_data->smp_pit_ratio)
            *
            plugin_data->sample_rate_ratios[(plugin_data->current_sample)];

    v_ifh_run(plugin_data->sample_read_heads[n][(plugin_data->current_sample)], (plugin_data->ratio));

    return check_sample_bounds(plugin_data, n);
}

static int calculate_ratio_linear(t_euphoria *__restrict plugin_data, int n)
{
    return calculate_ratio_sinc(plugin_data, n);
}

static int calculate_ratio_none(t_euphoria *__restrict plugin_data, int n)
{
    plugin_data->sample_read_heads[n][(plugin_data->current_sample)]->whole_number = (plugin_data->sample_read_heads[n][(plugin_data->current_sample)]->whole_number) + 1;
    
    return check_sample_bounds(plugin_data, n);
}

static void run_sampler_interpolation_sinc(t_euphoria *__restrict plugin_data, int n, int ch)
{    
    plugin_data->sample_last_interpolated_value[(plugin_data->current_sample)] = f_sinc_interpolate2(plugin_data->mono_modules->sinc_interpolator, 
            plugin_data->sampleData[ch][(plugin_data->current_sample)],
            (plugin_data->sample_read_heads[n][(plugin_data->current_sample)]->whole_number),
            (plugin_data->sample_read_heads[n][(plugin_data->current_sample)]->fraction));
}


static void run_sampler_interpolation_linear(t_euphoria *__restrict plugin_data, int n, int ch)
{
    plugin_data->sample_last_interpolated_value[(plugin_data->current_sample)] = f_cubic_interpolate_ptr_ifh(
            plugin_data->sampleData[ch][(plugin_data->current_sample)],
            (plugin_data->sample_read_heads[n][(plugin_data->current_sample)]->whole_number),
            (plugin_data->sample_read_heads[n][(plugin_data->current_sample)]->fraction),
            plugin_data->cubic_interpolator);            
}


static void run_sampler_interpolation_none(t_euphoria *__restrict plugin_data, int n, int ch)
{
    plugin_data->sample_last_interpolated_value[(plugin_data->current_sample)] = 
            plugin_data->sampleData[ch][(plugin_data->current_sample)][(plugin_data->sample_read_heads[n][(plugin_data->current_sample)]->whole_number)];
}

/* static void addSample(Sampler *plugin_data, 
 * int n, //The note number
 * int pos, //the position in the output buffer
 * int count) //how many samples to fill in the output buffer
 */
static void add_sample_lms_euphoria(t_euphoria *__restrict plugin_data, int n, int count)
{
    int ch;
    long i = 0;
    
    if((plugin_data->voices->voices[n].on) > (plugin_data->sampleNo))
    {
        i = ((plugin_data->voices->voices[n].on) - (plugin_data->sampleNo));
    }    
        
    for (; i < count; ++i) 
    {
        //Run things that aren't per-channel like envelopes
                
        v_adsr_run_db(plugin_data->data[n]->adsr_amp);        
                            
        if(plugin_data->data[n]->adsr_amp->stage == 4)
        {
            plugin_data->voices->voices[n].n_state = note_state_off;
            break;
        }

        v_adsr_run(plugin_data->data[n]->adsr_filter);
        
        //Run the glide module            
        f_rmp_run_ramp(plugin_data->data[n]->ramp_env);
        f_rmp_run_ramp(plugin_data->data[n]->glide_env);
        
        //Set and run the LFO
        v_lfs_set(plugin_data->data[n]->lfo1,  (*(plugin_data->lfo_freq)) * .01);
        v_lfs_run(plugin_data->data[n]->lfo1);
        
        plugin_data->data[n]->base_pitch = (plugin_data->data[n]->glide_env->output_multiplied)
                +  (plugin_data->mono_modules->pitchbend_smoother->output)  //(plugin_data->sv_pitch_bend_value)
                + (plugin_data->data[n]->last_pitch) + ((plugin_data->data[n]->lfo1->output) * (*(plugin_data->lfo_pitch)));
        
        if((plugin_data->voices->voices[n].off) == (i + plugin_data->sampleNo)) //pos + 
        {   
            if(plugin_data->voices->voices[n].n_state == note_state_killed)
            {
                v_euphoria_poly_note_off(plugin_data->data[n], 1);
            }
            else
            {
                v_euphoria_poly_note_off(plugin_data->data[n], 0);
            }            
	}        
                
        plugin_data->sample[0] = 0.0f;
        plugin_data->sample[1] = 0.0f;
                    
        plugin_data->i_loaded_samples = 0;

        //Calculating and summing all of the interpolated samples for this note
        while((plugin_data->i_loaded_samples) < (plugin_data->sample_indexes_count[n]))
        {
            plugin_data->current_sample = (plugin_data->sample_indexes[n][(plugin_data->i_loaded_samples)]);
            if(ratio_function_ptrs[(plugin_data->current_sample)](plugin_data, n) == 1)
            {
                plugin_data->i_loaded_samples = (plugin_data->i_loaded_samples) + 1;                
                continue;
            }
            
            plugin_data->data[n]->noise_sample = ((plugin_data->data[n]->noise_func_ptr(plugin_data->mono_modules->white_noise1[(plugin_data->data[n]->noise_index)])) * (plugin_data->data[n]->noise_linamp)); //add noise
            
            for (ch = 0; ch < (plugin_data->sample_channels[(plugin_data->i_loaded_samples)]); ++ch) 
            {                
                interpolation_modes[(plugin_data->current_sample)](plugin_data, n, ch);

                plugin_data->sample[ch] += plugin_data->sample_last_interpolated_value[(plugin_data->current_sample)];
                
                plugin_data->sample[ch] += (plugin_data->data[n]->noise_sample);

                plugin_data->sample[ch] = (plugin_data->sample[ch]) * (plugin_data->sample_amp[(plugin_data->current_sample)]); // * (plugin_data->data[n]->lfo_amp_output);

                plugin_data->data[n]->modulex_current_sample[ch] = (plugin_data->sample[ch]);
                
                
                if((plugin_data->sample_channels[(plugin_data->current_sample)]) == 1)
                {
                    plugin_data->data[n]->modulex_current_sample[1] = plugin_data->sample[0];
                    break;
                }
            }
                        
            //Modular PolyFX, processed from the index created during note_on
            for(plugin_data->i_dst = 0; (plugin_data->i_dst) < (plugin_data->active_polyfx_count[n]); plugin_data->i_dst = (plugin_data->i_dst) + 1)
            {            
                v_mf3_set(plugin_data->data[n]->multieffect[(plugin_data->active_polyfx[n][(plugin_data->i_dst)])][(plugin_data->current_sample)],
                    *(plugin_data->pfx_mod_knob[0][(plugin_data->active_polyfx[n][(plugin_data->i_dst)])][0]), *(plugin_data->pfx_mod_knob[0][(plugin_data->active_polyfx[n][(plugin_data->i_dst)])][1]), *(plugin_data->pfx_mod_knob[0][(plugin_data->active_polyfx[n][(plugin_data->i_dst)])][2])); 

                int f_mod_test;

                for(f_mod_test = 0; f_mod_test < (plugin_data->polyfx_mod_counts[n][(plugin_data->active_polyfx[n][(plugin_data->i_dst)])]); f_mod_test++)
                {
                    v_mf3_mod_single(
                            plugin_data->data[n]->multieffect[(plugin_data->active_polyfx[n][(plugin_data->i_dst)])][(plugin_data->current_sample)],                    
                            *(plugin_data->data[n]->modulator_outputs[(plugin_data->polyfx_mod_src_index[n][(plugin_data->active_polyfx[n][(plugin_data->i_dst)])][f_mod_test])]),
                            (plugin_data->polyfx_mod_matrix_values[n][(plugin_data->active_polyfx[n][(plugin_data->i_dst)])][f_mod_test]),
                            (plugin_data->polyfx_mod_ctrl_indexes[n][(plugin_data->active_polyfx[n][(plugin_data->i_dst)])][f_mod_test])
                            );
                }

                plugin_data->data[n]->fx_func_ptr[(plugin_data->active_polyfx[n][(plugin_data->i_dst)])](plugin_data->data[n]->multieffect[(plugin_data->active_polyfx[n][(plugin_data->i_dst)])][(plugin_data->current_sample)], (plugin_data->data[n]->modulex_current_sample[0]), (plugin_data->data[n]->modulex_current_sample[1])); 

                plugin_data->data[n]->modulex_current_sample[0] = plugin_data->data[n]->multieffect[(plugin_data->active_polyfx[n][(plugin_data->i_dst)])][(plugin_data->current_sample)]->output0;
                plugin_data->data[n]->modulex_current_sample[1] = plugin_data->data[n]->multieffect[(plugin_data->active_polyfx[n][(plugin_data->i_dst)])][(plugin_data->current_sample)]->output1;

            }

            plugin_data->mono_fx_buffers[(plugin_data->sample_mfx_groups_index[(plugin_data->current_sample)])][0][i] += (plugin_data->data[n]->modulex_current_sample[0]) * (plugin_data->data[n]->adsr_amp->output) * (plugin_data->amp);
            plugin_data->mono_fx_buffers[(plugin_data->sample_mfx_groups_index[(plugin_data->current_sample)])][1][i] += (plugin_data->data[n]->modulex_current_sample[1]) * (plugin_data->data[n]->adsr_amp->output) * (plugin_data->amp);
            
            plugin_data->i_loaded_samples = (plugin_data->i_loaded_samples) + 1;            
        }
    }
}

static inline void v_euphoria_slow_index(t_euphoria* plugin_data)
{    
    plugin_data->i_slow_index = 0;
    plugin_data->monofx_channel_index_count = 0;
    
    int i, i2, i3;

    for(i = 0; i  < (plugin_data->loaded_samples_count); i++)
    {
        plugin_data->monofx_index_contained = 0;

        for(i2 = 0; i2 < (plugin_data->monofx_channel_index_count); i2++)
        {
            if((plugin_data->monofx_channel_index[i2]) == ((int)(*(plugin_data->sample_mfx_groups[(plugin_data->loaded_samples[i])]))))
            {
                plugin_data->monofx_index_contained = 1;
                break;
            }
        }

        if((plugin_data->monofx_index_contained) == 0)
        {
            plugin_data->monofx_channel_index[(plugin_data->monofx_channel_index_count)] = (int)(*(plugin_data->sample_mfx_groups[(plugin_data->loaded_samples[i])]));
            plugin_data->monofx_channel_index_count = (plugin_data->monofx_channel_index_count) + 1;
        }
    }

    for(i2 = 0; i2 < EUPHORIA_MONO_FX_GROUPS_COUNT; i2++)
    {
        for(i3 = 0; i3 < EUPHORIA_MONO_FX_COUNT; i3++)
        {
            plugin_data->mono_modules->fx_func_ptr[i2][i3] = g_mf3_get_function_pointer((int)(*(plugin_data->mfx_comboboxes[i2][i3])));
        }
    }
}

static void v_run_lms_euphoria(PYFX_Handle instance, int sample_count,
		       t_pydaw_seq_event *events, int event_count)
{
    t_euphoria *plugin_data = (t_euphoria *) instance;    
    int event_pos = 0;
    int i, i2, i3;

    for (i = 0; i < plugin_data->channels; ++i)
    {
	memset(plugin_data->output[i], 0, sample_count * sizeof(float));
    }
    
    if (pthread_mutex_lock(&plugin_data->mutex))
    {
	return;
    }
    
    plugin_data->i_slow_index = (plugin_data->i_slow_index) + 1;
    
    if((plugin_data->i_slow_index) >= EUPHORIA_SLOW_INDEX_COUNT)
    {
        v_euphoria_slow_index(plugin_data);
    }    
    
    int f_note = 60;

    while (event_pos < event_count) // && pos >= events[event_pos].time.tick) 
    {
        /*Note-on event*/
        if (events[event_pos].type == SND_SEQ_EVENT_NOTEON) 
        {
            f_note = events[event_pos].note;
            
            if (events[event_pos].velocity > 0) 
            {
                int f_voice_num = i_pick_voice(plugin_data->voices, f_note, (plugin_data->sampleNo), events[event_pos].tick);
                plugin_data->velocities[f_voice_num] = events[event_pos].velocity;

                plugin_data->sample_indexes_count[f_voice_num] = 0;

                //Figure out which samples to play and stash all relevant values
                for(i = 0; i  < (plugin_data->loaded_samples_count); i++)
                {
                    if((f_note >= *(plugin_data->low_note[(plugin_data->loaded_samples[i])])) && 
                    (f_note <= *(plugin_data->high_note[(plugin_data->loaded_samples[i])])) &&
                    (plugin_data->velocities[f_voice_num] <= *(plugin_data->sample_vel_high[(plugin_data->loaded_samples[i])])) &&
                    (plugin_data->velocities[f_voice_num] >= *(plugin_data->sample_vel_low[(plugin_data->loaded_samples[i])])))
                    {
                        plugin_data->sample_indexes[f_voice_num][(plugin_data->sample_indexes_count[f_voice_num])] = (plugin_data->loaded_samples[i]);
                        plugin_data->sample_indexes_count[f_voice_num] = (plugin_data->sample_indexes_count[f_voice_num]) + 1;                            

                        plugin_data->sample_mfx_groups_index[(plugin_data->loaded_samples[i])] = (int)(*(plugin_data->sample_mfx_groups[(plugin_data->loaded_samples[i])]));

                        plugin_data->sampleStartPos[(plugin_data->loaded_samples[i])] = (EUPHORIA_SINC_INTERPOLATION_POINTS_DIV2 + ((plugin_data->sampleCount[(plugin_data->loaded_samples[i])]) * ((*(plugin_data->sampleStarts[(plugin_data->loaded_samples[i])])) * .0001)));
                        plugin_data->sampleLoopStartPos[(plugin_data->loaded_samples[i])] = (EUPHORIA_SINC_INTERPOLATION_POINTS_DIV2 + ((plugin_data->sampleCount[(plugin_data->loaded_samples[i])]) * ((*(plugin_data->sampleLoopStarts[(plugin_data->loaded_samples[i])])) * .0001)));

                        /* If loop mode is enabled for this sample, set the sample end to be the same as the
                           loop end.  Then, in the main loop, we'll recalculate sample_end to be the real sample end once
                           the note_off event is fired.  Doing it this way greatly reduces the need for extra if-then-else logic
                           in the main loop */
                        if(((int)(*(plugin_data->sampleLoopModes[(plugin_data->loaded_samples[i])]))) == 0)
                        {
                            plugin_data->sampleEndPos[(plugin_data->loaded_samples[i])] = (EUPHORIA_SINC_INTERPOLATION_POINTS_DIV2 + ((plugin_data->sampleCount[(plugin_data->loaded_samples[i])]) - ((int)(((float)((plugin_data->sampleCount[(plugin_data->loaded_samples[i])]) - 5)) * ((*(plugin_data->sampleEnds[(plugin_data->loaded_samples[i])])) * .0001)))));
                        }
                        else
                        {
                            plugin_data->sampleEndPos[(plugin_data->loaded_samples[i])] = (EUPHORIA_SINC_INTERPOLATION_POINTS_DIV2 + ((plugin_data->sampleCount[(plugin_data->loaded_samples[i])]) - ((int)(((float)((plugin_data->sampleCount[(plugin_data->loaded_samples[i])]) - 5)) * ((*(plugin_data->sampleLoopEnds[(plugin_data->loaded_samples[i])])) * .0001)))));
                        }

                        if((plugin_data->sampleEndPos[(plugin_data->loaded_samples[i])]) > ((float)((plugin_data->sampleCount[(plugin_data->loaded_samples[i])]))))
                        {
                            plugin_data->sampleEndPos[(plugin_data->loaded_samples[i])] = (float)(plugin_data->sampleCount[(plugin_data->loaded_samples[i])]);
                        }

                        plugin_data->adjusted_base_pitch[(plugin_data->loaded_samples[i])] = (*(plugin_data->basePitch[(plugin_data->loaded_samples[i])]))
                                - (*(plugin_data->sample_pitch[(plugin_data->loaded_samples[i])])) - ((*(plugin_data->sample_tune[(plugin_data->loaded_samples[i])])) * .01f);

                        v_ifh_retrigger(plugin_data->sample_read_heads[f_voice_num][(plugin_data->loaded_samples[i])],
                                (plugin_data->sampleStartPos[(plugin_data->loaded_samples[i])]));// 0.0f;
                        
                        plugin_data->vel_sens_output[f_voice_num][(plugin_data->loaded_samples[i])] = 
                                (1.0f -
                                (((float)(events[event_pos].velocity) - (*(plugin_data->sample_vel_low[(plugin_data->loaded_samples[i])])))
                                /
                                ((float)(*(plugin_data->sample_vel_high[(plugin_data->loaded_samples[i])]) - (*(plugin_data->sample_vel_low[(plugin_data->loaded_samples[i])]))))))
                                * (*(plugin_data->sample_vel_sens[(plugin_data->loaded_samples[i])])) * -1.0f;

                        plugin_data->sample_amp[(plugin_data->loaded_samples[i])] = f_db_to_linear(
                            (*(plugin_data->sample_vol[(plugin_data->loaded_samples[i])])) + 
                            (plugin_data->vel_sens_output[f_voice_num][(plugin_data->loaded_samples[i])])                        
                            , plugin_data->amp_ptr);

                        switch((int)(*(plugin_data->sample_interpolation_mode[(plugin_data->loaded_samples[i])])))
                        {
                            case 0:
                                interpolation_modes[(plugin_data->loaded_samples[i])] = run_sampler_interpolation_sinc;
                                ratio_function_ptrs[(plugin_data->loaded_samples[i])] = calculate_ratio_sinc;
                                break;
                            case 1:
                                interpolation_modes[(plugin_data->loaded_samples[i])] = run_sampler_interpolation_linear;
                                ratio_function_ptrs[(plugin_data->loaded_samples[i])] = calculate_ratio_linear;
                                break;
                            case 2:
                                interpolation_modes[(plugin_data->loaded_samples[i])] = run_sampler_interpolation_none;
                                ratio_function_ptrs[(plugin_data->loaded_samples[i])] = calculate_ratio_none;
                                break;
                            default:
                                printf("Error, invalid interpolation mode %i\n", ((int)(*(plugin_data->sample_interpolation_mode[(plugin_data->loaded_samples[i])]))));
                        }
                    }
                }

                plugin_data->active_polyfx_count[f_voice_num] = 0;
                //Determine which PolyFX have been enabled
                for(plugin_data->i_dst = 0; (plugin_data->i_dst) < EUPHORIA_MODULAR_POLYFX_COUNT; plugin_data->i_dst = (plugin_data->i_dst) + 1)
                {
                    int f_pfx_combobox_index = (int)(*(plugin_data->fx_combobox[0][(plugin_data->i_dst)]));
                    plugin_data->data[f_voice_num]->fx_func_ptr[(plugin_data->i_dst)] = g_mf3_get_function_pointer(f_pfx_combobox_index); 

                    if(f_pfx_combobox_index != 0)
                    {
                        plugin_data->active_polyfx[f_voice_num][(plugin_data->active_polyfx_count[f_voice_num])] = (plugin_data->i_dst);
                        plugin_data->active_polyfx_count[f_voice_num] = (plugin_data->active_polyfx_count[f_voice_num]) + 1;
                    }
                }    

                //Calculate an index of which mod_matrix controls to process.  This saves expensive iterations and if/then logic in the main loop
                for(plugin_data->i_fx_grps = 0; (plugin_data->i_fx_grps) < EUPHORIA_EFFECTS_GROUPS_COUNT; plugin_data->i_fx_grps = (plugin_data->i_fx_grps) + 1)
                {
                    for(plugin_data->i_dst = 0; (plugin_data->i_dst) < (plugin_data->active_polyfx_count[f_voice_num]); plugin_data->i_dst = (plugin_data->i_dst) + 1)
                    {
                        plugin_data->polyfx_mod_counts[f_voice_num][(plugin_data->active_polyfx[f_voice_num][(plugin_data->i_dst)])] = 0;

                        for(plugin_data->i_src = 0; (plugin_data->i_src) < EUPHORIA_MODULATOR_COUNT; plugin_data->i_src = (plugin_data->i_src) + 1)
                        {
                            for(plugin_data->i_ctrl = 0; (plugin_data->i_ctrl) < EUPHORIA_CONTROLS_PER_MOD_EFFECT; plugin_data->i_ctrl = (plugin_data->i_ctrl) + 1)
                            {
                                if((*(plugin_data->polyfx_mod_matrix[(plugin_data->i_fx_grps)][(plugin_data->active_polyfx[f_voice_num][(plugin_data->i_dst)])][(plugin_data->i_src)][(plugin_data->i_ctrl)])) != 0)
                                {                                        
                                    plugin_data->polyfx_mod_ctrl_indexes[f_voice_num][(plugin_data->active_polyfx[f_voice_num][(plugin_data->i_dst)])][(plugin_data->polyfx_mod_counts[f_voice_num][(plugin_data->active_polyfx[f_voice_num][(plugin_data->i_dst)])])] = (plugin_data->i_ctrl);
                                    plugin_data->polyfx_mod_src_index[f_voice_num][(plugin_data->active_polyfx[f_voice_num][(plugin_data->i_dst)])][(plugin_data->polyfx_mod_counts[f_voice_num][(plugin_data->active_polyfx[f_voice_num][(plugin_data->i_dst)])])] = (plugin_data->i_src);
                                    plugin_data->polyfx_mod_matrix_values[f_voice_num][(plugin_data->active_polyfx[f_voice_num][(plugin_data->i_dst)])][(plugin_data->polyfx_mod_counts[f_voice_num][(plugin_data->active_polyfx[f_voice_num][(plugin_data->i_dst)])])] = 
                                            (*(plugin_data->polyfx_mod_matrix[(plugin_data->i_fx_grps)][(plugin_data->active_polyfx[f_voice_num][(plugin_data->i_dst)])][(plugin_data->i_src)][(plugin_data->i_ctrl)])) * .01;

                                    plugin_data->polyfx_mod_counts[f_voice_num][(plugin_data->active_polyfx[f_voice_num][(plugin_data->i_dst)])] = (plugin_data->polyfx_mod_counts[f_voice_num][(plugin_data->active_polyfx[f_voice_num][(plugin_data->i_dst)])]) + 1;
                                }
                            }
                        }
                    }
                }
                //Get the noise function pointer
                plugin_data->data[f_voice_num]->noise_func_ptr = fp_get_noise_func_ptr((int)(*(plugin_data->noise_type)));

                plugin_data->data[f_voice_num]->noise_index = (plugin_data->mono_modules->noise_current_index);
                plugin_data->mono_modules->noise_current_index = (plugin_data->mono_modules->noise_current_index) + 1;

                if((plugin_data->mono_modules->noise_current_index) >= EUPHORIA_NOISE_COUNT)
                {
                    plugin_data->mono_modules->noise_current_index = 0;
                }
                
                plugin_data->amp = f_db_to_linear_fast(*(plugin_data->master_vol), plugin_data->mono_modules->amp_ptr);                     

                plugin_data->data[f_voice_num]->note_f = (float)f_note;

                plugin_data->data[f_voice_num]->target_pitch = (plugin_data->data[f_voice_num]->note_f);
                plugin_data->data[f_voice_num]->last_pitch = (plugin_data->sv_last_note);

                v_rmp_retrigger_glide_t(plugin_data->data[f_voice_num]->glide_env , (*(plugin_data->master_glide) * .01), 
                        (plugin_data->sv_last_note), (plugin_data->data[f_voice_num]->target_pitch));

                plugin_data->data[f_voice_num]->noise_linamp = f_db_to_linear_fast(*(plugin_data->noise_amp), plugin_data->mono_modules->amp_ptr);

                /*Here is where we perform any actions that should ONLY happen at note_on, you can save a lot of CPU by
                 placing things here that don't need to be modulated as a note is playing*/

                /*Retrigger ADSR envelopes and LFO*/
                v_adsr_retrigger(plugin_data->data[f_voice_num]->adsr_amp);
                v_adsr_retrigger(plugin_data->data[f_voice_num]->adsr_filter);
                v_lfs_sync(plugin_data->data[f_voice_num]->lfo1, 0.0f, *(plugin_data->lfo_type));

                float f_attack_a = (*(plugin_data->attack) * .01);  f_attack_a *= f_attack_a;
                float f_decay_a = (*(plugin_data->decay) * .01);  f_decay_a *= f_decay_a;
                float f_release_a = (*(plugin_data->release) * .01); f_release_a *= f_release_a;
                v_adsr_set_adsr_db(plugin_data->data[f_voice_num]->adsr_amp, f_attack_a, f_decay_a, (*(plugin_data->sustain)), f_release_a);
                
                float f_attack_f = (*(plugin_data->attack_f) * .01);  f_attack_f *= f_attack_f;
                float f_decay_f = (*(plugin_data->decay_f) * .01);  f_decay_f *= f_decay_f;
                float f_release_f = (*(plugin_data->release_f) * .01); f_release_f *= f_release_f;
                v_adsr_set_adsr(plugin_data->data[f_voice_num]->adsr_filter, f_attack_f, f_decay_f, (*(plugin_data->sustain_f) * .01), f_release_f);

                /*Retrigger the pitch envelope*/
                v_rmp_retrigger((plugin_data->data[f_voice_num]->ramp_env), (*(plugin_data->pitch_env_time) * .01), 1.0f);  

                plugin_data->data[f_voice_num]->noise_amp = f_db_to_linear(*(plugin_data->noise_amp), plugin_data->mono_modules->amp_ptr);

                /*Set the last_note property, so the next note can glide from it if glide is turned on*/
                plugin_data->sv_last_note = (plugin_data->data[f_voice_num]->note_f);

                //TODO:  Create a define for the number of channels
                //Move all of the multi-channel functions here
                //for(i = 0; i < 2; i++)
                //{

                //}

            } 
            else 
            {
                v_voc_note_off(plugin_data->voices, events[event_pos].note, plugin_data->sampleNo, events[event_pos].tick);
            }
        } /*Note-off event*/
        else if (events[event_pos].type == SND_SEQ_EVENT_NOTEOFF )
        {            
            f_note = events[event_pos].note; 
            v_voc_note_off(plugin_data->voices, events[event_pos].note, plugin_data->sampleNo, events[event_pos].tick);
        }

        /*Pitch-bend sequencer event, modify the voices pitch*/
        else if (events[event_pos].type == SND_SEQ_EVENT_PITCHBEND) 
        {
            plugin_data->sv_pitch_bend_value = 0.00012207 * events[event_pos].value * (*(plugin_data->master_pb_amt));
        }

        ++event_pos;
    }
    
    v_smr_iir_run_fast(plugin_data->mono_modules->pitchbend_smoother, (plugin_data->sv_pitch_bend_value));

    for(i = 0; i < sample_count; i++)
    {
        for(i2 = 0; i2 < (plugin_data->monofx_channel_index_count); i2++)
        {            
            plugin_data->mono_fx_buffers[(plugin_data->monofx_channel_index[i2])][0][i] = 0.0f;
            plugin_data->mono_fx_buffers[(plugin_data->monofx_channel_index[i2])][1][i] = 0.0f;            
        }
    }

    for (i = 0; i < EUPHORIA_POLYPHONY; ++i) 
    {
        if(((plugin_data->data[i]->adsr_amp->stage) < 4) && ((plugin_data->sample_indexes_count[i]) > 0))
        {    
            add_sample_lms_euphoria(plugin_data, i, sample_count);                                
        }
    }

    float f_temp_sample0, f_temp_sample1;

    for(i = 0; i < sample_count; i++)        
    {
        for(i2 = 0; i2 < (plugin_data->monofx_channel_index_count); i2++)
        {
            f_temp_sample0 = (plugin_data->mono_fx_buffers[(plugin_data->monofx_channel_index[i2])][0][i]);
            f_temp_sample1 = (plugin_data->mono_fx_buffers[(plugin_data->monofx_channel_index[i2])][1][i]);

            for(i3 = 0; i3 < EUPHORIA_MONO_FX_COUNT; i3++)
            {
                v_mf3_set(plugin_data->mono_modules->multieffect[(plugin_data->monofx_channel_index[i2])][i3], (*(plugin_data->mfx_knobs[(plugin_data->monofx_channel_index[i2])][i3][0])), (*(plugin_data->mfx_knobs[(plugin_data->monofx_channel_index[i2])][i3][1])), (*(plugin_data->mfx_knobs[(plugin_data->monofx_channel_index[i2])][i3][2])));
                plugin_data->mono_modules->fx_func_ptr[(plugin_data->monofx_channel_index[i2])][i3](plugin_data->mono_modules->multieffect[(plugin_data->monofx_channel_index[i2])][i3], f_temp_sample0, f_temp_sample1);

                f_temp_sample0 = (plugin_data->mono_modules->multieffect[(plugin_data->monofx_channel_index[i2])][i3]->output0);
                f_temp_sample1 = (plugin_data->mono_modules->multieffect[(plugin_data->monofx_channel_index[i2])][i3]->output1);
            }

            plugin_data->output[0][i] += f_temp_sample0;
            plugin_data->output[1][i] += f_temp_sample1;
        }            
    }

    if(((plugin_data->preview_sample_array_index) < (plugin_data->preview_length)) &&
            ((plugin_data->preview_sample_array_index) <  (plugin_data->preview_sample_max_length)))
    {
        if((plugin_data->sampleNo) == 0)  //Workaround for the last previewed sample playing on project startup.  This will be removed when the preview functionality gets reworked
        {
            plugin_data->preview_sample_array_index = plugin_data->sampleCount[EUPHORIA_MAX_SAMPLE_COUNT];
            plugin_data->sample_paths[EUPHORIA_MAX_SAMPLE_COUNT][0] = '\0';
        }
        else
        {
            for(i = 0; i < sample_count; i++)
            {
                plugin_data->output[0][i] += plugin_data->mono_fx_buffers[0][0][i];
                plugin_data->output[1][i] += plugin_data->mono_fx_buffers[0][1][i];

                if(plugin_data->sampleData[0][(EUPHORIA_MAX_SAMPLE_COUNT)])
                {
                    //Add the previewing sample
                    plugin_data->output[0][i] += f_cubic_interpolate_ptr(plugin_data->sampleData[0][(EUPHORIA_MAX_SAMPLE_COUNT)], 
                                (plugin_data->preview_sample_array_index), plugin_data->cubic_interpolator);

                    if(plugin_data->sample_channels[(EUPHORIA_MAX_SAMPLE_COUNT)] > 1)
                    {
                        plugin_data->output[1][i] += f_cubic_interpolate_ptr(plugin_data->sampleData[1][(EUPHORIA_MAX_SAMPLE_COUNT)], 
                                (plugin_data->preview_sample_array_index), plugin_data->cubic_interpolator);
                    }
                    else
                    {
                        plugin_data->output[1][i] += f_cubic_interpolate_ptr(plugin_data->sampleData[0][(EUPHORIA_MAX_SAMPLE_COUNT)], 
                                (plugin_data->preview_sample_array_index), plugin_data->cubic_interpolator);
                    }

                    plugin_data->preview_sample_array_index = (plugin_data->preview_sample_array_index) + (plugin_data->sample_rate_ratios[EUPHORIA_MAX_SAMPLE_COUNT]);

                    if((plugin_data->preview_sample_array_index) >= (plugin_data->preview_length))
                    {
                        plugin_data->sample_paths[EUPHORIA_MAX_SAMPLE_COUNT][0] = '\0';
                        break;
                    }
                }
            }
        }
    }
    
    plugin_data->sampleNo += sample_count;
    pthread_mutex_unlock(&plugin_data->mutex);
}

static char * PYINST_configure_message(const char *fmt, ...)
{
    va_list args;
    char buffer[256];

    va_start(args, fmt);
    vsnprintf(buffer, 256, fmt, args);
    va_end(args);
    return strdup(buffer);
}

static char *c_euphoria_sampler_load(t_euphoria *plugin_data, const char *path, int a_index)
{   
    /*Add that index to the list of loaded samples to iterate though when playing, if not already added*/
    
    if(a_index != EUPHORIA_MAX_SAMPLE_COUNT)
    {
        int i_loaded_samples = 0;
        plugin_data->sample_is_loaded = 0;

        while((i_loaded_samples) < (plugin_data->loaded_samples_count))
        {
            if((plugin_data->loaded_samples[(i_loaded_samples)]) == a_index)
            {
                //printf("Sample index %i is already loaded.\n", (i_loaded_samples));
                plugin_data->sample_is_loaded = 1;
                break;
            }
            i_loaded_samples++;
        }

        if((plugin_data->sample_is_loaded) == 0)
        {
            plugin_data->loaded_samples[(plugin_data->loaded_samples_count)] = a_index;
            plugin_data->loaded_samples_count = (plugin_data->loaded_samples_count) + 1;
            printf("plugin_data->loaded_samples_count == %i\n", (plugin_data->loaded_samples_count));
        }
    }
    
    SF_INFO info;
    SNDFILE *file;
    size_t samples = 0;
    float *tmpFrames, *tmpSamples[2], *tmpOld[2];
        
    tmpOld[0] = 0;
    tmpOld[1] = 0;
    
    info.format = 0;
    file = sf_open(path, SFM_READ, &info);

    if (!file) {

	const char *filename = strrchr(path, '/');
	if (filename) ++filename;
	else filename = path;
        
	if (!file) {
	    return PYINST_configure_message
		("error: unable to load sample file '%s'", path);
	}
    }
    
    if (info.frames > EUPHORIA_FRAMES_MAX) {
	return PYINST_configure_message
	    ("error: sample file '%s' is too large (%ld frames, maximum is %ld)",
	     path, info.frames, EUPHORIA_FRAMES_MAX);
    }

    //!!! complain also if more than 2 channels
    
    samples = info.frames;

    tmpFrames = (float *)malloc(info.frames * info.channels * sizeof(float));
    sf_readf_float(file, tmpFrames, info.frames);
    sf_close(file);

    if ((int)(info.samplerate) != (int)(plugin_data->sampleRate)) 
    {	
	double ratio = (double)(info.samplerate)/(double)(plugin_data->sampleRate);
	
        plugin_data->sample_rate_ratios[(a_index)] = (float)ratio;
    }
    else
    {
        plugin_data->sample_rate_ratios[(a_index)] = 1.0f;
    }
           
    int f_adjusted_channel_count = 1;
    if(info.channels >= 2)    
    {
        f_adjusted_channel_count = 2;
    }

    int f_actual_array_size = (samples + EUPHORIA_SINC_INTERPOLATION_POINTS + 1 + EUPHORIA_Sample_Padding);

    if(posix_memalign((void**)(&(tmpSamples[0])), 16, ((f_actual_array_size) * sizeof(float))) != 0)
    {
        printf("Call to posix_memalign failed for tmpSamples[0]\n");
        return NULL;
    }
    if(f_adjusted_channel_count > 1)
    {
        if(posix_memalign((void**)(&(tmpSamples[1])), 16, ((f_actual_array_size) * sizeof(float))) != 0)
        {
            printf("Call to posix_memalign failed for tmpSamples[1]\n");
            return NULL;
        }
    }
    
    int f_i, j;
    
    for(f_i = 0; f_i < EUPHORIA_CHANNEL_COUNT; f_i++)
    {
        v_dco_reset(plugin_data->mono_modules->dc_offset_filters[f_i]);
    }    
    
    //For performing a 5ms fadeout of the sample, for preventing clicks
    float f_fade_out_dec = (1.0f/(float)(info.samplerate))/(0.005);
    int f_fade_out_start = (samples + EUPHORIA_SINC_INTERPOLATION_POINTS_DIV2) - ((int)(0.005f * ((float)(info.samplerate))));
    float f_fade_out_envelope = 1.0f;
    float f_temp_sample = 0.0f;
        
    for(f_i = 0; f_i < f_actual_array_size; f_i++)
    {   
        if((f_i > EUPHORIA_SINC_INTERPOLATION_POINTS_DIV2) && (f_i < (samples + EUPHORIA_SINC_INTERPOLATION_POINTS_DIV2))) // + Sampler_Sample_Padding)))
        {
            if(f_i >= f_fade_out_start)
            {
                if(f_fade_out_envelope <= 0.0f)
                {
                    f_fade_out_dec = 0.0f;
                }
                
                f_fade_out_envelope -= f_fade_out_dec;
            }
            
	    for (j = 0; j < f_adjusted_channel_count; ++j) 
            {
                f_temp_sample = //(tmpFrames[(f_i - LMS_SINC_INTERPOLATION_POINTS_DIV2) * info.channels + j]);
                        f_dco_run(plugin_data->mono_modules->dc_offset_filters[j], 
                        (tmpFrames[(f_i - EUPHORIA_SINC_INTERPOLATION_POINTS_DIV2) * info.channels + j]));

                if(f_i >= f_fade_out_start)
                {
                    tmpSamples[j][f_i] = f_temp_sample * f_fade_out_envelope;
                }
                else
                {
                    tmpSamples[j][f_i] = f_temp_sample;
                }
            }
        }
        else
        {
            tmpSamples[0][f_i] = 0.0f;
            if(f_adjusted_channel_count > 1)
            {
                tmpSamples[1][f_i] = 0.0f;
            }
        }            
    }
        
    free(tmpFrames);
    
    pthread_mutex_lock(&plugin_data->mutex);
    
    if(plugin_data->sampleData[0][(a_index)])
    {
        tmpOld[0] = plugin_data->sampleData[0][(a_index)];
    }
    
    if(plugin_data->sampleData[1][(a_index)])
    {
        tmpOld[1] = plugin_data->sampleData[1][(a_index)];
    }
    
    plugin_data->sampleData[0][(a_index)] = tmpSamples[0];
    
    if(f_adjusted_channel_count > 1)
    {
        plugin_data->sampleData[1][(a_index)] = tmpSamples[1];
    }
    else
    {
        plugin_data->sampleData[1][(a_index)] = 0;
    }
    
    plugin_data->sampleCount[(a_index)] = (samples + EUPHORIA_SINC_INTERPOLATION_POINTS_DIV2 + EUPHORIA_Sample_Padding - 20);  //-20 to ensure we don't read past the end of the array
    
    plugin_data->sample_channels[(a_index)] = f_adjusted_channel_count;
    
    sprintf(plugin_data->sample_paths[(a_index)], "%s", path);
        
    //The last index is reserved for previewing samples for the UI;
    //Reset the array indexer so it will play from the beginning.
    if(a_index == EUPHORIA_MAX_SAMPLE_COUNT)
    {
        plugin_data->preview_sample_array_index = (float)(EUPHORIA_SINC_INTERPOLATION_POINTS_DIV2 + EUPHORIA_Sample_Padding);
        plugin_data->preview_length = (float)(plugin_data->sampleCount[EUPHORIA_MAX_SAMPLE_COUNT]);
    }
    
    pthread_mutex_unlock(&plugin_data->mutex);
    
    if (tmpOld[0]) 
    {
        free(tmpOld[0]);
    }
    
    if (tmpOld[1]) 
    {
        free(tmpOld[1]);
    }
    
    return NULL;
}

static char *c_euphoria_clear(t_euphoria *plugin_data, int a_index)
{
    plugin_data->sample_paths[a_index][0] = '\0';
    
    if(a_index != EUPHORIA_MAX_SAMPLE_COUNT)
    {
        if((plugin_data->loaded_samples_count) == 0)
        {
            return NULL;
        }

        /*Add that index to the list of loaded samples to iterate though when playing, if not already added*/
        plugin_data->i_loaded_samples = 0;
        plugin_data->sample_is_loaded = 0;

        while((plugin_data->i_loaded_samples) < (plugin_data->loaded_samples_count))
        {
            if((plugin_data->loaded_samples[(plugin_data->i_loaded_samples)]) == (a_index))
            {
                printf("Sample index %i is loaded, clearing.\n", (plugin_data->i_loaded_samples));
                plugin_data->sample_is_loaded = 1;
                break;
            }
            plugin_data->i_loaded_samples = (plugin_data->i_loaded_samples) + 1;
        }

        if((plugin_data->sample_is_loaded) == 0)
        {        
            return NULL;
        }
        else
        {
            if((plugin_data->loaded_samples_count) == 1)
            {
                plugin_data->loaded_samples_count = 0;
            }
            else
            {
                plugin_data->loaded_samples[(plugin_data->i_loaded_samples)] = (plugin_data->loaded_samples[(plugin_data->loaded_samples_count) - 1]);            
                plugin_data->loaded_samples_count = (plugin_data->loaded_samples_count) - 1;        
            }        
        }
    }
    float *tmpOld[2];    
    
    pthread_mutex_lock(&plugin_data->mutex);

    tmpOld[0] = plugin_data->sampleData[0][(a_index)];
    tmpOld[1] = plugin_data->sampleData[1][(a_index)];
    plugin_data->sampleData[0][(a_index)] = 0;
    plugin_data->sampleData[1][(a_index)] = 0;
    plugin_data->sampleCount[(a_index)] = 0;

    pthread_mutex_unlock(&plugin_data->mutex);
    
    if (tmpOld[0]) free(tmpOld[0]);
    if (tmpOld[1]) free(tmpOld[1]);
    
    return NULL;
}

/* Call samplerLoad for all samples.*/
static char *c_euphoria_load_all(t_euphoria *plugin_data, const char *paths)
{       
    sprintf(plugin_data->sample_files, "%s", paths);
    
    int f_index = 0;
    int f_samples_loaded_count = 0;
    int f_current_string_index = 0;
    
    char * f_result_string = (char*)malloc(sizeof(char) * 1024);
    
    while (f_samples_loaded_count < EUPHORIA_TOTAL_SAMPLE_COUNT)
    {    
        if(paths[f_index] == '\0')
        {
            break;
        }
        else if(paths[f_index] == EUPHORIA_FILES_STRING_DELIMITER)
        {
            f_result_string[f_current_string_index] = '\0';
            
            if(f_current_string_index == 0)
            {
                c_euphoria_clear(plugin_data, f_samples_loaded_count);
            }
            else if(strcmp(f_result_string, (plugin_data->sample_paths[f_samples_loaded_count])) != 0)
            {
                c_euphoria_sampler_load(plugin_data,f_result_string,f_samples_loaded_count);                
            }
            f_current_string_index = 0;
            f_samples_loaded_count++;
        }
        else if(paths[f_index] == EUPHORIA_FILES_STRING_RELOAD_DELIMITER)
        {            
            f_result_string[f_current_string_index] = '\0';
            
            if(f_current_string_index == 0)
            {
                c_euphoria_clear(plugin_data, f_samples_loaded_count);
            }
            else
            {
                c_euphoria_sampler_load(plugin_data,f_result_string,f_samples_loaded_count);
            }
            f_current_string_index = 0;
            f_samples_loaded_count++;
        }
        else
        {
            f_result_string[f_current_string_index] = paths[f_index];
            f_current_string_index++;
        }
        
        f_index++;
    }
    
    free(f_result_string);
    
    return NULL;
}

char *c_euphoria_configure(PYFX_Handle instance, const char *key, const char *value)
{
    t_euphoria *plugin_data = (t_euphoria *)instance;

    if (!strcmp(key, "load")) {	
        return c_euphoria_load_all(plugin_data, value);    
    /*} else if (!strcmp(key, PYINST_PROJECT_DIRECTORY_KEY)) {
	if (plugin_data->projectDir) free(plugin_data->projectDir);
	plugin_data->projectDir = strdup(value);
	return 0;*/
    } else if (!strcmp(key, "lastdir")) {
        //do nothing, this is only so the plugin host will recall the last sample directory
        return NULL;
    }

    return strdup("error: unrecognized configure key");
}

const PYFX_Descriptor *euphoria_PYFX_descriptor(int index)
{
    PYFX_Descriptor *euphoriaLDescriptor = NULL;

    char **port_names;
    PYFX_PortDescriptor *port_descriptors;
    PYFX_PortRangeHint *port_range_hints;
    int channels;
    int * automatable;
    int * value_tranform_hints;

    euphoriaLDescriptor = (PYFX_Descriptor *) malloc(sizeof(PYFX_Descriptor));

    for (channels = 1; channels <= 2; ++channels) {

	PYFX_Descriptor *desc = euphoriaLDescriptor;

	desc->UniqueID = channels;
	desc->Label = EUPHORIA_PLUGIN_NAME;
	desc->Name =  EUPHORIA_PLUGIN_LONG_NAME;
	desc->Maker = EUPHORIA_PLUGIN_DEV;
	desc->Copyright = "GPL";
	desc->PortCount = EUPHORIA_PORT_COUNT;

	port_descriptors = (PYFX_PortDescriptor *)
	    calloc(desc->PortCount, sizeof(PYFX_PortDescriptor));
	desc->PortDescriptors =
	    (const PYFX_PortDescriptor *) port_descriptors;

	port_range_hints = (PYFX_PortRangeHint *)
	    calloc(desc->PortCount, sizeof (PYFX_PortRangeHint));
	desc->PortRangeHints =
	    (const PYFX_PortRangeHint *) port_range_hints;

	port_names = (char **) calloc(desc->PortCount, sizeof(char *));
	desc->PortNames = (const char **) port_names;

        automatable = (int*)calloc(desc->PortCount, sizeof(int));
        desc->Automatable = automatable;
        
        value_tranform_hints = (int*)calloc(desc->PortCount, sizeof(int));
        desc->ValueTransformHint = value_tranform_hints;
        
	/* Parameters for output left */
	port_descriptors[EUPHORIA_OUTPUT_LEFT] = PYFX_PORT_OUTPUT | PYFX_PORT_AUDIO;
	port_names[EUPHORIA_OUTPUT_LEFT] = "Output L";
	port_range_hints[EUPHORIA_OUTPUT_LEFT].DefaultValue = 0.0f;
        
        /* Parameters for output right */
        port_descriptors[EUPHORIA_OUTPUT_RIGHT] = PYFX_PORT_OUTPUT | PYFX_PORT_AUDIO;
        port_names[EUPHORIA_OUTPUT_RIGHT] = "Output R";
        port_range_hints[EUPHORIA_OUTPUT_RIGHT].DefaultValue = 0.0f;


        /* Parameters for selected sample */
	port_descriptors[EUPHORIA_SELECTED_SAMPLE] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[EUPHORIA_SELECTED_SAMPLE] = "Selected Sample";
	port_range_hints[EUPHORIA_SELECTED_SAMPLE].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_SELECTED_SAMPLE].LowerBound = 0;
	port_range_hints[EUPHORIA_SELECTED_SAMPLE].UpperBound = (EUPHORIA_MAX_SAMPLE_COUNT - 1);
        
	port_descriptors[EUPHORIA_ATTACK] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[EUPHORIA_ATTACK] = "Master Attack";
	port_range_hints[EUPHORIA_ATTACK].DefaultValue = 10.0f;
	port_range_hints[EUPHORIA_ATTACK].LowerBound = 0.0f; 
	port_range_hints[EUPHORIA_ATTACK].UpperBound = 100.0f; 
        automatable[EUPHORIA_ATTACK] = 1;
        value_tranform_hints[EUPHORIA_ATTACK] = PYDAW_PLUGIN_HINT_TRANSFORM_DECIMAL;

	port_descriptors[EUPHORIA_DECAY] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[EUPHORIA_DECAY] = "Master Decay";
	port_range_hints[EUPHORIA_DECAY].DefaultValue = 50.0f;
	port_range_hints[EUPHORIA_DECAY].LowerBound = 10; 
	port_range_hints[EUPHORIA_DECAY].UpperBound = 100; 
        automatable[EUPHORIA_DECAY] = 1;
        value_tranform_hints[EUPHORIA_DECAY] = PYDAW_PLUGIN_HINT_TRANSFORM_DECIMAL;

	/* Parameters for sustain */
	port_descriptors[EUPHORIA_SUSTAIN] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[EUPHORIA_SUSTAIN] = "Master Sustain";
	port_range_hints[EUPHORIA_SUSTAIN].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_SUSTAIN].LowerBound = -60;
	port_range_hints[EUPHORIA_SUSTAIN].UpperBound = 0;         
        automatable[EUPHORIA_SUSTAIN] = 1;

	/* Parameters for release */
	port_descriptors[EUPHORIA_RELEASE] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[EUPHORIA_RELEASE] = "Master Release";
	port_range_hints[EUPHORIA_RELEASE].DefaultValue = 50.0f;
	port_range_hints[EUPHORIA_RELEASE].LowerBound = 10; 
	port_range_hints[EUPHORIA_RELEASE].UpperBound = 200; 
        automatable[EUPHORIA_RELEASE] = 1;
        value_tranform_hints[EUPHORIA_RELEASE] = PYDAW_PLUGIN_HINT_TRANSFORM_DECIMAL;
                
	/* Parameters for attack_f */
	port_descriptors[EUPHORIA_FILTER_ATTACK] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[EUPHORIA_FILTER_ATTACK] = "ADSR2 Attack";
	port_range_hints[EUPHORIA_FILTER_ATTACK].DefaultValue = 10.0f;
	port_range_hints[EUPHORIA_FILTER_ATTACK].LowerBound = 0.0f; 
	port_range_hints[EUPHORIA_FILTER_ATTACK].UpperBound = 100.0f; 
        automatable[EUPHORIA_FILTER_ATTACK] = 1;
        value_tranform_hints[EUPHORIA_FILTER_ATTACK] = PYDAW_PLUGIN_HINT_TRANSFORM_DECIMAL;

	/* Parameters for decay_f */
	port_descriptors[EUPHORIA_FILTER_DECAY] = port_descriptors[EUPHORIA_ATTACK];
	port_names[EUPHORIA_FILTER_DECAY] = "ADSR2 Decay";
	port_range_hints[EUPHORIA_FILTER_DECAY].DefaultValue = 50.0f;
	port_range_hints[EUPHORIA_FILTER_DECAY].LowerBound = 10;
	port_range_hints[EUPHORIA_FILTER_DECAY].UpperBound = 100;
        automatable[EUPHORIA_FILTER_DECAY] = 1;
        value_tranform_hints[EUPHORIA_FILTER_DECAY] = PYDAW_PLUGIN_HINT_TRANSFORM_DECIMAL;

	/* Parameters for sustain_f */
	port_descriptors[EUPHORIA_FILTER_SUSTAIN] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[EUPHORIA_FILTER_SUSTAIN] = "ADSR2 Sustain";
	port_range_hints[EUPHORIA_FILTER_SUSTAIN].DefaultValue = 100.0f;
	port_range_hints[EUPHORIA_FILTER_SUSTAIN].LowerBound = 0; 
	port_range_hints[EUPHORIA_FILTER_SUSTAIN].UpperBound = 100; 
        automatable[EUPHORIA_FILTER_SUSTAIN] = 1;
        value_tranform_hints[EUPHORIA_FILTER_SUSTAIN] = PYDAW_PLUGIN_HINT_TRANSFORM_DECIMAL;
        
	/* Parameters for release_f */
	port_descriptors[EUPHORIA_FILTER_RELEASE] = port_descriptors[EUPHORIA_ATTACK];
	port_names[EUPHORIA_FILTER_RELEASE] = "ADSR2 Release";
	port_range_hints[EUPHORIA_FILTER_RELEASE].DefaultValue = 50.0f;
	port_range_hints[EUPHORIA_FILTER_RELEASE].LowerBound = 10; 
	port_range_hints[EUPHORIA_FILTER_RELEASE].UpperBound = 200; 
        automatable[EUPHORIA_FILTER_RELEASE] = 1;
        value_tranform_hints[EUPHORIA_FILTER_RELEASE] = PYDAW_PLUGIN_HINT_TRANSFORM_DECIMAL;

        
        /*Parameters for noise_amp*/        
	port_descriptors[EUPHORIA_NOISE_AMP] = port_descriptors[EUPHORIA_ATTACK];
	port_names[EUPHORIA_NOISE_AMP] = "Noise Amp";
	port_range_hints[EUPHORIA_NOISE_AMP].DefaultValue = -30.0f;
	port_range_hints[EUPHORIA_NOISE_AMP].LowerBound =  -60;
	port_range_hints[EUPHORIA_NOISE_AMP].UpperBound =  0;
        automatable[EUPHORIA_NOISE_AMP] = 1;
                
        /*Parameters for master vol*/        
	port_descriptors[EUPHORIA_MASTER_VOLUME] = port_descriptors[EUPHORIA_ATTACK];
	port_names[EUPHORIA_MASTER_VOLUME] = "Master Vol";
	port_range_hints[EUPHORIA_MASTER_VOLUME].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_MASTER_VOLUME].LowerBound =  -24;
	port_range_hints[EUPHORIA_MASTER_VOLUME].UpperBound =  24;
                                
        /*Parameters for master glide*/        
	port_descriptors[EUPHORIA_MASTER_GLIDE] = port_descriptors[EUPHORIA_ATTACK];
	port_names[EUPHORIA_MASTER_GLIDE] = "Master Glide";
	port_range_hints[EUPHORIA_MASTER_GLIDE].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_MASTER_GLIDE].LowerBound =  0.0f;
	port_range_hints[EUPHORIA_MASTER_GLIDE].UpperBound =  200.0f;
        automatable[EUPHORIA_MASTER_GLIDE] = 1;
        value_tranform_hints[EUPHORIA_MASTER_GLIDE] = PYDAW_PLUGIN_HINT_TRANSFORM_DECIMAL;
        
        
        /*Parameters for master pitchbend amt*/        
	port_descriptors[EUPHORIA_MASTER_PITCHBEND_AMT] = port_descriptors[EUPHORIA_ATTACK];
	port_names[EUPHORIA_MASTER_PITCHBEND_AMT] = "Pitchbend Amt";
	port_range_hints[EUPHORIA_MASTER_PITCHBEND_AMT].DefaultValue = 18.0f;
	port_range_hints[EUPHORIA_MASTER_PITCHBEND_AMT].LowerBound =  1;
	port_range_hints[EUPHORIA_MASTER_PITCHBEND_AMT].UpperBound =  36;
        
        /*Parameters for pitch env time*/        
	port_descriptors[EUPHORIA_PITCH_ENV_TIME] = port_descriptors[EUPHORIA_ATTACK];
	port_names[EUPHORIA_PITCH_ENV_TIME] = "Pitch Env Time";
	port_range_hints[EUPHORIA_PITCH_ENV_TIME].DefaultValue = 100.0f;
	port_range_hints[EUPHORIA_PITCH_ENV_TIME].LowerBound = 0; 
	port_range_hints[EUPHORIA_PITCH_ENV_TIME].UpperBound = 200;
        automatable[EUPHORIA_PITCH_ENV_TIME] = 1;
        value_tranform_hints[EUPHORIA_PITCH_ENV_TIME] = PYDAW_PLUGIN_HINT_TRANSFORM_DECIMAL;
        
        /*Parameters for LFO Freq*/        
	port_descriptors[EUPHORIA_LFO_FREQ] = port_descriptors[EUPHORIA_ATTACK];
	port_names[EUPHORIA_LFO_FREQ] = "LFO Freq";
	port_range_hints[EUPHORIA_LFO_FREQ].DefaultValue = 200.0f;
	port_range_hints[EUPHORIA_LFO_FREQ].LowerBound = 10; 
	port_range_hints[EUPHORIA_LFO_FREQ].UpperBound = 1600;
        automatable[EUPHORIA_LFO_FREQ] = 1;
        value_tranform_hints[EUPHORIA_LFO_FREQ] = PYDAW_PLUGIN_HINT_TRANSFORM_DECIMAL;
        
        /*Parameters for LFO Type*/        
	port_descriptors[EUPHORIA_LFO_TYPE] = port_descriptors[EUPHORIA_ATTACK];
	port_names[EUPHORIA_LFO_TYPE] = "LFO Type";
	port_range_hints[EUPHORIA_LFO_TYPE].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_LFO_TYPE].LowerBound = 0; 
	port_range_hints[EUPHORIA_LFO_TYPE].UpperBound = 2;
        
        port_descriptors[EUPHORIA_FX0_KNOB0] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[EUPHORIA_FX0_KNOB0] = "FX0 Knob0";
	port_range_hints[EUPHORIA_FX0_KNOB0].DefaultValue = 64.0f;
	port_range_hints[EUPHORIA_FX0_KNOB0].LowerBound =  0;
	port_range_hints[EUPHORIA_FX0_KNOB0].UpperBound =  127;
        automatable[EUPHORIA_FX0_KNOB0] = 1;
        
        	
	port_descriptors[EUPHORIA_FX0_KNOB1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[EUPHORIA_FX0_KNOB1] = "FX0 Knob1";
	port_range_hints[EUPHORIA_FX0_KNOB1].DefaultValue = 64.0f;
	port_range_hints[EUPHORIA_FX0_KNOB1].LowerBound =  0;
	port_range_hints[EUPHORIA_FX0_KNOB1].UpperBound =  127;
        automatable[EUPHORIA_FX0_KNOB1] = 1;
        	
	port_descriptors[EUPHORIA_FX0_KNOB2] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[EUPHORIA_FX0_KNOB2] = "FX0 Knob2";
	port_range_hints[EUPHORIA_FX0_KNOB2].DefaultValue = 64.0f;
	port_range_hints[EUPHORIA_FX0_KNOB2].LowerBound =  0;
	port_range_hints[EUPHORIA_FX0_KNOB2].UpperBound =  127;
        automatable[EUPHORIA_FX0_KNOB2] = 1;
        
	port_descriptors[EUPHORIA_FX0_COMBOBOX] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[EUPHORIA_FX0_COMBOBOX] = "FX0 Type";
	port_range_hints[EUPHORIA_FX0_COMBOBOX].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_FX0_COMBOBOX].LowerBound =  0;
	port_range_hints[EUPHORIA_FX0_COMBOBOX].UpperBound =  MULTIFX3KNOB_MAX_INDEX;
        
        	
	port_descriptors[EUPHORIA_FX1_KNOB0] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[EUPHORIA_FX1_KNOB0] = "FX1 Knob0";
	port_range_hints[EUPHORIA_FX1_KNOB0].DefaultValue = 64.0f;
	port_range_hints[EUPHORIA_FX1_KNOB0].LowerBound =  0;
	port_range_hints[EUPHORIA_FX1_KNOB0].UpperBound =  127;
        automatable[EUPHORIA_FX1_KNOB0] = 1;
        
        	
	port_descriptors[EUPHORIA_FX1_KNOB1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[EUPHORIA_FX1_KNOB1] = "FX1 Knob1";
	port_range_hints[EUPHORIA_FX1_KNOB1].DefaultValue = 64.0f;
	port_range_hints[EUPHORIA_FX1_KNOB1].LowerBound =  0;
	port_range_hints[EUPHORIA_FX1_KNOB1].UpperBound =  127;
        automatable[EUPHORIA_FX1_KNOB1] = 1;
        	
	port_descriptors[EUPHORIA_FX1_KNOB2] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[EUPHORIA_FX1_KNOB2] = "FX1 Knob2";
	port_range_hints[EUPHORIA_FX1_KNOB2].DefaultValue = 64.0f;
	port_range_hints[EUPHORIA_FX1_KNOB2].LowerBound =  0;
	port_range_hints[EUPHORIA_FX1_KNOB2].UpperBound =  127;
        automatable[EUPHORIA_FX1_KNOB2] = 1;
        
	port_descriptors[EUPHORIA_FX1_COMBOBOX] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[EUPHORIA_FX1_COMBOBOX] = "FX1 Type";
	port_range_hints[EUPHORIA_FX1_COMBOBOX].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_FX1_COMBOBOX].LowerBound =  0;
	port_range_hints[EUPHORIA_FX1_COMBOBOX].UpperBound =  MULTIFX3KNOB_MAX_INDEX;
        
        port_descriptors[EUPHORIA_FX2_KNOB0] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[EUPHORIA_FX2_KNOB0] = "FX2 Knob0";
	port_range_hints[EUPHORIA_FX2_KNOB0].DefaultValue = 64.0f;
	port_range_hints[EUPHORIA_FX2_KNOB0].LowerBound =  0;
	port_range_hints[EUPHORIA_FX2_KNOB0].UpperBound =  127;
        automatable[EUPHORIA_FX2_KNOB0] = 1;
        	
	port_descriptors[EUPHORIA_FX2_KNOB1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[EUPHORIA_FX2_KNOB1] = "FX2 Knob1";
	port_range_hints[EUPHORIA_FX2_KNOB1].DefaultValue = 64.0f;
	port_range_hints[EUPHORIA_FX2_KNOB1].LowerBound =  0;
	port_range_hints[EUPHORIA_FX2_KNOB1].UpperBound =  127;
        automatable[EUPHORIA_FX2_KNOB1] = 1;
        	
	port_descriptors[EUPHORIA_FX2_KNOB2] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[EUPHORIA_FX2_KNOB2] = "FX2 Knob2";
	port_range_hints[EUPHORIA_FX2_KNOB2].DefaultValue = 64.0f;
	port_range_hints[EUPHORIA_FX2_KNOB2].LowerBound =  0;
	port_range_hints[EUPHORIA_FX2_KNOB2].UpperBound =  127;
        automatable[EUPHORIA_FX2_KNOB2] = 1;
        
	port_descriptors[EUPHORIA_FX2_COMBOBOX] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[EUPHORIA_FX2_COMBOBOX] = "FX2 Type";
	port_range_hints[EUPHORIA_FX2_COMBOBOX].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_FX2_COMBOBOX].LowerBound =  0;
	port_range_hints[EUPHORIA_FX2_COMBOBOX].UpperBound =  MULTIFX3KNOB_MAX_INDEX;        
        	
	port_descriptors[EUPHORIA_FX3_KNOB0] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[EUPHORIA_FX3_KNOB0] = "FX3 Knob0";
	port_range_hints[EUPHORIA_FX3_KNOB0].DefaultValue = 64.0f;
	port_range_hints[EUPHORIA_FX3_KNOB0].LowerBound =  0;
	port_range_hints[EUPHORIA_FX3_KNOB0].UpperBound =  127;
        automatable[EUPHORIA_FX3_KNOB0] = 1;
        	
	port_descriptors[EUPHORIA_FX3_KNOB1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[EUPHORIA_FX3_KNOB1] = "FX3 Knob1";
	port_range_hints[EUPHORIA_FX3_KNOB1].DefaultValue = 64.0f;
	port_range_hints[EUPHORIA_FX3_KNOB1].LowerBound =  0;
	port_range_hints[EUPHORIA_FX3_KNOB1].UpperBound =  127;
        automatable[EUPHORIA_FX3_KNOB1] = 1;
        	
	port_descriptors[EUPHORIA_FX3_KNOB2] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[EUPHORIA_FX3_KNOB2] = "FX3 Knob2";
	port_range_hints[EUPHORIA_FX3_KNOB2].DefaultValue = 64.0f;
	port_range_hints[EUPHORIA_FX3_KNOB2].LowerBound =  0;
	port_range_hints[EUPHORIA_FX3_KNOB2].UpperBound =  127;
        automatable[EUPHORIA_FX3_KNOB2] = 1;
        
	port_descriptors[EUPHORIA_FX3_COMBOBOX] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[EUPHORIA_FX3_COMBOBOX] = "FX3 Type";
	port_range_hints[EUPHORIA_FX3_COMBOBOX].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_FX3_COMBOBOX].LowerBound =  0;
	port_range_hints[EUPHORIA_FX3_COMBOBOX].UpperBound =  MULTIFX3KNOB_MAX_INDEX;
        
        //From PolyFX mod matrix
        
        port_descriptors[EUPHORIA_PFXMATRIX_GRP0DST0SRC0CTRL0] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL; port_names[EUPHORIA_PFXMATRIX_GRP0DST0SRC0CTRL0] = "LMS_PFXMATRIX_GRP0DST0SRC0CTRL0";
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST0SRC0CTRL0].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST0SRC0CTRL0].LowerBound =  -100; port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST0SRC0CTRL0].UpperBound =  100;

	port_descriptors[EUPHORIA_PFXMATRIX_GRP0DST0SRC0CTRL1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL; port_names[EUPHORIA_PFXMATRIX_GRP0DST0SRC0CTRL1] = "LMS_PFXMATRIX_GRP0DST0SRC0CTRL1";
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST0SRC0CTRL1].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST0SRC0CTRL1].LowerBound =  -100; port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST0SRC0CTRL1].UpperBound =  100;

	port_descriptors[EUPHORIA_PFXMATRIX_GRP0DST0SRC0CTRL2] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL; port_names[EUPHORIA_PFXMATRIX_GRP0DST0SRC0CTRL2] = "LMS_PFXMATRIX_GRP0DST0SRC0CTRL2";
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST0SRC0CTRL2].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST0SRC0CTRL2].LowerBound =  -100; port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST0SRC0CTRL2].UpperBound =  100;

	port_descriptors[EUPHORIA_PFXMATRIX_GRP0DST0SRC1CTRL0] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL; port_names[EUPHORIA_PFXMATRIX_GRP0DST0SRC1CTRL0] = "LMS_PFXMATRIX_GRP0DST0SRC1CTRL0";
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST0SRC1CTRL0].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST0SRC1CTRL0].LowerBound =  -100; port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST0SRC1CTRL0].UpperBound =  100;

	port_descriptors[EUPHORIA_PFXMATRIX_GRP0DST0SRC1CTRL1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL; port_names[EUPHORIA_PFXMATRIX_GRP0DST0SRC1CTRL1] = "LMS_PFXMATRIX_GRP0DST0SRC1CTRL1";
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST0SRC1CTRL1].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST0SRC1CTRL1].LowerBound =  -100; port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST0SRC1CTRL1].UpperBound =  100;

	port_descriptors[EUPHORIA_PFXMATRIX_GRP0DST0SRC1CTRL2] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL; port_names[EUPHORIA_PFXMATRIX_GRP0DST0SRC1CTRL2] = "LMS_PFXMATRIX_GRP0DST0SRC1CTRL2";
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST0SRC1CTRL2].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST0SRC1CTRL2].LowerBound =  -100; port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST0SRC1CTRL2].UpperBound =  100;

	port_descriptors[EUPHORIA_PFXMATRIX_GRP0DST0SRC2CTRL0] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL; port_names[EUPHORIA_PFXMATRIX_GRP0DST0SRC2CTRL0] = "LMS_PFXMATRIX_GRP0DST0SRC2CTRL0";
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST0SRC2CTRL0].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST0SRC2CTRL0].LowerBound =  -100; port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST0SRC2CTRL0].UpperBound =  100;

	port_descriptors[EUPHORIA_PFXMATRIX_GRP0DST0SRC2CTRL1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL; port_names[EUPHORIA_PFXMATRIX_GRP0DST0SRC2CTRL1] = "LMS_PFXMATRIX_GRP0DST0SRC2CTRL1";
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST0SRC2CTRL1].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST0SRC2CTRL1].LowerBound =  -100; port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST0SRC2CTRL1].UpperBound =  100;

	port_descriptors[EUPHORIA_PFXMATRIX_GRP0DST0SRC2CTRL2] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL; port_names[EUPHORIA_PFXMATRIX_GRP0DST0SRC2CTRL2] = "LMS_PFXMATRIX_GRP0DST0SRC2CTRL2";
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST0SRC2CTRL2].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST0SRC2CTRL2].LowerBound =  -100; port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST0SRC2CTRL2].UpperBound =  100;

	port_descriptors[EUPHORIA_PFXMATRIX_GRP0DST0SRC3CTRL0] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL; port_names[EUPHORIA_PFXMATRIX_GRP0DST0SRC3CTRL0] = "LMS_PFXMATRIX_GRP0DST0SRC3CTRL0";
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST0SRC3CTRL0].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST0SRC3CTRL0].LowerBound =  -100; port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST0SRC3CTRL0].UpperBound =  100;

	port_descriptors[EUPHORIA_PFXMATRIX_GRP0DST0SRC3CTRL1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL; port_names[EUPHORIA_PFXMATRIX_GRP0DST0SRC3CTRL1] = "LMS_PFXMATRIX_GRP0DST0SRC3CTRL1";
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST0SRC3CTRL1].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST0SRC3CTRL1].LowerBound =  -100; port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST0SRC3CTRL1].UpperBound =  100;

	port_descriptors[EUPHORIA_PFXMATRIX_GRP0DST0SRC3CTRL2] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL; port_names[EUPHORIA_PFXMATRIX_GRP0DST0SRC3CTRL2] = "LMS_PFXMATRIX_GRP0DST0SRC3CTRL2";
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST0SRC3CTRL2].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST0SRC3CTRL2].LowerBound =  -100; port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST0SRC3CTRL2].UpperBound =  100;

	port_descriptors[EUPHORIA_PFXMATRIX_GRP0DST1SRC0CTRL0] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL; port_names[EUPHORIA_PFXMATRIX_GRP0DST1SRC0CTRL0] = "LMS_PFXMATRIX_GRP0DST1SRC0CTRL0";
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST1SRC0CTRL0].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST1SRC0CTRL0].LowerBound =  -100; port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST1SRC0CTRL0].UpperBound =  100;

	port_descriptors[EUPHORIA_PFXMATRIX_GRP0DST1SRC0CTRL1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL; port_names[EUPHORIA_PFXMATRIX_GRP0DST1SRC0CTRL1] = "LMS_PFXMATRIX_GRP0DST1SRC0CTRL1";
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST1SRC0CTRL1].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST1SRC0CTRL1].LowerBound =  -100; port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST1SRC0CTRL1].UpperBound =  100;

	port_descriptors[EUPHORIA_PFXMATRIX_GRP0DST1SRC0CTRL2] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL; port_names[EUPHORIA_PFXMATRIX_GRP0DST1SRC0CTRL2] = "LMS_PFXMATRIX_GRP0DST1SRC0CTRL2";
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST1SRC0CTRL2].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST1SRC0CTRL2].LowerBound =  -100; port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST1SRC0CTRL2].UpperBound =  100;

	port_descriptors[EUPHORIA_PFXMATRIX_GRP0DST1SRC1CTRL0] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL; port_names[EUPHORIA_PFXMATRIX_GRP0DST1SRC1CTRL0] = "LMS_PFXMATRIX_GRP0DST1SRC1CTRL0";
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST1SRC1CTRL0].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST1SRC1CTRL0].LowerBound =  -100; port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST1SRC1CTRL0].UpperBound =  100;

	port_descriptors[EUPHORIA_PFXMATRIX_GRP0DST1SRC1CTRL1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL; port_names[EUPHORIA_PFXMATRIX_GRP0DST1SRC1CTRL1] = "LMS_PFXMATRIX_GRP0DST1SRC1CTRL1";
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST1SRC1CTRL1].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST1SRC1CTRL1].LowerBound =  -100; port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST1SRC1CTRL1].UpperBound =  100;

	port_descriptors[EUPHORIA_PFXMATRIX_GRP0DST1SRC1CTRL2] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL; port_names[EUPHORIA_PFXMATRIX_GRP0DST1SRC1CTRL2] = "LMS_PFXMATRIX_GRP0DST1SRC1CTRL2";
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST1SRC1CTRL2].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST1SRC1CTRL2].LowerBound =  -100; port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST1SRC1CTRL2].UpperBound =  100;

	port_descriptors[EUPHORIA_PFXMATRIX_GRP0DST1SRC2CTRL0] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL; port_names[EUPHORIA_PFXMATRIX_GRP0DST1SRC2CTRL0] = "LMS_PFXMATRIX_GRP0DST1SRC2CTRL0";
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST1SRC2CTRL0].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST1SRC2CTRL0].LowerBound =  -100; port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST1SRC2CTRL0].UpperBound =  100;

	port_descriptors[EUPHORIA_PFXMATRIX_GRP0DST1SRC2CTRL1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL; port_names[EUPHORIA_PFXMATRIX_GRP0DST1SRC2CTRL1] = "LMS_PFXMATRIX_GRP0DST1SRC2CTRL1";
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST1SRC2CTRL1].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST1SRC2CTRL1].LowerBound =  -100; port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST1SRC2CTRL1].UpperBound =  100;

	port_descriptors[EUPHORIA_PFXMATRIX_GRP0DST1SRC2CTRL2] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL; port_names[EUPHORIA_PFXMATRIX_GRP0DST1SRC2CTRL2] = "LMS_PFXMATRIX_GRP0DST1SRC2CTRL2";
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST1SRC2CTRL2].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST1SRC2CTRL2].LowerBound =  -100; port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST1SRC2CTRL2].UpperBound =  100;

	port_descriptors[EUPHORIA_PFXMATRIX_GRP0DST1SRC3CTRL0] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL; port_names[EUPHORIA_PFXMATRIX_GRP0DST1SRC3CTRL0] = "LMS_PFXMATRIX_GRP0DST1SRC3CTRL0";
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST1SRC3CTRL0].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST1SRC3CTRL0].LowerBound =  -100; port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST1SRC3CTRL0].UpperBound =  100;

	port_descriptors[EUPHORIA_PFXMATRIX_GRP0DST1SRC3CTRL1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL; port_names[EUPHORIA_PFXMATRIX_GRP0DST1SRC3CTRL1] = "LMS_PFXMATRIX_GRP0DST1SRC3CTRL1";
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST1SRC3CTRL1].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST1SRC3CTRL1].LowerBound =  -100; port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST1SRC3CTRL1].UpperBound =  100;

	port_descriptors[EUPHORIA_PFXMATRIX_GRP0DST1SRC3CTRL2] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL; port_names[EUPHORIA_PFXMATRIX_GRP0DST1SRC3CTRL2] = "LMS_PFXMATRIX_GRP0DST1SRC3CTRL2";
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST1SRC3CTRL2].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST1SRC3CTRL2].LowerBound =  -100; port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST1SRC3CTRL2].UpperBound =  100;

	port_descriptors[EUPHORIA_PFXMATRIX_GRP0DST2SRC0CTRL0] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL; port_names[EUPHORIA_PFXMATRIX_GRP0DST2SRC0CTRL0] = "LMS_PFXMATRIX_GRP0DST2SRC0CTRL0";
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST2SRC0CTRL0].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST2SRC0CTRL0].LowerBound =  -100; port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST2SRC0CTRL0].UpperBound =  100;

	port_descriptors[EUPHORIA_PFXMATRIX_GRP0DST2SRC0CTRL1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL; port_names[EUPHORIA_PFXMATRIX_GRP0DST2SRC0CTRL1] = "LMS_PFXMATRIX_GRP0DST2SRC0CTRL1";
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST2SRC0CTRL1].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST2SRC0CTRL1].LowerBound =  -100; port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST2SRC0CTRL1].UpperBound =  100;

	port_descriptors[EUPHORIA_PFXMATRIX_GRP0DST2SRC0CTRL2] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL; port_names[EUPHORIA_PFXMATRIX_GRP0DST2SRC0CTRL2] = "LMS_PFXMATRIX_GRP0DST2SRC0CTRL2";
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST2SRC0CTRL2].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST2SRC0CTRL2].LowerBound =  -100; port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST2SRC0CTRL2].UpperBound =  100;

	port_descriptors[EUPHORIA_PFXMATRIX_GRP0DST2SRC1CTRL0] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL; port_names[EUPHORIA_PFXMATRIX_GRP0DST2SRC1CTRL0] = "LMS_PFXMATRIX_GRP0DST2SRC1CTRL0";
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST2SRC1CTRL0].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST2SRC1CTRL0].LowerBound =  -100; port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST2SRC1CTRL0].UpperBound =  100;

	port_descriptors[EUPHORIA_PFXMATRIX_GRP0DST2SRC1CTRL1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL; port_names[EUPHORIA_PFXMATRIX_GRP0DST2SRC1CTRL1] = "LMS_PFXMATRIX_GRP0DST2SRC1CTRL1";
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST2SRC1CTRL1].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST2SRC1CTRL1].LowerBound =  -100; port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST2SRC1CTRL1].UpperBound =  100;

	port_descriptors[EUPHORIA_PFXMATRIX_GRP0DST2SRC1CTRL2] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL; port_names[EUPHORIA_PFXMATRIX_GRP0DST2SRC1CTRL2] = "LMS_PFXMATRIX_GRP0DST2SRC1CTRL2";
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST2SRC1CTRL2].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST2SRC1CTRL2].LowerBound =  -100; port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST2SRC1CTRL2].UpperBound =  100;

	port_descriptors[EUPHORIA_PFXMATRIX_GRP0DST2SRC2CTRL0] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL; port_names[EUPHORIA_PFXMATRIX_GRP0DST2SRC2CTRL0] = "LMS_PFXMATRIX_GRP0DST2SRC2CTRL0";
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST2SRC2CTRL0].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST2SRC2CTRL0].LowerBound =  -100; port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST2SRC2CTRL0].UpperBound =  100;

	port_descriptors[EUPHORIA_PFXMATRIX_GRP0DST2SRC2CTRL1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL; port_names[EUPHORIA_PFXMATRIX_GRP0DST2SRC2CTRL1] = "LMS_PFXMATRIX_GRP0DST2SRC2CTRL1";
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST2SRC2CTRL1].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST2SRC2CTRL1].LowerBound =  -100; port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST2SRC2CTRL1].UpperBound =  100;

	port_descriptors[EUPHORIA_PFXMATRIX_GRP0DST2SRC2CTRL2] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL; port_names[EUPHORIA_PFXMATRIX_GRP0DST2SRC2CTRL2] = "LMS_PFXMATRIX_GRP0DST2SRC2CTRL2";
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST2SRC2CTRL2].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST2SRC2CTRL2].LowerBound =  -100; port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST2SRC2CTRL2].UpperBound =  100;

	port_descriptors[EUPHORIA_PFXMATRIX_GRP0DST2SRC3CTRL0] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL; port_names[EUPHORIA_PFXMATRIX_GRP0DST2SRC3CTRL0] = "LMS_PFXMATRIX_GRP0DST2SRC3CTRL0";
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST2SRC3CTRL0].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST2SRC3CTRL0].LowerBound =  -100; port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST2SRC3CTRL0].UpperBound =  100;

	port_descriptors[EUPHORIA_PFXMATRIX_GRP0DST2SRC3CTRL1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL; port_names[EUPHORIA_PFXMATRIX_GRP0DST2SRC3CTRL1] = "LMS_PFXMATRIX_GRP0DST2SRC3CTRL1";
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST2SRC3CTRL1].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST2SRC3CTRL1].LowerBound =  -100; port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST2SRC3CTRL1].UpperBound =  100;

	port_descriptors[EUPHORIA_PFXMATRIX_GRP0DST2SRC3CTRL2] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL; port_names[EUPHORIA_PFXMATRIX_GRP0DST2SRC3CTRL2] = "LMS_PFXMATRIX_GRP0DST2SRC3CTRL2";
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST2SRC3CTRL2].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST2SRC3CTRL2].LowerBound =  -100; port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST2SRC3CTRL2].UpperBound =  100;

	port_descriptors[EUPHORIA_PFXMATRIX_GRP0DST3SRC0CTRL0] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL; port_names[EUPHORIA_PFXMATRIX_GRP0DST3SRC0CTRL0] = "LMS_PFXMATRIX_GRP0DST3SRC0CTRL0";
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST3SRC0CTRL0].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST3SRC0CTRL0].LowerBound =  -100; port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST3SRC0CTRL0].UpperBound =  100;

	port_descriptors[EUPHORIA_PFXMATRIX_GRP0DST3SRC0CTRL1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL; port_names[EUPHORIA_PFXMATRIX_GRP0DST3SRC0CTRL1] = "LMS_PFXMATRIX_GRP0DST3SRC0CTRL1";
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST3SRC0CTRL1].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST3SRC0CTRL1].LowerBound =  -100; port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST3SRC0CTRL1].UpperBound =  100;

	port_descriptors[EUPHORIA_PFXMATRIX_GRP0DST3SRC0CTRL2] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL; port_names[EUPHORIA_PFXMATRIX_GRP0DST3SRC0CTRL2] = "LMS_PFXMATRIX_GRP0DST3SRC0CTRL2";
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST3SRC0CTRL2].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST3SRC0CTRL2].LowerBound =  -100; port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST3SRC0CTRL2].UpperBound =  100;

	port_descriptors[EUPHORIA_PFXMATRIX_GRP0DST3SRC1CTRL0] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL; port_names[EUPHORIA_PFXMATRIX_GRP0DST3SRC1CTRL0] = "LMS_PFXMATRIX_GRP0DST3SRC1CTRL0";
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST3SRC1CTRL0].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST3SRC1CTRL0].LowerBound =  -100; port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST3SRC1CTRL0].UpperBound =  100;

	port_descriptors[EUPHORIA_PFXMATRIX_GRP0DST3SRC1CTRL1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL; port_names[EUPHORIA_PFXMATRIX_GRP0DST3SRC1CTRL1] = "LMS_PFXMATRIX_GRP0DST3SRC1CTRL1";
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST3SRC1CTRL1].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST3SRC1CTRL1].LowerBound =  -100; port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST3SRC1CTRL1].UpperBound =  100;

	port_descriptors[EUPHORIA_PFXMATRIX_GRP0DST3SRC1CTRL2] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL; port_names[EUPHORIA_PFXMATRIX_GRP0DST3SRC1CTRL2] = "LMS_PFXMATRIX_GRP0DST3SRC1CTRL2";
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST3SRC1CTRL2].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST3SRC1CTRL2].LowerBound =  -100; port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST3SRC1CTRL2].UpperBound =  100;

	port_descriptors[EUPHORIA_PFXMATRIX_GRP0DST3SRC2CTRL0] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL; port_names[EUPHORIA_PFXMATRIX_GRP0DST3SRC2CTRL0] = "LMS_PFXMATRIX_GRP0DST3SRC2CTRL0";
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST3SRC2CTRL0].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST3SRC2CTRL0].LowerBound =  -100; port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST3SRC2CTRL0].UpperBound =  100;

	port_descriptors[EUPHORIA_PFXMATRIX_GRP0DST3SRC2CTRL1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL; port_names[EUPHORIA_PFXMATRIX_GRP0DST3SRC2CTRL1] = "LMS_PFXMATRIX_GRP0DST3SRC2CTRL1";
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST3SRC2CTRL1].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST3SRC2CTRL1].LowerBound =  -100; port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST3SRC2CTRL1].UpperBound =  100;

	port_descriptors[EUPHORIA_PFXMATRIX_GRP0DST3SRC2CTRL2] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL; port_names[EUPHORIA_PFXMATRIX_GRP0DST3SRC2CTRL2] = "LMS_PFXMATRIX_GRP0DST3SRC2CTRL2";
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST3SRC2CTRL2].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST3SRC2CTRL2].LowerBound =  -100; port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST3SRC2CTRL2].UpperBound =  100;

	port_descriptors[EUPHORIA_PFXMATRIX_GRP0DST3SRC3CTRL0] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL; port_names[EUPHORIA_PFXMATRIX_GRP0DST3SRC3CTRL0] = "LMS_PFXMATRIX_GRP0DST3SRC3CTRL0";
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST3SRC3CTRL0].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST3SRC3CTRL0].LowerBound =  -100; port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST3SRC3CTRL0].UpperBound =  100;

	port_descriptors[EUPHORIA_PFXMATRIX_GRP0DST3SRC3CTRL1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL; port_names[EUPHORIA_PFXMATRIX_GRP0DST3SRC3CTRL1] = "LMS_PFXMATRIX_GRP0DST3SRC3CTRL1";
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST3SRC3CTRL1].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST3SRC3CTRL1].LowerBound =  -100; port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST3SRC3CTRL1].UpperBound =  100;

	port_descriptors[EUPHORIA_PFXMATRIX_GRP0DST3SRC3CTRL2] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL; port_names[EUPHORIA_PFXMATRIX_GRP0DST3SRC3CTRL2] = "LMS_PFXMATRIX_GRP0DST3SRC3CTRL2";
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST3SRC3CTRL2].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST3SRC3CTRL2].LowerBound =  -100; port_range_hints[EUPHORIA_PFXMATRIX_GRP0DST3SRC3CTRL2].UpperBound =  100;
        
        port_descriptors[EUPHORIA_NOISE_TYPE] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[EUPHORIA_NOISE_TYPE] = "Noise Type";
	port_range_hints[EUPHORIA_NOISE_TYPE].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_NOISE_TYPE].LowerBound =  0;
	port_range_hints[EUPHORIA_NOISE_TYPE].UpperBound =  2;
        
        port_descriptors[EUPHORIA_LFO_PITCH] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[EUPHORIA_LFO_PITCH] = "LFO Pitch";
	port_range_hints[EUPHORIA_LFO_PITCH].DefaultValue = 0.0f;
	port_range_hints[EUPHORIA_LFO_PITCH].LowerBound = -36.0f;
	port_range_hints[EUPHORIA_LFO_PITCH].UpperBound = 36.0f;
        automatable[EUPHORIA_LFO_PITCH] = 1;
        
        int f_i = EUPHORIA_SAMPLE_PITCH_PORT_RANGE_MIN;
                
        while(f_i < EUPHORIA_SAMPLE_PITCH_PORT_RANGE_MAX)
        {
            port_descriptors[f_i] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
            port_names[f_i] = "Sample Note";
            port_range_hints[f_i].DefaultValue = 60.0f;
            port_range_hints[f_i].LowerBound = 0; port_range_hints[f_i].UpperBound = 120;            
            f_i++;
        }
        
        while(f_i < EUPHORIA_PLAY_PITCH_LOW_PORT_RANGE_MAX)
        {
            port_descriptors[f_i] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
            port_names[f_i] = "Sample Pitch Low";
            port_range_hints[f_i].DefaultValue = 0.0f;
            port_range_hints[f_i].LowerBound = 0; port_range_hints[f_i].UpperBound = 120;            
            f_i++;
        }
        
        while(f_i < EUPHORIA_PLAY_PITCH_HIGH_PORT_RANGE_MAX)
        {
            port_descriptors[f_i] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
            port_names[f_i] = "Sample Pitch High";
            port_range_hints[f_i].DefaultValue = 120.0f;
            port_range_hints[f_i].LowerBound = 0; port_range_hints[f_i].UpperBound = 120;            
            f_i++;
        }
        
        while(f_i < EUPHORIA_SAMPLE_VOLUME_PORT_RANGE_MAX)
        {
            port_descriptors[f_i] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
            port_names[f_i] = "Sample Volume";
            port_range_hints[f_i].DefaultValue = 0.0f;
            port_range_hints[f_i].LowerBound = -50; port_range_hints[f_i].UpperBound = 36;            
            f_i++;
        }

        while(f_i < EUPHORIA_SAMPLE_START_PORT_RANGE_MAX)
        {
            port_descriptors[f_i] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
            port_names[f_i] = "Sample Start";
            port_range_hints[f_i].DefaultValue = 0.0f;
            port_range_hints[f_i].LowerBound = 0; port_range_hints[f_i].UpperBound = 10000;            
            f_i++;
        }
        
        while(f_i < EUPHORIA_SAMPLE_END_PORT_RANGE_MAX)
        {
            port_descriptors[f_i] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
            port_names[f_i] = "Sample End";
            port_range_hints[f_i].DefaultValue = 0.0f;
            port_range_hints[f_i].LowerBound = 0; port_range_hints[f_i].UpperBound = 10000;            
            f_i++;
        }
        
        while(f_i < EUPHORIA_SAMPLE_VEL_SENS_PORT_RANGE_MAX)
        {
            port_descriptors[f_i] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
            port_names[f_i] = "Velocity Sensitivity";
            port_range_hints[f_i].DefaultValue = 10.0f;
            port_range_hints[f_i].LowerBound = 0; port_range_hints[f_i].UpperBound = 20;            
            f_i++;
        }
        
        while(f_i < EUPHORIA_SAMPLE_VEL_LOW_PORT_RANGE_MAX)
        {
            port_descriptors[f_i] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
            port_names[f_i] = "Low Velocity";
            port_range_hints[f_i].DefaultValue = 0.0f;
            port_range_hints[f_i].LowerBound = 0; port_range_hints[f_i].UpperBound = 127;            
            f_i++;
        }
        
        while(f_i < EUPHORIA_SAMPLE_VEL_HIGH_PORT_RANGE_MAX)
        {
            port_descriptors[f_i] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
            port_names[f_i] = "High Velocity";
            port_range_hints[f_i].DefaultValue = 127.0f;
            port_range_hints[f_i].LowerBound = 0; port_range_hints[f_i].UpperBound = 127;            
            f_i++;
        }
        
        while(f_i < EUPHORIA_PITCH_PORT_RANGE_MAX)
        {
            port_descriptors[f_i] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
            port_names[f_i] = "Sample Pitch";
            port_range_hints[f_i].DefaultValue = 0.0f;
            port_range_hints[f_i].LowerBound = -36; port_range_hints[f_i].UpperBound = 36;            
            f_i++;
        }
        
        while(f_i < EUPHORIA_TUNE_PORT_RANGE_MAX)
        {
            port_descriptors[f_i] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
            port_names[f_i] = "Sample Tune";
            port_range_hints[f_i].DefaultValue = 0.0f;
            port_range_hints[f_i].LowerBound = -100; port_range_hints[f_i].UpperBound = 100;            
            f_i++;
        }
        
        while(f_i < EUPHORIA_SAMPLE_INTERPOLATION_MODE_PORT_RANGE_MAX)
        {
            port_descriptors[f_i] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
            port_names[f_i] = "Mode";
            port_range_hints[f_i].DefaultValue = 1.0f;
            port_range_hints[f_i].LowerBound = 0; port_range_hints[f_i].UpperBound = 3;            
            f_i++;
        }
        
        while(f_i < EUPHORIA_SAMPLE_LOOP_START_PORT_RANGE_MAX)
        {
            port_descriptors[f_i] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
            port_names[f_i] = "Loop Start";
            port_range_hints[f_i].DefaultValue = 0.0f;
            port_range_hints[f_i].LowerBound = 0; port_range_hints[f_i].UpperBound = 10000;            
            f_i++;
        }
        
        while(f_i < EUPHORIA_SAMPLE_LOOP_END_PORT_RANGE_MAX)
        {
            port_descriptors[f_i] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
            port_names[f_i] = "Loop End";
            port_range_hints[f_i].DefaultValue = 0.0f;
            port_range_hints[f_i].LowerBound = 0; port_range_hints[f_i].UpperBound = 10000;            
            f_i++;
        }
        
        while(f_i < EUPHORIA_SAMPLE_LOOP_MODE_PORT_RANGE_MAX)
        {
            port_descriptors[f_i] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
            port_names[f_i] = "Loop Modes";
            port_range_hints[f_i].DefaultValue = 0.0f;
            port_range_hints[f_i].LowerBound = 0; port_range_hints[f_i].UpperBound = 1;
            
            f_i++;
        }
        int f_group_num = 1;
        //MonoFX0
        while(f_i < EUPHORIA_MONO_FX0_KNOB0_PORT_RANGE_MAX)
        {
            port_descriptors[f_i] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
            port_names[f_i] =  (char*)malloc(sizeof(char) * 32); 
            sprintf(port_names[f_i], "Mono FX0 Knob0 Group %i", f_group_num);
            f_group_num++;
            automatable[f_i] = 1;
            port_range_hints[f_i].DefaultValue = 64.0f;
            port_range_hints[f_i].LowerBound = 0; port_range_hints[f_i].UpperBound = 127;            
            f_i++;
        }
        f_group_num = 1;
        while(f_i < EUPHORIA_MONO_FX0_KNOB1_PORT_RANGE_MAX)
        {
            port_descriptors[f_i] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
            port_names[f_i] = (char*)malloc(sizeof(char) * 32); 
            sprintf(port_names[f_i], "Mono FX0 Knob1 Group %i", f_group_num);
            f_group_num++;
            automatable[f_i] = 1;
            port_range_hints[f_i].DefaultValue = 64.0f;
            port_range_hints[f_i].LowerBound = 0; port_range_hints[f_i].UpperBound = 127;            
            f_i++;
        }
        f_group_num = 1;
        while(f_i < EUPHORIA_MONO_FX0_KNOB2_PORT_RANGE_MAX)
        {
            port_descriptors[f_i] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
            port_names[f_i] = (char*)malloc(sizeof(char) * 32); 
            sprintf(port_names[f_i], "Mono FX0 Knob2 Group %i", f_group_num);
            f_group_num++;
            automatable[f_i] = 1;
            port_range_hints[f_i].DefaultValue = 64.0f;
            port_range_hints[f_i].LowerBound = 0; port_range_hints[f_i].UpperBound = 127;            
            f_i++;
        }
        f_group_num = 1;
        while(f_i < EUPHORIA_MONO_FX0_COMBOBOX_PORT_RANGE_MAX)
        {
            port_descriptors[f_i] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
            port_names[f_i] = (char*)malloc(sizeof(char) * 32); 
            sprintf(port_names[f_i], "Mono FX0 Combobox Group %i", f_group_num);
            f_group_num++;
            port_range_hints[f_i].DefaultValue = 0.0f;
            port_range_hints[f_i].LowerBound = 0; port_range_hints[f_i].UpperBound = MULTIFX3KNOB_MAX_INDEX;            
            f_i++;
        }
        f_group_num = 1;
        while(f_i < EUPHORIA_MONO_FX1_KNOB0_PORT_RANGE_MAX)
        {
            port_descriptors[f_i] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
            port_names[f_i] = (char*)malloc(sizeof(char) * 32); 
            sprintf(port_names[f_i], "Mono FX1 Knob0 Group %i", f_group_num);
            f_group_num++;
            automatable[f_i] = 1;
            port_range_hints[f_i].DefaultValue = 64.0f;
            port_range_hints[f_i].LowerBound = 0; port_range_hints[f_i].UpperBound = 127;            
            f_i++;
        }
        f_group_num = 1;
        while(f_i < EUPHORIA_MONO_FX1_KNOB1_PORT_RANGE_MAX)
        {
            port_descriptors[f_i] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
            port_names[f_i] = (char*)malloc(sizeof(char) * 32); 
            sprintf(port_names[f_i], "Mono FX1 Knob1 Group %i", f_group_num);
            f_group_num++;
            automatable[f_i] = 1;
            port_range_hints[f_i].DefaultValue = 64.0f;
            port_range_hints[f_i].LowerBound = 0; port_range_hints[f_i].UpperBound = 127;            
            f_i++;
        }
        f_group_num = 1;
        while(f_i < EUPHORIA_MONO_FX1_KNOB2_PORT_RANGE_MAX)
        {
            port_descriptors[f_i] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
            port_names[f_i] = (char*)malloc(sizeof(char) * 32); 
            sprintf(port_names[f_i], "Mono FX1 Knob2 Group %i", f_group_num);
            f_group_num++;
            automatable[f_i] = 1;
            port_range_hints[f_i].DefaultValue = 64.0f;
            port_range_hints[f_i].LowerBound = 0; port_range_hints[f_i].UpperBound = 127;            
            f_i++;
        }
        f_group_num = 1;
        while(f_i < EUPHORIA_MONO_FX1_COMBOBOX_PORT_RANGE_MAX)
        {
            port_descriptors[f_i] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
            port_names[f_i] = (char*)malloc(sizeof(char) * 32); 
            sprintf(port_names[f_i], "Mono FX1 Combobox Group %i", f_group_num);
            f_group_num++;            
            port_range_hints[f_i].DefaultValue = 0.0f;
            port_range_hints[f_i].LowerBound = 0; port_range_hints[f_i].UpperBound = MULTIFX3KNOB_MAX_INDEX;            
            f_i++;
        }
        f_group_num = 1;
        while(f_i < EUPHORIA_MONO_FX2_KNOB0_PORT_RANGE_MAX)
        {
            port_descriptors[f_i] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
            port_names[f_i] = (char*)malloc(sizeof(char) * 32); 
            sprintf(port_names[f_i], "Mono FX2 Knob0 Group %i", f_group_num);
            f_group_num++;
            automatable[f_i] = 1;
            port_range_hints[f_i].DefaultValue = 64.0f;
            port_range_hints[f_i].LowerBound = 0; port_range_hints[f_i].UpperBound = 127;            
            f_i++;
        }
        f_group_num = 1;
        while(f_i < EUPHORIA_MONO_FX2_KNOB1_PORT_RANGE_MAX)
        {
            port_descriptors[f_i] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
            port_names[f_i] = (char*)malloc(sizeof(char) * 32); 
            sprintf(port_names[f_i], "Mono FX2 Knob1 Group %i", f_group_num);
            f_group_num++;
            automatable[f_i] = 1;
            port_range_hints[f_i].DefaultValue = 64.0f;
            port_range_hints[f_i].LowerBound = 0; port_range_hints[f_i].UpperBound = 127;            
            f_i++;
        }
        f_group_num = 1;
        while(f_i < EUPHORIA_MONO_FX2_KNOB2_PORT_RANGE_MAX)
        {
            port_descriptors[f_i] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
            port_names[f_i] = (char*)malloc(sizeof(char) * 32); 
            sprintf(port_names[f_i], "Mono FX2 Knob2 Group %i", f_group_num);
            f_group_num++;
            automatable[f_i] = 1;
            port_range_hints[f_i].DefaultValue = 64.0f;
            port_range_hints[f_i].LowerBound = 0; port_range_hints[f_i].UpperBound = 127;            
            f_i++;
        }
        f_group_num = 1;
        while(f_i < EUPHORIA_MONO_FX2_COMBOBOX_PORT_RANGE_MAX)
        {
            port_descriptors[f_i] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
            port_names[f_i] = (char*)malloc(sizeof(char) * 32); 
            sprintf(port_names[f_i], "Mono FX2 Combobox Group %i", f_group_num);
            f_group_num++;
            port_range_hints[f_i].DefaultValue = 0.0f;
            port_range_hints[f_i].LowerBound = 0; port_range_hints[f_i].UpperBound = MULTIFX3KNOB_MAX_INDEX;            
            f_i++;
        }
        f_group_num = 1;
        while(f_i < EUPHORIA_MONO_FX3_KNOB0_PORT_RANGE_MAX)
        {
            port_descriptors[f_i] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
            port_names[f_i] = (char*)malloc(sizeof(char) * 32); 
            sprintf(port_names[f_i], "Mono FX3 Knob0 Group %i", f_group_num);
            f_group_num++;
            automatable[f_i] = 1;
            port_range_hints[f_i].DefaultValue = 64.0f;
            port_range_hints[f_i].LowerBound = 0; port_range_hints[f_i].UpperBound = 127;            
            f_i++;
        }
        f_group_num = 1;
        while(f_i < EUPHORIA_MONO_FX3_KNOB1_PORT_RANGE_MAX)
        {
            port_descriptors[f_i] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
            port_names[f_i] = (char*)malloc(sizeof(char) * 32); 
            sprintf(port_names[f_i], "Mono FX3 Knob1 Group %i", f_group_num);
            f_group_num++;
            automatable[f_i] = 1;
            port_range_hints[f_i].DefaultValue = 64.0f;
            port_range_hints[f_i].LowerBound = 0; port_range_hints[f_i].UpperBound = 127;            
            f_i++;
        }
        f_group_num = 1;
        while(f_i < EUPHORIA_MONO_FX3_KNOB2_PORT_RANGE_MAX)
        {
            port_descriptors[f_i] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
            port_names[f_i] = (char*)malloc(sizeof(char) * 32); 
            sprintf(port_names[f_i], "Mono FX3 Knob2 Group %i", f_group_num);
            f_group_num++;
            automatable[f_i] = 1;
            port_range_hints[f_i].DefaultValue = 64.0f;
            port_range_hints[f_i].LowerBound = 0; port_range_hints[f_i].UpperBound = 127;            
            f_i++;
        }
        f_group_num = 1;
        while(f_i < EUPHORIA_MONO_FX3_COMBOBOX_PORT_RANGE_MAX)
        {
            port_descriptors[f_i] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
            port_names[f_i] = (char*)malloc(sizeof(char) * 32); 
            sprintf(port_names[f_i], "Mono FX3 Combobox Group %i", f_group_num);
            f_group_num++;            
            port_range_hints[f_i].DefaultValue = 0.0f;
            port_range_hints[f_i].LowerBound = 0; port_range_hints[f_i].UpperBound = MULTIFX3KNOB_MAX_INDEX;            
            f_i++;
        }
        
        while(f_i < EUPHORIA_SAMPLE_MONO_FX_GROUP_PORT_RANGE_MAX)
        {
            port_descriptors[f_i] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
            port_names[f_i] = "Sample MonoFX Group";
            port_range_hints[f_i].DefaultValue = 0.0f;
            port_range_hints[f_i].LowerBound = 0; port_range_hints[f_i].UpperBound = (EUPHORIA_MAX_SAMPLE_COUNT - 1);
            f_i++;
        }
        
	desc->activate = v_euphoria_activate;
	desc->cleanup = cleanupSampler;
	desc->connect_port = connectPortSampler;
	desc->deactivate = v_euphoria_activate;
	desc->instantiate = instantiateSampler;
	desc->run = NULL;
    }
    
    return euphoriaLDescriptor;
}

const PYINST_Descriptor *euphoria_PYINST_descriptor(int index)
{
    PYINST_Descriptor *euphoriaDDescriptor = NULL;
    
    euphoriaDDescriptor = (PYINST_Descriptor *) malloc(sizeof(PYINST_Descriptor));
    
    euphoriaDDescriptor->PYINST_API_Version = 1;
    euphoriaDDescriptor->PYFX_Plugin = euphoria_PYFX_descriptor(0);
    euphoriaDDescriptor->configure = c_euphoria_configure;
    euphoriaDDescriptor->run_synth = v_run_lms_euphoria;
    
    return euphoriaDDescriptor; 
}


/*
void v_euphoria_destructor()
{
    if (euphoriaLDescriptor) {
	free((PYFX_PortDescriptor *) euphoriaLDescriptor->PortDescriptors);
	free((char **) euphoriaLDescriptor->PortNames);
	free((PYFX_PortRangeHint *) euphoriaLDescriptor->PortRangeHints);
	free(euphoriaLDescriptor);
    }
    if (euphoriaDDescriptor) {
	free(euphoriaDDescriptor);
    }
}
*/