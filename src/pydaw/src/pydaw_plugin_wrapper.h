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

#ifndef PYDAW_PLUGIN_H
#define	PYDAW_PLUGIN_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "../include/pydaw_plugin.h"
#include "pydaw.h"
#include <stdlib.h>
#include <assert.h>

#include "../plugins/modulex/synth.c"
#include "../plugins/euphoria/synth.c"
#include "../plugins/way_v/synth.c"
#include "../plugins/ray_v/synth.c"


PYFX_Data g_pydaw_get_port_default(PYFX_Descriptor *plugin, int port)
{
    PYFX_PortRangeHint hint = plugin->PortRangeHints[port];
    assert(hint.DefaultValue <= hint.UpperBound && hint.DefaultValue >= hint.LowerBound );
    return hint.DefaultValue;
}

typedef struct st_pydaw_plugin
{
    //void * lib_handle;
    PYINST_Descriptor_Function descfn;
    PYFX_Handle PYFX_handle;

    PYINST_Descriptor *descriptor;
    float **pluginOutputBuffers;
    float *pluginControlIns;
}t_pydaw_plugin;


int v_pydaw_plugin_configure_handler(t_pydaw_plugin *instance, char *key, char *value, pthread_mutex_t * a_mutex)
{
    char * message = 0;

    if (instance->descriptor->configure)
    {
        message = instance->descriptor->configure(instance->PYFX_handle, key, value, a_mutex);
        if (message)
        {
            printf("PyDAW: on configure '%s' '%s', plugin returned error '%s'\n", key, value, message);
            free(message);
        }
    }

    return 0;
}


t_pydaw_plugin * g_pydaw_plugin_get(int a_sample_rate, int a_index,
        fp_get_wavpool_item_from_host a_host_wavpool_func,
        int a_track_num, fp_queue_message a_queue_func)
{
    t_pydaw_plugin * f_result = (t_pydaw_plugin*)malloc(sizeof(t_pydaw_plugin));  //TODO: posix_memalign instead...

    switch(a_index)
    {
        case -1:
            f_result->descfn = (PYINST_Descriptor_Function)modulex_PYINST_descriptor;
            break;
        case 1:
            f_result->descfn = (PYINST_Descriptor_Function)euphoria_PYINST_descriptor;
            break;
        case 2:
            f_result->descfn = (PYINST_Descriptor_Function)rayv_PYINST_descriptor;
            break;
        case 3:
            f_result->descfn = (PYINST_Descriptor_Function)wayv_PYINST_descriptor;
            break;
    }

    f_result->descriptor = f_result->descfn(0);

    int j;

    f_result->pluginOutputBuffers = (float**)malloc(2 * sizeof(float*));

    int f_i = 0;

    if(posix_memalign((void**)(&f_result->pluginControlIns), 16,
        (sizeof(float) * f_result->descriptor->PYFX_Plugin->PortCount)) != 0)
    {
        return 0;
    }

    f_i = 0;
    while(f_i < f_result->descriptor->PYFX_Plugin->PortCount)
    {
        f_result->pluginControlIns[f_i] = 0.0f;
        f_i++;
    }

    f_result->PYFX_handle = f_result->descriptor->PYFX_Plugin->instantiate(
            f_result->descriptor->PYFX_Plugin, a_sample_rate,
            a_host_wavpool_func, a_track_num, a_queue_func);

    for (j = 0; j < 2; j++)
    {
        if(posix_memalign((void**)(&f_result->pluginOutputBuffers[j]), 16, (sizeof(float) * 8192)) != 0)
        {
            return 0;
        }

        f_i = 0;

        while(f_i < 8192)
        {
            f_result->pluginOutputBuffers[j][f_i] = 0.0f;
            f_i++;
        }

        f_result->descriptor->PYFX_Plugin->connect_buffer(f_result->PYFX_handle, j, f_result->pluginOutputBuffers[j]);
    }

    for (j = 0; j < f_result->descriptor->PYFX_Plugin->PortCount; j++)
    {
        PYFX_PortDescriptor pod = f_result->descriptor->PYFX_Plugin->PortDescriptors[j];

        if(pod)
        {
            f_result->pluginControlIns[j] = g_pydaw_get_port_default(f_result->descriptor->PYFX_Plugin, j);

            f_result->descriptor->PYFX_Plugin->connect_port(f_result->PYFX_handle, j, &f_result->pluginControlIns[j]);
        }
    }

    f_result->descriptor->PYFX_Plugin->activate(f_result->PYFX_handle, f_result->pluginControlIns);

    return f_result;
}

void v_free_pydaw_plugin(t_pydaw_plugin * a_plugin)
{
    if(a_plugin)
    {
        if (a_plugin->descriptor->PYFX_Plugin->deactivate)
        {
            a_plugin->descriptor->PYFX_Plugin->deactivate(a_plugin->PYFX_handle);
        }

        if (a_plugin->descriptor->PYFX_Plugin->cleanup)
        {
            a_plugin->descriptor->PYFX_Plugin->cleanup(a_plugin->PYFX_handle);
        }

        free(a_plugin);
    }
    else
    {
        printf("Error, attempted to free NULL plugin with v_free_pydaw_plugin()\n");
    }
}

inline void v_run_plugin(t_pydaw_plugin * a_plugin, int a_sample_count, t_pydaw_seq_event * a_event_buffer,
        int a_event_count)
{
    a_plugin->descriptor->run_synth(a_plugin->PYFX_handle, a_sample_count, a_event_buffer, a_event_count);
}

#ifdef	__cplusplus
}
#endif

#endif	/* PYDAW_PLUGIN_H */

