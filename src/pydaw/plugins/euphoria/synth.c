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

#include "../../include/pydaw_plugin.h"

#include <sndfile.h>
#include <pthread.h>

#include "synth.h"
#include "../../libmodsynth/lib/lms_math.h"

static fp_get_wavpool_item_from_host wavpool_get_func;

static void v_run_lms_euphoria(PYFX_Handle instance, int sample_count,
		       t_pydaw_seq_event *events, int EventCount);

static inline void v_euphoria_slow_index(t_euphoria*);

PYFX_Descriptor *euphoria_PYFX_descriptor(int index);
PYINST_Descriptor *euphoria_PYINST_descriptor(int index);

static void cleanupSampler(PYFX_Handle instance)
{
    t_euphoria *plugin = (t_euphoria *)instance;
    free(plugin);
}

static void euphoriaPanic(PYFX_Handle instance)
{
    t_euphoria *plugin = (t_euphoria *)instance;
    int f_i = 0;
    while(f_i < EUPHORIA_POLYPHONY)
    {
        v_adsr_kill(plugin->data[f_i]->adsr_amp);
        f_i++;
    }
}

static void v_euphoria_on_stop(PYFX_Handle instance)
{
    t_euphoria *plugin = (t_euphoria *)instance;
    int f_i = 0;
    while(f_i < EUPHORIA_POLYPHONY)
    {
        v_euphoria_poly_note_off(plugin->data[f_i], 0);
        f_i++;
    }
}

static void euphoriaConnectBuffer(PYFX_Handle instance, int a_index,
        float * DataLocation)
{
    t_euphoria *plugin = (t_euphoria *) instance;

    switch(a_index)
    {
        case 0:
            plugin->output[0] = DataLocation;
            break;
        case 1:
            plugin->output[1] = DataLocation;
            break;
        default:
            assert(0);
            break;
    }
}

