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
#include "synth.h"



static PYFX_Descriptor *LMSLDescriptor = NULL;

void v_pydaw_run(PYFX_Handle instance, int sample_count,
        t_pydaw_seq_event *events, int event_count);

PYFX_Descriptor *PYFX_descriptor(int index)
{
    switch (index) {
    case 0:
	return LMSLDescriptor;
    default:
	return NULL;
    }
}

static void v_pydaw_cleanup(PYFX_Handle instance)
{
    free(instance);
}

static void v_pydaw_connect_port(PYFX_Handle instance, int port,
        PYFX_Data * data)
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

static PYFX_Handle g_pydaw_instantiate(PYFX_Descriptor * descriptor, int s_rate)
{
    t_pydaw_engine *plugin_data =
            (t_pydaw_engine *) malloc(sizeof(t_pydaw_engine));
    plugin_data->input_arr =
            (PYFX_Data**)malloc(sizeof(PYFX_Data*) * PYDAW_INPUT_COUNT);

    pydaw_data = g_pydaw_data_get(s_rate);

    plugin_data->fs = s_rate;

    return (PYFX_Handle) plugin_data;
}

void v_pydaw_activate(PYFX_Handle instance, int a_thread_count,
        int a_set_thread_affinity, char * a_project_path)
{
    v_open_project(pydaw_data, a_project_path, 1);

    v_pydaw_init_worker_threads(pydaw_data, a_thread_count,
            a_set_thread_affinity);
}

void v_pydaw_run(PYFX_Handle instance, int sample_count,
        t_pydaw_seq_event *events, int event_count)
{
    t_pydaw_engine *plugin_data = (t_pydaw_engine *) instance;

    pthread_spin_lock(&pydaw_data->main_lock);

    if(!pydaw_data->is_offline_rendering)
    {
        pydaw_data->input_buffers_active = 1;
        long f_next_current_sample =
            ((pydaw_data->current_sample) + sample_count);
        v_pydaw_run_main_loop(
                pydaw_data, sample_count, events, event_count,
                f_next_current_sample,
                plugin_data->output0, plugin_data->output1,
                plugin_data->input_arr);
        pydaw_data->current_sample = f_next_current_sample;
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

    pthread_spin_unlock(&pydaw_data->main_lock);
}


void v_pydaw_constructor()
{
    PYFX_PortDescriptor *port_descriptors;
    PYFX_PortRangeHint *port_range_hints;

    LMSLDescriptor =
	(PYFX_Descriptor *) malloc(sizeof(PYFX_Descriptor));
    if (LMSLDescriptor) {
        LMSLDescriptor->PortCount = PYDAW_COUNT;
	port_descriptors = (PYFX_PortDescriptor *)
				calloc(LMSLDescriptor->PortCount, sizeof
						(PYFX_PortDescriptor));
	LMSLDescriptor->PortDescriptors =
	    (PYFX_PortDescriptor *) port_descriptors;

	port_range_hints = (PYFX_PortRangeHint *)
				calloc(LMSLDescriptor->PortCount, sizeof
						(PYFX_PortRangeHint));
	LMSLDescriptor->PortRangeHints =
	    (PYFX_PortRangeHint *) port_range_hints;

        /* Parameters for input */
        int f_i;

        for(f_i = PYDAW_INPUT_MIN; f_i < PYDAW_INPUT_MAX; f_i++)
        {
            port_descriptors[f_i] = 1;
            //port_range_hints[f_i].HintDescriptor = 0;
        }
    }
}


void v_pydaw_destructor()
{
    if(pydaw_data)
    {
        pydaw_data->audio_recording_quit_notifier = 1;
        lo_address_free(pydaw_data->uiTarget);

        int f_i = 0;

        char tmp_sndfile_name[2048];

        while(f_i < PYDAW_AUDIO_INPUT_TRACK_COUNT)
        {
            if(pydaw_data->audio_inputs[f_i]->sndfile)
            {
                sf_close(pydaw_data->audio_inputs[f_i]->sndfile);
                sprintf(tmp_sndfile_name, "%s%i.wav",
                        pydaw_data->audio_tmp_folder, f_i);
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
            //abort the application rather than hang indefinitely
            assert(pydaw_data->track_thread_quit_notifier[f_i] == 2);
            //pthread_mutex_lock(&pydaw_data->track_block_mutexes[f_i]);
            //pthread_mutex_unlock(&pydaw_data->track_block_mutexes[f_i]);
            f_i++;
        }
    }

    if (LMSLDescriptor) {
	free((PYFX_PortDescriptor *) LMSLDescriptor->PortDescriptors);
	free((PYFX_PortRangeHint *) LMSLDescriptor->PortRangeHints);
	free(LMSLDescriptor);
    }
}
