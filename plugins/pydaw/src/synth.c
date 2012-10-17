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
#include "pydaw.h"
#include "osc_handlers.h"
#include "synth.h"
#include "meta.h"

#include <unistd.h>
#include <alsa/asoundlib.h>

static LADSPA_Descriptor *LMSLDescriptor = NULL;
static DSSI_Descriptor *LMSDDescriptor = NULL;

static t_pydaw_data * pydaw_data;

int pydaw_osc_debug_handler(const char *path, const char *types, lo_arg **argv,
                      int argc, void *data, void *user_data)
{
    int i;

    printf("PyDAW: got unhandled OSC message:\npath: <%s>\n", path);
    for (i=0; i<argc; i++) 
    {
        printf("PyDAW: arg %d '%c' ", i, types[i]);
        lo_arg_pp(types[i], argv[i]);
        printf("\n");
    }
    
    return 1;
}

int pydaw_osc_message_handler(const char *path, const char *types, lo_arg **argv,
                        int argc, void *data, void *user_data)
{
    int i;
    t_pydaw_plugin *instance;
    const char *method;
    unsigned int flen = 0;
    lo_message message;
    lo_address source;
    int send_to_ui = 0;
    char tmp[6];
    
    printf("pydaw_osc_message_handler: %s\n", path);
    
    if (strncmp(path, "/dssi/", 6))
    {
        return pydaw_osc_debug_handler(path, types, argv, argc, data, user_data);
    }
      
    flen = 2;
    
    
    for (i = 0; i < PYDAW_MAX_TRACK_COUNT; i++) 
    {
        sprintf(tmp, "%i", i);
	flen = strlen(tmp);
        if (!strncmp(path + 6, tmp, flen)
	    && *(path + 6 + flen) == '/') //avoid matching prefix only
        { 
            instance = pydaw_data->track_pool[i]->instrument; //&instances[i];
            break;
        }
    }
    
    
    if (!instance)
    {
        return pydaw_osc_debug_handler(path, types, argv, argc, data, user_data);
    }
    
    method = path + 6 + flen;
    if (*method != '/' || *(method + 1) == 0)
    {
        return pydaw_osc_debug_handler(path, types, argv, argc, data, user_data);
    }
    
    method++;

    message = (lo_message)data;
    source = lo_message_get_source(message);

    if (instance->uiSource && instance->uiTarget) 
    {
	if (strcmp(lo_address_get_hostname(source),lo_address_get_hostname(instance->uiSource)) 
                ||
	    strcmp(lo_address_get_port(source), lo_address_get_port(instance->uiSource))) 
        {
	    /* This didn't come from our known UI for this plugin, so send an update to that as well */
	    send_to_ui = 1;
	}
    }
    
    if (!strcmp(method, "configure") && argc == 2 && !strcmp(types, "ss")) 
    {
	if (send_to_ui) 
        {
	    lo_send(instance->uiTarget, instance->ui_osc_configure_path, "ss",
		    &argv[0]->s, &argv[1]->s);
	}

        return pydaw_osc_configure_handler(instance, argv);
    } 
    else if (!strcmp(method, "control") && argc == 2 && !strcmp(types, "if")) 
    {
	if (send_to_ui) 
        {
	    lo_send(instance->uiTarget, instance->ui_osc_control_path, "if",
		    argv[0]->i, argv[1]->f);
	}

        return pydaw_osc_control_handler(instance, argv);

    } 
    /*else if (!strcmp(method, "midi") && argc == 1 && !strcmp(types, "m")) 
    {
        return pydaw_osc_midi_handler(instance, argv);
    } */
    else if (!strcmp(method, "update") && argc == 1 && !strcmp(types, "s")) 
    {
        return pydaw_osc_update_handler(instance, argv, source);
    }
    /*
    else if (!strcmp(method, "exiting") && argc == 0) 
    {
        return pydaw_osc_exiting_handler(instance, argv);
    }
    */
    
    return pydaw_osc_debug_handler(path, types, argv, argc, data, user_data);
}

