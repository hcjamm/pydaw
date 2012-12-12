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
		      LADSPA_Data *out0, LADSPA_Data *out1, unsigned int count);

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
    case WAYV_ATTACK:
	plugin->attack = data;
	break;
    case WAYV_DECAY:
	plugin->decay = data;
	break;
    case WAYV_SUSTAIN:
	plugin->sustain = data;
	break;
    case WAYV_RELEASE:
	plugin->release = data;
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
    case WAYV_MASTER_UNISON_VOICES:
        plugin->master_uni_voice = data;
        break;
    case WAYV_MASTER_UNISON_SPREAD:
        plugin->master_uni_spread = data;        
        break;
    case WAYV_MASTER_GLIDE:
        plugin->master_glide = data;
        break;
    case WAYV_MASTER_PITCHBEND_AMT:
        plugin->master_pb_amt = data;
        break;
    /*case LMS_PROGRAM_CHANGE:
        plugin->program = data;
        break;*/    
    }
}

static LADSPA_Handle g_wayv_instantiate(const LADSPA_Descriptor * descriptor,
				   unsigned long s_rate)
{
    t_wayv *plugin_data = (t_wayv *) malloc(sizeof(t_wayv));
    
    plugin_data->fs = s_rate;
    
    plugin_data->midi_cc_map = g_ccm_get();
    v_ccm_set_cc(plugin_data->midi_cc_map, WAYV_ATTACK, 22, "Attack Amp");
    v_ccm_set_cc(plugin_data->midi_cc_map, WAYV_RELEASE, 5, "Release Amp");    
    v_ccm_set_cc(plugin_data->midi_cc_map, WAYV_NOISE_AMP, 73, "Noise Amp");
    
    v_ccm_read_file_to_array(plugin_data->midi_cc_map, "ray_v-cc_map.txt");
    
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
        plugin_data->data[i] = g_rayv_poly_init();
        plugin_data->data[i]->note_f = i;        
    }
    plugin_data->sampleNo = 0;
        
    plugin_data->pitch = 1.0f;
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

                v_adsr_retrigger(plugin_data->data[f_voice]->adsr_amp);
                v_adsr_retrigger(plugin_data->data[f_voice]->adsr_amp1);
                v_adsr_retrigger(plugin_data->data[f_voice]->adsr_amp2);

                float f_attack = *(plugin_data->attack) * .01f;
                f_attack = (f_attack) * (f_attack);
                float f_decay = *(plugin_data->decay) * .01f;
                f_decay = (f_decay) * (f_decay);
                float f_release = *(plugin_data->release) * .01f;
                f_release = (f_release) * (f_release);   
                
                v_adsr_set_adsr_db(plugin_data->data[f_voice]->adsr_amp, (f_attack), (f_decay), *(plugin_data->sustain), (f_release));
                
                
                float f_attack1 = *(plugin_data->attack1) * .01f;
                f_attack1 = (f_attack1) * (f_attack1);
                float f_decay1 = *(plugin_data->decay1) * .01f;
                f_decay1 = (f_decay1) * (f_decay1);
                float f_release1 = *(plugin_data->release1) * .01f;
                f_release1 = (f_release1) * (f_release1);   
                
                v_adsr_set_adsr_db(plugin_data->data[f_voice]->adsr_amp1, (f_attack1), (f_decay1), *(plugin_data->sustain1), (f_release1));
                


                float f_attack2 = *(plugin_data->attack2) * .01f;
                f_attack2 = (f_attack2) * (f_attack2);
                float f_decay2 = *(plugin_data->decay2) * .01f;
                f_decay2 = (f_decay2) * (f_decay2);
                float f_release2 = *(plugin_data->release2) * .01f;
                f_release2 = (f_release2) * (f_release2);   
                
                v_adsr_set_adsr_db(plugin_data->data[f_voice]->adsr_amp2, (f_attack2), (f_decay2), *(plugin_data->sustain2), (f_release2));
                                
                
                
                plugin_data->data[f_voice]->noise_amp = f_db_to_linear(*(plugin_data->noise_amp), plugin_data->mono_modules->amp_ptr);

                int f_osc_type1 = (int)(*plugin_data->osc1type);
                
                v_osc_wav_set_waveform(plugin_data->data[f_voice]->osc_wavtable1, 
                        plugin_data->data[f_voice]->wavetables->tables[f_osc_type1]->wavetable,
                        plugin_data->data[f_voice]->wavetables->tables[f_osc_type1]->length);
                
                int f_osc_type2 = 0;  // (int)(*plugin_data->osc2type);  //TODO: Fix this once the UI is proper again...
                
                v_osc_wav_set_waveform(plugin_data->data[f_voice]->osc_wavtable2, 
                        plugin_data->data[f_voice]->wavetables->tables[f_osc_type2]->wavetable,
                        plugin_data->data[f_voice]->wavetables->tables[f_osc_type2]->length);
                
                v_osc_wav_set_uni_voice_count(plugin_data->data[f_voice]->osc_wavtable1, *plugin_data->master_uni_voice);
                v_osc_wav_set_uni_voice_count(plugin_data->data[f_voice]->osc_wavtable2, *plugin_data->master_uni_voice);

                /*Set the last_note property, so the next note can glide from it if glide is turned on*/
                plugin_data->sv_last_note = (plugin_data->data[f_voice]->note_f);
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
        if((plugin_data->data[(plugin_data->i_run_poly_voice)]->adsr_amp->stage) != 4)        
        {
            v_run_wayv_voice(plugin_data,                    
                    plugin_data->voices->voices[(plugin_data->i_run_poly_voice)],
                    plugin_data->data[(plugin_data->i_run_poly_voice)],                    
                    output0,
                    output1,
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

static void v_run_wayv_voice(t_wayv *plugin_data, t_voc_single_voice a_poly_voice, t_rayv_poly_voice *a_voice, LADSPA_Data *out0, LADSPA_Data *out1, unsigned int count)
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
        
        f_rmp_run_ramp(a_voice->glide_env);
        
        a_voice->base_pitch = (a_voice->glide_env->output_multiplied) //+ (a_voice->pitch_env->output_multiplied) 
                + (plugin_data->mono_modules->pitchbend_smoother->output) + (a_voice->last_pitch);
               
        v_osc_wav_set_unison_pitch(a_voice->osc_wavtable1, (*plugin_data->master_uni_spread) * 0.01f,
                ((a_voice->base_pitch) + (*plugin_data->osc1pitch) + ((*plugin_data->osc1tune) * 0.01f) )); //+ (a_voice->lfo_pitch_output)));
       
        v_osc_wav_set_unison_pitch(a_voice->osc_wavtable2, (*plugin_data->master_uni_spread) * 0.01f,
                ((a_voice->base_pitch) + (*plugin_data->osc2pitch) + ((*plugin_data->osc2tune) * 0.01f) )); //+ (a_voice->lfo_pitch_output)));
        
        v_adsr_run_db(a_voice->adsr_amp1);
        v_adsr_run_db(a_voice->adsr_amp2);
        
        a_voice->current_sample += f_osc_wav_run_unison(a_voice->osc_wavtable1) * (a_voice->adsr_amp1->output);
        a_voice->current_sample += f_osc_wav_run_unison(a_voice->osc_wavtable2) * (a_voice->adsr_amp2->output);
        
        a_voice->current_sample += (f_run_white_noise(a_voice->white_noise1) * (a_voice->noise_linamp)); //white noise
        
        v_adsr_run_db(a_voice->adsr_amp);
                
        a_voice->current_sample = (a_voice->current_sample) * (a_voice->adsr_amp->output) * (a_voice->amp);
        
        /*Run the envelope and assign to the output buffers*/
        out0[(a_voice->i_voice)] += (a_voice->current_sample);
        out1[(a_voice->i_voice)] += (a_voice->current_sample);                
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
	port_descriptors[WAYV_ATTACK] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[WAYV_ATTACK] = "Attack time (s)";
	port_range_hints[WAYV_ATTACK].HintDescriptor =
			LADSPA_HINT_DEFAULT_MINIMUM |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_ATTACK].LowerBound = 10.0f; 
	port_range_hints[WAYV_ATTACK].UpperBound = 100.0f; 

	/* Parameters for decay */
	port_descriptors[WAYV_DECAY] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[WAYV_DECAY] = "Decay time (s)";
	port_range_hints[WAYV_DECAY].HintDescriptor =
			LADSPA_HINT_DEFAULT_LOW |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
			
	port_range_hints[WAYV_DECAY].LowerBound = 10.0f; 
	port_range_hints[WAYV_DECAY].UpperBound = 100.0f; 

	/* Parameters for sustain */
	port_descriptors[WAYV_SUSTAIN] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[WAYV_SUSTAIN] = "Sustain level (%)";
	port_range_hints[WAYV_SUSTAIN].HintDescriptor =
			LADSPA_HINT_DEFAULT_MAXIMUM |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_SUSTAIN].LowerBound = -60.0f;
	port_range_hints[WAYV_SUSTAIN].UpperBound = 0.0f;

	/* Parameters for release */
	port_descriptors[WAYV_RELEASE] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[WAYV_RELEASE] = "Release time (s)";
	port_range_hints[WAYV_RELEASE].HintDescriptor =
			LADSPA_HINT_DEFAULT_LOW | 
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_RELEASE].LowerBound = 10.0f; 
	port_range_hints[WAYV_RELEASE].UpperBound = 200.0f; 

        
        
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
	port_range_hints[WAYV_SUSTAIN1].LowerBound = -60.0f;
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
	port_range_hints[WAYV_SUSTAIN2].LowerBound = -60.0f;
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
	port_descriptors[WAYV_NOISE_AMP] = port_descriptors[WAYV_ATTACK];
	port_names[WAYV_NOISE_AMP] = "Dist";
	port_range_hints[WAYV_NOISE_AMP].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_NOISE_AMP].LowerBound =  -60.0f;
	port_range_hints[WAYV_NOISE_AMP].UpperBound =  0.0f;
                
        
        /*Parameters for osc1type*/        
	port_descriptors[WAYV_OSC1_TYPE] = port_descriptors[WAYV_ATTACK];
	port_names[WAYV_OSC1_TYPE] = "Osc 1 Type";
	port_range_hints[WAYV_OSC1_TYPE].HintDescriptor =
			LADSPA_HINT_DEFAULT_MINIMUM |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_OSC1_TYPE].LowerBound =  0.0f;
	port_range_hints[WAYV_OSC1_TYPE].UpperBound =  2.0f;
        
        
        /*Parameters for osc1pitch*/        
	port_descriptors[WAYV_OSC1_PITCH] = port_descriptors[WAYV_ATTACK];
	port_names[WAYV_OSC1_PITCH] = "Osc 1 Pitch";
	port_range_hints[WAYV_OSC1_PITCH].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_OSC1_PITCH].LowerBound =  -12.0f;
	port_range_hints[WAYV_OSC1_PITCH].UpperBound =  12.0f;
        
        
        /*Parameters for osc1tune*/        
	port_descriptors[WAYV_OSC1_TUNE] = port_descriptors[WAYV_ATTACK];
	port_names[WAYV_OSC1_TUNE] = "Osc 1 Tune";
	port_range_hints[WAYV_OSC1_TUNE].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_OSC1_TUNE].LowerBound = -100.0f;
	port_range_hints[WAYV_OSC1_TUNE].UpperBound =  100.0f;
        
        
        /*Parameters for osc1vol*/        
	port_descriptors[WAYV_OSC1_VOLUME] = port_descriptors[WAYV_ATTACK];
	port_names[WAYV_OSC1_VOLUME] = "Osc 1 Vol";
	port_range_hints[WAYV_OSC1_VOLUME].HintDescriptor =
			LADSPA_HINT_DEFAULT_MAXIMUM |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_OSC1_VOLUME].LowerBound =  -60.0f;
	port_range_hints[WAYV_OSC1_VOLUME].UpperBound =  0.0f;
        
        
        
        /*Parameters for osc2type*/        
	port_descriptors[WAYV_OSC2_TYPE] = port_descriptors[WAYV_ATTACK];
	port_names[WAYV_OSC2_TYPE] = "Osc 2 Type";
	port_range_hints[WAYV_OSC2_TYPE].HintDescriptor =
			LADSPA_HINT_DEFAULT_MAXIMUM |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_OSC2_TYPE].LowerBound =  0.0f;
	port_range_hints[WAYV_OSC2_TYPE].UpperBound =  2.0f;
        
        
        /*Parameters for osc2pitch*/        
	port_descriptors[WAYV_OSC2_PITCH] = port_descriptors[WAYV_ATTACK];
	port_names[WAYV_OSC2_PITCH] = "Osc 2 Pitch";
	port_range_hints[WAYV_OSC2_PITCH].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_OSC2_PITCH].LowerBound =  -12.0f;
	port_range_hints[WAYV_OSC2_PITCH].UpperBound =  12.0f;
        
        
        /*Parameters for osc2tune*/        
	port_descriptors[WAYV_OSC2_TUNE] = port_descriptors[WAYV_ATTACK];
	port_names[WAYV_OSC2_TUNE] = "Osc 2 Tune";
	port_range_hints[WAYV_OSC2_TUNE].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_OSC2_TUNE].LowerBound = -100.0f;
	port_range_hints[WAYV_OSC2_TUNE].UpperBound = 100.0f; 
        
        
        /*Parameters for osc2vol*/        
	port_descriptors[WAYV_OSC2_VOLUME] = port_descriptors[WAYV_ATTACK];
	port_names[WAYV_OSC2_VOLUME] = "Osc 2 Vol";
	port_range_hints[WAYV_OSC2_VOLUME].HintDescriptor =
			LADSPA_HINT_DEFAULT_MAXIMUM |
			LADSPA_HINT_BOUNDED_BELOW |  LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_OSC2_VOLUME].LowerBound =  -60.0f;
	port_range_hints[WAYV_OSC2_VOLUME].UpperBound =  0.0f;
        
        
        /*Parameters for master vol*/        
	port_descriptors[WAYV_MASTER_VOLUME] = port_descriptors[WAYV_ATTACK];
	port_names[WAYV_MASTER_VOLUME] = "Master Vol";
	port_range_hints[WAYV_MASTER_VOLUME].HintDescriptor =
			LADSPA_HINT_DEFAULT_HIGH |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_MASTER_VOLUME].LowerBound =  -60.0f;
	port_range_hints[WAYV_MASTER_VOLUME].UpperBound =  12.0f;
        
        
        
        /*Parameters for master unison voices*/        
	port_descriptors[WAYV_MASTER_UNISON_VOICES] = port_descriptors[WAYV_ATTACK];
	port_names[WAYV_MASTER_UNISON_VOICES] = "Master Unison";
	port_range_hints[WAYV_MASTER_UNISON_VOICES].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_MASTER_UNISON_VOICES].LowerBound =  1.0f;
	port_range_hints[WAYV_MASTER_UNISON_VOICES].UpperBound =  7.0f;
        
        
        /*Parameters for master unison spread*/        
	port_descriptors[WAYV_MASTER_UNISON_SPREAD] = port_descriptors[WAYV_ATTACK];
	port_names[WAYV_MASTER_UNISON_SPREAD] = "Master Unison Spread";
	port_range_hints[WAYV_MASTER_UNISON_SPREAD].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_MASTER_UNISON_SPREAD].LowerBound =  0.0f;
	port_range_hints[WAYV_MASTER_UNISON_SPREAD].UpperBound =  100.0f;
        
        
        /*Parameters for master glide*/        
	port_descriptors[WAYV_MASTER_GLIDE] = port_descriptors[WAYV_ATTACK];
	port_names[WAYV_MASTER_GLIDE] = "Master Glide";
	port_range_hints[WAYV_MASTER_GLIDE].HintDescriptor =
			LADSPA_HINT_DEFAULT_MINIMUM |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_MASTER_GLIDE].LowerBound =  0.0f;
	port_range_hints[WAYV_MASTER_GLIDE].UpperBound =  200.0f;
        
        
        /*Parameters for master pitchbend amt*/        
	port_descriptors[WAYV_MASTER_PITCHBEND_AMT] = port_descriptors[WAYV_ATTACK];
	port_names[WAYV_MASTER_PITCHBEND_AMT] = "Pitchbend Amt";
	port_range_hints[WAYV_MASTER_PITCHBEND_AMT].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[WAYV_MASTER_PITCHBEND_AMT].LowerBound =  1.0f;
	port_range_hints[WAYV_MASTER_PITCHBEND_AMT].UpperBound =  36.0f;
                        
        /*Parameters for program change*/
        /*
	port_descriptors[LMS_PROGRAM_CHANGE] = port_descriptors[LMS_ATTACK];
	port_names[LMS_PROGRAM_CHANGE] = "Program Change";
	port_range_hints[LMS_PROGRAM_CHANGE].HintDescriptor =
			LADSPA_HINT_DEFAULT_MINIMUM | LADSPA_HINT_BOUNDED_BELOW | 
                        LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PROGRAM_CHANGE].LowerBound = 0; 
	port_range_hints[LMS_PROGRAM_CHANGE].UpperBound = 127;  // > 127 loads the first preset
        */
        
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
