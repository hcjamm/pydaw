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
#include "../../plugins/pydaw/src/pydaw.h"
#include "../../plugins/pydaw/src/pydaw_files.h"
#include "../../plugins/pydaw/src/synth.c"

/* int main(
 * int argc, //ignored
 * char** argv) //ignored
 * 
 */
int main(int argc, char** argv) {
    
    
    /*t_osc_simple_unison * f_osc = g_osc_get_osc_simple_unison(44100.0f);
    v_osc_set_simple_osc_unison_type(f_osc, 0);
    
    v_osc_set_unison_pitch(f_osc, 0.1f, 66.0f);
    
    v_lim_set(f_limiter, -6.0f, -9.0f, 200.0f);
    
    float * test_square; // = (float*)malloc(sizeof(float) * 50);
    posix_memalign((void**)&test_square, 16, (sizeof(float) * 50));
        
    int i;
    
    for(i = 0; i < 10; i++)
    {
        test_square[i] = 0.0f;
    }
    
    test_square[10] = 1.0f;
    test_square[11] = 1.0f;
    test_square[12] = 1.0f;
    test_square[13] = 1.0f;
    test_square[14] = 1.0f;
    test_square[15] = 1.0f;
    test_square[16] = 1.0f;
    test_square[17] = 1.0f;
    test_square[18] = 1.0f;
    test_square[19] = -1.0f;
    test_square[20] = -1.0f;
    test_square[21] = -1.0f;
    test_square[22] = -1.0f;
    test_square[23] = -1.0f;
    test_square[24] = -1.0f;
    test_square[25] = -1.0f;
    test_square[26] = -1.0f;
    test_square[27] = -1.0f;
    
    for(i = 28; i < 50; i++)
    {
        test_square[i] = 0.0f;
    }
    
    int f_i;
    
    for(f_i = 0; f_i < 500000; f_i++)
    {
        float f_sample = f_osc_run_unison_osc(f_osc);
        
        v_lim_run(f_limiter, f_sample, f_sample);
    }
    
    
    for(f_i = 0; f_i < 500000; f_i++)
    {
        float f_sample = f_osc_run_unison_osc(f_osc);
        
        v_lim_run(f_limiter, f_sample, f_sample);
    }
    
    
    for(f_i = 0; f_i < 500000; f_i++)
    {
        float f_sample = f_osc_run_unison_osc(f_osc);
        
        v_lim_run(f_limiter, f_sample, f_sample);
    }
    */

    //midi_open();
    //snd_seq_close(seq);
    
    /*
    t_pydaw_data * f_data = g_pydaw_data_get(44100);
    v_set_tempo(f_data, 140.0f);
    v_open_project(f_data, "/home/bob/dssi/pydaw/default-project", "default");
    v_pydaw_parse_configure_message(f_data, "play", "0|0");
    */
    
    init();
    
    LADSPA_Handle f_handle = instantiateLMS(LMSLDescriptor, 44100);
    
    activateLMS(f_handle);
    
    t_pydaw_engine * f_engine = (t_pydaw_engine*)f_handle;
    
    f_engine->output0 = (LADSPA_Data*)malloc(sizeof(LADSPA_Data) * 8092);
    f_engine->output1 = (LADSPA_Data*)malloc(sizeof(LADSPA_Data) * 8092);
    
    v_set_tempo(pydaw_data, 140.0f);
    v_open_project(pydaw_data, "/home/bob/dssi/pydaw/default-project", "default");
            
    //snd_seq_event_t * f_events = (snd_seq_event_t*)malloc(sizeof(snd_seq_event_t) * 32);
    
    v_set_plugin_index(pydaw_data, 0, 2);
    //v_set_plugin_index(pydaw_data, 1, 2);
    //v_set_plugin_index(pydaw_data, 2, 2);
    //v_pydaw_save_tracks(pydaw_data);
    
    int i, i2;
    
    /*
    for(i = 0; i < 3; i++)
    {
        f_events[i].data.note.note = 44 + i;
        f_events[i].type = SND_SEQ_EVENT_NOTEON;
        f_events[i].time.tick = i;
    }
    */
    
    v_pydaw_parse_configure_message(pydaw_data, "tr", "4|0");
    
    for(i2 = 0; i2 < 10; i2++)
    {
        v_pydaw_parse_configure_message(pydaw_data, "play", "0|0");
        for(i = 0; i < 600; i++)
        {
            v_pydaw_run(f_handle, 4096, NULL, 0);
        }
        v_pydaw_parse_configure_message(pydaw_data, "stop", "");
    }   
    /*
    g_get_2d_array_from_file("/home/bob/dssi/pydaw/default-project/items/item-0.pyitem", 65536);
    */
    
    return 0; //(EXIT_SUCCESS);
}

