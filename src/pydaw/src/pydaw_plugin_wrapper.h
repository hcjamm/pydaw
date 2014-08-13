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
    assert(hint.DefaultValue <= hint.UpperBound &&
            hint.DefaultValue >= hint.LowerBound );
    return hint.DefaultValue;
}

typedef struct st_pydaw_plugin
{
    //void * lib_handle;
    PYFX_Handle PYFX_handle;
    PYFX_Descriptor_Function descfn;

    PYFX_Descriptor *descriptor;
    float *pluginControlIns;
}t_pydaw_plugin;


int v_pydaw_plugin_configure_handler(t_pydaw_plugin *instance,
        char *key, char *value, pthread_spinlock_t * a_spinlock)
{
    char * message = 0;

    if (instance->descriptor->configure)
    {
        message = instance->descriptor->configure(
                instance->PYFX_handle, key, value, a_spinlock);
        if (message)
        {
            printf("PyDAW: on configure '%s' '%s', "
                    "plugin returned error '%s'\n", key, value, message);
            free(message);
        }
    }

    return 0;
}


t_pydaw_plugin * g_pydaw_plugin_get(int a_sample_rate, int a_index,
        fp_get_wavpool_item_from_host a_host_wavpool_func,
        int a_track_num, fp_queue_message a_queue_func)
{
     //TODO: posix_memalign instead...
    t_pydaw_plugin * f_result = (t_pydaw_plugin*)malloc(sizeof(t_pydaw_plugin));

    switch(a_index)
    {
        case -1:
            f_result->descfn =
                    (PYFX_Descriptor_Function)modulex_PYFX_descriptor;
            break;
        case 1:
            f_result->descfn =
                    (PYFX_Descriptor_Function)euphoria_PYFX_descriptor;
            break;
        case 2:
            f_result->descfn =
                    (PYFX_Descriptor_Function)rayv_PYFX_descriptor;
            break;
        case 3:
            f_result->descfn =
                    (PYFX_Descriptor_Function)wayv_PYFX_descriptor;
            break;
    }

    f_result->descriptor = f_result->descfn(0);

    int j;

    int f_i = 0;

    if(posix_memalign((void**)(&f_result->pluginControlIns), 16,
        (sizeof(float) * f_result->descriptor->PortCount)) != 0)
    {
        return 0;
    }

    f_i = 0;
    while(f_i < f_result->descriptor->PortCount)
    {
        f_result->pluginControlIns[f_i] = 0.0f;
        f_i++;
    }

    f_result->PYFX_handle = f_result->descriptor->instantiate(
            f_result->descriptor, a_sample_rate,
            a_host_wavpool_func, a_track_num, a_queue_func,
            f_result->pluginControlIns);

    for (j = 0; j < f_result->descriptor->PortCount; j++)
    {
        PYFX_PortDescriptor pod =
                f_result->descriptor->PortDescriptors[j];

        if(pod)
        {
            f_result->pluginControlIns[j] =
                    g_pydaw_get_port_default(f_result->descriptor, j);

            f_result->descriptor->connect_port(
                f_result->PYFX_handle, j, &f_result->pluginControlIns[j]);
        }
    }

    return f_result;
}

void v_free_pydaw_plugin(t_pydaw_plugin * a_plugin)
{
    if(a_plugin)
    {
        if (a_plugin->descriptor->cleanup)
        {
            a_plugin->descriptor->cleanup(a_plugin->PYFX_handle);
        }

        free(a_plugin);
    }
    else
    {
        printf("Error, attempted to free NULL plugin "
                "with v_free_pydaw_plugin()\n");
    }
}

inline void v_run_plugin(t_pydaw_plugin * a_plugin, int a_sample_count,
        t_pydaw_seq_event * a_event_buffer, int a_event_count)
{
    a_plugin->descriptor->run_synth(
        a_plugin->PYFX_handle, a_sample_count, a_event_buffer, a_event_count);
}

#ifdef	__cplusplus
}
#endif

#endif	/* PYDAW_PLUGIN_H */

