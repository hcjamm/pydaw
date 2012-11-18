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
    
    //printf("\npydaw_osc_message_handler: %s\n\n", path);
    
    if (strncmp(path, "/dssi/", 6))
    {
        printf("\nstrncmp(path, \"/dssi/\", 6)\n\n");
        return pydaw_osc_debug_handler(path, types, argv, argc, data, user_data);
    }
      
    flen = 2;
    
    
    for (i = 0; i < PYDAW_MAX_TRACK_COUNT; i++) 
    {
        sprintf(tmp, "%i", i);
	flen = strlen(tmp);
        if (!strncmp(path + 20, tmp, flen)
	    && *(path + 20 + flen) == '/') //avoid matching prefix only
        { 
            //printf("instance==%i\n", i);
            instance = pydaw_data->track_pool[i]->instrument; //&instances[i];
            break;
        }
        
        sprintf(tmp, "%i-fx", i);
	flen = strlen(tmp);
        if (!strncmp(path + 20, tmp, flen)
	    && *(path + 20 + flen) == '/') //avoid matching prefix only
        { 
            //printf("instance==%i\n", i);
            instance = pydaw_data->track_pool[i]->effect; //&instances[i];
            break;
        }
    }
    
    
    if (!instance)
    {
        printf("\n!instance\n");
        return pydaw_osc_debug_handler(path, types, argv, argc, data, user_data);
    }
    
    method = path + 20 + flen;
    if (*method != '/' || *(method + 1) == 0)
    {
        printf("\n(*method != '/' || *(method + 1) == 0)\n\n%s\n\n", method);
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
    else if (!strcmp(method, "exiting") && argc == 0) 
    {
        return pydaw_osc_exiting_handler(instance, argv);
    }
        
    return pydaw_osc_debug_handler(path, types, argv, argc, data, user_data);
}

