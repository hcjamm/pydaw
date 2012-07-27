/* -*- c-basic-offset: 4 -*-  vi:set ts=8 sts=4 sw=4: */

/* synth.c
   
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

#include "dssi.h"
#include "ladspa.h"

#include <sndfile.h>
#include <pthread.h>

#include "synth.h"
#include "meta.h"
#include "../../libmodsynth/lib/lms_math.h"

static LADSPA_Descriptor *samplerStereoLDescriptor = NULL;

static DSSI_Descriptor *samplerStereoDDescriptor = NULL;

static void run_lms_euphoria(LADSPA_Handle instance, unsigned long sample_count,
		       snd_seq_event_t *events, unsigned long EventCount);

__attribute__ ((visibility("default")))
const LADSPA_Descriptor *ladspa_descriptor(unsigned long index)
{
    switch (index) {
    case 0:
	return samplerStereoLDescriptor;
    default:
	return NULL;
    }
}

__attribute__ ((visibility("default")))
const DSSI_Descriptor *dssi_descriptor(unsigned long index)
{
    switch (index) {
    case 0:
	return samplerStereoDDescriptor;    
    default:
	return NULL;
    }
}

static void cleanupSampler(LADSPA_Handle instance)
{
    Sampler *plugin = (Sampler *)instance;
    free(plugin);
}

static void connectPortSampler(LADSPA_Handle instance, unsigned long port,
			       LADSPA_Data * data)
{
    Sampler *plugin;
    plugin = (Sampler *) instance;
    
    if(port < LMS_SAMPLE_PITCH_PORT_RANGE_MIN)
    {
        switch (port) {
        case Sampler_OUTPUT_LEFT:
            plugin->output[0] = data;
            break;
        case Sampler_OUTPUT_RIGHT:
            plugin->output[1] = data;
            break;
        case Sampler_SELECTED_SAMPLE:
            plugin->selected_sample = data;
            break;

        case LMS_ATTACK:
            plugin->attack = data;
            break;
        case LMS_DECAY:
            plugin->decay = data;
            break;
        case LMS_SUSTAIN:
            plugin->sustain = data;
            break;
        case LMS_RELEASE:
            plugin->release = data;
            break;
        case LMS_FILTER_ATTACK:
            plugin->attack_f = data;
            break;
        case LMS_FILTER_DECAY:
            plugin->decay_f = data;
            break;
        case LMS_FILTER_SUSTAIN:
            plugin->sustain_f = data;
            break;
        case LMS_FILTER_RELEASE:
            plugin->release_f = data;
            break;
        case LMS_NOISE_AMP:
            plugin->noise_amp = data;
            break;
        case LMS_MASTER_VOLUME:
            plugin->master_vol = data;
            break;
        case LMS_MASTER_GLIDE:
            plugin->master_glide = data;
            break;
        case LMS_MASTER_PITCHBEND_AMT:
            plugin->master_pb_amt = data;
            break;
        case LMS_PITCH_ENV_TIME:
            plugin->pitch_env_time = data;
            break;
        case LMS_LFO_FREQ:
            plugin->lfo_freq = data;
            break;
        case LMS_LFO_TYPE:
            plugin->lfo_type = data;
            break;
        //End Ray-V ports
        //From Modulex
            
        case LMS_FX0_KNOB0: plugin->pfx_mod_knob[0][0][0] = data; break;
        case LMS_FX0_KNOB1: plugin->pfx_mod_knob[0][0][1] = data; break;
        case LMS_FX0_KNOB2: plugin->pfx_mod_knob[0][0][2] = data; break;
        case LMS_FX1_KNOB0: plugin->pfx_mod_knob[0][1][0] = data; break;
        case LMS_FX1_KNOB1: plugin->pfx_mod_knob[0][1][1] = data; break;
        case LMS_FX1_KNOB2: plugin->pfx_mod_knob[0][1][2] = data; break;
        case LMS_FX2_KNOB0: plugin->pfx_mod_knob[0][2][0] = data; break;
        case LMS_FX2_KNOB1: plugin->pfx_mod_knob[0][2][1] = data; break;
        case LMS_FX2_KNOB2: plugin->pfx_mod_knob[0][2][2] = data; break;
        case LMS_FX3_KNOB0: plugin->pfx_mod_knob[0][3][0] = data; break;
        case LMS_FX3_KNOB1: plugin->pfx_mod_knob[0][3][1] = data; break;
        case LMS_FX3_KNOB2: plugin->pfx_mod_knob[0][3][2] = data; break;
            
        case LMS_FX0_COMBOBOX: plugin->fx_combobox[0][0] = data; break;    
        case LMS_FX1_COMBOBOX: plugin->fx_combobox[0][1] = data; break;    
        case LMS_FX2_COMBOBOX: plugin->fx_combobox[0][2] = data; break;    
        case LMS_FX3_COMBOBOX: plugin->fx_combobox[0][3] = data; break;    
        //End from Modulex
        /*PolyFX mod matrix port connections*/
        case LMS_PFXMATRIX_GRP0DST0SRC0CTRL0: plugin->polyfx_mod_matrix[0][0][0][0] = data; break;
        case LMS_PFXMATRIX_GRP0DST0SRC0CTRL1: plugin->polyfx_mod_matrix[0][0][0][1] = data; break;
        case LMS_PFXMATRIX_GRP0DST0SRC0CTRL2: plugin->polyfx_mod_matrix[0][0][0][2] = data; break;
        case LMS_PFXMATRIX_GRP0DST0SRC1CTRL0: plugin->polyfx_mod_matrix[0][0][1][0] = data; break;
        case LMS_PFXMATRIX_GRP0DST0SRC1CTRL1: plugin->polyfx_mod_matrix[0][0][1][1] = data; break;
        case LMS_PFXMATRIX_GRP0DST0SRC1CTRL2: plugin->polyfx_mod_matrix[0][0][1][2] = data; break;
        case LMS_PFXMATRIX_GRP0DST0SRC2CTRL0: plugin->polyfx_mod_matrix[0][0][2][0] = data; break;
        case LMS_PFXMATRIX_GRP0DST0SRC2CTRL1: plugin->polyfx_mod_matrix[0][0][2][1] = data; break;
        case LMS_PFXMATRIX_GRP0DST0SRC2CTRL2: plugin->polyfx_mod_matrix[0][0][2][2] = data; break;
        case LMS_PFXMATRIX_GRP0DST0SRC3CTRL0: plugin->polyfx_mod_matrix[0][0][3][0] = data; break;
        case LMS_PFXMATRIX_GRP0DST0SRC3CTRL1: plugin->polyfx_mod_matrix[0][0][3][1] = data; break;
        case LMS_PFXMATRIX_GRP0DST0SRC3CTRL2: plugin->polyfx_mod_matrix[0][0][3][2] = data; break;
        case LMS_PFXMATRIX_GRP0DST1SRC0CTRL0: plugin->polyfx_mod_matrix[0][1][0][0] = data; break;
        case LMS_PFXMATRIX_GRP0DST1SRC0CTRL1: plugin->polyfx_mod_matrix[0][1][0][1] = data; break;
        case LMS_PFXMATRIX_GRP0DST1SRC0CTRL2: plugin->polyfx_mod_matrix[0][1][0][2] = data; break;
        case LMS_PFXMATRIX_GRP0DST1SRC1CTRL0: plugin->polyfx_mod_matrix[0][1][1][0] = data; break;
        case LMS_PFXMATRIX_GRP0DST1SRC1CTRL1: plugin->polyfx_mod_matrix[0][1][1][1] = data; break;
        case LMS_PFXMATRIX_GRP0DST1SRC1CTRL2: plugin->polyfx_mod_matrix[0][1][1][2] = data; break;
        case LMS_PFXMATRIX_GRP0DST1SRC2CTRL0: plugin->polyfx_mod_matrix[0][1][2][0] = data; break;
        case LMS_PFXMATRIX_GRP0DST1SRC2CTRL1: plugin->polyfx_mod_matrix[0][1][2][1] = data; break;
        case LMS_PFXMATRIX_GRP0DST1SRC2CTRL2: plugin->polyfx_mod_matrix[0][1][2][2] = data; break;
        case LMS_PFXMATRIX_GRP0DST1SRC3CTRL0: plugin->polyfx_mod_matrix[0][1][3][0] = data; break;
        case LMS_PFXMATRIX_GRP0DST1SRC3CTRL1: plugin->polyfx_mod_matrix[0][1][3][1] = data; break;
        case LMS_PFXMATRIX_GRP0DST1SRC3CTRL2: plugin->polyfx_mod_matrix[0][1][3][2] = data; break;
        case LMS_PFXMATRIX_GRP0DST2SRC0CTRL0: plugin->polyfx_mod_matrix[0][2][0][0] = data; break;
        case LMS_PFXMATRIX_GRP0DST2SRC0CTRL1: plugin->polyfx_mod_matrix[0][2][0][1] = data; break;
        case LMS_PFXMATRIX_GRP0DST2SRC0CTRL2: plugin->polyfx_mod_matrix[0][2][0][2] = data; break;
        case LMS_PFXMATRIX_GRP0DST2SRC1CTRL0: plugin->polyfx_mod_matrix[0][2][1][0] = data; break;
        case LMS_PFXMATRIX_GRP0DST2SRC1CTRL1: plugin->polyfx_mod_matrix[0][2][1][1] = data; break;
        case LMS_PFXMATRIX_GRP0DST2SRC1CTRL2: plugin->polyfx_mod_matrix[0][2][1][2] = data; break;
        case LMS_PFXMATRIX_GRP0DST2SRC2CTRL0: plugin->polyfx_mod_matrix[0][2][2][0] = data; break;
        case LMS_PFXMATRIX_GRP0DST2SRC2CTRL1: plugin->polyfx_mod_matrix[0][2][2][1] = data; break;
        case LMS_PFXMATRIX_GRP0DST2SRC2CTRL2: plugin->polyfx_mod_matrix[0][2][2][2] = data; break;
        case LMS_PFXMATRIX_GRP0DST2SRC3CTRL0: plugin->polyfx_mod_matrix[0][2][3][0] = data; break;
        case LMS_PFXMATRIX_GRP0DST2SRC3CTRL1: plugin->polyfx_mod_matrix[0][2][3][1] = data; break;
        case LMS_PFXMATRIX_GRP0DST2SRC3CTRL2: plugin->polyfx_mod_matrix[0][2][3][2] = data; break;
        case LMS_PFXMATRIX_GRP0DST3SRC0CTRL0: plugin->polyfx_mod_matrix[0][3][0][0] = data; break;
        case LMS_PFXMATRIX_GRP0DST3SRC0CTRL1: plugin->polyfx_mod_matrix[0][3][0][1] = data; break;
        case LMS_PFXMATRIX_GRP0DST3SRC0CTRL2: plugin->polyfx_mod_matrix[0][3][0][2] = data; break;
        case LMS_PFXMATRIX_GRP0DST3SRC1CTRL0: plugin->polyfx_mod_matrix[0][3][1][0] = data; break;
        case LMS_PFXMATRIX_GRP0DST3SRC1CTRL1: plugin->polyfx_mod_matrix[0][3][1][1] = data; break;
        case LMS_PFXMATRIX_GRP0DST3SRC1CTRL2: plugin->polyfx_mod_matrix[0][3][1][2] = data; break;
        case LMS_PFXMATRIX_GRP0DST3SRC2CTRL0: plugin->polyfx_mod_matrix[0][3][2][0] = data; break;
        case LMS_PFXMATRIX_GRP0DST3SRC2CTRL1: plugin->polyfx_mod_matrix[0][3][2][1] = data; break;
        case LMS_PFXMATRIX_GRP0DST3SRC2CTRL2: plugin->polyfx_mod_matrix[0][3][2][2] = data; break;
        case LMS_PFXMATRIX_GRP0DST3SRC3CTRL0: plugin->polyfx_mod_matrix[0][3][3][0] = data; break;
        case LMS_PFXMATRIX_GRP0DST3SRC3CTRL1: plugin->polyfx_mod_matrix[0][3][3][1] = data; break;
        case LMS_PFXMATRIX_GRP0DST3SRC3CTRL2: plugin->polyfx_mod_matrix[0][3][3][2] = data; break;

        //End PolyFX mod matrix
        
        //case LMS_GLOBAL_MIDI_CHANNEL: plugin->global_midi_channel = data; break;
        case LMS_GLOBAL_MIDI_OCTAVES_OFFSET: plugin->global_midi_octaves_offset = data; break;
        case LMS_NOISE_TYPE: plugin->noise_type = data; break;
        default:
            break;
        }
    }
    else if((port >= LMS_SAMPLE_PITCH_PORT_RANGE_MIN) && (port < LMS_SAMPLE_PITCH_PORT_RANGE_MAX))
    {
        plugin->basePitch[(port - LMS_SAMPLE_PITCH_PORT_RANGE_MIN)] = data;
    }
    else if((port >= LMS_PLAY_PITCH_LOW_PORT_RANGE_MIN) && (port < LMS_PLAY_PITCH_LOW_PORT_RANGE_MAX))
    {
        plugin->low_note[(port - LMS_PLAY_PITCH_LOW_PORT_RANGE_MIN)] = data;
    }    
    else if((port >= LMS_PLAY_PITCH_HIGH_PORT_RANGE_MIN) && (port < LMS_PLAY_PITCH_HIGH_PORT_RANGE_MAX))
    {
        plugin->high_note[(port - LMS_PLAY_PITCH_HIGH_PORT_RANGE_MIN)] = data;
    }
    else if((port >= LMS_SAMPLE_VOLUME_PORT_RANGE_MIN) && (port < LMS_SAMPLE_VOLUME_PORT_RANGE_MAX))
    {
        plugin->sample_vol[(port - LMS_SAMPLE_VOLUME_PORT_RANGE_MIN)] = data;
    }
    else if((port >= LMS_SAMPLE_START_PORT_RANGE_MIN) && (port < LMS_SAMPLE_START_PORT_RANGE_MAX))
    {
        plugin->sampleStarts[(port - LMS_SAMPLE_START_PORT_RANGE_MIN)] = data;
    }
    else if((port >= LMS_SAMPLE_END_PORT_RANGE_MIN) && (port < LMS_SAMPLE_END_PORT_RANGE_MAX))
    {
        plugin->sampleEnds[(port - LMS_SAMPLE_END_PORT_RANGE_MIN)] = data;
    }
    else if((port >= LMS_SAMPLE_VEL_SENS_PORT_RANGE_MIN) && (port < LMS_SAMPLE_VEL_SENS_PORT_RANGE_MAX))
    {
        plugin->sample_vel_sens[(port - LMS_SAMPLE_VEL_SENS_PORT_RANGE_MIN)] = data;
    }    
    else if((port >= LMS_SAMPLE_VEL_LOW_PORT_RANGE_MIN) && (port < LMS_SAMPLE_VEL_LOW_PORT_RANGE_MAX))
    {
        plugin->sample_vel_low[(port - LMS_SAMPLE_VEL_LOW_PORT_RANGE_MIN)] = data;
    }
    else if((port >= LMS_SAMPLE_VEL_HIGH_PORT_RANGE_MIN) && (port < LMS_SAMPLE_VEL_HIGH_PORT_RANGE_MAX))
    {
        plugin->sample_vel_high[(port - LMS_SAMPLE_VEL_HIGH_PORT_RANGE_MIN)] = data;
    }
    //new
    else if((port >= LMS_PITCH_PORT_RANGE_MIN) && (port < LMS_PITCH_PORT_RANGE_MAX))
    {
        plugin->sample_pitch[(port - LMS_PITCH_PORT_RANGE_MIN)] = data;
    }    
    else if((port >= LMS_TUNE_PORT_RANGE_MIN) && (port < LMS_TUNE_PORT_RANGE_MAX))
    {
        plugin->sample_tune[(port - LMS_TUNE_PORT_RANGE_MIN)] = data;
    }
    else if((port >= LMS_SAMPLE_INTERPOLATION_MODE_PORT_RANGE_MIN) && (port < LMS_SAMPLE_INTERPOLATION_MODE_PORT_RANGE_MAX))
    {
        plugin->sample_interpolation_mode[(port - LMS_SAMPLE_INTERPOLATION_MODE_PORT_RANGE_MIN)] = data;
    }
    
    else if((port >= LMS_SAMPLE_LOOP_START_PORT_RANGE_MIN) && (port < LMS_SAMPLE_LOOP_START_PORT_RANGE_MAX))
    {
        plugin->sampleLoopStarts[(port - LMS_SAMPLE_LOOP_START_PORT_RANGE_MIN)] = data;
    }
    else if((port >= LMS_SAMPLE_LOOP_END_PORT_RANGE_MIN) && (port < LMS_SAMPLE_LOOP_END_PORT_RANGE_MAX))
    {
        plugin->sampleLoopEnds[(port - LMS_SAMPLE_LOOP_END_PORT_RANGE_MIN)] = data;
    }
    else if((port >= LMS_SAMPLE_LOOP_MODE_PORT_RANGE_MIN) && (port < LMS_SAMPLE_LOOP_MODE_PORT_RANGE_MAX))
    {
        plugin->sampleLoopModes[(port - LMS_SAMPLE_LOOP_MODE_PORT_RANGE_MIN)] = data;
    }
}

