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

static void run_lms_ray_v(LADSPA_Handle instance, unsigned long sample_count,
		  snd_seq_event_t * events, unsigned long EventCount);

static void run_voice(LMS *p, synth_vals *vals, t_poly_voice *d,
		      LADSPA_Data *out0, LADSPA_Data *out1, unsigned int count);

int pick_voice(const t_poly_voice *data, int);


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

static void cleanupLMS(LADSPA_Handle instance)
{
    free(instance);
}

static void connectPortLMS(LADSPA_Handle instance, unsigned long port,
			  LADSPA_Data * data)
{
    LMS *plugin;

    plugin = (LMS *) instance;
    
    /*Add the ports from step 9 to the connectPortLMS event handler*/
    
    switch (port) {
    case LMS_OUTPUT0:
	plugin->output0 = data;
	break;
    case LMS_OUTPUT1:
	plugin->output1 = data;
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
    case LMS_TIMBRE:
	plugin->timbre = data;              
	break;
    case LMS_RES:
	plugin->res = data;              
	break;
    case LMS_DIST:
	plugin->dist = data;              
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
    case LMS_DIST_WET:
        plugin->dist_wet = data;
        break;
    case LMS_FILTER_ENV_AMT:
        plugin->filter_env_amt = data;
        break;
    case LMS_MASTER_VOLUME:
        plugin->master_vol = data;
        break;
    case LMS_OSC1_PITCH:
        plugin->osc1pitch = data;
        break;
    case LMS_OSC1_TUNE:
        plugin->osc1tune = data;
        break;
    case LMS_OSC1_TYPE:
        plugin->osc1type = data;
        break;
    case LMS_OSC1_VOLUME:
        plugin->osc1vol = data;
        break;
    case LMS_OSC2_PITCH:
        plugin->osc2pitch = data;
        break;
    case LMS_OSC2_TUNE:
        plugin->osc2tune = data;
        break;
    case LMS_OSC2_TYPE:
        plugin->osc2type = data;
        break;
    case LMS_OSC2_VOLUME:
        plugin->osc2vol = data;
        break;
    case LMS_MASTER_UNISON_VOICES:
        plugin->master_uni_voice = data;
        break;
    case LMS_MASTER_UNISON_SPREAD:
        plugin->master_uni_spread = data;        
        break;
    case LMS_MASTER_GLIDE:
        plugin->master_glide = data;
        break;
    case LMS_MASTER_PITCHBEND_AMT:
        plugin->master_pb_amt = data;
        break;
    case LMS_PITCH_ENV_AMT:
        plugin->pitch_env_amt = data;
        break;
    case LMS_PITCH_ENV_TIME:
        plugin->pitch_env_time = data;
        break;
    case LMS_PROGRAM_CHANGE:
        plugin->program = data;
        break;
    case LMS_LFO_FREQ:
        plugin->lfo_freq = data;
        break;
    case LMS_LFO_TYPE:
        plugin->lfo_type = data;
        break;
    case LMS_LFO_AMP:
        plugin->lfo_amp = data;
        break;
    case LMS_LFO_PITCH:
        plugin->lfo_pitch = data;
        break;
    case LMS_LFO_FILTER:
        plugin->lfo_filter = data;
        break;
    }
}

static LADSPA_Handle instantiateLMS(const LADSPA_Descriptor * descriptor,
				   unsigned long s_rate)
{
    LMS *plugin_data = (LMS *) malloc(sizeof(LMS));
    
    plugin_data->fs = s_rate;
    
    plugin_data->midi_cc_map = g_ccm_get();
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_ATTACK, 73, "Attack Amp");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_DECAY, 75, "Decay Amp");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_SUSTAIN, 79, "Sustain Amp");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_RELEASE, 72, "Release Amp");    
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_TIMBRE, 20, "Filter Cutoff");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_DIST, 51, "Distortion Gain");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_RES, 50, "Res");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_FILTER_ATTACK, 21, "Attack Filter");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_FILTER_DECAY, 22, "Decay Filter");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_FILTER_SUSTAIN, 23, "Sustain Filter");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_FILTER_RELEASE, 24, "Release Filter");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_NOISE_AMP, 25, "Noise Amp");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_FILTER_ENV_AMT, 26, "Filter Env Amt");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_DIST_WET, 27, "Distortion Wet");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_OSC1_TYPE, 28, "Osc1 Type");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_OSC1_PITCH, 29, "Osc1 Pitch");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_OSC1_TUNE, 30, "Osc1 Tune");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_OSC1_VOLUME, 31, "Osc1 Volume");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_OSC2_TYPE, 41, "Osc2 Type");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_OSC2_PITCH, 33, "Osc2 Pitch");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_OSC2_TUNE, 34, "Osc2 Tune");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_OSC2_VOLUME, 35, "Osc2 Volume");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_MASTER_VOLUME, 36, "Master Volume");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_MASTER_UNISON_VOICES, 37, "Unison Voices");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_MASTER_UNISON_SPREAD, 38, "Unison Spread");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_MASTER_GLIDE, 39, "Glide Time");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_MASTER_PITCHBEND_AMT, 40, "Pitchbend Amount");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_PITCH_ENV_AMT, 42, "Pitch Env Amt");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_PITCH_ENV_TIME, 43, "Pitch Env Time");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_LFO_FREQ, 44, "LFO Freq");    
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_LFO_TYPE, 45, "LFO Type");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_LFO_AMP, 46, "LFO Amp");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_LFO_PITCH, 47, "LFO Pitch");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_LFO_FILTER, 48, "LFO Filter");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_PROGRAM_CHANGE, 49, "Program Change");
    
    v_ccm_read_file_to_array(plugin_data->midi_cc_map, "ray_v-cc_map.txt");
    
    /*LibModSynth additions*/
    v_init_lms(s_rate);  //initialize any static variables    
    /*End LibModSynth additions*/
    
    return (LADSPA_Handle) plugin_data;
}

