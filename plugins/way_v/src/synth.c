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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <stdint.h>

#include <math.h>
#include <stdio.h>

#include "dssi.h"
#include "ladspa.h"

#include "libmodsynth.h"
#include "../../libmodsynth/lib/amp.h"
#include "../../libmodsynth/modules/filter/svf.h"
#include "../../libmodsynth/lib/lms_math.h"
#include "synth.h"
#include "meta.h"

static LADSPA_Descriptor *LMSLDescriptor = NULL;
static DSSI_Descriptor *LMSDDescriptor = NULL;

static void v_run_wayv(LADSPA_Handle instance, unsigned long sample_count,
		  snd_seq_event_t * events, unsigned long EventCount);

static void v_run_wayv_voice(t_wayv *p, t_voc_single_voice a_poly_voice, t_rayv_poly_voice *d,
		      LADSPA_Data *out0, LADSPA_Data *out1, unsigned int count, int n);

__attribute__ ((visibility("default")))
const LADSPA_Descriptor *ladspa_descriptor(unsigned long index)
{
    switch (index) {
    case 0:
	return LMSLDescriptor;
    default:
	return NULL;
    }
}

__attribute__ ((visibility("default")))
const DSSI_Descriptor *dssi_descriptor(unsigned long index)
{
    switch (index) {
    case 0:
	return LMSDDescriptor;
    default:
	return NULL;
    }
}

static void v_cleanup_wayv(LADSPA_Handle instance)
{
    free(instance);
}

static void v_wayv_connect_port(LADSPA_Handle instance, unsigned long port,
			  LADSPA_Data * data)
{
    t_wayv *plugin;

    plugin = (t_wayv *) instance;
    
    /*Add the ports from step 9 to the connectPortLMS event handler*/
    
    switch (port) {
    case WAYV_OUTPUT0:
	plugin->output0 = data;
	break;
    case WAYV_OUTPUT1:
	plugin->output1 = data;
	break;
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
    //case LMS_GLOBAL_MIDI_OCTAVES_OFFSET: plugin->global_midi_octaves_offset = data; break;
    case LMS_NOISE_TYPE: plugin->noise_type = data; break;
    case WAYV_ADSR1_CHECKBOX: plugin->adsr1_checked = data; break;
    case WAYV_ADSR2_CHECKBOX: plugin->adsr2_checked = data; break;
    
    case WAYV_LFO_AMP: plugin->lfo_amp = data; break;
    case WAYV_LFO_PITCH: plugin->lfo_pitch = data; break;
    
    case RAYV_PITCH_ENV_AMT: plugin->pitch_env_amt = data; break;
    }
}

static LADSPA_Handle g_wayv_instantiate(const LADSPA_Descriptor * descriptor,
				   unsigned long s_rate)
{
    t_wayv *plugin_data = (t_wayv *) malloc(sizeof(t_wayv));
    
    plugin_data->fs = s_rate;
    
    plugin_data->midi_cc_map = g_ccm_get();
    v_ccm_set_cc(plugin_data->midi_cc_map, WAYV_FX0_KNOB0, 74, "FX1 knob 1");
    v_ccm_set_cc(plugin_data->midi_cc_map, WAYV_FX0_KNOB1, 71, "FX1 knob 2");
    v_ccm_set_cc(plugin_data->midi_cc_map, WAYV_FX0_KNOB2, 70, "FX1 knob 3");
    v_ccm_set_cc(plugin_data->midi_cc_map, WAYV_FX1_KNOB0, 91, "FX2 knob 1");
    v_ccm_set_cc(plugin_data->midi_cc_map, WAYV_FX1_KNOB1, 20, "FX2 knob 2");
    v_ccm_set_cc(plugin_data->midi_cc_map, WAYV_FX1_KNOB2, 21, "FX2 knob 3");    
    v_ccm_set_cc(plugin_data->midi_cc_map, WAYV_FX2_KNOB0, 22, "FX3 knob 1");
    v_ccm_set_cc(plugin_data->midi_cc_map, WAYV_FX2_KNOB1, 5, "FX3 knob 2");
    v_ccm_set_cc(plugin_data->midi_cc_map, WAYV_ATTACK_MAIN, 73, "Attack");
    v_ccm_set_cc(plugin_data->midi_cc_map, WAYV_RELEASE_MAIN, 72, "Release");
    v_ccm_set_cc(plugin_data->midi_cc_map, WAYV_LFO_FREQ, 15, "LFO Speed");
    v_ccm_set_cc(plugin_data->midi_cc_map, WAYV_LFO_AMP, 78, "Tremolo");
    v_ccm_set_cc(plugin_data->midi_cc_map, WAYV_LFO_PITCH, 9, "Vibrato");
    
    v_ccm_read_file_to_array(plugin_data->midi_cc_map, "way_v-cc_map.txt");
    
    /*LibModSynth additions*/
    v_rayv_init_lms(s_rate);  //initialize any static variables    
    /*End LibModSynth additions*/
    
    return (LADSPA_Handle) plugin_data;
}

static void v_wayv_activate(LADSPA_Handle instance)
{
    t_wayv *plugin_data = (t_wayv *) instance;
    unsigned int i;
    
    plugin_data->voices = g_voc_get_voices(WAYV_POLYPHONY);    
    
    for (i=0; i<WAYV_POLYPHONY; i++) {
        plugin_data->data[i] = g_rayv_poly_init(plugin_data->fs);
        plugin_data->data[i]->note_f = i;        
    }
    plugin_data->sampleNo = 0;
        
    //plugin_data->pitch = 1.0f;
    plugin_data->sv_pitch_bend_value = 0.0f;
    plugin_data->sv_last_note = 60.0f;  //For glide
    
    plugin_data->mono_modules = v_rayv_mono_init();  //initialize all monophonic modules
}

