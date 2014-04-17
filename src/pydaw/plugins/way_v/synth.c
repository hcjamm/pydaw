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

#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <stdint.h>

#include <math.h>
#include <stdio.h>

#include "../../include/pydaw_plugin.h"

#include "libmodsynth.h"
#include "../../libmodsynth/lib/amp.h"
#include "../../libmodsynth/modules/filter/svf.h"
#include "../../libmodsynth/lib/lms_math.h"
#include "synth.h"
#include "../../src/pydaw_files.h"

static void v_run_wayv(PYFX_Handle instance, int sample_count,
		  t_pydaw_seq_event * events, int EventCount);

static void v_run_wayv_voice(t_wayv *, t_voc_single_voice, t_wayv_poly_voice *,
		      PYFX_Data *, PYFX_Data *, int, int );

const PYFX_Descriptor *wayv_PYFX_descriptor(int index);
const PYINST_Descriptor *wayv_PYINST_descriptor(int index);


static void v_cleanup_wayv(PYFX_Handle instance)
{
    free(instance);
}

static void v_wayv_or_prep(PYFX_Handle instance)
{
    t_wayv *plugin = (t_wayv *)instance;
    int f_i2 = 0;

    int f_osc1_on = ((int)(*plugin->osc1type) - 1);
    int f_osc2_on = ((int)(*plugin->osc2type) - 1);
    int f_osc3_on = ((int)(*plugin->osc3type) - 1);
    int f_osc4_on = ((int)(*plugin->osc4type) - 1);

    while(f_i2 < WAYV_POLYPHONY)
    {
        t_wayv_poly_voice * f_voice = plugin->data[f_i2];
        int f_i = 0;
        while(f_i < 1000000)
        {
            if(f_osc1_on >= 0)
            {
                v_osc_wav_run_unison_core_only(f_voice->osc_wavtable[0]);
            }
            if(f_osc2_on >= 0)
            {
                v_osc_wav_run_unison_core_only(f_voice->osc_wavtable[1]);
            }
            if(f_osc3_on >= 0)
            {
                v_osc_wav_run_unison_core_only(f_voice->osc_wavtable[2]);
            }
            if(f_osc4_on >= 0)
            {
                v_osc_wav_run_unison_core_only(f_voice->osc_wavtable[3]);
            }

            f_i++;
        }
        f_i2++;
    }

    plugin->mono_modules->fm_macro_smoother[0]->last_value =
            (*plugin->fm_macro[0] * 0.01f);

    plugin->mono_modules->fm_macro_smoother[1]->last_value =
            (*plugin->fm_macro[1] * 0.01f);
}

static void wayvPanic(PYFX_Handle instance)
{
    t_wayv *plugin = (t_wayv *)instance;
    int f_i = 0;
    while(f_i < WAYV_POLYPHONY)
    {
        v_adsr_kill(plugin->data[f_i]->adsr_amp);
        v_adsr_kill(plugin->data[f_i]->adsr_main);
        f_i++;
    }
}

static void v_wayv_connect_buffer(PYFX_Handle instance, int a_index,
        float * DataLocation)
{
    t_wayv *plugin = (t_wayv*)instance;

    switch(a_index)
    {
        case 0:
            plugin->output0 = DataLocation;
            break;
        case 1:
            plugin->output1 = DataLocation;
            break;
        default:
            assert(0);
            break;
    }
}