static void run_lms_pydaw(LADSPA_Handle instance, unsigned long sample_count,
		  snd_seq_event_t * events, unsigned long EventCount);


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
    t_pydaw_engine *plugin;

    plugin = (t_pydaw_engine *) instance;
        
    if((port >= LMS_INPUT_MIN) && (port < LMS_INPUT_MAX))
    {
        plugin->input_arr[(port - LMS_INPUT_MIN)] = data;
    }
    else
    {        
        switch (port) 
        {     
            case LMS_OUTPUT0: plugin->output0 = data; break;
            case LMS_OUTPUT1: plugin->output1 = data; break;        
        }
    }    
}

static LADSPA_Handle instantiateLMS(const LADSPA_Descriptor * descriptor,
				   unsigned long s_rate)
{
    t_pydaw_engine *plugin_data = (t_pydaw_engine *) malloc(sizeof(t_pydaw_engine));
    pydaw_data = g_pydaw_data_get(s_rate);
    
    plugin_data->fs = s_rate;
    
    v_init_lms(s_rate);
    
    return (LADSPA_Handle) plugin_data;
}

static void activateLMS(LADSPA_Handle instance)
{
    t_pydaw_engine *plugin_data = (t_pydaw_engine *) instance;        
    plugin_data->mono_modules = v_mono_init((plugin_data->fs));  
    v_pydaw_activate_osc_thread(pydaw_data, pydaw_osc_message_handler);
}

static void runLMSWrapper(LADSPA_Handle instance,
			 unsigned long sample_count)
{
    run_lms_pydaw(instance, sample_count, NULL, 0);
}


