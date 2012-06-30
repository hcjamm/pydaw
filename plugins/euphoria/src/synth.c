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
#include <samplerate.h>
#include <pthread.h>

#include "synth.h"
#include "meta.h"
#include "../../libmodsynth/lib/lms_math.h"

static LADSPA_Descriptor *samplerStereoLDescriptor = NULL;

static DSSI_Descriptor *samplerStereoDDescriptor = NULL;

static void runSampler(LADSPA_Handle instance, unsigned long sample_count,
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
            //Begin Ray-V ports

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
        case LMS_FILTER_TYPE:
            plugin->filter_type = data;
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

            //End Ray-V ports
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
}

static LADSPA_Handle instantiateSampler(const LADSPA_Descriptor * descriptor,
					unsigned long s_rate)
{
    Sampler *plugin_data = (Sampler *) malloc(sizeof(Sampler));
    pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

    //!!! check rv from malloc throughout

    plugin_data->output[0] = 0;
    plugin_data->output[1] = 0;
    plugin_data->retune = 0;    
    plugin_data->sustain = 0;
    plugin_data->release = 0;
    plugin_data->selected_sample = 0;
    plugin_data->i_selected_sample = 0;
    plugin_data->current_sample = 0;
    plugin_data->loaded_samples_count = 0;
    plugin_data->lin_interpolator = g_lin_get();
    
    int f_i = 0;
    while(f_i < LMS_MAX_SAMPLE_COUNT)
    {
        plugin_data->sampleData[0][f_i] = 0;
        plugin_data->sampleData[1][f_i] = 0;
        plugin_data->sampleCount[f_i] = 0;
        plugin_data->basePitch[f_i] = 0;
        plugin_data->low_note[f_i] = 0;
        plugin_data->high_note[f_i] = 0;
        plugin_data->sample_vol[f_i] = 0;
        plugin_data->sample_amp[f_i] = 1.0f;
        
        plugin_data->smp_pit_core[f_i] = g_pit_get();
        plugin_data->smp_pit_ratio[f_i] = g_pit_ratio();
        
        plugin_data->sample_paths[f_i] = "";
        
        f_i++;
    }
    
    f_i = 0;
    int f_i2;
    
    while(f_i < Sampler_NOTES)
    {
        plugin_data->data[f_i] = g_poly_init(s_rate);
        plugin_data->sampleStarts[f_i] = 0;
        plugin_data->sampleEnds[f_i] = 0;
        
        f_i2 = 0;
        while(f_i2 < LMS_MAX_SAMPLE_COUNT)
        {
            plugin_data->sample_position[f_i][f_i2] = 0.0f;
            f_i2++;
        }
        
        f_i++;
    }
    
    plugin_data->sampleRate = s_rate;
    plugin_data->projectDir = 0;

    plugin_data->channels = 2;
    plugin_data->amp_ptr = g_amp_get();
    
    memcpy(&plugin_data->mutex, &m, sizeof(pthread_mutex_t));
    
    plugin_data->mono_modules = g_mono_init(s_rate);
    
    
    //Begin Ray-V MIDI CC Map
    
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
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_MASTER_VOLUME, 36, "Master Volume");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_MASTER_GLIDE, 39, "Glide Time");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_MASTER_PITCHBEND_AMT, 40, "Pitchbend Amount");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_PITCH_ENV_AMT, 42, "Pitch Env Amt");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_PITCH_ENV_TIME, 43, "Pitch Env Time");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_LFO_FREQ, 44, "LFO Freq");    
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_LFO_TYPE, 45, "LFO Type");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_LFO_AMP, 46, "LFO Amp");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_LFO_PITCH, 47, "LFO Pitch");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_LFO_FILTER, 48, "LFO Filter");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_FILTER_TYPE, 49, "Filter Type");
    
    v_ccm_read_file_to_array(plugin_data->midi_cc_map, "euphoria-cc_map.txt");
    
    //End Ray-V MIDI CC Map
    

    return (LADSPA_Handle) plugin_data;
}

static void activateSampler(LADSPA_Handle instance)
{
    Sampler *plugin_data = (Sampler *) instance;
    unsigned int i;

    pthread_mutex_lock(&plugin_data->mutex);

    plugin_data->sampleNo = 0;

    for (i = 0; i < Sampler_NOTES; i++) {
	//plugin_data->ons[i] = -1;
	plugin_data->velocities[i] = 0;
    }

    pthread_mutex_unlock(&plugin_data->mutex);
}

