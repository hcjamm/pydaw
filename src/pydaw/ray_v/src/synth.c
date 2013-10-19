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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

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
#include "meta.h"




static void v_run_rayv(PYFX_Handle instance, int sample_count,
		  t_pydaw_seq_event * events, int EventCount);

static void v_run_rayv_voice(t_rayv *p, t_voc_single_voice a_poly_voice, t_rayv_poly_voice *d,
		      PYFX_Data *out0, PYFX_Data *out1, unsigned int count);

const PYFX_Descriptor *rayv_PYFX_descriptor(int index);
const PYINST_Descriptor *rayv_PYINST_descriptor(int index);

static void v_cleanup_rayv(PYFX_Handle instance)
{
    free(instance);
}


static void rayvPanic(PYFX_Handle instance)
{
    t_rayv *plugin = (t_rayv *)instance;
    int f_i = 0;
    while(f_i < RAYV_POLYPHONY)
    {
        v_adsr_kill(plugin->data[f_i]->adsr_amp);
        f_i++;
    }    
}

static void v_rayv_connect_port(PYFX_Handle instance, int port,
			  PYFX_Data * data)
{
    t_rayv *plugin;

    plugin = (t_rayv *) instance;
    
    /*Add the ports from step 9 to the connectPortLMS event handler*/
    
    switch (port) {
    case RAYV_OUTPUT0:
	plugin->output0 = data;
	break;
    case RAYV_OUTPUT1:
	plugin->output1 = data;
	break;
    case RAYV_ATTACK:
	plugin->attack = data;
	break;
    case RAYV_DECAY:
	plugin->decay = data;
	break;
    case RAYV_SUSTAIN:
	plugin->sustain = data;
	break;
    case RAYV_RELEASE:
	plugin->release = data;
	break;
    case RAYV_TIMBRE:
	plugin->timbre = data;              
	break;
    case RAYV_RES:
	plugin->res = data;              
	break;
    case RAYV_DIST:
	plugin->dist = data;              
	break;
    case RAYV_FILTER_ATTACK:
	plugin->attack_f = data;
	break;
    case RAYV_FILTER_DECAY:
	plugin->decay_f = data;
	break;
    case RAYV_FILTER_SUSTAIN:
	plugin->sustain_f = data;
	break;
    case RAYV_FILTER_RELEASE:
	plugin->release_f = data;
	break;
    case RAYV_NOISE_AMP:
        plugin->noise_amp = data;
        break;
    case RAYV_DIST_WET:
        plugin->dist_wet = data;
        break;
    case RAYV_FILTER_ENV_AMT:
        plugin->filter_env_amt = data;
        break;
    case RAYV_MASTER_VOLUME:
        plugin->master_vol = data;
        break;
    case RAYV_OSC1_PITCH:
        plugin->osc1pitch = data;
        break;
    case RAYV_OSC1_TUNE:
        plugin->osc1tune = data;
        break;
    case RAYV_OSC1_TYPE:
        plugin->osc1type = data;
        break;
    case RAYV_OSC1_VOLUME:
        plugin->osc1vol = data;
        break;
    case RAYV_OSC2_PITCH:
        plugin->osc2pitch = data;
        break;
    case RAYV_OSC2_TUNE:
        plugin->osc2tune = data;
        break;
    case RAYV_OSC2_TYPE:
        plugin->osc2type = data;
        break;
    case RAYV_OSC2_VOLUME:
        plugin->osc2vol = data;
        break;
    case RAYV_MASTER_UNISON_VOICES:
        plugin->master_uni_voice = data;
        break;
    case RAYV_MASTER_UNISON_SPREAD:
        plugin->master_uni_spread = data;        
        break;
    case RAYV_MASTER_GLIDE:
        plugin->master_glide = data;
        break;
    case RAYV_MASTER_PITCHBEND_AMT:
        plugin->master_pb_amt = data;
        break;
    case RAYV_PITCH_ENV_AMT:
        plugin->pitch_env_amt = data;
        break;
    case RAYV_PITCH_ENV_TIME:
        plugin->pitch_env_time = data;
        break;
    /*case LMS_PROGRAM_CHANGE:
        plugin->program = data;
        break;*/
    case RAYV_LFO_FREQ:
        plugin->lfo_freq = data;
        break;
    case RAYV_LFO_TYPE:
        plugin->lfo_type = data;
        break;
    case RAYV_LFO_AMP:
        plugin->lfo_amp = data;
        break;
    case RAYV_LFO_PITCH:
        plugin->lfo_pitch = data;
        break;
    case RAYV_LFO_FILTER:
        plugin->lfo_filter = data;
        break;
    case RAYV_OSC_HARD_SYNC:
        plugin->sync_hard = data;
        break;
    }
}

static PYFX_Handle g_rayv_instantiate(const PYFX_Descriptor * descriptor,
				   int s_rate, fp_get_wavpool_item_from_host a_host_wavpool_func)
{
    t_rayv *plugin_data = (t_rayv *) malloc(sizeof(t_rayv));
    
    plugin_data->fs = s_rate;    
    v_rayv_init_lms(s_rate);
        
    return (PYFX_Handle) plugin_data;
}

