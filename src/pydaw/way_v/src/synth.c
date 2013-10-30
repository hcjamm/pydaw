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

static void wayvPanic(PYFX_Handle instance)
{
    t_wayv *plugin = (t_wayv *)instance;
    int f_i = 0;
    while(f_i < WAYV_POLYPHONY)
    {
        v_adsr_kill(plugin->data[f_i]->adsr_amp);
        f_i++;
    }    
}

static void v_wayv_connect_buffer(PYFX_Handle instance, int a_index, float * DataLocation)
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
    
    /*Add the ports from step 9 to the connectPortLMS event handler*/
    
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
            plugin->osc1pitch = data;
            break;
        case WAYV_OSC1_TUNE:
            plugin->osc1tune = data;
            break;
        case WAYV_OSC1_TYPE:
            plugin->osc1type = data;
            break;
        case WAYV_OSC1_VOLUME:
            plugin->osc1vol = data;
            break;
        case WAYV_OSC2_PITCH:
            plugin->osc2pitch = data;
            break;
        case WAYV_OSC2_TUNE:
            plugin->osc2tune = data;
            break;
        case WAYV_OSC2_TYPE:
            plugin->osc2type = data;
            break;
        case WAYV_OSC2_VOLUME:
            plugin->osc2vol = data;
            break;
        case WAYV_OSC1_UNISON_VOICES:
            plugin->osc1_uni_voice = data;
            break;
        case WAYV_OSC1_UNISON_SPREAD:
            plugin->osc1_uni_spread = data;        
            break;
        case WAYV_OSC2_UNISON_VOICES:
            plugin->osc2_uni_voice = data;
            break;
        case WAYV_OSC2_UNISON_SPREAD:
            plugin->osc2_uni_spread = data;        
            break;
        case WAYV_OSC3_UNISON_VOICES:
            plugin->osc3_uni_voice = data;
            break;
        case WAYV_OSC3_UNISON_SPREAD:
            plugin->osc3_uni_spread = data;        
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

        case LMS_NOISE_TYPE: plugin->noise_type = data; break;
        case WAYV_ADSR1_CHECKBOX: plugin->adsr1_checked = data; break;
        case WAYV_ADSR2_CHECKBOX: plugin->adsr2_checked = data; break;

        case WAYV_LFO_AMP: plugin->lfo_amp = data; break;
        case WAYV_LFO_PITCH: plugin->lfo_pitch = data; break;

        case WAYV_PITCH_ENV_AMT: plugin->pitch_env_amt = data; break;
        case WAYV_LFO_AMOUNT: plugin->lfo_amount = data; break;

        case WAYV_OSC3_PITCH: plugin->osc3pitch = data; break;
        case WAYV_OSC3_TUNE: plugin->osc3tune = data; break;
        case WAYV_OSC3_TYPE: plugin->osc3type = data; break;
        case WAYV_OSC3_VOLUME: plugin->osc3vol = data;  break;

        case WAYV_OSC1_FM1: plugin->osc1fm1 = data;  break;
        case WAYV_OSC1_FM2: plugin->osc1fm2 = data;  break;
        case WAYV_OSC1_FM3: plugin->osc1fm3 = data;  break;

        case WAYV_OSC2_FM1: plugin->osc2fm1 = data;  break;
        case WAYV_OSC2_FM2: plugin->osc2fm2 = data;  break;
        case WAYV_OSC2_FM3: plugin->osc2fm3 = data;  break;

        case WAYV_OSC3_FM1: plugin->osc3fm1 = data;  break;
        case WAYV_OSC3_FM2: plugin->osc3fm2 = data;  break;
        case WAYV_OSC3_FM3: plugin->osc3fm3 = data;  break;

        case WAYV_ATTACK3: plugin->attack3 = data; break;
        case WAYV_DECAY3: plugin->decay3 = data; break;
        case WAYV_SUSTAIN3: plugin->sustain3 = data; break;
        case WAYV_RELEASE3: plugin->release3 = data; break;

        case WAYV_ADSR3_CHECKBOX: plugin->adsr3_checked = data; break;
    }
}

static PYFX_Handle g_wayv_instantiate(const PYFX_Descriptor * descriptor,
				   int s_rate, fp_get_wavpool_item_from_host a_host_wavpool_func)
{
    t_wayv *plugin_data = (t_wayv *) malloc(sizeof(t_wayv));
    
    plugin_data->fs = s_rate;    
    v_wayv_init(s_rate);  //initialize any static variables    
    
    return (PYFX_Handle) plugin_data;
}

static void v_wayv_activate(PYFX_Handle instance, float * a_port_table)
{
    t_wayv *plugin_data = (t_wayv *) instance;
    
    plugin_data->port_table = a_port_table;
    
    int i;
    
    plugin_data->voices = g_voc_get_voices(WAYV_POLYPHONY);    
    
    for (i=0; i<WAYV_POLYPHONY; i++) {
        plugin_data->data[i] = g_wayv_poly_init(plugin_data->fs);
        plugin_data->data[i]->note_f = i;        
    }
    plugin_data->sampleNo = 0;
        
    //plugin_data->pitch = 1.0f;
    plugin_data->sv_pitch_bend_value = 0.0f;
    plugin_data->sv_last_note = 60.0f;  //For glide
    
    plugin_data->mono_modules = v_wayv_mono_init();  //initialize all monophonic modules
}