static void v_pydaw_run(LADSPA_Handle instance, unsigned long sample_count,
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

static void v_pydaw_cleanup(LADSPA_Handle instance)
{
    free(instance);
}

static void v_pydaw_connect_port(LADSPA_Handle instance, unsigned long port, LADSPA_Data * data)
{
    t_pydaw_engine *plugin;

    plugin = (t_pydaw_engine *) instance;
        
    if((port >= PYDAW_INPUT_MIN) && (port < PYDAW_INPUT_MAX))
    {
        plugin->input_arr[(port - PYDAW_INPUT_MIN)] = data;
    }
    else
    {        
        switch (port) 
        {     
            case PYDAW_OUTPUT0: plugin->output0 = data; break;
            case PYDAW_OUTPUT1: plugin->output1 = data; break;        
        }
    }    
}

static LADSPA_Handle g_pydaw_instantiate(const LADSPA_Descriptor * descriptor, unsigned long s_rate)
{
    t_pydaw_engine *plugin_data = (t_pydaw_engine *) malloc(sizeof(t_pydaw_engine));
    pydaw_data = g_pydaw_data_get(s_rate);
    
    plugin_data->fs = s_rate;
        
    return (LADSPA_Handle) plugin_data;
}

static void v_pydaw_activate(LADSPA_Handle instance)
{
    t_pydaw_engine *plugin_data = (t_pydaw_engine *) instance;        
    plugin_data->mono_modules = v_mono_init((plugin_data->fs));  
    v_pydaw_activate_osc_thread(pydaw_data, pydaw_osc_message_handler);
    v_pydaw_init_worker_threads(pydaw_data);
    v_open_default_project(pydaw_data);
}

/*
static void runLMSWrapper(LADSPA_Handle instance, unsigned long sample_count)
{
    run_lms_pydaw(instance, sample_count, NULL, 0);
}
*/

static void v_pydaw_run(LADSPA_Handle instance, unsigned long sample_count, snd_seq_event_t *events, unsigned long event_count)
{
    t_pydaw_engine *plugin_data = (t_pydaw_engine *) instance;
    
    plugin_data->i_buffer_clear = 0;    
    /*Clear the output buffer*/
    while((plugin_data->i_buffer_clear) < sample_count)  //TODO:  Consider memset'ing???
    {
        //output0[(plugin_data->i_buffer_clear)] = 0.0f;
        //output1[(plugin_data->i_buffer_clear)] = 0.0f;
        plugin_data->output0[(plugin_data->i_buffer_clear)] = 0.0f;
        plugin_data->output1[(plugin_data->i_buffer_clear)] = 0.0f;
        plugin_data->i_buffer_clear = (plugin_data->i_buffer_clear) + 1;
    }
        
    int f_lock_result = pthread_mutex_trylock(&pydaw_data->offline_mutex);
    
    if(f_lock_result == 0)  //Don't try to process the main loop if another process, ie:  offline rendering of a project, has locked it
    {
        pthread_mutex_lock(&pydaw_data->main_mutex);
        long f_next_current_sample = ((pydaw_data->current_sample) + sample_count);
        v_pydaw_run_main_loop(pydaw_data, sample_count, events, event_count, f_next_current_sample, plugin_data->output0, plugin_data->output1);
        /*TODO:  Run the LMS Limiter algorithm here at 0.0db, long release time, to prevent clipping*/
        
        pydaw_data->current_sample = f_next_current_sample;
        
        pthread_mutex_unlock(&pydaw_data->main_mutex);
        pthread_mutex_unlock(&pydaw_data->offline_mutex);
    }
    else
    {
        //printf("offline_mutex unavailable, not running main loop...\n");
    }
}

char *c_pydaw_configure(LADSPA_Handle instance, const char *key, const char *value)
{
    //t_pydaw_engine *plugin_data = (t_pydaw_engine *)instance;
    v_pydaw_parse_configure_message(pydaw_data, key, value);
        
    return NULL;
}

int i_pydaw_get_controller(LADSPA_Handle instance, unsigned long port)
{    
    //t_pydaw_engine *plugin_data = (t_pydaw_engine *) instance;
    //return DSSI_CC(i_ccm_get_cc(plugin_data->midi_cc_map, port));
    return -1;     
}

#ifdef __GNUC__
__attribute__((constructor)) void v_pydaw_constructor()
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
        LMSLDescriptor->UniqueID = PYDAW_PLUGIN_UUID;
	LMSLDescriptor->Label = PYDAW_PLUGIN_NAME;
	LMSLDescriptor->Properties = LADSPA_PROPERTY_REALTIME;
	LMSLDescriptor->Name = PYDAW_PLUGIN_LONG_NAME;
	LMSLDescriptor->Maker = PYDAW_PLUGIN_DEV;
	LMSLDescriptor->Copyright = "GNU GPL v3";
	LMSLDescriptor->PortCount = PYDAW_COUNT;

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
        
        for(f_i = PYDAW_INPUT_MIN; f_i < PYDAW_INPUT_MAX; f_i++)
        {
            port_descriptors[f_i] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
            port_names[f_i] = "Input ";  //TODO:  Give a more descriptive port name
            port_range_hints[f_i].HintDescriptor = 0;
        }
        
	/* Parameters for output */
	port_descriptors[PYDAW_OUTPUT0] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
	port_names[PYDAW_OUTPUT0] = "Output 0";
	port_range_hints[PYDAW_OUTPUT0].HintDescriptor = 0;

        port_descriptors[PYDAW_OUTPUT1] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
	port_names[PYDAW_OUTPUT1] = "Output 1";
	port_range_hints[PYDAW_OUTPUT1].HintDescriptor = 0;
               
        
	LMSLDescriptor->activate = v_pydaw_activate;
	LMSLDescriptor->cleanup = v_pydaw_cleanup;
	LMSLDescriptor->connect_port = v_pydaw_connect_port;
	LMSLDescriptor->deactivate = NULL;
	LMSLDescriptor->instantiate = g_pydaw_instantiate;
	LMSLDescriptor->run = NULL;
	LMSLDescriptor->run_adding = NULL;
	LMSLDescriptor->set_run_adding_gain = NULL;
    }

    LMSDDescriptor = (DSSI_Descriptor *) malloc(sizeof(DSSI_Descriptor));
    if (LMSDDescriptor) {
	LMSDDescriptor->DSSI_API_Version = 1;
	LMSDDescriptor->LADSPA_Plugin = LMSLDescriptor;
	LMSDDescriptor->configure = c_pydaw_configure;
	LMSDDescriptor->get_program = NULL;
	LMSDDescriptor->get_midi_controller_for_port = i_pydaw_get_controller;
	LMSDDescriptor->select_program = NULL;
	LMSDDescriptor->run_synth = v_pydaw_run;
	LMSDDescriptor->run_synth_adding = NULL;
	LMSDDescriptor->run_multiple_synths = NULL;
	LMSDDescriptor->run_multiple_synths_adding = NULL;
    }
}

#ifdef __GNUC__
__attribute__((destructor)) void v_pydaw_destructor()
#else
void _fini()
#endif
{
    if(pydaw_data)
    {
        v_pydaw_close_all_uis(pydaw_data);

        sleep(3);

        //This ensures that the destructor doesn't start freeing memory before save-on-exit finishes...
        pthread_mutex_lock(&pydaw_data->quit_mutex);
        pthread_mutex_unlock(&pydaw_data->quit_mutex);

        int f_i = 0;
        while(f_i < pydaw_data->track_worker_thread_count)
        {
            pthread_mutex_lock(&pydaw_data->track_block_mutexes[f_i]);        
            pydaw_data->track_thread_quit_notifier[f_i] = 1;        
            pthread_mutex_unlock(&pydaw_data->track_block_mutexes[f_i]);
            f_i++;
        }

        pthread_mutex_lock(&pydaw_data->track_cond_mutex);
        pthread_cond_broadcast(&pydaw_data->track_cond);
        pthread_mutex_unlock(&pydaw_data->track_cond_mutex);

        f_i = 0;
        while(f_i < pydaw_data->track_worker_thread_count)
        {
            pthread_mutex_lock(&pydaw_data->track_block_mutexes[f_i]);
            pthread_mutex_unlock(&pydaw_data->track_block_mutexes[f_i]);
            f_i++;
        }
    }
        
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