static void v_rayv_activate(PYFX_Handle instance)
{
    t_rayv *plugin_data = (t_rayv *) instance;
    unsigned int i;
    
    plugin_data->voices = g_voc_get_voices(RAYV_POLYPHONY);    
    
    for (i=0; i<RAYV_POLYPHONY; i++) {
        plugin_data->data[i] = g_rayv_poly_init();
        plugin_data->data[i]->note_f = i;        
    }
    plugin_data->sampleNo = 0;
        
    plugin_data->pitch = 1.0f;
    plugin_data->sv_pitch_bend_value = 0.0f;
    plugin_data->sv_last_note = 60.0f;  //For glide
    
    plugin_data->mono_modules = v_rayv_mono_init();  //initialize all monophonic modules
}

static void v_run_rayv(PYFX_Handle instance, int sample_count,
		  t_pydaw_seq_event *events, int event_count)
{
    t_rayv *plugin_data = (t_rayv *) instance;
        
    plugin_data->i_run_poly_voice = 0;
    
    for(plugin_data->event_pos = 0; (plugin_data->event_pos) < event_count; plugin_data->event_pos = (plugin_data->event_pos) + 1)
    {
        if (events[(plugin_data->event_pos)].type == PYDAW_EVENT_NOTEON) 
        {
            if (events[(plugin_data->event_pos)].velocity > 0) 
            {
                int f_voice = i_pick_voice(plugin_data->voices, events[(plugin_data->event_pos)].note, plugin_data->sampleNo, events[(plugin_data->event_pos)].tick);
                
                plugin_data->data[f_voice]->amp = f_db_to_linear_fast(((events[(plugin_data->event_pos)].velocity * 0.094488) - 12.0f + (*(plugin_data->master_vol))), //-20db to 0db, + master volume (0 to -60)
                        plugin_data->mono_modules->amp_ptr); 
                v_svf_velocity_mod(plugin_data->data[f_voice]->svf_filter, events[(plugin_data->event_pos)].velocity);

                plugin_data->data[f_voice]->note_f = (float)events[(plugin_data->event_pos)].note;
                plugin_data->data[f_voice]->note = events[(plugin_data->event_pos)].note;

                plugin_data->data[f_voice]->target_pitch = (plugin_data->data[f_voice]->note_f);
                plugin_data->data[f_voice]->last_pitch = (plugin_data->sv_last_note);
                
                plugin_data->data[f_voice]->osc1_pitch_adjust = (*plugin_data->osc1pitch) + ((*plugin_data->osc1tune) * 0.01f);
                plugin_data->data[f_voice]->osc2_pitch_adjust = (*plugin_data->osc2pitch) + ((*plugin_data->osc2tune) * 0.01f);

                v_rmp_retrigger_glide_t(plugin_data->data[f_voice]->glide_env , (*(plugin_data->master_glide) * 0.01f), 
                        (plugin_data->sv_last_note), (plugin_data->data[f_voice]->target_pitch));

                plugin_data->data[f_voice]->osc1_linamp = f_db_to_linear_fast(*(plugin_data->osc1vol), plugin_data->mono_modules->amp_ptr); 
                plugin_data->data[f_voice]->osc2_linamp = f_db_to_linear_fast(*(plugin_data->osc2vol), plugin_data->mono_modules->amp_ptr);
                plugin_data->data[f_voice]->noise_linamp = f_db_to_linear_fast(*(plugin_data->noise_amp), plugin_data->mono_modules->amp_ptr);

                plugin_data->data[f_voice]->unison_spread = (*plugin_data->master_uni_spread) * 0.01f;
                
                v_adsr_retrigger(plugin_data->data[f_voice]->adsr_amp);
                v_adsr_retrigger(plugin_data->data[f_voice]->adsr_filter);
                v_lfs_sync(plugin_data->data[f_voice]->lfo1, 0.0f, *(plugin_data->lfo_type));

                float f_attack = *(plugin_data->attack) * .01;
                f_attack = (f_attack) * (f_attack);
                float f_decay = *(plugin_data->decay) * .01;
                f_decay = (f_decay) * (f_decay);
                float f_release = *(plugin_data->release) * .01;
                f_release = (f_release) * (f_release);                
                
                v_adsr_set_adsr_db(plugin_data->data[f_voice]->adsr_amp, (f_attack), (f_decay), *(plugin_data->sustain), (f_release));
                
                float f_attack_f = *(plugin_data->attack_f) * .01;
                f_attack_f = (f_attack_f) * (f_attack_f);
                float f_decay_f = *(plugin_data->decay_f) * .01; 
                f_decay_f = (f_decay_f) * (f_decay_f);                
                float f_release_f = *(plugin_data->release_f) * .01;
                f_release_f = (f_release_f) * (f_release_f);
                
                v_adsr_set_adsr(plugin_data->data[f_voice]->adsr_filter, (f_attack_f), (f_decay_f), *(plugin_data->sustain_f) * 0.01f, (f_release_f));

                v_rmp_retrigger((plugin_data->data[f_voice]->pitch_env), *(plugin_data->pitch_env_time) * 0.01f, *(plugin_data->pitch_env_amt));  

                v_clp_set_in_gain(plugin_data->data[f_voice]->clipper1, *plugin_data->dist);

                v_svf_set_res(plugin_data->data[f_voice]->svf_filter, *plugin_data->res);  

                plugin_data->data[f_voice]->noise_amp = f_db_to_linear(*(plugin_data->noise_amp), plugin_data->mono_modules->amp_ptr);

                v_axf_set_xfade(plugin_data->data[f_voice]->dist_dry_wet, *(plugin_data->dist_wet) * 0.01f);       

                plugin_data->data[f_voice]->hard_sync = (int)(*plugin_data->sync_hard);
                
                v_osc_set_simple_osc_unison_type(plugin_data->data[f_voice]->osc_unison1, (int)(*plugin_data->osc1type));
                v_osc_set_simple_osc_unison_type(plugin_data->data[f_voice]->osc_unison2, (int)(*plugin_data->osc2type));   
                v_osc_note_on_sync_phases(plugin_data->data[f_voice]->osc_unison1);
                v_osc_note_on_sync_phases(plugin_data->data[f_voice]->osc_unison2);

                v_osc_set_uni_voice_count(plugin_data->data[f_voice]->osc_unison1, *plugin_data->master_uni_voice);
                
                if(plugin_data->data[f_voice]->hard_sync)
                {
                    v_osc_set_uni_voice_count(plugin_data->data[f_voice]->osc_unison2, 1);
                }
                else
                {
                    v_osc_set_uni_voice_count(plugin_data->data[f_voice]->osc_unison2, *plugin_data->master_uni_voice);
                }                

                plugin_data->sv_last_note = (plugin_data->data[f_voice]->note_f);
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
            plugin_data->sv_pitch_bend_value = 0.00012207f
                    * (events[(plugin_data->event_pos)].value) * (*plugin_data->master_pb_amt);
        }        
    }
    
    /*Clear the output buffer*/
    plugin_data->i_iterator = 0;

    while((plugin_data->i_iterator) < sample_count)
    {
        plugin_data->output0[(plugin_data->i_iterator)] = 0.0f;                        
        plugin_data->output1[(plugin_data->i_iterator)] = 0.0f;     
        plugin_data->i_iterator = (plugin_data->i_iterator) + 1;
    }    

    v_smr_iir_run_fast(plugin_data->mono_modules->filter_smoother, (*plugin_data->timbre));
    v_smr_iir_run_fast(plugin_data->mono_modules->pitchbend_smoother, (plugin_data->sv_pitch_bend_value));
    
    plugin_data->i_run_poly_voice = 0; 
    while ((plugin_data->i_run_poly_voice) < RAYV_POLYPHONY) 
    {
        //if (data[voice].state != inactive) 
        if((plugin_data->data[(plugin_data->i_run_poly_voice)]->adsr_amp->stage) != 4)        
        {
            v_run_rayv_voice(plugin_data,                    
                    plugin_data->voices->voices[(plugin_data->i_run_poly_voice)],
                    plugin_data->data[(plugin_data->i_run_poly_voice)],                    
                    plugin_data->output0,
                    plugin_data->output1,
                    sample_count
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

static void v_run_rayv_voice(t_rayv *plugin_data, t_voc_single_voice a_poly_voice, t_rayv_poly_voice *a_voice, PYFX_Data *out0, PYFX_Data *out1, unsigned int count)
{   
    a_voice->i_voice = 0;
    
    if((plugin_data->sampleNo) < (a_poly_voice.on))
    {
        a_voice->i_voice =  (a_poly_voice.on) - (plugin_data->sampleNo);
    }
    
    for(; (a_voice->i_voice)<count;a_voice->i_voice = (a_voice->i_voice) + 1) 
    {           
        if ((((a_voice->i_voice) + (plugin_data->sampleNo)) == a_poly_voice.off) && ((a_voice->adsr_amp->stage) < 3))
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
        
        f_rmp_run_ramp(a_voice->pitch_env);
        f_rmp_run_ramp(a_voice->glide_env);

        v_lfs_set(a_voice->lfo1, (*plugin_data->lfo_freq) * 0.01f);
        v_lfs_run(a_voice->lfo1);
        a_voice->lfo_amp_output = f_db_to_linear_fast((((*plugin_data->lfo_amp) * (a_voice->lfo1->output)) - (f_lms_abs((*plugin_data->lfo_amp)) * 0.5)), a_voice->amp_ptr);
        a_voice->lfo_filter_output = (*plugin_data->lfo_filter) * (a_voice->lfo1->output);
        a_voice->lfo_pitch_output = (*plugin_data->lfo_pitch) * (a_voice->lfo1->output);

        if(a_voice->hard_sync)
        {
            a_voice->base_pitch = (a_voice->glide_env->output_multiplied) + (a_voice->pitch_env->output_multiplied) 
                    + (plugin_data->mono_modules->pitchbend_smoother->output) + (a_voice->last_pitch) + (a_voice->lfo_pitch_output);

            v_osc_set_unison_pitch(a_voice->osc_unison1, a_voice->unison_spread,
                    ((a_voice->target_pitch) + (a_voice->osc1_pitch_adjust) ));
            v_osc_set_unison_pitch(a_voice->osc_unison2, a_voice->unison_spread,
                    ((a_voice->base_pitch) + (a_voice->osc2_pitch_adjust)));

            a_voice->current_sample += f_osc_run_unison_osc_sync(a_voice->osc_unison2);
            
            if(a_voice->osc_unison2->is_resetting)
            {
                v_osc_note_on_sync_phases_hard(a_voice->osc_unison1);
            }
            
            a_voice->current_sample += f_osc_run_unison_osc(a_voice->osc_unison1) * (a_voice->osc1_linamp);
            
        }
        else
        {        
            a_voice->base_pitch = (a_voice->glide_env->output_multiplied) + (a_voice->pitch_env->output_multiplied) 
                    + (plugin_data->mono_modules->pitchbend_smoother->output) + (a_voice->last_pitch) + (a_voice->lfo_pitch_output);

            v_osc_set_unison_pitch(a_voice->osc_unison1, (*plugin_data->master_uni_spread) * 0.01f,
                    ((a_voice->base_pitch) + (a_voice->osc1_pitch_adjust) ));
            v_osc_set_unison_pitch(a_voice->osc_unison2, (*plugin_data->master_uni_spread) * 0.01f,
                    ((a_voice->base_pitch) + (a_voice->osc2_pitch_adjust)));

            a_voice->current_sample += f_osc_run_unison_osc(a_voice->osc_unison1) * (a_voice->osc1_linamp);
            a_voice->current_sample += f_osc_run_unison_osc(a_voice->osc_unison2) * (a_voice->osc2_linamp);            
        }
        
        a_voice->current_sample += (f_run_white_noise(a_voice->white_noise1) * (a_voice->noise_linamp)); //white noise
        
        v_adsr_run_db(a_voice->adsr_amp);        
        
        v_adsr_run(a_voice->adsr_filter);
        
        v_svf_set_cutoff_base(a_voice->svf_filter,  (plugin_data->mono_modules->filter_smoother->output));//vals->timbre);

        v_svf_add_cutoff_mod(a_voice->svf_filter, 
                (((a_voice->adsr_filter->output) * (*plugin_data->filter_env_amt)) + (a_voice->lfo_filter_output)));        

        v_svf_set_cutoff(a_voice->svf_filter);
        
        a_voice->filter_output = a_voice->svf_function(a_voice->svf_filter, (a_voice->current_sample));

        a_voice->current_sample = f_axf_run_xfade((a_voice->dist_dry_wet), (a_voice->filter_output), 
                f_clp_clip(a_voice->clipper1, (a_voice->filter_output)));
        
        a_voice->current_sample = (a_voice->current_sample) * (a_voice->adsr_amp->output) * (a_voice->amp) * (a_voice->lfo_amp_output);
        
        /*Run the envelope and assign to the output buffers*/
        out0[(a_voice->i_voice)] += (a_voice->current_sample);
        out1[(a_voice->i_voice)] += (a_voice->current_sample);                
    }
}

const PYFX_Descriptor *rayv_PYFX_descriptor(int index)
{
    PYFX_Descriptor *LMSLDescriptor = NULL;
    char **port_names;
    PYFX_PortDescriptor *port_descriptors;
    PYFX_PortRangeHint *port_range_hints;
    int * automatable;
    int * value_tranform_hints;

    LMSLDescriptor =
	(PYFX_Descriptor *) malloc(sizeof(PYFX_Descriptor));
    if (LMSLDescriptor) {
        LMSLDescriptor->UniqueID = RAYV_PLUGIN_UUID;  //Arbitrary number I made up, somewhat near the upper end of allowable UIDs
	LMSLDescriptor->Label = RAYV_PLUGIN_NAME;	
	LMSLDescriptor->Name = RAYV_PLUGIN_LONG_NAME;
	LMSLDescriptor->Maker = RAYV_PLUGIN_DEV;
	LMSLDescriptor->Copyright = "GNU GPL v3";
	LMSLDescriptor->PortCount = RAYV_COUNT;

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

	port_names = (char **) calloc(LMSLDescriptor->PortCount, sizeof(char *));
	LMSLDescriptor->PortNames = (const char **) port_names;
                
        automatable = (int*)calloc(LMSLDescriptor->PortCount, sizeof(int));
        LMSLDescriptor->Automatable = automatable;
        
        value_tranform_hints = (int*)calloc(LMSLDescriptor->PortCount, sizeof(int));
        LMSLDescriptor->ValueTransformHint = value_tranform_hints;

	/* Parameters for output */
	port_descriptors[RAYV_OUTPUT0] = PYFX_PORT_OUTPUT | PYFX_PORT_AUDIO;
	port_names[RAYV_OUTPUT0] = "Output 0";
	port_range_hints[RAYV_OUTPUT0].DefaultValue = 0.0f;

        port_descriptors[RAYV_OUTPUT1] = PYFX_PORT_OUTPUT | PYFX_PORT_AUDIO;
	port_names[RAYV_OUTPUT1] = "Output 1";
	port_range_hints[RAYV_OUTPUT1].DefaultValue = 0.0f;
                
	port_descriptors[RAYV_ATTACK] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[RAYV_ATTACK] = "Attack time (s)";
	port_range_hints[RAYV_ATTACK].DefaultValue = 10.0f;
	port_range_hints[RAYV_ATTACK].LowerBound = 0.0f; 
	port_range_hints[RAYV_ATTACK].UpperBound = 100.0f;
        automatable[RAYV_ATTACK] = 1;
        value_tranform_hints[RAYV_ATTACK] = PYDAW_PLUGIN_HINT_TRANSFORM_DECIMAL;

	port_descriptors[RAYV_DECAY] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[RAYV_DECAY] = "Decay time (s)";
	port_range_hints[RAYV_DECAY].DefaultValue = 10.0f;
	port_range_hints[RAYV_DECAY].LowerBound = 10.0f; 
	port_range_hints[RAYV_DECAY].UpperBound = 100.0f;
        automatable[RAYV_DECAY] = 1;
        value_tranform_hints[RAYV_DECAY] = PYDAW_PLUGIN_HINT_TRANSFORM_DECIMAL;

	port_descriptors[RAYV_SUSTAIN] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[RAYV_SUSTAIN] = "Sustain level (%)";
	port_range_hints[RAYV_SUSTAIN].DefaultValue = 0.0f;
	port_range_hints[RAYV_SUSTAIN].LowerBound = -60.0f;
	port_range_hints[RAYV_SUSTAIN].UpperBound = 0.0f;
        automatable[RAYV_SUSTAIN] = 1;

	port_descriptors[RAYV_RELEASE] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[RAYV_RELEASE] = "Release time (s)";
	port_range_hints[RAYV_RELEASE].DefaultValue = 50.0f;
	port_range_hints[RAYV_RELEASE].LowerBound = 10.0f; 
	port_range_hints[RAYV_RELEASE].UpperBound = 200.0f;
        automatable[RAYV_RELEASE] = 1;
        value_tranform_hints[RAYV_RELEASE] = PYDAW_PLUGIN_HINT_TRANSFORM_DECIMAL;

	port_descriptors[RAYV_TIMBRE] = port_descriptors[RAYV_ATTACK];
	port_names[RAYV_TIMBRE] = "Filter Cutoff";
	port_range_hints[RAYV_TIMBRE].DefaultValue = 124.0f;
	port_range_hints[RAYV_TIMBRE].LowerBound =  20.0f;
	port_range_hints[RAYV_TIMBRE].UpperBound =  124.0f;
        automatable[RAYV_TIMBRE] = 1;
        value_tranform_hints[RAYV_TIMBRE] = PYDAW_PLUGIN_HINT_TRANSFORM_PITCH_TO_HZ;
        
	port_descriptors[RAYV_RES] = port_descriptors[RAYV_ATTACK];
	port_names[RAYV_RES] = "Res";
	port_range_hints[RAYV_RES].DefaultValue = -15.0f;
	port_range_hints[RAYV_RES].LowerBound =  -30.0f;
	port_range_hints[RAYV_RES].UpperBound =  0.0f;
        automatable[RAYV_RES] = 1;
        
	port_descriptors[RAYV_DIST] = port_descriptors[RAYV_ATTACK];
	port_names[RAYV_DIST] = "Dist";
	port_range_hints[RAYV_DIST].DefaultValue = 15.0f;
	port_range_hints[RAYV_DIST].LowerBound =  -6.0f;
	port_range_hints[RAYV_DIST].UpperBound =  36.0f;
        automatable[RAYV_DIST] = 1;
        
	port_descriptors[RAYV_FILTER_ATTACK] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[RAYV_FILTER_ATTACK] = "Attack time (s) filter";
	port_range_hints[RAYV_FILTER_ATTACK].DefaultValue = 10.0f;
	port_range_hints[RAYV_FILTER_ATTACK].LowerBound = 0.0f;
	port_range_hints[RAYV_FILTER_ATTACK].UpperBound = 100.0f;
        automatable[RAYV_FILTER_ATTACK] = 1;
        value_tranform_hints[RAYV_FILTER_ATTACK] = PYDAW_PLUGIN_HINT_TRANSFORM_DECIMAL;

	port_descriptors[RAYV_FILTER_DECAY] = port_descriptors[RAYV_ATTACK];
	port_names[RAYV_FILTER_DECAY] = "Decay time (s) filter";
	port_range_hints[RAYV_FILTER_DECAY].DefaultValue = 50.0f;
	port_range_hints[RAYV_FILTER_DECAY].LowerBound = 10.0f;
	port_range_hints[RAYV_FILTER_DECAY].UpperBound = 100.0f;
        automatable[RAYV_FILTER_DECAY] = 1;
        value_tranform_hints[RAYV_FILTER_DECAY] = PYDAW_PLUGIN_HINT_TRANSFORM_DECIMAL;

	port_descriptors[RAYV_FILTER_SUSTAIN] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[RAYV_FILTER_SUSTAIN] = "Sustain level (%) filter";
	port_range_hints[RAYV_FILTER_SUSTAIN].DefaultValue = 100.0f;
	port_range_hints[RAYV_FILTER_SUSTAIN].LowerBound = 0.0f; 
	port_range_hints[RAYV_FILTER_SUSTAIN].UpperBound = 100.0f; 
        automatable[RAYV_FILTER_SUSTAIN] = 1;
        value_tranform_hints[RAYV_FILTER_SUSTAIN] = PYDAW_PLUGIN_HINT_TRANSFORM_DECIMAL;
        
	port_descriptors[RAYV_FILTER_RELEASE] = port_descriptors[RAYV_ATTACK];
	port_names[RAYV_FILTER_RELEASE] = "Release time (s) filter";
	port_range_hints[RAYV_FILTER_RELEASE].DefaultValue = 50.0f;
	port_range_hints[RAYV_FILTER_RELEASE].LowerBound = 10.0f; 
	port_range_hints[RAYV_FILTER_RELEASE].UpperBound = 200.0f; 
        automatable[RAYV_FILTER_RELEASE] = 1;
        value_tranform_hints[RAYV_FILTER_RELEASE] = PYDAW_PLUGIN_HINT_TRANSFORM_DECIMAL;
        
	port_descriptors[RAYV_NOISE_AMP] = port_descriptors[RAYV_ATTACK];
	port_names[RAYV_NOISE_AMP] = "Noise Amp";
	port_range_hints[RAYV_NOISE_AMP].DefaultValue = -30.0f;
	port_range_hints[RAYV_NOISE_AMP].LowerBound =  -60.0f;
	port_range_hints[RAYV_NOISE_AMP].UpperBound =  0.0f;
        automatable[RAYV_NOISE_AMP] = 1;
                
	port_descriptors[RAYV_FILTER_ENV_AMT] = port_descriptors[RAYV_ATTACK];
	port_names[RAYV_FILTER_ENV_AMT] = "Filter Env Amt";
	port_range_hints[RAYV_FILTER_ENV_AMT].DefaultValue = 0.0f;
	port_range_hints[RAYV_FILTER_ENV_AMT].LowerBound =  -36.0f;
	port_range_hints[RAYV_FILTER_ENV_AMT].UpperBound =  36.0f;
        automatable[RAYV_FILTER_ENV_AMT] = 1;
        
	port_descriptors[RAYV_DIST_WET] = port_descriptors[RAYV_ATTACK];
	port_names[RAYV_DIST_WET] = "Dist Wet";
	port_range_hints[RAYV_DIST_WET].DefaultValue = 0.0f;
	port_range_hints[RAYV_DIST_WET].LowerBound =  0.0f; 
	port_range_hints[RAYV_DIST_WET].UpperBound =  100.0f;
        automatable[RAYV_DIST_WET] = 1;
        value_tranform_hints[RAYV_DIST_WET] = PYDAW_PLUGIN_HINT_TRANSFORM_DECIMAL;        
        
	port_descriptors[RAYV_OSC1_TYPE] = port_descriptors[RAYV_ATTACK];
	port_names[RAYV_OSC1_TYPE] = "Osc 1 Type";
	port_range_hints[RAYV_OSC1_TYPE].DefaultValue = 0.0f;
	port_range_hints[RAYV_OSC1_TYPE].LowerBound =  0.0f;
	port_range_hints[RAYV_OSC1_TYPE].UpperBound =  5.0f;        
        
	port_descriptors[RAYV_OSC1_PITCH] = port_descriptors[RAYV_ATTACK];
	port_names[RAYV_OSC1_PITCH] = "Osc 1 Pitch";
	port_range_hints[RAYV_OSC1_PITCH].DefaultValue = 0.0f;
	port_range_hints[RAYV_OSC1_PITCH].LowerBound =  -36.0f;
	port_range_hints[RAYV_OSC1_PITCH].UpperBound =  36.0f;
        
	port_descriptors[RAYV_OSC1_TUNE] = port_descriptors[RAYV_ATTACK];
	port_names[RAYV_OSC1_TUNE] = "Osc 1 Tune";
	port_range_hints[RAYV_OSC1_TUNE].DefaultValue = 0.0f;
	port_range_hints[RAYV_OSC1_TUNE].LowerBound = -100.0f;
	port_range_hints[RAYV_OSC1_TUNE].UpperBound =  100.0f;
        
	port_descriptors[RAYV_OSC1_VOLUME] = port_descriptors[RAYV_ATTACK];
	port_names[RAYV_OSC1_VOLUME] = "Osc 1 Vol";
	port_range_hints[RAYV_OSC1_VOLUME].DefaultValue = 0.0f;
	port_range_hints[RAYV_OSC1_VOLUME].LowerBound =  -60.0f;
	port_range_hints[RAYV_OSC1_VOLUME].UpperBound =  0.0f;
                
	port_descriptors[RAYV_OSC2_TYPE] = port_descriptors[RAYV_ATTACK];
	port_names[RAYV_OSC2_TYPE] = "Osc 2 Type";
	port_range_hints[RAYV_OSC2_TYPE].DefaultValue = 4.0f;
	port_range_hints[RAYV_OSC2_TYPE].LowerBound =  0.0f;
	port_range_hints[RAYV_OSC2_TYPE].UpperBound =  4.0f;
        
	port_descriptors[RAYV_OSC2_PITCH] = port_descriptors[RAYV_ATTACK];
	port_names[RAYV_OSC2_PITCH] = "Osc 2 Pitch";
	port_range_hints[RAYV_OSC2_PITCH].DefaultValue = 0.0f;
	port_range_hints[RAYV_OSC2_PITCH].LowerBound =  -36.0f;
	port_range_hints[RAYV_OSC2_PITCH].UpperBound =  36.0f;
        
	port_descriptors[RAYV_OSC2_TUNE] = port_descriptors[RAYV_ATTACK];
	port_names[RAYV_OSC2_TUNE] = "Osc 2 Tune";
	port_range_hints[RAYV_OSC2_TUNE].DefaultValue = 0.0f;
	port_range_hints[RAYV_OSC2_TUNE].LowerBound = -100.0f;
	port_range_hints[RAYV_OSC2_TUNE].UpperBound = 100.0f; 
        
	port_descriptors[RAYV_OSC2_VOLUME] = port_descriptors[RAYV_ATTACK];
	port_names[RAYV_OSC2_VOLUME] = "Osc 2 Vol";
	port_range_hints[RAYV_OSC2_VOLUME].DefaultValue = 0.0f;
	port_range_hints[RAYV_OSC2_VOLUME].LowerBound =  -60.0f;
	port_range_hints[RAYV_OSC2_VOLUME].UpperBound =  0.0f;
        
	port_descriptors[RAYV_MASTER_VOLUME] = port_descriptors[RAYV_ATTACK];
	port_names[RAYV_MASTER_VOLUME] = "Master Vol";
	port_range_hints[RAYV_MASTER_VOLUME].DefaultValue = -6.0f;
	port_range_hints[RAYV_MASTER_VOLUME].LowerBound =  -60.0f;
	port_range_hints[RAYV_MASTER_VOLUME].UpperBound =  12.0f;
        
	port_descriptors[RAYV_MASTER_UNISON_VOICES] = port_descriptors[RAYV_ATTACK];
	port_names[RAYV_MASTER_UNISON_VOICES] = "Master Unison";
	port_range_hints[RAYV_MASTER_UNISON_VOICES].DefaultValue = 4.0f;
	port_range_hints[RAYV_MASTER_UNISON_VOICES].LowerBound =  1.0f;
	port_range_hints[RAYV_MASTER_UNISON_VOICES].UpperBound =  7.0f;
        
	port_descriptors[RAYV_MASTER_UNISON_SPREAD] = port_descriptors[RAYV_ATTACK];
	port_names[RAYV_MASTER_UNISON_SPREAD] = "Master Unison Spread";
	port_range_hints[RAYV_MASTER_UNISON_SPREAD].DefaultValue = 50.0f;
	port_range_hints[RAYV_MASTER_UNISON_SPREAD].LowerBound =  0.0f;
	port_range_hints[RAYV_MASTER_UNISON_SPREAD].UpperBound =  100.0f;
        
	port_descriptors[RAYV_MASTER_GLIDE] = port_descriptors[RAYV_ATTACK];
	port_names[RAYV_MASTER_GLIDE] = "Master Glide";
	port_range_hints[RAYV_MASTER_GLIDE].DefaultValue = 0.0f;
	port_range_hints[RAYV_MASTER_GLIDE].LowerBound =  0.0f;
	port_range_hints[RAYV_MASTER_GLIDE].UpperBound =  200.0f;
        automatable[RAYV_MASTER_GLIDE] = 1;
        value_tranform_hints[RAYV_MASTER_GLIDE] = PYDAW_PLUGIN_HINT_TRANSFORM_DECIMAL;
        
	port_descriptors[RAYV_MASTER_PITCHBEND_AMT] = port_descriptors[RAYV_ATTACK];
	port_names[RAYV_MASTER_PITCHBEND_AMT] = "Pitchbend Amt";
	port_range_hints[RAYV_MASTER_PITCHBEND_AMT].DefaultValue = 18.0f;
	port_range_hints[RAYV_MASTER_PITCHBEND_AMT].LowerBound =  1.0f;
	port_range_hints[RAYV_MASTER_PITCHBEND_AMT].UpperBound =  36.0f;
        
	port_descriptors[RAYV_PITCH_ENV_AMT] = port_descriptors[RAYV_ATTACK];
	port_names[RAYV_PITCH_ENV_AMT] = "Pitch Env Amt";
	port_range_hints[RAYV_PITCH_ENV_AMT].DefaultValue = 0.0f;
	port_range_hints[RAYV_PITCH_ENV_AMT].LowerBound =  -36.0f;
	port_range_hints[RAYV_PITCH_ENV_AMT].UpperBound =   36.0f;
        automatable[RAYV_PITCH_ENV_AMT] = 1;
        
	port_descriptors[RAYV_PITCH_ENV_TIME] = port_descriptors[RAYV_ATTACK];
	port_names[RAYV_PITCH_ENV_TIME] = "Pitch Env Time";
	port_range_hints[RAYV_PITCH_ENV_TIME].DefaultValue = 100.0f;
	port_range_hints[RAYV_PITCH_ENV_TIME].LowerBound = 0.0f; 
	port_range_hints[RAYV_PITCH_ENV_TIME].UpperBound = 200.0f;
        automatable[RAYV_PITCH_ENV_TIME] = 1;
        value_tranform_hints[RAYV_PITCH_ENV_TIME] = PYDAW_PLUGIN_HINT_TRANSFORM_DECIMAL;
        
	port_descriptors[RAYV_LFO_FREQ] = port_descriptors[RAYV_ATTACK];
	port_names[RAYV_LFO_FREQ] = "LFO Freq";
	port_range_hints[RAYV_LFO_FREQ].DefaultValue = 200.0f;
	port_range_hints[RAYV_LFO_FREQ].LowerBound = 10.0f;
	port_range_hints[RAYV_LFO_FREQ].UpperBound = 1600.0f;
        automatable[RAYV_LFO_FREQ] = 1;
        value_tranform_hints[RAYV_LFO_FREQ] = PYDAW_PLUGIN_HINT_TRANSFORM_DECIMAL;
        
	port_descriptors[RAYV_LFO_TYPE] = port_descriptors[RAYV_ATTACK];
	port_names[RAYV_LFO_TYPE] = "LFO Type";
	port_range_hints[RAYV_LFO_TYPE].DefaultValue = 0.0f;
	port_range_hints[RAYV_LFO_TYPE].LowerBound = 0.0f; 
	port_range_hints[RAYV_LFO_TYPE].UpperBound = 2.0f;
        
	port_descriptors[RAYV_LFO_AMP] = port_descriptors[RAYV_ATTACK];
	port_names[RAYV_LFO_AMP] = "LFO Amp";
	port_range_hints[RAYV_LFO_AMP].DefaultValue = 0.0f;
	port_range_hints[RAYV_LFO_AMP].LowerBound = -24.0f;
	port_range_hints[RAYV_LFO_AMP].UpperBound = 24.0f;
        automatable[RAYV_LFO_AMP] = 1;        
        
	port_descriptors[RAYV_LFO_PITCH] = port_descriptors[RAYV_ATTACK];
	port_names[RAYV_LFO_PITCH] = "LFO Pitch";
	port_range_hints[RAYV_LFO_PITCH].DefaultValue = 0.0f;
	port_range_hints[RAYV_LFO_PITCH].LowerBound = -36.0f;
	port_range_hints[RAYV_LFO_PITCH].UpperBound = 36.0f;
        automatable[RAYV_LFO_PITCH] = 1;
        
	port_descriptors[RAYV_LFO_FILTER] = port_descriptors[RAYV_ATTACK];
	port_names[RAYV_LFO_FILTER] = "LFO Filter";
	port_range_hints[RAYV_LFO_FILTER].DefaultValue = 0.0f;
	port_range_hints[RAYV_LFO_FILTER].LowerBound = -48.0f;
	port_range_hints[RAYV_LFO_FILTER].UpperBound = 48.0f;
        automatable[RAYV_LFO_FILTER] = 1;
        
        port_descriptors[RAYV_OSC_HARD_SYNC] = port_descriptors[RAYV_ATTACK];
	port_names[RAYV_OSC_HARD_SYNC] = "Osc Hard Sync";
	port_range_hints[RAYV_OSC_HARD_SYNC].DefaultValue = 0.0f;
	port_range_hints[RAYV_OSC_HARD_SYNC].LowerBound = 0;
	port_range_hints[RAYV_OSC_HARD_SYNC].UpperBound = 1;
                
	LMSLDescriptor->activate = v_rayv_activate;
	LMSLDescriptor->cleanup = v_cleanup_rayv;
	LMSLDescriptor->connect_port = v_rayv_connect_port;
	LMSLDescriptor->deactivate = NULL;
	LMSLDescriptor->instantiate = g_rayv_instantiate;
	LMSLDescriptor->run = NULL;
        LMSLDescriptor->panic = rayvPanic;
    }

    return LMSLDescriptor;
    
}


const PYINST_Descriptor *rayv_PYINST_descriptor(int index)
{
    PYINST_Descriptor *LMSDDescriptor = NULL;
    
    
    LMSDDescriptor = (PYINST_Descriptor *) malloc(sizeof(PYINST_Descriptor));
    if (LMSDDescriptor) {
	LMSDDescriptor->PYINST_API_Version = 1;
	LMSDDescriptor->PYFX_Plugin = rayv_PYFX_descriptor(0);
	LMSDDescriptor->configure = NULL;
	LMSDDescriptor->run_synth = v_run_rayv;
    }
    
    return LMSDDescriptor;    
}


/*
void v_rayv_destructor()
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