static void v_run_wayv(PYFX_Handle instance, int sample_count,
		  t_pydaw_seq_event *events, int event_count)
{
    t_wayv *plugin_data = (t_wayv *) instance;
    
    plugin_data->i_run_poly_voice = 0;
    plugin_data->midi_event_count = 0;
    
    int midi_event_pos = 0;
    
    for(plugin_data->event_pos = 0; (plugin_data->event_pos) < event_count; plugin_data->event_pos = (plugin_data->event_pos) + 1)
    {        
        if (events[(plugin_data->event_pos)].type == PYDAW_EVENT_NOTEON) 
        {
            if (events[(plugin_data->event_pos)].velocity > 0) 
            {
                int f_voice = i_pick_voice(plugin_data->voices, events[(plugin_data->event_pos)].note, plugin_data->sampleNo, events[(plugin_data->event_pos)].tick);
                
                plugin_data->data[f_voice]->amp = f_db_to_linear_fast(((events[(plugin_data->event_pos)].velocity * 0.094488) - 12 + (*(plugin_data->master_vol))), //-20db to 0db, + master volume (0 to -60)
                        plugin_data->mono_modules->amp_ptr); 
                
                plugin_data->data[f_voice]->note_f = (float)events[(plugin_data->event_pos)].note;
                plugin_data->data[f_voice]->note = events[(plugin_data->event_pos)].note;

                plugin_data->data[f_voice]->target_pitch = (plugin_data->data[f_voice]->note_f);
                plugin_data->data[f_voice]->last_pitch = (plugin_data->sv_last_note);

                v_rmp_retrigger_glide_t(plugin_data->data[f_voice]->glide_env , (*(plugin_data->master_glide) * 0.01f), 
                        (plugin_data->sv_last_note), (plugin_data->data[f_voice]->target_pitch));

                plugin_data->data[f_voice]->osc1_linamp = f_db_to_linear_fast(*(plugin_data->osc1vol), plugin_data->mono_modules->amp_ptr); 
                plugin_data->data[f_voice]->osc2_linamp = f_db_to_linear_fast(*(plugin_data->osc2vol), plugin_data->mono_modules->amp_ptr);
                plugin_data->data[f_voice]->osc3_linamp = f_db_to_linear_fast(*(plugin_data->osc3vol), plugin_data->mono_modules->amp_ptr);
                plugin_data->data[f_voice]->noise_linamp = f_db_to_linear_fast(*(plugin_data->noise_amp), plugin_data->mono_modules->amp_ptr);

                v_adsr_retrigger(plugin_data->data[f_voice]->adsr_main);
                v_adsr_retrigger(plugin_data->data[f_voice]->adsr_amp1);
                v_adsr_retrigger(plugin_data->data[f_voice]->adsr_amp2);
                v_adsr_retrigger(plugin_data->data[f_voice]->adsr_amp3);

                float f_attack = *(plugin_data->attack_main) * .01f;
                f_attack = (f_attack) * (f_attack);
                float f_decay = *(plugin_data->decay_main) * .01f;
                f_decay = (f_decay) * (f_decay);
                float f_release = *(plugin_data->release_main) * .01f;
                f_release = (f_release) * (f_release);   
                
                v_adsr_set_adsr_db(plugin_data->data[f_voice]->adsr_main, (f_attack), (f_decay), *(plugin_data->sustain_main), (f_release));
                
                
                plugin_data->data[f_voice]->adsr_amp1_on = (int)(*(plugin_data->adsr1_checked));
                
                if(plugin_data->data[f_voice]->adsr_amp1_on)
                {
                    float f_attack1 = *(plugin_data->attack1) * .01f;
                    f_attack1 = (f_attack1) * (f_attack1);
                    float f_decay1 = *(plugin_data->decay1) * .01f;
                    f_decay1 = (f_decay1) * (f_decay1);
                    float f_release1 = *(plugin_data->release1) * .01f;
                    f_release1 = (f_release1) * (f_release1);   

                    v_adsr_set_adsr_db(plugin_data->data[f_voice]->adsr_amp1, (f_attack1), (f_decay1), *(plugin_data->sustain1), (f_release1));                    
                }

                plugin_data->data[f_voice]->adsr_amp2_on = (int)(*(plugin_data->adsr2_checked));

                if(plugin_data->data[f_voice]->adsr_amp2_on)
                {
                    float f_attack2 = *(plugin_data->attack2) * .01f;
                    f_attack2 = (f_attack2) * (f_attack2);
                    float f_decay2 = *(plugin_data->decay2) * .01f;
                    f_decay2 = (f_decay2) * (f_decay2);
                    float f_release2 = *(plugin_data->release2) * .01f;
                    f_release2 = (f_release2) * (f_release2);   

                    v_adsr_set_adsr_db(plugin_data->data[f_voice]->adsr_amp2, (f_attack2), (f_decay2), *(plugin_data->sustain2), (f_release2));                    
                }
                
                plugin_data->data[f_voice]->adsr_amp3_on = (int)(*(plugin_data->adsr3_checked));

                if(plugin_data->data[f_voice]->adsr_amp3_on)
                {
                    float f_attack3 = *(plugin_data->attack3) * .01f;
                    f_attack3 = (f_attack3) * (f_attack3);
                    float f_decay3 = *(plugin_data->decay3) * .01f;
                    f_decay3 = (f_decay3) * (f_decay3);
                    float f_release3 = *(plugin_data->release3) * .01f;
                    f_release3 = (f_release3) * (f_release3);   

                    v_adsr_set_adsr_db(plugin_data->data[f_voice]->adsr_amp3, (f_attack3), (f_decay3), *(plugin_data->sustain3), (f_release3));                    
                }
                
                plugin_data->data[f_voice]->noise_amp = f_db_to_linear(*(plugin_data->noise_amp), plugin_data->mono_modules->amp_ptr);

                int f_osc_type1 = (int)(*plugin_data->osc1type) - 1;
                
                if(f_osc_type1 >= 0)
                {
                    plugin_data->data[f_voice]->osc1_on = 1;
                    
                    v_osc_wav_note_on_sync_phases(plugin_data->data[f_voice]->osc_wavtable1);
                    
                    v_osc_wav_set_waveform(plugin_data->data[f_voice]->osc_wavtable1, 
                            plugin_data->data[f_voice]->wavetables->tables[f_osc_type1]->wavetable,
                            plugin_data->data[f_voice]->wavetables->tables[f_osc_type1]->length);
                    v_osc_wav_set_uni_voice_count(plugin_data->data[f_voice]->osc_wavtable1, *plugin_data->osc1_uni_voice);
                }
                else
                {
                    plugin_data->data[f_voice]->osc1_on = 0;
                }
                
                int f_osc_type2 = (int)(*plugin_data->osc2type) - 1;
                
                if(f_osc_type2 >= 0)
                {
                    plugin_data->data[f_voice]->osc2_on = 1;
                    
                    v_osc_wav_note_on_sync_phases(plugin_data->data[f_voice]->osc_wavtable2);
                    
                    v_osc_wav_set_waveform(plugin_data->data[f_voice]->osc_wavtable2, 
                            plugin_data->data[f_voice]->wavetables->tables[f_osc_type2]->wavetable,
                            plugin_data->data[f_voice]->wavetables->tables[f_osc_type2]->length);
                    v_osc_wav_set_uni_voice_count(plugin_data->data[f_voice]->osc_wavtable2, *plugin_data->osc2_uni_voice);
                }
                else
                {
                    plugin_data->data[f_voice]->osc2_on = 0;
                }
                
                int f_osc_type3 = (int)(*plugin_data->osc3type) - 1;
                
                if(f_osc_type3 >= 0)
                {
                    plugin_data->data[f_voice]->osc3_on = 1;
                    
                    v_osc_wav_note_on_sync_phases(plugin_data->data[f_voice]->osc_wavtable3);
                    
                    v_osc_wav_set_waveform(plugin_data->data[f_voice]->osc_wavtable3, 
                            plugin_data->data[f_voice]->wavetables->tables[f_osc_type3]->wavetable,
                            plugin_data->data[f_voice]->wavetables->tables[f_osc_type3]->length);
                    v_osc_wav_set_uni_voice_count(plugin_data->data[f_voice]->osc_wavtable3, *plugin_data->osc3_uni_voice);
                }
                else
                {
                    plugin_data->data[f_voice]->osc3_on = 0;
                }
                
                
                /*Set the last_note property, so the next note can glide from it if glide is turned on*/
                plugin_data->sv_last_note = (plugin_data->data[f_voice]->note_f);
                
                
                plugin_data->active_polyfx_count[f_voice] = 0;
                //Determine which PolyFX have been enabled
                for(plugin_data->i_dst = 0; (plugin_data->i_dst) < WAYV_MODULAR_POLYFX_COUNT; plugin_data->i_dst = (plugin_data->i_dst) + 1)
                {
                    int f_pfx_combobox_index = (int)(*(plugin_data->fx_combobox[0][(plugin_data->i_dst)]));
                    plugin_data->data[f_voice]->fx_func_ptr[(plugin_data->i_dst)] = g_mf3_get_function_pointer(f_pfx_combobox_index); 

                    if(f_pfx_combobox_index != 0)
                    {
                        plugin_data->active_polyfx[f_voice][(plugin_data->active_polyfx_count[f_voice])] = (plugin_data->i_dst);
                        plugin_data->active_polyfx_count[f_voice] = (plugin_data->active_polyfx_count[f_voice]) + 1;
                    }
                }    

                //Calculate an index of which mod_matrix controls to process.  This saves expensive iterations and if/then logic in the main loop
                for(plugin_data->i_fx_grps = 0; (plugin_data->i_fx_grps) < WAYV_EFFECTS_GROUPS_COUNT; plugin_data->i_fx_grps = (plugin_data->i_fx_grps) + 1)
                {
                    for(plugin_data->i_dst = 0; (plugin_data->i_dst) < (plugin_data->active_polyfx_count[f_voice]); plugin_data->i_dst = (plugin_data->i_dst) + 1)
                    {
                        plugin_data->polyfx_mod_counts[f_voice][(plugin_data->active_polyfx[f_voice][(plugin_data->i_dst)])] = 0;

                        for(plugin_data->i_src = 0; (plugin_data->i_src) < WAYV_MODULATOR_COUNT; plugin_data->i_src = (plugin_data->i_src) + 1)
                        {
                            for(plugin_data->i_ctrl = 0; (plugin_data->i_ctrl) < WAYV_CONTROLS_PER_MOD_EFFECT; plugin_data->i_ctrl = (plugin_data->i_ctrl) + 1)
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
                plugin_data->data[f_voice]->noise_func_ptr = fp_get_noise_func_ptr((int)(*(plugin_data->noise_type)));

                plugin_data->data[f_voice]->osc1_uni_spread = (*plugin_data->osc1_uni_spread) * 0.01f;
                plugin_data->data[f_voice]->osc2_uni_spread = (*plugin_data->osc2_uni_spread) * 0.01f;
                plugin_data->data[f_voice]->osc3_uni_spread = (*plugin_data->osc3_uni_spread) * 0.01f;
                
                plugin_data->data[f_voice]->osc1fm1 = (*plugin_data->osc1fm1) * 0.005f;
                plugin_data->data[f_voice]->osc1fm2 = (*plugin_data->osc1fm2) * 0.005f;
                plugin_data->data[f_voice]->osc1fm3 = (*plugin_data->osc1fm3) * 0.005f;
                
                plugin_data->data[f_voice]->osc2fm1 = (*plugin_data->osc2fm1) * 0.005f;
                plugin_data->data[f_voice]->osc2fm2 = (*plugin_data->osc2fm2) * 0.005f;
                plugin_data->data[f_voice]->osc2fm3 = (*plugin_data->osc2fm3) * 0.005f;
                
                plugin_data->data[f_voice]->osc3fm1 = (*plugin_data->osc3fm1) * 0.005f;
                plugin_data->data[f_voice]->osc3fm2 = (*plugin_data->osc3fm2) * 0.005f;
                plugin_data->data[f_voice]->osc3fm3 = (*plugin_data->osc3fm3) * 0.005f;
                
                v_adsr_retrigger(plugin_data->data[f_voice]->adsr_amp);
                v_adsr_retrigger(plugin_data->data[f_voice]->adsr_filter);
                v_lfs_sync(plugin_data->data[f_voice]->lfo1, 0.0f, *(plugin_data->lfo_type));

                float f_attack_a = (*(plugin_data->attack) * .01);  f_attack_a *= f_attack_a;
                float f_decay_a = (*(plugin_data->decay) * .01);  f_decay_a *= f_decay_a;
                float f_release_a = (*(plugin_data->release) * .01); f_release_a *= f_release_a;
                v_adsr_set_adsr_db(plugin_data->data[f_voice]->adsr_amp, f_attack_a, f_decay_a, (*(plugin_data->sustain)), f_release_a);
                
                float f_attack_f = (*(plugin_data->attack_f) * .01);  f_attack_f *= f_attack_f;
                float f_decay_f = (*(plugin_data->decay_f) * .01);  f_decay_f *= f_decay_f;
                float f_release_f = (*(plugin_data->release_f) * .01); f_release_f *= f_release_f;
                v_adsr_set_adsr(plugin_data->data[f_voice]->adsr_filter, f_attack_f, f_decay_f, (*(plugin_data->sustain_f) * .01), f_release_f);

                /*Retrigger the pitch envelope*/
                v_rmp_retrigger((plugin_data->data[f_voice]->ramp_env), (*(plugin_data->pitch_env_time) * .01), 1.0f);  

                plugin_data->data[f_voice]->noise_amp = f_db_to_linear(*(plugin_data->noise_amp), plugin_data->mono_modules->amp_ptr);
            } 
            /*0 velocity, the same as note-off*/
            else 
            {
                v_voc_note_off(plugin_data->voices, events[(plugin_data->event_pos)].note, (plugin_data->sampleNo), (events[(plugin_data->event_pos)].tick));
            }
        } 
        /*Note-off event*/
        else if (events[(plugin_data->event_pos)].type == PYDAW_EVENT_NOTEOFF) 
        {
            v_voc_note_off(plugin_data->voices, events[(plugin_data->event_pos)].note, (plugin_data->sampleNo), (events[(plugin_data->event_pos)].tick));
        } 
        /*Pitch-bend sequencer event, modify the voices pitch*/
        else if (events[(plugin_data->event_pos)].type == PYDAW_EVENT_PITCHBEND) 
        {
            plugin_data->midi_event_types[plugin_data->midi_event_count] = PYDAW_EVENT_PITCHBEND;
            plugin_data->midi_event_ticks[plugin_data->midi_event_count] = events[plugin_data->event_pos].tick;
            plugin_data->midi_event_values[plugin_data->midi_event_count] = 
                    0.00012207 * events[plugin_data->event_pos].value * (*(plugin_data->master_pb_amt));
            plugin_data->midi_event_count++;
        }        
    }
    
    /*Clear the output buffer*/
    plugin_data->i_iterator = 0;

    while((plugin_data->i_iterator) < sample_count)
    {
        plugin_data->output0[(plugin_data->i_iterator)] = 0.0f;
        plugin_data->output1[(plugin_data->i_iterator)] = 0.0f;
        
        while(midi_event_pos < plugin_data->midi_event_count && plugin_data->midi_event_ticks[midi_event_pos] == plugin_data->i_iterator)
        {
            if(plugin_data->midi_event_types[midi_event_pos] == PYDAW_EVENT_PITCHBEND)
            {
                plugin_data->sv_pitch_bend_value = plugin_data->midi_event_values[midi_event_pos];
            }
            
            midi_event_pos++;
        }
        
        v_smr_iir_run(plugin_data->mono_modules->pitchbend_smoother, (plugin_data->sv_pitch_bend_value));
    
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
                plugin_data->voices->voices[(plugin_data->i_run_poly_voice)].n_state = note_state_off;
            }

            plugin_data->i_run_poly_voice = (plugin_data->i_run_poly_voice) + 1; 
        }

        plugin_data->sampleNo++;
        plugin_data->i_iterator = (plugin_data->i_iterator) + 1;
    }    
    
    //plugin_data->sampleNo += sample_count;
}

static void v_run_wayv_voice(t_wayv *plugin_data, t_voc_single_voice a_poly_voice, t_wayv_poly_voice *a_voice, 
        PYFX_Data *out0, PYFX_Data *out1, int a_i, int a_voice_num)
{   
    a_voice->i_voice = a_i;
    
    if((plugin_data->sampleNo) < (a_poly_voice.on))
    {
        return;
    }
    
    if (((plugin_data->sampleNo) == a_poly_voice.off) && ((a_voice->adsr_main->stage) < 3))
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

    f_rmp_run_ramp(plugin_data->data[a_voice_num]->ramp_env);        

    //Set and run the LFO
    v_lfs_set(plugin_data->data[a_voice_num]->lfo1,  (*(plugin_data->lfo_freq)) * .01);
    v_lfs_run(plugin_data->data[a_voice_num]->lfo1);

    a_voice->lfo_amount_output = (a_voice->lfo1->output) * ((*plugin_data->lfo_amount) * 0.01f);

    a_voice->lfo_amp_output = f_db_to_linear_fast((((*plugin_data->lfo_amp) * (a_voice->lfo_amount_output)) - (f_lms_abs((*plugin_data->lfo_amp)) * 0.5)), a_voice->amp_ptr);        
    a_voice->lfo_pitch_output = (*plugin_data->lfo_pitch) * (a_voice->lfo_amount_output);        

    a_voice->base_pitch = (a_voice->glide_env->output_multiplied) + ((a_voice->ramp_env->output_multiplied) * (*plugin_data->pitch_env_amt))
            + (plugin_data->mono_modules->pitchbend_smoother->output) + (a_voice->last_pitch) + (a_voice->lfo_pitch_output);

    if(a_voice->osc1_on)
    {
        v_osc_wav_set_unison_pitch(a_voice->osc_wavtable1, (a_voice->osc1_uni_spread),
                ((a_voice->base_pitch) + (*plugin_data->osc1pitch) + ((*plugin_data->osc1tune) * 0.01f) )); //+ (a_voice->lfo_pitch_output)));       

        v_osc_wav_apply_fm(a_voice->osc_wavtable1, a_voice->osc1fm1, a_voice->fm1_last);
        v_osc_wav_apply_fm(a_voice->osc_wavtable1, a_voice->osc1fm2, a_voice->fm2_last);
        v_osc_wav_apply_fm(a_voice->osc_wavtable1, a_voice->osc1fm3, a_voice->fm3_last);

        if(a_voice->adsr_amp1_on)
        {
            v_adsr_run_db(a_voice->adsr_amp1);      
            a_voice->fm1_last = f_osc_wav_run_unison(a_voice->osc_wavtable1) * (a_voice->adsr_amp1->output);
            a_voice->current_sample += (a_voice->fm1_last) * (a_voice->osc1_linamp);
        }
        else
        {
            a_voice->fm1_last = f_osc_wav_run_unison(a_voice->osc_wavtable1);
            a_voice->current_sample += (a_voice->fm1_last) * (a_voice->osc1_linamp);
        }
    }

    if(a_voice->osc2_on)
    {
        v_osc_wav_set_unison_pitch(a_voice->osc_wavtable2, (a_voice->osc2_uni_spread),
                ((a_voice->base_pitch) + (*plugin_data->osc2pitch) + ((*plugin_data->osc2tune) * 0.01f) )); //+ (a_voice->lfo_pitch_output)));        

        v_osc_wav_apply_fm(a_voice->osc_wavtable2, a_voice->osc2fm1, a_voice->fm1_last);
        v_osc_wav_apply_fm(a_voice->osc_wavtable2, a_voice->osc2fm2, a_voice->fm2_last);
        v_osc_wav_apply_fm(a_voice->osc_wavtable2, a_voice->osc2fm3, a_voice->fm3_last);

        if(a_voice->adsr_amp2_on)
        {
            v_adsr_run_db(a_voice->adsr_amp2);
            a_voice->fm2_last = f_osc_wav_run_unison(a_voice->osc_wavtable2) * (a_voice->adsr_amp2->output);
            a_voice->current_sample += (a_voice->fm2_last) * (a_voice->adsr_amp2->output) * (a_voice->osc2_linamp);
        }
        else
        {
            a_voice->fm2_last = f_osc_wav_run_unison(a_voice->osc_wavtable2);
            a_voice->current_sample += (a_voice->fm2_last) * (a_voice->osc2_linamp);
        }
    }

    if(a_voice->osc3_on)
    {
        v_osc_wav_set_unison_pitch(a_voice->osc_wavtable3, (a_voice->osc3_uni_spread),
                ((a_voice->base_pitch) + (*plugin_data->osc3pitch) + ((*plugin_data->osc3tune) * 0.01f) )); //+ (a_voice->lfo_pitch_output)));        

        v_osc_wav_apply_fm(a_voice->osc_wavtable3, a_voice->osc3fm1, a_voice->fm1_last);
        v_osc_wav_apply_fm(a_voice->osc_wavtable3, a_voice->osc3fm2, a_voice->fm2_last);
        v_osc_wav_apply_fm(a_voice->osc_wavtable3, a_voice->osc3fm3, a_voice->fm3_last);

        if(a_voice->adsr_amp3_on)
        {
            v_adsr_run_db(a_voice->adsr_amp3);
            a_voice->fm3_last = f_osc_wav_run_unison(a_voice->osc_wavtable3) * (a_voice->adsr_amp3->output);
            a_voice->current_sample += (a_voice->fm3_last) * (a_voice->adsr_amp3->output) * (a_voice->osc3_linamp);
        }
        else
        {
            a_voice->fm3_last = f_osc_wav_run_unison(a_voice->osc_wavtable3);
            a_voice->current_sample += (a_voice->fm3_last) * (a_voice->osc3_linamp);
        }
    }

    a_voice->current_sample += (f_run_white_noise(a_voice->white_noise1) * (a_voice->noise_linamp)); //white noise

    v_adsr_run_db(a_voice->adsr_main);

    a_voice->current_sample = (a_voice->current_sample) * (a_voice->amp) * (a_voice->lfo_amp_output);

    a_voice->modulex_current_sample[0] = (a_voice->current_sample);
    a_voice->modulex_current_sample[1] = (a_voice->current_sample);

    //Modular PolyFX, processed from the index created during note_on
    for(plugin_data->i_dst = 0; (plugin_data->i_dst) < (plugin_data->active_polyfx_count[a_voice_num]); plugin_data->i_dst = (plugin_data->i_dst) + 1)
    {            
        v_mf3_set(a_voice->multieffect[(plugin_data->active_polyfx[a_voice_num][(plugin_data->i_dst)])],
            *(plugin_data->pfx_mod_knob[0][(plugin_data->active_polyfx[a_voice_num][(plugin_data->i_dst)])][0]), *(plugin_data->pfx_mod_knob[0][(plugin_data->active_polyfx[a_voice_num][(plugin_data->i_dst)])][1]), *(plugin_data->pfx_mod_knob[0][(plugin_data->active_polyfx[a_voice_num][(plugin_data->i_dst)])][2])); 

        int f_mod_test;

        for(f_mod_test = 0; f_mod_test < (plugin_data->polyfx_mod_counts[a_voice_num][(plugin_data->active_polyfx[a_voice_num][(plugin_data->i_dst)])]); f_mod_test++)
        {
            v_mf3_mod_single(
                    a_voice->multieffect[(plugin_data->active_polyfx[a_voice_num][(plugin_data->i_dst)])],
                    *(a_voice->modulator_outputs[(plugin_data->polyfx_mod_src_index[a_voice_num][(plugin_data->active_polyfx[a_voice_num][(plugin_data->i_dst)])][f_mod_test])]),
                    (plugin_data->polyfx_mod_matrix_values[a_voice_num][(plugin_data->active_polyfx[a_voice_num][(plugin_data->i_dst)])][f_mod_test]),
                    (plugin_data->polyfx_mod_ctrl_indexes[a_voice_num][(plugin_data->active_polyfx[a_voice_num][(plugin_data->i_dst)])][f_mod_test])
                    );
        }

        a_voice->fx_func_ptr[(plugin_data->active_polyfx[a_voice_num][(plugin_data->i_dst)])](a_voice->multieffect[(plugin_data->active_polyfx[a_voice_num][(plugin_data->i_dst)])], (a_voice->modulex_current_sample[0]), (a_voice->modulex_current_sample[1])); 

        a_voice->modulex_current_sample[0] = a_voice->multieffect[(plugin_data->active_polyfx[a_voice_num][(plugin_data->i_dst)])]->output0;
        a_voice->modulex_current_sample[1] = a_voice->multieffect[(plugin_data->active_polyfx[a_voice_num][(plugin_data->i_dst)])]->output1;

    }

    /*Run the envelope and assign to the output buffers*/
    out0[(a_voice->i_voice)] += (a_voice->modulex_current_sample[0]) * (a_voice->adsr_main->output);
    out1[(a_voice->i_voice)] += (a_voice->modulex_current_sample[1]) * (a_voice->adsr_main->output);       
    
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

        
	port_descriptors[WAYV_ATTACK_MAIN] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAYV_ATTACK_MAIN].DefaultValue = 10.0f;
	port_range_hints[WAYV_ATTACK_MAIN].LowerBound = 0.0f; 
	port_range_hints[WAYV_ATTACK_MAIN].UpperBound = 100.0f;

	port_descriptors[WAYV_DECAY_MAIN] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAYV_DECAY_MAIN].DefaultValue = 50.0f;
	port_range_hints[WAYV_DECAY_MAIN].LowerBound = 10.0f; 
	port_range_hints[WAYV_DECAY_MAIN].UpperBound = 100.0f;

	port_descriptors[WAYV_SUSTAIN_MAIN] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAYV_SUSTAIN_MAIN].DefaultValue = 0.0f;
	port_range_hints[WAYV_SUSTAIN_MAIN].LowerBound = -30.0f;
	port_range_hints[WAYV_SUSTAIN_MAIN].UpperBound = 0.0f;

	port_descriptors[WAYV_RELEASE_MAIN] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAYV_RELEASE_MAIN].DefaultValue = 50.0f;
	port_range_hints[WAYV_RELEASE_MAIN].LowerBound = 10.0f; 
	port_range_hints[WAYV_RELEASE_MAIN].UpperBound = 200.0f;

	port_descriptors[WAYV_ATTACK1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAYV_ATTACK1].DefaultValue = 10.0f;
	port_range_hints[WAYV_ATTACK1].LowerBound = 0.0f; 
	port_range_hints[WAYV_ATTACK1].UpperBound = 100.0f; 

	port_descriptors[WAYV_DECAY1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAYV_DECAY1].DefaultValue = 50.0f;
	port_range_hints[WAYV_DECAY1].LowerBound = 10.0f; 
	port_range_hints[WAYV_DECAY1].UpperBound = 100.0f; 

	port_descriptors[WAYV_SUSTAIN1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAYV_SUSTAIN1].DefaultValue = 0.0f;
	port_range_hints[WAYV_SUSTAIN1].LowerBound = -30.0f;
	port_range_hints[WAYV_SUSTAIN1].UpperBound = 0.0f;

	port_descriptors[WAYV_RELEASE1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAYV_RELEASE1].DefaultValue = 50.0f;
	port_range_hints[WAYV_RELEASE1].LowerBound = 10.0f; 
	port_range_hints[WAYV_RELEASE1].UpperBound = 200.0f; 
                
	port_descriptors[WAYV_ATTACK2] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAYV_ATTACK2].DefaultValue = 10.0f;
	port_range_hints[WAYV_ATTACK2].LowerBound = 0.0f; 
	port_range_hints[WAYV_ATTACK2].UpperBound = 100.0f; 

	port_descriptors[WAYV_DECAY2] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAYV_DECAY2].DefaultValue = 50.0f;
	port_range_hints[WAYV_DECAY2].LowerBound = 10.0f; 
	port_range_hints[WAYV_DECAY2].UpperBound = 100.0f; 

	port_descriptors[WAYV_SUSTAIN2] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAYV_SUSTAIN2].DefaultValue = 0.0f;
	port_range_hints[WAYV_SUSTAIN2].LowerBound = -30.0f;
	port_range_hints[WAYV_SUSTAIN2].UpperBound = 0.0f;

	port_descriptors[WAYV_RELEASE2] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAYV_RELEASE2].DefaultValue = 50.0f;
	port_range_hints[WAYV_RELEASE2].LowerBound = 10.0f; 
	port_range_hints[WAYV_RELEASE2].UpperBound = 200.0f; 
        
	port_descriptors[WAYV_NOISE_AMP] = port_descriptors[WAYV_ATTACK_MAIN];
	port_range_hints[WAYV_NOISE_AMP].DefaultValue = -30.0f;
	port_range_hints[WAYV_NOISE_AMP].LowerBound =  -60.0f;
	port_range_hints[WAYV_NOISE_AMP].UpperBound =  0.0f;
        
	port_descriptors[WAYV_OSC1_TYPE] = port_descriptors[WAYV_ATTACK_MAIN];
	port_range_hints[WAYV_OSC1_TYPE].DefaultValue = 1.0f;
	port_range_hints[WAYV_OSC1_TYPE].LowerBound =  0.0f;
	port_range_hints[WAYV_OSC1_TYPE].UpperBound =  (float)WT_TOTAL_WAVETABLE_COUNT;
        
	port_descriptors[WAYV_OSC1_PITCH] = port_descriptors[WAYV_ATTACK_MAIN];
	port_range_hints[WAYV_OSC1_PITCH].DefaultValue = 0.0f;
	port_range_hints[WAYV_OSC1_PITCH].LowerBound =  -36.0f;
	port_range_hints[WAYV_OSC1_PITCH].UpperBound =  36.0f;
        
	port_descriptors[WAYV_OSC1_TUNE] = port_descriptors[WAYV_ATTACK_MAIN];
	port_range_hints[WAYV_OSC1_TUNE].DefaultValue = 0.0f;
	port_range_hints[WAYV_OSC1_TUNE].LowerBound = -100.0f;
	port_range_hints[WAYV_OSC1_TUNE].UpperBound =  100.0f;
        
	port_descriptors[WAYV_OSC1_VOLUME] = port_descriptors[WAYV_ATTACK_MAIN];
	port_range_hints[WAYV_OSC1_VOLUME].DefaultValue = 0.0f;
	port_range_hints[WAYV_OSC1_VOLUME].LowerBound =  -30.0f;
	port_range_hints[WAYV_OSC1_VOLUME].UpperBound =  0.0f;
        
	port_descriptors[WAYV_OSC2_TYPE] = port_descriptors[WAYV_ATTACK_MAIN];
	port_range_hints[WAYV_OSC2_TYPE].DefaultValue = 0.0f;
	port_range_hints[WAYV_OSC2_TYPE].LowerBound =  0.0f;
	port_range_hints[WAYV_OSC2_TYPE].UpperBound =  (float)WT_TOTAL_WAVETABLE_COUNT;
        
	port_descriptors[WAYV_OSC2_PITCH] = port_descriptors[WAYV_ATTACK_MAIN];
	port_range_hints[WAYV_OSC2_PITCH].DefaultValue = 0.0f;
	port_range_hints[WAYV_OSC2_PITCH].LowerBound =  -36.0f;
	port_range_hints[WAYV_OSC2_PITCH].UpperBound =  36.0f;
        
	port_descriptors[WAYV_OSC2_TUNE] = port_descriptors[WAYV_ATTACK_MAIN];
	port_range_hints[WAYV_OSC2_TUNE].DefaultValue = 0.0f;
	port_range_hints[WAYV_OSC2_TUNE].LowerBound = -100.0f;
	port_range_hints[WAYV_OSC2_TUNE].UpperBound = 100.0f; 
        
	port_descriptors[WAYV_OSC2_VOLUME] = port_descriptors[WAYV_ATTACK_MAIN];
	port_range_hints[WAYV_OSC2_VOLUME].DefaultValue = 0.0f;
	port_range_hints[WAYV_OSC2_VOLUME].LowerBound =  -30.0f;
	port_range_hints[WAYV_OSC2_VOLUME].UpperBound =  0.0f;
        
	port_descriptors[WAYV_MASTER_VOLUME] = port_descriptors[WAYV_ATTACK_MAIN];
	port_range_hints[WAYV_MASTER_VOLUME].DefaultValue = -6.0f;
	port_range_hints[WAYV_MASTER_VOLUME].LowerBound =  -30.0f;
	port_range_hints[WAYV_MASTER_VOLUME].UpperBound =  12.0f;
        
	port_descriptors[WAYV_OSC1_UNISON_VOICES] = port_descriptors[WAYV_ATTACK_MAIN];
	port_range_hints[WAYV_OSC1_UNISON_VOICES].DefaultValue = 4.0f;
	port_range_hints[WAYV_OSC1_UNISON_VOICES].LowerBound =  1.0f;
	port_range_hints[WAYV_OSC1_UNISON_VOICES].UpperBound =  7.0f;
        
	port_descriptors[WAYV_OSC1_UNISON_SPREAD] = port_descriptors[WAYV_ATTACK_MAIN];
	port_range_hints[WAYV_OSC1_UNISON_SPREAD].DefaultValue = 50.0f;
	port_range_hints[WAYV_OSC1_UNISON_SPREAD].LowerBound =  0.0f;
	port_range_hints[WAYV_OSC1_UNISON_SPREAD].UpperBound =  100.0f;
        
	port_descriptors[WAYV_MASTER_GLIDE] = port_descriptors[WAYV_ATTACK_MAIN];
	port_range_hints[WAYV_MASTER_GLIDE].DefaultValue = 0.0f;
	port_range_hints[WAYV_MASTER_GLIDE].LowerBound =  0.0f;
	port_range_hints[WAYV_MASTER_GLIDE].UpperBound =  200.0f;
        
	port_descriptors[WAYV_MASTER_PITCHBEND_AMT] = port_descriptors[WAYV_ATTACK_MAIN];
	port_range_hints[WAYV_MASTER_PITCHBEND_AMT].DefaultValue = 18.0f;
	port_range_hints[WAYV_MASTER_PITCHBEND_AMT].LowerBound =  1.0f;
	port_range_hints[WAYV_MASTER_PITCHBEND_AMT].UpperBound =  36.0f;
        
	port_descriptors[WAYV_ATTACK_PFX1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAYV_ATTACK_PFX1].DefaultValue = 10.0f;
	port_range_hints[WAYV_ATTACK_PFX1].LowerBound = 0.0f; 
	port_range_hints[WAYV_ATTACK_PFX1].UpperBound = 100.0f; 
        
	port_descriptors[WAYV_DECAY_PFX1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAYV_DECAY_PFX1].DefaultValue = 50.0f;
	port_range_hints[WAYV_DECAY_PFX1].LowerBound = 10.0f; 
	port_range_hints[WAYV_DECAY_PFX1].UpperBound = 100.0f;

	port_descriptors[WAYV_SUSTAIN_PFX1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAYV_SUSTAIN_PFX1].DefaultValue = 0.0f;
	port_range_hints[WAYV_SUSTAIN_PFX1].LowerBound = -60.0f;
	port_range_hints[WAYV_SUSTAIN_PFX1].UpperBound = 0.0f;

	port_descriptors[WAYV_RELEASE_PFX1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAYV_RELEASE_PFX1].DefaultValue = 50.0f;
	port_range_hints[WAYV_RELEASE_PFX1].LowerBound = 10.0f; 
	port_range_hints[WAYV_RELEASE_PFX1].UpperBound = 200.0f; 
        
	port_descriptors[WAYV_ATTACK_PFX2] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAYV_ATTACK_PFX2].DefaultValue = 10.0f;
	port_range_hints[WAYV_ATTACK_PFX2].LowerBound = 0.0f; 
	port_range_hints[WAYV_ATTACK_PFX2].UpperBound = 100.0f; 

	port_descriptors[WAYV_DECAY_PFX2] = port_descriptors[WAYV_ATTACK_PFX1];
	port_range_hints[WAYV_DECAY_PFX2].DefaultValue = 50.0f;
	port_range_hints[WAYV_DECAY_PFX2].LowerBound = 10.0f;
	port_range_hints[WAYV_DECAY_PFX2].UpperBound = 100.0f;

	port_descriptors[WAYV_SUSTAIN_PFX2] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAYV_SUSTAIN_PFX2].DefaultValue = 100.0f;
	port_range_hints[WAYV_SUSTAIN_PFX2].LowerBound = 0.0f; 
	port_range_hints[WAYV_SUSTAIN_PFX2].UpperBound = 100.0f; 
        
	port_descriptors[WAYV_RELEASE_PFX2] = port_descriptors[WAYV_ATTACK_PFX1];
	port_range_hints[WAYV_RELEASE_PFX2].DefaultValue = 50.0f;
	port_range_hints[WAYV_RELEASE_PFX2].LowerBound = 10.0f; 
	port_range_hints[WAYV_RELEASE_PFX2].UpperBound = 200.0f; 

	port_descriptors[WAYV_NOISE_AMP] = port_descriptors[WAYV_ATTACK_PFX1];
	port_range_hints[WAYV_NOISE_AMP].DefaultValue = -60.0f;
	port_range_hints[WAYV_NOISE_AMP].LowerBound =  -60.0f;
	port_range_hints[WAYV_NOISE_AMP].UpperBound =  0.0f;
        
	port_descriptors[WAYV_RAMP_ENV_TIME] = port_descriptors[WAYV_ATTACK_PFX1];
	port_range_hints[WAYV_RAMP_ENV_TIME].DefaultValue = 100.0f;
	port_range_hints[WAYV_RAMP_ENV_TIME].LowerBound = 0.0f; 
	port_range_hints[WAYV_RAMP_ENV_TIME].UpperBound = 200.0f;
                
	port_descriptors[WAYV_LFO_FREQ] = port_descriptors[WAYV_ATTACK_PFX1];
	port_range_hints[WAYV_LFO_FREQ].DefaultValue = 200.0f;
	port_range_hints[WAYV_LFO_FREQ].LowerBound = 10; 
	port_range_hints[WAYV_LFO_FREQ].UpperBound = 1600;
                
	port_descriptors[WAYV_LFO_TYPE] = port_descriptors[WAYV_ATTACK_PFX1];
	port_range_hints[WAYV_LFO_TYPE].DefaultValue = 0.0f;
	port_range_hints[WAYV_LFO_TYPE].LowerBound = 0.0f; 
	port_range_hints[WAYV_LFO_TYPE].UpperBound = 2.0f;
        
        port_descriptors[WAYV_FX0_KNOB0] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAYV_FX0_KNOB0].DefaultValue = 64.0f;
	port_range_hints[WAYV_FX0_KNOB0].LowerBound =  0.0f;
	port_range_hints[WAYV_FX0_KNOB0].UpperBound =  127.0f;
                
	port_descriptors[WAYV_FX0_KNOB1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAYV_FX0_KNOB1].DefaultValue = 64.0f;
	port_range_hints[WAYV_FX0_KNOB1].LowerBound =  0.0f;
	port_range_hints[WAYV_FX0_KNOB1].UpperBound =  127.0f;
        	
	port_descriptors[WAYV_FX0_KNOB2] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAYV_FX0_KNOB2].DefaultValue = 64.0f;
	port_range_hints[WAYV_FX0_KNOB2].LowerBound =  0.0f;
	port_range_hints[WAYV_FX0_KNOB2].UpperBound =  127.0f;
        
	port_descriptors[WAYV_FX0_COMBOBOX] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAYV_FX0_COMBOBOX].DefaultValue = 0.0f;
	port_range_hints[WAYV_FX0_COMBOBOX].LowerBound =  0.0f;
	port_range_hints[WAYV_FX0_COMBOBOX].UpperBound =  MULTIFX3KNOB_MAX_INDEX;        
        	
	port_descriptors[WAYV_FX1_KNOB0] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAYV_FX1_KNOB0].DefaultValue = 64.0f;
	port_range_hints[WAYV_FX1_KNOB0].LowerBound =  0.0f;
	port_range_hints[WAYV_FX1_KNOB0].UpperBound =  127.0f;
        	
	port_descriptors[WAYV_FX1_KNOB1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAYV_FX1_KNOB1].DefaultValue = 64.0f;
	port_range_hints[WAYV_FX1_KNOB1].LowerBound =  0.0f;
	port_range_hints[WAYV_FX1_KNOB1].UpperBound =  127.0f;
        	
	port_descriptors[WAYV_FX1_KNOB2] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAYV_FX1_KNOB2].DefaultValue = 64.0f;
	port_range_hints[WAYV_FX1_KNOB2].LowerBound =  0.0f;
	port_range_hints[WAYV_FX1_KNOB2].UpperBound =  127.0f;
        
	port_descriptors[WAYV_FX1_COMBOBOX] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAYV_FX1_COMBOBOX].DefaultValue = 0.0f;
	port_range_hints[WAYV_FX1_COMBOBOX].LowerBound =  0.0f;
	port_range_hints[WAYV_FX1_COMBOBOX].UpperBound =  MULTIFX3KNOB_MAX_INDEX;
                
        port_descriptors[WAYV_FX2_KNOB0] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAYV_FX2_KNOB0].DefaultValue = 64.0f;
	port_range_hints[WAYV_FX2_KNOB0].LowerBound =  0.0f;
	port_range_hints[WAYV_FX2_KNOB0].UpperBound =  127.0f;
        	
	port_descriptors[WAYV_FX2_KNOB1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAYV_FX2_KNOB1].DefaultValue = 64.0f;
	port_range_hints[WAYV_FX2_KNOB1].LowerBound =  0.0f;
	port_range_hints[WAYV_FX2_KNOB1].UpperBound =  127.0f;
        	
	port_descriptors[WAYV_FX2_KNOB2] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAYV_FX2_KNOB2].DefaultValue = 64.0f;
	port_range_hints[WAYV_FX2_KNOB2].LowerBound =  0.0f;
	port_range_hints[WAYV_FX2_KNOB2].UpperBound =  127.0f;
        
	port_descriptors[WAYV_FX2_COMBOBOX] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAYV_FX2_COMBOBOX].DefaultValue = 0.0f;
	port_range_hints[WAYV_FX2_COMBOBOX].LowerBound =  0.0f;
	port_range_hints[WAYV_FX2_COMBOBOX].UpperBound =  MULTIFX3KNOB_MAX_INDEX;        
        	
	port_descriptors[WAYV_FX3_KNOB0] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAYV_FX3_KNOB0].DefaultValue = 64.0f;
	port_range_hints[WAYV_FX3_KNOB0].LowerBound =  0.0f;
	port_range_hints[WAYV_FX3_KNOB0].UpperBound =  127.0f;
        	
	port_descriptors[WAYV_FX3_KNOB1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAYV_FX3_KNOB1].DefaultValue = 64.0f;
	port_range_hints[WAYV_FX3_KNOB1].LowerBound =  0.0f;
	port_range_hints[WAYV_FX3_KNOB1].UpperBound =  127.0f;
        	
	port_descriptors[WAYV_FX3_KNOB2] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAYV_FX3_KNOB2].DefaultValue = 64.0f;
	port_range_hints[WAYV_FX3_KNOB2].LowerBound =  0.0f;
	port_range_hints[WAYV_FX3_KNOB2].UpperBound =  127.0f;
        
	port_descriptors[WAYV_FX3_COMBOBOX] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAYV_FX3_COMBOBOX].DefaultValue = 0.0f;
	port_range_hints[WAYV_FX3_COMBOBOX].LowerBound =  0.0f;
	port_range_hints[WAYV_FX3_COMBOBOX].UpperBound =  MULTIFX3KNOB_MAX_INDEX;
        
        port_descriptors[WAVV_PFXMATRIX_GRP0DST0SRC0CTRL0] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC0CTRL0].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC0CTRL0].LowerBound =  -100; port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC0CTRL0].UpperBound =  100;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST0SRC0CTRL1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC0CTRL1].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC0CTRL1].LowerBound =  -100; port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC0CTRL1].UpperBound =  100;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST0SRC0CTRL2] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC0CTRL2].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC0CTRL2].LowerBound =  -100; port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC0CTRL2].UpperBound =  100;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST0SRC1CTRL0] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC1CTRL0].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC1CTRL0].LowerBound =  -100; port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC1CTRL0].UpperBound =  100;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST0SRC1CTRL1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC1CTRL1].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC1CTRL1].LowerBound =  -100; port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC1CTRL1].UpperBound =  100;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST0SRC1CTRL2] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC1CTRL2].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC1CTRL2].LowerBound =  -100; port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC1CTRL2].UpperBound =  100;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST0SRC2CTRL0] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC2CTRL0].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC2CTRL0].LowerBound =  -100; port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC2CTRL0].UpperBound =  100;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST0SRC2CTRL1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC2CTRL1].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC2CTRL1].LowerBound =  -100; port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC2CTRL1].UpperBound =  100;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST0SRC2CTRL2] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC2CTRL2].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC2CTRL2].LowerBound =  -100; port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC2CTRL2].UpperBound =  100;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST0SRC3CTRL0] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC3CTRL0].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC3CTRL0].LowerBound =  -100; port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC3CTRL0].UpperBound =  100;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST0SRC3CTRL1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC3CTRL1].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC3CTRL1].LowerBound =  -100; port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC3CTRL1].UpperBound =  100;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST0SRC3CTRL2] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC3CTRL2].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC3CTRL2].LowerBound =  -100; port_range_hints[WAVV_PFXMATRIX_GRP0DST0SRC3CTRL2].UpperBound =  100;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST1SRC0CTRL0] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC0CTRL0].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC0CTRL0].LowerBound =  -100; port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC0CTRL0].UpperBound =  100;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST1SRC0CTRL1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC0CTRL1].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC0CTRL1].LowerBound =  -100; port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC0CTRL1].UpperBound =  100;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST1SRC0CTRL2] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC0CTRL2].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC0CTRL2].LowerBound =  -100; port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC0CTRL2].UpperBound =  100;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST1SRC1CTRL0] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC1CTRL0].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC1CTRL0].LowerBound =  -100; port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC1CTRL0].UpperBound =  100;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST1SRC1CTRL1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC1CTRL1].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC1CTRL1].LowerBound =  -100; port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC1CTRL1].UpperBound =  100;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST1SRC1CTRL2] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC1CTRL2].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC1CTRL2].LowerBound =  -100; port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC1CTRL2].UpperBound =  100;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST1SRC2CTRL0] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC2CTRL0].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC2CTRL0].LowerBound =  -100; port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC2CTRL0].UpperBound =  100;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST1SRC2CTRL1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC2CTRL1].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC2CTRL1].LowerBound =  -100; port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC2CTRL1].UpperBound =  100;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST1SRC2CTRL2] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC2CTRL2].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC2CTRL2].LowerBound =  -100; port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC2CTRL2].UpperBound =  100;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST1SRC3CTRL0] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC3CTRL0].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC3CTRL0].LowerBound =  -100; port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC3CTRL0].UpperBound =  100;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST1SRC3CTRL1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC3CTRL1].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC3CTRL1].LowerBound =  -100; port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC3CTRL1].UpperBound =  100;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST1SRC3CTRL2] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC3CTRL2].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC3CTRL2].LowerBound =  -100; port_range_hints[WAVV_PFXMATRIX_GRP0DST1SRC3CTRL2].UpperBound =  100;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST2SRC0CTRL0] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC0CTRL0].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC0CTRL0].LowerBound =  -100; port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC0CTRL0].UpperBound =  100;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST2SRC0CTRL1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC0CTRL1].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC0CTRL1].LowerBound =  -100; port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC0CTRL1].UpperBound =  100;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST2SRC0CTRL2] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC0CTRL2].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC0CTRL2].LowerBound =  -100; port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC0CTRL2].UpperBound =  100;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST2SRC1CTRL0] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC1CTRL0].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC1CTRL0].LowerBound =  -100; port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC1CTRL0].UpperBound =  100;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST2SRC1CTRL1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC1CTRL1].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC1CTRL1].LowerBound =  -100; port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC1CTRL1].UpperBound =  100;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST2SRC1CTRL2] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC1CTRL2].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC1CTRL2].LowerBound =  -100; port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC1CTRL2].UpperBound =  100;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST2SRC2CTRL0] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC2CTRL0].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC2CTRL0].LowerBound =  -100; port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC2CTRL0].UpperBound =  100;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST2SRC2CTRL1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC2CTRL1].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC2CTRL1].LowerBound =  -100; port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC2CTRL1].UpperBound =  100;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST2SRC2CTRL2] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC2CTRL2].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC2CTRL2].LowerBound =  -100; port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC2CTRL2].UpperBound =  100;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST2SRC3CTRL0] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC3CTRL0].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC3CTRL0].LowerBound =  -100; port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC3CTRL0].UpperBound =  100;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST2SRC3CTRL1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC3CTRL1].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC3CTRL1].LowerBound =  -100; port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC3CTRL1].UpperBound =  100;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST2SRC3CTRL2] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC3CTRL2].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC3CTRL2].LowerBound =  -100; port_range_hints[WAVV_PFXMATRIX_GRP0DST2SRC3CTRL2].UpperBound =  100;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST3SRC0CTRL0] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC0CTRL0].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC0CTRL0].LowerBound =  -100; port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC0CTRL0].UpperBound =  100;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST3SRC0CTRL1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC0CTRL1].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC0CTRL1].LowerBound =  -100; port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC0CTRL1].UpperBound =  100;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST3SRC0CTRL2] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC0CTRL2].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC0CTRL2].LowerBound =  -100; port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC0CTRL2].UpperBound =  100;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST3SRC1CTRL0] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC1CTRL0].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC1CTRL0].LowerBound =  -100; port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC1CTRL0].UpperBound =  100;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST3SRC1CTRL1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC1CTRL1].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC1CTRL1].LowerBound =  -100; port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC1CTRL1].UpperBound =  100;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST3SRC1CTRL2] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC1CTRL2].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC1CTRL2].LowerBound =  -100; port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC1CTRL2].UpperBound =  100;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST3SRC2CTRL0] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC2CTRL0].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC2CTRL0].LowerBound =  -100; port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC2CTRL0].UpperBound =  100;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST3SRC2CTRL1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC2CTRL1].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC2CTRL1].LowerBound =  -100; port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC2CTRL1].UpperBound =  100;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST3SRC2CTRL2] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC2CTRL2].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC2CTRL2].LowerBound =  -100; port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC2CTRL2].UpperBound =  100;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST3SRC3CTRL0] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC3CTRL0].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC3CTRL0].LowerBound =  -100; port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC3CTRL0].UpperBound =  100;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST3SRC3CTRL1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC3CTRL1].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC3CTRL1].LowerBound =  -100; port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC3CTRL1].UpperBound =  100;

	port_descriptors[WAVV_PFXMATRIX_GRP0DST3SRC3CTRL2] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC3CTRL2].DefaultValue = 0.0f;
	port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC3CTRL2].LowerBound =  -100; port_range_hints[WAVV_PFXMATRIX_GRP0DST3SRC3CTRL2].UpperBound =  100;
                	
        port_descriptors[LMS_NOISE_TYPE] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[LMS_NOISE_TYPE].DefaultValue = 0.0f;
	port_range_hints[LMS_NOISE_TYPE].LowerBound =  0;
	port_range_hints[LMS_NOISE_TYPE].UpperBound =  2;
        
        port_descriptors[WAYV_ADSR1_CHECKBOX] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAYV_ADSR1_CHECKBOX].DefaultValue = 0.0f;
	port_range_hints[WAYV_ADSR1_CHECKBOX].LowerBound =  0;
	port_range_hints[WAYV_ADSR1_CHECKBOX].UpperBound =  1;
        
        port_descriptors[WAYV_ADSR2_CHECKBOX] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAYV_ADSR2_CHECKBOX].DefaultValue = 0.0f;
	port_range_hints[WAYV_ADSR2_CHECKBOX].LowerBound =  0;
	port_range_hints[WAYV_ADSR2_CHECKBOX].UpperBound =  1;        
        
	port_descriptors[WAYV_LFO_AMP] = port_descriptors[WAYV_ATTACK_MAIN];
	port_range_hints[WAYV_LFO_AMP].DefaultValue = 0.0f;
	port_range_hints[WAYV_LFO_AMP].LowerBound = -24.0f;
	port_range_hints[WAYV_LFO_AMP].UpperBound = 24.0f;
        
	port_descriptors[WAYV_LFO_PITCH] = port_descriptors[WAYV_ATTACK_MAIN];
	port_range_hints[WAYV_LFO_PITCH].DefaultValue = 0.0f;
	port_range_hints[WAYV_LFO_PITCH].LowerBound = -36.0f;
	port_range_hints[WAYV_LFO_PITCH].UpperBound = 36.0f;
                
	port_descriptors[WAYV_PITCH_ENV_AMT] = port_descriptors[WAYV_ATTACK_MAIN];
	port_range_hints[WAYV_PITCH_ENV_AMT].DefaultValue = 0.0f;
	port_range_hints[WAYV_PITCH_ENV_AMT].LowerBound =  -36.0f;
	port_range_hints[WAYV_PITCH_ENV_AMT].UpperBound =   36.0f;
        
	port_descriptors[WAYV_OSC2_UNISON_VOICES] = port_descriptors[WAYV_ATTACK_MAIN];
	port_range_hints[WAYV_OSC2_UNISON_VOICES].DefaultValue = 4.0f;
	port_range_hints[WAYV_OSC2_UNISON_VOICES].LowerBound =  1.0f;
	port_range_hints[WAYV_OSC2_UNISON_VOICES].UpperBound =  7.0f;
        
	port_descriptors[WAYV_OSC2_UNISON_SPREAD] = port_descriptors[WAYV_ATTACK_MAIN];
	port_range_hints[WAYV_OSC2_UNISON_SPREAD].DefaultValue = 50.0f;
	port_range_hints[WAYV_OSC2_UNISON_SPREAD].LowerBound =  0.0f;
	port_range_hints[WAYV_OSC2_UNISON_SPREAD].UpperBound =  100.0f;
        
	port_descriptors[WAYV_LFO_AMOUNT] = port_descriptors[WAYV_ATTACK_MAIN];
	port_range_hints[WAYV_LFO_AMOUNT].DefaultValue = 100.0f;
	port_range_hints[WAYV_LFO_AMOUNT].LowerBound = 0.0f;
	port_range_hints[WAYV_LFO_AMOUNT].UpperBound = 100.0f;
        
        port_descriptors[WAYV_OSC3_TYPE] = port_descriptors[WAYV_ATTACK_MAIN];
	port_range_hints[WAYV_OSC3_TYPE].DefaultValue = 0.0f;
	port_range_hints[WAYV_OSC3_TYPE].LowerBound =  0.0f;
	port_range_hints[WAYV_OSC3_TYPE].UpperBound =  (float)WT_TOTAL_WAVETABLE_COUNT;
        
	port_descriptors[WAYV_OSC3_PITCH] = port_descriptors[WAYV_ATTACK_MAIN];
	port_range_hints[WAYV_OSC3_PITCH].DefaultValue = 0.0f;
	port_range_hints[WAYV_OSC3_PITCH].LowerBound =  -36.0f;
	port_range_hints[WAYV_OSC3_PITCH].UpperBound =  36.0f;
        
	port_descriptors[WAYV_OSC3_TUNE] = port_descriptors[WAYV_ATTACK_MAIN];
	port_range_hints[WAYV_OSC3_TUNE].DefaultValue = 0.0f;
	port_range_hints[WAYV_OSC3_TUNE].LowerBound = -100.0f;
	port_range_hints[WAYV_OSC3_TUNE].UpperBound = 100.0f; 
        
	port_descriptors[WAYV_OSC3_VOLUME] = port_descriptors[WAYV_ATTACK_MAIN];
	port_range_hints[WAYV_OSC3_VOLUME].DefaultValue = 0.0f;
	port_range_hints[WAYV_OSC3_VOLUME].LowerBound =  -30.0f;
	port_range_hints[WAYV_OSC3_VOLUME].UpperBound =  0.0f;
        
        port_descriptors[WAYV_OSC3_UNISON_VOICES] = port_descriptors[WAYV_ATTACK_MAIN];
	port_range_hints[WAYV_OSC3_UNISON_VOICES].DefaultValue = 4.0f;
	port_range_hints[WAYV_OSC3_UNISON_VOICES].LowerBound =  1.0f;
	port_range_hints[WAYV_OSC3_UNISON_VOICES].UpperBound =  7.0f;
        
	port_descriptors[WAYV_OSC3_UNISON_SPREAD] = port_descriptors[WAYV_ATTACK_MAIN];
	port_range_hints[WAYV_OSC3_UNISON_SPREAD].DefaultValue = 50.0f;
	port_range_hints[WAYV_OSC3_UNISON_SPREAD].LowerBound =  0.0f;
	port_range_hints[WAYV_OSC3_UNISON_SPREAD].UpperBound =  100.0f;
        
        port_descriptors[WAYV_OSC1_FM1] = port_descriptors[WAYV_ATTACK_MAIN];
	port_range_hints[WAYV_OSC1_FM1].DefaultValue = 0.0f;
	port_range_hints[WAYV_OSC1_FM1].LowerBound =  0.0f;
	port_range_hints[WAYV_OSC1_FM1].UpperBound =  100.0f;
        
        port_descriptors[WAYV_OSC1_FM2] = port_descriptors[WAYV_ATTACK_MAIN];
	port_range_hints[WAYV_OSC1_FM2].DefaultValue = 0.0f;
	port_range_hints[WAYV_OSC1_FM2].LowerBound =  0.0f;
	port_range_hints[WAYV_OSC1_FM2].UpperBound =  100.0f;
                
        port_descriptors[WAYV_OSC1_FM3] = port_descriptors[WAYV_ATTACK_MAIN];
	port_range_hints[WAYV_OSC1_FM3].DefaultValue = 0.0f;
	port_range_hints[WAYV_OSC1_FM3].LowerBound =  0.0f;
	port_range_hints[WAYV_OSC1_FM3].UpperBound =  100.0f;
        
        port_descriptors[WAYV_OSC2_FM1] = port_descriptors[WAYV_ATTACK_MAIN];	
	port_range_hints[WAYV_OSC2_FM1].DefaultValue = 0.0f;
	port_range_hints[WAYV_OSC2_FM1].LowerBound =  0.0f;
	port_range_hints[WAYV_OSC2_FM1].UpperBound =  100.0f;
        
        port_descriptors[WAYV_OSC2_FM2] = port_descriptors[WAYV_ATTACK_MAIN];
	port_range_hints[WAYV_OSC2_FM2].DefaultValue = 0.0f;
	port_range_hints[WAYV_OSC2_FM2].LowerBound =  0.0f;
	port_range_hints[WAYV_OSC2_FM2].UpperBound =  100.0f;
        
        port_descriptors[WAYV_OSC2_FM3] = port_descriptors[WAYV_ATTACK_MAIN];
	port_range_hints[WAYV_OSC2_FM3].DefaultValue = 0.0f;
	port_range_hints[WAYV_OSC2_FM3].LowerBound =  0.0f;
	port_range_hints[WAYV_OSC2_FM3].UpperBound =  100.0f;
        
        port_descriptors[WAYV_OSC3_FM1] = port_descriptors[WAYV_ATTACK_MAIN];
	port_range_hints[WAYV_OSC3_FM1].DefaultValue = 0.0f;
	port_range_hints[WAYV_OSC3_FM1].LowerBound =  0.0f;
	port_range_hints[WAYV_OSC3_FM1].UpperBound =  100.0f;
        
        port_descriptors[WAYV_OSC3_FM2] = port_descriptors[WAYV_ATTACK_MAIN];
	port_range_hints[WAYV_OSC3_FM2].DefaultValue = 0.0f;
	port_range_hints[WAYV_OSC3_FM2].LowerBound =  0.0f;
	port_range_hints[WAYV_OSC3_FM2].UpperBound =  100.0f;
        
        port_descriptors[WAYV_OSC3_FM3] = port_descriptors[WAYV_ATTACK_MAIN];
	port_range_hints[WAYV_OSC3_FM3].DefaultValue = 0.0f;
	port_range_hints[WAYV_OSC3_FM3].LowerBound =  0.0f;
	port_range_hints[WAYV_OSC3_FM3].UpperBound =  100.0f;
        
        port_descriptors[WAYV_ATTACK3] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAYV_ATTACK3].DefaultValue = 10.0f;
	port_range_hints[WAYV_ATTACK3].LowerBound = 0.0f; 
	port_range_hints[WAYV_ATTACK3].UpperBound = 100.0f;

	port_descriptors[WAYV_DECAY3] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAYV_DECAY3].DefaultValue = 50.0f;
	port_range_hints[WAYV_DECAY3].LowerBound = 10.0f; 
	port_range_hints[WAYV_DECAY3].UpperBound = 100.0f;

	port_descriptors[WAYV_SUSTAIN3] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAYV_SUSTAIN3].DefaultValue = 0.0f;
	port_range_hints[WAYV_SUSTAIN3].LowerBound = -30.0f;
	port_range_hints[WAYV_SUSTAIN3].UpperBound = 0.0f;

	port_descriptors[WAYV_RELEASE3] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAYV_RELEASE3].DefaultValue = 50.0f;
	port_range_hints[WAYV_RELEASE3].LowerBound = 10.0f; 
	port_range_hints[WAYV_RELEASE3].UpperBound = 200.0f; 
        
        port_descriptors[WAYV_ADSR3_CHECKBOX] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_range_hints[WAYV_ADSR3_CHECKBOX].DefaultValue = 0.0f;
	port_range_hints[WAYV_ADSR3_CHECKBOX].LowerBound =  0;
	port_range_hints[WAYV_ADSR3_CHECKBOX].UpperBound =  1;
        
        
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
	LMSDDescriptor->configure = NULL;
	LMSDDescriptor->run_synth = v_run_wayv;
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