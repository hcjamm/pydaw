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
#include <lo/lo.h>
#include <math.h>
#include <stdlib.h>

#include "../modulex/src/synth.c"
#include "../euphoria/src/synth.c"
#include "../way_v/src/synth.c"
#include "../ray_v/src/synth.c"
    
#define PYDAW_MAX_BUFFER_SIZE 4096
    
PYFX_Data g_pydaw_get_port_default(const PYFX_Descriptor *plugin, int port)
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
    
    const PYINST_Descriptor *descriptor;
    int    ins;
    int    outs;
    float **pluginInputBuffers, **pluginOutputBuffers;
    float *pluginControlIns;    
}t_pydaw_plugin;

#ifdef PYDAW_PLUGIN_MEMCHECK    
void v_pydaw_plugin_memcheck(t_pydaw_plugin * a_plugin);
#endif

int v_pydaw_plugin_configure_handler(t_pydaw_plugin *instance, const char *key, const char *value, pthread_mutex_t * a_mutex)
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


t_pydaw_plugin * g_pydaw_plugin_get(int a_sample_rate, int a_index, fp_get_wavpool_item_from_host a_host_wavpool_func)
{
    t_pydaw_plugin * f_result = (t_pydaw_plugin*)malloc(sizeof(t_pydaw_plugin));  //TODO: posix_memalign instead...
    
    switch(a_index)
    {
        case -1:
            f_result->descfn = (PYINST_Descriptor_Function)modulex_PYINST_descriptor;
            //f_result->lib_handle = dlopen("/usr/lib/dssi/lms_modulex.so", RTLD_NOW | RTLD_LOCAL);                  
            break;
        case 1:
            f_result->descfn = (PYINST_Descriptor_Function)euphoria_PYINST_descriptor;
            //f_result->lib_handle = dlopen("/usr/lib/dssi/euphoria.so", RTLD_NOW | RTLD_LOCAL);                  
            break;
        case 2:
            f_result->descfn = (PYINST_Descriptor_Function)rayv_PYINST_descriptor;
            //f_result->lib_handle = dlopen("/usr/lib/dssi/ray_v.so", RTLD_NOW | RTLD_LOCAL);                  
            break;
        case 3:
            f_result->descfn = (PYINST_Descriptor_Function)wayv_PYINST_descriptor;
            //f_result->lib_handle = dlopen("/usr/lib/dssi/way_v.so", RTLD_NOW | RTLD_LOCAL);                  
            break;
    }
    
    //f_result->descfn = (PYINST_Descriptor_Function)dlsym(f_result->lib_handle, "PYINST_descriptor");
    
    f_result->descriptor = f_result->descfn(0);
    
    f_result->ins = 0;
    f_result->outs = 0;

    int j;
    
    for (j = 0; j < f_result->descriptor->PYFX_Plugin->PortCount; j++) 
    {
        PYFX_PortDescriptor pod = f_result->descriptor->PYFX_Plugin->PortDescriptors[j];

        if (PYFX_IS_PORT_AUDIO(pod)) 
        {
            if (PYFX_IS_PORT_INPUT(pod)) ++f_result->ins;
            else if (PYFX_IS_PORT_OUTPUT(pod)) ++f_result->outs;
        } 
    }
       
    
    f_result->pluginInputBuffers = (float**)malloc((f_result->ins) * sizeof(float*));
    f_result->pluginControlIns = (float*)calloc(f_result->descriptor->PYFX_Plugin->PortCount, sizeof(float));
    f_result->pluginOutputBuffers = (float**)malloc((f_result->outs) * sizeof(float*));
    
    f_result->PYFX_handle = f_result->descriptor->PYFX_Plugin->instantiate(f_result->descriptor->PYFX_Plugin, a_sample_rate, a_host_wavpool_func);
    
    int in, out;
    
    in = out = 0;
    
    for (j = 0; j < f_result->descriptor->PYFX_Plugin->PortCount; j++) 
    {
        PYFX_PortDescriptor pod =
            f_result->descriptor->PYFX_Plugin->PortDescriptors[j];

        f_result->pluginControlIns[j] = 0.0f;

        if (PYFX_IS_PORT_AUDIO(pod)) {

            if (PYFX_IS_PORT_INPUT(pod)) 
            {
                if(posix_memalign((void**)(&f_result->pluginInputBuffers[in]), 16, (sizeof(float) * 8192)) != 0)
                {
                    return 0;
                }
                
                int f_i = 0;
                while(f_i < 8192)
                {
                    f_result->pluginInputBuffers[in][f_i] = 0.0f;
                    f_i++;
                }

                f_result->descriptor->PYFX_Plugin->connect_port(f_result->PYFX_handle, j, f_result->pluginInputBuffers[in]);                                
                in++;
            } 
            else if (PYFX_IS_PORT_OUTPUT(pod)) 
            {
                if(posix_memalign((void**)(&f_result->pluginOutputBuffers[out]), 16, (sizeof(float) * 8192)) != 0)
                {
                    return 0;
                }                
                
                int f_i = 0;
                while(f_i < 8192)
                {
                    f_result->pluginOutputBuffers[out][f_i] = 0.0f;
                    f_i++;
                }

                f_result->descriptor->PYFX_Plugin->connect_port(f_result->PYFX_handle, j, f_result->pluginOutputBuffers[out]);                
                out++;
            }

        } 
        else if (PYFX_IS_PORT_CONTROL(pod)) 
        {
            if (PYFX_IS_PORT_INPUT(pod))
            {
                f_result->pluginControlIns[j] = g_pydaw_get_port_default(f_result->descriptor->PYFX_Plugin, j);

                f_result->descriptor->PYFX_Plugin->connect_port
                    (f_result->PYFX_handle, j, &f_result->pluginControlIns[j]);
            }
        }
    }
    
    f_result->descriptor->PYFX_Plugin->activate(f_result->PYFX_handle);
    
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

void v_run_plugin(t_pydaw_plugin * a_plugin, int a_sample_count, t_pydaw_seq_event * a_event_buffer, 
        int a_event_count)
{
    a_plugin->descriptor->run_synth(a_plugin->PYFX_handle, a_sample_count, a_event_buffer, a_event_count);
}

#ifdef	__cplusplus
}
#endif

#endif	/* PYDAW_PLUGIN_H */