static void connectPortSampler(PYFX_Handle instance, int port,
			       PYFX_Data * data)
{
    t_euphoria *plugin = (t_euphoria *) instance;

    if(port < EUPHORIA_LAST_REGULAR_CONTROL_PORT)
    {
        switch (port)
        {
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

            //keyboard tracking
            case EUPHORIA_PFXMATRIX_GRP0DST0SRC4CTRL0: plugin->polyfx_mod_matrix[0][0][4][0] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST0SRC4CTRL1: plugin->polyfx_mod_matrix[0][0][4][1] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST0SRC4CTRL2: plugin->polyfx_mod_matrix[0][0][4][2] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST1SRC4CTRL0: plugin->polyfx_mod_matrix[0][1][4][0] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST1SRC4CTRL1: plugin->polyfx_mod_matrix[0][1][4][1] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST1SRC4CTRL2: plugin->polyfx_mod_matrix[0][1][4][2] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST2SRC4CTRL0: plugin->polyfx_mod_matrix[0][2][4][0] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST2SRC4CTRL1: plugin->polyfx_mod_matrix[0][2][4][1] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST2SRC4CTRL2: plugin->polyfx_mod_matrix[0][2][4][2] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST3SRC4CTRL0: plugin->polyfx_mod_matrix[0][3][4][0] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST3SRC4CTRL1: plugin->polyfx_mod_matrix[0][3][4][1] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST3SRC4CTRL2: plugin->polyfx_mod_matrix[0][3][4][2] = data; break;

            //velocity tracking
            case EUPHORIA_PFXMATRIX_GRP0DST0SRC5CTRL0: plugin->polyfx_mod_matrix[0][0][5][0] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST0SRC5CTRL1: plugin->polyfx_mod_matrix[0][0][5][1] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST0SRC5CTRL2: plugin->polyfx_mod_matrix[0][0][5][2] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST1SRC5CTRL0: plugin->polyfx_mod_matrix[0][1][5][0] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST1SRC5CTRL1: plugin->polyfx_mod_matrix[0][1][5][1] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST1SRC5CTRL2: plugin->polyfx_mod_matrix[0][1][5][2] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST2SRC5CTRL0: plugin->polyfx_mod_matrix[0][2][5][0] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST2SRC5CTRL1: plugin->polyfx_mod_matrix[0][2][5][1] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST2SRC5CTRL2: plugin->polyfx_mod_matrix[0][2][5][2] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST3SRC5CTRL0: plugin->polyfx_mod_matrix[0][3][5][0] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST3SRC5CTRL1: plugin->polyfx_mod_matrix[0][3][5][1] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST3SRC5CTRL2: plugin->polyfx_mod_matrix[0][3][5][2] = data; break;

            //End PolyFX mod matrix
            case EUPHORIA_LFO_PITCH: plugin->lfo_pitch = data; break;
            default:
                break;
        }
    }
    else if((port >= EUPHORIA_SAMPLE_PITCH_PORT_RANGE_MIN) &&
            (port < EUPHORIA_SAMPLE_PITCH_PORT_RANGE_MAX))
    {
        plugin->basePitch[(port - EUPHORIA_SAMPLE_PITCH_PORT_RANGE_MIN)] = data;
    }
    else if((port >= EUPHORIA_PLAY_PITCH_LOW_PORT_RANGE_MIN) &&
            (port < EUPHORIA_PLAY_PITCH_LOW_PORT_RANGE_MAX))
    {
        plugin->low_note[(port - EUPHORIA_PLAY_PITCH_LOW_PORT_RANGE_MIN)] = data;
    }
    else if((port >= EUPHORIA_PLAY_PITCH_HIGH_PORT_RANGE_MIN) &&
            (port < EUPHORIA_PLAY_PITCH_HIGH_PORT_RANGE_MAX))
    {
        plugin->high_note[(port - EUPHORIA_PLAY_PITCH_HIGH_PORT_RANGE_MIN)] = data;
    }
    else if((port >= LMS_SAMPLE_VOLUME_PORT_RANGE_MIN) &&
            (port < EUPHORIA_SAMPLE_VOLUME_PORT_RANGE_MAX))
    {
        plugin->sample_vol[(port - LMS_SAMPLE_VOLUME_PORT_RANGE_MIN)] = data;
    }
    else if((port >= EUPHORIA_SAMPLE_START_PORT_RANGE_MIN) &&
            (port < EUPHORIA_SAMPLE_START_PORT_RANGE_MAX))
    {
        plugin->sampleStarts[(port - EUPHORIA_SAMPLE_START_PORT_RANGE_MIN)] = data;
    }
    else if((port >= EUPHORIA_SAMPLE_END_PORT_RANGE_MIN) &&
            (port < EUPHORIA_SAMPLE_END_PORT_RANGE_MAX))
    {
        plugin->sampleEnds[(port - EUPHORIA_SAMPLE_END_PORT_RANGE_MIN)] = data;
    }
    else if((port >= EUPHORIA_SAMPLE_VEL_SENS_PORT_RANGE_MIN) &&
            (port < EUPHORIA_SAMPLE_VEL_SENS_PORT_RANGE_MAX))
    {
        plugin->sample_vel_sens[(port - EUPHORIA_SAMPLE_VEL_SENS_PORT_RANGE_MIN)] = data;
    }
    else if((port >= EUPHORIA_SAMPLE_VEL_LOW_PORT_RANGE_MIN) &&
            (port < EUPHORIA_SAMPLE_VEL_LOW_PORT_RANGE_MAX))
    {
        plugin->sample_vel_low[(port - EUPHORIA_SAMPLE_VEL_LOW_PORT_RANGE_MIN)] = data;
    }
    else if((port >= EUPHORIA_SAMPLE_VEL_HIGH_PORT_RANGE_MIN) &&
            (port < EUPHORIA_SAMPLE_VEL_HIGH_PORT_RANGE_MAX))
    {
        plugin->sample_vel_high[(port - EUPHORIA_SAMPLE_VEL_HIGH_PORT_RANGE_MIN)] = data;
    }
    //new
    else if((port >= EUPHORIA_PITCH_PORT_RANGE_MIN) &&
            (port < EUPHORIA_PITCH_PORT_RANGE_MAX))
    {
        plugin->sample_pitch[(port - EUPHORIA_PITCH_PORT_RANGE_MIN)] = data;
    }
    else if((port >= EUPHORIA_TUNE_PORT_RANGE_MIN) &&
            (port < EUPHORIA_TUNE_PORT_RANGE_MAX))
    {
        plugin->sample_tune[(port - EUPHORIA_TUNE_PORT_RANGE_MIN)] = data;
    }
    else if((port >= EUPHORIA_SAMPLE_INTERPOLATION_MODE_PORT_RANGE_MIN) &&
            (port < EUPHORIA_SAMPLE_INTERPOLATION_MODE_PORT_RANGE_MAX))
    {
        plugin->sample_interpolation_mode[(port - EUPHORIA_SAMPLE_INTERPOLATION_MODE_PORT_RANGE_MIN)] = data;
    }

    else if((port >= EUPHORIA_SAMPLE_LOOP_START_PORT_RANGE_MIN) &&
            (port < EUPHORIA_SAMPLE_LOOP_START_PORT_RANGE_MAX))
    {
        plugin->sampleLoopStarts[(port - EUPHORIA_SAMPLE_LOOP_START_PORT_RANGE_MIN)] = data;
    }
    else if((port >= EUPHORIA_SAMPLE_LOOP_END_PORT_RANGE_MIN) &&
            (port < EUPHORIA_SAMPLE_LOOP_END_PORT_RANGE_MAX))
    {
        plugin->sampleLoopEnds[(port - EUPHORIA_SAMPLE_LOOP_END_PORT_RANGE_MIN)] = data;
    }
    else if((port >= EUPHORIA_SAMPLE_LOOP_MODE_PORT_RANGE_MIN) &&
            (port < EUPHORIA_SAMPLE_LOOP_MODE_PORT_RANGE_MAX))
    {
        plugin->sampleLoopModes[(port - EUPHORIA_SAMPLE_LOOP_MODE_PORT_RANGE_MIN)] = data;
    }

    //MonoFX0
    else if((port >= EUPHORIA_MONO_FX0_KNOB0_PORT_RANGE_MIN) &&
            (port < EUPHORIA_MONO_FX0_KNOB0_PORT_RANGE_MAX))
    {
        plugin->mfx_knobs[(port - EUPHORIA_MONO_FX0_KNOB0_PORT_RANGE_MIN)][0][0] = data;
    }
    else if((port >= EUPHORIA_MONO_FX0_KNOB1_PORT_RANGE_MIN) &&
            (port < EUPHORIA_MONO_FX0_KNOB1_PORT_RANGE_MAX))
    {
        plugin->mfx_knobs[(port - EUPHORIA_MONO_FX0_KNOB1_PORT_RANGE_MIN)][0][1] = data;
    }
    else if((port >= EUPHORIA_MONO_FX0_KNOB2_PORT_RANGE_MIN) &&
            (port < EUPHORIA_MONO_FX0_KNOB2_PORT_RANGE_MAX))
    {
        plugin->mfx_knobs[(port - EUPHORIA_MONO_FX0_KNOB2_PORT_RANGE_MIN)][0][2] = data;
    }
    else if((port >= EUPHORIA_MONO_FX0_COMBOBOX_PORT_RANGE_MIN) &&
            (port < EUPHORIA_MONO_FX0_COMBOBOX_PORT_RANGE_MAX))
    {
        plugin->mfx_comboboxes[(port - EUPHORIA_MONO_FX0_COMBOBOX_PORT_RANGE_MIN)][0] = data;
    }
    //MonoFX1
    else if((port >= EUPHORIA_MONO_FX1_KNOB0_PORT_RANGE_MIN) &&
            (port < EUPHORIA_MONO_FX1_KNOB0_PORT_RANGE_MAX))
    {
        plugin->mfx_knobs[(port - EUPHORIA_MONO_FX1_KNOB0_PORT_RANGE_MIN)][1][0] = data;
    }
    else if((port >= EUPHORIA_MONO_FX1_KNOB1_PORT_RANGE_MIN) &&
            (port < EUPHORIA_MONO_FX1_KNOB1_PORT_RANGE_MAX))
    {
        plugin->mfx_knobs[(port - EUPHORIA_MONO_FX1_KNOB1_PORT_RANGE_MIN)][1][1] = data;
    }
    else if((port >= EUPHORIA_MONO_FX1_KNOB2_PORT_RANGE_MIN) &&
            (port < EUPHORIA_MONO_FX1_KNOB2_PORT_RANGE_MAX))
    {
        plugin->mfx_knobs[(port - EUPHORIA_MONO_FX1_KNOB2_PORT_RANGE_MIN)][1][2] = data;
    }
    else if((port >= EUPHORIA_MONO_FX1_COMBOBOX_PORT_RANGE_MIN) &&
            (port < EUPHORIA_MONO_FX1_COMBOBOX_PORT_RANGE_MAX))
    {
        plugin->mfx_comboboxes[(port - EUPHORIA_MONO_FX1_COMBOBOX_PORT_RANGE_MIN)][1] = data;
    }
    //MonoFX2
    else if((port >= EUPHORIA_MONO_FX2_KNOB0_PORT_RANGE_MIN) &&
            (port < EUPHORIA_MONO_FX2_KNOB0_PORT_RANGE_MAX))
    {
        plugin->mfx_knobs[(port - EUPHORIA_MONO_FX2_KNOB0_PORT_RANGE_MIN)][2][0] = data;
    }
    else if((port >= EUPHORIA_MONO_FX2_KNOB1_PORT_RANGE_MIN) &&
            (port < EUPHORIA_MONO_FX2_KNOB1_PORT_RANGE_MAX))
    {
        plugin->mfx_knobs[(port - EUPHORIA_MONO_FX2_KNOB1_PORT_RANGE_MIN)][2][1] = data;
    }
    else if((port >= EUPHORIA_MONO_FX2_KNOB2_PORT_RANGE_MIN) &&
            (port < EUPHORIA_MONO_FX2_KNOB2_PORT_RANGE_MAX))
    {
        plugin->mfx_knobs[(port - EUPHORIA_MONO_FX2_KNOB2_PORT_RANGE_MIN)][2][2] = data;
    }
    else if((port >= EUPHORIA_MONO_FX2_COMBOBOX_PORT_RANGE_MIN) &&
            (port < EUPHORIA_MONO_FX2_COMBOBOX_PORT_RANGE_MAX))
    {
        plugin->mfx_comboboxes[(port - EUPHORIA_MONO_FX2_COMBOBOX_PORT_RANGE_MIN)][2] = data;
    }
    //MonoFX3
    else if((port >= EUPHORIA_MONO_FX3_KNOB0_PORT_RANGE_MIN) &&
            (port < EUPHORIA_MONO_FX3_KNOB0_PORT_RANGE_MAX))
    {
        plugin->mfx_knobs[(port - EUPHORIA_MONO_FX3_KNOB0_PORT_RANGE_MIN)][3][0] = data;
    }
    else if((port >= EUPHORIA_MONO_FX3_KNOB1_PORT_RANGE_MIN) &&
            (port < EUPHORIA_MONO_FX3_KNOB1_PORT_RANGE_MAX))
    {
        plugin->mfx_knobs[(port - EUPHORIA_MONO_FX3_KNOB1_PORT_RANGE_MIN)][3][1] = data;
    }
    else if((port >= EUPHORIA_MONO_FX3_KNOB2_PORT_RANGE_MIN) &&
            (port < EUPHORIA_MONO_FX3_KNOB2_PORT_RANGE_MAX))
    {
        plugin->mfx_knobs[(port - EUPHORIA_MONO_FX3_KNOB2_PORT_RANGE_MIN)][3][2] = data;
    }
    else if((port >= EUPHORIA_MONO_FX3_COMBOBOX_PORT_RANGE_MIN) &&
            (port < EUPHORIA_MONO_FX3_COMBOBOX_PORT_RANGE_MAX))
    {
        plugin->mfx_comboboxes[(port - EUPHORIA_MONO_FX3_COMBOBOX_PORT_RANGE_MIN)][3] = data;
    }

    else if((port >= EUPHORIA_SAMPLE_MONO_FX_GROUP_PORT_RANGE_MIN) &&
            (port < EUPHORIA_SAMPLE_MONO_FX_GROUP_PORT_RANGE_MAX))
    {
        plugin->sample_mfx_groups[(port - EUPHORIA_SAMPLE_MONO_FX_GROUP_PORT_RANGE_MIN)] = data;
    }
    else if(port >= EUPHORIA_NOISE_AMP_MIN && port < EUPHORIA_NOISE_AMP_MAX)
    {
        plugin->noise_amp[(port - EUPHORIA_NOISE_AMP_MIN)] = data;
    }
    else if(port >= EUPHORIA_NOISE_TYPE_MIN && port < EUPHORIA_NOISE_TYPE_MAX)
    {
        plugin->noise_type[(port - EUPHORIA_NOISE_TYPE_MIN)] = data;
    }
    else if(port >= EUPHORIA_SAMPLE_FADE_IN_MIN &&
            port < EUPHORIA_SAMPLE_FADE_IN_MAX)
    {
        plugin->sampleFadeInEnds[(port - EUPHORIA_SAMPLE_FADE_IN_MIN)] = data;
    }
    else if(port >= EUPHORIA_SAMPLE_FADE_OUT_MIN &&
            port < EUPHORIA_SAMPLE_FADE_OUT_MAX)
    {
        plugin->sampleFadeOutStarts[(port - EUPHORIA_SAMPLE_FADE_OUT_MIN)] = data;
    }
    else if(port >= EUPHORIA_FIRST_EQ_PORT &&
            port < EUPHORIA_LAST_EQ_PORT)
    {
        int f_port = port - EUPHORIA_FIRST_EQ_PORT;
        int f_instance = f_port / 18;
        int f_diff = f_port % 18;
        v_eq6_connect_port(plugin->mono_modules->eqs[f_instance], f_diff, data);
    }
    else if(port == EUPHORIA_LFO_PITCH_FINE)
    {
        plugin->lfo_pitch_fine = data;
    }
    else if(port == EUPHORIA_MIN_NOTE)
    {
        plugin->min_note = data;
    }
    else if(port == EUPHORIA_MAX_NOTE)
    {
        plugin->max_note = data;
    }
}

