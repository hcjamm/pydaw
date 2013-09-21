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

#include <stdio.h>
#include <stdlib.h>

#include "../../src/pydaw/src/synth.c"
#include "../../src/pydaw/include/pydaw3/pydaw_plugin.h"
#include "pyfx_ports.h"
#include <unistd.h>

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
    PYFX_Handle f_handle =  g_pydaw_instantiate(f_ldesc, 44100);
    v_pydaw_activate(f_handle, 1);

    t_pydaw_engine * f_engine = (t_pydaw_engine*)f_handle;
        
    //It's not necessary to call this, it gets called anyways at startup...  Only use it to load an alternate project
    //v_open_project(pydaw_data, "/home/cletus/pydaw3/default-project/default.pydaw3");

    f_engine->output0 = (PYFX_Data*)malloc(sizeof(PYFX_Data) * 8192);
    f_engine->output1 = (PYFX_Data*)malloc(sizeof(PYFX_Data) * 8192);
    
    int arg_dump = 0;
    
    int f_i = 0;
    
    while(f_i < argc)
    {
        printf("arg[%i]: %s\n", f_i, argv[f_i]);
        if(!strcmp(argv[f_i], "dump"))  //force a core dump
        {
            arg_dump = 1;
        }
        f_i++;
    }
    
    f_i = 0;
    
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
    
    t_pydaw_seq_event * f_midi_events = (t_pydaw_seq_event*)malloc(sizeof(t_pydaw_seq_event) * 512);
    v_pydaw_ev_clear(&f_midi_events[0]);
    v_pydaw_ev_set_noteon(&f_midi_events[0], 0, 66, 66);
    f_midi_events[0].tick = 0;
    v_pydaw_ev_clear(&f_midi_events[1]);
    v_pydaw_ev_set_controller(&f_midi_events[1], 0, 1, 100);
    f_midi_events[1].tick = 5;
    v_pydaw_ev_clear(&f_midi_events[2]);
    v_pydaw_ev_set_pitchbend(&f_midi_events[2], 0, 1000);
    f_midi_events[2].tick = 10;
    v_pydaw_ev_clear(&f_midi_events[3]);
    v_pydaw_ev_set_noteoff(&f_midi_events[3], 0, 66, 0);
    f_midi_events[2].tick = 1000;

#ifdef DEBUGGER_SIMULATE_RECORD
    v_set_playback_mode(pydaw_data, 2, 0, 0);
#endif
    
    //Run it a few times to get the kinks out...  Ideally this shouldn't have to be done, though...
    while(f_i < 100)
    {
#ifdef DEBUGGER_SIMULATE_EXTERNAL_MIDI
        v_pydaw_run(f_handle, DEBUGGER_SAMPLE_COUNT, f_midi_events, 4);
#else
        f_ddesc->run_synth(f_handle, DEBUGGER_SAMPLE_COUNT, NULL, 0);
#endif
        f_i++;
    }

#ifdef DEBUGGER_SIMULATE_RECORD
    v_set_playback_mode(pydaw_data, 0, 0, 0);
#endif
    
    v_pydaw_offline_render(pydaw_data, 0, 0, 3, 3, "test.wav", 0);
    
    if(arg_dump)
    {
        assert(0);
    }
    
    exit(0);
    //return 0;     
}

