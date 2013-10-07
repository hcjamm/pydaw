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
#include <unistd.h>
#include <math.h>
#include <stdio.h>

#include "../include/pydaw_plugin.h"
#include "../libmodsynth/lib/amp.h"
#include "pydaw.h"
#include "osc_handlers.h"
#include "synth.h"



static PYFX_Descriptor *LMSLDescriptor = NULL;
static PYINST_Descriptor *LMSDDescriptor = NULL;

static t_pydaw_data * pydaw_data;

void v_pydaw_run(PYFX_Handle instance, int sample_count, t_pydaw_seq_event *events, int event_count);

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
    t_pydaw_plugin *instance = 0;
    const char *method;
    unsigned int flen = 0;
    lo_message message;
    lo_address source;    
    char tmp[32];
    
    //printf("\npydaw_osc_message_handler: %s\n\n", path);
    
    if (strncmp(path, "/dssi/", 6))
    {
        printf("\nstrncmp(path, \"/dssi/\", 6)\n\n");
        return pydaw_osc_debug_handler(path, types, argv, argc, data, user_data);
    }
      
    flen = 2;    
    
    for (i = 0; i < PYDAW_MIDI_TRACK_COUNT; i++) 
    {
        sprintf(tmp, "%i-mi", i);
	flen = strlen(tmp);
        if (!strncmp(path + 20, tmp, flen) && *(path + 20 + flen) == '/') //avoid matching prefix only
        {
            instance = pydaw_data->track_pool[i]->instrument; //&instances[i];
            break;
        }
        
        sprintf(tmp, "%i-mfx", i);
	flen = strlen(tmp);
        if (!strncmp(path + 20, tmp, flen) && *(path + 20 + flen) == '/') //avoid matching prefix only
        {
            instance = pydaw_data->track_pool[i]->effect; //&instances[i];
            break;
        }
    }
    
    if(!instance)  //Try the audio tracks
    {        
        for (i = 0; i < PYDAW_AUDIO_TRACK_COUNT; i++) 
        {     
            sprintf(tmp, "%i-afx", i);
            flen = strlen(tmp);
            if (!strncmp(path + 20, tmp, flen) && *(path + 20 + flen) == '/') //avoid matching prefix only
            {
                instance = pydaw_data->audio_track_pool[i]->effect;
                break;
            }
        }
    }
    
    if(!instance)  //Try the bus tracks
    {        
        for (i = 0; i < PYDAW_BUS_TRACK_COUNT; i++) 
        {     
            sprintf(tmp, "%i-bfx", i);
            flen = strlen(tmp);
            if (!strncmp(path + 20, tmp, flen) && *(path + 20 + flen) == '/') //avoid matching prefix only
            {
                instance = pydaw_data->bus_pool[i]->effect;
                break;
            }
        }
    }    
    
    if (!instance)
    {
        printf("\n!instance\n");
        return pydaw_osc_debug_handler(path, types, argv, argc, data, user_data);
    }    
    
    method = path + 20 + flen;
    
    if (*method != '/')
    {
        printf("\n(*method != '/')\n\n%s\n\n", method);
        return pydaw_osc_debug_handler(path, types, argv, argc, data, user_data);
    }
    else if(*(method + 1) == 0)
    {
        printf("\n(*(method + 1) == 0)\n\n%s\n\n", method);
        return pydaw_osc_debug_handler(path, types, argv, argc, data, user_data);
    }
    /*else
    {
        printf("method == %s\n", method);
    }*/
    
    method++;    
    
    message = (lo_message)data;
    source = lo_message_get_source(message);
    
    if (!strcmp(method, "configure") && argc == 2 && !strcmp(types, "ss")) 
    {
        return pydaw_osc_configure_handler(instance, argv);
    } 
    else if (!strcmp(method, "control") && argc == 2 && !strcmp(types, "if")) 
    {
        return pydaw_osc_control_handler(instance, argv);

    }
    else if (!strcmp(method, "update") && argc == 1 && !strcmp(types, "s")) 
    {
        return pydaw_osc_update_handler(instance, argv, source);
    }
    else if (!strcmp(method, "exiting") && argc == 0) 
    {
        return pydaw_osc_exiting_handler(instance, argv);
    }
    else
    {
        printf("Did not match any known method: %s\n", method);
    }
    return pydaw_osc_debug_handler(path, types, argv, argc, data, user_data);
}


