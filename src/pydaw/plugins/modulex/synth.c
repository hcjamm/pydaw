/* -*- c-basic-offset: 4 -*-  vi:set ts=8 sts=4 sw=4: */
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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <stdint.h>

#include <math.h>
#include <stdio.h>

#include "../../include/pydaw_plugin.h"

#include "libmodsynth.h"
#include "../../libmodsynth/lib/amp.h"
#include "../../libmodsynth/modules/filter/svf.h"

#include "../../libmodsynth/modules/delay/reverb.h"

#include "synth.h"


static void v_modulex_run(PYFX_Handle instance, int sample_count,
		  t_pydaw_seq_event * events, int EventCount);

PYFX_Descriptor *modulex_PYFX_descriptor(int index);
PYINST_Descriptor *modulex_PYINST_descriptor(int index);

static void v_modulex_cleanup(PYFX_Handle instance)
{
    free(instance);
}

static void v_modulex_panic(PYFX_Handle instance)
{
    t_modulex *plugin = (t_modulex *)instance;

    int f_i = 0;
    while(f_i < plugin->mono_modules->delay->delay0->sample_count)
    {
        plugin->mono_modules->delay->delay0->buffer[f_i] = 0.0f;
        plugin->mono_modules->delay->delay1->buffer[f_i] = 0.0f;
        f_i++;
    }
}

static void v_modulex_connect_buffer(PYFX_Handle instance, int a_index,
        float * DataLocation)
{
    t_modulex *plugin = (t_modulex*)instance;

    switch(a_index)
    {
        case 0:
            plugin->output0 = DataLocation;
            break;
        case 1:
            plugin->output1 = DataLocation;
            break;
        default:
            assert(0);
            break;
    }
}

static void v_modulex_connect_port(PYFX_Handle instance, int port,
        PYFX_Data * data)
{
    t_modulex *plugin;

    plugin = (t_modulex *) instance;

    switch (port)
    {
        case MODULEX_FX0_KNOB0: plugin->fx_knob0[0] = data; break;
        case MODULEX_FX0_KNOB1:	plugin->fx_knob1[0] = data; break;
        case MODULEX_FX0_KNOB2: plugin->fx_knob2[0] = data; break;
        case MODULEX_FX0_COMBOBOX: plugin->fx_combobox[0] = data; break;

        case MODULEX_FX1_KNOB0: plugin->fx_knob0[1] = data; break;
        case MODULEX_FX1_KNOB1:	plugin->fx_knob1[1] = data; break;
        case MODULEX_FX1_KNOB2: plugin->fx_knob2[1] = data; break;
        case MODULEX_FX1_COMBOBOX: plugin->fx_combobox[1] = data; break;

        case MODULEX_FX2_KNOB0: plugin->fx_knob0[2] = data; break;
        case MODULEX_FX2_KNOB1:	plugin->fx_knob1[2] = data; break;
        case MODULEX_FX2_KNOB2: plugin->fx_knob2[2] = data; break;
        case MODULEX_FX2_COMBOBOX: plugin->fx_combobox[2] = data; break;

        case MODULEX_FX3_KNOB0: plugin->fx_knob0[3] = data; break;
        case MODULEX_FX3_KNOB1:	plugin->fx_knob1[3] = data; break;
        case MODULEX_FX3_KNOB2: plugin->fx_knob2[3] = data; break;
        case MODULEX_FX3_COMBOBOX: plugin->fx_combobox[3] = data; break;

        case MODULEX_FX4_KNOB0: plugin->fx_knob0[4] = data; break;
        case MODULEX_FX4_KNOB1:	plugin->fx_knob1[4] = data; break;
        case MODULEX_FX4_KNOB2: plugin->fx_knob2[4] = data; break;
        case MODULEX_FX4_COMBOBOX: plugin->fx_combobox[4] = data; break;

        case MODULEX_FX5_KNOB0: plugin->fx_knob0[5] = data; break;
        case MODULEX_FX5_KNOB1:	plugin->fx_knob1[5] = data; break;
        case MODULEX_FX5_KNOB2: plugin->fx_knob2[5] = data; break;
        case MODULEX_FX5_COMBOBOX: plugin->fx_combobox[5] = data; break;

        case MODULEX_FX6_KNOB0: plugin->fx_knob0[6] = data; break;
        case MODULEX_FX6_KNOB1:	plugin->fx_knob1[6] = data; break;
        case MODULEX_FX6_KNOB2: plugin->fx_knob2[6] = data; break;
        case MODULEX_FX6_COMBOBOX: plugin->fx_combobox[6] = data; break;

        case MODULEX_FX7_KNOB0: plugin->fx_knob0[7] = data; break;
        case MODULEX_FX7_KNOB1:	plugin->fx_knob1[7] = data; break;
        case MODULEX_FX7_KNOB2: plugin->fx_knob2[7] = data; break;
        case MODULEX_FX7_COMBOBOX: plugin->fx_combobox[7] = data; break;

        case MODULEX_DELAY_TIME: plugin->delay_time = data; break;
        case MODULEX_FEEDBACK: plugin->feedback = data; break;
        case MODULEX_DRY: plugin->dry = data;  break;
        case MODULEX_WET: plugin->wet = data; break;
        case MODULEX_DUCK: plugin->duck = data; break;
        case MODULEX_CUTOFF: plugin->cutoff = data; break;
        case MODULEX_STEREO: plugin->stereo = data; break;

        case MODULEX_VOL_SLIDER: plugin->vol_slider = data; break;

        case MODULEX_REVERB_TIME: plugin->reverb_time = data; break;
        case MODULEX_REVERB_WET: plugin->reverb_wet = data; break;
        case MODULEX_REVERB_COLOR: plugin->reverb_color = data; break;
    }
}

