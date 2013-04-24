/* 
 * File:   main.c
 * Author: Jeff Hubbard 
 */

#include <stdio.h>
#include <stdlib.h>

#include "../../plugins/pydaw/src/synth.c"
#include "../../plugins/include/pydaw3/pydaw_plugin.h"
#include "pyfx_ports.h"
#include <unistd.h>
#include <alsa/asoundlib.h>

#define DEBUGGER_SIMULATE_EXTERNAL_MIDI
//#define DEBUGGER_SIMULATE_RECORD  //currently requires an existing ~/pydaw3/default-project to work without crashing...
#define DEBUGGER_SAMPLE_COUNT 512


void v_print_plugin_controller_maps()
{
    const PYFX_Descriptor * f_wayv = wayv_PYFX_descriptor(0);
    const PYFX_Descriptor * f_rayv = rayv_PYFX_descriptor(0);
    const PYFX_Descriptor * f_euphoria = euphoria_PYFX_descriptor(0);
    const PYFX_Descriptor * f_modulex = modulex_PYFX_descriptor(0);
    
    char f_file_names[4][32] = {"Euphoria", "Way-V", "Ray-V", "Modulex"};
    const PYFX_Descriptor * f_desc[] = {f_euphoria, f_wayv, f_rayv, f_modulex};
    
    int f_i = 0;
    char f_line_buffer[1024] = "\0";
    while(f_i < 4)
    {        
        char f_buffer[1000000] = "\0";
        int f_i2 = 0;
        while(f_i2 < f_desc[f_i]->PortCount)
        {
            if(f_desc[f_i]->Automatable[f_i2] == 1)
            {
                sprintf(f_line_buffer, "%s|%i|%i|%f|%f\n", f_desc[f_i]->PortNames[f_i2], f_i2, f_desc[f_i]->ValueTransformHint[f_i2],
                        f_desc[f_i]->PortRangeHints[f_i2].LowerBound, f_desc[f_i]->PortRangeHints[f_i2].UpperBound);
                strcat(f_buffer, f_line_buffer);
            }
            f_i2++;
        }
        char f_file_temp_name[32];
        sprintf(f_file_temp_name, "%s.pymap", f_file_names[f_i]);
        FILE * f_file = fopen(f_file_temp_name, "w");
        fprintf(f_file, "%s", f_buffer);
        fclose(f_file);
        f_i++;
    }
}

int main(int argc, char** argv) 
{
    v_print_plugin_controller_maps();
    v_pydaw_constructor();

    const PYFX_Descriptor * f_ldesc = PYFX_descriptor(0);
    const PYINST_Descriptor * f_ddesc = PYINST_descriptor(0);
    PYFX_Handle f_handle =  f_ldesc->instantiate(f_ldesc, 44100);
    f_ldesc->activate(f_handle);

    t_pydaw_engine * f_engine = (t_pydaw_engine*)f_handle;
        
    //It's not necessary to call this, it gets called anyways at startup...  Only use it to load an alternate project
    //v_open_project(pydaw_data, "/home/cletus/pydaw3/default-project/default.pydaw3");

    f_engine->output0 = (PYFX_Data*)malloc(sizeof(PYFX_Data) * 8192);
    f_engine->output1 = (PYFX_Data*)malloc(sizeof(PYFX_Data) * 8192);
    
    int f_i = 0;
    
    while(f_i < PYDAW_INPUT_COUNT)
    {
        f_engine->input_arr[f_i] = (PYFX_Data*)malloc(sizeof(PYFX_Data) * 8192);
        
        int f_i2 = 0;
        while(f_i2 < DEBUGGER_SAMPLE_COUNT)
        {
            f_engine->input_arr[f_i][f_i2] = 0.0f;
            f_i2++;
        }
        
        f_i++;
    }
    
    float * f_control_ins = (float*)malloc(sizeof(float) * 3000);
    set_PYFX_ports(f_ddesc, f_handle, f_control_ins);
    
    f_i = 0;
    
    snd_seq_event_t * f_midi_events = (snd_seq_event_t*)malloc(sizeof(snd_seq_event_t) * 512);
    snd_seq_ev_clear(&f_midi_events[0]);
    snd_seq_ev_set_noteon(&f_midi_events[0], 0, 66, 66);
    f_midi_events[0].time.tick = 0;
    snd_seq_ev_clear(&f_midi_events[1]);
    snd_seq_ev_set_controller(&f_midi_events[1], 0, 1, 100);
    f_midi_events[1].time.tick = 5;
    snd_seq_ev_clear(&f_midi_events[2]);
    snd_seq_ev_set_pitchbend(&f_midi_events[2], 0, 1000);
    f_midi_events[2].time.tick = 10;
    snd_seq_ev_clear(&f_midi_events[3]);
    snd_seq_ev_set_noteoff(&f_midi_events[3], 0, 66, 0);
    f_midi_events[2].time.tick = 1000;

#ifdef DEBUGGER_SIMULATE_RECORD
    v_set_playback_mode(pydaw_data, 2, 0, 0);
#endif
    
    //Run it a few times to get the kinks out...  Ideally this shouldn't have to be done, though...
    while(f_i < 100)
    {
#ifdef DEBUGGER_SIMULATE_EXTERNAL_MIDI
        f_ddesc->run_synth(f_handle, DEBUGGER_SAMPLE_COUNT, f_midi_events, 4);
#else
        f_ddesc->run_synth(f_handle, DEBUGGER_SAMPLE_COUNT, NULL, 0);
#endif
        f_i++;
    }

#ifdef DEBUGGER_SIMULATE_RECORD
    v_set_playback_mode(pydaw_data, 0, 0, 0);
#endif
    
    v_pydaw_offline_render(pydaw_data, 0, 0, 0, 3, "test.wav");
    
    exit(0);
    //return 0;     
}