static void v_run_wayv(LADSPA_Handle instance, unsigned long sample_count,
		  snd_seq_event_t *events, unsigned long event_count)
{
    t_wayv *plugin_data = (t_wayv *) instance;
    
    LADSPA_Data *const output0 = plugin_data->output0;    
    LADSPA_Data *const output1 = plugin_data->output1;
    //t_poly_voice *data = plugin_data->data;
    
    plugin_data->i_run_poly_voice = 0;
    
    for(plugin_data->event_pos = 0; (plugin_data->event_pos) < event_count; plugin_data->event_pos = (plugin_data->event_pos) + 1)
    {
        //printf("plugin_data->event_pos == %i\n", plugin_data->event_pos);
        /*Note on event*/
        if (events[(plugin_data->event_pos)].type == SND_SEQ_EVENT_NOTEON) 
        {
            //printf("events[(plugin_data->event_pos)].type == SND_SEQ_EVENT_NOTEON\n");
            snd_seq_ev_note_t n = events[(plugin_data->event_pos)].data.note;

            //printf("n.note == %i\nn.velocity == %i\n n.duration == %i\n", n.note, n.velocity, n.duration);

            //printf("events[(plugin_data->event_pos)].time.tick == %i\nevents[(plugin_data->event_pos)].time.time.tv_sec == %i\n", 
            //        events[(plugin_data->event_pos)].time.tick, events[(plugin_data->event_pos)].time.time.tv_sec);

            if (n.velocity > 0) 
            {
                int f_voice = i_pick_voice(plugin_data->voices, n.note, plugin_data->sampleNo, events[(plugin_data->event_pos)].time.tick);
                
                plugin_data->data[f_voice]->amp = f_db_to_linear_fast(((n.velocity * 0.094488) - 12 + (*(plugin_data->master_vol))), //-20db to 0db, + master volume (0 to -60)
                        plugin_data->mono_modules->amp_ptr); 
                
                plugin_data->data[f_voice]->note_f = (float)n.note;
                plugin_data->data[f_voice]->note = n.note;

                plugin_data->data[f_voice]->target_pitch = (plugin_data->data[f_voice]->note_f);
                plugin_data->data[f_voice]->last_pitch = (plugin_data->sv_last_note);

                v_rmp_retrigger_glide_t(plugin_data->data[f_voice]->glide_env , (*(plugin_data->master_glide) * 0.01f), 
                        (plugin_data->sv_last_note), (plugin_data->data[f_voice]->target_pitch));

                plugin_data->data[f_voice]->osc1_linamp = f_db_to_linear_fast(*(plugin_data->osc1vol), plugin_data->mono_modules->amp_ptr); 
                plugin_data->data[f_voice]->osc2_linamp = f_db_to_linear_fast(*(plugin_data->osc2vol), plugin_data->mono_modules->amp_ptr);
                plugin_data->data[f_voice]->noise_linamp = f_db_to_linear_fast(*(plugin_data->noise_amp), plugin_data->mono_modules->amp_ptr);

                v_adsr_retrigger(plugin_data->data[f_voice]->adsr_main);
                v_adsr_retrigger(plugin_data->data[f_voice]->adsr_amp1);
                v_adsr_retrigger(plugin_data->data[f_voice]->adsr_amp2);

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
                    v_osc_wav_note_on_sync_phases(plugin_data->data[f_voice]->osc_wavtable1);
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
                    v_osc_wav_note_on_sync_phases(plugin_data->data[f_voice]->osc_wavtable2);
                }               
                
                
                plugin_data->data[f_voice]->noise_amp = f_db_to_linear(*(plugin_data->noise_amp), plugin_data->mono_modules->amp_ptr);

                int f_osc_type1 = (int)(*plugin_data->osc1type);
                
                if(f_osc_type1 < WT_TOTAL_WAVETABLE_COUNT)
                {
                    plugin_data->data[f_voice]->osc1_on = 1;
                    v_osc_wav_set_waveform(plugin_data->data[f_voice]->osc_wavtable1, 
                            plugin_data->data[f_voice]->wavetables->tables[f_osc_type1]->wavetable,
                            plugin_data->data[f_voice]->wavetables->tables[f_osc_type1]->length);
                    v_osc_wav_set_uni_voice_count(plugin_data->data[f_voice]->osc_wavtable1, *plugin_data->osc1_uni_voice);
                }
                
                int f_osc_type2 = (int)(*plugin_data->osc2type);
                
                if(f_osc_type2 < WT_TOTAL_WAVETABLE_COUNT)
                {
                    plugin_data->data[f_voice]->osc2_on = 1;
                    v_osc_wav_set_waveform(plugin_data->data[f_voice]->osc_wavtable2, 
                            plugin_data->data[f_voice]->wavetables->tables[f_osc_type2]->wavetable,
                            plugin_data->data[f_voice]->wavetables->tables[f_osc_type2]->length);
                    v_osc_wav_set_uni_voice_count(plugin_data->data[f_voice]->osc_wavtable2, *plugin_data->osc2_uni_voice);
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
                snd_seq_ev_note_t n = events[(plugin_data->event_pos)].data.note;

                v_voc_note_off(plugin_data->voices, n.note, (plugin_data->sampleNo), (events[(plugin_data->event_pos)].time.tick));
            }
        } 
        /*Note-off event*/
        else if (events[(plugin_data->event_pos)].type == SND_SEQ_EVENT_NOTEOFF) 
        {
            snd_seq_ev_note_t n = events[(plugin_data->event_pos)].data.note;

            v_voc_note_off(plugin_data->voices, n.note, (plugin_data->sampleNo), (events[(plugin_data->event_pos)].time.tick));
        } 
        /*Pitch-bend sequencer event, modify the voices pitch*/
        else if (events[(plugin_data->event_pos)].type == SND_SEQ_EVENT_PITCHBEND) 
        {
            plugin_data->sv_pitch_bend_value = 0.00012207f
                    * (events[(plugin_data->event_pos)].data.control.value) * (*plugin_data->master_pb_amt);
        }        
    }
    
    /*Clear the output buffer*/
    plugin_data->i_iterator = 0;

    while((plugin_data->i_iterator) < sample_count)
    {
        output0[(plugin_data->i_iterator)] = 0.0f;                        
        output1[(plugin_data->i_iterator)] = 0.0f;     
        plugin_data->i_iterator = (plugin_data->i_iterator) + 1;
    }    

    v_smr_iir_run_fast(plugin_data->mono_modules->pitchbend_smoother, (plugin_data->sv_pitch_bend_value));
    
    plugin_data->i_run_poly_voice = 0; 
    while ((plugin_data->i_run_poly_voice) < WAYV_POLYPHONY) 
    {
        //if (data[voice].state != inactive) 
        if((plugin_data->data[(plugin_data->i_run_poly_voice)]->adsr_main->stage) != 4)        
        {
            v_run_wayv_voice(plugin_data,                    
                    plugin_data->voices->voices[(plugin_data->i_run_poly_voice)],
                    plugin_data->data[(plugin_data->i_run_poly_voice)],                    
                    output0,
                    output1,
                    sample_count,
                    plugin_data->i_run_poly_voice
                    );
        }
        else
        {
            plugin_data->voices->voices[(plugin_data->i_run_poly_voice)].n_state = note_state_off;
        }

        plugin_data->i_run_poly_voice = (plugin_data->i_run_poly_voice) + 1; 
    }
        
    plugin_data->sampleNo += sample_count;
}