__attribute__ ((visibility("default")))
const PYFX_Descriptor *PYFX_descriptor(int index)
{
    switch (index) {
    case 0:
	return LMSLDescriptor;
    default:
	return NULL;
    }
}

__attribute__ ((visibility("default")))
const PYINST_Descriptor *PYINST_descriptor(int index)
{
    switch (index) {
    case 0:
	return LMSDDescriptor;
    default:
	return NULL;
    }
}

static void v_pydaw_cleanup(PYFX_Handle instance)
{
    free(instance);
}

static void v_pydaw_connect_port(PYFX_Handle instance, int port, PYFX_Data * data)
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

static PYFX_Handle g_pydaw_instantiate(const PYFX_Descriptor * descriptor, int s_rate)
{
    t_pydaw_engine *plugin_data = (t_pydaw_engine *) malloc(sizeof(t_pydaw_engine));
    plugin_data->input_arr = (PYFX_Data**)malloc(sizeof(PYFX_Data*) * PYDAW_INPUT_COUNT);
    
    pydaw_data = g_pydaw_data_get(s_rate);
    
    plugin_data->fs = s_rate;
        
    return (PYFX_Handle) plugin_data;
}

void v_pydaw_activate(PYFX_Handle instance, int a_thread_count, int a_set_thread_affinity)
{
    //t_pydaw_engine *plugin_data = (t_pydaw_engine *) instance;    
    v_pydaw_activate_osc_thread(pydaw_data, pydaw_osc_message_handler);
    v_pydaw_init_worker_threads(pydaw_data, a_thread_count, a_set_thread_affinity);
    //v_pydaw_init_busses(pydaw_data);
    v_open_default_project(pydaw_data);
}

/*
static void runLMSWrapper(PYFX_Handle instance, int sample_count)
{
    run_lms_pydaw(instance, sample_count, NULL, 0);
}
*/

void v_pydaw_run(PYFX_Handle instance, int sample_count, t_pydaw_seq_event *events, int event_count)
{
    t_pydaw_engine *plugin_data = (t_pydaw_engine *) instance;
               
    int f_lock_result = pthread_mutex_trylock(&pydaw_data->offline_mutex);
    
    if(f_lock_result == 0)  //Don't try to process the main loop if another process, ie:  offline rendering of a project, has locked it
    {
        pthread_mutex_lock(&pydaw_data->main_mutex);
        pydaw_data->input_buffers_active = 1;
        long f_next_current_sample = ((pydaw_data->current_sample) + sample_count);
        v_pydaw_run_main_loop(pydaw_data, sample_count, events, event_count, f_next_current_sample, 
                plugin_data->output0, plugin_data->output1, plugin_data->input_arr);
        /*TODO:  Run the LMS Limiter algorithm here at 0.0db, long release time, to prevent clipping*/
        
        pydaw_data->current_sample = f_next_current_sample;
        
        pthread_mutex_unlock(&pydaw_data->main_mutex);
        pthread_mutex_unlock(&pydaw_data->offline_mutex);
    }
    else
    {
        /*Clear the output buffer*/
        pydaw_data->input_buffers_active = 0;
        plugin_data->i_buffer_clear = 0;    
        
        while((plugin_data->i_buffer_clear) < sample_count)
        {
            plugin_data->output0[(plugin_data->i_buffer_clear)] = 0.0f;
            plugin_data->output1[(plugin_data->i_buffer_clear)] = 0.0f;
            plugin_data->i_buffer_clear = (plugin_data->i_buffer_clear) + 1;
        }
    }
}