static PYFX_Handle instantiateSampler(PYFX_Descriptor * descriptor,
        int s_rate,
        fp_get_wavpool_item_from_host a_host_wavpool_func,
        int a_track_num, fp_queue_message a_queue_func)
{
    wavpool_get_func = a_host_wavpool_func;
    t_euphoria *plugin_data; // = (Sampler *) malloc(sizeof(Sampler));

    if(posix_memalign((void**)&plugin_data, 16, sizeof(t_euphoria)) != 0)
    {
        return NULL;
    }

    //pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

    plugin_data->voices = g_voc_get_voices(EUPHORIA_POLYPHONY,
            EUPHORIA_POLYPHONY_THRESH);

    plugin_data->track_num = a_track_num;

    plugin_data->i_selected_sample = 0;
    plugin_data->current_sample = 0;
    plugin_data->loaded_samples_count = 0;
    plugin_data->cubic_interpolator = g_cubic_get();
    plugin_data->linear_interpolator = g_lin_get();
    plugin_data->amp = 1.0f;
    plugin_data->i_slow_index = 0;

    plugin_data->smp_pit_core = g_pit_get();
    plugin_data->smp_pit_ratio = g_pit_ratio();

    int f_i = 0;
    while(f_i < EUPHORIA_MAX_SAMPLE_COUNT)
    {
        plugin_data->sampleStarts[f_i] = 0;
        plugin_data->sampleEnds[f_i] = 0;
        plugin_data->basePitch[f_i] = 0;
        plugin_data->low_note[f_i] = 0;
        plugin_data->high_note[f_i] = 0;
        plugin_data->sample_vol[f_i] = 0;
        plugin_data->sample_amp[f_i] = 1.0f;
        plugin_data->sampleEndPos[f_i] = 0.0f;
        plugin_data->sample_last_interpolated_value[f_i] = 0.0f;
        plugin_data->adjusted_base_pitch[f_i] = 60.0f;
        plugin_data->wavpool_items[f_i] = 0;

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

    plugin_data->sampleRate = s_rate;
    plugin_data->sv_pitch_bend_value = 0.0f;
    plugin_data->sv_last_note = -1.0f;
    plugin_data->channels = 2;
    plugin_data->amp_ptr = g_amp_get();
    plugin_data->mono_modules = g_euphoria_mono_init(s_rate);
    plugin_data->fs = s_rate;

    return (PYFX_Handle) plugin_data;
}

static void v_euphoria_activate(PYFX_Handle instance, float * a_port_table)
{
    t_euphoria *plugin_data = (t_euphoria *) instance;

    plugin_data->port_table = a_port_table;
    int i;

    plugin_data->sampleNo = 0;

    for (i = 0; i < EUPHORIA_POLYPHONY; i++)
    {
	plugin_data->velocities[i] = 0;
    }

    v_euphoria_slow_index(plugin_data);
}


//For the per-sample interpolation modes
typedef int (*fp_calculate_ratio)(t_euphoria *__restrict plugin_data, int n);
typedef void (*fp_run_sampler_interpolation)(t_euphoria *__restrict plugin_data,
        int n, int ch);

static fp_calculate_ratio ratio_function_ptrs[EUPHORIA_MAX_SAMPLE_COUNT];
static fp_run_sampler_interpolation interpolation_modes[EUPHORIA_MAX_SAMPLE_COUNT];

static inline int check_sample_bounds(t_euphoria *__restrict plugin_data, int n)
{
    if ((plugin_data->sample_read_heads[n][(plugin_data->current_sample)]->whole_number) >=
            plugin_data->sampleEndPos[(plugin_data->current_sample)])
    {
        if(((int)(*(plugin_data->sampleLoopModes[(plugin_data->current_sample)]))) > 0)
        {
            //TODO:  write a special function that either maintains the fraction, or
            //else wraps the negative interpolation back to where it was before the loop happened, to avoid clicks and pops
            v_ifh_retrigger(plugin_data->sample_read_heads[n][(plugin_data->current_sample)],
                    (EUPHORIA_SINC_INTERPOLATION_POINTS_DIV2 +
                    (plugin_data->sampleLoopStartPos[(plugin_data->current_sample)])));// 0.0f;

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
            plugin_data->wavpool_items[(plugin_data->current_sample)]->ratio_orig;

    v_ifh_run(plugin_data->sample_read_heads[n][(plugin_data->current_sample)],
            (plugin_data->ratio));

    return check_sample_bounds(plugin_data, n);
}

static int calculate_ratio_linear(t_euphoria *__restrict plugin_data, int n)
{
    return calculate_ratio_sinc(plugin_data, n);
}

static int calculate_ratio_none(t_euphoria *__restrict plugin_data, int n)
{
    plugin_data->sample_read_heads[n][(plugin_data->current_sample)]->whole_number =
            (plugin_data->sample_read_heads[n][(plugin_data->current_sample)]->whole_number) + 1;

    return check_sample_bounds(plugin_data, n);
}

static void run_sampler_interpolation_sinc(t_euphoria *__restrict plugin_data, int n, int ch)
{
    plugin_data->sample_last_interpolated_value[(plugin_data->current_sample)] =
        f_sinc_interpolate2(plugin_data->mono_modules->sinc_interpolator,
        plugin_data->wavpool_items[(plugin_data->current_sample)]->samples[ch],
        (plugin_data->sample_read_heads[n][(plugin_data->current_sample)]->whole_number),
        (plugin_data->sample_read_heads[n][(plugin_data->current_sample)]->fraction));
}


static void run_sampler_interpolation_linear(t_euphoria *__restrict plugin_data, int n, int ch)
{
    plugin_data->sample_last_interpolated_value[(plugin_data->current_sample)] =
        f_cubic_interpolate_ptr_ifh(
            plugin_data->wavpool_items[(plugin_data->current_sample)]->samples[ch],
            (plugin_data->sample_read_heads[n][(plugin_data->current_sample)]->whole_number),
            (plugin_data->sample_read_heads[n][(plugin_data->current_sample)]->fraction),
            plugin_data->cubic_interpolator);
}


static void run_sampler_interpolation_none(t_euphoria *__restrict plugin_data, int n, int ch)
{
    plugin_data->sample_last_interpolated_value[(plugin_data->current_sample)] =
            plugin_data->wavpool_items[(plugin_data->current_sample)]->
            samples[ch][(plugin_data->sample_read_heads[n][(plugin_data->current_sample)]->whole_number)];
}

/* void add_sample_lms_euphoria(t_euphoria *__restrict plugin_data,
 *                                                      int n) //voice number
*/
static void add_sample_lms_euphoria(t_euphoria *__restrict plugin_data, int n)
{
    if((plugin_data->voices->voices[n].on) > (plugin_data->sampleNo))
    {
        return;
    }

    int ch;

    t_euphoria_poly_voice * f_voice = plugin_data->data[n];

    //Run things that aren't per-channel like envelopes

    v_adsr_run_db(f_voice->adsr_amp);

    if(f_voice->adsr_amp->stage == ADSR_STAGE_OFF)
    {
        plugin_data->voices->voices[n].n_state = note_state_off;
        return;
    }

    v_adsr_run(f_voice->adsr_filter);

    //Run the glide module
    f_rmp_run_ramp(f_voice->ramp_env);
    f_rmp_run_ramp(f_voice->glide_env);

    //Set and run the LFO
    v_lfs_set(f_voice->lfo1,  (*(plugin_data->lfo_freq)) * .01);
    v_lfs_run(f_voice->lfo1);

    f_voice->base_pitch = (f_voice->glide_env->output_multiplied)
            +  (plugin_data->mono_modules->pitchbend_smoother->last_value *
            (*(plugin_data->master_pb_amt)))
            + (f_voice->last_pitch) + ((f_voice->lfo1->output) *
            (*plugin_data->lfo_pitch + (*plugin_data->lfo_pitch_fine * 0.01f)));

    if((plugin_data->voices->voices[n].off == plugin_data->sampleNo) &&
        (f_voice->adsr_amp->stage < ADSR_STAGE_RELEASE))
    {
        if(plugin_data->voices->voices[n].n_state == note_state_killed)
        {
            v_euphoria_poly_note_off(f_voice, 1);
        }
        else
        {
            v_euphoria_poly_note_off(f_voice, 0);
        }
    }

    plugin_data->sample[0] = 0.0f;
    plugin_data->sample[1] = 0.0f;
    f_voice->modulex_current_sample[0] = 0.0f;
    f_voice->modulex_current_sample[1] = 0.0f;

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

        float f_fade_vol = 1.0f;

        if(plugin_data->sample_read_heads[n][plugin_data->current_sample]->whole_number <
           f_voice->sample_fade_in_end_sample[plugin_data->current_sample])
        {
            float f_fade_in_inc =
                f_voice->sample_fade_in_inc[plugin_data->current_sample];
            float f_start_pos =
                (plugin_data->sampleStartPos[plugin_data->current_sample]);
            float f_read_head_pos =
                (float)(plugin_data->sample_read_heads[n][plugin_data->current_sample]->whole_number);

            f_fade_vol =  (f_read_head_pos - f_start_pos) * f_fade_in_inc;
            f_fade_vol = (f_fade_vol * 18.0f) - 18.0f;
            f_fade_vol = f_db_to_linear_fast(f_fade_vol, plugin_data->amp_ptr);
        }
        else if(plugin_data->sample_read_heads[n][plugin_data->current_sample]->whole_number >
                f_voice->sample_fade_out_start_sample[plugin_data->current_sample])
        {
            float f_sample_end_pos =
                plugin_data->sampleEndPos[plugin_data->current_sample];
            float f_read_head_pos =
                (float)(plugin_data->sample_read_heads[n][plugin_data->current_sample]->whole_number);
            float f_fade_out_dec = f_voice->sample_fade_out_dec[plugin_data->current_sample];

            f_fade_vol = (f_sample_end_pos - f_read_head_pos) * f_fade_out_dec;
            f_fade_vol = (f_fade_vol * 18.0f) - 18.0f;
            f_fade_vol = f_db_to_linear_fast(f_fade_vol, plugin_data->amp_ptr);
        }

        f_voice->noise_sample =
                ((plugin_data->mono_modules->noise_func_ptr[(plugin_data->current_sample)](
                plugin_data->mono_modules->white_noise1[(f_voice->noise_index)]))
                * (plugin_data->mono_modules->noise_linamp[(plugin_data->current_sample)])); //add noise

        for (ch = 0; ch < (plugin_data->wavpool_items[(plugin_data->current_sample)]->channels); ++ch)
        {
            interpolation_modes[(plugin_data->current_sample)](plugin_data, n, ch);

            plugin_data->sample[ch] +=
                plugin_data->sample_last_interpolated_value[(plugin_data->current_sample)] * f_fade_vol;

            plugin_data->sample[ch] += (f_voice->noise_sample);

            plugin_data->sample[ch] = (plugin_data->sample[ch]) *
                    (plugin_data->sample_amp[(plugin_data->current_sample)]);

            f_voice->modulex_current_sample[ch] += (plugin_data->sample[ch]);


            if((plugin_data->wavpool_items[(plugin_data->current_sample)]->channels) == 1)
            {
                f_voice->modulex_current_sample[1] += plugin_data->sample[0];
                break;
            }
        }

        plugin_data->i_loaded_samples = (plugin_data->i_loaded_samples) + 1;
    }

    //Modular PolyFX, processed from the index created during note_on
    for(plugin_data->i_dst = 0; (plugin_data->i_dst) < (plugin_data->active_polyfx_count[n]);
            plugin_data->i_dst = (plugin_data->i_dst) + 1)
    {
        v_mf3_set(f_voice->multieffect[(plugin_data->active_polyfx[n][(plugin_data->i_dst)])],
            *(plugin_data->pfx_mod_knob[0][(plugin_data->active_polyfx[n][(plugin_data->i_dst)])][0]),
                *(plugin_data->pfx_mod_knob[0][(plugin_data->active_polyfx[n][(plugin_data->i_dst)])][1]),
                *(plugin_data->pfx_mod_knob[0][(plugin_data->active_polyfx[n][(plugin_data->i_dst)])][2]));

        int f_mod_test;

        for(f_mod_test = 0;
            f_mod_test < (plugin_data->polyfx_mod_counts[n][(plugin_data->active_polyfx[n][(plugin_data->i_dst)])]);
            f_mod_test++)
        {
            v_mf3_mod_single(
                    f_voice->multieffect[(plugin_data->active_polyfx[n][(plugin_data->i_dst)])],
                    *(f_voice->modulator_outputs[(plugin_data->polyfx_mod_src_index[n][(plugin_data->active_polyfx[n][(plugin_data->i_dst)])][f_mod_test])]),
                    (plugin_data->polyfx_mod_matrix_values[n][(plugin_data->active_polyfx[n][(plugin_data->i_dst)])][f_mod_test]),
                    (plugin_data->polyfx_mod_ctrl_indexes[n][(plugin_data->active_polyfx[n][(plugin_data->i_dst)])][f_mod_test])
                    );
        }

        f_voice->fx_func_ptr[(plugin_data->active_polyfx[n][(plugin_data->i_dst)])](
                f_voice->multieffect[(plugin_data->active_polyfx[n][(plugin_data->i_dst)])],
                (f_voice->modulex_current_sample[0]),
                (f_voice->modulex_current_sample[1]));

        f_voice->modulex_current_sample[0] = f_voice->multieffect[(plugin_data->active_polyfx[n][(plugin_data->i_dst)])]->output0;
        f_voice->modulex_current_sample[1] = f_voice->multieffect[(plugin_data->active_polyfx[n][(plugin_data->i_dst)])]->output1;
    }

    plugin_data->mono_fx_buffers[(plugin_data->sample_mfx_groups_index[(plugin_data->current_sample)])][0] +=
            (f_voice->modulex_current_sample[0]) * (f_voice->adsr_amp->output) *
            (plugin_data->amp);
    plugin_data->mono_fx_buffers[(plugin_data->sample_mfx_groups_index[(plugin_data->current_sample)])][1] +=
            (f_voice->modulex_current_sample[1]) * (f_voice->adsr_amp->output) *
            (plugin_data->amp);
}

static inline void v_euphoria_slow_index(t_euphoria* plugin_data)
{
    plugin_data->i_slow_index = 0;
    plugin_data->monofx_channel_index_count = 0;

    int i, i3;

    for(i = 0; i < EUPHORIA_MONO_FX_GROUPS_COUNT; i++)
    {
        plugin_data->monofx_channel_index_tracker[i] = 0;
    }

    for(i = 0; i  < (plugin_data->loaded_samples_count); i++)
    {
        int f_mono_fx_group = ((int)(*(plugin_data->sample_mfx_groups[(plugin_data->loaded_samples[i])])));

        if((plugin_data->monofx_channel_index_tracker[f_mono_fx_group]) == 0)
        {
            plugin_data->monofx_channel_index_tracker[f_mono_fx_group] = 1;
            plugin_data->monofx_channel_index[(plugin_data->monofx_channel_index_count)] =
                    (int)(*(plugin_data->sample_mfx_groups[(plugin_data->loaded_samples[i])]));
            plugin_data->monofx_channel_index_count = (plugin_data->monofx_channel_index_count) + 1;

            for(i3 = 0; i3 < EUPHORIA_MONO_FX_COUNT; i3++)
            {
                plugin_data->mono_modules->fx_func_ptr[f_mono_fx_group][i3] =
                        g_mf3_get_function_pointer((int)(*(plugin_data->mfx_comboboxes[f_mono_fx_group][i3])));
            }
        }

        int f_index = (int)*(plugin_data->noise_type[(plugin_data->loaded_samples[i])]);
        //Get the noise function pointer
        plugin_data->mono_modules->noise_func_ptr[(plugin_data->loaded_samples[i])] = fp_get_noise_func_ptr(f_index);

        if(f_index > 0)
        {
            plugin_data->mono_modules->noise_linamp[(plugin_data->loaded_samples[i])] =
                    f_db_to_linear_fast(*(plugin_data->noise_amp[(plugin_data->loaded_samples[i])]), plugin_data->mono_modules->amp_ptr);
        }
    }
}

static void v_run_lms_euphoria(PYFX_Handle instance, int sample_count,
		       t_pydaw_seq_event *events, int event_count)
{
    t_euphoria *plugin_data = (t_euphoria *) instance;
    int event_pos = 0;
    int midi_event_pos = 0;
    int i, i2, i3;

    plugin_data->i_slow_index = (plugin_data->i_slow_index) + 1;

    plugin_data->midi_event_count = 0;

    if((plugin_data->i_slow_index) >= EUPHORIA_SLOW_INDEX_COUNT)
    {
        v_euphoria_slow_index(plugin_data);
    }

    int f_note = 60;

    int f_min_note = (int)*plugin_data->min_note;
    int f_max_note = (int)*plugin_data->max_note;

    while (event_pos < event_count) // && pos >= events[event_pos].time.tick)
    {
        /*Note-on event*/
        if (events[event_pos].type == PYDAW_EVENT_NOTEON)
        {
            f_note = events[event_pos].note;

            if (events[event_pos].velocity > 0)
            {
                if(events[event_pos].note > f_max_note ||
                    events[event_pos].note < f_min_note)
                {
                    event_pos++;
                    continue;
                }
                int f_voice_num = i_pick_voice(plugin_data->voices, f_note, (plugin_data->sampleNo), events[event_pos].tick);
                plugin_data->velocities[f_voice_num] = events[event_pos].velocity;

                plugin_data->data[f_voice_num]->keyboard_track = ((float)(events[(event_pos)].note)) * 0.007874016f;
                plugin_data->data[f_voice_num]->velocity_track = ((float)(events[(event_pos)].velocity)) * 0.007874016f;

                plugin_data->sample_indexes_count[f_voice_num] = 0;

                //Figure out which samples to play and stash all relevant values
                for(i = 0; i  < (plugin_data->loaded_samples_count); i++)
                {
                    if((f_note >= ((int)(*(plugin_data->low_note[(plugin_data->loaded_samples[i])])))) &&
                    (f_note <= ((int)(*(plugin_data->high_note[(plugin_data->loaded_samples[i])])))) &&
                    (plugin_data->velocities[f_voice_num] <=
                        ((int)(*(plugin_data->sample_vel_high[(plugin_data->loaded_samples[i])])))) &&
                    (plugin_data->velocities[f_voice_num] >=
                            ((int)(*(plugin_data->sample_vel_low[(plugin_data->loaded_samples[i])])))))
                    {
                        plugin_data->sample_indexes[f_voice_num][(plugin_data->sample_indexes_count[f_voice_num])] = (plugin_data->loaded_samples[i]);
                        plugin_data->sample_indexes_count[f_voice_num] = (plugin_data->sample_indexes_count[f_voice_num]) + 1;

                        plugin_data->sample_mfx_groups_index[(plugin_data->loaded_samples[i])] =
                                (int)(*(plugin_data->sample_mfx_groups[(plugin_data->loaded_samples[i])]));

                        plugin_data->sampleStartPos[(plugin_data->loaded_samples[i])] = (EUPHORIA_SINC_INTERPOLATION_POINTS_DIV2 +
                                ((plugin_data->wavpool_items[(plugin_data->loaded_samples[i])]->length) *
                                ((*(plugin_data->sampleStarts[(plugin_data->loaded_samples[i])])) * .001)));

                        plugin_data->sampleLoopStartPos[(plugin_data->loaded_samples[i])] = (EUPHORIA_SINC_INTERPOLATION_POINTS_DIV2 +
                                ((plugin_data->wavpool_items[(plugin_data->loaded_samples[i])]->length) *
                                ((*(plugin_data->sampleLoopStarts[(plugin_data->loaded_samples[i])])) * .001)));

                        /* If loop mode is enabled for this sample, set the sample end to be the same as the
                           loop end.  Then, in the main loop, we'll recalculate sample_end to be the real sample end once
                           the note_off event is fired.  Doing it this way greatly reduces the need for extra if-then-else logic
                           in the main loop */
                        if(((int)(*(plugin_data->sampleLoopModes[(plugin_data->loaded_samples[i])]))) == 0)
                        {
                            plugin_data->sampleEndPos[(plugin_data->loaded_samples[i])] = (EUPHORIA_SINC_INTERPOLATION_POINTS_DIV2 +
                                    ((int)(((float)((plugin_data->wavpool_items[(plugin_data->loaded_samples[i])]->length) - 5)) *
                                    ((*(plugin_data->sampleEnds[(plugin_data->loaded_samples[i])])) * .001))));
                        }
                        else
                        {
                            plugin_data->sampleEndPos[(plugin_data->loaded_samples[i])] = (EUPHORIA_SINC_INTERPOLATION_POINTS_DIV2 +
                                    ((int)(((float)((plugin_data->wavpool_items[(plugin_data->loaded_samples[i])]->length) - 5)) *
                                    ((*(plugin_data->sampleLoopEnds[(plugin_data->loaded_samples[i])])) * .001))));
                        }

                        if((plugin_data->sampleEndPos[(plugin_data->loaded_samples[i])]) >
                                ((float)((plugin_data->wavpool_items[(plugin_data->loaded_samples[i])]->length))))
                        {
                            plugin_data->sampleEndPos[(plugin_data->loaded_samples[i])] =
                                    (float)(plugin_data->wavpool_items[(plugin_data->loaded_samples[i])]->length);
                        }

                        //get the fade in values
                        plugin_data->data[f_voice_num]->sample_fade_in_end_sample[(plugin_data->loaded_samples[i])] =
                                (int)((*plugin_data->sampleFadeInEnds[(plugin_data->loaded_samples[i])]) * 0.001f *
                                (float)(plugin_data->wavpool_items[(plugin_data->loaded_samples[i])]->length));

                        if(plugin_data->data[f_voice_num]->sample_fade_in_end_sample[(plugin_data->loaded_samples[i])] <
                                (plugin_data->sampleStartPos[(plugin_data->loaded_samples[i])]))
                        {
                            plugin_data->data[f_voice_num]->sample_fade_in_end_sample[(plugin_data->loaded_samples[i])] =
                                (plugin_data->sampleStartPos[(plugin_data->loaded_samples[i])]);
                        }
                        else if(plugin_data->data[f_voice_num]->sample_fade_in_end_sample[(plugin_data->loaded_samples[i])] >
                                (plugin_data->sampleEndPos[(plugin_data->loaded_samples[i])]))
                        {
                            plugin_data->data[f_voice_num]->sample_fade_in_end_sample[(plugin_data->loaded_samples[i])] =
                                (plugin_data->sampleEndPos[(plugin_data->loaded_samples[i])]);
                        }

                        if(plugin_data->data[f_voice_num]->sample_fade_in_end_sample[(plugin_data->loaded_samples[i])] >
                                (plugin_data->sampleStartPos[(plugin_data->loaded_samples[i])]))
                        {
                                plugin_data->data[f_voice_num]->sample_fade_in_inc[(plugin_data->loaded_samples[i])] = 1.0f /
                                    (plugin_data->data[f_voice_num]->sample_fade_in_end_sample[(plugin_data->loaded_samples[i])] -
                                        (plugin_data->sampleStartPos[(plugin_data->loaded_samples[i])]));
                        }
                        else
                        {
                            plugin_data->data[f_voice_num]->sample_fade_in_inc[(plugin_data->loaded_samples[i])] = 1.0f;
                        }

                        //get the fade out values
                        plugin_data->data[f_voice_num]->sample_fade_out_start_sample[(plugin_data->loaded_samples[i])] =
                                (int)((*plugin_data->sampleFadeOutStarts[(plugin_data->loaded_samples[i])]) * 0.001f *
                                (float)(plugin_data->wavpool_items[(plugin_data->loaded_samples[i])]->length));

                        if(plugin_data->data[f_voice_num]->sample_fade_out_start_sample[(plugin_data->loaded_samples[i])] <
                                (plugin_data->sampleStartPos[(plugin_data->loaded_samples[i])]))
                        {
                            plugin_data->data[f_voice_num]->sample_fade_out_start_sample[(plugin_data->loaded_samples[i])] =
                                (plugin_data->sampleStartPos[(plugin_data->loaded_samples[i])]);
                        }
                        else if(plugin_data->data[f_voice_num]->sample_fade_out_start_sample[(plugin_data->loaded_samples[i])] >
                                (plugin_data->sampleEndPos[(plugin_data->loaded_samples[i])]))
                        {
                            plugin_data->data[f_voice_num]->sample_fade_out_start_sample[(plugin_data->loaded_samples[i])] =
                                (plugin_data->sampleEndPos[(plugin_data->loaded_samples[i])]);
                        }

                        if(plugin_data->data[f_voice_num]->sample_fade_out_start_sample[(plugin_data->loaded_samples[i])] <
                                (plugin_data->sampleEndPos[(plugin_data->loaded_samples[i])]))
                        {
                                plugin_data->data[f_voice_num]->sample_fade_out_dec[(plugin_data->loaded_samples[i])] = 1.0f /
                                    ((plugin_data->sampleEndPos[(plugin_data->loaded_samples[i])]) -
                                    plugin_data->data[f_voice_num]->sample_fade_out_start_sample[(plugin_data->loaded_samples[i])]);
                        }
                        else
                        {
                            plugin_data->data[f_voice_num]->sample_fade_out_dec[(plugin_data->loaded_samples[i])] = 1.0f;
                        }


                        //end fade stuff

                        plugin_data->adjusted_base_pitch[(plugin_data->loaded_samples[i])] =
                                (*(plugin_data->basePitch[(plugin_data->loaded_samples[i])]))
                                - (*(plugin_data->sample_pitch[(plugin_data->loaded_samples[i])])) -
                                ((*(plugin_data->sample_tune[(plugin_data->loaded_samples[i])])) * .01f);

                        v_ifh_retrigger(plugin_data->sample_read_heads[f_voice_num][(plugin_data->loaded_samples[i])],
                                (plugin_data->sampleStartPos[(plugin_data->loaded_samples[i])]));// 0.0f;

                        plugin_data->vel_sens_output[f_voice_num][(plugin_data->loaded_samples[i])] =
                                (1.0f -
                                (((float)(events[event_pos].velocity) -
                                (*(plugin_data->sample_vel_low[(plugin_data->loaded_samples[i])])))
                                /
                                ((float)(*(plugin_data->sample_vel_high[(plugin_data->loaded_samples[i])]) -
                                (*(plugin_data->sample_vel_low[(plugin_data->loaded_samples[i])]))))))
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
                                printf("Error, invalid interpolation mode %i\n",
                                        ((int)(*(plugin_data->sample_interpolation_mode[(plugin_data->loaded_samples[i])]))));
                        }
                    }
                }

                plugin_data->active_polyfx_count[f_voice_num] = 0;
                //Determine which PolyFX have been enabled
                for(plugin_data->i_dst = 0;
                    (plugin_data->i_dst) < EUPHORIA_MODULAR_POLYFX_COUNT;
                        plugin_data->i_dst = (plugin_data->i_dst) + 1)
                {
                    int f_pfx_combobox_index =
                        (int)(*(plugin_data->fx_combobox[0][(plugin_data->i_dst)]));
                    plugin_data->data[f_voice_num]->fx_func_ptr[(plugin_data->i_dst)] =
                            g_mf3_get_function_pointer(f_pfx_combobox_index);
                    plugin_data->data[f_voice_num]->fx_reset_ptr[(plugin_data->i_dst)] =
                            g_mf3_get_reset_function_pointer(f_pfx_combobox_index);

                    plugin_data->data[f_voice_num]->fx_reset_ptr[(plugin_data->i_dst)](
                        plugin_data->data[f_voice_num]->multieffect[(plugin_data->i_dst)]);

                    if(f_pfx_combobox_index != 0)
                    {
                        plugin_data->active_polyfx[f_voice_num][(plugin_data->active_polyfx_count[f_voice_num])] = (plugin_data->i_dst);
                        plugin_data->active_polyfx_count[f_voice_num] = (plugin_data->active_polyfx_count[f_voice_num]) + 1;
                    }
                }

                //Calculate an index of which mod_matrix controls to process.  This saves expensive iterations and if/then logic in the main loop
                for(plugin_data->i_fx_grps = 0;
                    (plugin_data->i_fx_grps) < EUPHORIA_EFFECTS_GROUPS_COUNT;
                    plugin_data->i_fx_grps = (plugin_data->i_fx_grps) + 1)
                {
                    for(plugin_data->i_dst = 0;
                        (plugin_data->i_dst) < (plugin_data->active_polyfx_count[f_voice_num]);
                        plugin_data->i_dst = (plugin_data->i_dst) + 1)
                    {
                        plugin_data->polyfx_mod_counts[f_voice_num][(plugin_data->active_polyfx[f_voice_num][(plugin_data->i_dst)])] = 0;

                        for(plugin_data->i_src = 0;
                            (plugin_data->i_src) < EUPHORIA_MODULATOR_COUNT;
                            plugin_data->i_src = (plugin_data->i_src) + 1)
                        {
                            for(plugin_data->i_ctrl = 0;
                                (plugin_data->i_ctrl) < EUPHORIA_CONTROLS_PER_MOD_EFFECT;
                                plugin_data->i_ctrl = (plugin_data->i_ctrl) + 1)
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

                plugin_data->data[f_voice_num]->noise_index =
                        (plugin_data->mono_modules->noise_current_index);
                plugin_data->mono_modules->noise_current_index =
                        (plugin_data->mono_modules->noise_current_index) + 1;

                if((plugin_data->mono_modules->noise_current_index) >=
                        EUPHORIA_NOISE_COUNT)
                {
                    plugin_data->mono_modules->noise_current_index = 0;
                }

                plugin_data->amp =
                        f_db_to_linear_fast(*(plugin_data->master_vol),
                        plugin_data->mono_modules->amp_ptr);

                plugin_data->data[f_voice_num]->note_f = (float)f_note;

                plugin_data->data[f_voice_num]->target_pitch =
                        (plugin_data->data[f_voice_num]->note_f);

                if(plugin_data->sv_last_note < 0.0f)
                {
                    plugin_data->data[f_voice_num]->last_pitch =
                        (plugin_data->data[f_voice_num]->note_f);
                }
                else
                {
                    plugin_data->data[f_voice_num]->last_pitch =
                            (plugin_data->sv_last_note);
                }

                v_rmp_retrigger_glide_t(
                        plugin_data->data[f_voice_num]->glide_env,
                        (*(plugin_data->master_glide) * .01),
                        (plugin_data->data[f_voice_num]->last_pitch),
                        (plugin_data->data[f_voice_num]->target_pitch));

                /*Retrigger ADSR envelopes and LFO*/
                v_adsr_retrigger(plugin_data->data[f_voice_num]->adsr_amp);
                v_adsr_retrigger(plugin_data->data[f_voice_num]->adsr_filter);
                v_lfs_sync(plugin_data->data[f_voice_num]->lfo1, 0.0f, *(plugin_data->lfo_type));

                float f_attack_a = (*(plugin_data->attack) * .01);
                f_attack_a *= f_attack_a;
                float f_decay_a = (*(plugin_data->decay) * .01);
                f_decay_a *= f_decay_a;
                float f_release_a = (*(plugin_data->release) * .01);
                f_release_a *= f_release_a;
                v_adsr_set_adsr_db(plugin_data->data[f_voice_num]->adsr_amp,
                        f_attack_a, f_decay_a, (*(plugin_data->sustain)),
                        f_release_a);

                float f_attack_f = (*(plugin_data->attack_f) * .01);
                f_attack_f *= f_attack_f;
                float f_decay_f = (*(plugin_data->decay_f) * .01);
                f_decay_f *= f_decay_f;
                float f_release_f = (*(plugin_data->release_f) * .01);
                f_release_f *= f_release_f;
                v_adsr_set_adsr(plugin_data->data[f_voice_num]->adsr_filter,
                        f_attack_f, f_decay_f,
                        (*(plugin_data->sustain_f) * .01), f_release_f);

                /*Retrigger the pitch envelope*/
                v_rmp_retrigger((plugin_data->data[f_voice_num]->ramp_env), (*(plugin_data->pitch_env_time) * .01), 1.0f);

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
                v_voc_note_off(plugin_data->voices, events[event_pos].note,
                        plugin_data->sampleNo, events[event_pos].tick);
            }
        }
        else if (events[event_pos].type == PYDAW_EVENT_NOTEOFF )
        {
            f_note = events[event_pos].note;
            v_voc_note_off(plugin_data->voices, events[event_pos].note,
                    plugin_data->sampleNo, events[event_pos].tick);
        }
        else if (events[event_pos].type == PYDAW_EVENT_CONTROLLER)
        {
            if (events[event_pos].plugin_index != -1) //The host already filters out messages for other instruments
            {
                assert(events[event_pos].port >= EUPHORIA_FIRST_CONTROL_PORT &&
                        events[event_pos].port < EUPHORIA_PORT_COUNT);

                plugin_data->midi_event_types[plugin_data->midi_event_count] = PYDAW_EVENT_CONTROLLER;
                plugin_data->midi_event_ticks[plugin_data->midi_event_count] = events[event_pos].tick;
                plugin_data->midi_event_ports[plugin_data->midi_event_count] = events[event_pos].port;
                plugin_data->midi_event_values[plugin_data->midi_event_count] = events[event_pos].value;
                plugin_data->midi_event_count++;
            }
        }
        else if (events[event_pos].type == PYDAW_EVENT_PITCHBEND)
        {
            plugin_data->midi_event_types[plugin_data->midi_event_count] = PYDAW_EVENT_PITCHBEND;
            plugin_data->midi_event_ticks[plugin_data->midi_event_count] = events[event_pos].tick;
            plugin_data->midi_event_values[plugin_data->midi_event_count] =
                    0.00012207 * events[event_pos].value;
            plugin_data->midi_event_count++;
        }

        ++event_pos;
    }

    float f_temp_sample0, f_temp_sample1;

    int f_monofx_index = 0;

    for(i2 = 0; i2 < (plugin_data->monofx_channel_index_count); i2++)
    {
        f_monofx_index = (plugin_data->monofx_channel_index[i2]);
        v_eq6_set(plugin_data->mono_modules->eqs[f_monofx_index]);
    }

    for(i = 0; i < sample_count; i++)
    {
	plugin_data->output[0][i] = 0.0f;
        plugin_data->output[1][i] = 0.0f;

        while(midi_event_pos < plugin_data->midi_event_count &&
                plugin_data->midi_event_ticks[midi_event_pos] == i)
        {
            if(plugin_data->midi_event_types[midi_event_pos] == PYDAW_EVENT_PITCHBEND)
            {
                plugin_data->sv_pitch_bend_value = plugin_data->midi_event_values[midi_event_pos];
            }
            else if(plugin_data->midi_event_types[midi_event_pos] == PYDAW_EVENT_CONTROLLER)
            {
                plugin_data->port_table[plugin_data->midi_event_ports[midi_event_pos]] =
                        plugin_data->midi_event_values[midi_event_pos];
            }

            midi_event_pos++;
        }

        v_sml_run(plugin_data->mono_modules->pitchbend_smoother,
                (plugin_data->sv_pitch_bend_value));

        for(i2 = 0; i2 < (plugin_data->monofx_channel_index_count); i2++)
        {
            plugin_data->mono_fx_buffers[(plugin_data->monofx_channel_index[i2])][0] = 0.0f;
            plugin_data->mono_fx_buffers[(plugin_data->monofx_channel_index[i2])][1] = 0.0f;
        }

        for (i2 = 0; i2 < EUPHORIA_POLYPHONY; ++i2)
        {
            if(((plugin_data->data[i2]->adsr_amp->stage) != ADSR_STAGE_OFF) &&
                    ((plugin_data->sample_indexes_count[i2]) > 0))
            {
                add_sample_lms_euphoria(plugin_data, i2);
            }
        }



        for(i2 = 0; i2 < (plugin_data->monofx_channel_index_count); i2++)
        {
            f_monofx_index = (plugin_data->monofx_channel_index[i2]);

            f_temp_sample0 = (plugin_data->mono_fx_buffers[f_monofx_index][0]);
            f_temp_sample1 = (plugin_data->mono_fx_buffers[f_monofx_index][1]);

            for(i3 = 0; i3 < EUPHORIA_MONO_FX_COUNT; i3++)
            {
                v_mf3_set(plugin_data->mono_modules->multieffect[f_monofx_index][i3],
                        (*(plugin_data->mfx_knobs[f_monofx_index][i3][0])),
                        (*(plugin_data->mfx_knobs[f_monofx_index][i3][1])),
                        (*(plugin_data->mfx_knobs[f_monofx_index][i3][2])));
                plugin_data->mono_modules->fx_func_ptr[f_monofx_index][i3](
                    plugin_data->mono_modules->multieffect[f_monofx_index][i3],
                    f_temp_sample0, f_temp_sample1);

                f_temp_sample0 = (plugin_data->mono_modules->multieffect[f_monofx_index][i3]->output0);
                f_temp_sample1 = (plugin_data->mono_modules->multieffect[f_monofx_index][i3]->output1);
            }

            v_eq6_run(plugin_data->mono_modules->eqs[f_monofx_index],
                    f_temp_sample0, f_temp_sample1);

            plugin_data->output[0][i] +=
                    plugin_data->mono_modules->eqs[f_monofx_index]->output0;
            plugin_data->output[1][i] +=
                    plugin_data->mono_modules->eqs[f_monofx_index]->output1;
        }

        plugin_data->sampleNo++;
    }

    //plugin_data->sampleNo += sample_count;
    //pthread_mutex_unlock(&plugin_data->mutex);
}

static char *c_euphoria_load_all(t_euphoria *plugin_data, char *paths,
        pthread_mutex_t * a_mutex)
{
    int f_index = 0;
    int f_samples_loaded_count = 0;
    int f_current_string_index = 0;
    int f_total_index = 0;

    t_wav_pool_item * f_wavpool_items[EUPHORIA_MAX_SAMPLE_COUNT];
    int f_loaded_samples[EUPHORIA_MAX_SAMPLE_COUNT];

    int f_i = 0;
    while(f_i < EUPHORIA_MAX_SAMPLE_COUNT)
    {
        f_wavpool_items[f_i] = 0;
        f_loaded_samples[f_i] = 0;
        f_i++;
    }

    char * f_result_string = (char*)malloc(sizeof(char) * 2048);

    while (f_samples_loaded_count < EUPHORIA_MAX_SAMPLE_COUNT)
    {
        if(paths[f_index] == EUPHORIA_FILES_STRING_DELIMITER || paths[f_index] == '\0')
        {
            f_result_string[f_current_string_index] = '\0';

            if(f_current_string_index == 0)
            {
                f_wavpool_items[f_total_index] = 0;
            }
            else
            {
                int f_uid = atoi(f_result_string);
                t_wav_pool_item * f_wavpool_item = wavpool_get_func(f_uid);
                f_wavpool_items[f_total_index] = f_wavpool_item;
                f_loaded_samples[f_samples_loaded_count] = f_total_index;
                f_samples_loaded_count++;
            }

            f_current_string_index = 0;
            f_total_index++;

            if(paths[f_index] == '\0')
            {
                break;
            }
        }
        else
        {
            f_result_string[f_current_string_index] = paths[f_index];
            f_current_string_index++;
        }

        f_index++;
    }

    free(f_result_string);

    if(a_mutex)
    {
        pthread_mutex_lock(a_mutex);
    }

    f_i = 0;

    while(f_i < f_samples_loaded_count)
    {
        plugin_data->loaded_samples[f_i] = f_loaded_samples[f_i];
        f_i++;
    }

    plugin_data->loaded_samples_count = f_samples_loaded_count;

    f_i = 0;

    while(f_i < EUPHORIA_MAX_SAMPLE_COUNT)
    {
        plugin_data->wavpool_items[f_i] = f_wavpool_items[f_i];
        f_i++;
    }

    //Force a re-index before the next sample period
    plugin_data->i_slow_index = EUPHORIA_SLOW_INDEX_COUNT;

    if(a_mutex)
    {
        pthread_mutex_unlock(a_mutex);
    }

    return NULL;
}

char *c_euphoria_configure(PYFX_Handle instance, char *key,
        char *value, pthread_mutex_t * a_mutex)
{
    t_euphoria *plugin_data = (t_euphoria *)instance;

    if (!strcmp(key, "load"))
    {
        return c_euphoria_load_all(plugin_data, value, a_mutex);
    }

    return strdup("error: unrecognized configure key");
}

PYFX_Descriptor *euphoria_PYFX_descriptor(int index)
{
    PYFX_Descriptor *f_result =
        pydaw_get_pyfx_descriptor(99883366, "Euphoria", EUPHORIA_PORT_COUNT);

    pydaw_set_pyfx_port(f_result, EUPHORIA_SELECTED_SAMPLE, 0.0f, 0, (EUPHORIA_MAX_SAMPLE_COUNT - 1));
    pydaw_set_pyfx_port(f_result, EUPHORIA_ATTACK, 0.0f, 0.0f, 200.0f);
    pydaw_set_pyfx_port(f_result, EUPHORIA_DECAY, 50.0f, 10.0f, 200.0f);
    pydaw_set_pyfx_port(f_result, EUPHORIA_SUSTAIN, 0.0f, -60, 0);
    pydaw_set_pyfx_port(f_result, EUPHORIA_RELEASE, 50.0f, 10.0f, 400.0f);
    pydaw_set_pyfx_port(f_result, EUPHORIA_FILTER_ATTACK, 10.0f, 0.0f, 200.0f);
    pydaw_set_pyfx_port(f_result, EUPHORIA_FILTER_DECAY, 50.0f, 10.0f, 200.0f);
    pydaw_set_pyfx_port(f_result, EUPHORIA_FILTER_SUSTAIN, 100.0f, 0.0f, 100.0f);
    pydaw_set_pyfx_port(f_result, EUPHORIA_FILTER_RELEASE, 50.0f, 10.0f, 400.0f);
    pydaw_set_pyfx_port(f_result, EUPHORIA_MASTER_VOLUME, -6.0f, -24, 24);
    pydaw_set_pyfx_port(f_result, EUPHORIA_MASTER_GLIDE, 0.0f, 0.0f, 200.0f);
    pydaw_set_pyfx_port(f_result, EUPHORIA_MASTER_PITCHBEND_AMT, 18.0f, 1, 36);
    pydaw_set_pyfx_port(f_result, EUPHORIA_PITCH_ENV_TIME, 100.0f, 1.0f, 600.0f);
    pydaw_set_pyfx_port(f_result, EUPHORIA_LFO_FREQ, 200.0f, 10, 1600);
    pydaw_set_pyfx_port(f_result, EUPHORIA_LFO_TYPE, 0.0f, 0, 2);
    pydaw_set_pyfx_port(f_result, EUPHORIA_FX0_KNOB0, 64.0f, 0, 127);
    pydaw_set_pyfx_port(f_result, EUPHORIA_FX0_KNOB1, 64.0f, 0, 127);
    pydaw_set_pyfx_port(f_result, EUPHORIA_FX0_KNOB2, 64.0f, 0, 127);
    pydaw_set_pyfx_port(f_result, EUPHORIA_FX0_COMBOBOX, 0.0f, 0, MULTIFX3KNOB_MAX_INDEX);
    pydaw_set_pyfx_port(f_result, EUPHORIA_FX1_KNOB0, 64.0f, 0, 127);
    pydaw_set_pyfx_port(f_result, EUPHORIA_FX1_KNOB1, 64.0f, 0, 127);
    pydaw_set_pyfx_port(f_result, EUPHORIA_FX1_KNOB2, 64.0f, 0, 127);
    pydaw_set_pyfx_port(f_result, EUPHORIA_FX1_COMBOBOX, 0.0f, 0, MULTIFX3KNOB_MAX_INDEX);
    pydaw_set_pyfx_port(f_result, EUPHORIA_FX2_KNOB0, 64.0f, 0, 127);
    pydaw_set_pyfx_port(f_result, EUPHORIA_FX2_KNOB1, 64.0f, 0, 127);
    pydaw_set_pyfx_port(f_result, EUPHORIA_FX2_KNOB2, 64.0f, 0, 127);
    pydaw_set_pyfx_port(f_result, EUPHORIA_FX2_COMBOBOX, 0.0f, 0, MULTIFX3KNOB_MAX_INDEX);
    pydaw_set_pyfx_port(f_result, EUPHORIA_FX3_KNOB0, 64.0f, 0, 127);
    pydaw_set_pyfx_port(f_result, EUPHORIA_FX3_KNOB1, 64.0f, 0, 127);
    pydaw_set_pyfx_port(f_result, EUPHORIA_FX3_KNOB2, 64.0f, 0, 127);
    pydaw_set_pyfx_port(f_result, EUPHORIA_FX3_COMBOBOX, 0.0f, 0, MULTIFX3KNOB_MAX_INDEX);

    int f_i = EUPHORIA_PFXMATRIX_GRP0DST0SRC0CTRL0;

    while(f_i <= EUPHORIA_PFXMATRIX_GRP0DST3SRC5CTRL2)
    {
        pydaw_set_pyfx_port(f_result, f_i, 0.0f, -100.0f, 100.0f);
        f_i++;
    }

    pydaw_set_pyfx_port(f_result, EUPHORIA_LFO_PITCH, 0.0f, -36.0f, 36.0f);

    f_i = EUPHORIA_SAMPLE_PITCH_PORT_RANGE_MIN;

    while(f_i < EUPHORIA_SAMPLE_PITCH_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 60.0f, 0, 120);
        f_i++;
    }

    while(f_i < EUPHORIA_PLAY_PITCH_LOW_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 0.0f, 0, 120);
        f_i++;
    }

    while(f_i < EUPHORIA_PLAY_PITCH_HIGH_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 120.0f, 0, 120);
        f_i++;
    }

    while(f_i < EUPHORIA_SAMPLE_VOLUME_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 0.0f, -50, 36);
        f_i++;
    }

    while(f_i < EUPHORIA_SAMPLE_START_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 0.0f, 0.0f, 1000.0f);
        f_i++;
    }

    while(f_i < EUPHORIA_SAMPLE_END_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 1000.0f, 0.0f, 1000.0f);
        f_i++;
    }

    while(f_i < EUPHORIA_SAMPLE_VEL_SENS_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 10.0f, 0.0f, 20.0f);
        f_i++;
    }

    while(f_i < EUPHORIA_SAMPLE_VEL_LOW_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 1.0f, 1.0f, 127.0f);
        f_i++;
    }

    while(f_i < EUPHORIA_SAMPLE_VEL_HIGH_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 128.0f, 1.0f, 128.0f);
        f_i++;
    }

    while(f_i < EUPHORIA_PITCH_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 0.0f, -36.0f, 36.0f);
        f_i++;
    }

    while(f_i < EUPHORIA_TUNE_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 0.0f, -100.0f, 100.0f);
        f_i++;
    }

    while(f_i < EUPHORIA_SAMPLE_INTERPOLATION_MODE_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 1.0f, 0.0f, 3.0f);
        f_i++;
    }

    while(f_i < EUPHORIA_SAMPLE_LOOP_START_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 0.0f, 0.0f, 1000.0f);
        f_i++;
    }

    while(f_i < EUPHORIA_SAMPLE_LOOP_END_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 1000.0f, 0.0f, 1000.0f);
        f_i++;
    }

    while(f_i < EUPHORIA_SAMPLE_LOOP_MODE_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 0.0f, 0.0f, 1.0f);
        f_i++;
    }

    while(f_i < EUPHORIA_MONO_FX0_KNOB0_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 64.0f, 0.0f, 127.0f);
        f_i++;
    }

    while(f_i < EUPHORIA_MONO_FX0_KNOB1_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 64.0f, 0, 127);
        f_i++;
    }

    while(f_i < EUPHORIA_MONO_FX0_KNOB2_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 64.0f, 0, 127);
        f_i++;
    }

    while(f_i < EUPHORIA_MONO_FX0_COMBOBOX_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 0.0f, 0, MULTIFX3KNOB_MAX_INDEX);
        f_i++;
    }

    while(f_i < EUPHORIA_MONO_FX1_KNOB0_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 64.0f, 0, 127);
        f_i++;
    }

    while(f_i < EUPHORIA_MONO_FX1_KNOB1_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 64.0f, 0, 127);
        f_i++;
    }

    while(f_i < EUPHORIA_MONO_FX1_KNOB2_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 64.0f, 0, 127);
        f_i++;
    }

    while(f_i < EUPHORIA_MONO_FX1_COMBOBOX_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 0.0f, 0, MULTIFX3KNOB_MAX_INDEX);
        f_i++;
    }

    while(f_i < EUPHORIA_MONO_FX2_KNOB0_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 64.0f, 0, 127);
        f_i++;
    }

    while(f_i < EUPHORIA_MONO_FX2_KNOB1_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 64.0f, 0, 127);
        f_i++;
    }

    while(f_i < EUPHORIA_MONO_FX2_KNOB2_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 64.0f, 0, 127);
        f_i++;
    }

    while(f_i < EUPHORIA_MONO_FX2_COMBOBOX_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 0.0f, 0, MULTIFX3KNOB_MAX_INDEX);
        f_i++;
    }

    while(f_i < EUPHORIA_MONO_FX3_KNOB0_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 64.0f, 0, 127);
        f_i++;
    }

    while(f_i < EUPHORIA_MONO_FX3_KNOB1_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 64.0f, 0, 127);
        f_i++;
    }

    while(f_i < EUPHORIA_MONO_FX3_KNOB2_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 64.0f, 0, 127);
        f_i++;
    }

    while(f_i < EUPHORIA_MONO_FX3_COMBOBOX_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 0.0f, 0, MULTIFX3KNOB_MAX_INDEX);
        f_i++;
    }

    while(f_i < EUPHORIA_SAMPLE_MONO_FX_GROUP_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 0.0f, 0, (EUPHORIA_MAX_SAMPLE_COUNT - 1));
        f_i++;
    }

    while(f_i < EUPHORIA_NOISE_AMP_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, -30.0f, -60.0f, 0.0f);
        f_i++;
    }

    while(f_i < EUPHORIA_NOISE_TYPE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 0.0f, 0.0f, 2.0f);
        f_i++;
    }

    while(f_i < EUPHORIA_SAMPLE_FADE_IN_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 0.0f, 0.0f, 1000.0f);
        f_i++;
    }

    while(f_i < EUPHORIA_SAMPLE_FADE_OUT_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 1000.0f, 0.0f, 1000.0f);
        f_i++;
    }

    f_i = EUPHORIA_FIRST_EQ_PORT;

    int f_i2 = 0;

    while(f_i2 < EUPHORIA_MAX_SAMPLE_COUNT)
    {
        int f_i3 = 0;
        while(f_i3 < 6)
        {
            pydaw_set_pyfx_port(f_result, f_i, (f_i3 * 18.0f) + 24.0f, 20.0f, 120.0f);
            f_i++;

            pydaw_set_pyfx_port(f_result, f_i, 300.0f, 100.0f, 600.0f);
            f_i++;

            pydaw_set_pyfx_port(f_result, f_i, 0.0f, -24.0f, 24.0f);
            f_i++;

            f_i3++;
        }

        f_i2++;
    }

    pydaw_set_pyfx_port(f_result, EUPHORIA_LFO_PITCH_FINE, 0.0f, -100.0f, 100.0f);

    pydaw_set_pyfx_port(f_result, EUPHORIA_MIN_NOTE, 0.0f, 0.0f, 120.0f);
    pydaw_set_pyfx_port(f_result, EUPHORIA_MAX_NOTE, 120.0f, 0.0f, 120.0f);

    f_result->activate = v_euphoria_activate;
    f_result->cleanup = cleanupSampler;
    f_result->connect_port = connectPortSampler;
    f_result->connect_buffer = euphoriaConnectBuffer;
    f_result->deactivate = NULL;
    f_result->instantiate = instantiateSampler;
    f_result->panic = euphoriaPanic;

    return f_result;
}

PYINST_Descriptor *euphoria_PYINST_descriptor(int index)
{
    PYINST_Descriptor *euphoriaDDescriptor = NULL;

    euphoriaDDescriptor = (PYINST_Descriptor *) malloc(sizeof(PYINST_Descriptor));

    euphoriaDDescriptor->PYINST_API_Version = 1;
    euphoriaDDescriptor->PYFX_Plugin = euphoria_PYFX_descriptor(0);
    euphoriaDDescriptor->configure = c_euphoria_configure;
    euphoriaDDescriptor->run_synth = v_run_lms_euphoria;
    euphoriaDDescriptor->offline_render_prep = NULL;
    euphoriaDDescriptor->on_stop = v_euphoria_on_stop;

    return euphoriaDDescriptor;
}