static void v_run_wayv_voice(t_wayv *plugin_data, t_voc_single_voice a_poly_voice, t_rayv_poly_voice *a_voice, LADSPA_Data *out0, LADSPA_Data *out1, unsigned int count, int n)
{   
    a_voice->i_voice = 0;
    
    if((plugin_data->sampleNo) < (a_poly_voice.on))
    {
        a_voice->i_voice =  (a_poly_voice.on) - (plugin_data->sampleNo);
    }
    
    for(; (a_voice->i_voice)<count;a_voice->i_voice = (a_voice->i_voice) + 1) 
    {           
        if ((((a_voice->i_voice) + (plugin_data->sampleNo)) == a_poly_voice.off) && ((a_voice->adsr_main->stage) < 3))
        {
            if(a_poly_voice.n_state == note_state_killed)
            {
                v_rayv_poly_note_off(a_voice, 1);
            }
            else
            {
                v_rayv_poly_note_off(a_voice, 0);
            }
	}        

        a_voice->current_sample = 0;
        
        f_rmp_run_ramp(a_voice->glide_env);
                        
        v_adsr_run_db(plugin_data->data[n]->adsr_amp);        

        v_adsr_run(plugin_data->data[n]->adsr_filter);
                
        f_rmp_run_ramp(plugin_data->data[n]->ramp_env);        
        
        //Set and run the LFO
        v_lfs_set(plugin_data->data[n]->lfo1,  (*(plugin_data->lfo_freq)) * .01);
        v_lfs_run(plugin_data->data[n]->lfo1);
        
        a_voice->lfo_amp_output = f_db_to_linear_fast((((*plugin_data->lfo_amp) * (a_voice->lfo1->output)) - (f_lms_abs((*plugin_data->lfo_amp)) * 0.5)), a_voice->amp_ptr);        
        a_voice->lfo_pitch_output = (*plugin_data->lfo_pitch) * (a_voice->lfo1->output);        
        
        a_voice->base_pitch = (a_voice->glide_env->output_multiplied) + ((a_voice->ramp_env->output_multiplied) * (*plugin_data->pitch_env_amt))
                + (plugin_data->mono_modules->pitchbend_smoother->output) + (a_voice->last_pitch) + (a_voice->lfo_pitch_output);
               
        if(a_voice->osc1_on)
        {
            v_osc_wav_set_unison_pitch(a_voice->osc_wavtable1, (*plugin_data->osc1_uni_spread) * 0.01f,
                    ((a_voice->base_pitch) + (*plugin_data->osc1pitch) + ((*plugin_data->osc1tune) * 0.01f) )); //+ (a_voice->lfo_pitch_output)));       
            if(a_voice->adsr_amp1_on)
            {
                v_adsr_run_db(a_voice->adsr_amp1);                
                a_voice->current_sample += f_osc_wav_run_unison(a_voice->osc_wavtable1) * (a_voice->adsr_amp1->output) * (a_voice->osc1_linamp);
            }
            else
            {
                a_voice->current_sample += f_osc_wav_run_unison(a_voice->osc_wavtable1) * (a_voice->osc1_linamp);
            }
        }
        
        if(a_voice->osc2_on)
        {
            v_osc_wav_set_unison_pitch(a_voice->osc_wavtable2, (*plugin_data->osc2_uni_spread) * 0.01f,
                    ((a_voice->base_pitch) + (*plugin_data->osc2pitch) + ((*plugin_data->osc2tune) * 0.01f) )); //+ (a_voice->lfo_pitch_output)));        
            if(a_voice->adsr_amp1_on)
            {
                v_adsr_run_db(a_voice->adsr_amp2);
                a_voice->current_sample += f_osc_wav_run_unison(a_voice->osc_wavtable2) * (a_voice->adsr_amp2->output) * (a_voice->osc2_linamp);
            }
            else
            {
                a_voice->current_sample += f_osc_wav_run_unison(a_voice->osc_wavtable2) * (a_voice->osc2_linamp);
            }
        }
        
        a_voice->current_sample += (f_run_white_noise(a_voice->white_noise1) * (a_voice->noise_linamp)); //white noise
        
        v_adsr_run_db(a_voice->adsr_main);
                
        a_voice->current_sample = (a_voice->current_sample) * (a_voice->amp) * (a_voice->lfo_amp_output);
        
        a_voice->modulex_current_sample[0] = (a_voice->current_sample);
        a_voice->modulex_current_sample[1] = (a_voice->current_sample);
        
        //Modular PolyFX, processed from the index created during note_on
        for(plugin_data->i_dst = 0; (plugin_data->i_dst) < (plugin_data->active_polyfx_count[n]); plugin_data->i_dst = (plugin_data->i_dst) + 1)
        {            
            v_mf3_set(a_voice->multieffect[(plugin_data->active_polyfx[n][(plugin_data->i_dst)])],
                *(plugin_data->pfx_mod_knob[0][(plugin_data->active_polyfx[n][(plugin_data->i_dst)])][0]), *(plugin_data->pfx_mod_knob[0][(plugin_data->active_polyfx[n][(plugin_data->i_dst)])][1]), *(plugin_data->pfx_mod_knob[0][(plugin_data->active_polyfx[n][(plugin_data->i_dst)])][2])); 

            int f_mod_test;

            for(f_mod_test = 0; f_mod_test < (plugin_data->polyfx_mod_counts[n][(plugin_data->active_polyfx[n][(plugin_data->i_dst)])]); f_mod_test++)
            {
                v_mf3_mod_single(
                        a_voice->multieffect[(plugin_data->active_polyfx[n][(plugin_data->i_dst)])],
                        *(a_voice->modulator_outputs[(plugin_data->polyfx_mod_src_index[n][(plugin_data->active_polyfx[n][(plugin_data->i_dst)])][f_mod_test])]),
                        (plugin_data->polyfx_mod_matrix_values[n][(plugin_data->active_polyfx[n][(plugin_data->i_dst)])][f_mod_test]),
                        (plugin_data->polyfx_mod_ctrl_indexes[n][(plugin_data->active_polyfx[n][(plugin_data->i_dst)])][f_mod_test])
                        );
            }

            a_voice->fx_func_ptr[(plugin_data->active_polyfx[n][(plugin_data->i_dst)])](a_voice->multieffect[(plugin_data->active_polyfx[n][(plugin_data->i_dst)])], (a_voice->modulex_current_sample[0]), (a_voice->modulex_current_sample[1])); 

            a_voice->modulex_current_sample[0] = a_voice->multieffect[(plugin_data->active_polyfx[n][(plugin_data->i_dst)])]->output0;
            a_voice->modulex_current_sample[1] = a_voice->multieffect[(plugin_data->active_polyfx[n][(plugin_data->i_dst)])]->output1;

        }
        
        
        /*Run the envelope and assign to the output buffers*/
        out0[(a_voice->i_voice)] += (a_voice->modulex_current_sample[0]) * (a_voice->adsr_main->output);
        out1[(a_voice->i_voice)] += (a_voice->modulex_current_sample[1]) * (a_voice->adsr_main->output);       
    }
}

