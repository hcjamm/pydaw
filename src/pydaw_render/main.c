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

#include "../pydaw/src/synth.c"
#include "../pydaw/include/pydaw_plugin.h"
#include <unistd.h>


void print_help()
{
    printf("Usage:  %s_render [project_dir] [output_file] [start_region] "
            "[start_bar] [end_region] [end_bar] [sample_rate] "
            "[buffer_size] [thread_count]\n\n", PYDAW_VERSION);
}

int main(int argc, char** argv)
{
    if(argc != 10)
    {
        print_help();
        exit(100);
    }

    char * f_project_dir = argv[1];
    char * f_output_file = argv[2];
    int f_start_region = atoi(argv[3]);
    int f_start_bar = atoi(argv[4]);
    int f_end_region = atoi(argv[5]);
    int f_end_bar = atoi(argv[6]);
    int f_sample_rate = atoi(argv[7]);
    int f_buffer_size = atoi(argv[8]);
    int f_thread_count = atoi(argv[9]);

    v_pydaw_constructor();

    PYFX_Descriptor * f_ldesc = PYFX_descriptor(0);
    PYFX_Handle f_handle =  g_pydaw_instantiate(f_ldesc, f_sample_rate);

    v_pydaw_activate(f_handle, f_thread_count, 0, f_project_dir);
    t_pydaw_engine * f_engine = (t_pydaw_engine*)f_handle;

    f_engine->output0 = (PYFX_Data*)malloc(sizeof(PYFX_Data) * f_buffer_size);
    f_engine->output1 = (PYFX_Data*)malloc(sizeof(PYFX_Data) * f_buffer_size);

    int f_i = 0;
    while(f_i < f_buffer_size)
    {
        f_engine->output0[f_i] = 0.0f;
        f_engine->output1[f_i] = 0.0f;
        f_i++;
    }

    pydaw_data->sample_count = f_buffer_size;
    v_pydaw_offline_render_prep(pydaw_data);

    v_pydaw_offline_render(pydaw_data, f_start_region, f_start_bar,
            f_end_region, f_end_bar, f_output_file, 0);

    return 0;
}