/* static void addSample(Sampler *plugin_data, 
 * int n, //The note number?
 * unsigned long pos, //the position in the output buffer
 * unsigned long count) //how many samples to fill in the output buffer?
 */
static void addSample(Sampler *plugin_data, int n, unsigned long pos, unsigned long count)
{
    plugin_data->sample_amp[(plugin_data->current_sample)] = f_db_to_linear(*(plugin_data->sample_vol[(plugin_data->current_sample)]) , plugin_data->amp_ptr);
    
    float ratio = 1.0f;
    
    //float gain_test = 1.0;
    unsigned long i, ch;

    //if (pos + plugin_data->sampleNo < plugin_data->ons[n]) return;

    for (i = 0; i < count; ++i) {

        //Run the glide module
            
        f_rmp_run_ramp(plugin_data->data[n]->pitch_env);
        f_rmp_run_ramp(plugin_data->data[n]->glide_env);
        
        //Set and run the LFO
        v_lfs_set(plugin_data->data[n]->lfo1,  (*(plugin_data->lfo_freq)) * .01  );
        v_lfs_run(plugin_data->data[n]->lfo1);
        plugin_data->data[n]->lfo_amp_output = f_db_to_linear_fast((((*(plugin_data->lfo_amp)
                ) * (plugin_data->data[n]->lfo1->output)) - (f_lms_abs((*(plugin_data->lfo_amp)
                )) * 0.5)), plugin_data->data[n]->amp_ptr);
        plugin_data->data[n]->lfo_filter_output = ( *(plugin_data->lfo_filter)) * (plugin_data->data[n]->lfo1->output);
        plugin_data->data[n]->lfo_pitch_output = (*(plugin_data->lfo_pitch)) * (plugin_data->data[n]->lfo1->output);
        
        plugin_data->data[n]->base_pitch = (plugin_data->data[n]->glide_env->output_multiplied) + (plugin_data->data[n]->pitch_env->output_multiplied) 
                +  (plugin_data->mono_modules->pitchbend_smoother->output)  //(plugin_data->sv_pitch_bend_value)
                + (plugin_data->data[n]->last_pitch);
                
        if (plugin_data->basePitch[(plugin_data->current_sample)])
        {
            ratio =
            f_pit_midi_note_to_ratio_fast(*(plugin_data->basePitch[(plugin_data->current_sample)]),                     
                    ((plugin_data->data[n]->base_pitch) + (plugin_data->data[n]->lfo_pitch_output)),
                    plugin_data->smp_pit_core[(plugin_data->current_sample)], plugin_data->smp_pit_ratio[(plugin_data->current_sample)]);
        }
        	
        plugin_data->sample_position[n][(plugin_data->current_sample)] = (plugin_data->sample_position[n][(plugin_data->current_sample)]) + ratio;
        
        float f_adjusted_sample_position = (plugin_data->sample_position[n][(plugin_data->current_sample)]) + (plugin_data->sampleStartPos[(plugin_data->current_sample)]);
        
	if ((f_adjusted_sample_position) >=  plugin_data->sampleEndPos[plugin_data->current_sample]){
	    //plugin_data->ons[n] = -1;
            //TODO:  Find a way to stop processing earlier when the sample is finished playing
	    break;
	}
       
        //Run things that aren't per-channel like envelopes
                
        v_adsr_run(plugin_data->data[n]->adsr_amp);        

        v_adsr_run(plugin_data->data[n]->adsr_filter);
        
	for (ch = 0; ch < plugin_data->channels; ++ch) {

            float sample = f_linear_interpolate_ptr_wrap(plugin_data->sampleData[ch][(plugin_data->current_sample)], 
                    (plugin_data->sampleCount[(plugin_data->current_sample)]),
                    (f_adjusted_sample_position),
                    plugin_data->lin_interpolator);
            
            /*Process PolyFX here*/
            
            //Call everything defined in libmodsynth.h in the order it should be called in
                                    
            sample += (f_run_white_noise(plugin_data->data[n]->white_noise1[ch]) * (plugin_data->data[n]->noise_linamp)); //white noise
                        
            //TODO:  Run the filter smoother
            v_svf_set_cutoff_base(plugin_data->data[n]->svf_filter[ch],  (plugin_data->mono_modules->filter_smoother->output));//(*(plugin_data->timbre)));
            //Run v_svf_add_cutoff_mod once for every input source
            v_svf_add_cutoff_mod(plugin_data->data[n]->svf_filter[ch], 
                    (((plugin_data->data[n]->adsr_filter->output) * ( *(plugin_data->filter_env_amt)
                    )) + (plugin_data->data[n]->lfo_filter_output)));        
            //calculate the cutoff
            v_svf_set_cutoff(plugin_data->data[n]->svf_filter[ch]);
            
            plugin_data->data[n]->filter_output = plugin_data->data[n]->svf_function(plugin_data->data[n]->svf_filter[ch], (sample));

            //Crossfade between the filter, and the filter run through the distortion unit
            sample = f_axf_run_xfade((plugin_data->data[n]->dist_dry_wet[ch]), (plugin_data->data[n]->filter_output), 
                    f_clp_clip(plugin_data->data[n]->clipper1[ch], (plugin_data->data[n]->filter_output)));
            
            sample = (sample) * (plugin_data->data[n]->adsr_amp->output) * (plugin_data->data[n]->amp) * (plugin_data->data[n]->lfo_amp_output) 
                    * (plugin_data->sample_amp[(plugin_data->current_sample)]);
    
            //If the main ADSR envelope has reached the end it's release stage, kill the voice.
            //However, you don't have to necessarily have to kill the voice, but you will waste a lot of CPU if you don't            
            if(plugin_data->data[n]->adsr_amp->stage == 4)
            {
                break;
            }
            
            /*End process PolyFX*/
            
	    plugin_data->output[ch][pos + i] += sample;
	}
    }
}

