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

/*Change these to the project you would like to debug*/
//#include "../../plugins/libmodsynth/lib/lms_sequencer.h"
//#include "../../plugins/pydaw/src/pydaw.h"
//#include "../../plugins/pydaw/src/pydaw_files.h"
//#include "../../plugins/pydaw/src/synth.c"
#include "../../plugins/ray_v/src/synth.c"
#include <dssi.h>
#include "ladspa_ports.h"
#include <unistd.h>
#include <alsa/asoundlib.h>
/* int main(
 * int argc, //ignored
 * char** argv) //ignored
 * 
 */
int main(int argc, char** argv) 
{   
    /*
    t_pydaw_data * f_data = g_pydaw_data_get(44100);
    v_set_tempo(f_data, 140.0f);
    v_open_project(f_data, "/home/bob/dssi/pydaw/default-project", "default");
    v_pydaw_parse_configure_message(f_data, "play", "0|0");
    */
    /*
    v_pydaw_constructor();
    const LADSPA_Descriptor * f_ldesc = ladspa_descriptor(0);
    const DSSI_Descriptor * f_ddesc = dssi_descriptor(0);
    LADSPA_Handle f_handle =  f_ldesc->instantiate(f_ldesc, 44100);
    f_ldesc->activate(f_handle);
    
    t_pydaw_engine * f_engine = (t_pydaw_engine*)f_handle;
    
    f_engine->output0 = (LADSPA_Data*)malloc(sizeof(LADSPA_Data) * 8092);
    f_engine->output1 = (LADSPA_Data*)malloc(sizeof(LADSPA_Data) * 8092);
    
    v_set_tempo(pydaw_data, 140.0f);
    v_open_project(pydaw_data, "/home/bob/dssi/pydaw/default-project", "default");
    
    v_set_plugin_index(pydaw_data, 0, 2);
    v_set_plugin_index(pydaw_data, 1, 1);    
    v_set_plugin_index(pydaw_data, 2, 2);    
    */
    
    v_rayv_constructor();
    const LADSPA_Descriptor * f_ldesc = ladspa_descriptor(0);
    const DSSI_Descriptor * f_ddesc = dssi_descriptor(0);
    LADSPA_Handle f_handle =  f_ldesc->instantiate(f_ldesc, 44100);
    f_ldesc->activate(f_handle);
    
    t_rayv * f_engine = (t_rayv*)f_handle;
    
    f_engine->output0 = (LADSPA_Data*)malloc(sizeof(LADSPA_Data) * 8092);
    f_engine->output1 = (LADSPA_Data*)malloc(sizeof(LADSPA_Data) * 8092);
    
    float * f_control_ins = (float*)malloc(sizeof(float) * 3000);
    set_ladspa_ports(f_ddesc, f_handle, f_control_ins);
    
    snd_seq_event_t * f_events = (snd_seq_event_t*)malloc(sizeof(snd_seq_event_t) * 32);
    int i, i2;    
    
    for(i = 0; i < 3; i++)
    {
        snd_seq_ev_set_noteon(&f_events[i], 0, 44 + i, 100);
        f_events[i].time.tick = i;
        snd_seq_ev_set_noteoff(&f_events[i + 3], 0, 44 + i, 100);
        f_events[i + 3].time.tick = i + 500;
    }
    
    f_ddesc->run_synth(f_handle, 4096, f_events, 3);
    
    /*
    v_pydaw_parse_configure_message(pydaw_data, "tr", "0|1");    
    v_pydaw_parse_configure_message(pydaw_data, "play", "0|0");
    
    while(pydaw_data->current_region < 4)
    {        
        f_ddesc->run_synth(f_handle, 4096, NULL, 0);     
    }   
    
    f_ddesc->configure(pydaw_data, "stop", "");
     */
    
    /*
    g_get_2d_array_from_file("/home/bob/dssi/pydaw/default-project/items/item-0.pyitem", 65536);
    */
    
    return 0; //(EXIT_SUCCESS);
}