static void activateLMS(LADSPA_Handle instance)
{
    LMS *plugin_data = (LMS *) instance;
    unsigned int i;
    
    for (i=0; i<POLYPHONY; i++) {
        plugin_data->data[i] = g_poly_init();
        plugin_data->data[i]->note_f = i;        
    }
    plugin_data->sampleNo = 0;
    
    for (i = 0; i < VOICES_MAX_MIDI_NOTE_NUMBER; i++) {
	plugin_data->ons[i] = -1;
	plugin_data->offs[i] = -1;
    }
    
    plugin_data->pitch = 1.0f;
    plugin_data->sv_pitch_bend_value = 0.0f;
    plugin_data->sv_last_note = 60.0f;  //For glide
    
    plugin_data->mono_modules = v_mono_init();  //initialize all monophonic modules
}

static void runLMSWrapper(LADSPA_Handle instance,
			 unsigned long sample_count)
{
    run_lms_ray_v(instance, sample_count, NULL, 0);
}

static void run_lms_ray_v(LADSPA_Handle instance, unsigned long sample_count,
		  snd_seq_event_t *events, unsigned long event_count)
{
    LMS *plugin_data = (LMS *) instance;
    
    LADSPA_Data *const output0 = plugin_data->output0;    
    LADSPA_Data *const output1 = plugin_data->output1;
    //t_poly_voice *data = plugin_data->data;
    
    plugin_data->count= 0;    
    plugin_data->voice = 0;
    
    /*Set the values from synth_vals in RunLMS*/
    plugin_data->vals.attack = *(plugin_data->attack) * .01;
    plugin_data->vals.decay = *(plugin_data->decay) * .01; 
    plugin_data->vals.sustain = *(plugin_data->sustain);
    plugin_data->vals.release = *(plugin_data->release) * .01;
    plugin_data->vals.timbre = *(plugin_data->timbre);
    
    plugin_data->vals.res = *(plugin_data->res);
    plugin_data->vals.dist = *(plugin_data->dist);

    plugin_data->vals.attack_f = *(plugin_data->attack_f) * .01;
    plugin_data->vals.decay_f = *(plugin_data->decay_f) * .01; 
    plugin_data->vals.sustain_f = *(plugin_data->sustain_f) * .01;
    plugin_data->vals.release_f = *(plugin_data->release_f) * .01;
    
    plugin_data->vals.noise_amp = *(plugin_data->noise_amp);
    
    plugin_data->vals.dist_wet = *(plugin_data->dist_wet) * .01;    
    plugin_data->vals.filter_env_amt = *(plugin_data->filter_env_amt);
    plugin_data->vals.master_vol = *(plugin_data->master_vol);
    
    plugin_data->vals.osc1pitch = *(plugin_data->osc1pitch);
    plugin_data->vals.osc1tune = *(plugin_data->osc1tune) * .01;
    plugin_data->vals.osc1type = *(plugin_data->osc1type);
    plugin_data->vals.osc1vol = *(plugin_data->osc1vol);
    
    plugin_data->vals.osc2pitch = *(plugin_data->osc2pitch);
    plugin_data->vals.osc2tune = *(plugin_data->osc2tune) * .01;
    plugin_data->vals.osc2type = *(plugin_data->osc2type);
    plugin_data->vals.osc2vol = *(plugin_data->osc2vol);
    
    plugin_data->vals.master_uni_voice = *(plugin_data->master_uni_voice);
    plugin_data->vals.master_uni_spread = *(plugin_data->master_uni_spread) * .01;
    plugin_data->vals.master_glide = *(plugin_data->master_glide) * .01;
    plugin_data->vals.master_pb_amt = *(plugin_data->master_pb_amt);
    
    plugin_data->vals.pitch_env_amt = *(plugin_data->pitch_env_amt);
    plugin_data->vals.pitch_env_time = *(plugin_data->pitch_env_time) * .01;
    
    plugin_data->vals.lfo_freq = *(plugin_data->lfo_freq) * .01;
    plugin_data->vals.lfo_type = *(plugin_data->lfo_type);
    plugin_data->vals.lfo_amp = *(plugin_data->lfo_amp);
    plugin_data->vals.lfo_pitch = *(plugin_data->lfo_pitch);
    plugin_data->vals.lfo_filter = *(plugin_data->lfo_filter);
    
    /*Events is an array of snd_seq_event_t objects, 
     event_count is the number of events,
     and sample_count is the block size          
     */
    for (plugin_data->pos = 0; (plugin_data->pos) < sample_count; plugin_data->pos = (plugin_data->pos) + STEP_SIZE) 
    {	        
        v_smr_iir_run(plugin_data->mono_modules->filter_smoother, (plugin_data->vals.timbre));
        v_smr_iir_run(plugin_data->mono_modules->pitchbend_smoother, (plugin_data->sv_pitch_bend_value));
        
	for(plugin_data->event_pos = 0; (plugin_data->event_pos) < event_count; plugin_data->event_pos = (plugin_data->event_pos) + 1)
        {
            /*Note on event*/
	    if (events[(plugin_data->event_pos)].type == SND_SEQ_EVENT_NOTEON) 
            {
		snd_seq_ev_note_t n = events[(plugin_data->event_pos)].data.note;

                if(n.note >= POLYPHONY)
                {
                    continue;
                }
                
		if (n.velocity > 0) 
                {
                    if((plugin_data->ons[n.note]) != -1)
                    {
                        continue;
                    }
                    
                    plugin_data->ons[n.note] =
			plugin_data->sampleNo + events[(plugin_data->event_pos)].time.tick;
		    plugin_data->offs[n.note] = -1;
                    
		    plugin_data->data[n.note]->amp = f_db_to_linear_fast(((n.velocity * 0.094488) - 12 + (plugin_data->vals.master_vol)), //-20db to 0db, + master volume (0 to -60)
                            plugin_data->mono_modules->amp_ptr); 
                    v_svf_velocity_mod(plugin_data->data[n.note]->svf_filter, n.velocity);
      
                    plugin_data->data[n.note]->note_f = (float)n.note;
                    plugin_data->data[n.note]->note = n.note;
                    
                    plugin_data->data[n.note]->target_pitch = (plugin_data->data[n.note]->note_f);
                    plugin_data->data[n.note]->last_pitch = (plugin_data->sv_last_note);
                    
                    v_rmp_retrigger_glide_t(plugin_data->data[n.note]->glide_env , (plugin_data->vals.master_glide), 
                            (plugin_data->sv_last_note), (plugin_data->data[n.note]->target_pitch));
                                        
                    plugin_data->data[n.note]->osc1_linamp = f_db_to_linear_fast((plugin_data->vals.osc1vol), plugin_data->mono_modules->amp_ptr); 
                    plugin_data->data[n.note]->osc2_linamp = f_db_to_linear_fast((plugin_data->vals.osc2vol), plugin_data->mono_modules->amp_ptr);
                    plugin_data->data[n.note]->noise_linamp = f_db_to_linear_fast((plugin_data->vals.noise_amp), plugin_data->mono_modules->amp_ptr);
                    
                    v_adsr_retrigger(plugin_data->data[n.note]->adsr_amp);
                    v_adsr_retrigger(plugin_data->data[n.note]->adsr_filter);
                    v_lfs_sync(plugin_data->data[n.note]->lfo1, 0.0f, (plugin_data->vals.lfo_type));
                    
                    v_adsr_set_adsr_db(plugin_data->data[n.note]->adsr_amp, (plugin_data->vals.attack), (plugin_data->vals.decay), (plugin_data->vals.sustain), (plugin_data->vals.release));
                    v_adsr_set_adsr(plugin_data->data[n.note]->adsr_filter, (plugin_data->vals.attack_f), (plugin_data->vals.decay_f), (plugin_data->vals.sustain_f), (plugin_data->vals.release_f));
                                        
                    v_rmp_retrigger((plugin_data->data[n.note]->pitch_env), (plugin_data->vals.pitch_env_time), (plugin_data->vals.pitch_env_amt));  
                    
                    v_clp_set_in_gain(plugin_data->data[n.note]->clipper1, plugin_data->vals.dist);
    
                    v_svf_set_res(plugin_data->data[n.note]->svf_filter, plugin_data->vals.res);  
                    
                    plugin_data->data[n.note]->noise_amp = f_db_to_linear((plugin_data->vals.noise_amp), plugin_data->mono_modules->amp_ptr);
                    
                    v_axf_set_xfade(plugin_data->data[n.note]->dist_dry_wet, plugin_data->vals.dist_wet);       
                    
                    v_osc_set_simple_osc_unison_type(plugin_data->data[n.note]->osc_unison1, (int)(plugin_data->vals.osc1type));
                    v_osc_set_simple_osc_unison_type(plugin_data->data[n.note]->osc_unison2, (int)(plugin_data->vals.osc2type));   
                    
                    v_osc_set_uni_voice_count(plugin_data->data[n.note]->osc_unison1, plugin_data->vals.master_uni_voice);
                    v_osc_set_uni_voice_count(plugin_data->data[n.note]->osc_unison2, plugin_data->vals.master_uni_voice);                    
                                        
                    /*Set the last_note property, so the next note can glide from it if glide is turned on*/
                    plugin_data->sv_last_note = (plugin_data->data[n.note]->note_f);
		} 
                /*0 velocity, the same as note-off*/
                else 
                {
                    snd_seq_ev_note_t n = events[(plugin_data->event_pos)].data.note;
                    
                    plugin_data->offs[n.note] = 
                        (plugin_data->sampleNo) + (events[(plugin_data->event_pos)].time.tick);		    
		}
	    } 
            /*Note-off event*/
            else if (events[(plugin_data->event_pos)].type == SND_SEQ_EVENT_NOTEOFF) 
            {
		snd_seq_ev_note_t n = events[(plugin_data->event_pos)].data.note;
                
                plugin_data->offs[n.note] = 
                        (plugin_data->sampleNo) + (events[(plugin_data->event_pos)].time.tick);
                //v_voc_note_off(plugin_data->voices, n.note);
	    } 
            /*Pitch-bend sequencer event, modify the voices pitch*/
            else if (events[(plugin_data->event_pos)].type == SND_SEQ_EVENT_PITCHBEND) 
            {
		plugin_data->sv_pitch_bend_value = 0.00012207f
                        * (events[(plugin_data->event_pos)].data.control.value) * (plugin_data->vals.master_pb_amt);
	    }
            else if(events[(plugin_data->event_pos)].type == SND_SEQ_EVENT_STOP)
            {
                plugin_data->sv_pitch_bend_value = 0.0f;
                
                int f_note_off;
                
                for(f_note_off = 0; f_note_off < POLYPHONY; f_note_off++)
                {
                    if((plugin_data->ons[f_note_off]) > -1)
                    {
                        plugin_data->offs[f_note_off] = (plugin_data->sampleNo) + (events[(plugin_data->event_pos)].time.tick);
                    }
                }
            }    
	}
                
	plugin_data->count = (sample_count - (plugin_data->pos)) > STEP_SIZE ? STEP_SIZE : sample_count - (plugin_data->pos);
	
        /*Clear the output buffer*/
        plugin_data->i_iterator = 0;
        
        while((plugin_data->i_iterator)<(plugin_data->count))
        {
	    output0[((plugin_data->pos) + (plugin_data->i_iterator))] = 0.0f;                        
            output1[((plugin_data->pos) + (plugin_data->i_iterator))] = 0.0f;     
            plugin_data->i_iterator = (plugin_data->i_iterator) + 1;
	}
        
        plugin_data->voice = 0; 
	while ((plugin_data->voice) < POLYPHONY) 
        {
	    //if (data[voice].state != inactive) 
            if((plugin_data->data[(plugin_data->voice)]->adsr_amp->stage) != 4)
            {
		run_voice(plugin_data,
                        &(plugin_data->vals),
                        plugin_data->data[(plugin_data->voice)],
                        output0 + (plugin_data->pos),
                        output1 + (plugin_data->pos),
			  (plugin_data->count)
                        );
	    }
            
            plugin_data->voice = (plugin_data->voice) + 1; 
	}
        
    /*TODO:  create a loop here that corresponds to mono effects not processed per-voice*/
        
    }
    
    plugin_data->sampleNo += sample_count;
}