static void v_wayv_connect_port(PYFX_Handle instance, int port,
			  PYFX_Data * data)
{
    t_wayv *plugin;

    plugin = (t_wayv *) instance;

    switch (port)
    {
        case WAYV_ATTACK_MAIN:
            plugin->attack_main = data;
            break;
        case WAYV_DECAY_MAIN:
            plugin->decay_main = data;
            break;
        case WAYV_SUSTAIN_MAIN:
            plugin->sustain_main = data;
            break;
        case WAYV_RELEASE_MAIN:
            plugin->release_main = data;
            break;

        case WAYV_ATTACK1:
            plugin->attack1 = data;
            break;
        case WAYV_DECAY1:
            plugin->decay1 = data;
            break;
        case WAYV_SUSTAIN1:
            plugin->sustain1 = data;
            break;
        case WAYV_RELEASE1:
            plugin->release1 = data;
            break;

        case WAYV_ATTACK2:
            plugin->attack2 = data;
            break;
        case WAYV_DECAY2:
            plugin->decay2 = data;
            break;
        case WAYV_SUSTAIN2:
            plugin->sustain2 = data;
            break;
        case WAYV_RELEASE2:
            plugin->release2 = data;
            break;

        case WAYV_NOISE_AMP:
            plugin->noise_amp = data;
            break;
        case WAYV_MASTER_VOLUME:
            plugin->master_vol = data;
            break;
        case WAYV_OSC1_PITCH:
            plugin->osc_pitch[0] = data;
            break;
        case WAYV_OSC1_TUNE:
            plugin->osc_tune[0] = data;
            break;
        case WAYV_OSC1_TYPE:
            plugin->osc1type = data;
            break;
        case WAYV_OSC1_VOLUME:
            plugin->osc_vol[0] = data;
            break;
        case WAYV_OSC2_PITCH:
            plugin->osc_pitch[1] = data;
            break;
        case WAYV_OSC2_TUNE:
            plugin->osc_tune[1] = data;
            break;
        case WAYV_OSC2_TYPE:
            plugin->osc2type = data;
            break;
        case WAYV_OSC2_VOLUME:
            plugin->osc_vol[1] = data;
            break;
        case WAYV_OSC1_UNISON_VOICES:
            plugin->osc_uni_voice[0] = data;
            break;
        case WAYV_OSC1_UNISON_SPREAD:
            plugin->osc_uni_spread[0] = data;
            break;
        case WAYV_OSC2_UNISON_VOICES:
            plugin->osc_uni_voice[1] = data;
            break;
        case WAYV_OSC2_UNISON_SPREAD:
            plugin->osc_uni_spread[1] = data;
            break;
        case WAYV_OSC3_UNISON_VOICES:
            plugin->osc_uni_voice[2] = data;
            break;
        case WAYV_OSC3_UNISON_SPREAD:
            plugin->osc_uni_spread[2] = data;
            break;
        case WAYV_MASTER_GLIDE:
            plugin->master_glide = data;
            break;
        case WAYV_MASTER_PITCHBEND_AMT:
            plugin->master_pb_amt = data;
            break;


        case WAYV_ATTACK_PFX1:
            plugin->attack = data;
            break;
        case WAYV_DECAY_PFX1:
            plugin->decay = data;
            break;
        case WAYV_SUSTAIN_PFX1:
            plugin->sustain = data;
            break;
        case WAYV_RELEASE_PFX1:
            plugin->release = data;
            break;
        case WAYV_ATTACK_PFX2:
            plugin->attack_f = data;
            break;
        case WAYV_DECAY_PFX2:
            plugin->decay_f = data;
            break;
        case WAYV_SUSTAIN_PFX2:
            plugin->sustain_f = data;
            break;
        case WAYV_RELEASE_PFX2:
            plugin->release_f = data;
            break;
        case WAYV_RAMP_ENV_TIME:
            plugin->pitch_env_time = data;
            break;
        case WAYV_LFO_FREQ:
            plugin->lfo_freq = data;
            break;
        case WAYV_LFO_TYPE:
            plugin->lfo_type = data;
            break;

        case WAYV_FX0_KNOB0: plugin->pfx_mod_knob[0][0][0] = data; break;
        case WAYV_FX0_KNOB1: plugin->pfx_mod_knob[0][0][1] = data; break;
        case WAYV_FX0_KNOB2: plugin->pfx_mod_knob[0][0][2] = data; break;
        case WAYV_FX1_KNOB0: plugin->pfx_mod_knob[0][1][0] = data; break;
        case WAYV_FX1_KNOB1: plugin->pfx_mod_knob[0][1][1] = data; break;
        case WAYV_FX1_KNOB2: plugin->pfx_mod_knob[0][1][2] = data; break;
        case WAYV_FX2_KNOB0: plugin->pfx_mod_knob[0][2][0] = data; break;
        case WAYV_FX2_KNOB1: plugin->pfx_mod_knob[0][2][1] = data; break;
        case WAYV_FX2_KNOB2: plugin->pfx_mod_knob[0][2][2] = data; break;
        case WAYV_FX3_KNOB0: plugin->pfx_mod_knob[0][3][0] = data; break;
        case WAYV_FX3_KNOB1: plugin->pfx_mod_knob[0][3][1] = data; break;
        case WAYV_FX3_KNOB2: plugin->pfx_mod_knob[0][3][2] = data; break;

        case WAYV_FX0_COMBOBOX: plugin->fx_combobox[0][0] = data; break;
        case WAYV_FX1_COMBOBOX: plugin->fx_combobox[0][1] = data; break;
        case WAYV_FX2_COMBOBOX: plugin->fx_combobox[0][2] = data; break;
        case WAYV_FX3_COMBOBOX: plugin->fx_combobox[0][3] = data; break;
        //End from Modulex
        /*PolyFX mod matrix port connections*/
        case WAVV_PFXMATRIX_GRP0DST0SRC0CTRL0: plugin->polyfx_mod_matrix[0][0][0][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST0SRC0CTRL1: plugin->polyfx_mod_matrix[0][0][0][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST0SRC0CTRL2: plugin->polyfx_mod_matrix[0][0][0][2] = data; break;
        case WAVV_PFXMATRIX_GRP0DST0SRC1CTRL0: plugin->polyfx_mod_matrix[0][0][1][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST0SRC1CTRL1: plugin->polyfx_mod_matrix[0][0][1][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST0SRC1CTRL2: plugin->polyfx_mod_matrix[0][0][1][2] = data; break;
        case WAVV_PFXMATRIX_GRP0DST0SRC2CTRL0: plugin->polyfx_mod_matrix[0][0][2][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST0SRC2CTRL1: plugin->polyfx_mod_matrix[0][0][2][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST0SRC2CTRL2: plugin->polyfx_mod_matrix[0][0][2][2] = data; break;
        case WAVV_PFXMATRIX_GRP0DST0SRC3CTRL0: plugin->polyfx_mod_matrix[0][0][3][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST0SRC3CTRL1: plugin->polyfx_mod_matrix[0][0][3][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST0SRC3CTRL2: plugin->polyfx_mod_matrix[0][0][3][2] = data; break;
        case WAVV_PFXMATRIX_GRP0DST1SRC0CTRL0: plugin->polyfx_mod_matrix[0][1][0][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST1SRC0CTRL1: plugin->polyfx_mod_matrix[0][1][0][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST1SRC0CTRL2: plugin->polyfx_mod_matrix[0][1][0][2] = data; break;
        case WAVV_PFXMATRIX_GRP0DST1SRC1CTRL0: plugin->polyfx_mod_matrix[0][1][1][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST1SRC1CTRL1: plugin->polyfx_mod_matrix[0][1][1][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST1SRC1CTRL2: plugin->polyfx_mod_matrix[0][1][1][2] = data; break;
        case WAVV_PFXMATRIX_GRP0DST1SRC2CTRL0: plugin->polyfx_mod_matrix[0][1][2][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST1SRC2CTRL1: plugin->polyfx_mod_matrix[0][1][2][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST1SRC2CTRL2: plugin->polyfx_mod_matrix[0][1][2][2] = data; break;
        case WAVV_PFXMATRIX_GRP0DST1SRC3CTRL0: plugin->polyfx_mod_matrix[0][1][3][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST1SRC3CTRL1: plugin->polyfx_mod_matrix[0][1][3][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST1SRC3CTRL2: plugin->polyfx_mod_matrix[0][1][3][2] = data; break;
        case WAVV_PFXMATRIX_GRP0DST2SRC0CTRL0: plugin->polyfx_mod_matrix[0][2][0][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST2SRC0CTRL1: plugin->polyfx_mod_matrix[0][2][0][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST2SRC0CTRL2: plugin->polyfx_mod_matrix[0][2][0][2] = data; break;
        case WAVV_PFXMATRIX_GRP0DST2SRC1CTRL0: plugin->polyfx_mod_matrix[0][2][1][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST2SRC1CTRL1: plugin->polyfx_mod_matrix[0][2][1][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST2SRC1CTRL2: plugin->polyfx_mod_matrix[0][2][1][2] = data; break;
        case WAVV_PFXMATRIX_GRP0DST2SRC2CTRL0: plugin->polyfx_mod_matrix[0][2][2][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST2SRC2CTRL1: plugin->polyfx_mod_matrix[0][2][2][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST2SRC2CTRL2: plugin->polyfx_mod_matrix[0][2][2][2] = data; break;
        case WAVV_PFXMATRIX_GRP0DST2SRC3CTRL0: plugin->polyfx_mod_matrix[0][2][3][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST2SRC3CTRL1: plugin->polyfx_mod_matrix[0][2][3][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST2SRC3CTRL2: plugin->polyfx_mod_matrix[0][2][3][2] = data; break;
        case WAVV_PFXMATRIX_GRP0DST3SRC0CTRL0: plugin->polyfx_mod_matrix[0][3][0][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST3SRC0CTRL1: plugin->polyfx_mod_matrix[0][3][0][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST3SRC0CTRL2: plugin->polyfx_mod_matrix[0][3][0][2] = data; break;
        case WAVV_PFXMATRIX_GRP0DST3SRC1CTRL0: plugin->polyfx_mod_matrix[0][3][1][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST3SRC1CTRL1: plugin->polyfx_mod_matrix[0][3][1][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST3SRC1CTRL2: plugin->polyfx_mod_matrix[0][3][1][2] = data; break;
        case WAVV_PFXMATRIX_GRP0DST3SRC2CTRL0: plugin->polyfx_mod_matrix[0][3][2][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST3SRC2CTRL1: plugin->polyfx_mod_matrix[0][3][2][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST3SRC2CTRL2: plugin->polyfx_mod_matrix[0][3][2][2] = data; break;
        case WAVV_PFXMATRIX_GRP0DST3SRC3CTRL0: plugin->polyfx_mod_matrix[0][3][3][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST3SRC3CTRL1: plugin->polyfx_mod_matrix[0][3][3][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST3SRC3CTRL2: plugin->polyfx_mod_matrix[0][3][3][2] = data; break;

        //keyboard tracking
        case WAVV_PFXMATRIX_GRP0DST0SRC4CTRL0: plugin->polyfx_mod_matrix[0][0][4][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST0SRC4CTRL1: plugin->polyfx_mod_matrix[0][0][4][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST0SRC4CTRL2: plugin->polyfx_mod_matrix[0][0][4][2] = data; break;
        case WAVV_PFXMATRIX_GRP0DST1SRC4CTRL0: plugin->polyfx_mod_matrix[0][1][4][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST1SRC4CTRL1: plugin->polyfx_mod_matrix[0][1][4][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST1SRC4CTRL2: plugin->polyfx_mod_matrix[0][1][4][2] = data; break;
        case WAVV_PFXMATRIX_GRP0DST2SRC4CTRL0: plugin->polyfx_mod_matrix[0][2][4][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST2SRC4CTRL1: plugin->polyfx_mod_matrix[0][2][4][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST2SRC4CTRL2: plugin->polyfx_mod_matrix[0][2][4][2] = data; break;
        case WAVV_PFXMATRIX_GRP0DST3SRC4CTRL0: plugin->polyfx_mod_matrix[0][3][4][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST3SRC4CTRL1: plugin->polyfx_mod_matrix[0][3][4][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST3SRC4CTRL2: plugin->polyfx_mod_matrix[0][3][4][2] = data; break;

        //velocity tracking
        case WAVV_PFXMATRIX_GRP0DST0SRC5CTRL0: plugin->polyfx_mod_matrix[0][0][5][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST0SRC5CTRL1: plugin->polyfx_mod_matrix[0][0][5][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST0SRC5CTRL2: plugin->polyfx_mod_matrix[0][0][5][2] = data; break;
        case WAVV_PFXMATRIX_GRP0DST1SRC5CTRL0: plugin->polyfx_mod_matrix[0][1][5][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST1SRC5CTRL1: plugin->polyfx_mod_matrix[0][1][5][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST1SRC5CTRL2: plugin->polyfx_mod_matrix[0][1][5][2] = data; break;
        case WAVV_PFXMATRIX_GRP0DST2SRC5CTRL0: plugin->polyfx_mod_matrix[0][2][5][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST2SRC5CTRL1: plugin->polyfx_mod_matrix[0][2][5][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST2SRC5CTRL2: plugin->polyfx_mod_matrix[0][2][5][2] = data; break;
        case WAVV_PFXMATRIX_GRP0DST3SRC5CTRL0: plugin->polyfx_mod_matrix[0][3][5][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST3SRC5CTRL1: plugin->polyfx_mod_matrix[0][3][5][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST3SRC5CTRL2: plugin->polyfx_mod_matrix[0][3][5][2] = data; break;


        case LMS_NOISE_TYPE: plugin->noise_type = data; break;
        case WAYV_ADSR1_CHECKBOX: plugin->adsr1_checked = data; break;
        case WAYV_ADSR2_CHECKBOX: plugin->adsr2_checked = data; break;

        case WAYV_LFO_AMP: plugin->lfo_amp = data; break;
        case WAYV_LFO_PITCH: plugin->lfo_pitch = data; break;

        case WAYV_PITCH_ENV_AMT: plugin->pitch_env_amt = data; break;
        case WAYV_LFO_AMOUNT: plugin->lfo_amount = data; break;

        case WAYV_OSC3_PITCH: plugin->osc_pitch[2] = data; break;
        case WAYV_OSC3_TUNE: plugin->osc_tune[2] = data; break;
        case WAYV_OSC3_TYPE: plugin->osc3type = data; break;
        case WAYV_OSC3_VOLUME: plugin->osc_vol[2] = data;  break;

        case WAYV_OSC1_FM1: plugin->osc_fm[0][0] = data;  break;
        case WAYV_OSC1_FM2: plugin->osc_fm[0][1] = data;  break;
        case WAYV_OSC1_FM3: plugin->osc_fm[0][2] = data;  break;

        case WAYV_OSC2_FM1: plugin->osc_fm[1][0] = data;  break;
        case WAYV_OSC2_FM2: plugin->osc_fm[1][1] = data;  break;
        case WAYV_OSC2_FM3: plugin->osc_fm[1][2] = data;  break;

        case WAYV_OSC3_FM1: plugin->osc_fm[2][0] = data;  break;
        case WAYV_OSC3_FM2: plugin->osc_fm[2][1] = data;  break;
        case WAYV_OSC3_FM3: plugin->osc_fm[2][2] = data;  break;

        case WAYV_ATTACK3: plugin->attack3 = data; break;
        case WAYV_DECAY3: plugin->decay3 = data; break;
        case WAYV_SUSTAIN3: plugin->sustain3 = data; break;
        case WAYV_RELEASE3: plugin->release3 = data; break;

        case WAYV_ADSR3_CHECKBOX: plugin->adsr3_checked = data; break;

        case WAYV_PERC_ENV_PITCH1: plugin->perc_env_pitch1 = data; break;
        case WAYV_PERC_ENV_TIME1: plugin->perc_env_time1 = data; break;
        case WAYV_PERC_ENV_PITCH2: plugin->perc_env_pitch2 = data; break;
        case WAYV_PERC_ENV_TIME2: plugin->perc_env_time2 = data; break;
        case WAYV_PERC_ENV_ON: plugin->perc_env_on = data; break;

        case WAYV_RAMP_CURVE: plugin->ramp_curve = data; break;

        case WAYV_MONO_MODE: plugin->mono_mode = data; break;

        case WAYV_OSC1_FM4: plugin->osc_fm[0][3] = data;  break;
        case WAYV_OSC2_FM4: plugin->osc_fm[1][3] = data;  break;
        case WAYV_OSC3_FM4: plugin->osc_fm[2][3] = data;  break;

        case WAYV_OSC4_UNISON_VOICES: plugin->osc_uni_voice[3] = data; break;
        case WAYV_OSC4_UNISON_SPREAD: plugin->osc_uni_spread[3] = data; break;

        case WAYV_OSC4_PITCH: plugin->osc_pitch[3] = data; break;
        case WAYV_OSC4_TUNE: plugin->osc_tune[3] = data; break;
        case WAYV_OSC4_TYPE: plugin->osc4type = data; break;
        case WAYV_OSC4_VOLUME: plugin->osc_vol[3] = data;  break;

        case WAYV_OSC4_FM1: plugin->osc_fm[3][0] = data;  break;
        case WAYV_OSC4_FM2: plugin->osc_fm[3][1] = data;  break;
        case WAYV_OSC4_FM3: plugin->osc_fm[3][2] = data;  break;
        case WAYV_OSC4_FM4: plugin->osc_fm[3][3] = data;  break;

        case WAYV_ATTACK4: plugin->attack4 = data; break;
        case WAYV_DECAY4: plugin->decay4 = data; break;
        case WAYV_SUSTAIN4: plugin->sustain4 = data; break;
        case WAYV_RELEASE4: plugin->release4 = data; break;

        case WAYV_ADSR4_CHECKBOX: plugin->adsr4_checked = data; break;

        case WAYV_FM_MACRO1: plugin->fm_macro[0] = data; break;
        case WAYV_FM_MACRO1_OSC1_FM1: plugin->fm_macro_values[0][0][0] = data; break;
        case WAYV_FM_MACRO1_OSC1_FM2: plugin->fm_macro_values[0][0][1] = data; break;
        case WAYV_FM_MACRO1_OSC1_FM3: plugin->fm_macro_values[0][0][2] = data; break;
        case WAYV_FM_MACRO1_OSC1_FM4: plugin->fm_macro_values[0][0][3] = data; break;
        case WAYV_FM_MACRO1_OSC2_FM1: plugin->fm_macro_values[0][1][0] = data; break;
        case WAYV_FM_MACRO1_OSC2_FM2: plugin->fm_macro_values[0][1][1] = data; break;
        case WAYV_FM_MACRO1_OSC2_FM3: plugin->fm_macro_values[0][1][2] = data; break;
        case WAYV_FM_MACRO1_OSC2_FM4: plugin->fm_macro_values[0][1][3] = data; break;
        case WAYV_FM_MACRO1_OSC3_FM1: plugin->fm_macro_values[0][2][0] = data; break;
        case WAYV_FM_MACRO1_OSC3_FM2: plugin->fm_macro_values[0][2][1] = data; break;
        case WAYV_FM_MACRO1_OSC3_FM3: plugin->fm_macro_values[0][2][2] = data; break;
        case WAYV_FM_MACRO1_OSC3_FM4: plugin->fm_macro_values[0][2][3] = data; break;
        case WAYV_FM_MACRO1_OSC4_FM1: plugin->fm_macro_values[0][3][0] = data; break;
        case WAYV_FM_MACRO1_OSC4_FM2: plugin->fm_macro_values[0][3][1] = data; break;
        case WAYV_FM_MACRO1_OSC4_FM3: plugin->fm_macro_values[0][3][2] = data; break;
        case WAYV_FM_MACRO1_OSC4_FM4: plugin->fm_macro_values[0][3][3] = data; break;

        case WAYV_FM_MACRO2: plugin->fm_macro[1] = data; break;
        case WAYV_FM_MACRO2_OSC1_FM1: plugin->fm_macro_values[1][0][0] = data; break;
        case WAYV_FM_MACRO2_OSC1_FM2: plugin->fm_macro_values[1][0][1] = data; break;
        case WAYV_FM_MACRO2_OSC1_FM3: plugin->fm_macro_values[1][0][2] = data; break;
        case WAYV_FM_MACRO2_OSC1_FM4: plugin->fm_macro_values[1][0][3] = data; break;
        case WAYV_FM_MACRO2_OSC2_FM1: plugin->fm_macro_values[1][1][0] = data; break;
        case WAYV_FM_MACRO2_OSC2_FM2: plugin->fm_macro_values[1][1][1] = data; break;
        case WAYV_FM_MACRO2_OSC2_FM3: plugin->fm_macro_values[1][1][2] = data; break;
        case WAYV_FM_MACRO2_OSC2_FM4: plugin->fm_macro_values[1][1][3] = data; break;
        case WAYV_FM_MACRO2_OSC3_FM1: plugin->fm_macro_values[1][2][0] = data; break;
        case WAYV_FM_MACRO2_OSC3_FM2: plugin->fm_macro_values[1][2][1] = data; break;
        case WAYV_FM_MACRO2_OSC3_FM3: plugin->fm_macro_values[1][2][2] = data; break;
        case WAYV_FM_MACRO2_OSC3_FM4: plugin->fm_macro_values[1][2][3] = data; break;
        case WAYV_FM_MACRO2_OSC4_FM1: plugin->fm_macro_values[1][3][0] = data; break;
        case WAYV_FM_MACRO2_OSC4_FM2: plugin->fm_macro_values[1][3][1] = data; break;
        case WAYV_FM_MACRO2_OSC4_FM3: plugin->fm_macro_values[1][3][2] = data; break;
        case WAYV_FM_MACRO2_OSC4_FM4: plugin->fm_macro_values[1][3][3] = data; break;

        case WAYV_LFO_PHASE: plugin->lfo_phase = data; break;
    }
}

static PYFX_Handle g_wayv_instantiate(const PYFX_Descriptor * descriptor,
                int s_rate, fp_get_wavpool_item_from_host a_host_wavpool_func)
{
    t_wayv *plugin_data = (t_wayv *) malloc(sizeof(t_wayv));
    plugin_data->fs = s_rate;

    return (PYFX_Handle) plugin_data;
}

static void v_wayv_activate(PYFX_Handle instance, float * a_port_table)
{
    t_wayv *plugin_data = (t_wayv *) instance;

    plugin_data->port_table = a_port_table;

    int i;

    plugin_data->voices = g_voc_get_voices(WAYV_POLYPHONY,
            WAYV_POLYPHONY_THRESH);

    for (i = 0; i < WAYV_POLYPHONY; i++)
    {
        plugin_data->data[i] = g_wayv_poly_init(plugin_data->fs);
        plugin_data->data[i]->note_f = i;
    }
    plugin_data->sampleNo = 0;

    //plugin_data->pitch = 1.0f;
    plugin_data->sv_pitch_bend_value = 0.0f;
    plugin_data->sv_last_note = -1.0f;  //For glide

    plugin_data->mono_modules = v_wayv_mono_init(plugin_data->fs);
}

static void v_run_wayv(PYFX_Handle instance, int sample_count,
		  t_pydaw_seq_event *events, int event_count)
{
    t_wayv *plugin_data = (t_wayv *) instance;

    plugin_data->i_run_poly_voice = 0;
    plugin_data->midi_event_count = 0;

    int midi_event_pos = 0;

    int f_poly_mode = (int)(*plugin_data->mono_mode);

    if(f_poly_mode == 2 && plugin_data->voices->poly_mode != 2)
    {
        wayvPanic(instance);  //avoid hung notes
    }

    plugin_data->voices->poly_mode = f_poly_mode;

    for(plugin_data->event_pos = 0; (plugin_data->event_pos) < event_count;
            plugin_data->event_pos = (plugin_data->event_pos) + 1)
    {
        if (events[(plugin_data->event_pos)].type == PYDAW_EVENT_NOTEON)
        {
            if (events[(plugin_data->event_pos)].velocity > 0)
            {
                int f_voice = i_pick_voice(plugin_data->voices,
                        events[(plugin_data->event_pos)].note,
                        plugin_data->sampleNo,
                        events[(plugin_data->event_pos)].tick);

                plugin_data->data[f_voice]->amp =
                        f_db_to_linear_fast(
                        ((events[(plugin_data->event_pos)].velocity
                        * 0.094488) - 12), //-12db to 0db
                        plugin_data->mono_modules->amp_ptr);

                plugin_data->data[f_voice]->master_vol_lin =
                        f_db_to_linear_fast(
                            (*(plugin_data->master_vol)),
                            plugin_data->mono_modules->amp_ptr);

                plugin_data->data[f_voice]->keyboard_track =
                        ((float)(events[(plugin_data->event_pos)].note))
                        * 0.007874016f;

                plugin_data->data[f_voice]->velocity_track =
                        ((float)(events[(plugin_data->event_pos)].velocity))
                        * 0.007874016f;

                plugin_data->data[f_voice]->note_f =
                        (float)events[(plugin_data->event_pos)].note;
                plugin_data->data[f_voice]->note =
                        events[(plugin_data->event_pos)].note;

                plugin_data->data[f_voice]->target_pitch =
                        (plugin_data->data[f_voice]->note_f);

                if(plugin_data->sv_last_note < 0.0f)
                {
                    plugin_data->data[f_voice]->last_pitch =
                            (plugin_data->data[f_voice]->note_f);
                }
                else
                {
                    plugin_data->data[f_voice]->last_pitch =
                            (plugin_data->sv_last_note);
                }

                v_rmp_retrigger_glide_t(plugin_data->data[f_voice]->glide_env,
                        (*(plugin_data->master_glide) * 0.01f),
                        (plugin_data->data[f_voice]->last_pitch),
                        (plugin_data->data[f_voice]->target_pitch));

                int f_i = 0;

                float f_db;

                while(f_i < 4)
                {
                    f_db = (*plugin_data->osc_vol[f_i]);

                    v_adsr_retrigger(
                        plugin_data->data[f_voice]->adsr_amp_osc[f_i]);

                    if(f_db > -29.2f)
                    {
                        plugin_data->data[f_voice]->osc_linamp[f_i] =
                            f_db_to_linear_fast(f_db,
                                plugin_data->mono_modules->amp_ptr);
                        plugin_data->data[f_voice]->osc_audible[f_i] = 1;
                    }
                    else
                    {
                        plugin_data->data[f_voice]->osc_audible[f_i] = 0;
                    }

                    f_i++;
                }

                plugin_data->data[f_voice]->noise_linamp =
                    f_db_to_linear_fast(*(plugin_data->noise_amp),
                        plugin_data->mono_modules->amp_ptr);

                v_adsr_retrigger(plugin_data->data[f_voice]->adsr_main);

                float f_attack = *(plugin_data->attack_main) * .01f;
                f_attack = (f_attack) * (f_attack);
                float f_decay = *(plugin_data->decay_main) * .01f;
                f_decay = (f_decay) * (f_decay);
                float f_release = *(plugin_data->release_main) * .01f;
                f_release = (f_release) * (f_release);

                v_adsr_set_adsr_db(plugin_data->data[f_voice]->adsr_main,
                    (f_attack), (f_decay), *(plugin_data->sustain_main),
                        (f_release));


                plugin_data->data[f_voice]->adsr_amp_on[0] =
                        (int)(*(plugin_data->adsr1_checked));

                if(plugin_data->data[f_voice]->adsr_amp_on[0])
                {
                    float f_attack1 = *(plugin_data->attack1) * .01f;
                    f_attack1 = (f_attack1) * (f_attack1);
                    float f_decay1 = *(plugin_data->decay1) * .01f;
                    f_decay1 = (f_decay1) * (f_decay1);
                    float f_release1 = *(plugin_data->release1) * .01f;
                    f_release1 = (f_release1) * (f_release1);

                    v_adsr_set_adsr_db(plugin_data->data[f_voice]->adsr_amp_osc[0],
                            (f_attack1), (f_decay1), *(plugin_data->sustain1),
                            (f_release1));
                }

                plugin_data->data[f_voice]->adsr_amp_on[1] =
                        (int)(*(plugin_data->adsr2_checked));

                if(plugin_data->data[f_voice]->adsr_amp_on[1])
                {
                    float f_attack2 = *(plugin_data->attack2) * .01f;
                    f_attack2 = (f_attack2) * (f_attack2);
                    float f_decay2 = *(plugin_data->decay2) * .01f;
                    f_decay2 = (f_decay2) * (f_decay2);
                    float f_release2 = *(plugin_data->release2) * .01f;
                    f_release2 = (f_release2) * (f_release2);

                    v_adsr_set_adsr_db(plugin_data->data[f_voice]->adsr_amp_osc[1],
                        (f_attack2), (f_decay2), *(plugin_data->sustain2),
                            (f_release2));
                }

                plugin_data->data[f_voice]->adsr_amp_on[2] =
                        (int)(*(plugin_data->adsr3_checked));

                if(plugin_data->data[f_voice]->adsr_amp_on[2])
                {
                    float f_attack3 = *(plugin_data->attack3) * .01f;
                    f_attack3 = (f_attack3) * (f_attack3);
                    float f_decay3 = *(plugin_data->decay3) * .01f;
                    f_decay3 = (f_decay3) * (f_decay3);
                    float f_release3 = *(plugin_data->release3) * .01f;
                    f_release3 = (f_release3) * (f_release3);

                    v_adsr_set_adsr_db(plugin_data->data[f_voice]->adsr_amp_osc[2],
                        (f_attack3), (f_decay3), *(plugin_data->sustain3),
                            (f_release3));
                }

                plugin_data->data[f_voice]->adsr_amp_on[3] =
                        (int)(*(plugin_data->adsr4_checked));

                if(plugin_data->data[f_voice]->adsr_amp_on[3])
                {
                    float f_attack4 = *(plugin_data->attack4) * .01f;
                    f_attack4 = (f_attack4) * (f_attack4);
                    float f_decay4 = *(plugin_data->decay4) * .01f;
                    f_decay4 = (f_decay4) * (f_decay4);
                    float f_release4 = *(plugin_data->release4) * .01f;
                    f_release4 = (f_release4) * (f_release4);

                    v_adsr_set_adsr_db(plugin_data->data[f_voice]->adsr_amp_osc[3],
                        (f_attack4), (f_decay4), *(plugin_data->sustain4),
                            (f_release4));
                }

                plugin_data->data[f_voice]->noise_amp =
                    f_db_to_linear(*(plugin_data->noise_amp),
                        plugin_data->mono_modules->amp_ptr);

                int f_osc_type1 = (int)(*plugin_data->osc1type) - 1;

                if(f_osc_type1 >= 0)
                {
                    plugin_data->data[f_voice]->osc_on[0] = 1;

                    if(f_poly_mode == 0)
                    {
                        v_osc_wav_note_on_sync_phases(
                                plugin_data->data[f_voice]->osc_wavtable[0]);
                    }
                    v_osc_wav_set_waveform(
                            plugin_data->data[f_voice]->osc_wavtable[0],
                            plugin_data->mono_modules->
                            wavetables->tables[f_osc_type1]->wavetable,
                            plugin_data->mono_modules->wavetables->
                            tables[f_osc_type1]->length);
                    v_osc_wav_set_uni_voice_count(
                            plugin_data->data[f_voice]->osc_wavtable[0],
                            *plugin_data->osc_uni_voice[0]);
                }
                else
                {
                    plugin_data->data[f_voice]->osc_on[0] = 0;
                }

                int f_osc_type2 = (int)(*plugin_data->osc2type) - 1;

                if(f_osc_type2 >= 0)
                {
                    plugin_data->data[f_voice]->osc_on[1] = 1;

                    if(f_poly_mode == 0)
                    {
                        v_osc_wav_note_on_sync_phases(
                                plugin_data->data[f_voice]->osc_wavtable[1]);
                    }
                    v_osc_wav_set_waveform(
                            plugin_data->data[f_voice]->osc_wavtable[1],
                            plugin_data->mono_modules->wavetables->
                            tables[f_osc_type2]->wavetable,
                            plugin_data->mono_modules->wavetables->
                            tables[f_osc_type2]->length);

                    v_osc_wav_set_uni_voice_count(
                            plugin_data->data[f_voice]->osc_wavtable[1],
                            *plugin_data->osc_uni_voice[1]);
                }
                else
                {
                    plugin_data->data[f_voice]->osc_on[1] = 0;
                }

                int f_osc_type3 = (int)(*plugin_data->osc3type) - 1;

                if(f_osc_type3 >= 0)
                {
                    plugin_data->data[f_voice]->osc_on[2] = 1;

                    if(f_poly_mode == 0)
                    {
                        v_osc_wav_note_on_sync_phases(
                                plugin_data->data[f_voice]->osc_wavtable[2]);
                    }
                    v_osc_wav_set_waveform(
                            plugin_data->data[f_voice]->osc_wavtable[2],
                            plugin_data->mono_modules->wavetables->
                            tables[f_osc_type3]->wavetable,
                            plugin_data->mono_modules->wavetables->
                            tables[f_osc_type3]->length);

                    v_osc_wav_set_uni_voice_count(
                            plugin_data->data[f_voice]->osc_wavtable[2],
                            *plugin_data->osc_uni_voice[2]);
                }
                else
                {
                    plugin_data->data[f_voice]->osc_on[2] = 0;
                }


                int f_osc_type4 = (int)(*plugin_data->osc4type) - 1;

                if(f_osc_type4 >= 0)
                {
                    plugin_data->data[f_voice]->osc_on[3] = 1;

                    if(f_poly_mode == 0)
                    {
                        v_osc_wav_note_on_sync_phases(
                                plugin_data->data[f_voice]->osc_wavtable[3]);
                    }
                    v_osc_wav_set_waveform(
                            plugin_data->data[f_voice]->osc_wavtable[3],
                            plugin_data->mono_modules->wavetables->
                            tables[f_osc_type4]->wavetable,
                            plugin_data->mono_modules->wavetables->
                            tables[f_osc_type4]->length);

                    v_osc_wav_set_uni_voice_count(
                            plugin_data->data[f_voice]->osc_wavtable[3],
                            *plugin_data->osc_uni_voice[3]);
                }
                else
                {
                    plugin_data->data[f_voice]->osc_on[3] = 0;
                }


                /*Set the last_note property, so the next note can glide from
                 * it if glide is turned on*/
                plugin_data->sv_last_note =
                        (plugin_data->data[f_voice]->note_f);


                plugin_data->active_polyfx_count[f_voice] = 0;
                //Determine which PolyFX have been enabled
                for(plugin_data->i_dst = 0;
                    (plugin_data->i_dst) < WAYV_MODULAR_POLYFX_COUNT;
                    plugin_data->i_dst = (plugin_data->i_dst) + 1)
                {
                    int f_pfx_combobox_index =
                        (int)(*(plugin_data->fx_combobox[0][(plugin_data->i_dst)]));
                    plugin_data->data[f_voice]->fx_func_ptr[(plugin_data->i_dst)] =
                        g_mf3_get_function_pointer(f_pfx_combobox_index);

                    if(f_pfx_combobox_index != 0)
                    {
                        plugin_data->active_polyfx[f_voice]
                                [(plugin_data->active_polyfx_count[f_voice])] =
                                (plugin_data->i_dst);
                        plugin_data->active_polyfx_count[f_voice] =
                                (plugin_data->active_polyfx_count[f_voice]) + 1;
                    }
                }

                /*Calculate an index of which mod_matrix controls to process.
                 * This saves expensive iterations and if/then logic in
                 * the main loop*/
                for(plugin_data->i_fx_grps = 0;
                    (plugin_data->i_fx_grps) < WAYV_EFFECTS_GROUPS_COUNT;
                    plugin_data->i_fx_grps = (plugin_data->i_fx_grps) + 1)
                {
                    for(plugin_data->i_dst = 0;
                        (plugin_data->i_dst) < (plugin_data->active_polyfx_count[f_voice]);
                        plugin_data->i_dst = (plugin_data->i_dst) + 1)
                    {
                        plugin_data->polyfx_mod_counts[f_voice]
                                [(plugin_data->active_polyfx[f_voice]
                                [(plugin_data->i_dst)])] = 0;

                        for(plugin_data->i_src = 0;
                                (plugin_data->i_src) < WAYV_MODULATOR_COUNT;
                                plugin_data->i_src = (plugin_data->i_src) + 1)
                        {
                            for(plugin_data->i_ctrl = 0;
                                (plugin_data->i_ctrl) < WAYV_CONTROLS_PER_MOD_EFFECT;
                                plugin_data->i_ctrl = (plugin_data->i_ctrl) + 1)
                            {
                                if((*(plugin_data->polyfx_mod_matrix[(plugin_data->i_fx_grps)][(plugin_data->active_polyfx[f_voice][(plugin_data->i_dst)])][(plugin_data->i_src)][(plugin_data->i_ctrl)])) != 0)
                                {
                                    plugin_data->polyfx_mod_ctrl_indexes[f_voice][(plugin_data->active_polyfx[f_voice][(plugin_data->i_dst)])][(plugin_data->polyfx_mod_counts[f_voice][(plugin_data->active_polyfx[f_voice][(plugin_data->i_dst)])])] = (plugin_data->i_ctrl);
                                    plugin_data->polyfx_mod_src_index[f_voice][(plugin_data->active_polyfx[f_voice][(plugin_data->i_dst)])][(plugin_data->polyfx_mod_counts[f_voice][(plugin_data->active_polyfx[f_voice][(plugin_data->i_dst)])])] = (plugin_data->i_src);
                                    plugin_data->polyfx_mod_matrix_values[f_voice][(plugin_data->active_polyfx[f_voice][(plugin_data->i_dst)])][(plugin_data->polyfx_mod_counts[f_voice][(plugin_data->active_polyfx[f_voice][(plugin_data->i_dst)])])] =
                                            (*(plugin_data->polyfx_mod_matrix[(plugin_data->i_fx_grps)][(plugin_data->active_polyfx[f_voice][(plugin_data->i_dst)])][(plugin_data->i_src)][(plugin_data->i_ctrl)])) * .01;

                                    plugin_data->polyfx_mod_counts[f_voice][(plugin_data->active_polyfx[f_voice][(plugin_data->i_dst)])] = (plugin_data->polyfx_mod_counts[f_voice][(plugin_data->active_polyfx[f_voice][(plugin_data->i_dst)])]) + 1;
                                }
                            }
                        }
                    }
                }
                //Get the noise function pointer
                plugin_data->data[f_voice]->noise_func_ptr =
                        fp_get_noise_func_ptr((int)(*(plugin_data->noise_type)));

                f_i = 0;

                while(f_i < 4)
                {
                    plugin_data->data[f_voice]->osc_uni_spread[f_i] =
                        (*plugin_data->osc_uni_spread[f_i]) * 0.01f;

                    int f_i2 = 0;
                    while(f_i2 < 4)
                    {
                        plugin_data->data[f_voice]->osc_fm[f_i][f_i2] =
                                (*plugin_data->osc_fm[f_i][f_i2]) * 0.005f;
                        f_i2++;
                    }
                    f_i++;
                }

                v_adsr_retrigger(plugin_data->data[f_voice]->adsr_amp);
                v_adsr_retrigger(plugin_data->data[f_voice]->adsr_filter);
                v_lfs_sync(plugin_data->data[f_voice]->lfo1,
                           *plugin_data->lfo_phase * 0.01f,
                           *plugin_data->lfo_type);

                float f_attack_a = (*(plugin_data->attack) * .01);
                f_attack_a *= f_attack_a;
                float f_decay_a = (*(plugin_data->decay) * .01);
                f_decay_a *= f_decay_a;
                float f_release_a = (*(plugin_data->release) * .01);
                f_release_a *= f_release_a;

                v_adsr_set_adsr_db(plugin_data->data[f_voice]->adsr_amp,
                        f_attack_a, f_decay_a, (*(plugin_data->sustain)),
                        f_release_a);

                float f_attack_f = (*(plugin_data->attack_f) * .01);
                f_attack_f *= f_attack_f;
                float f_decay_f = (*(plugin_data->decay_f) * .01);
                f_decay_f *= f_decay_f;
                float f_release_f = (*(plugin_data->release_f) * .01);
                f_release_f *= f_release_f;

                v_adsr_set_adsr(plugin_data->data[f_voice]->adsr_filter,
                        f_attack_f, f_decay_f,
                        (*(plugin_data->sustain_f) * .01), f_release_f);

                /*Retrigger the pitch envelope*/
                v_rmp_retrigger_curve((plugin_data->data[f_voice]->ramp_env),
                        (*(plugin_data->pitch_env_time) * .01), 1.0f,
                        (*plugin_data->ramp_curve) * 0.01f);

                plugin_data->data[f_voice]->noise_amp =
                        f_db_to_linear(*(plugin_data->noise_amp),
                        plugin_data->mono_modules->amp_ptr);

                plugin_data->data[f_voice]->perc_env_on =
                        (int)(*plugin_data->perc_env_on);

                if(plugin_data->data[f_voice]->perc_env_on)
                {
                    v_pnv_set(plugin_data->data[f_voice]->perc_env,
                            (*plugin_data->perc_env_time1) * 0.001f,
                            (*plugin_data->perc_env_pitch1),
                            (*plugin_data->perc_env_time2) * 0.001f,
                            (*plugin_data->perc_env_pitch2),
                            plugin_data->data[f_voice]->note_f);
                }
            }
            /*0 velocity, the same as note-off*/
            else
            {
                v_voc_note_off(plugin_data->voices,
                    events[(plugin_data->event_pos)].note,
                    (plugin_data->sampleNo),
                    (events[(plugin_data->event_pos)].tick));
            }
        }
        else if (events[(plugin_data->event_pos)].type == PYDAW_EVENT_NOTEOFF)
        {
            v_voc_note_off(plugin_data->voices,
                events[(plugin_data->event_pos)].note, (plugin_data->sampleNo),
                (events[(plugin_data->event_pos)].tick));
        }
        else if (events[plugin_data->event_pos].type == PYDAW_EVENT_CONTROLLER)
        {
            //The host already filters out messages for other instruments
            if (events[plugin_data->event_pos].plugin_index != -1)
            {
                assert(events[plugin_data->event_pos].port >=
                        WAYV_FIRST_CONTROL_PORT &&
                        events[plugin_data->event_pos].port < WAYV_COUNT);

                plugin_data->midi_event_types[plugin_data->midi_event_count] =
                        PYDAW_EVENT_CONTROLLER;
                plugin_data->midi_event_ticks[plugin_data->midi_event_count] =
                        events[plugin_data->event_pos].tick;
                plugin_data->midi_event_ports[plugin_data->midi_event_count] =
                        events[plugin_data->event_pos].port;
                plugin_data->midi_event_values[plugin_data->midi_event_count] =
                        events[plugin_data->event_pos].value;
                plugin_data->midi_event_count++;
            }
        }
        else if (events[(plugin_data->event_pos)].type == PYDAW_EVENT_PITCHBEND)
        {
            plugin_data->midi_event_types[plugin_data->midi_event_count] =
                    PYDAW_EVENT_PITCHBEND;
            plugin_data->midi_event_ticks[plugin_data->midi_event_count] =
                    events[plugin_data->event_pos].tick;
            plugin_data->midi_event_values[plugin_data->midi_event_count] =
                    0.00012207 * events[plugin_data->event_pos].value;
            plugin_data->midi_event_count++;
        }
    }

    /*Clear the output buffer*/
    plugin_data->i_iterator = 0;

    while((plugin_data->i_iterator) < sample_count)
    {
        plugin_data->output0[(plugin_data->i_iterator)] = 0.0f;
        plugin_data->output1[(plugin_data->i_iterator)] = 0.0f;

        while(midi_event_pos < plugin_data->midi_event_count &&
                plugin_data->midi_event_ticks[midi_event_pos] == plugin_data->i_iterator)
        {
            if(plugin_data->midi_event_types[midi_event_pos] == PYDAW_EVENT_PITCHBEND)
            {
                plugin_data->sv_pitch_bend_value =
                        plugin_data->midi_event_values[midi_event_pos];
            }
            else if(plugin_data->midi_event_types[midi_event_pos] == PYDAW_EVENT_CONTROLLER)
            {
                plugin_data->port_table[plugin_data->midi_event_ports[midi_event_pos]] =
                        plugin_data->midi_event_values[midi_event_pos];
            }

            midi_event_pos++;
        }

        if(plugin_data->mono_modules->reset_wavetables)
        {
            int f_voice = 0;

            int f_osc_type1 = (int)(*plugin_data->osc1type) - 1;
            int f_osc_type2 = (int)(*plugin_data->osc2type) - 1;
            int f_osc_type3 = (int)(*plugin_data->osc3type) - 1;
            int f_osc_type4 = (int)(*plugin_data->osc4type) - 1;

            while(f_voice < WAYV_POLYPHONY)
            {
                if(f_osc_type1 >= 0)
                {
                    v_osc_wav_set_waveform(
                            plugin_data->data[f_voice]->osc_wavtable[0],
                            plugin_data->mono_modules->wavetables->
                                tables[f_osc_type1]->wavetable,
                            plugin_data->mono_modules->wavetables->
                                tables[f_osc_type1]->length);
                }

                if(f_osc_type2 >= 0)
                {
                    v_osc_wav_set_waveform(
                            plugin_data->data[f_voice]->osc_wavtable[1],
                            plugin_data->mono_modules->wavetables->
                                tables[f_osc_type2]->wavetable,
                            plugin_data->mono_modules->wavetables->
                                tables[f_osc_type2]->length);
                }

                if(f_osc_type3 >= 0)
                {
                    v_osc_wav_set_waveform(
                            plugin_data->data[f_voice]->osc_wavtable[2],
                            plugin_data->mono_modules->wavetables->
                                tables[f_osc_type3]->wavetable,
                            plugin_data->mono_modules->wavetables->
                                tables[f_osc_type3]->length);
                }

                if(f_osc_type4 >= 0)
                {
                    v_osc_wav_set_waveform(
                            plugin_data->data[f_voice]->osc_wavtable[3],
                            plugin_data->mono_modules->wavetables->
                                tables[f_osc_type4]->wavetable,
                            plugin_data->mono_modules->wavetables->
                                tables[f_osc_type4]->length);
                }

                f_voice++;
            }

            plugin_data->mono_modules->reset_wavetables = 0;
        }

        v_sml_run(plugin_data->mono_modules->pitchbend_smoother,
                (plugin_data->sv_pitch_bend_value));

        v_sml_run(plugin_data->mono_modules->fm_macro_smoother[0],
                (*plugin_data->fm_macro[0] * 0.01f));

        v_sml_run(plugin_data->mono_modules->fm_macro_smoother[1],
                (*plugin_data->fm_macro[1] * 0.01f));

        plugin_data->i_run_poly_voice = 0;
        while ((plugin_data->i_run_poly_voice) < WAYV_POLYPHONY)
        {
            //if (data[voice].state != inactive)
            if((plugin_data->data[(plugin_data->i_run_poly_voice)]->adsr_main->stage) != 4)
            {
                v_run_wayv_voice(plugin_data,
                        plugin_data->voices->voices[(plugin_data->i_run_poly_voice)],
                        plugin_data->data[(plugin_data->i_run_poly_voice)],
                        plugin_data->output0,
                        plugin_data->output1,
                        plugin_data->i_iterator,
                        plugin_data->i_run_poly_voice
                        );
            }
            else
            {
                plugin_data->voices->voices[(plugin_data->i_run_poly_voice)].
                        n_state = note_state_off;
            }

            plugin_data->i_run_poly_voice = (plugin_data->i_run_poly_voice) + 1;
        }

        plugin_data->sampleNo++;
        plugin_data->i_iterator = (plugin_data->i_iterator) + 1;
    }

    //plugin_data->sampleNo += sample_count;
}

static void v_run_wayv_voice(t_wayv *plugin_data,
        t_voc_single_voice a_poly_voice, t_wayv_poly_voice *a_voice,
        PYFX_Data *out0, PYFX_Data *out1, int a_i, int a_voice_num)
{
    a_voice->i_voice = a_i;

    if((plugin_data->sampleNo) < (a_poly_voice.on))
    {
        return;
    }

    if (((plugin_data->sampleNo) == a_poly_voice.off) &&
            ((a_voice->adsr_main->stage) < 3))
    {
        if(a_poly_voice.n_state == note_state_killed)
        {
            v_wayv_poly_note_off(a_voice, 1);
        }
        else
        {
            v_wayv_poly_note_off(a_voice, 0);
        }
    }

    a_voice->current_sample = 0.0f;

    f_rmp_run_ramp(a_voice->glide_env);

    v_adsr_run_db(plugin_data->data[a_voice_num]->adsr_amp);

    v_adsr_run(plugin_data->data[a_voice_num]->adsr_filter);

    f_rmp_run_ramp_curve(plugin_data->data[a_voice_num]->ramp_env);

    //Set and run the LFO
    v_lfs_set(plugin_data->data[a_voice_num]->lfo1,
            (*(plugin_data->lfo_freq)) * .01);
    v_lfs_run(plugin_data->data[a_voice_num]->lfo1);

    a_voice->lfo_amount_output =
            (a_voice->lfo1->output) * ((*plugin_data->lfo_amount) * 0.01f);

    a_voice->lfo_amp_output =
            f_db_to_linear_fast((((*plugin_data->lfo_amp) *
            (a_voice->lfo_amount_output)) -
            (f_lms_abs((*plugin_data->lfo_amp)) * 0.5)),
            a_voice->amp_ptr);

    a_voice->lfo_pitch_output =
            (*plugin_data->lfo_pitch) * (a_voice->lfo_amount_output);

    if(a_voice->perc_env_on)
    {
        a_voice->base_pitch = f_pnv_run(a_voice->perc_env);
    }
    else
    {
        a_voice->base_pitch =
            (a_voice->glide_env->output_multiplied) +
            ((a_voice->ramp_env->output_multiplied) *
            (*plugin_data->pitch_env_amt))
            + (plugin_data->mono_modules->pitchbend_smoother->last_value  *
            (*(plugin_data->master_pb_amt))) + (a_voice->last_pitch) +
            (a_voice->lfo_pitch_output);
    }

    int f_osc_num = 0;

    while(f_osc_num < 4)
    {
        if(a_voice->osc_on[f_osc_num])
        {
            v_osc_wav_set_unison_pitch(
                    a_voice->osc_wavtable[f_osc_num],
                    (a_voice->osc_uni_spread[f_osc_num]),
                    ((a_voice->base_pitch) +
                    (*plugin_data->osc_pitch[f_osc_num]) +
                    ((*plugin_data->osc_tune[f_osc_num]) * 0.01f) ));

            int f_i = 0;
            while(f_i < 4)
            {
                a_voice->fm_osc_values[f_osc_num][f_i] =
                    a_voice->osc_fm[f_osc_num][f_i];
                f_i++;
            }

            f_i = 0;

            while(f_i < 2)
            {
                if(plugin_data->mono_modules->fm_macro_smoother[f_i]->last_value
                        > 0.0f)
                {
                    int f_i2 = 0;
                    while(f_i2 < 4)
                    {
                        a_voice->fm_osc_values[f_osc_num][f_i2] +=
                          ((*plugin_data->fm_macro_values[f_i][f_osc_num][f_i2]
                                * 0.005f) *
                            plugin_data->mono_modules->
                                fm_macro_smoother[f_i]->last_value);
                        f_i2++;
                    }
                }
                f_i++;
            }

            f_i = 0;

            while(f_i < 4)
            {
                if(a_voice->fm_osc_values[f_osc_num][f_i] < 0.0f)
                {
                    a_voice->fm_osc_values[f_osc_num][f_i] = 0.0f;
                }
                else if(a_voice->fm_osc_values[f_osc_num][f_i] > 0.5f)
                {
                    a_voice->fm_osc_values[f_osc_num][f_i] = 0.5f;
                }

                v_osc_wav_apply_fm(a_voice->osc_wavtable[f_osc_num],
                    a_voice->fm_last[f_i],
                    a_voice->fm_osc_values[f_osc_num][f_i]);
                f_i++;
            }

            if(a_voice->adsr_amp_on[f_osc_num])
            {
                v_adsr_run_db(a_voice->adsr_amp_osc[f_osc_num]);
                a_voice->fm_last[f_osc_num] =
                    f_osc_wav_run_unison(a_voice->osc_wavtable[f_osc_num])
                        * (a_voice->adsr_amp_osc[f_osc_num]->output);
            }
            else
            {
                a_voice->fm_last[f_osc_num] =
                        f_osc_wav_run_unison(a_voice->osc_wavtable[f_osc_num]);
            }

            if(a_voice->osc_audible[f_osc_num])
            {
                a_voice->current_sample +=
                        (a_voice->fm_last[f_osc_num]) *
                        (a_voice->osc_linamp[f_osc_num]);
            }
        }

        f_osc_num++;
    }


    a_voice->current_sample += (a_voice->noise_func_ptr(a_voice->white_noise1) *
            (a_voice->noise_linamp)); // noise

    v_adsr_run_db(a_voice->adsr_main);

    a_voice->current_sample = (a_voice->current_sample) * (a_voice->amp) *
            (a_voice->lfo_amp_output);

    a_voice->modulex_current_sample[0] = (a_voice->current_sample);
    a_voice->modulex_current_sample[1] = (a_voice->current_sample);

    //Modular PolyFX, processed from the index created during note_on
    for(plugin_data->i_dst = 0;
        (plugin_data->i_dst) < (plugin_data->active_polyfx_count[a_voice_num]);
        plugin_data->i_dst = (plugin_data->i_dst) + 1)
    {
        v_mf3_set(a_voice->multieffect[(plugin_data->active_polyfx[a_voice_num][(plugin_data->i_dst)])],
            *(plugin_data->pfx_mod_knob[0][(plugin_data->active_polyfx[a_voice_num][(plugin_data->i_dst)])][0]),
                *(plugin_data->pfx_mod_knob[0][(plugin_data->active_polyfx[a_voice_num][(plugin_data->i_dst)])][1]),
                *(plugin_data->pfx_mod_knob[0][(plugin_data->active_polyfx[a_voice_num][(plugin_data->i_dst)])][2]));

        int f_mod_test;

        for(f_mod_test = 0;
            f_mod_test < (plugin_data->polyfx_mod_counts[a_voice_num][(plugin_data->active_polyfx[a_voice_num][(plugin_data->i_dst)])]);
            f_mod_test++)
        {
            v_mf3_mod_single(
                    a_voice->multieffect[(plugin_data->active_polyfx[a_voice_num][(plugin_data->i_dst)])],
                    *(a_voice->modulator_outputs[(plugin_data->polyfx_mod_src_index[a_voice_num][(plugin_data->active_polyfx[a_voice_num][(plugin_data->i_dst)])][f_mod_test])]),
                    (plugin_data->polyfx_mod_matrix_values[a_voice_num][(plugin_data->active_polyfx[a_voice_num][(plugin_data->i_dst)])][f_mod_test]),
                    (plugin_data->polyfx_mod_ctrl_indexes[a_voice_num][(plugin_data->active_polyfx[a_voice_num][(plugin_data->i_dst)])][f_mod_test])
                    );
        }

        a_voice->fx_func_ptr[(plugin_data->active_polyfx[a_voice_num][(plugin_data->i_dst)])](
                a_voice->multieffect[(plugin_data->active_polyfx[a_voice_num][(plugin_data->i_dst)])],
                (a_voice->modulex_current_sample[0]), (a_voice->modulex_current_sample[1]));

        a_voice->modulex_current_sample[0] =
            a_voice->multieffect[(plugin_data->active_polyfx[a_voice_num]
                [(plugin_data->i_dst)])]->output0;
        a_voice->modulex_current_sample[1] =
            a_voice->multieffect[(plugin_data->active_polyfx[a_voice_num]
                [(plugin_data->i_dst)])]->output1;

    }

    /*Run the envelope and assign to the output buffers*/
    out0[(a_voice->i_voice)] += (a_voice->modulex_current_sample[0]) *
            (a_voice->adsr_main->output) * (a_voice->master_vol_lin);
    out1[(a_voice->i_voice)] += (a_voice->modulex_current_sample[1]) *
            (a_voice->adsr_main->output) * (a_voice->master_vol_lin);

}


float * f_char_to_wavetable(const char * a_char)
{
    float * f_result;

    if(posix_memalign((void**)&f_result, 16, (sizeof(float) * 1024)) != 0)
    {
        return 0;
    }

    t_1d_char_array * f_arr = c_split_str(a_char, '|', 1025, 32);

    int f_i = 1;

    int f_count = atoi(f_arr->array[0]);
    printf("%i\n", f_count);

    while(f_i < 1025)
    {
        f_result[f_i - 1] = atof(f_arr->array[f_i]);
        f_i++;
    }

    g_free_1d_char_array(f_arr);

    return f_result;
}

char *c_wayv_configure(PYFX_Handle instance, const char *key,
        const char *value, pthread_mutex_t * a_mutex)
{
    t_wayv *plugin_data = (t_wayv*)instance;

    if (!strcmp(key, "wayv_add_eng0"))
    {
        float * f_table = f_char_to_wavetable(value);
        v_wt_set_wavetable(plugin_data->mono_modules->wavetables, 17, f_table,
                1024, a_mutex, &plugin_data->mono_modules->reset_wavetables);
    }
    else if (!strcmp(key, "wayv_add_eng1"))
    {
        float * f_table = f_char_to_wavetable(value);
        v_wt_set_wavetable(plugin_data->mono_modules->wavetables, 18, f_table,
                1024, a_mutex, &plugin_data->mono_modules->reset_wavetables);
    }
    else if (!strcmp(key, "wayv_add_eng2"))
    {
        float * f_table = f_char_to_wavetable(value);
        v_wt_set_wavetable(plugin_data->mono_modules->wavetables, 19, f_table,
                1024, a_mutex, &plugin_data->mono_modules->reset_wavetables);
    }
    else
    {
        printf("Way-V unhandled configure key %s\n", key);
    }

    return NULL;
}


const PYFX_Descriptor *wayv_PYFX_descriptor(int index)
{
    PYFX_Descriptor *LMSLDescriptor = NULL;

    PYFX_PortDescriptor *port_descriptors;
    PYFX_PortRangeHint *port_range_hints;

    LMSLDescriptor =
	(PYFX_Descriptor *) malloc(sizeof(PYFX_Descriptor));
    if (LMSLDescriptor) {
        LMSLDescriptor->UniqueID = 987564;
	LMSLDescriptor->Name = "Way-V";
	LMSLDescriptor->Maker = "PyDAW Team";
	LMSLDescriptor->Copyright = "GNU GPL v3";
	LMSLDescriptor->PortCount = WAYV_COUNT;

	port_descriptors = (PYFX_PortDescriptor *)
				calloc(LMSLDescriptor->PortCount, sizeof
						(PYFX_PortDescriptor));
	LMSLDescriptor->PortDescriptors =
	    (const PYFX_PortDescriptor *) port_descriptors;

	port_range_hints = (PYFX_PortRangeHint *)
				calloc(LMSLDescriptor->PortCount, sizeof
						(PYFX_PortRangeHint));
	LMSLDescriptor->PortRangeHints =
	    (const PYFX_PortRangeHint *) port_range_hints;


	port_descriptors[WAYV_ATTACK_MAIN] = 1;
	port_range_hints[WAYV_ATTACK_MAIN].DefaultValue = 10.0f;
	port_range_hints[WAYV_ATTACK_MAIN].LowerBound = 0.0f;
	port_range_hints[WAYV_ATTACK_MAIN].UpperBound = 200.0f;

	port_descriptors[WAYV_DECAY_MAIN] = 1;
	port_range_hints[WAYV_DECAY_MAIN].DefaultValue = 50.0f;
	port_range_hints[WAYV_DECAY_MAIN].LowerBound = 10.0f;
	port_range_hints[WAYV_DECAY_MAIN].UpperBound = 200.0f;

	port_descriptors[WAYV_SUSTAIN_MAIN] = 1;
	port_range_hints[WAYV_SUSTAIN_MAIN].DefaultValue = 0.0f;
	port_range_hints[WAYV_SUSTAIN_MAIN].LowerBound = -30.0f;
	port_range_hints[WAYV_SUSTAIN_MAIN].UpperBound = 0.0f;

	port_descriptors[WAYV_RELEASE_MAIN] = 1;
	port_range_hints[WAYV_RELEASE_MAIN].DefaultValue = 50.0f;
	port_range_hints[WAYV_RELEASE_MAIN].LowerBound = 10.0f;
	port_range_hints[WAYV_RELEASE_MAIN].UpperBound = 400.0f;

	port_descriptors[WAYV_ATTACK1] = 1;
	port_range_hints[WAYV_ATTACK1].DefaultValue = 10.0f;
	port_range_hints[WAYV_ATTACK1].LowerBound = 0.0f;
	port_range_hints[WAYV_ATTACK1].UpperBound = 200.0f;

	port_descriptors[WAYV_DECAY1] = 1;
	port_range_hints[WAYV_DECAY1].DefaultValue = 50.0f;
	port_range_hints[WAYV_DECAY1].LowerBound = 10.0f;
	port_range_hints[WAYV_DECAY1].UpperBound = 200.0f;

	port_descriptors[WAYV_SUSTAIN1] = 1;
	port_range_hints[WAYV_SUSTAIN1].DefaultValue = 0.0f;
	port_range_hints[WAYV_SUSTAIN1].LowerBound = -30.0f;
	port_range_hints[WAYV_SUSTAIN1].UpperBound = 0.0f;

	port_descriptors[WAYV_RELEASE1] = 1;
	port_range_hints[WAYV_RELEASE1].DefaultValue = 50.0f;
	port_range_hints[WAYV_RELEASE1].LowerBound = 10.0f;
	port_range_hints[WAYV_RELEASE1].UpperBound = 400.0f;

	port_descriptors[WAYV_ATTACK2] = 1;
	port_range_hints[WAYV_ATTACK2].DefaultValue = 10.0f;
	port_range_hints[WAYV_ATTACK2].LowerBound = 0.0f;
	port_range_hints[WAYV_ATTACK2].UpperBound = 200.0f;

	port_descriptors[WAYV_DECAY2] = 1;
	port_range_hints[WAYV_DECAY2].DefaultValue = 50.0f;
	port_range_hints[WAYV_DECAY2].LowerBound = 10.0f;
	port_range_hints[WAYV_DECAY2].UpperBound = 200.0f;

	port_descriptors[WAYV_SUSTAIN2] = 1;
	port_range_hints[WAYV_SUSTAIN2].DefaultValue = 0.0f;
	port_range_hints[WAYV_SUSTAIN2].LowerBound = -30.0f;
	port_range_hints[WAYV_SUSTAIN2].UpperBound = 0.0f;

	port_descriptors[WAYV_RELEASE2] = 1;
	port_range_hints[WAYV_RELEASE2].DefaultValue = 50.0f;
	port_range_hints[WAYV_RELEASE2].LowerBound = 10.0f;
	port_range_hints[WAYV_RELEASE2].UpperBound = 400.0f;

	port_descriptors[WAYV_NOISE_AMP] = 1;
	port_range_hints[WAYV_NOISE_AMP].DefaultValue = -30.0f;
	port_range_hints[WAYV_NOISE_AMP].LowerBound =  -60.0f;
	port_range_hints[WAYV_NOISE_AMP].UpperBound =  0.0f;

	port_descriptors[WAYV_OSC1_TYPE] = 1;
	port_range_hints[WAYV_OSC1_TYPE].DefaultValue = 1.0f;
	port_range_hints[WAYV_OSC1_TYPE].LowerBound =  0.0f;
	port_range_hints[WAYV_OSC1_TYPE].UpperBound =  (float)WT_TOTAL_WAVETABLE_COUNT;

	port_descriptors[WAYV_OSC1_PITCH] = 1;
	port_range_hints[WAYV_OSC1_PITCH].DefaultValue = 0.0f;
	port_range_hints[WAYV_OSC1_PITCH].LowerBound = -72.0f;
	port_range_hints[WAYV_OSC1_PITCH].UpperBound = 72.0f;

	port_descriptors[WAYV_OSC1_TUNE] = 1;
	port_range_hints[WAYV_OSC1_TUNE].DefaultValue = 0.0f;
	port_range_hints[WAYV_OSC1_TUNE].LowerBound = -100.0f;
	port_range_hints[WAYV_OSC1_TUNE].UpperBound =  100.0f;

	port_descriptors[WAYV_OSC1_VOLUME] = 1;
	port_range_hints[WAYV_OSC1_VOLUME].DefaultValue = 0.0f;
	port_range_hints[WAYV_OSC1_VOLUME].LowerBound =  -30.0f;
	port_range_hints[WAYV_OSC1_VOLUME].UpperBound =  0.0f;

	port_descriptors[WAYV_OSC2_TYPE] = 1;
	port_range_hints[WAYV_OSC2_TYPE].DefaultValue = 0.0f;
	port_range_hints[WAYV_OSC2_TYPE].LowerBound =  0.0f;
	port_range_hints[WAYV_OSC2_TYPE].UpperBound =  (float)WT_TOTAL_WAVETABLE_COUNT;

	port_descriptors[WAYV_OSC2_PITCH] = 1;
	port_range_hints[WAYV_OSC2_PITCH].DefaultValue = 0.0f;
	port_range_hints[WAYV_OSC2_PITCH].LowerBound = -72.0f;
	port_range_hints[WAYV_OSC2_PITCH].UpperBound = 72.0f;

	port_descriptors[WAYV_OSC2_TUNE] = 1;
	port_range_hints[WAYV_OSC2_TUNE].DefaultValue = 0.0f;
	port_range_hints[WAYV_OSC2_TUNE].LowerBound = -100.0f;
	port_range_hints[WAYV_OSC2_TUNE].UpperBound = 100.0f;

	port_descriptors[WAYV_OSC2_VOLUME] = 1;
	port_range_hints[WAYV_OSC2_VOLUME].DefaultValue = 0.0f;
	port_range_hints[WAYV_OSC2_VOLUME].LowerBound =  -30.0f;
	port_range_hints[WAYV_OSC2_VOLUME].UpperBound =  0.0f;

	port_descriptors[WAYV_MASTER_VOLUME] = 1;
	port_range_hints[WAYV_MASTER_VOLUME].DefaultValue = -6.0f;
	port_range_hints[WAYV_MASTER_VOLUME].LowerBound =  -30.0f;
	port_range_hints[WAYV_MASTER_VOLUME].UpperBound =  12.0f;

	port_descriptors[WAYV_OSC1_UNISON_VOICES] = 1;
	port_range_hints[WAYV_OSC1_UNISON_VOICES].DefaultValue = 4.0f;
	port_range_hints[WAYV_OSC1_UNISON_VOICES].LowerBound =  1.0f;
	port_range_hints[WAYV_OSC1_UNISON_VOICES].UpperBound =  7.0f;

	port_descriptors[WAYV_OSC1_UNISON_SPREAD] = 1;
	port_range_hints[WAYV_OSC1_UNISON_SPREAD].DefaultValue = 50.0f;
	port_range_hints[WAYV_OSC1_UNISON_SPREAD].LowerBound =  0.0f;
	port_range_hints[WAYV_OSC1_UNISON_SPREAD].UpperBound =  100.0f;

	port_descriptors[WAYV_MASTER_GLIDE] = 1;
	port_range_hints[WAYV_MASTER_GLIDE].DefaultValue = 0.0f;
	port_range_hints[WAYV_MASTER_GLIDE].LowerBound =  0.0f;
	port_range_hints[WAYV_MASTER_GLIDE].UpperBound =  200.0f;

	port_descriptors[WAYV_MASTER_PITCHBEND_AMT] = 1;
	port_range_hints[WAYV_MASTER_PITCHBEND_AMT].DefaultValue = 18.0f;
	port_range_hints[WAYV_MASTER_PITCHBEND_AMT].LowerBound =  1.0f;
	port_range_hints[WAYV_MASTER_PITCHBEND_AMT].UpperBound =  36.0f;

	port_descriptors[WAYV_ATTACK_PFX1] = 1;
	port_range_hints[WAYV_ATTACK_PFX1].DefaultValue = 10.0f;
	port_range_hints[WAYV_ATTACK_PFX1].LowerBound = 0.0f;
	port_range_hints[WAYV_ATTACK_PFX1].UpperBound = 200.0f;

	port_descriptors[WAYV_DECAY_PFX1] = 1;
	port_range_hints[WAYV_DECAY_PFX1].DefaultValue = 50.0f;
	port_range_hints[WAYV_DECAY_PFX1].LowerBound = 10.0f;
	port_range_hints[WAYV_DECAY_PFX1].UpperBound = 200.0f;

	port_descriptors[WAYV_SUSTAIN_PFX1] = 1;
	port_range_hints[WAYV_SUSTAIN_PFX1].DefaultValue = 0.0f;
	port_range_hints[WAYV_SUSTAIN_PFX1].LowerBound = -60.0f;
	port_range_hints[WAYV_SUSTAIN_PFX1].UpperBound = 0.0f;

	port_descriptors[WAYV_RELEASE_PFX1] = 1;
	port_range_hints[WAYV_RELEASE_PFX1].DefaultValue = 50.0f;
	port_range_hints[WAYV_RELEASE_PFX1].LowerBound = 10.0f;
	port_range_hints[WAYV_RELEASE_PFX1].UpperBound = 400.0f;

	port_descriptors[WAYV_ATTACK_PFX2] = 1;
	port_range_hints[WAYV_ATTACK_PFX2].DefaultValue = 10.0f;
	port_range_hints[WAYV_ATTACK_PFX2].LowerBound = 0.0f;
	port_range_hints[WAYV_ATTACK_PFX2].UpperBound = 200.0f;

	port_descriptors[WAYV_DECAY_PFX2]= 1;
	port_range_hints[WAYV_DECAY_PFX2].DefaultValue = 50.0f;
	port_range_hints[WAYV_DECAY_PFX2].LowerBound = 10.0f;
	port_range_hints[WAYV_DECAY_PFX2].UpperBound = 200.0f;

	port_descriptors[WAYV_SUSTAIN_PFX2] = 1;
	port_range_hints[WAYV_SUSTAIN_PFX2].DefaultValue = 100.0f;
	port_range_hints[WAYV_SUSTAIN_PFX2].LowerBound = 0.0f;
	port_range_hints[WAYV_SUSTAIN_PFX2].UpperBound = 100.0f;

	port_descriptors[WAYV_RELEASE_PFX2]= 1;
	port_range_hints[WAYV_RELEASE_PFX2].DefaultValue = 50.0f;
	port_range_hints[WAYV_RELEASE_PFX2].LowerBound = 10.0f;
	port_range_hints[WAYV_RELEASE_PFX2].UpperBound = 400.0f;

	port_descriptors[WAYV_NOISE_AMP]= 1;
	port_range_hints[WAYV_NOISE_AMP].DefaultValue = -60.0f;
	port_range_hints[WAYV_NOISE_AMP].LowerBound =  -60.0f;
	port_range_hints[WAYV_NOISE_AMP].UpperBound =  0.0f;

	port_descriptors[WAYV_RAMP_ENV_TIME]= 1;
	port_range_hints[WAYV_RAMP_ENV_TIME].DefaultValue = 100.0f;
	port_range_hints[WAYV_RAMP_ENV_TIME].LowerBound = 0.0f;
	port_range_hints[WAYV_RAMP_ENV_TIME].UpperBound = 600.0f;

	port_descriptors[WAYV_LFO_FREQ]= 1;
	port_range_hints[WAYV_LFO_FREQ].DefaultValue = 200.0f;
	port_range_hints[WAYV_LFO_FREQ].LowerBound = 10;
	port_range_hints[WAYV_LFO_FREQ].UpperBound = 1600;

	port_descriptors[WAYV_LFO_TYPE]= 1;
	port_range_hints[WAYV_LFO_TYPE].DefaultValue = 0.0f;
	port_range_hints[WAYV_LFO_TYPE].LowerBound = 0.0f;
	port_range_hints[WAYV_LFO_TYPE].UpperBound = 2.0f;

        port_descriptors[WAYV_FX0_KNOB0] = 1;
	port_range_hints[WAYV_FX0_KNOB0].DefaultValue = 64.0f;
	port_range_hints[WAYV_FX0_KNOB0].LowerBound =  0.0f;
	port_range_hints[WAYV_FX0_KNOB0].UpperBound =  127.0f;

	port_descriptors[WAYV_FX0_KNOB1] = 1;
	port_range_hints[WAYV_FX0_KNOB1].DefaultValue = 64.0f;
	port_range_hints[WAYV_FX0_KNOB1].LowerBound =  0.0f;
	port_range_hints[WAYV_FX0_KNOB1].UpperBound =  127.0f;

	port_descriptors[WAYV_FX0_KNOB2] = 1;
	port_range_hints[WAYV_FX0_KNOB2].DefaultValue = 64.0f;
	port_range_hints[WAYV_FX0_KNOB2].LowerBound =  0.0f;
	port_range_hints[WAYV_FX0_KNOB2].UpperBound =  127.0f;

	port_descriptors[WAYV_FX0_COMBOBOX] = 1;
	port_range_hints[WAYV_FX0_COMBOBOX].DefaultValue = 0.0f;
	port_range_hints[WAYV_FX0_COMBOBOX].LowerBound =  0.0f;
	port_range_hints[WAYV_FX0_COMBOBOX].UpperBound =  MULTIFX3KNOB_MAX_INDEX;

	port_descriptors[WAYV_FX1_KNOB0] = 1;
	port_range_hints[WAYV_FX1_KNOB0].DefaultValue = 64.0f;
	port_range_hints[WAYV_FX1_KNOB0].LowerBound =  0.0f;
	port_range_hints[WAYV_FX1_KNOB0].UpperBound =  127.0f;

	port_descriptors[WAYV_FX1_KNOB1] = 1;
	port_range_hints[WAYV_FX1_KNOB1].DefaultValue = 64.0f;
	port_range_hints[WAYV_FX1_KNOB1].LowerBound =  0.0f;
	port_range_hints[WAYV_FX1_KNOB1].UpperBound =  127.0f;

	port_descriptors[WAYV_FX1_KNOB2] = 1;
	port_range_hints[WAYV_FX1_KNOB2].DefaultValue = 64.0f;
	port_range_hints[WAYV_FX1_KNOB2].LowerBound =  0.0f;
	port_range_hints[WAYV_FX1_KNOB2].UpperBound =  127.0f;

	port_descriptors[WAYV_FX1_COMBOBOX] = 1;
	port_range_hints[WAYV_FX1_COMBOBOX].DefaultValue = 0.0f;
	port_range_hints[WAYV_FX1_COMBOBOX].LowerBound =  0.0f;
	port_range_hints[WAYV_FX1_COMBOBOX].UpperBound =  MULTIFX3KNOB_MAX_INDEX;

        port_descriptors[WAYV_FX2_KNOB0] = 1;
	port_range_hints[WAYV_FX2_KNOB0].DefaultValue = 64.0f;
	port_range_hints[WAYV_FX2_KNOB0].LowerBound =  0.0f;
	port_range_hints[WAYV_FX2_KNOB0].UpperBound =  127.0f;

	port_descriptors[WAYV_FX2_KNOB1] = 1;
	port_range_hints[WAYV_FX2_KNOB1].DefaultValue = 64.0f;
	port_range_hints[WAYV_FX2_KNOB1].LowerBound =  0.0f;
	port_range_hints[WAYV_FX2_KNOB1].UpperBound =  127.0f;

	port_descriptors[WAYV_FX2_KNOB2] = 1;
	port_range_hints[WAYV_FX2_KNOB2].DefaultValue = 64.0f;
	port_range_hints[WAYV_FX2_KNOB2].LowerBound =  0.0f;
	port_range_hints[WAYV_FX2_KNOB2].UpperBound =  127.0f;

	port_descriptors[WAYV_FX2_COMBOBOX] = 1;
	port_range_hints[WAYV_FX2_COMBOBOX].DefaultValue = 0.0f;
	port_range_hints[WAYV_FX2_COMBOBOX].LowerBound =  0.0f;
	port_range_hints[WAYV_FX2_COMBOBOX].UpperBound =  MULTIFX3KNOB_MAX_INDEX;

	port_descriptors[WAYV_FX3_KNOB0] = 1;
	port_range_hints[WAYV_FX3_KNOB0].DefaultValue = 64.0f;
	port_range_hints[WAYV_FX3_KNOB0].LowerBound =  0.0f;
	port_range_hints[WAYV_FX3_KNOB0].UpperBound =  127.0f;

	port_descriptors[WAYV_FX3_KNOB1] = 1;
	port_range_hints[WAYV_FX3_KNOB1].DefaultValue = 64.0f;
	port_range_hints[WAYV_FX3_KNOB1].LowerBound =  0.0f;
	port_range_hints[WAYV_FX3_KNOB1].UpperBound =  127.0f;

	port_descriptors[WAYV_FX3_KNOB2] = 1;
	port_range_hints[WAYV_FX3_KNOB2].DefaultValue = 64.0f;
	port_range_hints[WAYV_FX3_KNOB2].LowerBound =  0.0f;
	port_range_hints[WAYV_FX3_KNOB2].UpperBound =  127.0f;

	port_descriptors[WAYV_FX3_COMBOBOX] = 1;
	port_range_hints[WAYV_FX3_COMBOBOX].DefaultValue = 0.0f;
	port_range_hints[WAYV_FX3_COMBOBOX].LowerBound =  0.0f;
	port_range_hints[WAYV_FX3_COMBOBOX].UpperBound =  MULTIFX3KNOB_MAX_INDEX;

        port_descriptors[WAVV_PFXMATRIX_GRP0DST0SRC0CTRL0] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC0CTRL0].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC0CTRL0].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC0CTRL0].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST0SRC0CTRL1] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC0CTRL1].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC0CTRL1].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC0CTRL1].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST0SRC0CTRL2] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC0CTRL2].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC0CTRL2].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC0CTRL2].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST0SRC1CTRL0] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC1CTRL0].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC1CTRL0].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC1CTRL0].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST0SRC1CTRL1] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC1CTRL1].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC1CTRL1].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC1CTRL1].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST0SRC1CTRL2] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC1CTRL2].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC1CTRL2].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC1CTRL2].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST0SRC2CTRL0] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC2CTRL0].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC2CTRL0].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC2CTRL0].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST0SRC2CTRL1] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC2CTRL1].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC2CTRL1].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC2CTRL1].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST0SRC2CTRL2] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC2CTRL2].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC2CTRL2].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC2CTRL2].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST0SRC3CTRL0] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC3CTRL0].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC3CTRL0].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC3CTRL0].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST0SRC3CTRL1] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC3CTRL1].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC3CTRL1].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC3CTRL1].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST0SRC3CTRL2] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC3CTRL2].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC3CTRL2].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC3CTRL2].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST1SRC0CTRL0] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC0CTRL0].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC0CTRL0].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC0CTRL0].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST1SRC0CTRL1] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC0CTRL1].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC0CTRL1].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC0CTRL1].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST1SRC0CTRL2] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC0CTRL2].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC0CTRL2].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC0CTRL2].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST1SRC1CTRL0] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC1CTRL0].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC1CTRL0].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC1CTRL0].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST1SRC1CTRL1] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC1CTRL1].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC1CTRL1].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC1CTRL1].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST1SRC1CTRL2] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC1CTRL2].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC1CTRL2].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC1CTRL2].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST1SRC2CTRL0] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC2CTRL0].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC2CTRL0].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC2CTRL0].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST1SRC2CTRL1] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC2CTRL1].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC2CTRL1].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC2CTRL1].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST1SRC2CTRL2] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC2CTRL2].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC2CTRL2].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC2CTRL2].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST1SRC3CTRL0] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC3CTRL0].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC3CTRL0].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC3CTRL0].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST1SRC3CTRL1] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC3CTRL1].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC3CTRL1].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC3CTRL1].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST1SRC3CTRL2] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC3CTRL2].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC3CTRL2].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC3CTRL2].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST2SRC0CTRL0] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC0CTRL0].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC0CTRL0].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC0CTRL0].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST2SRC0CTRL1] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC0CTRL1].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC0CTRL1].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC0CTRL1].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST2SRC0CTRL2] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC0CTRL2].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC0CTRL2].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC0CTRL2].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST2SRC1CTRL0] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC1CTRL0].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC1CTRL0].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC1CTRL0].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST2SRC1CTRL1] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC1CTRL1].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC1CTRL1].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC1CTRL1].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST2SRC1CTRL2] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC1CTRL2].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC1CTRL2].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC1CTRL2].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST2SRC2CTRL0] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC2CTRL0].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC2CTRL0].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC2CTRL0].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST2SRC2CTRL1] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC2CTRL1].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC2CTRL1].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC2CTRL1].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST2SRC2CTRL2] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC2CTRL2].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC2CTRL2].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC2CTRL2].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST2SRC3CTRL0] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC3CTRL0].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC3CTRL0].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC3CTRL0].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST2SRC3CTRL1] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC3CTRL1].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC3CTRL1].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC3CTRL1].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST2SRC3CTRL2] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC3CTRL2].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC3CTRL2].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC3CTRL2].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST3SRC0CTRL0] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC0CTRL0].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC0CTRL0].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC0CTRL0].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST3SRC0CTRL1] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC0CTRL1].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC0CTRL1].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC0CTRL1].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST3SRC0CTRL2] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC0CTRL2].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC0CTRL2].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC0CTRL2].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST3SRC1CTRL0] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC1CTRL0].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC1CTRL0].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC1CTRL0].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST3SRC1CTRL1] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC1CTRL1].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC1CTRL1].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC1CTRL1].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST3SRC1CTRL2] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC1CTRL2].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC1CTRL2].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC1CTRL2].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST3SRC2CTRL0] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC2CTRL0].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC2CTRL0].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC2CTRL0].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST3SRC2CTRL1] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC2CTRL1].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC2CTRL1].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC2CTRL1].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST3SRC2CTRL2] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC2CTRL2].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC2CTRL2].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC2CTRL2].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST3SRC3CTRL0] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC3CTRL0].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC3CTRL0].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC3CTRL0].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST3SRC3CTRL1] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC3CTRL1].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC3CTRL1].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC3CTRL1].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST3SRC3CTRL2] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC3CTRL2].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC3CTRL2].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC3CTRL2].UpperBound =  100.0f;

        port_descriptors[LMS_NOISE_TYPE] = 1;
	port_range_hints[LMS_NOISE_TYPE].DefaultValue = 0.0f;
	port_range_hints[LMS_NOISE_TYPE].LowerBound =  0;
	port_range_hints[LMS_NOISE_TYPE].UpperBound =  2;

        port_descriptors[WAYV_ADSR1_CHECKBOX] = 1;
	port_range_hints[WAYV_ADSR1_CHECKBOX].DefaultValue = 0.0f;
	port_range_hints[WAYV_ADSR1_CHECKBOX].LowerBound =  0;
	port_range_hints[WAYV_ADSR1_CHECKBOX].UpperBound =  1;

        port_descriptors[WAYV_ADSR2_CHECKBOX] = 1;
	port_range_hints[WAYV_ADSR2_CHECKBOX].DefaultValue = 0.0f;
	port_range_hints[WAYV_ADSR2_CHECKBOX].LowerBound =  0;
	port_range_hints[WAYV_ADSR2_CHECKBOX].UpperBound =  1;

	port_descriptors[WAYV_LFO_AMP] = 1;
	port_range_hints[WAYV_LFO_AMP].DefaultValue = 0.0f;
	port_range_hints[WAYV_LFO_AMP].LowerBound = -24.0f;
	port_range_hints[WAYV_LFO_AMP].UpperBound = 24.0f;

	port_descriptors[WAYV_LFO_PITCH] = 1;
	port_range_hints[WAYV_LFO_PITCH].DefaultValue = 0.0f;
	port_range_hints[WAYV_LFO_PITCH].LowerBound = -36.0f;
	port_range_hints[WAYV_LFO_PITCH].UpperBound = 36.0f;

	port_descriptors[WAYV_PITCH_ENV_AMT] = 1;
	port_range_hints[WAYV_PITCH_ENV_AMT].DefaultValue = 0.0f;
	port_range_hints[WAYV_PITCH_ENV_AMT].LowerBound =  -60.0f;
	port_range_hints[WAYV_PITCH_ENV_AMT].UpperBound =   60.0f;

	port_descriptors[WAYV_OSC2_UNISON_VOICES] = 1;
	port_range_hints[WAYV_OSC2_UNISON_VOICES].DefaultValue = 4.0f;
	port_range_hints[WAYV_OSC2_UNISON_VOICES].LowerBound =  1.0f;
	port_range_hints[WAYV_OSC2_UNISON_VOICES].UpperBound =  7.0f;

	port_descriptors[WAYV_OSC2_UNISON_SPREAD] = 1;
	port_range_hints[WAYV_OSC2_UNISON_SPREAD].DefaultValue = 50.0f;
	port_range_hints[WAYV_OSC2_UNISON_SPREAD].LowerBound =  0.0f;
	port_range_hints[WAYV_OSC2_UNISON_SPREAD].UpperBound =  100.0f;

	port_descriptors[WAYV_LFO_AMOUNT] = 1;
	port_range_hints[WAYV_LFO_AMOUNT].DefaultValue = 100.0f;
	port_range_hints[WAYV_LFO_AMOUNT].LowerBound = 0.0f;
	port_range_hints[WAYV_LFO_AMOUNT].UpperBound = 100.0f;

        port_descriptors[WAYV_OSC3_TYPE] = 1;
	port_range_hints[WAYV_OSC3_TYPE].DefaultValue = 0.0f;
	port_range_hints[WAYV_OSC3_TYPE].LowerBound =  0.0f;
	port_range_hints[WAYV_OSC3_TYPE].UpperBound =  (float)WT_TOTAL_WAVETABLE_COUNT;

	port_descriptors[WAYV_OSC3_PITCH] = 1;
	port_range_hints[WAYV_OSC3_PITCH].DefaultValue = 0.0f;
	port_range_hints[WAYV_OSC3_PITCH].LowerBound =  -72.0f;
	port_range_hints[WAYV_OSC3_PITCH].UpperBound =  72.0f;

	port_descriptors[WAYV_OSC3_TUNE] = 1;
	port_range_hints[WAYV_OSC3_TUNE].DefaultValue = 0.0f;
	port_range_hints[WAYV_OSC3_TUNE].LowerBound = -100.0f;
	port_range_hints[WAYV_OSC3_TUNE].UpperBound = 100.0f;

	port_descriptors[WAYV_OSC3_VOLUME] = 1;
	port_range_hints[WAYV_OSC3_VOLUME].DefaultValue = 0.0f;
	port_range_hints[WAYV_OSC3_VOLUME].LowerBound =  -30.0f;
	port_range_hints[WAYV_OSC3_VOLUME].UpperBound =  0.0f;

        port_descriptors[WAYV_OSC3_UNISON_VOICES] = 1;
	port_range_hints[WAYV_OSC3_UNISON_VOICES].DefaultValue = 4.0f;
	port_range_hints[WAYV_OSC3_UNISON_VOICES].LowerBound =  1.0f;
	port_range_hints[WAYV_OSC3_UNISON_VOICES].UpperBound =  7.0f;

	port_descriptors[WAYV_OSC3_UNISON_SPREAD] = 1;
	port_range_hints[WAYV_OSC3_UNISON_SPREAD].DefaultValue = 50.0f;
	port_range_hints[WAYV_OSC3_UNISON_SPREAD].LowerBound =  0.0f;
	port_range_hints[WAYV_OSC3_UNISON_SPREAD].UpperBound =  100.0f;

        port_descriptors[WAYV_OSC1_FM1] = 1;
	port_range_hints[WAYV_OSC1_FM1].DefaultValue = 0.0f;
	port_range_hints[WAYV_OSC1_FM1].LowerBound =  0.0f;
	port_range_hints[WAYV_OSC1_FM1].UpperBound =  100.0f;

        port_descriptors[WAYV_OSC1_FM2] = 1;
	port_range_hints[WAYV_OSC1_FM2].DefaultValue = 0.0f;
	port_range_hints[WAYV_OSC1_FM2].LowerBound =  0.0f;
	port_range_hints[WAYV_OSC1_FM2].UpperBound =  100.0f;

        port_descriptors[WAYV_OSC1_FM3] = 1;
	port_range_hints[WAYV_OSC1_FM3].DefaultValue = 0.0f;
	port_range_hints[WAYV_OSC1_FM3].LowerBound =  0.0f;
	port_range_hints[WAYV_OSC1_FM3].UpperBound =  100.0f;

        port_descriptors[WAYV_OSC2_FM1] = 1;
	port_range_hints[WAYV_OSC2_FM1].DefaultValue = 0.0f;
	port_range_hints[WAYV_OSC2_FM1].LowerBound =  0.0f;
	port_range_hints[WAYV_OSC2_FM1].UpperBound =  100.0f;

        port_descriptors[WAYV_OSC2_FM2] = 1;
	port_range_hints[WAYV_OSC2_FM2].DefaultValue = 0.0f;
	port_range_hints[WAYV_OSC2_FM2].LowerBound =  0.0f;
	port_range_hints[WAYV_OSC2_FM2].UpperBound =  100.0f;

        port_descriptors[WAYV_OSC2_FM3] = 1;
	port_range_hints[WAYV_OSC2_FM3].DefaultValue = 0.0f;
	port_range_hints[WAYV_OSC2_FM3].LowerBound =  0.0f;
	port_range_hints[WAYV_OSC2_FM3].UpperBound =  100.0f;

        port_descriptors[WAYV_OSC3_FM1] = 1;
	port_range_hints[WAYV_OSC3_FM1].DefaultValue = 0.0f;
	port_range_hints[WAYV_OSC3_FM1].LowerBound =  0.0f;
	port_range_hints[WAYV_OSC3_FM1].UpperBound =  100.0f;

        port_descriptors[WAYV_OSC3_FM2] = 1;
	port_range_hints[WAYV_OSC3_FM2].DefaultValue = 0.0f;
	port_range_hints[WAYV_OSC3_FM2].LowerBound =  0.0f;
	port_range_hints[WAYV_OSC3_FM2].UpperBound =  100.0f;

        port_descriptors[WAYV_OSC3_FM3] = 1;
	port_range_hints[WAYV_OSC3_FM3].DefaultValue = 0.0f;
	port_range_hints[WAYV_OSC3_FM3].LowerBound =  0.0f;
	port_range_hints[WAYV_OSC3_FM3].UpperBound =  100.0f;

        port_descriptors[WAYV_ATTACK3] = 1;
	port_range_hints[WAYV_ATTACK3].DefaultValue = 10.0f;
	port_range_hints[WAYV_ATTACK3].LowerBound = 0.0f;
	port_range_hints[WAYV_ATTACK3].UpperBound = 200.0f;

	port_descriptors[WAYV_DECAY3] = 1;
	port_range_hints[WAYV_DECAY3].DefaultValue = 50.0f;
	port_range_hints[WAYV_DECAY3].LowerBound = 10.0f;
	port_range_hints[WAYV_DECAY3].UpperBound = 200.0f;

	port_descriptors[WAYV_SUSTAIN3] = 1;
	port_range_hints[WAYV_SUSTAIN3].DefaultValue = 0.0f;
	port_range_hints[WAYV_SUSTAIN3].LowerBound = -30.0f;
	port_range_hints[WAYV_SUSTAIN3].UpperBound = 0.0f;

	port_descriptors[WAYV_RELEASE3] = 1;
	port_range_hints[WAYV_RELEASE3].DefaultValue = 50.0f;
	port_range_hints[WAYV_RELEASE3].LowerBound = 10.0f;
	port_range_hints[WAYV_RELEASE3].UpperBound = 400.0f;

        port_descriptors[WAYV_ADSR3_CHECKBOX] = 1;
	port_range_hints[WAYV_ADSR3_CHECKBOX].DefaultValue = 0.0f;
	port_range_hints[WAYV_ADSR3_CHECKBOX].LowerBound =  0;
	port_range_hints[WAYV_ADSR3_CHECKBOX].UpperBound =  1;


        /* Keyboard tracking */

        port_descriptors[WAVV_PFXMATRIX_GRP0DST0SRC4CTRL0] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC4CTRL0].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC4CTRL0].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC4CTRL0].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST0SRC4CTRL1] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC4CTRL1].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC4CTRL1].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC4CTRL1].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST0SRC4CTRL2] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC4CTRL2].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC4CTRL2].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC4CTRL2].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST1SRC4CTRL0] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC4CTRL0].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC4CTRL0].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC4CTRL0].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST1SRC4CTRL1] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC4CTRL1].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC4CTRL1].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC4CTRL1].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST1SRC4CTRL2] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC4CTRL2].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC4CTRL2].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC4CTRL2].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST2SRC4CTRL0] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC4CTRL0].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC4CTRL0].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC4CTRL0].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST2SRC4CTRL1] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC4CTRL1].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC4CTRL1].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC4CTRL1].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST2SRC4CTRL2] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC4CTRL2].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC4CTRL2].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC4CTRL2].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST3SRC4CTRL0] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC4CTRL0].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC4CTRL0].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC4CTRL0].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST3SRC4CTRL1] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC4CTRL1].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC4CTRL1].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC4CTRL1].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST3SRC4CTRL2] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC4CTRL2].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC4CTRL2].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC4CTRL2].UpperBound =  100.0f;


        /* Velocity tracking */


        port_descriptors[WAVV_PFXMATRIX_GRP0DST0SRC5CTRL0] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC5CTRL0].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC5CTRL0].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC5CTRL0].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST0SRC5CTRL1] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC5CTRL1].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC5CTRL1].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC5CTRL1].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST0SRC5CTRL2] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC5CTRL2].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC5CTRL2].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC5CTRL2].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST1SRC5CTRL0] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC5CTRL0].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC5CTRL0].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC5CTRL0].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST1SRC5CTRL1] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC5CTRL1].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC5CTRL1].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC5CTRL1].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST1SRC5CTRL2] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC5CTRL2].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC5CTRL2].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC5CTRL2].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST2SRC5CTRL0] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC5CTRL0].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC5CTRL0].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC5CTRL0].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST2SRC5CTRL1] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC5CTRL1].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC5CTRL1].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC5CTRL1].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST2SRC5CTRL2] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC5CTRL2].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC5CTRL2].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC5CTRL2].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST3SRC5CTRL0] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC5CTRL0].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC5CTRL0].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC5CTRL0].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST3SRC5CTRL1] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC5CTRL1].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC5CTRL1].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC5CTRL1].UpperBound =  100.0f;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST3SRC5CTRL2] = 1;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC5CTRL2].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC5CTRL2].LowerBound =  -100.0f;
        port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC5CTRL2].UpperBound =  100.0f;

        port_descriptors[WAYV_PERC_ENV_TIME1] = 1;
	port_range_hints[WAYV_PERC_ENV_TIME1].DefaultValue = 10.0f;
	port_range_hints[WAYV_PERC_ENV_TIME1].LowerBound =  2.0f;
        port_range_hints[WAYV_PERC_ENV_TIME1].UpperBound =  40.0f;

        port_descriptors[WAYV_PERC_ENV_PITCH1] = 1;
	port_range_hints[WAYV_PERC_ENV_PITCH1].DefaultValue = 66.0f;
	port_range_hints[WAYV_PERC_ENV_PITCH1].LowerBound =  42.0f;
        port_range_hints[WAYV_PERC_ENV_PITCH1].UpperBound =  120.0f;

        port_descriptors[WAYV_PERC_ENV_TIME2] = 1;
	port_range_hints[WAYV_PERC_ENV_TIME2].DefaultValue = 100.0f;
	port_range_hints[WAYV_PERC_ENV_TIME2].LowerBound =  20.0f;
        port_range_hints[WAYV_PERC_ENV_TIME2].UpperBound =  400.0f;

        port_descriptors[WAYV_PERC_ENV_PITCH2] = 1;
	port_range_hints[WAYV_PERC_ENV_PITCH2].DefaultValue = 48.0f;
	port_range_hints[WAYV_PERC_ENV_PITCH2].LowerBound =  33.0f;
        port_range_hints[WAYV_PERC_ENV_PITCH2].UpperBound =  63.0f;

        port_descriptors[WAYV_PERC_ENV_ON] = 1;
	port_range_hints[WAYV_PERC_ENV_ON].DefaultValue = 0.0f;
	port_range_hints[WAYV_PERC_ENV_ON].LowerBound =  0.0f;
        port_range_hints[WAYV_PERC_ENV_ON].UpperBound =  1.0f;

        port_descriptors[WAYV_RAMP_CURVE] = 1;
	port_range_hints[WAYV_RAMP_CURVE].DefaultValue = 50.0f;
	port_range_hints[WAYV_RAMP_CURVE].LowerBound = 0.0f;
	port_range_hints[WAYV_RAMP_CURVE].UpperBound = 100.0f;

        port_descriptors[WAYV_MONO_MODE] = 1;
	port_range_hints[WAYV_MONO_MODE].DefaultValue = 0.0f;
	port_range_hints[WAYV_MONO_MODE].LowerBound = 0.0f;
	port_range_hints[WAYV_MONO_MODE].UpperBound = 3.0f;


        port_descriptors[WAYV_OSC1_FM4] = 1;
	port_range_hints[WAYV_OSC1_FM4].DefaultValue = 0.0f;
	port_range_hints[WAYV_OSC1_FM4].LowerBound =  0.0f;
	port_range_hints[WAYV_OSC1_FM4].UpperBound =  100.0f;

        port_descriptors[WAYV_OSC2_FM4] = 1;
	port_range_hints[WAYV_OSC2_FM4].DefaultValue = 0.0f;
	port_range_hints[WAYV_OSC2_FM4].LowerBound =  0.0f;
	port_range_hints[WAYV_OSC2_FM4].UpperBound =  100.0f;

        port_descriptors[WAYV_OSC3_FM4] = 1;
	port_range_hints[WAYV_OSC3_FM4].DefaultValue = 0.0f;
	port_range_hints[WAYV_OSC3_FM4].LowerBound =  0.0f;
	port_range_hints[WAYV_OSC3_FM4].UpperBound =  100.0f;

        // Oscillator 4

        port_descriptors[WAYV_OSC4_TYPE] = 1;
	port_range_hints[WAYV_OSC4_TYPE].DefaultValue = 0.0f;
	port_range_hints[WAYV_OSC4_TYPE].LowerBound =  0.0f;
	port_range_hints[WAYV_OSC4_TYPE].UpperBound =  (float)WT_TOTAL_WAVETABLE_COUNT;

	port_descriptors[WAYV_OSC4_PITCH] = 1;
	port_range_hints[WAYV_OSC4_PITCH].DefaultValue = 0.0f;
	port_range_hints[WAYV_OSC4_PITCH].LowerBound =  -72.0f;
	port_range_hints[WAYV_OSC4_PITCH].UpperBound =  72.0f;

	port_descriptors[WAYV_OSC4_TUNE] = 1;
	port_range_hints[WAYV_OSC4_TUNE].DefaultValue = 0.0f;
	port_range_hints[WAYV_OSC4_TUNE].LowerBound = -100.0f;
	port_range_hints[WAYV_OSC4_TUNE].UpperBound = 100.0f;

	port_descriptors[WAYV_OSC4_VOLUME] = 1;
	port_range_hints[WAYV_OSC4_VOLUME].DefaultValue = 0.0f;
	port_range_hints[WAYV_OSC4_VOLUME].LowerBound =  -30.0f;
	port_range_hints[WAYV_OSC4_VOLUME].UpperBound =  0.0f;

        port_descriptors[WAYV_OSC4_UNISON_VOICES] = 1;
	port_range_hints[WAYV_OSC4_UNISON_VOICES].DefaultValue = 4.0f;
	port_range_hints[WAYV_OSC4_UNISON_VOICES].LowerBound =  1.0f;
	port_range_hints[WAYV_OSC4_UNISON_VOICES].UpperBound =  7.0f;

	port_descriptors[WAYV_OSC4_UNISON_SPREAD] = 1;
	port_range_hints[WAYV_OSC4_UNISON_SPREAD].DefaultValue = 50.0f;
	port_range_hints[WAYV_OSC4_UNISON_SPREAD].LowerBound =  0.0f;
	port_range_hints[WAYV_OSC4_UNISON_SPREAD].UpperBound =  100.0f;

        port_descriptors[WAYV_OSC4_FM1] = 1;
	port_range_hints[WAYV_OSC4_FM1].DefaultValue = 0.0f;
	port_range_hints[WAYV_OSC4_FM1].LowerBound =  0.0f;
	port_range_hints[WAYV_OSC4_FM1].UpperBound =  100.0f;

        port_descriptors[WAYV_OSC4_FM2] = 1;
	port_range_hints[WAYV_OSC4_FM2].DefaultValue = 0.0f;
	port_range_hints[WAYV_OSC4_FM2].LowerBound =  0.0f;
	port_range_hints[WAYV_OSC4_FM2].UpperBound =  100.0f;

        port_descriptors[WAYV_OSC4_FM3] = 1;
	port_range_hints[WAYV_OSC4_FM3].DefaultValue = 0.0f;
	port_range_hints[WAYV_OSC4_FM3].LowerBound =  0.0f;
	port_range_hints[WAYV_OSC4_FM3].UpperBound =  100.0f;

        port_descriptors[WAYV_OSC4_FM4] = 1;
	port_range_hints[WAYV_OSC4_FM4].DefaultValue = 0.0f;
	port_range_hints[WAYV_OSC4_FM4].LowerBound =  0.0f;
	port_range_hints[WAYV_OSC4_FM4].UpperBound =  100.0f;

        port_descriptors[WAYV_ATTACK4] = 1;
	port_range_hints[WAYV_ATTACK4].DefaultValue = 10.0f;
	port_range_hints[WAYV_ATTACK4].LowerBound = 0.0f;
	port_range_hints[WAYV_ATTACK4].UpperBound = 200.0f;

	port_descriptors[WAYV_DECAY4] = 1;
	port_range_hints[WAYV_DECAY4].DefaultValue = 50.0f;
	port_range_hints[WAYV_DECAY4].LowerBound = 10.0f;
	port_range_hints[WAYV_DECAY4].UpperBound = 200.0f;

	port_descriptors[WAYV_SUSTAIN4] = 1;
	port_range_hints[WAYV_SUSTAIN4].DefaultValue = 0.0f;
	port_range_hints[WAYV_SUSTAIN4].LowerBound = -30.0f;
	port_range_hints[WAYV_SUSTAIN4].UpperBound = 0.0f;

	port_descriptors[WAYV_RELEASE4] = 1;
	port_range_hints[WAYV_RELEASE4].DefaultValue = 50.0f;
	port_range_hints[WAYV_RELEASE4].LowerBound = 10.0f;
	port_range_hints[WAYV_RELEASE4].UpperBound = 400.0f;

        port_descriptors[WAYV_ADSR4_CHECKBOX] = 1;
	port_range_hints[WAYV_ADSR4_CHECKBOX].DefaultValue = 0.0f;
	port_range_hints[WAYV_ADSR4_CHECKBOX].LowerBound =  0;
	port_range_hints[WAYV_ADSR4_CHECKBOX].UpperBound =  1;

        port_descriptors[WAYV_LFO_PHASE] = 1;
	port_range_hints[WAYV_LFO_PHASE].DefaultValue = 0.0f;
	port_range_hints[WAYV_LFO_PHASE].LowerBound =  0.0f;
	port_range_hints[WAYV_LFO_PHASE].UpperBound =  100.0;

        int f_i = 0;
        int f_port = WAYV_FM_MACRO1;

        while(f_i < 2)
        {
            port_descriptors[f_port] = 1;
            port_range_hints[f_port].DefaultValue = 0.0f;
            port_range_hints[f_port].LowerBound =  0.0f;
            port_range_hints[f_port].UpperBound =  100.0f;
            f_port++;

            int f_i2 = 0;

            while(f_i2 < 4)
            {
                int f_i3 = 0;

                while(f_i3 < 4)
                {
                    port_descriptors[f_port] = 1;
                    port_range_hints[f_port].DefaultValue = 0.0f;
                    port_range_hints[f_port].LowerBound =  -100.0f;
                    port_range_hints[f_port].UpperBound =  100.0f;
                    f_port++;
                    f_i3++;
                }

                f_i2++;
            }

            f_i++;
        }


	LMSLDescriptor->activate = v_wayv_activate;
	LMSLDescriptor->cleanup = v_cleanup_wayv;
	LMSLDescriptor->connect_port = v_wayv_connect_port;
        LMSLDescriptor->connect_buffer = v_wayv_connect_buffer;
	LMSLDescriptor->deactivate = NULL;
	LMSLDescriptor->instantiate = g_wayv_instantiate;
        LMSLDescriptor->panic = wayvPanic;
    }

    return LMSLDescriptor;
}

const PYINST_Descriptor *wayv_PYINST_descriptor(int index)
{
    PYINST_Descriptor *LMSDDescriptor = NULL;

    LMSDDescriptor = (PYINST_Descriptor *) malloc(sizeof(PYINST_Descriptor));

    if (LMSDDescriptor)
    {
	LMSDDescriptor->PYINST_API_Version = 1;
	LMSDDescriptor->PYFX_Plugin = wayv_PYFX_descriptor(0);
	LMSDDescriptor->configure = c_wayv_configure;
	LMSDDescriptor->run_synth = v_run_wayv;
        LMSDDescriptor->offline_render_prep = v_wayv_or_prep;
    }

    return LMSDDescriptor;
}


/*
void v_wayv_destructor()
{
    if (LMSLDescriptor) {
	free((PYFX_PortDescriptor *) LMSLDescriptor->PortDescriptors);
	free((char **) LMSLDescriptor->PortNames);
	free((PYFX_PortRangeHint *) LMSLDescriptor->PortRangeHints);
	free(LMSLDescriptor);
    }
    if (LMSDDescriptor) {
	free(LMSDDescriptor);
    }
}
*/