static LADSPA_Handle instantiateSampler(const LADSPA_Descriptor * descriptor,
					unsigned long s_rate)
{
    Sampler *plugin_data = (Sampler *) malloc(sizeof(Sampler));
    pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

    plugin_data->i_selected_sample = 0;
    plugin_data->current_sample = 0;
    plugin_data->loaded_samples_count = 0;
    plugin_data->lin_interpolator = g_lin_get();
    plugin_data->amp = 1.0f;
    
    plugin_data->preview_sample_array_index = 0;
    plugin_data->sampleCount[LMS_MAX_SAMPLE_COUNT] = 0;  //To prevent a SEGFAULT on the first call of the main loop
    plugin_data->sample_paths[LMS_MAX_SAMPLE_COUNT] = "";
    plugin_data->preview_sample_max_length = s_rate * 5;  //Sets the maximum time to preview a sample to 5 seconds, lest a user unwittlingly tries to preview a 2 hour long sample.
    
    plugin_data->smp_pit_core = g_pit_get();
    plugin_data->smp_pit_ratio = g_pit_ratio();
        
    int f_i = 0;
    while(f_i < LMS_MAX_SAMPLE_COUNT)
    {
        plugin_data->sampleCount[f_i] = 0;
        plugin_data->basePitch[f_i] = 0;
        plugin_data->low_note[f_i] = 0;
        plugin_data->high_note[f_i] = 0;
        plugin_data->sample_vol[f_i] = 0;
        plugin_data->sample_amp[f_i] = 1.0f;
        
        plugin_data->sample_paths[f_i] = "";
        
        plugin_data->adjusted_base_pitch[f_i] = 60.0f;
        plugin_data->sample_rate_ratios[f_i] = 1.0f;
        
        f_i++;
    }
    
    f_i = 0;
    int f_i2;
    
    while(f_i < Sampler_NOTES)
    {
        plugin_data->sampleStarts[f_i] = 0;
        plugin_data->sampleEnds[f_i] = 0;
        plugin_data->sample_indexes_count[f_i] = 0;
        plugin_data->data[f_i] = g_poly_init(s_rate);
        
        f_i2 = 0;
        while(f_i2 < LMS_MAX_SAMPLE_COUNT)
        {
            plugin_data->sample_read_heads[f_i][f_i2] = g_ifh_get();
            plugin_data->sample_indexes[f_i][f_i2] = 0;
            plugin_data->vel_sens_output[f_i][f_i2] = 0.0f;
            f_i2++;
        }
        
        f_i++;
    }
    
    for(f_i = 0; f_i < LMS_MONO_FX_COUNT; f_i++)
    {
        for(f_i2 = 0; f_i2 < 4096; f_i2++)
        {
            plugin_data->mono_fx_buffers[f_i][0][f_i2] = 0.0f;
            plugin_data->mono_fx_buffers[f_i][1][f_i2] = 0.0f;
        }
    }
    
    
    
    plugin_data->sampleRate = s_rate;
    plugin_data->projectDir = 0;
    
    plugin_data->sv_pitch_bend_value = 0.0f;
    plugin_data->sv_last_note = 36.0f;

    plugin_data->channels = 2;
    plugin_data->amp_ptr = g_amp_get();
    
    memcpy(&plugin_data->mutex, &m, sizeof(pthread_mutex_t));
    
    plugin_data->mono_modules = g_mono_init(s_rate);
    
        plugin_data->fs = s_rate;
    
    plugin_data->midi_cc_map = g_ccm_get();
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_ATTACK, 73, "Attack Amp");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_DECAY, 75, "Decay Amp");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_SUSTAIN, 79, "Sustain Amp");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_RELEASE, 72, "Release Amp");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_FILTER_ATTACK, 21, "Attack Filter");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_FILTER_DECAY, 22, "Decay Filter");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_FILTER_SUSTAIN, 23, "Sustain Filter");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_FILTER_RELEASE, 24, "Release Filter");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_NOISE_AMP, 25, "Noise Amp");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_MASTER_VOLUME, 36, "Master Volume");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_MASTER_GLIDE, 39, "Glide Time");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_MASTER_PITCHBEND_AMT, 40, "Pitchbend Amount");    
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_PITCH_ENV_TIME, 43, "Pitch Env Time");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_LFO_FREQ, 44, "LFO Freq");    
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_LFO_TYPE, 45, "LFO Type");
    
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_FX0_KNOB0, 49, "FX0Knob0");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_FX0_KNOB1, 50, "FX0Knob1");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_FX0_KNOB2, 51, "FX0Knob2");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_FX0_COMBOBOX, 52, "FX0Combobox");
    
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_FX1_KNOB0, 53, "FX1Knob0");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_FX1_KNOB1, 54, "FX1Knob1");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_FX1_KNOB2, 55, "FX1Knob2");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_FX1_COMBOBOX, 56, "FX1Combobox");
    
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_FX2_KNOB0, 57, "FX2Knob0");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_FX2_KNOB1, 58, "FX2Knob1");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_FX2_KNOB2, 59, "FX2Knob2");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_FX2_COMBOBOX, 60, "FX2Combobox");
    
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_FX3_KNOB0, 61, "FX3Knob0");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_FX3_KNOB1, 62, "FX3Knob1");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_FX3_KNOB2, 63, "FX3Knob2");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_FX3_COMBOBOX, 64, "FX3Combobox");
    
    v_ccm_read_file_to_array(plugin_data->midi_cc_map, "euphoria-cc_map.txt");
        
    return (LADSPA_Handle) plugin_data;
}

static void activateSampler(LADSPA_Handle instance)
{
    Sampler *plugin_data = (Sampler *) instance;
    unsigned int i;

    pthread_mutex_lock(&plugin_data->mutex);

    plugin_data->sampleNo = 0;

    for (i = 0; i < Sampler_NOTES; i++) {
	plugin_data->ons[i] = -1;
	plugin_data->offs[i] = -1;
	plugin_data->velocities[i] = 0;
    }

    pthread_mutex_unlock(&plugin_data->mutex);
}


//For the per-sample interpolation modes
typedef int (*fp_calculate_ratio)(Sampler *__restrict plugin_data, int n);
typedef void (*fp_run_sampler_interpolation)(Sampler *__restrict plugin_data, int n, int ch);

static fp_calculate_ratio ratio_function_ptrs[LMS_MAX_SAMPLE_COUNT];
static fp_run_sampler_interpolation interpolation_modes[LMS_MAX_SAMPLE_COUNT];

