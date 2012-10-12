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
#include <dlfcn.h>
    
#define PYDAW_MAX_BUFFER_SIZE 4096

typedef struct st_pydaw_plugin
{
    void * lib_handle;
    float buffer[2][PYDAW_MAX_BUFFER_SIZE];  //Buffer for writing the audio output to(and reading from???)
    
    DSSI_Descriptor_Function descfn;
    
    LADSPA_Handle ladspa_handle;
    
    const DSSI_Descriptor *descriptor;
    int    ins;
    int    outs;
    int    controlIns;
    int    controlOuts;    
    int    firstControlIn;                       /* the offset to translate instance control in # to global control in # */
    int    *pluginPortControlInNumbers;           /* maps instance LADSPA port # to global control in # */
    long   controllerMap[128]; /* maps MIDI controller to global control in # */
    DSSI_Program_Descriptor *pluginPrograms;
        
    lo_address       uiTarget;
    lo_address       uiSource;
    int              ui_initial_show_sent;
    int              uiNeedsProgramUpdate;
    char            *ui_osc_control_path;
    char            *ui_osc_configure_path;
    char            *ui_osc_program_path;
    char            *ui_osc_quit_path;
    char            *ui_osc_rate_path;
    char            *ui_osc_show_path;
}t_pydaw_plugin;

t_pydaw_plugin * g_pydaw_plugin_get(int a_sample_rate, int a_index)
{
    t_pydaw_plugin * f_result = (t_pydaw_plugin*)malloc(sizeof(t_pydaw_plugin));  //TODO:  posix_memalign instead...
     
    switch(a_index)
    {
        case 1:
            f_result->lib_handle = dlopen("/usr/lib/dssi/euphoria.so", RTLD_NOW | RTLD_LOCAL);                  
            break;
        case 2:
            f_result->lib_handle = dlopen("/usr/lib/dssi/ray_v.so", RTLD_NOW | RTLD_LOCAL);                  
            break;
    }
    
    f_result->descfn = (DSSI_Descriptor_Function)dlsym(f_result->lib_handle, "dssi_descriptor");
    
    f_result->descriptor = f_result->descfn(0);
    
    f_result->ins = 0;
    f_result->outs = 0;
    f_result->controlIns = 0;
    f_result->controlOuts = 0;

    int j;
    
    for (j = 0; j < f_result->descriptor->LADSPA_Plugin->PortCount; j++) 
    {
        LADSPA_PortDescriptor pod = f_result->descriptor->LADSPA_Plugin->PortDescriptors[j];

        if (LADSPA_IS_PORT_AUDIO(pod)) 
        {
            if (LADSPA_IS_PORT_INPUT(pod)) ++f_result->ins;
            else if (LADSPA_IS_PORT_OUTPUT(pod)) ++f_result->outs;
        } 
        else if (LADSPA_IS_PORT_CONTROL(pod)) 
        {
            if (LADSPA_IS_PORT_INPUT(pod)) ++f_result->controlIns;
            else if (LADSPA_IS_PORT_OUTPUT(pod)) ++f_result->controlOuts;
        }
    }
   
    /* finish up new plugin */
    f_result->pluginPortControlInNumbers =
            (int*)malloc(f_result->descriptor->LADSPA_Plugin->PortCount *
                          sizeof(int));

    printf("f_result->in %i\n", f_result->ins);
    printf("f_result->outs %i\n", f_result->outs);
    printf("f_result->controlIns %i\n", f_result->controlIns);
    printf("f_result->controlOuts %i\n", f_result->controlOuts);
    
    //TODO:  Count ins and outs from the loop at line 1142.  Or just rely on that we already know it
    
    f_result->ladspa_handle = f_result->descriptor->LADSPA_Plugin->instantiate
            (f_result->descriptor->LADSPA_Plugin, a_sample_rate);
    
    
    //Leaving off at or around line 1230
    
    return f_result;
}

void v_free_pydaw_plugin(t_pydaw_plugin * a_plugin)
{
    free(a_plugin);
}

void v_show_plugin_ui(t_pydaw_plugin * a_plugin)
{
    
}

void v_run_plugin(t_pydaw_plugin * a_plugin, snd_seq_event_t * a_event_buffer, 
        int a_event_count)
{
    
}

#ifdef	__cplusplus
}
#endif

#endif	/* PYDAW_PLUGIN_H */