static PYFX_Handle g_modulex_instantiate(const PYFX_Descriptor * descriptor,
        int s_rate,
        fp_get_wavpool_item_from_host a_host_wavpool_func)
{
    t_modulex *plugin_data = (t_modulex *) malloc(sizeof(t_modulex));

    plugin_data->fs = s_rate;
    return (PYFX_Handle) plugin_data;
}

static void v_modulex_activate(PYFX_Handle instance, float * a_port_table)
{
    t_modulex *plugin_data = (t_modulex *) instance;

    plugin_data->port_table = a_port_table;

    plugin_data->mono_modules = v_modulex_mono_init((plugin_data->fs));

    plugin_data->i_slow_index = MODULEX_SLOW_INDEX_ITERATIONS;
    plugin_data->is_on = 0;
}

static void v_modulex_check_if_on(t_modulex *plugin_data)
{
    int f_i = 0;

    while(f_i < 8)
    {
        plugin_data->mono_modules->fx_func_ptr[f_i] =
            g_mf3_get_function_pointer((int)(*(plugin_data->fx_combobox[f_i])));

        if(plugin_data->mono_modules->fx_func_ptr[f_i] != v_mf3_run_off)
        {
            plugin_data->is_on = 1;
        }

        f_i++;
    }
}

static void v_modulex_run(PYFX_Handle instance, int sample_count,
		  t_pydaw_seq_event *events, int event_count)
{
    t_modulex *plugin_data = (t_modulex *) instance;

    int event_pos = 0;
    int midi_event_pos = 0;
    plugin_data->midi_event_count = 0;

    while (event_pos < event_count)
    {
        if (events[event_pos].type == PYDAW_EVENT_CONTROLLER)
        {
            if(events[event_pos].plugin_index == -1)
            {
                assert(events[event_pos].port < MODULEX_COUNT &&
                        events[event_pos].port >= MODULEX_FIRST_CONTROL_PORT);

                plugin_data->midi_event_types[plugin_data->midi_event_count] =
                        PYDAW_EVENT_CONTROLLER;
                plugin_data->midi_event_ticks[plugin_data->midi_event_count] =
                        events[event_pos].tick;
                plugin_data->midi_event_ports[plugin_data->midi_event_count] =
                        events[event_pos].port;
                plugin_data->midi_event_values[plugin_data->midi_event_count] =
                        events[event_pos].value;

                if(!plugin_data->is_on)
                {
                    v_modulex_check_if_on(plugin_data);

                    //Meaning that we now have set the port anyways because the
                    //main loop won't be running
                    if(!plugin_data->is_on)
                    {
                        plugin_data->port_table[plugin_data->midi_event_ports[
                                plugin_data->midi_event_count]] =
                                plugin_data->midi_event_values[
                                plugin_data->midi_event_count];
                    }
                }

                plugin_data->midi_event_count++;
            }
        }

        event_pos++;
    }

    int f_i = 0;

    if(plugin_data->i_slow_index >= MODULEX_SLOW_INDEX_ITERATIONS)
    {
        plugin_data->i_slow_index = 0;
        plugin_data->is_on = 0;
        v_modulex_check_if_on(plugin_data);
    }
    else
    {
        plugin_data->i_slow_index = (plugin_data->i_slow_index) + 1;
    }

    f_i = 0;

    if(plugin_data->is_on)
    {
        plugin_data->i_mono_out = 0;

        while((plugin_data->i_mono_out) < sample_count)
        {
            while(midi_event_pos < plugin_data->midi_event_count &&
                    plugin_data->midi_event_ticks[midi_event_pos] ==
                    plugin_data->i_mono_out)
            {
                if(plugin_data->midi_event_types[midi_event_pos] ==
                        PYDAW_EVENT_CONTROLLER)
                {
                    plugin_data->port_table[
                            plugin_data->midi_event_ports[midi_event_pos]] =
                            plugin_data->midi_event_values[midi_event_pos];
                }

                midi_event_pos++;
            }

            plugin_data->mono_modules->current_sample0 =
                    plugin_data->output0[(plugin_data->i_mono_out)];
            plugin_data->mono_modules->current_sample1 =
                    plugin_data->output1[(plugin_data->i_mono_out)];

            f_i = 0;

            while(f_i < 8)
            {
                if(plugin_data->mono_modules->fx_func_ptr[f_i] != v_mf3_run_off)
                {
                    v_sml_run(plugin_data->mono_modules->smoothers[f_i][0],
                            *plugin_data->fx_knob0[f_i]);
                    v_sml_run(plugin_data->mono_modules->smoothers[f_i][1],
                            *plugin_data->fx_knob1[f_i]);
                    v_sml_run(plugin_data->mono_modules->smoothers[f_i][2],
                            *plugin_data->fx_knob2[f_i]);

                    v_mf3_set(plugin_data->mono_modules->multieffect[f_i],
                    plugin_data->mono_modules->smoothers[f_i][0]->last_value,
                    plugin_data->mono_modules->smoothers[f_i][1]->last_value,
                    plugin_data->mono_modules->smoothers[f_i][2]->last_value  );

                    plugin_data->mono_modules->fx_func_ptr[f_i](
                        plugin_data->mono_modules->multieffect[f_i],
                        (plugin_data->mono_modules->current_sample0),
                        (plugin_data->mono_modules->current_sample1));

                    plugin_data->mono_modules->current_sample0 =
                        plugin_data->mono_modules->multieffect[f_i]->output0;
                    plugin_data->mono_modules->current_sample1 =
                        plugin_data->mono_modules->multieffect[f_i]->output1;
                }
                f_i++;
            }

            plugin_data->output0[(plugin_data->i_mono_out)] =
                    (plugin_data->mono_modules->current_sample0);
            plugin_data->output1[(plugin_data->i_mono_out)] =
                    (plugin_data->mono_modules->current_sample1);

            plugin_data->i_mono_out = (plugin_data->i_mono_out) + 1;
        }
    }

    if((*(plugin_data->wet)) > -29.0f)
    {
        plugin_data->i_mono_out = 0;

        while((plugin_data->i_mono_out) < sample_count)
        {
            v_sml_run(plugin_data->mono_modules->time_smoother,
                    (*(plugin_data->delay_time)));

            v_ldl_set_delay(plugin_data->mono_modules->delay,
                (plugin_data->mono_modules->time_smoother->last_value * 0.01f),
                *(plugin_data->feedback),
                *(plugin_data->wet), *(plugin_data->dry),
                (*(plugin_data->stereo) * .01), (*plugin_data->duck),
                (*plugin_data->cutoff));

            v_ldl_run_delay(plugin_data->mono_modules->delay,
                    (plugin_data->output0[(plugin_data->i_mono_out)]),
                    (plugin_data->output0[(plugin_data->i_mono_out)]));

            plugin_data->output0[(plugin_data->i_mono_out)] =
                    (plugin_data->mono_modules->delay->output0);
            plugin_data->output1[(plugin_data->i_mono_out)] =
                    (plugin_data->mono_modules->delay->output1);

            plugin_data->i_mono_out = (plugin_data->i_mono_out) + 1;
        }
    }

    v_sml_run(plugin_data->mono_modules->reverb_smoother,
            (*plugin_data->reverb_wet));

    if((plugin_data->mono_modules->reverb_smoother->last_value) > 0.005f)
    {
        v_rvb_reverb_set(plugin_data->mono_modules->reverb,
                (*plugin_data->reverb_time) * 0.01f,
                f_db_to_linear_fast(((plugin_data->mono_modules->
                reverb_smoother->last_value) * 0.4f) - 40.0f,
                plugin_data->mono_modules->amp_ptr),
                (*plugin_data->reverb_color) * 0.01f);

        int f_i = 0;
        while(f_i < sample_count)
        {
            v_rvb_reverb_run(plugin_data->mono_modules->reverb,
                plugin_data->output0[f_i],
                plugin_data->output1[f_i]);

            plugin_data->output0[f_i] +=
                    plugin_data->mono_modules->reverb->output;
            plugin_data->output1[f_i] +=
                    plugin_data->mono_modules->reverb->output;

            f_i++;
        }
    }

    if((plugin_data->mono_modules->volume_smoother->last_value) != 0.0f ||
            (*plugin_data->vol_slider != 0.0f))
    {
        f_i = 0;

        while(f_i < sample_count)
        {
            v_sml_run(plugin_data->mono_modules->volume_smoother,
                    (*plugin_data->vol_slider));

            plugin_data->mono_modules->vol_linear =
                f_db_to_linear_fast(
                    (plugin_data->mono_modules->volume_smoother->last_value),
                    plugin_data->mono_modules->amp_ptr);

            plugin_data->output0[f_i] *=
                    (plugin_data->mono_modules->vol_linear);
            plugin_data->output1[f_i] *=
                    (plugin_data->mono_modules->vol_linear);
            f_i++;
        }
    }

}

