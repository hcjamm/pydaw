/* 
 * File:   main.c
 * Author: Jeff Hubbard
 * 
 * 
 *
 * Created on March 12, 2012, 7:46 PM
 */

#include <stdio.h>
#include <stdlib.h>

#define DEBUG_PYDAW
//#define DEBUG_RAYV
#define DEBUG_EXTERNAL_MIDI

#ifdef DEBUG_PYDAW
#include "../../plugins/pydaw/src/synth.c"
#endif
#ifdef DEBUG_RAYV
#include "../../plugins/ray_v/src/synth.c"
#endif
#include <dssi.h>
#include "ladspa_ports.h"
#include <unistd.h>
#include <alsa/asoundlib.h>


int main(int argc, char** argv) 
{
#ifdef DEBUG_PYDAW
    v_pydaw_constructor();
#endif
#ifdef DEBUG_RAYV
    v_rayv_constructor();
#endif
    const LADSPA_Descriptor * f_ldesc = ladspa_descriptor(0);
    const DSSI_Descriptor * f_ddesc = dssi_descriptor(0);
    LADSPA_Handle f_handle =  f_ldesc->instantiate(f_ldesc, 44100);
    f_ldesc->activate(f_handle);
#ifdef DEBUG_PYDAW
    t_pydaw_engine * f_engine = (t_pydaw_engine*)f_handle;
    
    
    //v_open_project(pydaw_data, "/home/bob/dssi/pydaw/default-project", "default");    
    //v_set_plugin_index(pydaw_data, 0, 2);
    //v_set_plugin_index(pydaw_data, 1, 1);    
    //v_set_plugin_index(pydaw_data, 2, 2);
    //v_pydaw_parse_configure_message(pydaw_data, "tr", "0|1");
    v_set_tempo(pydaw_data, 140.0f); 
    v_pydaw_parse_configure_message(pydaw_data, "play", "0|0");
#endif    
#ifdef DEBUG_RAYV
    t_rayv * f_engine = (t_rayv*)f_handle;
#endif
    f_engine->output0 = (LADSPA_Data*)malloc(sizeof(LADSPA_Data) * 8092);
    f_engine->output1 = (LADSPA_Data*)malloc(sizeof(LADSPA_Data) * 8092);
    
    float * f_control_ins = (float*)malloc(sizeof(float) * 3000);
    set_ladspa_ports(f_ddesc, f_handle, f_control_ins);
    
#ifdef DEBUG_EXTERNAL_MIDI
    snd_seq_event_t * f_events = (snd_seq_event_t*)malloc(sizeof(snd_seq_event_t) * 32);
        
    snd_seq_ev_set_noteon(&f_events[0], 0, 44, 100);
    f_events[0].time.tick = 3;
    snd_seq_ev_set_noteoff(&f_events[1], 0, 44, 0);
    f_events[1].time.tick = 4000;

    
    int f_last_bar = 0;
#endif    
        
    while(pydaw_data->current_region < 2)
    {        
#ifdef DEBUG_EXTERNAL_MIDI
        if((pydaw_data->current_bar) != f_last_bar)
        {
            f_ddesc->run_synth(f_handle, 4096, f_events, 2);
            f_last_bar = (pydaw_data->current_bar);
        }
        else
        {
            f_ddesc->run_synth(f_handle, 4096, NULL, 0);
        }
#else
        f_ddesc->run_synth(f_handle, 4096, NULL, 0);     
#endif
    }   
    
#ifdef DEBUG_PYDAW
    f_ddesc->configure(pydaw_data, "stop", "");
#endif
    
    
    return 0; //(EXIT_SUCCESS);
}