static void run_lms_pydaw(LADSPA_Handle instance, unsigned long sample_count,
		  snd_seq_event_t *events, unsigned long event_count)
{
    t_pydaw_engine *plugin_data = (t_pydaw_engine *) instance;
    /*Define our inputs*/
    
    /*define our outputs*/
    LADSPA_Data *const output0 = plugin_data->output0;    
    LADSPA_Data *const output1 = plugin_data->output1;
    
    plugin_data->i_buffer_clear = 0;
    /*Clear the output buffer*/
    
    while((plugin_data->i_buffer_clear) < sample_count)  //TODO:  Consider memset'ing???
    {
        output0[(plugin_data->i_buffer_clear)] = 0.0f;                        
        output1[(plugin_data->i_buffer_clear)] = 0.0f;     
        plugin_data->i_buffer_clear = (plugin_data->i_buffer_clear) + 1;
    }
    
    
    int f_i = 0;
    
    pthread_mutex_lock(&pydaw_data->mutex);
    
    double f_next_period = (pydaw_data->playback_cursor) + ((pydaw_data->playback_inc) * ((double)(sample_count)));    
    int f_next_current_sample = ((pydaw_data->current_sample) + sample_count);
       
    if((pydaw_data->is_initialized) && ((pydaw_data->playback_mode) > 0))
    {                
        //event_loop_label:
                
        double f_current_period_beats = (pydaw_data->playback_cursor) * 4.0f;
        double f_next_period_beats = f_next_period * 4.0f;
                
        while(f_i < PYDAW_MAX_TRACK_COUNT)
        {
            
            /* TODO:
             * 1.  Figure out how to determine which tick of the period to send an event on from the given fractional bar
             * 2.  A next/last fractional bar, that possible event firings must begin between
             * 3.  Persistent note/cc event list iterators
             * 4.  Check if (f_next_period >= 1.0f) earlier, then begin iterating the item.  Until then, there will be issues with timing and missed events
             * 5.  Calculate note_offs, which currently are not calculated.
             */
         
            if(pydaw_data->track_pool[f_i]->plugin_index == 0)
            {
                f_i++;
                continue;
            }
            
            pydaw_data->track_pool[f_i]->event_index = 0;
            
            if((pydaw_data->pysong->regions[(pydaw_data->current_region)]) && (pydaw_data->pysong->regions[(pydaw_data->current_region)]->item_populated[f_i][(pydaw_data->current_bar)]))
            {                
                t_pyitem f_current_item = *(pydaw_data->pysong->regions[(pydaw_data->current_region)]->items[f_i][(pydaw_data->current_bar)]);
                
                while(1)
                {
                    if((pydaw_data->track_note_event_indexes[f_i]) >= (f_current_item.note_count))
                    {
                        break;
                    }

                    if(((f_current_item.notes[(pydaw_data->track_note_event_indexes[f_i])]->start) >= f_current_period_beats) &&
                        ((f_current_item.notes[(pydaw_data->track_note_event_indexes[f_i])]->start) < f_next_period_beats))
                    {
                        printf("Sending note_on event\n");
                        
                        snd_seq_ev_clear(&pydaw_data->track_pool[f_i]->event_buffer[(pydaw_data->track_pool[f_i]->event_index)]);
                        
                        snd_seq_ev_set_noteon(&pydaw_data->track_pool[f_i]->event_buffer[(pydaw_data->track_pool[f_i]->event_index)], 0,
                                f_current_item.notes[(pydaw_data->track_note_event_indexes[f_i])]->note,
                                f_current_item.notes[(pydaw_data->track_note_event_indexes[f_i])]->velocity);
                        
                        pydaw_data->track_pool[f_i]->event_index = (pydaw_data->track_pool[f_i]->event_index) + 1;
                                                                        
                        pydaw_data->note_offs[f_i][(f_current_item.notes[(pydaw_data->track_note_event_indexes[f_i])]->note)] = (pydaw_data->current_sample) + 5000;  //TODO:  replace 5000 with a real calculated length
                                                
                        pydaw_data->track_note_event_indexes[f_i] = (pydaw_data->track_note_event_indexes[f_i]) + 1;
                    }
                    else
                    {
                        break;
                    }
                }                
                
                /*
                while(1)  //CCs???
                {

                }
                */   
                                 
                int f_i2 = 0;
                //This is going to be painfully slow scanning all 16x128=2048 entries, but it may not really matter, so I won't prematurely optimize it now.
                while(f_i2 < PYDAW_MIDI_NOTE_COUNT)
                {
                    if((pydaw_data->note_offs[f_i][f_i2]) >= (pydaw_data->current_sample) &&
                       (pydaw_data->note_offs[f_i][f_i2]) < f_next_current_sample)
                    {
                        printf("sending note_off\n");
                        snd_seq_ev_clear(&pydaw_data->track_pool[f_i]->event_buffer[(pydaw_data->track_pool[f_i]->event_index)]);
                        
                        snd_seq_ev_set_noteoff(&pydaw_data->track_pool[f_i]->event_buffer[(pydaw_data->track_pool[f_i]->event_index)], 0, f_i2, 0);                        
                        
                        pydaw_data->track_pool[f_i]->event_index = (pydaw_data->track_pool[f_i]->event_index) + 1;
                    }

                    f_i2++;
                }                
                v_run_plugin(pydaw_data->track_pool[f_i]->instrument, sample_count, 
                        pydaw_data->track_pool[f_i]->event_buffer, pydaw_data->track_pool[f_i]->event_index);
                
                
                //if(pydaw_data->track_pool[f_i]->instrument)
                //{
                    int f_i3 = 0;

                    while(f_i3 < sample_count)
                    {
                        output0[f_i3] += (pydaw_data->track_pool[f_i]->instrument->pluginOutputBuffers[0][f_i3]);
                        output1[f_i3] += (pydaw_data->track_pool[f_i]->instrument->pluginOutputBuffers[1][f_i3]);
                        f_i3++;
                    }
                //}
            }
        
            f_i++;
        }
        
        pydaw_data->playback_cursor = f_next_period;
        
        if((pydaw_data->playback_cursor) >= 1.0f)
        {
            //Calculate the remainder of this bar that occurs within the sample period
            //pydaw_data->playback_cursor = (pydaw_data->playback_cursor) - 1.0f;
            pydaw_data->playback_cursor = 0.0f;
            f_next_period = (pydaw_data->playback_cursor) + ((pydaw_data->playback_inc) * ((double)(sample_count)));
            
            int f_i2 = 0;
            
            while(f_i2 < PYDAW_MAX_TRACK_COUNT)
            {
                pydaw_data->track_note_event_indexes[f_i2] = 0;
                pydaw_data->track_cc_event_indexes[f_i2] = 0;
                f_i2++;
            }
            
            if(pydaw_data->loop_mode != PYDAW_LOOP_MODE_BAR)
            {
                pydaw_data->current_bar = (pydaw_data->current_bar) + 1;
                
                if((pydaw_data->current_bar) >= PYDAW_REGION_SIZE)
                {
                    pydaw_data->current_bar = 0;
                    
                    if(pydaw_data->loop_mode != PYDAW_LOOP_MODE_REGION)
                    {
                        pydaw_data->current_region = (pydaw_data->current_region) + 1;
                        
                        if((pydaw_data->current_region) >= PYDAW_MAX_REGION_COUNT)
                        {
                            pydaw_data->playback_mode = 0;
                            pydaw_data->current_region = 0;
                        }
                    }
                }
            }
            
            printf("pydaw_data->current_region == %i, pydaw_data->current_bar == %i\n", (pydaw_data->current_region), (pydaw_data->current_bar));            
            //Use this to go back and process the early parts of the next item
            //goto event_loop_label;
        }
        
        pydaw_data->current_sample = f_next_current_sample;
    }
    pthread_mutex_unlock(&pydaw_data->mutex);
    
    //Mix together the audio input channels from the plugins
    
        
    /*
    int f_i_mix = 0;
    while(f_i_mix < PYDAW_MAX_TRACK_COUNT)
    {
        int f_i2 = f_i_mix + 1;
        plugin_data->i_mono_out = 0;
        
        while((plugin_data->i_mono_out) < sample_count)
        {
            output0[(plugin_data->i_mono_out)] += *(plugin_data->input_arr[f_i_mix]);
            output1[(plugin_data->i_mono_out)] += *(plugin_data->input_arr[f_i2]);

            plugin_data->i_mono_out = (plugin_data->i_mono_out) + 1;
        }
        f_i_mix += 2;
        
    }
    */
}