PYFX_Descriptor *modulex_PYFX_descriptor(int index)
{
    PYFX_Descriptor *LMSLDescriptor = NULL;

    PYFX_PortDescriptor *port_descriptors;
    PYFX_PortRangeHint *port_range_hints;

    LMSLDescriptor =
	(PYFX_Descriptor *) malloc(sizeof(PYFX_Descriptor));
    if (LMSLDescriptor)
    {
        LMSLDescriptor->UniqueID = 123456;
	LMSLDescriptor->Name = "Modulex";
	LMSLDescriptor->Maker = "PyDAW Team";
	LMSLDescriptor->Copyright = "GNU GPL v3";
	LMSLDescriptor->PortCount = MODULEX_COUNT;

	port_descriptors = (PYFX_PortDescriptor *)
				calloc(LMSLDescriptor->PortCount, sizeof
						(PYFX_PortDescriptor));
	LMSLDescriptor->PortDescriptors =
	    (const PYFX_PortDescriptor *) port_descriptors;

	port_range_hints = (PYFX_PortRangeHint *)
				calloc(LMSLDescriptor->PortCount, sizeof
						(PYFX_PortRangeHint));
	LMSLDescriptor->PortRangeHints =
	    (const PYFX_PortRangeHint *) port_range_hints;


	port_descriptors[MODULEX_FX0_KNOB0] = 1;
	port_range_hints[MODULEX_FX0_KNOB0].DefaultValue = 64.0f;
	port_range_hints[MODULEX_FX0_KNOB0].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX0_KNOB0].UpperBound =  127.0f;

	port_descriptors[MODULEX_FX0_KNOB1] = 1;
	port_range_hints[MODULEX_FX0_KNOB1].DefaultValue = 64.0f;
	port_range_hints[MODULEX_FX0_KNOB1].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX0_KNOB1].UpperBound =  127.0f;

	port_descriptors[MODULEX_FX0_KNOB2] = 1;
	port_range_hints[MODULEX_FX0_KNOB2].DefaultValue = 64.0f;
	port_range_hints[MODULEX_FX0_KNOB2].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX0_KNOB2].UpperBound =  127.0f;

	port_descriptors[MODULEX_FX0_COMBOBOX] = 1;
	port_range_hints[MODULEX_FX0_COMBOBOX].DefaultValue = 0.0f;
	port_range_hints[MODULEX_FX0_COMBOBOX].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX0_COMBOBOX].UpperBound =
                MULTIFX3KNOB_MAX_INDEX;

	port_descriptors[MODULEX_FX1_KNOB0] = 1;
	port_range_hints[MODULEX_FX1_KNOB0].DefaultValue = 64.0f;
	port_range_hints[MODULEX_FX1_KNOB0].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX1_KNOB0].UpperBound =  127.0f;

	port_descriptors[MODULEX_FX1_KNOB1] = 1;
	port_range_hints[MODULEX_FX1_KNOB1].DefaultValue = 64.0f;
	port_range_hints[MODULEX_FX1_KNOB1].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX1_KNOB1].UpperBound =  127.0f;

	port_descriptors[MODULEX_FX1_KNOB2] = 1;
	port_range_hints[MODULEX_FX1_KNOB2].DefaultValue = 64.0f;
	port_range_hints[MODULEX_FX1_KNOB2].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX1_KNOB2].UpperBound =  127.0f;

	port_descriptors[MODULEX_FX1_COMBOBOX] = 1;
	port_range_hints[MODULEX_FX1_COMBOBOX].DefaultValue = 0.0f;
	port_range_hints[MODULEX_FX1_COMBOBOX].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX1_COMBOBOX].UpperBound =
                MULTIFX3KNOB_MAX_INDEX;

        port_descriptors[MODULEX_FX2_KNOB0] = 1;
	port_range_hints[MODULEX_FX2_KNOB0].DefaultValue = 64.0f;
	port_range_hints[MODULEX_FX2_KNOB0].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX2_KNOB0].UpperBound =  127.0f;

	port_descriptors[MODULEX_FX2_KNOB1] = 1;
	port_range_hints[MODULEX_FX2_KNOB1].DefaultValue = 64.0f;
	port_range_hints[MODULEX_FX2_KNOB1].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX2_KNOB1].UpperBound =  127.0f;

	port_descriptors[MODULEX_FX2_KNOB2] = 1;
	port_range_hints[MODULEX_FX2_KNOB2].DefaultValue = 64.0f;
	port_range_hints[MODULEX_FX2_KNOB2].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX2_KNOB2].UpperBound =  127.0f;

	port_descriptors[MODULEX_FX2_COMBOBOX] = 1;
	port_range_hints[MODULEX_FX2_COMBOBOX].DefaultValue = 0.0f;
	port_range_hints[MODULEX_FX2_COMBOBOX].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX2_COMBOBOX].UpperBound =
                MULTIFX3KNOB_MAX_INDEX;

	port_descriptors[MODULEX_FX3_KNOB0] = 1;
	port_range_hints[MODULEX_FX3_KNOB0].DefaultValue = 64.0f;
	port_range_hints[MODULEX_FX3_KNOB0].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX3_KNOB0].UpperBound =  127.0f;

	port_descriptors[MODULEX_FX3_KNOB1] = 1;
	port_range_hints[MODULEX_FX3_KNOB1].DefaultValue = 64.0f;
	port_range_hints[MODULEX_FX3_KNOB1].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX3_KNOB1].UpperBound =  127.0f;

	port_descriptors[MODULEX_FX3_KNOB2] = 1;
	port_range_hints[MODULEX_FX3_KNOB2].DefaultValue = 64.0f;
	port_range_hints[MODULEX_FX3_KNOB2].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX3_KNOB2].UpperBound =  127.0f;

	port_descriptors[MODULEX_FX3_COMBOBOX] = 1;
	port_range_hints[MODULEX_FX3_COMBOBOX].DefaultValue = 0.0f;
	port_range_hints[MODULEX_FX3_COMBOBOX].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX3_COMBOBOX].UpperBound =
                MULTIFX3KNOB_MAX_INDEX;

	port_descriptors[MODULEX_FX4_KNOB0] = 1;
	port_range_hints[MODULEX_FX4_KNOB0].DefaultValue = 64.0f;
	port_range_hints[MODULEX_FX4_KNOB0].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX4_KNOB0].UpperBound =  127.0f;

	port_descriptors[MODULEX_FX4_KNOB1] = 1;
	port_range_hints[MODULEX_FX4_KNOB1].DefaultValue = 64.0f;
	port_range_hints[MODULEX_FX4_KNOB1].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX4_KNOB1].UpperBound =  127.0f;

	port_descriptors[MODULEX_FX4_KNOB2] = 1;
	port_range_hints[MODULEX_FX4_KNOB2].DefaultValue = 64.0f;
	port_range_hints[MODULEX_FX4_KNOB2].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX4_KNOB2].UpperBound =  127.0f;

	port_descriptors[MODULEX_FX4_COMBOBOX] = 1;
	port_range_hints[MODULEX_FX4_COMBOBOX].DefaultValue = 0.0f;
	port_range_hints[MODULEX_FX4_COMBOBOX].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX4_COMBOBOX].UpperBound =
                MULTIFX3KNOB_MAX_INDEX;

	port_descriptors[MODULEX_FX5_KNOB0] = 1;
	port_range_hints[MODULEX_FX5_KNOB0].DefaultValue = 64.0f;
	port_range_hints[MODULEX_FX5_KNOB0].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX5_KNOB0].UpperBound =  127.0f;

	port_descriptors[MODULEX_FX5_KNOB1] = 1;
	port_range_hints[MODULEX_FX5_KNOB1].DefaultValue = 64.0f;
	port_range_hints[MODULEX_FX5_KNOB1].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX5_KNOB1].UpperBound =  127.0f;

	port_descriptors[MODULEX_FX5_KNOB2] = 1;
	port_range_hints[MODULEX_FX5_KNOB2].DefaultValue = 64.0f;
	port_range_hints[MODULEX_FX5_KNOB2].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX5_KNOB2].UpperBound =  127.0f;

	port_descriptors[MODULEX_FX5_COMBOBOX] = 1;
	port_range_hints[MODULEX_FX5_COMBOBOX].DefaultValue = 0.0f;
	port_range_hints[MODULEX_FX5_COMBOBOX].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX5_COMBOBOX].UpperBound =
                MULTIFX3KNOB_MAX_INDEX;

        port_descriptors[MODULEX_FX6_KNOB0] = 1;
	port_range_hints[MODULEX_FX6_KNOB0].DefaultValue = 64.0f;
	port_range_hints[MODULEX_FX6_KNOB0].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX6_KNOB0].UpperBound =  127.0f;

	port_descriptors[MODULEX_FX6_KNOB1] = 1;
	port_range_hints[MODULEX_FX6_KNOB1].DefaultValue = 64.0f;
	port_range_hints[MODULEX_FX6_KNOB1].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX6_KNOB1].UpperBound =  127.0f;

	port_descriptors[MODULEX_FX6_KNOB2] = 1;
	port_range_hints[MODULEX_FX6_KNOB2].DefaultValue = 64.0f;
	port_range_hints[MODULEX_FX6_KNOB2].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX6_KNOB2].UpperBound =  127.0f;

	port_descriptors[MODULEX_FX6_COMBOBOX] = 1;
	port_range_hints[MODULEX_FX6_COMBOBOX].DefaultValue = 0.0f;
	port_range_hints[MODULEX_FX6_COMBOBOX].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX6_COMBOBOX].UpperBound =
                MULTIFX3KNOB_MAX_INDEX;

	port_descriptors[MODULEX_FX7_KNOB0] = 1;
	port_range_hints[MODULEX_FX7_KNOB0].DefaultValue = 64.0f;
	port_range_hints[MODULEX_FX7_KNOB0].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX7_KNOB0].UpperBound =  127.0f;

	port_descriptors[MODULEX_FX7_KNOB1] = 1;
	port_range_hints[MODULEX_FX7_KNOB1].DefaultValue = 64.0f;
	port_range_hints[MODULEX_FX7_KNOB1].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX7_KNOB1].UpperBound =  127.0f;

	port_descriptors[MODULEX_FX7_KNOB2] = 1;
	port_range_hints[MODULEX_FX7_KNOB2].DefaultValue = 64.0f;
	port_range_hints[MODULEX_FX7_KNOB2].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX7_KNOB2].UpperBound =  127.0f;

	port_descriptors[MODULEX_FX7_COMBOBOX] = 1;
	port_range_hints[MODULEX_FX7_COMBOBOX].DefaultValue = 0.0f;
	port_range_hints[MODULEX_FX7_COMBOBOX].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX7_COMBOBOX].UpperBound =
                MULTIFX3KNOB_MAX_INDEX;

	port_descriptors[MODULEX_DELAY_TIME] = 1;
	port_range_hints[MODULEX_DELAY_TIME].DefaultValue = 50.0f;
	port_range_hints[MODULEX_DELAY_TIME].LowerBound =  10.0f;
	port_range_hints[MODULEX_DELAY_TIME].UpperBound =  100.0f;

	port_descriptors[MODULEX_FEEDBACK] = 1;
	port_range_hints[MODULEX_FEEDBACK].DefaultValue = -12.0f;
	port_range_hints[MODULEX_FEEDBACK].LowerBound =  -15.0f;
	port_range_hints[MODULEX_FEEDBACK].UpperBound =  0.0f;

	port_descriptors[MODULEX_DRY] = 1;
	port_range_hints[MODULEX_DRY].DefaultValue = 0.0f;
	port_range_hints[MODULEX_DRY].LowerBound =  -30.0f;
	port_range_hints[MODULEX_DRY].UpperBound =  0.0f;

	port_descriptors[MODULEX_WET] = 1;
	port_range_hints[MODULEX_WET].DefaultValue = -30.0f;
	port_range_hints[MODULEX_WET].LowerBound =  -30.0f;
	port_range_hints[MODULEX_WET].UpperBound =  0.0f;

	port_descriptors[MODULEX_DUCK] = 1;
	port_range_hints[MODULEX_DUCK].DefaultValue = -20.0f;
	port_range_hints[MODULEX_DUCK].LowerBound =  -40.0f;
	port_range_hints[MODULEX_DUCK].UpperBound =  0.0f;

	port_descriptors[MODULEX_CUTOFF] = 1;
	port_range_hints[MODULEX_CUTOFF].DefaultValue = 90.0f;
	port_range_hints[MODULEX_CUTOFF].LowerBound =  40.0f;
	port_range_hints[MODULEX_CUTOFF].UpperBound =  118.0f;

        /* Parameters for stereo */
	port_descriptors[MODULEX_STEREO] = 1;
	port_range_hints[MODULEX_STEREO].DefaultValue = 100.0f;
	port_range_hints[MODULEX_STEREO].LowerBound =  0.0f;
	port_range_hints[MODULEX_STEREO].UpperBound =  100.0f;

        port_descriptors[MODULEX_VOL_SLIDER] = 1;
	port_range_hints[MODULEX_VOL_SLIDER].DefaultValue = 0.0f;
	port_range_hints[MODULEX_VOL_SLIDER].LowerBound =  -50.0f;
	port_range_hints[MODULEX_VOL_SLIDER].UpperBound =  0.0f;

        port_descriptors[MODULEX_REVERB_TIME] = 1;
	port_range_hints[MODULEX_REVERB_TIME].DefaultValue = 50.0f;
	port_range_hints[MODULEX_REVERB_TIME].LowerBound =  0.0f;
	port_range_hints[MODULEX_REVERB_TIME].UpperBound =  100.0f;

        port_descriptors[MODULEX_REVERB_WET] = 1;
	port_range_hints[MODULEX_REVERB_WET].DefaultValue = 0.0f;
	port_range_hints[MODULEX_REVERB_WET].LowerBound =  0.0f;
	port_range_hints[MODULEX_REVERB_WET].UpperBound =  100.0f;

        port_descriptors[MODULEX_REVERB_COLOR] = 1;
	port_range_hints[MODULEX_REVERB_COLOR].DefaultValue = 50.0f;
	port_range_hints[MODULEX_REVERB_COLOR].LowerBound =  0.0f;
	port_range_hints[MODULEX_REVERB_COLOR].UpperBound =  100.0f;

	LMSLDescriptor->activate = v_modulex_activate;
	LMSLDescriptor->cleanup = v_modulex_cleanup;
	LMSLDescriptor->connect_port = v_modulex_connect_port;
        LMSLDescriptor->connect_buffer = v_modulex_connect_buffer;
	LMSLDescriptor->deactivate = NULL;
	LMSLDescriptor->instantiate = g_modulex_instantiate;
        LMSLDescriptor->panic = v_modulex_panic;
    }

    return LMSLDescriptor;
}


PYINST_Descriptor *modulex_PYINST_descriptor(int index)
{
    PYINST_Descriptor *LMSDDescriptor = NULL;

    LMSDDescriptor = (PYINST_Descriptor *) malloc(sizeof(PYINST_Descriptor));
    if (LMSDDescriptor)
    {
	LMSDDescriptor->PYINST_API_Version = 1;
	LMSDDescriptor->PYFX_Plugin = modulex_PYFX_descriptor(0);
	LMSDDescriptor->configure = NULL;
	LMSDDescriptor->run_synth = v_modulex_run;
    }

    return LMSDDescriptor;
}


/*
void v_modulex_destructor()
{
    if (LMSLDescriptor) {
	free((PYFX_PortDescriptor *) LMSLDescriptor->PortDescriptors);
	free((char **) LMSLDescriptor->PortNames);
	free((PYFX_PortRangeHint *) LMSLDescriptor->PortRangeHints);
	free(LMSLDescriptor);
    }
    if (LMSDDescriptor) {
	free(LMSDDescriptor);
    }
}
*/