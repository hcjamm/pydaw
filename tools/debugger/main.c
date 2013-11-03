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

//Required for sched.h
#ifndef __USE_GNU
#define __USE_GNU
#endif
    
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif 

#include <stdio.h>
#include <stdlib.h>

#include "../../src/pydaw/src/synth.c"
#include "../../src/pydaw/include/pydaw_plugin.h"
#include <unistd.h>

//#define DEBUGGER_SIMULATE_EXTERNAL_MIDI
//#define DEBUGGER_SIMULATE_RECORD  //currently requires an existing ~/pydaw4/default-project to 
                                    //work without crashing...  THIS WILL WRITE EVENTS INTO YOUR PROJECT!!!!!!
#define DEBUGGER_SAMPLE_COUNT 512


int main(int argc, char** argv) 
{
    v_pydaw_constructor();

    const PYFX_Descriptor * f_ldesc = PYFX_descriptor(0);    
    PYFX_Handle f_handle =  g_pydaw_instantiate(f_ldesc, 44100);
    v_pydaw_activate(f_handle, 0, 1);

    t_pydaw_engine * f_engine = (t_pydaw_engine*)f_handle;
        
    //It's not necessary to call this, it gets called anyways at startup...  Only use it to load an alternate project
    //v_open_project(pydaw_data, "/home/cletus/pydaw4/default-project/default.pydaw4");

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
                
    t_pydaw_seq_event * f_midi_events = (t_pydaw_seq_event*)malloc(sizeof(t_pydaw_seq_event) * 512);
    v_pydaw_ev_clear(&f_midi_events[0]);
    v_pydaw_ev_set_noteon(&f_midi_events[0], 0, 66, 66);
    f_midi_events[0].tick = 0;
    v_pydaw_ev_clear(&f_midi_events[1]);
    v_pydaw_ev_set_controller(&f_midi_events[1], 0, 84, 100);
    f_midi_events[1].tick = 50;
    v_pydaw_ev_clear(&f_midi_events[2]);
    v_pydaw_ev_set_pitchbend(&f_midi_events[2], 0, 1000);
    f_midi_events[2].tick = 100;
    v_pydaw_ev_clear(&f_midi_events[3]);
    v_pydaw_ev_set_noteoff(&f_midi_events[3], 0, 66, 0);
    f_midi_events[3].tick = 500;

#ifdef DEBUGGER_SIMULATE_RECORD
    pydaw_data->overdub_mode = 1;
    v_set_playback_mode(pydaw_data, 2, 0, 0);
#endif

#ifdef DEBUGGER_SIMULATE_EXTERNAL_MIDI    
    pydaw_data->record_armed_track = pydaw_data->track_pool_all[PYDAW_MIDI_TRACK_COUNT];
    pydaw_data->record_armed_track_index_all = PYDAW_MIDI_TRACK_COUNT;
#endif
    
#ifdef DEBUGGER_SIMULATE_EXTERNAL_MIDI
    f_i = 0;
    
    while(f_i < 50)
    {
        v_pydaw_run(f_handle, DEBUGGER_SAMPLE_COUNT, f_midi_events, 4);
        v_pydaw_run(f_handle, DEBUGGER_SAMPLE_COUNT, f_midi_events, 0);
        v_pydaw_run(f_handle, DEBUGGER_SAMPLE_COUNT, f_midi_events, 0);
        v_pydaw_run(f_handle, DEBUGGER_SAMPLE_COUNT, f_midi_events, 0);
        f_i++;
    }
#else
    f_i = 0;
    
    while(f_i < 100)
    {
        v_pydaw_run(f_handle, DEBUGGER_SAMPLE_COUNT, f_midi_events, 0);        
        f_i++;
    }
#endif



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