/*This returns MIDI CCs for the different knobs*/ 
static int i_rayv_get_controller(LADSPA_Handle instance, unsigned long port)
{
    t_wayv *plugin_data = (t_wayv *) instance;
    return DSSI_CC(i_ccm_get_cc(plugin_data->midi_cc_map, port));
}

/*Here we define how all of the LADSPA and DSSI header stuff is setup,
 we also define the ports and the GUI.*/
#ifdef __GNUC__
__attribute__((constructor)) void v_wayv_constructor()
#else
void _init()
#endif
{
    char **port_names;
    LADSPA_PortDescriptor *port_descriptors;
    LADSPA_PortRangeHint *port_range_hints;

    LMSLDescriptor =
	(LADSPA_Descriptor *) malloc(sizeof(LADSPA_Descriptor));
    if (LMSLDescriptor) {
        LMSLDescriptor->UniqueID = WAYV_PLUGIN_UUID;  //Arbitrary number I made up, somewhat near the upper end of allowable UIDs
	LMSLDescriptor->Label = WAYV_PLUGIN_NAME;
	LMSLDescriptor->Properties = LADSPA_PROPERTY_REALTIME | LADSPA_PROPERTY_HARD_RT_CAPABLE; //0;
	LMSLDescriptor->Name = WAYV_PLUGIN_LONG_NAME;
	LMSLDescriptor->Maker = WAYV_PLUGIN_DEV;
	LMSLDescriptor->Copyright = "GNU GPL v3";
	LMSLDescriptor->PortCount = WAYV_COUNT;

	port_descriptors = (LADSPA_PortDescriptor *)
				calloc(LMSLDescriptor->PortCount, sizeof
						(LADSPA_PortDescriptor));
	LMSLDescriptor->PortDescriptors =
	    (const LADSPA_PortDescriptor *) port_descriptors;

	port_range_hints = (LADSPA_PortRangeHint *)
				calloc(LMSLDescriptor->PortCount, sizeof
						(LADSPA_PortRangeHint));
	LMSLDescriptor->PortRangeHints =
	    (const LADSPA_PortRangeHint *) port_range_hints;

	port_names = (char **) calloc(LMSLDescriptor->PortCount, sizeof(char *));
	LMSLDescriptor->PortNames = (const char **) port_names;

	/* Parameters for output */
	port_descriptors[WAYV_OUTPUT0] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
	port_names[WAYV_OUTPUT0] = "Output 0";
	port_range_hints[WAYV_OUTPUT0].HintDescriptor = 0;

        port_descriptors[WAYV_OUTPUT1] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
	port_names[WAYV_OUTPUT1] = "Output 1";
	port_range_hints[WAYV_OUTPUT1].HintDescriptor = 0;
        
        /*Define the LADSPA ports for the plugin in the class constructor*/
        
	/* Parameters for attack */
	port_descriptors[WAYV_ATTACK_MAIN] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[WAYV_ATTACK_MAIN] = "Attack time (s)";
	port_range_hints[WAYV_ATTACK_MAIN].HintDescriptor =
			LADSPA_HINT_DEFAULT_MINIMUM |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_ATTACK_MAIN].LowerBound = 10.0f; 
	port_range_hints[WAYV_ATTACK_MAIN].UpperBound = 100.0f; 

	/* Parameters for decay */
	port_descriptors[WAYV_DECAY_MAIN] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[WAYV_DECAY_MAIN] = "Decay time (s)";
	port_range_hints[WAYV_DECAY_MAIN].HintDescriptor =
			LADSPA_HINT_DEFAULT_LOW |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
			
	port_range_hints[WAYV_DECAY_MAIN].LowerBound = 10.0f; 
	port_range_hints[WAYV_DECAY_MAIN].UpperBound = 100.0f; 

	/* Parameters for sustain */
	port_descriptors[WAYV_SUSTAIN_MAIN] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[WAYV_SUSTAIN_MAIN] = "Sustain level (%)";
	port_range_hints[WAYV_SUSTAIN_MAIN].HintDescriptor =
			LADSPA_HINT_DEFAULT_MAXIMUM |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_SUSTAIN_MAIN].LowerBound = -30.0f;
	port_range_hints[WAYV_SUSTAIN_MAIN].UpperBound = 0.0f;

	/* Parameters for release */
	port_descriptors[WAYV_RELEASE_MAIN] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[WAYV_RELEASE_MAIN] = "Release time (s)";
	port_range_hints[WAYV_RELEASE_MAIN].HintDescriptor =
			LADSPA_HINT_DEFAULT_LOW | 
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_RELEASE_MAIN].LowerBound = 10.0f; 
	port_range_hints[WAYV_RELEASE_MAIN].UpperBound = 200.0f; 

        
        
	/* Parameters for attack */
	port_descriptors[WAYV_ATTACK1] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[WAYV_ATTACK1] = "Attack time (s)";
	port_range_hints[WAYV_ATTACK1].HintDescriptor =
			LADSPA_HINT_DEFAULT_MINIMUM |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_ATTACK1].LowerBound = 10.0f; 
	port_range_hints[WAYV_ATTACK1].UpperBound = 100.0f; 

	/* Parameters for decay */
	port_descriptors[WAYV_DECAY1] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[WAYV_DECAY1] = "Decay time (s)";
	port_range_hints[WAYV_DECAY1].HintDescriptor =
			LADSPA_HINT_DEFAULT_LOW |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
			
	port_range_hints[WAYV_DECAY1].LowerBound = 10.0f; 
	port_range_hints[WAYV_DECAY1].UpperBound = 100.0f; 

	/* Parameters for sustain */
	port_descriptors[WAYV_SUSTAIN1] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[WAYV_SUSTAIN1] = "Sustain level (%)";
	port_range_hints[WAYV_SUSTAIN1].HintDescriptor =
			LADSPA_HINT_DEFAULT_MAXIMUM |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_SUSTAIN1].LowerBound = -30.0f;
	port_range_hints[WAYV_SUSTAIN1].UpperBound = 0.0f;

	/* Parameters for release */
	port_descriptors[WAYV_RELEASE1] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[WAYV_RELEASE1] = "Release time (s)";
	port_range_hints[WAYV_RELEASE1].HintDescriptor =
			LADSPA_HINT_DEFAULT_LOW | 
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_RELEASE1].LowerBound = 10.0f; 
	port_range_hints[WAYV_RELEASE1].UpperBound = 200.0f; 
        
        
        
        
	/* Parameters for attack */
	port_descriptors[WAYV_ATTACK2] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[WAYV_ATTACK2] = "Attack time (s)";
	port_range_hints[WAYV_ATTACK2].HintDescriptor =
			LADSPA_HINT_DEFAULT_MINIMUM |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_ATTACK2].LowerBound = 10.0f; 
	port_range_hints[WAYV_ATTACK2].UpperBound = 100.0f; 

	/* Parameters for decay */
	port_descriptors[WAYV_DECAY2] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[WAYV_DECAY2] = "Decay time (s)";
	port_range_hints[WAYV_DECAY2].HintDescriptor =
			LADSPA_HINT_DEFAULT_LOW |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
			
	port_range_hints[WAYV_DECAY2].LowerBound = 10.0f; 
	port_range_hints[WAYV_DECAY2].UpperBound = 100.0f; 

	/* Parameters for sustain */
	port_descriptors[WAYV_SUSTAIN2] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[WAYV_SUSTAIN2] = "Sustain level (%)";
	port_range_hints[WAYV_SUSTAIN2].HintDescriptor =
			LADSPA_HINT_DEFAULT_MAXIMUM |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_SUSTAIN2].LowerBound = -30.0f;
	port_range_hints[WAYV_SUSTAIN2].UpperBound = 0.0f;

	/* Parameters for release */
	port_descriptors[WAYV_RELEASE2] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[WAYV_RELEASE2] = "Release time (s)";
	port_range_hints[WAYV_RELEASE2].HintDescriptor =
			LADSPA_HINT_DEFAULT_LOW | 
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_RELEASE2].LowerBound = 10.0f; 
	port_range_hints[WAYV_RELEASE2].UpperBound = 200.0f; 
        
        
        
        /*Parameters for noise_amp*/        
	port_descriptors[WAYV_NOISE_AMP] = port_descriptors[WAYV_ATTACK_MAIN];
	port_names[WAYV_NOISE_AMP] = "Dist";
	port_range_hints[WAYV_NOISE_AMP].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_NOISE_AMP].LowerBound =  -60.0f;
	port_range_hints[WAYV_NOISE_AMP].UpperBound =  0.0f;
                
        
        /*Parameters for osc1type*/        
	port_descriptors[WAYV_OSC1_TYPE] = port_descriptors[WAYV_ATTACK_MAIN];
	port_names[WAYV_OSC1_TYPE] = "Osc 1 Type";
	port_range_hints[WAYV_OSC1_TYPE].HintDescriptor =
			LADSPA_HINT_DEFAULT_MINIMUM |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_OSC1_TYPE].LowerBound =  0.0f;
	port_range_hints[WAYV_OSC1_TYPE].UpperBound =  (float)WT_TOTAL_WAVETABLE_COUNT;
        
        
        /*Parameters for osc1pitch*/        
	port_descriptors[WAYV_OSC1_PITCH] = port_descriptors[WAYV_ATTACK_MAIN];
	port_names[WAYV_OSC1_PITCH] = "Osc 1 Pitch";
	port_range_hints[WAYV_OSC1_PITCH].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_OSC1_PITCH].LowerBound =  -12.0f;
	port_range_hints[WAYV_OSC1_PITCH].UpperBound =  12.0f;
        
        
        /*Parameters for osc1tune*/        
	port_descriptors[WAYV_OSC1_TUNE] = port_descriptors[WAYV_ATTACK_MAIN];
	port_names[WAYV_OSC1_TUNE] = "Osc 1 Tune";
	port_range_hints[WAYV_OSC1_TUNE].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_OSC1_TUNE].LowerBound = -100.0f;
	port_range_hints[WAYV_OSC1_TUNE].UpperBound =  100.0f;
        
        
        /*Parameters for osc1vol*/        
	port_descriptors[WAYV_OSC1_VOLUME] = port_descriptors[WAYV_ATTACK_MAIN];
	port_names[WAYV_OSC1_VOLUME] = "Osc 1 Vol";
	port_range_hints[WAYV_OSC1_VOLUME].HintDescriptor =
			LADSPA_HINT_DEFAULT_MAXIMUM |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_OSC1_VOLUME].LowerBound =  -30.0f;
	port_range_hints[WAYV_OSC1_VOLUME].UpperBound =  0.0f;
        
        
        
        /*Parameters for osc2type*/        
	port_descriptors[WAYV_OSC2_TYPE] = port_descriptors[WAYV_ATTACK_MAIN];
	port_names[WAYV_OSC2_TYPE] = "Osc 2 Type";
	port_range_hints[WAYV_OSC2_TYPE].HintDescriptor =
			LADSPA_HINT_DEFAULT_MAXIMUM |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_OSC2_TYPE].LowerBound =  0.0f;
	port_range_hints[WAYV_OSC2_TYPE].UpperBound =  (float)WT_TOTAL_WAVETABLE_COUNT;
        
        
        /*Parameters for osc2pitch*/        
	port_descriptors[WAYV_OSC2_PITCH] = port_descriptors[WAYV_ATTACK_MAIN];
	port_names[WAYV_OSC2_PITCH] = "Osc 2 Pitch";
	port_range_hints[WAYV_OSC2_PITCH].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_OSC2_PITCH].LowerBound =  -12.0f;
	port_range_hints[WAYV_OSC2_PITCH].UpperBound =  12.0f;
        
        
        /*Parameters for osc2tune*/        
	port_descriptors[WAYV_OSC2_TUNE] = port_descriptors[WAYV_ATTACK_MAIN];
	port_names[WAYV_OSC2_TUNE] = "Osc 2 Tune";
	port_range_hints[WAYV_OSC2_TUNE].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_OSC2_TUNE].LowerBound = -100.0f;
	port_range_hints[WAYV_OSC2_TUNE].UpperBound = 100.0f; 
        
        
        /*Parameters for osc2vol*/        
	port_descriptors[WAYV_OSC2_VOLUME] = port_descriptors[WAYV_ATTACK_MAIN];
	port_names[WAYV_OSC2_VOLUME] = "Osc 2 Vol";
	port_range_hints[WAYV_OSC2_VOLUME].HintDescriptor =
			LADSPA_HINT_DEFAULT_MAXIMUM |
			LADSPA_HINT_BOUNDED_BELOW |  LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_OSC2_VOLUME].LowerBound =  -30.0f;
	port_range_hints[WAYV_OSC2_VOLUME].UpperBound =  0.0f;
        
        
        /*Parameters for master vol*/        
	port_descriptors[WAYV_MASTER_VOLUME] = port_descriptors[WAYV_ATTACK_MAIN];
	port_names[WAYV_MASTER_VOLUME] = "Master Vol";
	port_range_hints[WAYV_MASTER_VOLUME].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_MASTER_VOLUME].LowerBound =  -30.0f;
	port_range_hints[WAYV_MASTER_VOLUME].UpperBound =  12.0f;
        
        
        
        /*Parameters for master unison voices*/        
	port_descriptors[WAYV_OSC1_UNISON_VOICES] = port_descriptors[WAYV_ATTACK_MAIN];
	port_names[WAYV_OSC1_UNISON_VOICES] = "Master Unison";
	port_range_hints[WAYV_OSC1_UNISON_VOICES].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_OSC1_UNISON_VOICES].LowerBound =  1.0f;
	port_range_hints[WAYV_OSC1_UNISON_VOICES].UpperBound =  7.0f;
        
        
        /*Parameters for master unison spread*/        
	port_descriptors[WAYV_OSC1_UNISON_SPREAD] = port_descriptors[WAYV_ATTACK_MAIN];
	port_names[WAYV_OSC1_UNISON_SPREAD] = "Master Unison Spread";
	port_range_hints[WAYV_OSC1_UNISON_SPREAD].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_OSC1_UNISON_SPREAD].LowerBound =  0.0f;
	port_range_hints[WAYV_OSC1_UNISON_SPREAD].UpperBound =  100.0f;
        
        
        /*Parameters for master glide*/        
	port_descriptors[WAYV_MASTER_GLIDE] = port_descriptors[WAYV_ATTACK_MAIN];
	port_names[WAYV_MASTER_GLIDE] = "Master Glide";
	port_range_hints[WAYV_MASTER_GLIDE].HintDescriptor =
			LADSPA_HINT_DEFAULT_MINIMUM |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_MASTER_GLIDE].LowerBound =  0.0f;
	port_range_hints[WAYV_MASTER_GLIDE].UpperBound =  200.0f;
        
        
        /*Parameters for master pitchbend amt*/        
	port_descriptors[WAYV_MASTER_PITCHBEND_AMT] = port_descriptors[WAYV_ATTACK_MAIN];
	port_names[WAYV_MASTER_PITCHBEND_AMT] = "Pitchbend Amt";
	port_range_hints[WAYV_MASTER_PITCHBEND_AMT].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_MASTER_PITCHBEND_AMT].LowerBound =  1.0f;
	port_range_hints[WAYV_MASTER_PITCHBEND_AMT].UpperBound =  36.0f;
                        
        
        /* Parameters for attack */
	port_descriptors[WAYV_ATTACK_PFX1] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[WAYV_ATTACK_PFX1] = "Attack time (s)";
	port_range_hints[WAYV_ATTACK_PFX1].HintDescriptor =
			LADSPA_HINT_DEFAULT_MINIMUM |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_ATTACK_PFX1].LowerBound = 10; 
	port_range_hints[WAYV_ATTACK_PFX1].UpperBound = 100; 

	/* Parameters for decay */
	port_descriptors[WAYV_DECAY_PFX1] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[WAYV_DECAY_PFX1] = "Decay time (s)";
	port_range_hints[WAYV_DECAY_PFX1].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_DECAY_PFX1].LowerBound = 10; 
	port_range_hints[WAYV_DECAY_PFX1].UpperBound = 100; 

	/* Parameters for sustain */
	port_descriptors[WAYV_SUSTAIN_PFX1] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[WAYV_SUSTAIN_PFX1] = "Sustain level (%)";
	port_range_hints[WAYV_SUSTAIN_PFX1].HintDescriptor =
			LADSPA_HINT_DEFAULT_MAXIMUM |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_SUSTAIN_PFX1].LowerBound = -60;
	port_range_hints[WAYV_SUSTAIN_PFX1].UpperBound = 0;

	/* Parameters for release */
	port_descriptors[WAYV_RELEASE_PFX1] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[WAYV_RELEASE_PFX1] = "Release time (s)";
	port_range_hints[WAYV_RELEASE_PFX1].HintDescriptor =
			LADSPA_HINT_DEFAULT_LOW | 
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_RELEASE_PFX1].LowerBound = 10; 
	port_range_hints[WAYV_RELEASE_PFX1].UpperBound = 200; 
                
	/* Parameters for attack_f */
	port_descriptors[WAYV_ATTACK_PFX2] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[WAYV_ATTACK_PFX2] = "Attack time (s) filter";
	port_range_hints[WAYV_ATTACK_PFX2].HintDescriptor =
			LADSPA_HINT_DEFAULT_LOW |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_ATTACK_PFX2].LowerBound = 10; 
	port_range_hints[WAYV_ATTACK_PFX2].UpperBound = 100; 

	/* Parameters for decay_f */
	port_descriptors[WAYV_DECAY_PFX2] = port_descriptors[WAYV_ATTACK_PFX1];
	port_names[WAYV_DECAY_PFX2] = "Decay time (s) filter";
	port_range_hints[WAYV_DECAY_PFX2].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_DECAY_PFX2].LowerBound = 10;
	port_range_hints[WAYV_DECAY_PFX2].UpperBound = 100;

	/* Parameters for sustain_f */
	port_descriptors[WAYV_SUSTAIN_PFX2] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[WAYV_SUSTAIN_PFX2] = "Sustain level (%) filter";
	port_range_hints[WAYV_SUSTAIN_PFX2].HintDescriptor =
			LADSPA_HINT_DEFAULT_HIGH |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_SUSTAIN_PFX2].LowerBound = 0; 
	port_range_hints[WAYV_SUSTAIN_PFX2].UpperBound = 100; 
        
	/* Parameters for release_f */
	port_descriptors[WAYV_RELEASE_PFX2] = port_descriptors[WAYV_ATTACK_PFX1];
	port_names[WAYV_RELEASE_PFX2] = "Release time (s) filter";
	port_range_hints[WAYV_RELEASE_PFX2].HintDescriptor =
			LADSPA_HINT_DEFAULT_LOW  |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_RELEASE_PFX2].LowerBound = 10; 
	port_range_hints[WAYV_RELEASE_PFX2].UpperBound = 200; 

        
        /*Parameters for noise_amp*/        
	port_descriptors[WAYV_NOISE_AMP] = port_descriptors[WAYV_ATTACK_PFX1];
	port_names[WAYV_NOISE_AMP] = "Noise Amp";
	port_range_hints[WAYV_NOISE_AMP].HintDescriptor =
			LADSPA_HINT_DEFAULT_MINIMUM |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_NOISE_AMP].LowerBound =  -60;
	port_range_hints[WAYV_NOISE_AMP].UpperBound =  0;
                
        
        /*Parameters for pitch env time*/        
	port_descriptors[WAYV_RAMP_ENV_TIME] = port_descriptors[WAYV_ATTACK_PFX1];
	port_names[WAYV_RAMP_ENV_TIME] = "Pitch Env Time";
	port_range_hints[WAYV_RAMP_ENV_TIME].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_RAMP_ENV_TIME].LowerBound = 0; 
	port_range_hints[WAYV_RAMP_ENV_TIME].UpperBound = 200;
        
        /*Parameters for LFO Freq*/        
	port_descriptors[WAYV_LFO_FREQ] = port_descriptors[WAYV_ATTACK_PFX1];
	port_names[WAYV_LFO_FREQ] = "LFO Freq";
	port_range_hints[WAYV_LFO_FREQ].HintDescriptor =
			LADSPA_HINT_DEFAULT_LOW |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_LFO_FREQ].LowerBound = 10; 
	port_range_hints[WAYV_LFO_FREQ].UpperBound = 1600;
        
        /*Parameters for LFO Type*/        
	port_descriptors[WAYV_LFO_TYPE] = port_descriptors[WAYV_ATTACK_PFX1];
	port_names[WAYV_LFO_TYPE] = "LFO Type";
	port_range_hints[WAYV_LFO_TYPE].HintDescriptor =
			LADSPA_HINT_DEFAULT_MINIMUM |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_LFO_TYPE].LowerBound = 0; 
	port_range_hints[WAYV_LFO_TYPE].UpperBound = 2;
        
        port_descriptors[WAYV_FX0_KNOB0] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[WAYV_FX0_KNOB0] = "FX0 Knob0";
	port_range_hints[WAYV_FX0_KNOB0].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_FX0_KNOB0].LowerBound =  0;
	port_range_hints[WAYV_FX0_KNOB0].UpperBound =  127;
        
        	
	port_descriptors[WAYV_FX0_KNOB1] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[WAYV_FX0_KNOB1] = "FX0 Knob1";
	port_range_hints[WAYV_FX0_KNOB1].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_FX0_KNOB1].LowerBound =  0;
	port_range_hints[WAYV_FX0_KNOB1].UpperBound =  127;
        	
	port_descriptors[WAYV_FX0_KNOB2] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[WAYV_FX0_KNOB2] = "FX0 Knob2";
	port_range_hints[WAYV_FX0_KNOB2].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_FX0_KNOB2].LowerBound =  0;
	port_range_hints[WAYV_FX0_KNOB2].UpperBound =  127;
        
	port_descriptors[WAYV_FX0_COMBOBOX] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[WAYV_FX0_COMBOBOX] = "FX0 Type";
	port_range_hints[WAYV_FX0_COMBOBOX].HintDescriptor =
                        LADSPA_HINT_DEFAULT_MINIMUM | LADSPA_HINT_INTEGER |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_FX0_COMBOBOX].LowerBound =  0;
	port_range_hints[WAYV_FX0_COMBOBOX].UpperBound =  MULTIFX3KNOB_MAX_INDEX;
        
        	
	port_descriptors[WAYV_FX1_KNOB0] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[WAYV_FX1_KNOB0] = "FX1 Knob0";
	port_range_hints[WAYV_FX1_KNOB0].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_FX1_KNOB0].LowerBound =  0;
	port_range_hints[WAYV_FX1_KNOB0].UpperBound =  127;
        
        	
	port_descriptors[WAYV_FX1_KNOB1] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[WAYV_FX1_KNOB1] = "FX1 Knob1";
	port_range_hints[WAYV_FX1_KNOB1].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_FX1_KNOB1].LowerBound =  0;
	port_range_hints[WAYV_FX1_KNOB1].UpperBound =  127;
        	
	port_descriptors[WAYV_FX1_KNOB2] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[WAYV_FX1_KNOB2] = "FX1 Knob2";
	port_range_hints[WAYV_FX1_KNOB2].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_FX1_KNOB2].LowerBound =  0;
	port_range_hints[WAYV_FX1_KNOB2].UpperBound =  127;
        
	port_descriptors[WAYV_FX1_COMBOBOX] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[WAYV_FX1_COMBOBOX] = "FX1 Type";
	port_range_hints[WAYV_FX1_COMBOBOX].HintDescriptor =
                        LADSPA_HINT_DEFAULT_MINIMUM | LADSPA_HINT_INTEGER |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_FX1_COMBOBOX].LowerBound =  0;
	port_range_hints[WAYV_FX1_COMBOBOX].UpperBound =  MULTIFX3KNOB_MAX_INDEX;
        
        
        
        port_descriptors[WAYV_FX2_KNOB0] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[WAYV_FX2_KNOB0] = "FX2 Knob0";
	port_range_hints[WAYV_FX2_KNOB0].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_FX2_KNOB0].LowerBound =  0;
	port_range_hints[WAYV_FX2_KNOB0].UpperBound =  127;
        
        	
	port_descriptors[WAYV_FX2_KNOB1] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[WAYV_FX2_KNOB1] = "FX2 Knob1";
	port_range_hints[WAYV_FX2_KNOB1].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_FX2_KNOB1].LowerBound =  0;
	port_range_hints[WAYV_FX2_KNOB1].UpperBound =  127;
        	
	port_descriptors[WAYV_FX2_KNOB2] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[WAYV_FX2_KNOB2] = "FX2 Knob2";
	port_range_hints[WAYV_FX2_KNOB2].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_FX2_KNOB2].LowerBound =  0;
	port_range_hints[WAYV_FX2_KNOB2].UpperBound =  127;
        
	port_descriptors[WAYV_FX2_COMBOBOX] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[WAYV_FX2_COMBOBOX] = "FX2 Type";
	port_range_hints[WAYV_FX2_COMBOBOX].HintDescriptor =
                        LADSPA_HINT_DEFAULT_MINIMUM | LADSPA_HINT_INTEGER |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_FX2_COMBOBOX].LowerBound =  0;
	port_range_hints[WAYV_FX2_COMBOBOX].UpperBound =  MULTIFX3KNOB_MAX_INDEX;
        
        	
	port_descriptors[WAYV_FX3_KNOB0] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[WAYV_FX3_KNOB0] = "FX3 Knob0";
	port_range_hints[WAYV_FX3_KNOB0].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_FX3_KNOB0].LowerBound =  0;
	port_range_hints[WAYV_FX3_KNOB0].UpperBound =  127;
        
        	
	port_descriptors[WAYV_FX3_KNOB1] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[WAYV_FX3_KNOB1] = "FX3 Knob1";
	port_range_hints[WAYV_FX3_KNOB1].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_FX3_KNOB1].LowerBound =  0;
	port_range_hints[WAYV_FX3_KNOB1].UpperBound =  127;
        	
	port_descriptors[WAYV_FX3_KNOB2] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[WAYV_FX3_KNOB2] = "FX3 Knob2";
	port_range_hints[WAYV_FX3_KNOB2].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_FX3_KNOB2].LowerBound =  0;
	port_range_hints[WAYV_FX3_KNOB2].UpperBound =  127;
        
	port_descriptors[WAYV_FX3_COMBOBOX] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[WAYV_FX3_COMBOBOX] = "FX3 Type";
	port_range_hints[WAYV_FX3_COMBOBOX].HintDescriptor =
                        LADSPA_HINT_DEFAULT_MINIMUM | LADSPA_HINT_INTEGER |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_FX3_COMBOBOX].LowerBound =  0;
	port_range_hints[WAYV_FX3_COMBOBOX].UpperBound =  MULTIFX3KNOB_MAX_INDEX;
        
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
        
	/*port_descriptors[LMS_GLOBAL_MIDI_OCTAVES_OFFSET] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_GLOBAL_MIDI_OCTAVES_OFFSET] = "Global MIDI Offset(Octaves)";
	port_range_hints[LMS_GLOBAL_MIDI_OCTAVES_OFFSET].HintDescriptor =
                        LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_INTEGER |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_GLOBAL_MIDI_OCTAVES_OFFSET].LowerBound =  -3;
	port_range_hints[LMS_GLOBAL_MIDI_OCTAVES_OFFSET].UpperBound =  3;
        */
        port_descriptors[LMS_NOISE_TYPE] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_NOISE_TYPE] = "Noise Type";
	port_range_hints[LMS_NOISE_TYPE].HintDescriptor =
                        LADSPA_HINT_DEFAULT_MINIMUM | LADSPA_HINT_INTEGER |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_NOISE_TYPE].LowerBound =  0;
	port_range_hints[LMS_NOISE_TYPE].UpperBound =  2;
        
        port_descriptors[WAYV_ADSR1_CHECKBOX] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[WAYV_ADSR1_CHECKBOX] = "ADSR1 Checkbox";
	port_range_hints[WAYV_ADSR1_CHECKBOX].HintDescriptor =
                        LADSPA_HINT_DEFAULT_MINIMUM | LADSPA_HINT_INTEGER |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_ADSR1_CHECKBOX].LowerBound =  0;
	port_range_hints[WAYV_ADSR1_CHECKBOX].UpperBound =  1;
        
        port_descriptors[WAYV_ADSR2_CHECKBOX] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[WAYV_ADSR2_CHECKBOX] = "ADSR2 Checkbox";
	port_range_hints[WAYV_ADSR2_CHECKBOX].HintDescriptor =
                        LADSPA_HINT_DEFAULT_MINIMUM | LADSPA_HINT_INTEGER |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_ADSR2_CHECKBOX].LowerBound =  0;
	port_range_hints[WAYV_ADSR2_CHECKBOX].UpperBound =  1;
        
        
        /*Parameters for LFO Amp*/
	port_descriptors[WAYV_LFO_AMP] = port_descriptors[WAYV_ATTACK_MAIN];
	port_names[WAYV_LFO_AMP] = "LFO Amp";
	port_range_hints[WAYV_LFO_AMP].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_LFO_AMP].LowerBound = -24.0f;
	port_range_hints[WAYV_LFO_AMP].UpperBound = 24.0f;
        
        /*Parameters for LFO Pitch*/
	port_descriptors[WAYV_LFO_PITCH] = port_descriptors[WAYV_ATTACK_MAIN];
	port_names[WAYV_LFO_PITCH] = "LFO Pitch";
	port_range_hints[WAYV_LFO_PITCH].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_LFO_PITCH].LowerBound = -36.0f;
	port_range_hints[WAYV_LFO_PITCH].UpperBound = 36.0f;
        
        
        /*Parameters for pitch env amt*/        
	port_descriptors[RAYV_PITCH_ENV_AMT] = port_descriptors[WAYV_ATTACK_MAIN];
	port_names[RAYV_PITCH_ENV_AMT] = "Pitch Env Amt";
	port_range_hints[RAYV_PITCH_ENV_AMT].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[RAYV_PITCH_ENV_AMT].LowerBound =  -36.0f;
	port_range_hints[RAYV_PITCH_ENV_AMT].UpperBound =   36.0f;
                
        /*Parameters for master unison voices*/        
	port_descriptors[WAYV_OSC2_UNISON_VOICES] = port_descriptors[WAYV_ATTACK_MAIN];
	port_names[WAYV_OSC2_UNISON_VOICES] = "Master Unison";
	port_range_hints[WAYV_OSC2_UNISON_VOICES].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_OSC2_UNISON_VOICES].LowerBound =  1.0f;
	port_range_hints[WAYV_OSC2_UNISON_VOICES].UpperBound =  7.0f;
        
        
        /*Parameters for master unison spread*/        
	port_descriptors[WAYV_OSC2_UNISON_SPREAD] = port_descriptors[WAYV_ATTACK_MAIN];
	port_names[WAYV_OSC2_UNISON_SPREAD] = "Master Unison Spread";
	port_range_hints[WAYV_OSC2_UNISON_SPREAD].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_OSC2_UNISON_SPREAD].LowerBound =  0.0f;
	port_range_hints[WAYV_OSC2_UNISON_SPREAD].UpperBound =  100.0f;
        
        
	LMSLDescriptor->activate = v_wayv_activate;
	LMSLDescriptor->cleanup = v_cleanup_wayv;
	LMSLDescriptor->connect_port = v_wayv_connect_port;
	LMSLDescriptor->deactivate = NULL;
	LMSLDescriptor->instantiate = g_wayv_instantiate;
	LMSLDescriptor->run = NULL;
	LMSLDescriptor->run_adding = NULL;
	LMSLDescriptor->set_run_adding_gain = NULL;
    }

    LMSDDescriptor = (DSSI_Descriptor *) malloc(sizeof(DSSI_Descriptor));
    if (LMSDDescriptor) {
	LMSDDescriptor->DSSI_API_Version = 1;
	LMSDDescriptor->LADSPA_Plugin = LMSLDescriptor;
	LMSDDescriptor->configure = NULL;  //TODO:  I think this is where the host can set plugin state, etc...
	LMSDDescriptor->get_program = NULL;  //TODO:  This is where program change is read, plugin state retrieved, etc...
	LMSDDescriptor->get_midi_controller_for_port = i_rayv_get_controller;
	LMSDDescriptor->select_program = NULL;  //TODO:  This is how the host can select programs, not sure how it differs from a MIDI program change
	LMSDDescriptor->run_synth = v_run_wayv;
	LMSDDescriptor->run_synth_adding = NULL;
	LMSDDescriptor->run_multiple_synths = NULL;
	LMSDDescriptor->run_multiple_synths_adding = NULL;
    }
}

#ifdef __GNUC__
__attribute__((destructor)) void v_wayv_destructor()
#else
void _fini()
#endif
{
    if (LMSLDescriptor) {
	free((LADSPA_PortDescriptor *) LMSLDescriptor->PortDescriptors);
	free((char **) LMSLDescriptor->PortNames);
	free((LADSPA_PortRangeHint *) LMSLDescriptor->PortRangeHints);
	free(LMSLDescriptor);
    }
    if (LMSDDescriptor) {
	free(LMSDDescriptor);
    }
}