static inline int check_sample_bounds(Sampler *__restrict plugin_data, int n)
{
    
    if ((plugin_data->sample_read_heads[n][(plugin_data->current_sample)]->whole_number) >=  plugin_data->sampleEndPos[(plugin_data->current_sample)])
    {
        if(((int)(*(plugin_data->sampleLoopModes[(plugin_data->current_sample)]))) > 0)
        {
            //TODO:  write a special function that either maintains the fraction, or
            //else wraps the negative interpolation back to where it was before the loop happened, to avoid clicks and pops
            v_ifh_retrigger(plugin_data->sample_read_heads[n][(plugin_data->current_sample)], 
                    (LMS_SINC_INTERPOLATION_POINTS_DIV2 + (plugin_data->sampleLoopStartPos[(plugin_data->current_sample)])));// 0.0f;
   
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

static int calculate_ratio_sinc(Sampler *__restrict plugin_data, int n)
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

static int calculate_ratio_linear(Sampler *__restrict plugin_data, int n)
{
    return calculate_ratio_sinc(plugin_data, n);
}

static int calculate_ratio_none(Sampler *__restrict plugin_data, int n)
{
    plugin_data->sample_read_heads[n][(plugin_data->current_sample)]->whole_number = (plugin_data->sample_read_heads[n][(plugin_data->current_sample)]->whole_number) + 1;
    
    return check_sample_bounds(plugin_data, n);
}

static void run_sampler_interpolation_sinc(Sampler *__restrict plugin_data, int n, int ch)
{    
    plugin_data->sample_last_interpolated_value[(plugin_data->current_sample)] = f_sinc_interpolate2(plugin_data->mono_modules->sinc_interpolator, 
            plugin_data->sampleData[ch][(plugin_data->current_sample)],
            (plugin_data->sample_read_heads[n][(plugin_data->current_sample)]->whole_number),
            (plugin_data->sample_read_heads[n][(plugin_data->current_sample)]->fraction))  
            * (plugin_data->sample_amp[(plugin_data->current_sample)]);
}


static void run_sampler_interpolation_linear(Sampler *__restrict plugin_data, int n, int ch)
{
    plugin_data->sample_last_interpolated_value[(plugin_data->current_sample)] = f_linear_interpolate_ptr_ifh(
            plugin_data->sampleData[ch][(plugin_data->current_sample)],
            (plugin_data->sample_read_heads[n][(plugin_data->current_sample)]->whole_number),
            (plugin_data->sample_read_heads[n][(plugin_data->current_sample)]->fraction),
            plugin_data->lin_interpolator)  
            * (plugin_data->sample_amp[(plugin_data->current_sample)]);    
}


static void run_sampler_interpolation_none(Sampler *__restrict plugin_data, int n, int ch)
{
    plugin_data->sample_last_interpolated_value[(plugin_data->current_sample)] = 
            plugin_data->sampleData[ch][(plugin_data->current_sample)][(plugin_data->sample_read_heads[n][(plugin_data->current_sample)]->whole_number)];
}

/* static void addSample(Sampler *plugin_data, 
 * int n, //The note number
 * unsigned long pos, //the position in the output buffer
 * unsigned long count) //how many samples to fill in the output buffer
 */
static void add_sample_lms_euphoria(Sampler *__restrict plugin_data, int n, unsigned long pos, unsigned long count)
{
    plugin_data->ratio = 1.0f;
    
    unsigned long i, ch;

    for (i = 0; i < count; ++i) {

        //Delay the note-on event until the sample it was called for
        if(((plugin_data->sampleNo) + i) < (plugin_data->ons[n]))
            continue;

        //Run things that aren't per-channel like envelopes
                
        v_adsr_run(plugin_data->data[n]->adsr_amp);        
                            
        if(plugin_data->data[n]->adsr_amp->stage == 4)
        {
            plugin_data->ons[n] = -1;
            break;
        }

        v_adsr_run(plugin_data->data[n]->adsr_filter);
        
        //Run the glide module            
        f_rmp_run_ramp(plugin_data->data[n]->ramp_env);
        f_rmp_run_ramp(plugin_data->data[n]->glide_env);
        
        //Set and run the LFO
        v_lfs_set(plugin_data->data[n]->lfo1,  (*(plugin_data->lfo_freq)) * .01  );
        v_lfs_run(plugin_data->data[n]->lfo1);
        
        plugin_data->data[n]->base_pitch = (plugin_data->data[n]->glide_env->output_multiplied)
                +  (plugin_data->mono_modules->pitchbend_smoother->output)  //(plugin_data->sv_pitch_bend_value)
                + (plugin_data->data[n]->last_pitch);
        
        
                 
	if (plugin_data->offs[n] >= 0 &&
	    pos + i + plugin_data->sampleNo > plugin_data->offs[n]) 
        {            
            v_poly_note_off(plugin_data->data[n]);
	}        
        
        float sample[2];
        sample[0] = 0.0f;
        sample[1] = 0.0f;
                    
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
            
            for (ch = 0; ch < (plugin_data->channels); ++ch) 
            {
                if((ch == 2) && ((plugin_data->sample_channels[(plugin_data->current_sample)]) == 1))
                {
                    sample[ch] += (plugin_data->sample_last_interpolated_value[(plugin_data->current_sample)]);
                    continue;
                }

                plugin_data->sample_amp[(plugin_data->current_sample)] = f_db_to_linear(
                        (*(plugin_data->sample_vol[(plugin_data->current_sample)])) + 
                        (plugin_data->vel_sens_output[n][(plugin_data->current_sample)])                        
                        , plugin_data->amp_ptr);

                interpolation_modes[(plugin_data->current_sample)](plugin_data, n, ch);

                sample[ch] += plugin_data->sample_last_interpolated_value[(plugin_data->current_sample)];
                
                sample[ch] += ((plugin_data->data[n]->noise_func_ptr(plugin_data->data[n]->white_noise1[ch])) * (plugin_data->data[n]->noise_linamp)); //add noise

                sample[ch] = (sample[ch]) * (plugin_data->data[n]->adsr_amp->output) * (plugin_data->amp); // * (plugin_data->data[n]->lfo_amp_output);

                plugin_data->data[n]->modulex_current_sample[ch] = sample[ch];
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

            //plugin_data->output[0][(plugin_data->pos_plus_i)] += plugin_data->data[n]->modulex_current_sample[0];
            //plugin_data->output[1][(plugin_data->pos_plus_i)] += plugin_data->data[n]->modulex_current_sample[1];
            plugin_data->mono_fx_buffers[0][0][i] += plugin_data->data[n]->modulex_current_sample[0];
            plugin_data->mono_fx_buffers[0][1][i] += plugin_data->data[n]->modulex_current_sample[1];
            
            
            plugin_data->i_loaded_samples = (plugin_data->i_loaded_samples) + 1;
            
        }
           
    }
}

static void run_lms_euphoria(LADSPA_Handle instance, unsigned long sample_count,
		       snd_seq_event_t *events, unsigned long event_count)
{
    Sampler *plugin_data = (Sampler *) instance;
    unsigned long pos;
    unsigned long count;
    unsigned long event_pos;
    int i;

    for (i = 0; i < plugin_data->channels; ++i) {
	memset(plugin_data->output[i], 0, sample_count * sizeof(float));
    }
    
    if (pthread_mutex_trylock(&plugin_data->mutex)) {
	return;
    }

    if (!plugin_data->sampleData || !plugin_data->sampleCount) {
	plugin_data->sampleNo += sample_count;
	pthread_mutex_unlock(&plugin_data->mutex);
	return;
    }
    
    int f_note = 60;
    int f_note_adjusted = 60;

    for (pos = 0, event_pos = 0; pos < sample_count; ) {
        
	while (event_pos < event_count
	       && pos >= events[event_pos].time.tick) {
            /*Note-on event*/
	    if (events[event_pos].type == SND_SEQ_EVENT_NOTEON) {
		snd_seq_ev_note_t n = events[event_pos].data.note;
                
                //Code for accepting MIDI only on a user selected MIDI channel.  Not currently implemented
                //because jack-dssi-host does not accept MIDI on all channels in it's current state
                /*
                if(*(plugin_data->global_midi_channel) < 16)
                {
                    if((int)(*(plugin_data->global_midi_channel)) != (int)(n.channel))
                    {
                        ++event_pos;
                        printf("Skipping event on channel %i\n", (int)(n.channel));
                        continue;
                    }                    
                }
                */
                
                f_note = n.note;
                f_note_adjusted = n.note + (*(plugin_data->global_midi_octaves_offset) * -12);
                
                if(f_note < 0)
                {
                    f_note = 0;
                }
                if(f_note > Sampler_NOTES_m1)
                {
                    f_note = Sampler_NOTES_m1;
                }
                
                if(f_note < 0)
                {
                    f_note = 0;
                }
                if(f_note > Sampler_NOTES_m1)
                {
                    f_note = Sampler_NOTES_m1;
                }
                
		if (n.velocity > 0) {
		    plugin_data->ons[f_note] =
			plugin_data->sampleNo + events[event_pos].time.tick;
		    plugin_data->offs[f_note] = -1;
		    plugin_data->velocities[f_note] = n.velocity;

                    plugin_data->sample_indexes_count[f_note] = 0;
                    
                    //Figure out which samples to play and stash all relevant values
                    for(i = 0; i  < (plugin_data->loaded_samples_count); i++)
                    {
                        if((f_note >= *(plugin_data->low_note[(plugin_data->loaded_samples[i])])) && 
                        (f_note <= *(plugin_data->high_note[(plugin_data->loaded_samples[i])])) &&
                        (plugin_data->velocities[f_note] <= *(plugin_data->sample_vel_high[(plugin_data->loaded_samples[i])])) &&
                        (plugin_data->velocities[f_note] >= *(plugin_data->sample_vel_low[(plugin_data->loaded_samples[i])])))
                        {
                            plugin_data->sample_indexes[f_note][(plugin_data->sample_indexes_count[f_note])] = (plugin_data->loaded_samples[i]);
                            plugin_data->sample_indexes_count[f_note] = (plugin_data->sample_indexes_count[f_note]) + 1;                            
                            
                            plugin_data->sampleStartPos[(plugin_data->loaded_samples[i])] = (LMS_SINC_INTERPOLATION_POINTS_DIV2 + ((plugin_data->sampleCount[(plugin_data->loaded_samples[i])]) * ((*(plugin_data->sampleStarts[(plugin_data->loaded_samples[i])])) * .0001)));
                            plugin_data->sampleLoopStartPos[(plugin_data->loaded_samples[i])] = (LMS_SINC_INTERPOLATION_POINTS_DIV2 + ((plugin_data->sampleCount[(plugin_data->loaded_samples[i])]) * ((*(plugin_data->sampleLoopStarts[(plugin_data->loaded_samples[i])])) * .0001)));
                            
                            /* If loop mode is enabled for this sample, set the sample end to be the same as the
                               loop end.  Then, in the main loop, we'll recalculate sample_end to be the real sample end once
                               the note_off event is fired.  Doing it this way greatly reduces the need for extra if-then-else logic
                               in the main loop */
                            if(((int)(*(plugin_data->sampleLoopModes[(plugin_data->loaded_samples[i])]))) == 0)
                            {
                                plugin_data->sampleEndPos[(plugin_data->loaded_samples[i])] = (LMS_SINC_INTERPOLATION_POINTS_DIV2 + ((plugin_data->sampleCount[(plugin_data->loaded_samples[i])]) - ((int)(((float)((plugin_data->sampleCount[(plugin_data->loaded_samples[i])]) - 5)) * ((*(plugin_data->sampleEnds[(plugin_data->loaded_samples[i])])) * .0001)))));
                            }
                            else
                            {
                                plugin_data->sampleEndPos[(plugin_data->loaded_samples[i])] = (LMS_SINC_INTERPOLATION_POINTS_DIV2 + ((plugin_data->sampleCount[(plugin_data->loaded_samples[i])]) - ((int)(((float)((plugin_data->sampleCount[(plugin_data->loaded_samples[i])]) - 5)) * ((*(plugin_data->sampleLoopEnds[(plugin_data->loaded_samples[i])])) * .0001)))));
                            }
                            
                            if((plugin_data->sampleEndPos[(plugin_data->loaded_samples[i])]) > ((float)((plugin_data->sampleCount[(plugin_data->loaded_samples[i])]))))
                            {
                                plugin_data->sampleEndPos[(plugin_data->loaded_samples[i])] = (float)(plugin_data->sampleCount[(plugin_data->loaded_samples[i])]);
                            }
                            
                            plugin_data->adjusted_base_pitch[(plugin_data->loaded_samples[i])] = (*(plugin_data->basePitch[(plugin_data->loaded_samples[i])])) - ((*(plugin_data->global_midi_octaves_offset) + 2) * 12)
                                    - (*(plugin_data->sample_pitch[(plugin_data->loaded_samples[i])])) - ((*(plugin_data->sample_tune[(plugin_data->loaded_samples[i])])) * .01f);
                            
                            v_ifh_retrigger(plugin_data->sample_read_heads[f_note][(plugin_data->loaded_samples[i])], 
                                    (plugin_data->sampleStartPos[(plugin_data->current_sample)]));// 0.0f;
                            
                            plugin_data->vel_sens_output[f_note][(plugin_data->loaded_samples[i])] = 
                                    ((1 -
                                    (((float)(n.velocity) - (*(plugin_data->sample_vel_low[(plugin_data->loaded_samples[i])])))
                                    /
                                    ((float)(*(plugin_data->sample_vel_high[(plugin_data->loaded_samples[i])]) - (*(plugin_data->sample_vel_low[(plugin_data->loaded_samples[i])]))))))
                                    * (*(plugin_data->sample_vel_sens[(plugin_data->loaded_samples[i])])) * -1.0f);
                            
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
                            }
                        }
                    }
                        
                    plugin_data->active_polyfx_count[f_note] = 0;
                    //Determine which PolyFX have been enabled
                    for(plugin_data->i_dst = 0; (plugin_data->i_dst) < LMS_MODULAR_POLYFX_COUNT; plugin_data->i_dst = (plugin_data->i_dst) + 1)
                    {
                        int f_pfx_combobox_index = (int)(*(plugin_data->fx_combobox[0][(plugin_data->i_dst)]));
                        plugin_data->data[f_note]->fx_func_ptr[(plugin_data->i_dst)] = g_mf3_get_function_pointer(f_pfx_combobox_index); 
                        
                        if(f_pfx_combobox_index != 0)
                        {
                            plugin_data->active_polyfx[f_note][(plugin_data->active_polyfx_count[f_note])] = (plugin_data->i_dst);
                            plugin_data->active_polyfx_count[f_note] = (plugin_data->active_polyfx_count[f_note]) + 1;
                        }
                    }    
                    
                    //Calculate an index of which mod_matrix controls to process.  This saves expensive iterations and if/then logic in the main loop
                    for(plugin_data->i_fx_grps = 0; (plugin_data->i_fx_grps) < LMS_EFFECTS_GROUPS_COUNT; plugin_data->i_fx_grps = (plugin_data->i_fx_grps) + 1)
                    {
                        for(plugin_data->i_dst = 0; (plugin_data->i_dst) < (plugin_data->active_polyfx_count[f_note]); plugin_data->i_dst = (plugin_data->i_dst) + 1)
                        {
                            plugin_data->polyfx_mod_counts[f_note][(plugin_data->active_polyfx[f_note][(plugin_data->i_dst)])] = 0;
                            
                            for(plugin_data->i_src = 0; (plugin_data->i_src) < LMS_MODULATOR_COUNT; plugin_data->i_src = (plugin_data->i_src) + 1)
                            {
                                for(plugin_data->i_ctrl = 0; (plugin_data->i_ctrl) < LMS_CONTROLS_PER_MOD_EFFECT; plugin_data->i_ctrl = (plugin_data->i_ctrl) + 1)
                                {
                                    if((*(plugin_data->polyfx_mod_matrix[(plugin_data->i_fx_grps)][(plugin_data->active_polyfx[f_note][(plugin_data->i_dst)])][(plugin_data->i_src)][(plugin_data->i_ctrl)])) != 0)
                                    {                                        
                                        plugin_data->polyfx_mod_ctrl_indexes[f_note][(plugin_data->active_polyfx[f_note][(plugin_data->i_dst)])][(plugin_data->polyfx_mod_counts[f_note][(plugin_data->active_polyfx[f_note][(plugin_data->i_dst)])])] = (plugin_data->i_ctrl);
                                        plugin_data->polyfx_mod_src_index[f_note][(plugin_data->active_polyfx[f_note][(plugin_data->i_dst)])][(plugin_data->polyfx_mod_counts[f_note][(plugin_data->active_polyfx[f_note][(plugin_data->i_dst)])])] = (plugin_data->i_src);
                                        plugin_data->polyfx_mod_matrix_values[f_note][(plugin_data->active_polyfx[f_note][(plugin_data->i_dst)])][(plugin_data->polyfx_mod_counts[f_note][(plugin_data->active_polyfx[f_note][(plugin_data->i_dst)])])] = 
                                                (*(plugin_data->polyfx_mod_matrix[(plugin_data->i_fx_grps)][(plugin_data->active_polyfx[f_note][(plugin_data->i_dst)])][(plugin_data->i_src)][(plugin_data->i_ctrl)])) * .01;
                                        
                                        plugin_data->polyfx_mod_counts[f_note][(plugin_data->active_polyfx[f_note][(plugin_data->i_dst)])] = (plugin_data->polyfx_mod_counts[f_note][(plugin_data->active_polyfx[f_note][(plugin_data->i_dst)])]) + 1;
                                    }
                                }
                            }
                        }
                    }
                    //Get the noise function pointer
                    plugin_data->data[f_note]->noise_func_ptr = fp_get_noise_func_ptr((int)(*(plugin_data->noise_type)));
                    
                    //Begin Ray-V additions
                    
                    //const int voice = i_pick_voice(plugin_data->voices, f_note);
                    
		    plugin_data->amp = f_db_to_linear_fast(*(plugin_data->master_vol), plugin_data->mono_modules->amp_ptr);                     
                    
                    plugin_data->data[f_note]->note_f = (float)f_note;
                                        
                    plugin_data->data[f_note]->target_pitch = (plugin_data->data[f_note]->note_f);
                    plugin_data->data[f_note]->last_pitch = (plugin_data->sv_last_note);
                    
                    v_rmp_retrigger_glide_t(plugin_data->data[f_note]->glide_env , (*(plugin_data->master_glide) * .01), 
                            (plugin_data->sv_last_note), (plugin_data->data[f_note]->target_pitch));
                                                    
                    plugin_data->data[f_note]->noise_linamp = f_db_to_linear_fast(*(plugin_data->noise_amp), plugin_data->mono_modules->amp_ptr);
                                        
                    /*Here is where we perform any actions that should ONLY happen at note_on, you can save a lot of CPU by
                     placing things here that don't need to be modulated as a note is playing*/
                    
                    /*Retrigger ADSR envelopes and LFO*/
                    v_adsr_retrigger(plugin_data->data[f_note]->adsr_amp);
                    v_adsr_retrigger(plugin_data->data[f_note]->adsr_filter);
                    v_lfs_sync(plugin_data->data[f_note]->lfo1, 0.0f, *(plugin_data->lfo_type));
                    
                    v_adsr_set_adsr_db(plugin_data->data[f_note]->adsr_amp, (*(plugin_data->attack) * .01), (*(plugin_data->decay) * .01), (*(plugin_data->sustain)), (*(plugin_data->release) * .01));
                    v_adsr_set_adsr(plugin_data->data[f_note]->adsr_filter, (*(plugin_data->attack_f) * .01), (*(plugin_data->decay_f) * .01), (*(plugin_data->sustain_f) * .01), (*(plugin_data->release_f) * .01));
                    
                    /*Retrigger the pitch envelope*/
                    v_rmp_retrigger((plugin_data->data[f_note]->ramp_env), (*(plugin_data->pitch_env_time) * .01), 1);  
                    
                    plugin_data->data[f_note]->noise_amp = f_db_to_linear(*(plugin_data->noise_amp), plugin_data->mono_modules->amp_ptr);
                                        
                    /*Set the last_note property, so the next note can glide from it if glide is turned on*/
                    plugin_data->sv_last_note = (plugin_data->data[f_note]->note_f);
                    
                    //TODO:  Create a define for the number of channels
                    //Move all of the multi-channel functions here
                    //for(i = 0; i < 2; i++)
                    //{
                        
                    //}
                    
		} else {    
                    plugin_data->offs[f_note] = 
                        plugin_data->sampleNo + events[event_pos].time.tick;		    
		}
	    } /*Note-off event*/
            else if (events[event_pos].type == SND_SEQ_EVENT_NOTEOFF )
            {
		snd_seq_ev_note_t n = events[event_pos].data.note;
                f_note = n.note; // + (*(plugin_data->global_midi_octaves_offset) * -12);
                
                
                if(f_note < 0)
                {
                    f_note = 0;
                }
                if(f_note > Sampler_NOTES_m1)
                {
                    f_note = Sampler_NOTES_m1;
                }
                
		plugin_data->offs[f_note] = 
		    plugin_data->sampleNo + events[event_pos].time.tick;
	    }
            
            /*Pitch-bend sequencer event, modify the voices pitch*/
            else if (events[event_pos].type == SND_SEQ_EVENT_PITCHBEND) 
            {
		plugin_data->sv_pitch_bend_value = 0.00012207
                        * events[event_pos].data.control.value * (*(plugin_data->master_pb_amt));
	    }

	    ++event_pos;
	}

	count = sample_count - pos;
	if (event_pos < event_count &&
	    events[event_pos].time.tick < sample_count) {
	    count = events[event_pos].time.tick - pos;
	}
        
        v_smr_iir_run_fast(plugin_data->mono_modules->pitchbend_smoother, (plugin_data->sv_pitch_bend_value));
        
        for(i = 0; i < count; i++)
        {
            plugin_data->mono_fx_buffers[0][0][i] = 0.0f;
            plugin_data->mono_fx_buffers[0][1][i] = 0.0f;
        }
        
	for (i = 0; i < Sampler_NOTES; ++i) {
	    if(((plugin_data->data[i]->adsr_amp->stage) < 4) && ((plugin_data->sample_indexes_count[i]) > 0))
            {    
                add_sample_lms_euphoria(plugin_data, i, pos, count);                                
	    }
	}
        
        for(i = 0; i < count; i++)
        {
            plugin_data->pos_plus_i = pos + i;

            //Process MonoFX (eventually will be happening here)

            plugin_data->output[0][(plugin_data->pos_plus_i)] += plugin_data->mono_fx_buffers[0][0][i];
            plugin_data->output[1][(plugin_data->pos_plus_i)] += plugin_data->mono_fx_buffers[0][1][i];
        }
        
        if(((plugin_data->preview_sample_array_index) < (plugin_data->sampleCount[LMS_MAX_SAMPLE_COUNT])) &&
                ((plugin_data->preview_sample_array_index) <  (plugin_data->preview_sample_max_length)))
        {
            for(i = 0; i < count; i++)
            {
                plugin_data->pos_plus_i = pos + i;
                
                //Process MonoFX (eventually will be happening here)
                
                plugin_data->output[0][(plugin_data->pos_plus_i)] += plugin_data->mono_fx_buffers[0][0][i];
                plugin_data->output[1][(plugin_data->pos_plus_i)] += plugin_data->mono_fx_buffers[0][1][i];
                
                //Add the previewing sample
                plugin_data->output[0][(plugin_data->pos_plus_i)] += (plugin_data->sampleData[0][(LMS_MAX_SAMPLE_COUNT)][(plugin_data->preview_sample_array_index)]);
                plugin_data->output[1][(plugin_data->pos_plus_i)] += (plugin_data->sampleData[1][(LMS_MAX_SAMPLE_COUNT)][(plugin_data->preview_sample_array_index)]);
                
                plugin_data->preview_sample_array_index = (plugin_data->preview_sample_array_index) + 1;
                
                if((plugin_data->preview_sample_array_index) >= (plugin_data->sampleCount[LMS_MAX_SAMPLE_COUNT]))
                {
                    plugin_data->sample_paths[LMS_MAX_SAMPLE_COUNT] = "";
                    break;
                }
            }
        }
	pos += count;
    }

    plugin_data->sampleNo += sample_count;
    pthread_mutex_unlock(&plugin_data->mutex);
}

static void runSamplerWrapper(LADSPA_Handle instance,
			 unsigned long sample_count)
{
    run_lms_euphoria(instance, sample_count, NULL, 0);
}

int getControllerSampler(LADSPA_Handle instance, unsigned long port)
{
    Sampler *plugin_data = (Sampler *) instance;
    return DSSI_CC(i_ccm_get_cc(plugin_data->midi_cc_map, port));    
}

char * dssi_configure_message(const char *fmt, ...)
{
    va_list args;
    char buffer[256];

    va_start(args, fmt);
    vsnprintf(buffer, 256, fmt, args);
    va_end(args);
    return strdup(buffer);
}

char *samplerLoad(Sampler *plugin_data, const char *path, int a_index)
{   
    /*Add that index to the list of loaded samples to iterate though when playing, if not already added*/
    
    if(a_index != LMS_MAX_SAMPLE_COUNT)
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
    char *revisedPath = 0;
    
    info.format = 0;
    file = sf_open(path, SFM_READ, &info);

    if (!file) {

	const char *filename = strrchr(path, '/');
	if (filename) ++filename;
	else filename = path;

	if (*filename && plugin_data->projectDir) {
	    revisedPath = (char *)malloc(strlen(filename) +
					 strlen(plugin_data->projectDir) + 2);
	    sprintf(revisedPath, "%s/%s", plugin_data->projectDir, filename);
	    file = sf_open(revisedPath, SFM_READ, &info);
	    if (!file) {
		free(revisedPath);
	    }
	}

	if (!file) {
	    return dssi_configure_message
		("error: unable to load sample file '%s'", path);
	}
    }
    
    if (info.frames > Sampler_FRAMES_MAX) {
	return dssi_configure_message
	    ("error: sample file '%s' is too large (%ld frames, maximum is %ld)",
	     path, info.frames, Sampler_FRAMES_MAX);
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

    int f_actual_array_size = (samples + LMS_SINC_INTERPOLATION_POINTS + 1 + Sampler_Sample_Padding);
    
    tmpSamples[0] = (float *)malloc((f_actual_array_size) * sizeof(float));
    tmpSamples[1] = (float *)malloc((f_actual_array_size) * sizeof(float));
    
    int f_i, j;
    
    for(f_i = 0; f_i < LMS_CHANNEL_COUNT; f_i++)
    {
        v_dco_reset(plugin_data->mono_modules->dc_offset_filters[f_i]);
    }    
    
    //For performing a 5ms fadeout of the sample, for preventing clicks
    float f_fade_out_dec = (1.0f/(float)(info.samplerate))/(0.005);
    int f_fade_out_start = (samples + LMS_SINC_INTERPOLATION_POINTS_DIV2) - ((int)(0.005f * ((float)(info.samplerate))));
    float f_fade_out_envelope = 1.0f;
    float f_temp_sample = 0.0f;
        
    for(f_i = 0; f_i < f_actual_array_size; f_i++)
    {   
        if((f_i > LMS_SINC_INTERPOLATION_POINTS_DIV2) && (f_i < (samples + LMS_SINC_INTERPOLATION_POINTS_DIV2))) // + Sampler_Sample_Padding)))
        {
            if(f_i >= f_fade_out_start)
            {
                if(f_fade_out_envelope <= 0.0f)
                {
                    f_fade_out_dec = 0.0f;
                }
                
                f_fade_out_envelope -= f_fade_out_dec;
            }
            
	    for (j = 0; j < 2; ++j) {
		//if (j == 1 && info.channels < 2) {
                if (j == 1 && info.channels <= 2) {
		    tmpSamples[j][f_i] = tmpSamples[0][f_i];
		} else {
		    f_temp_sample = //(tmpFrames[(f_i - LMS_SINC_INTERPOLATION_POINTS_DIV2) * info.channels + j]);
                            f_dco_run(plugin_data->mono_modules->dc_offset_filters[j], 
                            (tmpFrames[(f_i - LMS_SINC_INTERPOLATION_POINTS_DIV2) * info.channels + j]));
                    
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
        }
        else
        {
            tmpSamples[0][f_i] = 0.0f;
            tmpSamples[1][f_i] = 0.0f;
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
    plugin_data->sampleData[1][(a_index)] = tmpSamples[1];
    plugin_data->sampleCount[(a_index)] = (samples + LMS_SINC_INTERPOLATION_POINTS_DIV2 + Sampler_Sample_Padding - 20);  //-20 to ensure we don't read past the end of the array
    
    if((info.channels) >= 2)
    {
        plugin_data->sample_channels[(a_index)] = 2;
    }
    else
    {
        plugin_data->sample_channels[(a_index)] = 1;
    }
    plugin_data->sample_paths[(a_index)] = path;
    
    
    //The last index is reserved for previewing samples for the UI;
    //Reset the array indexer so it will play from the beginning.
    if(a_index == LMS_MAX_SAMPLE_COUNT)
    {
        plugin_data->preview_sample_array_index = 0;
    }
    
    pthread_mutex_unlock(&plugin_data->mutex);

    /*
    if (tmpOld[0]) 
    {
        free(tmpOld[0]);
    }
    
    if (tmpOld[1]) 
    {
        free(tmpOld[1]);
    }
    */
    
    //printf("%s: loaded %s (%ld samples from original %ld channels resampled from %ld frames at %ld Hz)\n", Sampler_Stereo_LABEL, path, (long)samples, (long)info.channels, (long)info.frames, (long)info.samplerate);

    if (revisedPath) {
	char *message = dssi_configure_message("warning: sample file '%s' not found: loading from '%s' instead", path, revisedPath);
	free(revisedPath);
	return message;
    }

    return NULL;
}

char *samplerClear(Sampler *plugin_data, int a_index)
{
    plugin_data->sample_paths[a_index] = "";
    
    if(a_index != LMS_MAX_SAMPLE_COUNT)
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
    float *tmpSamples[2], *tmpOld[2];    

    tmpSamples[0] = (float*)malloc(sizeof(float));        
    tmpSamples[1] = (float *)malloc(sizeof(float));
    
    pthread_mutex_lock(&plugin_data->mutex);

    tmpOld[0] = plugin_data->sampleData[0][(a_index)];
    tmpOld[1] = plugin_data->sampleData[1][(a_index)];
    plugin_data->sampleData[0][(a_index)] = tmpSamples[0];
    plugin_data->sampleData[1][(a_index)] = tmpSamples[1];
    plugin_data->sampleCount[(a_index)] = 0;

    pthread_mutex_unlock(&plugin_data->mutex);

    /*
    if (tmpOld[0]) free(tmpOld[0]);
    if (tmpOld[1]) free(tmpOld[1]);
    */
    
    return NULL;
}

/* Call samplerLoad for all samples.*/
char *samplerLoadAll(Sampler *plugin_data, const char *paths)
{       
    plugin_data->sample_files = paths;
    
    int f_index = 0;
    int f_samples_loaded_count = 0;
    int f_current_string_index = 0;
    
    char * f_result_string = malloc(256);    
    
    while (f_samples_loaded_count < LMS_TOTAL_SAMPLE_COUNT)
    {    
        if(paths[f_index] == '\0')
        {
            break;
        }
        else if(paths[f_index] == LMS_FILES_STRING_DELIMITER)
        {
            f_result_string[f_current_string_index] = '\0';
            
            if(f_current_string_index == 0)
            {
                samplerClear(plugin_data, f_samples_loaded_count);
            }
            else if(strcmp(f_result_string, (plugin_data->sample_paths[f_samples_loaded_count])) != 0)
            {
                samplerLoad(plugin_data,f_result_string,f_samples_loaded_count);                
            }
            f_current_string_index = 0;
            f_samples_loaded_count++;
        }
        else if(paths[f_index] == LMS_FILES_STRING_RELOAD_DELIMITER)
        {            
            f_result_string[f_current_string_index] = '\0';
            
            if(f_current_string_index == 0)
            {
                samplerClear(plugin_data, f_samples_loaded_count);
            }
            else
            {
                samplerLoad(plugin_data,f_result_string,f_samples_loaded_count);
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

char *samplerConfigure(LADSPA_Handle instance, const char *key, const char *value)
{
    Sampler *plugin_data = (Sampler *)instance;

    if (!strcmp(key, "load")) {	
        return samplerLoadAll(plugin_data, value);    
    } else if (!strcmp(key, DSSI_PROJECT_DIRECTORY_KEY)) {
	if (plugin_data->projectDir) free(plugin_data->projectDir);
	plugin_data->projectDir = strdup(value);
	return 0;
    } else if (!strcmp(key, "lastdir")) {
        //do nothing, this is only so the plugin host will recall the last sample directory
        return NULL;
    }

    return strdup("error: unrecognized configure key");
}

#ifdef __GNUC__
__attribute__((constructor)) void init()
#else
void _init()
#endif
{
    char **port_names;
    LADSPA_PortDescriptor *port_descriptors;
    LADSPA_PortRangeHint *port_range_hints;
    int channels;

    samplerStereoLDescriptor =
	(LADSPA_Descriptor *) malloc(sizeof(LADSPA_Descriptor));

    samplerStereoDDescriptor =
	(DSSI_Descriptor *) malloc(sizeof(DSSI_Descriptor));

    for (channels = 1; channels <= 2; ++channels) {

	int stereo = (channels == 2);

	LADSPA_Descriptor *desc = samplerStereoLDescriptor;

	desc->UniqueID = channels;
	desc->Label = LMS_PLUGIN_NAME;
	desc->Properties = LADSPA_PROPERTY_REALTIME | LADSPA_PROPERTY_HARD_RT_CAPABLE;
	desc->Name =  LMS_PLUGIN_LONG_NAME;
	desc->Maker = LMS_PLUGIN_DEV;
	desc->Copyright = "GPL";
	desc->PortCount = Sampler_Stereo_COUNT;

	port_descriptors = (LADSPA_PortDescriptor *)
	    calloc(desc->PortCount, sizeof(LADSPA_PortDescriptor));
	desc->PortDescriptors =
	    (const LADSPA_PortDescriptor *) port_descriptors;

	port_range_hints = (LADSPA_PortRangeHint *)
	    calloc(desc->PortCount, sizeof (LADSPA_PortRangeHint));
	desc->PortRangeHints =
	    (const LADSPA_PortRangeHint *) port_range_hints;

	port_names = (char **) calloc(desc->PortCount, sizeof(char *));
	desc->PortNames = (const char **) port_names;

	/* Parameters for output left */
	port_descriptors[Sampler_OUTPUT_LEFT] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
	port_names[Sampler_OUTPUT_LEFT] = "Output L";
	port_range_hints[Sampler_OUTPUT_LEFT].HintDescriptor = 0;

        /* Parameters for selected sample */
	port_descriptors[Sampler_SELECTED_SAMPLE] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[Sampler_SELECTED_SAMPLE] = "Selected Sample";
	port_range_hints[Sampler_SELECTED_SAMPLE].HintDescriptor =
	    LADSPA_HINT_DEFAULT_MINIMUM | LADSPA_HINT_INTEGER |
	    LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[Sampler_SELECTED_SAMPLE].LowerBound = 0;
	port_range_hints[Sampler_SELECTED_SAMPLE].UpperBound = (LMS_MAX_SAMPLE_COUNT - 1);
        
	if (stereo) {

	    /* Parameters for output right */
	    port_descriptors[Sampler_OUTPUT_RIGHT] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
	    port_names[Sampler_OUTPUT_RIGHT] = "Output R";
	    port_range_hints[Sampler_OUTPUT_RIGHT].HintDescriptor = 0;
	}
                
	/* Parameters for attack */
	port_descriptors[LMS_ATTACK] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_ATTACK] = "Attack time (s)";
	port_range_hints[LMS_ATTACK].HintDescriptor =
			LADSPA_HINT_DEFAULT_MINIMUM |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_ATTACK].LowerBound = 1; 
	port_range_hints[LMS_ATTACK].UpperBound = 100; 

	/* Parameters for decay */
	port_descriptors[LMS_DECAY] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_DECAY] = "Decay time (s)";
	port_range_hints[LMS_DECAY].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_DECAY].LowerBound = 1; 
	port_range_hints[LMS_DECAY].UpperBound = 100; 

	/* Parameters for sustain */
	port_descriptors[LMS_SUSTAIN] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_SUSTAIN] = "Sustain level (%)";
	port_range_hints[LMS_SUSTAIN].HintDescriptor =
			LADSPA_HINT_DEFAULT_MAXIMUM |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_SUSTAIN].LowerBound = -60;
	port_range_hints[LMS_SUSTAIN].UpperBound = 0;

	/* Parameters for release */
	port_descriptors[LMS_RELEASE] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_RELEASE] = "Release time (s)";
	port_range_hints[LMS_RELEASE].HintDescriptor =
			LADSPA_HINT_DEFAULT_MINIMUM | 
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_RELEASE].LowerBound = 5; 
	port_range_hints[LMS_RELEASE].UpperBound = 400; 
                
	/* Parameters for attack_f */
	port_descriptors[LMS_FILTER_ATTACK] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_FILTER_ATTACK] = "Attack time (s) filter";
	port_range_hints[LMS_FILTER_ATTACK].HintDescriptor =
			LADSPA_HINT_DEFAULT_LOW |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_FILTER_ATTACK].LowerBound = 1; 
	port_range_hints[LMS_FILTER_ATTACK].UpperBound = 100; 

	/* Parameters for decay_f */
	port_descriptors[LMS_FILTER_DECAY] = port_descriptors[LMS_ATTACK];
	port_names[LMS_FILTER_DECAY] = "Decay time (s) filter";
	port_range_hints[LMS_FILTER_DECAY].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_FILTER_DECAY].LowerBound = 1;
	port_range_hints[LMS_FILTER_DECAY].UpperBound = 100;

	/* Parameters for sustain_f */
	port_descriptors[LMS_FILTER_SUSTAIN] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_FILTER_SUSTAIN] = "Sustain level (%) filter";
	port_range_hints[LMS_FILTER_SUSTAIN].HintDescriptor =
			LADSPA_HINT_DEFAULT_HIGH |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_FILTER_SUSTAIN].LowerBound = 0; 
	port_range_hints[LMS_FILTER_SUSTAIN].UpperBound = 100; 
        
	/* Parameters for release_f */
	port_descriptors[LMS_FILTER_RELEASE] = port_descriptors[LMS_ATTACK];
	port_names[LMS_FILTER_RELEASE] = "Release time (s) filter";
	port_range_hints[LMS_FILTER_RELEASE].HintDescriptor =
			LADSPA_HINT_DEFAULT_LOW  |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_FILTER_RELEASE].LowerBound = 1; 
	port_range_hints[LMS_FILTER_RELEASE].UpperBound = 400; 

        
        /*Parameters for noise_amp*/        
	port_descriptors[LMS_NOISE_AMP] = port_descriptors[LMS_ATTACK];
	port_names[LMS_NOISE_AMP] = "Noise Amp";
	port_range_hints[LMS_NOISE_AMP].HintDescriptor =
			LADSPA_HINT_DEFAULT_MINIMUM |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_NOISE_AMP].LowerBound =  -60;
	port_range_hints[LMS_NOISE_AMP].UpperBound =  0;
                
        /*Parameters for master vol*/        
	port_descriptors[LMS_MASTER_VOLUME] = port_descriptors[LMS_ATTACK];
	port_names[LMS_MASTER_VOLUME] = "Master Vol";
	port_range_hints[LMS_MASTER_VOLUME].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_MASTER_VOLUME].LowerBound =  -24;
	port_range_hints[LMS_MASTER_VOLUME].UpperBound =  24;
                        
        /*Parameters for master glide*/        
	port_descriptors[LMS_MASTER_GLIDE] = port_descriptors[LMS_ATTACK];
	port_names[LMS_MASTER_GLIDE] = "Master Glide";
	port_range_hints[LMS_MASTER_GLIDE].HintDescriptor =
			LADSPA_HINT_DEFAULT_MINIMUM |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_MASTER_GLIDE].LowerBound =  0;
	port_range_hints[LMS_MASTER_GLIDE].UpperBound =  200;
        
        
        /*Parameters for master pitchbend amt*/        
	port_descriptors[LMS_MASTER_PITCHBEND_AMT] = port_descriptors[LMS_ATTACK];
	port_names[LMS_MASTER_PITCHBEND_AMT] = "Pitchbend Amt";
	port_range_hints[LMS_MASTER_PITCHBEND_AMT].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_MASTER_PITCHBEND_AMT].LowerBound =  1;
	port_range_hints[LMS_MASTER_PITCHBEND_AMT].UpperBound =  36;
        
        /*Parameters for pitch env time*/        
	port_descriptors[LMS_PITCH_ENV_TIME] = port_descriptors[LMS_ATTACK];
	port_names[LMS_PITCH_ENV_TIME] = "Pitch Env Time";
	port_range_hints[LMS_PITCH_ENV_TIME].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PITCH_ENV_TIME].LowerBound = 0; 
	port_range_hints[LMS_PITCH_ENV_TIME].UpperBound = 200;
        
        /*Parameters for LFO Freq*/        
	port_descriptors[LMS_LFO_FREQ] = port_descriptors[LMS_ATTACK];
	port_names[LMS_LFO_FREQ] = "LFO Freq";
	port_range_hints[LMS_LFO_FREQ].HintDescriptor =
			LADSPA_HINT_DEFAULT_LOW |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_LFO_FREQ].LowerBound = 10; 
	port_range_hints[LMS_LFO_FREQ].UpperBound = 1600;
        
        /*Parameters for LFO Type*/        
	port_descriptors[LMS_LFO_TYPE] = port_descriptors[LMS_ATTACK];
	port_names[LMS_LFO_TYPE] = "LFO Type";
	port_range_hints[LMS_LFO_TYPE].HintDescriptor =
			LADSPA_HINT_DEFAULT_MINIMUM |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_LFO_TYPE].LowerBound = 0; 
	port_range_hints[LMS_LFO_TYPE].UpperBound = 2;
        
        port_descriptors[LMS_FX0_KNOB0] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_FX0_KNOB0] = "FX0 Knob0";
	port_range_hints[LMS_FX0_KNOB0].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_FX0_KNOB0].LowerBound =  0;
	port_range_hints[LMS_FX0_KNOB0].UpperBound =  127;
        
        	
	port_descriptors[LMS_FX0_KNOB1] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_FX0_KNOB1] = "FX0 Knob1";
	port_range_hints[LMS_FX0_KNOB1].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_FX0_KNOB1].LowerBound =  0;
	port_range_hints[LMS_FX0_KNOB1].UpperBound =  127;
        	
	port_descriptors[LMS_FX0_KNOB2] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_FX0_KNOB2] = "FX0 Knob2";
	port_range_hints[LMS_FX0_KNOB2].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_FX0_KNOB2].LowerBound =  0;
	port_range_hints[LMS_FX0_KNOB2].UpperBound =  127;
        
	port_descriptors[LMS_FX0_COMBOBOX] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_FX0_COMBOBOX] = "FX0 Type";
	port_range_hints[LMS_FX0_COMBOBOX].HintDescriptor =
                        LADSPA_HINT_DEFAULT_MINIMUM | LADSPA_HINT_INTEGER |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_FX0_COMBOBOX].LowerBound =  0;
	port_range_hints[LMS_FX0_COMBOBOX].UpperBound =  MULTIFX3KNOB_MAX_INDEX;
        
        	
	port_descriptors[LMS_FX1_KNOB0] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_FX1_KNOB0] = "FX1 Knob0";
	port_range_hints[LMS_FX1_KNOB0].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_FX1_KNOB0].LowerBound =  0;
	port_range_hints[LMS_FX1_KNOB0].UpperBound =  127;
        
        	
	port_descriptors[LMS_FX1_KNOB1] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_FX1_KNOB1] = "FX1 Knob1";
	port_range_hints[LMS_FX1_KNOB1].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_FX1_KNOB1].LowerBound =  0;
	port_range_hints[LMS_FX1_KNOB1].UpperBound =  127;
        	
	port_descriptors[LMS_FX1_KNOB2] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_FX1_KNOB2] = "FX1 Knob2";
	port_range_hints[LMS_FX1_KNOB2].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_FX1_KNOB2].LowerBound =  0;
	port_range_hints[LMS_FX1_KNOB2].UpperBound =  127;
        
	port_descriptors[LMS_FX1_COMBOBOX] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_FX1_COMBOBOX] = "FX1 Type";
	port_range_hints[LMS_FX1_COMBOBOX].HintDescriptor =
                        LADSPA_HINT_DEFAULT_MINIMUM | LADSPA_HINT_INTEGER |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_FX1_COMBOBOX].LowerBound =  0;
	port_range_hints[LMS_FX1_COMBOBOX].UpperBound =  MULTIFX3KNOB_MAX_INDEX;
        
        
        
        port_descriptors[LMS_FX2_KNOB0] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_FX2_KNOB0] = "FX2 Knob0";
	port_range_hints[LMS_FX2_KNOB0].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_FX2_KNOB0].LowerBound =  0;
	port_range_hints[LMS_FX2_KNOB0].UpperBound =  127;
        
        	
	port_descriptors[LMS_FX2_KNOB1] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_FX2_KNOB1] = "FX2 Knob1";
	port_range_hints[LMS_FX2_KNOB1].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_FX2_KNOB1].LowerBound =  0;
	port_range_hints[LMS_FX2_KNOB1].UpperBound =  127;
        	
	port_descriptors[LMS_FX2_KNOB2] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_FX2_KNOB2] = "FX2 Knob2";
	port_range_hints[LMS_FX2_KNOB2].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_FX2_KNOB2].LowerBound =  0;
	port_range_hints[LMS_FX2_KNOB2].UpperBound =  127;
        
	port_descriptors[LMS_FX2_COMBOBOX] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_FX2_COMBOBOX] = "FX2 Type";
	port_range_hints[LMS_FX2_COMBOBOX].HintDescriptor =
                        LADSPA_HINT_DEFAULT_MINIMUM | LADSPA_HINT_INTEGER |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_FX2_COMBOBOX].LowerBound =  0;
	port_range_hints[LMS_FX2_COMBOBOX].UpperBound =  MULTIFX3KNOB_MAX_INDEX;
        
        	
	port_descriptors[LMS_FX3_KNOB0] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_FX3_KNOB0] = "FX3 Knob0";
	port_range_hints[LMS_FX3_KNOB0].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_FX3_KNOB0].LowerBound =  0;
	port_range_hints[LMS_FX3_KNOB0].UpperBound =  127;
        
        	
	port_descriptors[LMS_FX3_KNOB1] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_FX3_KNOB1] = "FX3 Knob1";
	port_range_hints[LMS_FX3_KNOB1].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_FX3_KNOB1].LowerBound =  0;
	port_range_hints[LMS_FX3_KNOB1].UpperBound =  127;
        	
	port_descriptors[LMS_FX3_KNOB2] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_FX3_KNOB2] = "FX3 Knob2";
	port_range_hints[LMS_FX3_KNOB2].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_FX3_KNOB2].LowerBound =  0;
	port_range_hints[LMS_FX3_KNOB2].UpperBound =  127;
        
	port_descriptors[LMS_FX3_COMBOBOX] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_FX3_COMBOBOX] = "FX3 Type";
	port_range_hints[LMS_FX3_COMBOBOX].HintDescriptor =
                        LADSPA_HINT_DEFAULT_MINIMUM | LADSPA_HINT_INTEGER |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_FX3_COMBOBOX].LowerBound =  0;
	port_range_hints[LMS_FX3_COMBOBOX].UpperBound =  MULTIFX3KNOB_MAX_INDEX;
        
        //From PolyFX mod matrix
        
        port_descriptors[LMS_PFXMATRIX_GRP0DST0SRC0CTRL0] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL; port_names[LMS_PFXMATRIX_GRP0DST0SRC0CTRL0] = "LMS_PFXMATRIX_GRP0DST0SRC0CTRL0";
	port_range_hints[LMS_PFXMATRIX_GRP0DST0SRC0CTRL0].HintDescriptor = LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PFXMATRIX_GRP0DST0SRC0CTRL0].LowerBound =  -100; port_range_hints[LMS_PFXMATRIX_GRP0DST0SRC0CTRL0].UpperBound =  100;

	port_descriptors[LMS_PFXMATRIX_GRP0DST0SRC0CTRL1] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL; port_names[LMS_PFXMATRIX_GRP0DST0SRC0CTRL1] = "LMS_PFXMATRIX_GRP0DST0SRC0CTRL1";
	port_range_hints[LMS_PFXMATRIX_GRP0DST0SRC0CTRL1].HintDescriptor = LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PFXMATRIX_GRP0DST0SRC0CTRL1].LowerBound =  -100; port_range_hints[LMS_PFXMATRIX_GRP0DST0SRC0CTRL1].UpperBound =  100;

	port_descriptors[LMS_PFXMATRIX_GRP0DST0SRC0CTRL2] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL; port_names[LMS_PFXMATRIX_GRP0DST0SRC0CTRL2] = "LMS_PFXMATRIX_GRP0DST0SRC0CTRL2";
	port_range_hints[LMS_PFXMATRIX_GRP0DST0SRC0CTRL2].HintDescriptor = LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PFXMATRIX_GRP0DST0SRC0CTRL2].LowerBound =  -100; port_range_hints[LMS_PFXMATRIX_GRP0DST0SRC0CTRL2].UpperBound =  100;

	port_descriptors[LMS_PFXMATRIX_GRP0DST0SRC1CTRL0] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL; port_names[LMS_PFXMATRIX_GRP0DST0SRC1CTRL0] = "LMS_PFXMATRIX_GRP0DST0SRC1CTRL0";
	port_range_hints[LMS_PFXMATRIX_GRP0DST0SRC1CTRL0].HintDescriptor = LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PFXMATRIX_GRP0DST0SRC1CTRL0].LowerBound =  -100; port_range_hints[LMS_PFXMATRIX_GRP0DST0SRC1CTRL0].UpperBound =  100;

	port_descriptors[LMS_PFXMATRIX_GRP0DST0SRC1CTRL1] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL; port_names[LMS_PFXMATRIX_GRP0DST0SRC1CTRL1] = "LMS_PFXMATRIX_GRP0DST0SRC1CTRL1";
	port_range_hints[LMS_PFXMATRIX_GRP0DST0SRC1CTRL1].HintDescriptor = LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PFXMATRIX_GRP0DST0SRC1CTRL1].LowerBound =  -100; port_range_hints[LMS_PFXMATRIX_GRP0DST0SRC1CTRL1].UpperBound =  100;

	port_descriptors[LMS_PFXMATRIX_GRP0DST0SRC1CTRL2] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL; port_names[LMS_PFXMATRIX_GRP0DST0SRC1CTRL2] = "LMS_PFXMATRIX_GRP0DST0SRC1CTRL2";
	port_range_hints[LMS_PFXMATRIX_GRP0DST0SRC1CTRL2].HintDescriptor = LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PFXMATRIX_GRP0DST0SRC1CTRL2].LowerBound =  -100; port_range_hints[LMS_PFXMATRIX_GRP0DST0SRC1CTRL2].UpperBound =  100;

	port_descriptors[LMS_PFXMATRIX_GRP0DST0SRC2CTRL0] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL; port_names[LMS_PFXMATRIX_GRP0DST0SRC2CTRL0] = "LMS_PFXMATRIX_GRP0DST0SRC2CTRL0";
	port_range_hints[LMS_PFXMATRIX_GRP0DST0SRC2CTRL0].HintDescriptor = LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PFXMATRIX_GRP0DST0SRC2CTRL0].LowerBound =  -100; port_range_hints[LMS_PFXMATRIX_GRP0DST0SRC2CTRL0].UpperBound =  100;

	port_descriptors[LMS_PFXMATRIX_GRP0DST0SRC2CTRL1] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL; port_names[LMS_PFXMATRIX_GRP0DST0SRC2CTRL1] = "LMS_PFXMATRIX_GRP0DST0SRC2CTRL1";
	port_range_hints[LMS_PFXMATRIX_GRP0DST0SRC2CTRL1].HintDescriptor = LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PFXMATRIX_GRP0DST0SRC2CTRL1].LowerBound =  -100; port_range_hints[LMS_PFXMATRIX_GRP0DST0SRC2CTRL1].UpperBound =  100;

	port_descriptors[LMS_PFXMATRIX_GRP0DST0SRC2CTRL2] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL; port_names[LMS_PFXMATRIX_GRP0DST0SRC2CTRL2] = "LMS_PFXMATRIX_GRP0DST0SRC2CTRL2";
	port_range_hints[LMS_PFXMATRIX_GRP0DST0SRC2CTRL2].HintDescriptor = LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PFXMATRIX_GRP0DST0SRC2CTRL2].LowerBound =  -100; port_range_hints[LMS_PFXMATRIX_GRP0DST0SRC2CTRL2].UpperBound =  100;

	port_descriptors[LMS_PFXMATRIX_GRP0DST0SRC3CTRL0] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL; port_names[LMS_PFXMATRIX_GRP0DST0SRC3CTRL0] = "LMS_PFXMATRIX_GRP0DST0SRC3CTRL0";
	port_range_hints[LMS_PFXMATRIX_GRP0DST0SRC3CTRL0].HintDescriptor = LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PFXMATRIX_GRP0DST0SRC3CTRL0].LowerBound =  -100; port_range_hints[LMS_PFXMATRIX_GRP0DST0SRC3CTRL0].UpperBound =  100;

	port_descriptors[LMS_PFXMATRIX_GRP0DST0SRC3CTRL1] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL; port_names[LMS_PFXMATRIX_GRP0DST0SRC3CTRL1] = "LMS_PFXMATRIX_GRP0DST0SRC3CTRL1";
	port_range_hints[LMS_PFXMATRIX_GRP0DST0SRC3CTRL1].HintDescriptor = LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PFXMATRIX_GRP0DST0SRC3CTRL1].LowerBound =  -100; port_range_hints[LMS_PFXMATRIX_GRP0DST0SRC3CTRL1].UpperBound =  100;

	port_descriptors[LMS_PFXMATRIX_GRP0DST0SRC3CTRL2] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL; port_names[LMS_PFXMATRIX_GRP0DST0SRC3CTRL2] = "LMS_PFXMATRIX_GRP0DST0SRC3CTRL2";
	port_range_hints[LMS_PFXMATRIX_GRP0DST0SRC3CTRL2].HintDescriptor = LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PFXMATRIX_GRP0DST0SRC3CTRL2].LowerBound =  -100; port_range_hints[LMS_PFXMATRIX_GRP0DST0SRC3CTRL2].UpperBound =  100;

	port_descriptors[LMS_PFXMATRIX_GRP0DST1SRC0CTRL0] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL; port_names[LMS_PFXMATRIX_GRP0DST1SRC0CTRL0] = "LMS_PFXMATRIX_GRP0DST1SRC0CTRL0";
	port_range_hints[LMS_PFXMATRIX_GRP0DST1SRC0CTRL0].HintDescriptor = LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PFXMATRIX_GRP0DST1SRC0CTRL0].LowerBound =  -100; port_range_hints[LMS_PFXMATRIX_GRP0DST1SRC0CTRL0].UpperBound =  100;

	port_descriptors[LMS_PFXMATRIX_GRP0DST1SRC0CTRL1] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL; port_names[LMS_PFXMATRIX_GRP0DST1SRC0CTRL1] = "LMS_PFXMATRIX_GRP0DST1SRC0CTRL1";
	port_range_hints[LMS_PFXMATRIX_GRP0DST1SRC0CTRL1].HintDescriptor = LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PFXMATRIX_GRP0DST1SRC0CTRL1].LowerBound =  -100; port_range_hints[LMS_PFXMATRIX_GRP0DST1SRC0CTRL1].UpperBound =  100;

	port_descriptors[LMS_PFXMATRIX_GRP0DST1SRC0CTRL2] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL; port_names[LMS_PFXMATRIX_GRP0DST1SRC0CTRL2] = "LMS_PFXMATRIX_GRP0DST1SRC0CTRL2";
	port_range_hints[LMS_PFXMATRIX_GRP0DST1SRC0CTRL2].HintDescriptor = LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PFXMATRIX_GRP0DST1SRC0CTRL2].LowerBound =  -100; port_range_hints[LMS_PFXMATRIX_GRP0DST1SRC0CTRL2].UpperBound =  100;

	port_descriptors[LMS_PFXMATRIX_GRP0DST1SRC1CTRL0] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL; port_names[LMS_PFXMATRIX_GRP0DST1SRC1CTRL0] = "LMS_PFXMATRIX_GRP0DST1SRC1CTRL0";
	port_range_hints[LMS_PFXMATRIX_GRP0DST1SRC1CTRL0].HintDescriptor = LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PFXMATRIX_GRP0DST1SRC1CTRL0].LowerBound =  -100; port_range_hints[LMS_PFXMATRIX_GRP0DST1SRC1CTRL0].UpperBound =  100;

	port_descriptors[LMS_PFXMATRIX_GRP0DST1SRC1CTRL1] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL; port_names[LMS_PFXMATRIX_GRP0DST1SRC1CTRL1] = "LMS_PFXMATRIX_GRP0DST1SRC1CTRL1";
	port_range_hints[LMS_PFXMATRIX_GRP0DST1SRC1CTRL1].HintDescriptor = LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PFXMATRIX_GRP0DST1SRC1CTRL1].LowerBound =  -100; port_range_hints[LMS_PFXMATRIX_GRP0DST1SRC1CTRL1].UpperBound =  100;

	port_descriptors[LMS_PFXMATRIX_GRP0DST1SRC1CTRL2] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL; port_names[LMS_PFXMATRIX_GRP0DST1SRC1CTRL2] = "LMS_PFXMATRIX_GRP0DST1SRC1CTRL2";
	port_range_hints[LMS_PFXMATRIX_GRP0DST1SRC1CTRL2].HintDescriptor = LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PFXMATRIX_GRP0DST1SRC1CTRL2].LowerBound =  -100; port_range_hints[LMS_PFXMATRIX_GRP0DST1SRC1CTRL2].UpperBound =  100;

	port_descriptors[LMS_PFXMATRIX_GRP0DST1SRC2CTRL0] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL; port_names[LMS_PFXMATRIX_GRP0DST1SRC2CTRL0] = "LMS_PFXMATRIX_GRP0DST1SRC2CTRL0";
	port_range_hints[LMS_PFXMATRIX_GRP0DST1SRC2CTRL0].HintDescriptor = LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PFXMATRIX_GRP0DST1SRC2CTRL0].LowerBound =  -100; port_range_hints[LMS_PFXMATRIX_GRP0DST1SRC2CTRL0].UpperBound =  100;

	port_descriptors[LMS_PFXMATRIX_GRP0DST1SRC2CTRL1] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL; port_names[LMS_PFXMATRIX_GRP0DST1SRC2CTRL1] = "LMS_PFXMATRIX_GRP0DST1SRC2CTRL1";
	port_range_hints[LMS_PFXMATRIX_GRP0DST1SRC2CTRL1].HintDescriptor = LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PFXMATRIX_GRP0DST1SRC2CTRL1].LowerBound =  -100; port_range_hints[LMS_PFXMATRIX_GRP0DST1SRC2CTRL1].UpperBound =  100;

	port_descriptors[LMS_PFXMATRIX_GRP0DST1SRC2CTRL2] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL; port_names[LMS_PFXMATRIX_GRP0DST1SRC2CTRL2] = "LMS_PFXMATRIX_GRP0DST1SRC2CTRL2";
	port_range_hints[LMS_PFXMATRIX_GRP0DST1SRC2CTRL2].HintDescriptor = LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PFXMATRIX_GRP0DST1SRC2CTRL2].LowerBound =  -100; port_range_hints[LMS_PFXMATRIX_GRP0DST1SRC2CTRL2].UpperBound =  100;

	port_descriptors[LMS_PFXMATRIX_GRP0DST1SRC3CTRL0] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL; port_names[LMS_PFXMATRIX_GRP0DST1SRC3CTRL0] = "LMS_PFXMATRIX_GRP0DST1SRC3CTRL0";
	port_range_hints[LMS_PFXMATRIX_GRP0DST1SRC3CTRL0].HintDescriptor = LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PFXMATRIX_GRP0DST1SRC3CTRL0].LowerBound =  -100; port_range_hints[LMS_PFXMATRIX_GRP0DST1SRC3CTRL0].UpperBound =  100;

	port_descriptors[LMS_PFXMATRIX_GRP0DST1SRC3CTRL1] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL; port_names[LMS_PFXMATRIX_GRP0DST1SRC3CTRL1] = "LMS_PFXMATRIX_GRP0DST1SRC3CTRL1";
	port_range_hints[LMS_PFXMATRIX_GRP0DST1SRC3CTRL1].HintDescriptor = LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PFXMATRIX_GRP0DST1SRC3CTRL1].LowerBound =  -100; port_range_hints[LMS_PFXMATRIX_GRP0DST1SRC3CTRL1].UpperBound =  100;

	port_descriptors[LMS_PFXMATRIX_GRP0DST1SRC3CTRL2] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL; port_names[LMS_PFXMATRIX_GRP0DST1SRC3CTRL2] = "LMS_PFXMATRIX_GRP0DST1SRC3CTRL2";
	port_range_hints[LMS_PFXMATRIX_GRP0DST1SRC3CTRL2].HintDescriptor = LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PFXMATRIX_GRP0DST1SRC3CTRL2].LowerBound =  -100; port_range_hints[LMS_PFXMATRIX_GRP0DST1SRC3CTRL2].UpperBound =  100;

	port_descriptors[LMS_PFXMATRIX_GRP0DST2SRC0CTRL0] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL; port_names[LMS_PFXMATRIX_GRP0DST2SRC0CTRL0] = "LMS_PFXMATRIX_GRP0DST2SRC0CTRL0";
	port_range_hints[LMS_PFXMATRIX_GRP0DST2SRC0CTRL0].HintDescriptor = LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PFXMATRIX_GRP0DST2SRC0CTRL0].LowerBound =  -100; port_range_hints[LMS_PFXMATRIX_GRP0DST2SRC0CTRL0].UpperBound =  100;

	port_descriptors[LMS_PFXMATRIX_GRP0DST2SRC0CTRL1] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL; port_names[LMS_PFXMATRIX_GRP0DST2SRC0CTRL1] = "LMS_PFXMATRIX_GRP0DST2SRC0CTRL1";
	port_range_hints[LMS_PFXMATRIX_GRP0DST2SRC0CTRL1].HintDescriptor = LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PFXMATRIX_GRP0DST2SRC0CTRL1].LowerBound =  -100; port_range_hints[LMS_PFXMATRIX_GRP0DST2SRC0CTRL1].UpperBound =  100;

	port_descriptors[LMS_PFXMATRIX_GRP0DST2SRC0CTRL2] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL; port_names[LMS_PFXMATRIX_GRP0DST2SRC0CTRL2] = "LMS_PFXMATRIX_GRP0DST2SRC0CTRL2";
	port_range_hints[LMS_PFXMATRIX_GRP0DST2SRC0CTRL2].HintDescriptor = LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PFXMATRIX_GRP0DST2SRC0CTRL2].LowerBound =  -100; port_range_hints[LMS_PFXMATRIX_GRP0DST2SRC0CTRL2].UpperBound =  100;

	port_descriptors[LMS_PFXMATRIX_GRP0DST2SRC1CTRL0] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL; port_names[LMS_PFXMATRIX_GRP0DST2SRC1CTRL0] = "LMS_PFXMATRIX_GRP0DST2SRC1CTRL0";
	port_range_hints[LMS_PFXMATRIX_GRP0DST2SRC1CTRL0].HintDescriptor = LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PFXMATRIX_GRP0DST2SRC1CTRL0].LowerBound =  -100; port_range_hints[LMS_PFXMATRIX_GRP0DST2SRC1CTRL0].UpperBound =  100;

	port_descriptors[LMS_PFXMATRIX_GRP0DST2SRC1CTRL1] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL; port_names[LMS_PFXMATRIX_GRP0DST2SRC1CTRL1] = "LMS_PFXMATRIX_GRP0DST2SRC1CTRL1";
	port_range_hints[LMS_PFXMATRIX_GRP0DST2SRC1CTRL1].HintDescriptor = LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PFXMATRIX_GRP0DST2SRC1CTRL1].LowerBound =  -100; port_range_hints[LMS_PFXMATRIX_GRP0DST2SRC1CTRL1].UpperBound =  100;

	port_descriptors[LMS_PFXMATRIX_GRP0DST2SRC1CTRL2] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL; port_names[LMS_PFXMATRIX_GRP0DST2SRC1CTRL2] = "LMS_PFXMATRIX_GRP0DST2SRC1CTRL2";
	port_range_hints[LMS_PFXMATRIX_GRP0DST2SRC1CTRL2].HintDescriptor = LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PFXMATRIX_GRP0DST2SRC1CTRL2].LowerBound =  -100; port_range_hints[LMS_PFXMATRIX_GRP0DST2SRC1CTRL2].UpperBound =  100;

	port_descriptors[LMS_PFXMATRIX_GRP0DST2SRC2CTRL0] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL; port_names[LMS_PFXMATRIX_GRP0DST2SRC2CTRL0] = "LMS_PFXMATRIX_GRP0DST2SRC2CTRL0";
	port_range_hints[LMS_PFXMATRIX_GRP0DST2SRC2CTRL0].HintDescriptor = LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PFXMATRIX_GRP0DST2SRC2CTRL0].LowerBound =  -100; port_range_hints[LMS_PFXMATRIX_GRP0DST2SRC2CTRL0].UpperBound =  100;

	port_descriptors[LMS_PFXMATRIX_GRP0DST2SRC2CTRL1] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL; port_names[LMS_PFXMATRIX_GRP0DST2SRC2CTRL1] = "LMS_PFXMATRIX_GRP0DST2SRC2CTRL1";
	port_range_hints[LMS_PFXMATRIX_GRP0DST2SRC2CTRL1].HintDescriptor = LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PFXMATRIX_GRP0DST2SRC2CTRL1].LowerBound =  -100; port_range_hints[LMS_PFXMATRIX_GRP0DST2SRC2CTRL1].UpperBound =  100;

	port_descriptors[LMS_PFXMATRIX_GRP0DST2SRC2CTRL2] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL; port_names[LMS_PFXMATRIX_GRP0DST2SRC2CTRL2] = "LMS_PFXMATRIX_GRP0DST2SRC2CTRL2";
	port_range_hints[LMS_PFXMATRIX_GRP0DST2SRC2CTRL2].HintDescriptor = LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PFXMATRIX_GRP0DST2SRC2CTRL2].LowerBound =  -100; port_range_hints[LMS_PFXMATRIX_GRP0DST2SRC2CTRL2].UpperBound =  100;

	port_descriptors[LMS_PFXMATRIX_GRP0DST2SRC3CTRL0] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL; port_names[LMS_PFXMATRIX_GRP0DST2SRC3CTRL0] = "LMS_PFXMATRIX_GRP0DST2SRC3CTRL0";
	port_range_hints[LMS_PFXMATRIX_GRP0DST2SRC3CTRL0].HintDescriptor = LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PFXMATRIX_GRP0DST2SRC3CTRL0].LowerBound =  -100; port_range_hints[LMS_PFXMATRIX_GRP0DST2SRC3CTRL0].UpperBound =  100;

	port_descriptors[LMS_PFXMATRIX_GRP0DST2SRC3CTRL1] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL; port_names[LMS_PFXMATRIX_GRP0DST2SRC3CTRL1] = "LMS_PFXMATRIX_GRP0DST2SRC3CTRL1";
	port_range_hints[LMS_PFXMATRIX_GRP0DST2SRC3CTRL1].HintDescriptor = LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PFXMATRIX_GRP0DST2SRC3CTRL1].LowerBound =  -100; port_range_hints[LMS_PFXMATRIX_GRP0DST2SRC3CTRL1].UpperBound =  100;

	port_descriptors[LMS_PFXMATRIX_GRP0DST2SRC3CTRL2] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL; port_names[LMS_PFXMATRIX_GRP0DST2SRC3CTRL2] = "LMS_PFXMATRIX_GRP0DST2SRC3CTRL2";
	port_range_hints[LMS_PFXMATRIX_GRP0DST2SRC3CTRL2].HintDescriptor = LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PFXMATRIX_GRP0DST2SRC3CTRL2].LowerBound =  -100; port_range_hints[LMS_PFXMATRIX_GRP0DST2SRC3CTRL2].UpperBound =  100;

	port_descriptors[LMS_PFXMATRIX_GRP0DST3SRC0CTRL0] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL; port_names[LMS_PFXMATRIX_GRP0DST3SRC0CTRL0] = "LMS_PFXMATRIX_GRP0DST3SRC0CTRL0";
	port_range_hints[LMS_PFXMATRIX_GRP0DST3SRC0CTRL0].HintDescriptor = LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PFXMATRIX_GRP0DST3SRC0CTRL0].LowerBound =  -100; port_range_hints[LMS_PFXMATRIX_GRP0DST3SRC0CTRL0].UpperBound =  100;

	port_descriptors[LMS_PFXMATRIX_GRP0DST3SRC0CTRL1] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL; port_names[LMS_PFXMATRIX_GRP0DST3SRC0CTRL1] = "LMS_PFXMATRIX_GRP0DST3SRC0CTRL1";
	port_range_hints[LMS_PFXMATRIX_GRP0DST3SRC0CTRL1].HintDescriptor = LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PFXMATRIX_GRP0DST3SRC0CTRL1].LowerBound =  -100; port_range_hints[LMS_PFXMATRIX_GRP0DST3SRC0CTRL1].UpperBound =  100;

	port_descriptors[LMS_PFXMATRIX_GRP0DST3SRC0CTRL2] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL; port_names[LMS_PFXMATRIX_GRP0DST3SRC0CTRL2] = "LMS_PFXMATRIX_GRP0DST3SRC0CTRL2";
	port_range_hints[LMS_PFXMATRIX_GRP0DST3SRC0CTRL2].HintDescriptor = LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PFXMATRIX_GRP0DST3SRC0CTRL2].LowerBound =  -100; port_range_hints[LMS_PFXMATRIX_GRP0DST3SRC0CTRL2].UpperBound =  100;

	port_descriptors[LMS_PFXMATRIX_GRP0DST3SRC1CTRL0] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL; port_names[LMS_PFXMATRIX_GRP0DST3SRC1CTRL0] = "LMS_PFXMATRIX_GRP0DST3SRC1CTRL0";
	port_range_hints[LMS_PFXMATRIX_GRP0DST3SRC1CTRL0].HintDescriptor = LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PFXMATRIX_GRP0DST3SRC1CTRL0].LowerBound =  -100; port_range_hints[LMS_PFXMATRIX_GRP0DST3SRC1CTRL0].UpperBound =  100;

	port_descriptors[LMS_PFXMATRIX_GRP0DST3SRC1CTRL1] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL; port_names[LMS_PFXMATRIX_GRP0DST3SRC1CTRL1] = "LMS_PFXMATRIX_GRP0DST3SRC1CTRL1";
	port_range_hints[LMS_PFXMATRIX_GRP0DST3SRC1CTRL1].HintDescriptor = LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PFXMATRIX_GRP0DST3SRC1CTRL1].LowerBound =  -100; port_range_hints[LMS_PFXMATRIX_GRP0DST3SRC1CTRL1].UpperBound =  100;

	port_descriptors[LMS_PFXMATRIX_GRP0DST3SRC1CTRL2] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL; port_names[LMS_PFXMATRIX_GRP0DST3SRC1CTRL2] = "LMS_PFXMATRIX_GRP0DST3SRC1CTRL2";
	port_range_hints[LMS_PFXMATRIX_GRP0DST3SRC1CTRL2].HintDescriptor = LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PFXMATRIX_GRP0DST3SRC1CTRL2].LowerBound =  -100; port_range_hints[LMS_PFXMATRIX_GRP0DST3SRC1CTRL2].UpperBound =  100;

	port_descriptors[LMS_PFXMATRIX_GRP0DST3SRC2CTRL0] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL; port_names[LMS_PFXMATRIX_GRP0DST3SRC2CTRL0] = "LMS_PFXMATRIX_GRP0DST3SRC2CTRL0";
	port_range_hints[LMS_PFXMATRIX_GRP0DST3SRC2CTRL0].HintDescriptor = LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PFXMATRIX_GRP0DST3SRC2CTRL0].LowerBound =  -100; port_range_hints[LMS_PFXMATRIX_GRP0DST3SRC2CTRL0].UpperBound =  100;

	port_descriptors[LMS_PFXMATRIX_GRP0DST3SRC2CTRL1] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL; port_names[LMS_PFXMATRIX_GRP0DST3SRC2CTRL1] = "LMS_PFXMATRIX_GRP0DST3SRC2CTRL1";
	port_range_hints[LMS_PFXMATRIX_GRP0DST3SRC2CTRL1].HintDescriptor = LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PFXMATRIX_GRP0DST3SRC2CTRL1].LowerBound =  -100; port_range_hints[LMS_PFXMATRIX_GRP0DST3SRC2CTRL1].UpperBound =  100;

	port_descriptors[LMS_PFXMATRIX_GRP0DST3SRC2CTRL2] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL; port_names[LMS_PFXMATRIX_GRP0DST3SRC2CTRL2] = "LMS_PFXMATRIX_GRP0DST3SRC2CTRL2";
	port_range_hints[LMS_PFXMATRIX_GRP0DST3SRC2CTRL2].HintDescriptor = LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PFXMATRIX_GRP0DST3SRC2CTRL2].LowerBound =  -100; port_range_hints[LMS_PFXMATRIX_GRP0DST3SRC2CTRL2].UpperBound =  100;

	port_descriptors[LMS_PFXMATRIX_GRP0DST3SRC3CTRL0] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL; port_names[LMS_PFXMATRIX_GRP0DST3SRC3CTRL0] = "LMS_PFXMATRIX_GRP0DST3SRC3CTRL0";
	port_range_hints[LMS_PFXMATRIX_GRP0DST3SRC3CTRL0].HintDescriptor = LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PFXMATRIX_GRP0DST3SRC3CTRL0].LowerBound =  -100; port_range_hints[LMS_PFXMATRIX_GRP0DST3SRC3CTRL0].UpperBound =  100;

	port_descriptors[LMS_PFXMATRIX_GRP0DST3SRC3CTRL1] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL; port_names[LMS_PFXMATRIX_GRP0DST3SRC3CTRL1] = "LMS_PFXMATRIX_GRP0DST3SRC3CTRL1";
	port_range_hints[LMS_PFXMATRIX_GRP0DST3SRC3CTRL1].HintDescriptor = LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PFXMATRIX_GRP0DST3SRC3CTRL1].LowerBound =  -100; port_range_hints[LMS_PFXMATRIX_GRP0DST3SRC3CTRL1].UpperBound =  100;

	port_descriptors[LMS_PFXMATRIX_GRP0DST3SRC3CTRL2] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL; port_names[LMS_PFXMATRIX_GRP0DST3SRC3CTRL2] = "LMS_PFXMATRIX_GRP0DST3SRC3CTRL2";
	port_range_hints[LMS_PFXMATRIX_GRP0DST3SRC3CTRL2].HintDescriptor = LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PFXMATRIX_GRP0DST3SRC3CTRL2].LowerBound =  -100; port_range_hints[LMS_PFXMATRIX_GRP0DST3SRC3CTRL2].UpperBound =  100;
        
        //End from PolyFX mod matrix
        /*
        port_descriptors[LMS_GLOBAL_MIDI_CHANNEL] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_GLOBAL_MIDI_CHANNEL] = "Global MIDI Channel";
	port_range_hints[LMS_GLOBAL_MIDI_CHANNEL].HintDescriptor =
			LADSPA_HINT_DEFAULT_MAXIMUM |LADSPA_HINT_INTEGER |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_GLOBAL_MIDI_CHANNEL].LowerBound =  0;
	port_range_hints[LMS_GLOBAL_MIDI_CHANNEL].UpperBound =  16;
        */
        
	port_descriptors[LMS_GLOBAL_MIDI_OCTAVES_OFFSET] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_GLOBAL_MIDI_OCTAVES_OFFSET] = "Global MIDI Offset(Octaves)";
	port_range_hints[LMS_GLOBAL_MIDI_OCTAVES_OFFSET].HintDescriptor =
                        LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_INTEGER |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_GLOBAL_MIDI_OCTAVES_OFFSET].LowerBound =  -3;
	port_range_hints[LMS_GLOBAL_MIDI_OCTAVES_OFFSET].UpperBound =  3;
        
        port_descriptors[LMS_NOISE_TYPE] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_NOISE_TYPE] = "Noise Type";
	port_range_hints[LMS_NOISE_TYPE].HintDescriptor =
                        LADSPA_HINT_DEFAULT_MINIMUM | LADSPA_HINT_INTEGER |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_NOISE_TYPE].LowerBound =  0;
	port_range_hints[LMS_NOISE_TYPE].UpperBound =  2;
        
        int f_i = LMS_SAMPLE_PITCH_PORT_RANGE_MIN;
        
        while(f_i < LMS_SAMPLE_PITCH_PORT_RANGE_MAX)
        {
            port_descriptors[f_i] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
            port_names[f_i] = "Sample Note";
            port_range_hints[f_i].HintDescriptor = LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
            port_range_hints[f_i].LowerBound = 0; port_range_hints[f_i].UpperBound = 120;
            
            f_i++;
        }
        
        while(f_i < LMS_PLAY_PITCH_LOW_PORT_RANGE_MAX)
        {
            port_descriptors[f_i] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
            port_names[f_i] = "Sample Pitch Low";
            port_range_hints[f_i].HintDescriptor = LADSPA_HINT_DEFAULT_MINIMUM | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
            port_range_hints[f_i].LowerBound = 0; port_range_hints[f_i].UpperBound = 120;
            
            f_i++;
        }
        
        while(f_i < LMS_PLAY_PITCH_HIGH_PORT_RANGE_MAX)
        {
            port_descriptors[f_i] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
            port_names[f_i] = "Sample Pitch High";
            port_range_hints[f_i].HintDescriptor = LADSPA_HINT_DEFAULT_MAXIMUM | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
            port_range_hints[f_i].LowerBound = 0; port_range_hints[f_i].UpperBound = 120;
            
            f_i++;
        }
        
        while(f_i < LMS_SAMPLE_VOLUME_PORT_RANGE_MAX)
        {
            port_descriptors[f_i] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
            port_names[f_i] = "Sample Volume";
            port_range_hints[f_i].HintDescriptor = LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
            port_range_hints[f_i].LowerBound = -50; port_range_hints[f_i].UpperBound = 36;
            
            f_i++;
        }

        while(f_i < LMS_SAMPLE_START_PORT_RANGE_MAX)
        {
            port_descriptors[f_i] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
            port_names[f_i] = "Sample Start";
            port_range_hints[f_i].HintDescriptor = LADSPA_HINT_DEFAULT_MINIMUM | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_INTEGER;
            port_range_hints[f_i].LowerBound = 0; port_range_hints[f_i].UpperBound = 10000;
            
            f_i++;
        }
        
        while(f_i < LMS_SAMPLE_END_PORT_RANGE_MAX)
        {
            port_descriptors[f_i] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
            port_names[f_i] = "Sample End";
            port_range_hints[f_i].HintDescriptor = LADSPA_HINT_DEFAULT_MINIMUM | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_INTEGER;
            port_range_hints[f_i].LowerBound = 0; port_range_hints[f_i].UpperBound = 10000;
            
            f_i++;
        }
        
        while(f_i < LMS_SAMPLE_VEL_SENS_PORT_RANGE_MAX)
        {
            port_descriptors[f_i] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
            port_names[f_i] = "Velocity Sensitivity";
            port_range_hints[f_i].HintDescriptor = LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_INTEGER;
            port_range_hints[f_i].LowerBound = 0; port_range_hints[f_i].UpperBound = 20;
            
            f_i++;
        }
        
        while(f_i < LMS_SAMPLE_VEL_LOW_PORT_RANGE_MAX)
        {
            port_descriptors[f_i] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
            port_names[f_i] = "Low Velocity";
            port_range_hints[f_i].HintDescriptor = LADSPA_HINT_DEFAULT_MINIMUM | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_INTEGER;
            port_range_hints[f_i].LowerBound = 0; port_range_hints[f_i].UpperBound = 127;
            
            f_i++;
        }
        
        while(f_i < LMS_SAMPLE_VEL_HIGH_PORT_RANGE_MAX)
        {
            port_descriptors[f_i] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
            port_names[f_i] = "High Velocity";
            port_range_hints[f_i].HintDescriptor = LADSPA_HINT_DEFAULT_MAXIMUM | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_INTEGER;
            port_range_hints[f_i].LowerBound = 0; port_range_hints[f_i].UpperBound = 127;
            
            f_i++;
        }
        //new
        while(f_i < LMS_PITCH_PORT_RANGE_MAX)
        {
            port_descriptors[f_i] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
            port_names[f_i] = "Sample Pitch";
            port_range_hints[f_i].HintDescriptor = LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
            port_range_hints[f_i].LowerBound = -36; port_range_hints[f_i].UpperBound = 36;
            
            f_i++;
        }
        
        while(f_i < LMS_TUNE_PORT_RANGE_MAX)
        {
            port_descriptors[f_i] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
            port_names[f_i] = "Sample Tune";
            port_range_hints[f_i].HintDescriptor = LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
            port_range_hints[f_i].LowerBound = -100; port_range_hints[f_i].UpperBound = 100;
            
            f_i++;
        }
        
        while(f_i < LMS_SAMPLE_INTERPOLATION_MODE_PORT_RANGE_MAX)
        {
            port_descriptors[f_i] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
            port_names[f_i] = "Mode";
            port_range_hints[f_i].HintDescriptor = LADSPA_HINT_DEFAULT_MINIMUM | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_INTEGER;
            port_range_hints[f_i].LowerBound = 0; port_range_hints[f_i].UpperBound = 3;
            
            f_i++;
        }
        
        while(f_i < LMS_SAMPLE_LOOP_START_PORT_RANGE_MAX)
        {
            port_descriptors[f_i] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
            port_names[f_i] = "Loop Start";
            port_range_hints[f_i].HintDescriptor = LADSPA_HINT_DEFAULT_MINIMUM | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_INTEGER;
            port_range_hints[f_i].LowerBound = 0; port_range_hints[f_i].UpperBound = 10000;
            
            f_i++;
        }
        
        while(f_i < LMS_SAMPLE_LOOP_END_PORT_RANGE_MAX)
        {
            port_descriptors[f_i] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
            port_names[f_i] = "Loop End";
            port_range_hints[f_i].HintDescriptor = LADSPA_HINT_DEFAULT_MINIMUM | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_INTEGER;
            port_range_hints[f_i].LowerBound = 0; port_range_hints[f_i].UpperBound = 10000;
            
            f_i++;
        }
        
        while(f_i < LMS_SAMPLE_LOOP_MODE_PORT_RANGE_MAX)
        {
            port_descriptors[f_i] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
            port_names[f_i] = "Loop Modes";
            port_range_hints[f_i].HintDescriptor = LADSPA_HINT_DEFAULT_MINIMUM | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_INTEGER;
            port_range_hints[f_i].LowerBound = 0; port_range_hints[f_i].UpperBound = 1;
            
            f_i++;
        }
        
	desc->activate = activateSampler;
	desc->cleanup = cleanupSampler;
	desc->connect_port = connectPortSampler;
	desc->deactivate = activateSampler;
	desc->instantiate = instantiateSampler;
	desc->run = runSamplerWrapper;
	desc->run_adding = NULL;
	desc->set_run_adding_gain = NULL;
    }

    samplerStereoDDescriptor->DSSI_API_Version = 1;
    samplerStereoDDescriptor->LADSPA_Plugin = samplerStereoLDescriptor;
    samplerStereoDDescriptor->configure = samplerConfigure;
    samplerStereoDDescriptor->get_program = NULL;
    samplerStereoDDescriptor->get_midi_controller_for_port = getControllerSampler;
    samplerStereoDDescriptor->select_program = NULL;
    samplerStereoDDescriptor->run_synth = run_lms_euphoria;
    samplerStereoDDescriptor->run_synth_adding = NULL;
    samplerStereoDDescriptor->run_multiple_synths = NULL;
    samplerStereoDDescriptor->run_multiple_synths_adding = NULL;
}

#ifdef __GNUC__
__attribute__((destructor)) void fini()
#else
void _fini()
#endif
{
    if (samplerStereoLDescriptor) {
	free((LADSPA_PortDescriptor *) samplerStereoLDescriptor->PortDescriptors);
	free((char **) samplerStereoLDescriptor->PortNames);
	free((LADSPA_PortRangeHint *) samplerStereoLDescriptor->PortRangeHints);
	free(samplerStereoLDescriptor);
    }
    if (samplerStereoDDescriptor) {
	free(samplerStereoDDescriptor);
    }
}