static void run_voice(LMS *plugin_data, synth_vals *vals, t_poly_voice *a_voice, LADSPA_Data *out0, LADSPA_Data *out1, unsigned int count)
{   
    /*Process an audio block*/
    for(a_voice->i_voice = 0; (a_voice->i_voice)<count;a_voice->i_voice = (a_voice->i_voice) + 1) 
    {        
        //Delay the note-on event until the sample it was called for
        if(((plugin_data->sampleNo) + (a_voice->i_voice) + (plugin_data->pos)) < (plugin_data->ons[a_voice->note]))
        {
            continue;
        }
        
        if (((plugin_data->offs[(a_voice->note)]) >= 0) && (((a_voice->i_voice) + (plugin_data->sampleNo) + (plugin_data->pos)) >= plugin_data->offs[(a_voice->note)])
                && ((a_voice->adsr_amp->stage) < 3))
        {
            //This is here to prevent stuck notes because it was discovered that Muse2
            //(and by extension LMS DAW) will sometimes send back note_off on a different note
            //than the note_on was sent, and this code will be invoked constantly with no effect
            //while the note hangs.  This may cause all of the notes to release at the wrong time, 
            //but it at least ensures the release does happen on all notes.
            //TODO: Fix it in LMS DAW
            if((plugin_data->ons[(a_voice->note)]) == -1)
            {
                int f_stuck_note = 0;
                
                for(f_stuck_note = 0; f_stuck_note < POLYPHONY; f_stuck_note++)
                {
                    if((plugin_data->data[f_stuck_note]->adsr_amp->stage) < 3)
                    {
                        plugin_data->ons[f_stuck_note] = -1;
                        v_poly_note_off(plugin_data->data[f_stuck_note]);
                        break;  //only unstick one note...
                    }
                }
            }
            else
            {
                plugin_data->ons[(a_voice->note)] = -1;
                v_poly_note_off(a_voice);
            }
	}        

        a_voice->current_sample = 0;
        
        f_rmp_run_ramp(a_voice->pitch_env);
        f_rmp_run_ramp(a_voice->glide_env);

        v_lfs_set(a_voice->lfo1, vals->lfo_freq);
        v_lfs_run(a_voice->lfo1);
        a_voice->lfo_amp_output = f_db_to_linear_fast((((vals->lfo_amp) * (a_voice->lfo1->output)) - (f_lms_abs((vals->lfo_amp)) * 0.5)), a_voice->amp_ptr);
        a_voice->lfo_filter_output = (vals->lfo_filter) * (a_voice->lfo1->output);
        a_voice->lfo_pitch_output = (vals->lfo_pitch) * (a_voice->lfo1->output);

        a_voice->base_pitch = (a_voice->glide_env->output_multiplied) + (a_voice->pitch_env->output_multiplied) 
                + (plugin_data->mono_modules->pitchbend_smoother->output) + (a_voice->last_pitch);
       
        v_osc_set_unison_pitch(a_voice->osc_unison1, vals->master_uni_spread,   
                ((a_voice->base_pitch) + (vals->osc1pitch) + (vals->osc1tune) + (a_voice->lfo_pitch_output)));
        
        v_osc_set_unison_pitch(a_voice->osc_unison2, vals->master_uni_spread, 
                ((a_voice->base_pitch) + (vals->osc2pitch) + (vals->osc2tune) + (a_voice->lfo_pitch_output)));

        a_voice->current_sample += f_osc_run_unison_osc(a_voice->osc_unison1) * (a_voice->osc1_linamp);
        
        a_voice->current_sample += f_osc_run_unison_osc(a_voice->osc_unison2) * (a_voice->osc2_linamp);
        
        a_voice->current_sample += (f_run_white_noise(a_voice->white_noise1) * (a_voice->noise_linamp)); //white noise
        
        v_adsr_run(a_voice->adsr_amp);        
        
        v_adsr_run(a_voice->adsr_filter);
        
        v_svf_set_cutoff_base(a_voice->svf_filter,  (plugin_data->mono_modules->filter_smoother->output));//vals->timbre);

        v_svf_add_cutoff_mod(a_voice->svf_filter, 
                (((a_voice->adsr_filter->output) * (vals->filter_env_amt)) + (a_voice->lfo_filter_output)));        

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

/*This returns MIDI CCs for the different knobs*/ 
int getControllerLMS(LADSPA_Handle instance, unsigned long port)
{
    LMS *plugin_data = (LMS *) instance;
    return DSSI_CC(i_ccm_get_cc(plugin_data->midi_cc_map, port));
}

/*Here we define how all of the LADSPA and DSSI header stuff is setup,
 we also define the ports and the GUI.*/
#ifdef __GNUC__
__attribute__((constructor)) void init()
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
        LMSLDescriptor->UniqueID = LMS_PLUGIN_UUID;  //Arbitrary number I made up, somewhat near the upper end of allowable UIDs
	LMSLDescriptor->Label = LMS_PLUGIN_NAME;
	LMSLDescriptor->Properties = LADSPA_PROPERTY_REALTIME | LADSPA_PROPERTY_HARD_RT_CAPABLE; //0;
	LMSLDescriptor->Name = LMS_PLUGIN_LONG_NAME;
	LMSLDescriptor->Maker = LMS_PLUGIN_DEV;
	LMSLDescriptor->Copyright = "GNU GPL v3";
	LMSLDescriptor->PortCount = LMS_COUNT;

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
	port_descriptors[LMS_OUTPUT0] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
	port_names[LMS_OUTPUT0] = "Output 0";
	port_range_hints[LMS_OUTPUT0].HintDescriptor = 0;

        port_descriptors[LMS_OUTPUT1] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
	port_names[LMS_OUTPUT1] = "Output 1";
	port_range_hints[LMS_OUTPUT1].HintDescriptor = 0;
        
        /*Define the LADSPA ports for the plugin in the class constructor*/
        
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
			LADSPA_HINT_DEFAULT_LOW |
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
			LADSPA_HINT_DEFAULT_LOW | 
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_RELEASE].LowerBound = 1; 
	port_range_hints[LMS_RELEASE].UpperBound = 400; 

	/* Parameters for timbre */
	port_descriptors[LMS_TIMBRE] = port_descriptors[LMS_ATTACK];
	port_names[LMS_TIMBRE] = "Timbre";
	port_range_hints[LMS_TIMBRE].HintDescriptor =
			LADSPA_HINT_DEFAULT_MAXIMUM |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_TIMBRE].LowerBound =  20;
	port_range_hints[LMS_TIMBRE].UpperBound =  124;
        
        /* Parameters for res */
	port_descriptors[LMS_RES] = port_descriptors[LMS_ATTACK];
	port_names[LMS_RES] = "Res";
	port_range_hints[LMS_RES].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_RES].LowerBound =  -30;
	port_range_hints[LMS_RES].UpperBound =  0;
        
        
        /* Parameters for dist */
	port_descriptors[LMS_DIST] = port_descriptors[LMS_ATTACK];
	port_names[LMS_DIST] = "Dist";
	port_range_hints[LMS_DIST].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_DIST].LowerBound =  -6;
	port_range_hints[LMS_DIST].UpperBound =  36;
                
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
			LADSPA_HINT_DEFAULT_LOW |
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
	port_names[LMS_NOISE_AMP] = "Dist";
	port_range_hints[LMS_NOISE_AMP].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_NOISE_AMP].LowerBound =  -60;
	port_range_hints[LMS_NOISE_AMP].UpperBound =  0;
        
        
        
        /*Parameters for filter env amt*/        
	port_descriptors[LMS_FILTER_ENV_AMT] = port_descriptors[LMS_ATTACK];
	port_names[LMS_FILTER_ENV_AMT] = "Filter Env Amt";
	port_range_hints[LMS_FILTER_ENV_AMT].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_FILTER_ENV_AMT].LowerBound =  -36;
	port_range_hints[LMS_FILTER_ENV_AMT].UpperBound =  36;
        
        /*Parameters for dist wet*/        
	port_descriptors[LMS_DIST_WET] = port_descriptors[LMS_ATTACK];
	port_names[LMS_DIST_WET] = "Dist Wet";
	port_range_hints[LMS_DIST_WET].HintDescriptor =
			LADSPA_HINT_DEFAULT_MINIMUM |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_DIST_WET].LowerBound =  0; 
	port_range_hints[LMS_DIST_WET].UpperBound =  100;
        
        
        /*Parameters for osc1type*/        
	port_descriptors[LMS_OSC1_TYPE] = port_descriptors[LMS_ATTACK];
	port_names[LMS_OSC1_TYPE] = "Osc 1 Type";
	port_range_hints[LMS_OSC1_TYPE].HintDescriptor =
			LADSPA_HINT_DEFAULT_MINIMUM |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_OSC1_TYPE].LowerBound =  0;
	port_range_hints[LMS_OSC1_TYPE].UpperBound =  5;
        
        
        /*Parameters for osc1pitch*/        
	port_descriptors[LMS_OSC1_PITCH] = port_descriptors[LMS_ATTACK];
	port_names[LMS_OSC1_PITCH] = "Osc 1 Pitch";
	port_range_hints[LMS_OSC1_PITCH].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_OSC1_PITCH].LowerBound =  -12;
	port_range_hints[LMS_OSC1_PITCH].UpperBound =  12;
        
        
        /*Parameters for osc1tune*/        
	port_descriptors[LMS_OSC1_TUNE] = port_descriptors[LMS_ATTACK];
	port_names[LMS_OSC1_TUNE] = "Osc 1 Tune";
	port_range_hints[LMS_OSC1_TUNE].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_OSC1_TUNE].LowerBound = -100;
	port_range_hints[LMS_OSC1_TUNE].UpperBound =  100;
        
        
        /*Parameters for osc1vol*/        
	port_descriptors[LMS_OSC1_VOLUME] = port_descriptors[LMS_ATTACK];
	port_names[LMS_OSC1_VOLUME] = "Osc 1 Vol";
	port_range_hints[LMS_OSC1_VOLUME].HintDescriptor =
			LADSPA_HINT_DEFAULT_MAXIMUM |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_OSC1_VOLUME].LowerBound =  -60;
	port_range_hints[LMS_OSC1_VOLUME].UpperBound =  0;
        
        
        
        /*Parameters for osc2type*/        
	port_descriptors[LMS_OSC2_TYPE] = port_descriptors[LMS_ATTACK];
	port_names[LMS_OSC2_TYPE] = "Osc 2 Type";
	port_range_hints[LMS_OSC2_TYPE].HintDescriptor =
			LADSPA_HINT_DEFAULT_MAXIMUM |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_OSC2_TYPE].LowerBound =  0;
	port_range_hints[LMS_OSC2_TYPE].UpperBound =  4;
        
        
        /*Parameters for osc2pitch*/        
	port_descriptors[LMS_OSC2_PITCH] = port_descriptors[LMS_ATTACK];
	port_names[LMS_OSC2_PITCH] = "Osc 2 Pitch";
	port_range_hints[LMS_OSC2_PITCH].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_OSC2_PITCH].LowerBound =  -12;
	port_range_hints[LMS_OSC2_PITCH].UpperBound =  12;
        
        
        /*Parameters for osc2tune*/        
	port_descriptors[LMS_OSC2_TUNE] = port_descriptors[LMS_ATTACK];
	port_names[LMS_OSC2_TUNE] = "Osc 2 Tune";
	port_range_hints[LMS_OSC2_TUNE].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_OSC2_TUNE].LowerBound = -100;
	port_range_hints[LMS_OSC2_TUNE].UpperBound = 100; 
        
        
        /*Parameters for osc2vol*/        
	port_descriptors[LMS_OSC2_VOLUME] = port_descriptors[LMS_ATTACK];
	port_names[LMS_OSC2_VOLUME] = "Osc 2 Vol";
	port_range_hints[LMS_OSC2_VOLUME].HintDescriptor =
			LADSPA_HINT_DEFAULT_MAXIMUM |
			LADSPA_HINT_BOUNDED_BELOW |  LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_OSC2_VOLUME].LowerBound =  -60;
	port_range_hints[LMS_OSC2_VOLUME].UpperBound =  0;
        
        
        /*Parameters for master vol*/        
	port_descriptors[LMS_MASTER_VOLUME] = port_descriptors[LMS_ATTACK];
	port_names[LMS_MASTER_VOLUME] = "Master Vol";
	port_range_hints[LMS_MASTER_VOLUME].HintDescriptor =
			LADSPA_HINT_DEFAULT_HIGH |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_MASTER_VOLUME].LowerBound =  -60;
	port_range_hints[LMS_MASTER_VOLUME].UpperBound =  12;
        
        
        
        /*Parameters for master unison voices*/        
	port_descriptors[LMS_MASTER_UNISON_VOICES] = port_descriptors[LMS_ATTACK];
	port_names[LMS_MASTER_UNISON_VOICES] = "Master Unison";
	port_range_hints[LMS_MASTER_UNISON_VOICES].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_MASTER_UNISON_VOICES].LowerBound =  1;
	port_range_hints[LMS_MASTER_UNISON_VOICES].UpperBound =  7;
        
        
        /*Parameters for master unison spread*/        
	port_descriptors[LMS_MASTER_UNISON_SPREAD] = port_descriptors[LMS_ATTACK];
	port_names[LMS_MASTER_UNISON_SPREAD] = "Master Unison Spread";
	port_range_hints[LMS_MASTER_UNISON_SPREAD].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_MASTER_UNISON_SPREAD].LowerBound =  0;
	port_range_hints[LMS_MASTER_UNISON_SPREAD].UpperBound =  100;
        
        
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
        
        
        /*Parameters for pitch env amt*/        
	port_descriptors[LMS_PITCH_ENV_AMT] = port_descriptors[LMS_ATTACK];
	port_names[LMS_PITCH_ENV_AMT] = "Pitch Env Amt";
	port_range_hints[LMS_PITCH_ENV_AMT].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PITCH_ENV_AMT].LowerBound =  -36;
	port_range_hints[LMS_PITCH_ENV_AMT].UpperBound =   36;
        
        
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
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_LFO_FREQ].LowerBound = 10; 
	port_range_hints[LMS_LFO_FREQ].UpperBound = 400;
        
        /*Parameters for LFO Type*/        
	port_descriptors[LMS_LFO_TYPE] = port_descriptors[LMS_ATTACK];
	port_names[LMS_LFO_TYPE] = "LFO Type";
	port_range_hints[LMS_LFO_TYPE].HintDescriptor =
			LADSPA_HINT_DEFAULT_MINIMUM |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_LFO_TYPE].LowerBound = 0; 
	port_range_hints[LMS_LFO_TYPE].UpperBound = 2;
        
        /*Parameters for LFO Amp*/
	port_descriptors[LMS_LFO_AMP] = port_descriptors[LMS_ATTACK];
	port_names[LMS_LFO_AMP] = "LFO Amp";
	port_range_hints[LMS_LFO_AMP].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_LFO_AMP].LowerBound = -24;
	port_range_hints[LMS_LFO_AMP].UpperBound = 24;
        
        /*Parameters for LFO Pitch*/
	port_descriptors[LMS_LFO_PITCH] = port_descriptors[LMS_ATTACK];
	port_names[LMS_LFO_PITCH] = "LFO Pitch";
	port_range_hints[LMS_LFO_PITCH].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_LFO_PITCH].LowerBound = -36;
	port_range_hints[LMS_LFO_PITCH].UpperBound = 36;
        
        /*Parameters for LFO Filter*/
	port_descriptors[LMS_LFO_FILTER] = port_descriptors[LMS_ATTACK];
	port_names[LMS_LFO_FILTER] = "LFO Filter";
	port_range_hints[LMS_LFO_FILTER].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_LFO_FILTER].LowerBound = -48;
	port_range_hints[LMS_LFO_FILTER].UpperBound = 48;
        
        /*Parameters for program change*/        
	port_descriptors[LMS_PROGRAM_CHANGE] = port_descriptors[LMS_ATTACK];
	port_names[LMS_PROGRAM_CHANGE] = "Program Change";
	port_range_hints[LMS_PROGRAM_CHANGE].HintDescriptor =
			LADSPA_HINT_DEFAULT_MINIMUM | LADSPA_HINT_BOUNDED_BELOW | 
                        LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PROGRAM_CHANGE].LowerBound = 0; 
	port_range_hints[LMS_PROGRAM_CHANGE].UpperBound = 127;  // > 127 loads the first preset
        
        /*Step 17:  Add LADSPA ports*/
        
        
        /*Here is where the functions in synth.c get pointed to for the host to call*/
	LMSLDescriptor->activate = activateLMS;
	LMSLDescriptor->cleanup = cleanupLMS;
	LMSLDescriptor->connect_port = connectPortLMS;
	LMSLDescriptor->deactivate = NULL;
	LMSLDescriptor->instantiate = instantiateLMS;
	LMSLDescriptor->run = runLMSWrapper;
	LMSLDescriptor->run_adding = NULL;
	LMSLDescriptor->set_run_adding_gain = NULL;
    }

    LMSDDescriptor = (DSSI_Descriptor *) malloc(sizeof(DSSI_Descriptor));
    if (LMSDDescriptor) {
	LMSDDescriptor->DSSI_API_Version = 1;
	LMSDDescriptor->LADSPA_Plugin = LMSLDescriptor;
	LMSDDescriptor->configure = NULL;  //TODO:  I think this is where the host can set plugin state, etc...
	LMSDDescriptor->get_program = NULL;  //TODO:  This is where program change is read, plugin state retrieved, etc...
	LMSDDescriptor->get_midi_controller_for_port = getControllerLMS;
	LMSDDescriptor->select_program = NULL;  //TODO:  This is how the host can select programs, not sure how it differs from a MIDI program change
	LMSDDescriptor->run_synth = run_lms_ray_v;
	LMSDDescriptor->run_synth_adding = NULL;
	LMSDDescriptor->run_multiple_synths = NULL;
	LMSDDescriptor->run_multiple_synths_adding = NULL;
    }
}

#ifdef __GNUC__
__attribute__((destructor)) void fini()
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
