/* 
 * File:   pydaw_plugin.h
 * Author: jeffh
 *
 * Created on October 11, 2012, 6:48 PM
 */

#ifndef PYDAW_PLUGIN_H
#define	PYDAW_PLUGIN_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "dssi.h"
#include "pydaw.h"
#include <ladspa.h>
#include <lo/lo.h>
#include <alsa/asoundlib.h>
    
#define PYDAW_MAX_BUFFER_SIZE 4096

typedef struct st_pydaw_plugin
{
     float buffer[2][PYDAW_MAX_BUFFER_SIZE];  //Buffer for writing the audio output to(and reading from???)
}t_pydaw_plugin;

t_pydaw_plugin * g_pydaw_plugin_get(int a_index)
{
    t_pydaw_plugin * f_result = (t_pydaw_plugin*)malloc(sizeof(t_pydaw_plugin));  //TODO:  posix_memalign instead...
        
    return f_result;
}

void v_free_pydaw_plugin(t_pydaw_plugin * a_plugin)
{
    free(a_plugin);
}

void v_run_plugin(t_pydaw_plugin * a_plugin, snd_seq_event_t * a_event_buffer, 
        int a_event_count)
{
    
}

#ifdef	__cplusplus
}
#endif

#endif	/* PYDAW_PLUGIN_H */