static void runSampler(LADSPA_Handle instance, unsigned long sample_count,
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

    for (pos = 0, event_pos = 0; pos < sample_count; ) {
        
	while (event_pos < event_count){
	       //&& pos >= events[event_pos].time.tick) {
            /*Note-on event*/
	    if (events[event_pos].type == SND_SEQ_EVENT_NOTEON) {
		snd_seq_ev_note_t n = events[event_pos].data.note;
		if (n.velocity > 0) {
		    /*plugin_data->ons[n.note] =
			plugin_data->sampleNo + events[event_pos].time.tick;*/
		    
		    plugin_data->velocities[n.note] = n.velocity;
                    
                    //Reset the sample start positions to 0.  TODO:  optimize this
                    for(i = 0; i < LMS_MAX_SAMPLE_COUNT; i++)
                    {
                        plugin_data->sample_position[n.note][i] = 0.0f;
                    }
                    
                    /*Set the svf_function function pointer to the filter type selected in the GUI*/
                    switch((int)(*(plugin_data->filter_type)))
                    {
                                case 0:
                                    plugin_data->data[n.note]->svf_function = v_svf_run_2_pole_lp;
                                    break;
                                case 1:
                                    plugin_data->data[n.note]->svf_function = v_svf_run_2_pole_hp;
                                    break;
                                case 2:
                                    plugin_data->data[n.note]->svf_function = v_svf_run_2_pole_bp;
                                    break;
                                case 3:
                                    plugin_data->data[n.note]->svf_function = v_svf_run_4_pole_lp;
                                    break;
                                case 4:
                                    plugin_data->data[n.note]->svf_function = v_svf_run_4_pole_hp;
                                    break;
                                case 5:
                                    plugin_data->data[n.note]->svf_function = v_svf_run_4_pole_bp;
                                    break;                
                                case 6:
                                    plugin_data->data[n.note]->svf_function = v_svf_run_no_filter;
                                    break;
                                default:
                                    plugin_data->data[n.note]->svf_function = v_svf_run_no_filter;
                                    printf("Invalid filter type:  %i", (int)(*(plugin_data->filter_type)));
                                    break;

                    }
                    
                    //Begin Ray-V additions
                    
                    //const int voice = i_pick_voice(plugin_data->voices, n.note);
                    
		    plugin_data->data[n.note]->amp = f_db_to_linear_fast(((n.velocity * 0.094488) - 12 + *(plugin_data->master_vol)), //-20db to 0db, + master volume (0 to -60)
                            plugin_data->mono_modules->amp_ptr);                     
                    
                    plugin_data->data[n.note]->note_f = (float)n.note;
                                        
                    plugin_data->data[n.note]->target_pitch = (plugin_data->data[n.note]->note_f);
                    plugin_data->data[n.note]->last_pitch = (plugin_data->sv_last_note);
                    
                    v_rmp_retrigger_glide_t(plugin_data->data[n.note]->glide_env , (*(plugin_data->master_glide) * .01), 
                            (plugin_data->sv_last_note), (plugin_data->data[n.note]->target_pitch));
                                                    
                    plugin_data->data[n.note]->noise_linamp = f_db_to_linear_fast(*(plugin_data->noise_amp), plugin_data->mono_modules->amp_ptr);
                                        
                    /*Here is where we perform any actions that should ONLY happen at note_on, you can save a lot of CPU by
                     placing things here that don't need to be modulated as a note is playing*/
                    
                    /*Retrigger ADSR envelopes and LFO*/
                    v_adsr_retrigger(plugin_data->data[n.note]->adsr_amp);
                    v_adsr_retrigger(plugin_data->data[n.note]->adsr_filter);
                    v_lfs_sync(plugin_data->data[n.note]->lfo1, 0.0f, *(plugin_data->lfo_type));
                    
                    v_adsr_set_adsr_db(plugin_data->data[n.note]->adsr_amp, (*(plugin_data->attack) * .01), (*(plugin_data->decay) * .01), (*(plugin_data->sustain)), (*(plugin_data->release) * .01));
                    v_adsr_set_adsr(plugin_data->data[n.note]->adsr_filter, (*(plugin_data->attack_f) * .01), (*(plugin_data->decay_f) * .01), (*(plugin_data->sustain_f) * .01), (*(plugin_data->release_f) * .01));
                    
                    /*Retrigger the pitch envelope*/
                    v_rmp_retrigger((plugin_data->data[n.note]->pitch_env), (*(plugin_data->pitch_env_time) * .01), *(plugin_data->pitch_env_amt));  
                    
                    plugin_data->data[n.note]->noise_amp = f_db_to_linear(*(plugin_data->noise_amp), plugin_data->mono_modules->amp_ptr);
                                        
                    /*Set the last_note property, so the next note can glide from it if glide is turned on*/
                    plugin_data->sv_last_note = (plugin_data->data[n.note]->note_f);
                    
                    //TODO:  Create a define for the number of channels
                    //Move all of the multi-channel functions here
                    for(i = 0; i < 2; i++)
                    {
                        v_svf_velocity_mod(plugin_data->data[n.note]->svf_filter[i], n.velocity);
                        v_clp_set_in_gain(plugin_data->data[n.note]->clipper1[i], *(plugin_data->dist));
                        v_svf_set_res(plugin_data->data[n.note]->svf_filter[i], *(plugin_data->res));
                        v_axf_set_xfade(plugin_data->data[n.note]->dist_dry_wet[i], (*(plugin_data->dist_wet) * .01));
                    }
                    
                    //End Ray-V additions                    
                    
		} else {
                    v_poly_note_off(plugin_data->data[n.note]);
		    //if (!plugin_data->sustain || (*plugin_data->sustain < 0.001)) {
			/*plugin_data->offs[n.note] = 
			    plugin_data->sampleNo + events[event_pos].time.tick;*/
		    //}
		}
	    } /*Note-off event*/
            else if (events[event_pos].type == SND_SEQ_EVENT_NOTEOFF )
                    //&& (!plugin_data->sustain || (*plugin_data->sustain < 0.001))) 
            {
		snd_seq_ev_note_t n = events[event_pos].data.note;
		/*plugin_data->offs[n.note] = 
		    plugin_data->sampleNo + events[event_pos].time.tick;*/
                
                v_poly_note_off(plugin_data->data[n.note]);
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
        
        //An ugly hack to get the smoother to run faster.  TODO:  Fix this correctly
        int f_i2;        
        for(f_i2 = 0; f_i2 < 8; f_i2++)
        {
            v_smr_iir_run(plugin_data->mono_modules->filter_smoother, (*(plugin_data->timbre)));
            v_smr_iir_run(plugin_data->mono_modules->pitchbend_smoother, (plugin_data->sv_pitch_bend_value));
        }
        
	for (i = 0; i < Sampler_NOTES; ++i) {
            if(plugin_data->data[i]->adsr_amp->stage != 4){
                plugin_data->i_loaded_samples = 0;

                while((plugin_data->i_loaded_samples) < (plugin_data->loaded_samples_count))
                {
                    if((i >= *(plugin_data->low_note[(plugin_data->loaded_samples[(plugin_data->i_loaded_samples)])])) && 
                            (i <= *(plugin_data->high_note[(plugin_data->loaded_samples[(plugin_data->i_loaded_samples)])])))
                    {
                        plugin_data->current_sample = (plugin_data->loaded_samples[(plugin_data->i_loaded_samples)]);
                        
                        plugin_data->sampleStartPos[(plugin_data->current_sample)] = (plugin_data->sampleCount[(plugin_data->current_sample)]) * ((*(plugin_data->sampleStarts[(plugin_data->current_sample)])) * .0001);
                        plugin_data->sampleEndPos[(plugin_data->current_sample)] = (plugin_data->sampleCount[(plugin_data->current_sample)]) - ((plugin_data->sampleCount[(plugin_data->current_sample)]) * ((*(plugin_data->sampleEnds[(plugin_data->current_sample)])) * .0001));

                        
                        addSample(plugin_data, i, pos, count);
                    }

                    plugin_data->i_loaded_samples = (plugin_data->i_loaded_samples) + 1;
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
    runSampler(instance, sample_count, NULL, 0);
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
    int i_loaded_samples = 0;
    plugin_data->sample_is_loaded = 0;
    
    while((i_loaded_samples) < (plugin_data->loaded_samples_count))
    {
        if((plugin_data->loaded_samples[(i_loaded_samples)]) == a_index)
        {
            printf("Sample index %i is already loaded.\n", (i_loaded_samples));
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
    
    SF_INFO info;
    SNDFILE *file;
    size_t samples = 0;
    float *tmpFrames, *tmpSamples[2], *tmpResamples, *tmpOld[2];
    char *revisedPath = 0;
    size_t i;

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

    tmpResamples = 0;

    if (info.samplerate != plugin_data->sampleRate) {
	
	double ratio = (double)plugin_data->sampleRate / (double)info.samplerate;
	size_t target = (size_t)(info.frames * ratio);
	SRC_DATA data;

	tmpResamples = (float *)malloc(target * info.channels * sizeof(float));
	memset(tmpResamples, 0, target * info.channels * sizeof(float));

	data.data_in = tmpFrames;
	data.data_out = tmpResamples;
	data.input_frames = info.frames;
	data.output_frames = target;
	data.src_ratio = ratio;

	if (!src_simple(&data, SRC_SINC_BEST_QUALITY, info.channels)) {
	    free(tmpFrames);
	    tmpFrames = tmpResamples;
	    samples = target;
	} else {
	    free(tmpResamples);
	}
    }

    /* add an extra sample for linear interpolation */
    tmpSamples[0] = (float *)malloc((samples + 1) * sizeof(float));

    if (plugin_data->channels == 2) {
	tmpSamples[1] = (float *)malloc((samples + 1) * sizeof(float));
    } else {
	tmpSamples[1] = NULL;
    }


    if (plugin_data->channels == 2) {
	for (i = 0; i < samples; ++i) {
	    int j;
	    for (j = 0; j < 2; ++j) {
		if (j == 1 && info.frames < 2) {
		    tmpSamples[j][i] = tmpSamples[0][i];
		} else {
		    tmpSamples[j][i] = tmpFrames[i * info.channels + j];
		}
	    }
	}
    } else {
	for (i = 0; i < samples; ++i) {
	    int j;
	    tmpSamples[0][i] = 0.0f;
	    for (j = 0; j < info.channels; ++j) {
		tmpSamples[0][i] += tmpFrames[i * info.channels + j];
	    }
	}
    }

    free(tmpFrames);

    /* add an extra sample for linear interpolation */
    tmpSamples[0][samples] = 0.0f;
    if (plugin_data->channels == 2) {
	tmpSamples[1][samples] = 0.0f;
    }
    
    pthread_mutex_lock(&plugin_data->mutex);

    tmpOld[0] = plugin_data->sampleData[0][(a_index)];
    tmpOld[1] = plugin_data->sampleData[1][(a_index)];
    plugin_data->sampleData[0][(a_index)] = tmpSamples[0];
    plugin_data->sampleData[1][(a_index)] = tmpSamples[1];
    plugin_data->sampleCount[(a_index)] = samples;

    plugin_data->sample_paths[(a_index)] = path;
    
    /*
    for (i = 0; i < Sampler_NOTES; ++i) {
	plugin_data->ons[i] = -1;
	plugin_data->velocities[i] = 0;
    }
    */
    pthread_mutex_unlock(&plugin_data->mutex);

    if (tmpOld[0]) free(tmpOld[0]);
    if (tmpOld[1]) free(tmpOld[1]);
    
    printf("%s: loaded %s (%ld samples from original %ld channels resampled from %ld frames at %ld Hz)\n", Sampler_Stereo_LABEL, path, (long)samples, (long)info.channels, (long)info.frames, (long)info.samplerate);

    if (revisedPath) {
	char *message = dssi_configure_message("warning: sample file '%s' not found: loading from '%s' instead", path, revisedPath);
	free(revisedPath);
	return message;
    }

    return NULL;
}

char *samplerClear(Sampler *plugin_data, int a_index)
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
            printf("Sample index %i is loaded.\n", (plugin_data->i_loaded_samples));
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

    if (tmpOld[0]) free(tmpOld[0]);
    if (tmpOld[1]) free(tmpOld[1]);

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
    
    while (f_samples_loaded_count < LMS_MAX_SAMPLE_COUNT)
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
            else if(strcmp(f_result_string, plugin_data->sample_paths[f_samples_loaded_count]) != 0)
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

    //!!! malloc rv

    for (channels = 1; channels <= 2; ++channels) {

	int stereo = (channels == 2);

	LADSPA_Descriptor *desc = samplerStereoLDescriptor;

	desc->UniqueID = channels;
	desc->Label = "LMS_EUPHORIA"; //(stereo ? Sampler_Stereo_LABEL : Sampler_Mono_LABEL);
	desc->Properties = LADSPA_PROPERTY_REALTIME | LADSPA_PROPERTY_HARD_RT_CAPABLE;
	desc->Name =  LMS_PLUGIN_LONG_NAME; //(stereo ? "Simple Stereo Sampler" : "Simple Mono Sampler");
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
        
        //Begin Ray-V PolyFX ports        
        
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
			//LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_DIST_WET].LowerBound =  0; 
	port_range_hints[LMS_DIST_WET].UpperBound =  100;
        
        /*Parameters for master vol*/        
	port_descriptors[LMS_MASTER_VOLUME] = port_descriptors[LMS_ATTACK];
	port_names[LMS_MASTER_VOLUME] = "Master Vol";
	port_range_hints[LMS_MASTER_VOLUME].HintDescriptor =
			LADSPA_HINT_DEFAULT_HIGH |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_MASTER_VOLUME].LowerBound =  -60;
	port_range_hints[LMS_MASTER_VOLUME].UpperBound =  12;
                        
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
        
        /*Parameters for filter type*/        
	port_descriptors[LMS_FILTER_TYPE] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_FILTER_TYPE] = "Filter Type";
	port_range_hints[LMS_FILTER_TYPE].HintDescriptor =
                        LADSPA_HINT_DEFAULT_MINIMUM | LADSPA_HINT_INTEGER |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_FILTER_TYPE].LowerBound =  0;
	port_range_hints[LMS_FILTER_TYPE].UpperBound =  5;
        
        //End Ray-V
        
        int f_i = LMS_SAMPLE_PITCH_PORT_RANGE_MIN;
        
        while(f_i < LMS_SAMPLE_PITCH_PORT_RANGE_MAX)
        {
            port_descriptors[f_i] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
            port_names[f_i] = "Sample Pitch";
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
            port_names[f_i] = "Sample Start";
            port_range_hints[f_i].HintDescriptor = LADSPA_HINT_DEFAULT_MINIMUM | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_INTEGER;
            port_range_hints[f_i].LowerBound = 0; port_range_hints[f_i].UpperBound = 10000;
            
            f_i++;
        }
        
	desc->activate = activateSampler;
	desc->cleanup = cleanupSampler;
	desc->connect_port = connectPortSampler;
	desc->deactivate = activateSampler; // sic
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
    samplerStereoDDescriptor->run_synth = runSampler;
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