char *pydaw_configure(LADSPA_Handle instance, const char *key, const char *value)
{
    //t_pydaw_engine *plugin_data = (t_pydaw_engine *)instance;
    v_pydaw_parse_configure_message(pydaw_data, key, value);
        
    return NULL;
}

int getControllerLMS(LADSPA_Handle instance, unsigned long port)
{    
    //t_pydaw_engine *plugin_data = (t_pydaw_engine *) instance;
    //return DSSI_CC(i_ccm_get_cc(plugin_data->midi_cc_map, port));
    return 0;     
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

    LMSLDescriptor =
	(LADSPA_Descriptor *) malloc(sizeof(LADSPA_Descriptor));
    if (LMSLDescriptor) {
        LMSLDescriptor->UniqueID = LMS_PLUGIN_UUID;
	LMSLDescriptor->Label = LMS_PLUGIN_NAME;
	LMSLDescriptor->Properties = LADSPA_PROPERTY_REALTIME;
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

        /* Parameters for input */
        int f_i;
        
        for(f_i = LMS_INPUT_MIN; f_i < LMS_INPUT_MAX; f_i++)
        {
            port_descriptors[f_i] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
            port_names[f_i] = "Input ";  //TODO:  Give a more descriptive port name
            port_range_hints[f_i].HintDescriptor = 0;
        }
        
	/* Parameters for output */
	port_descriptors[LMS_OUTPUT0] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
	port_names[LMS_OUTPUT0] = "Output 0";
	port_range_hints[LMS_OUTPUT0].HintDescriptor = 0;

        port_descriptors[LMS_OUTPUT1] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
	port_names[LMS_OUTPUT1] = "Output 1";
	port_range_hints[LMS_OUTPUT1].HintDescriptor = 0;
               
        
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
	LMSDDescriptor->configure = pydaw_configure;
	LMSDDescriptor->get_program = NULL;
	LMSDDescriptor->get_midi_controller_for_port = getControllerLMS;
	LMSDDescriptor->select_program = NULL;
	LMSDDescriptor->run_synth = NULL;
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