void v_pydaw_constructor()
{
    char **port_names;
    PYFX_PortDescriptor *port_descriptors;
    PYFX_PortRangeHint *port_range_hints;

    LMSLDescriptor =
	(PYFX_Descriptor *) malloc(sizeof(PYFX_Descriptor));
    if (LMSLDescriptor) {
	LMSLDescriptor->Copyright = "GNU GPL v3";
	LMSLDescriptor->PortCount = PYDAW_COUNT;

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

        /* Parameters for input */
        int f_i;
        
        for(f_i = PYDAW_INPUT_MIN; f_i < PYDAW_INPUT_MAX; f_i++)
        {
            port_descriptors[f_i] = PYFX_PORT_INPUT | PYFX_PORT_AUDIO;
            port_names[f_i] = "Input ";  //TODO:  Give a more descriptive port name
            //port_range_hints[f_i].HintDescriptor = 0;
        }
                
	/* Parameters for output */
	port_descriptors[PYDAW_OUTPUT0] = PYFX_PORT_OUTPUT | PYFX_PORT_AUDIO;
	port_names[PYDAW_OUTPUT0] = "Output 0";
	//port_range_hints[PYDAW_OUTPUT0].HintDescriptor = 0;

        port_descriptors[PYDAW_OUTPUT1] = PYFX_PORT_OUTPUT | PYFX_PORT_AUDIO;
	port_names[PYDAW_OUTPUT1] = "Output 1";
	//port_range_hints[PYDAW_OUTPUT1].HintDescriptor = 0;
        	
    }

    LMSDDescriptor = (PYINST_Descriptor *) malloc(sizeof(PYINST_Descriptor));
    if (LMSDDescriptor) 
    {
	LMSDDescriptor->PYFX_Plugin = LMSLDescriptor;	
    }
}


void v_pydaw_destructor()
{
    if(pydaw_data)
    {
        pydaw_data->audio_recording_quit_notifier = 1;        
        lo_address_free(pydaw_data->uiTarget);
        
        //This ensures that the destructor doesn't start freeing memory before save-on-exit finishes...
        pthread_mutex_lock(&pydaw_data->quit_mutex);
        pthread_mutex_unlock(&pydaw_data->quit_mutex);
        
        int f_i = 0;

        char tmp_sndfile_name[2048];
        
        while(f_i < PYDAW_AUDIO_INPUT_TRACK_COUNT)
        {
            if(pydaw_data->audio_inputs[f_i]->sndfile)
            {
                sf_close(pydaw_data->audio_inputs[f_i]->sndfile);
                sprintf(tmp_sndfile_name, "%s%i.wav", pydaw_data->audio_tmp_folder, f_i);
                printf("Deleting %s\n", tmp_sndfile_name);
                remove(tmp_sndfile_name);
            }
            f_i++;
        }
        
        f_i = 1;
        while(f_i < pydaw_data->track_worker_thread_count)
        {
            //pthread_mutex_lock(&pydaw_data->track_block_mutexes[f_i]);        
            pydaw_data->track_thread_quit_notifier[f_i] = 1;        
            //pthread_mutex_unlock(&pydaw_data->track_block_mutexes[f_i]);
            f_i++;
        }
        
        f_i = 1;
        while(f_i < pydaw_data->track_worker_thread_count)
        {
            pthread_mutex_lock(&pydaw_data->track_block_mutexes[f_i]);
            pthread_cond_broadcast(&pydaw_data->track_cond[f_i]);
            pthread_mutex_unlock(&pydaw_data->track_block_mutexes[f_i]);
            f_i++;
        }

        sleep(1);
        
        f_i = 1;
        while(f_i < pydaw_data->track_worker_thread_count)
        {
            assert(pydaw_data->track_thread_quit_notifier[f_i] == 2);  //abort the application rather than hang indefinitely
            //pthread_mutex_lock(&pydaw_data->track_block_mutexes[f_i]);
            //pthread_mutex_unlock(&pydaw_data->track_block_mutexes[f_i]);
            f_i++;
        }
    }
        
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
