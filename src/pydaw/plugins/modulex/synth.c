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

#define MODULEX_EVENT_GATE_ON 1001
#define MODULEX_EVENT_GATE_OFF 1002
#define MODULEX_EVENT_GLITCH_ON 1003
#define MODULEX_EVENT_GLITCH_OFF 1004

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

        case MODULEX_EQ_ON: plugin->eq_on = data; break;

        case MODULEX_EQ1_FREQ: plugin->eq_freq[0] = data; break;
        case MODULEX_EQ2_FREQ: plugin->eq_freq[1] = data; break;
        case MODULEX_EQ3_FREQ: plugin->eq_freq[2] = data; break;
        case MODULEX_EQ4_FREQ: plugin->eq_freq[3] = data; break;
        case MODULEX_EQ5_FREQ: plugin->eq_freq[4] = data; break;
        case MODULEX_EQ6_FREQ: plugin->eq_freq[5] = data; break;

        case MODULEX_EQ1_RES: plugin->eq_res[0] = data; break;
        case MODULEX_EQ2_RES: plugin->eq_res[1] = data; break;
        case MODULEX_EQ3_RES: plugin->eq_res[2] = data; break;
        case MODULEX_EQ4_RES: plugin->eq_res[3] = data; break;
        case MODULEX_EQ5_RES: plugin->eq_res[4] = data; break;
        case MODULEX_EQ6_RES: plugin->eq_res[5] = data; break;

        case MODULEX_EQ1_GAIN: plugin->eq_gain[0] = data; break;
        case MODULEX_EQ2_GAIN: plugin->eq_gain[1] = data; break;
        case MODULEX_EQ3_GAIN: plugin->eq_gain[2] = data; break;
        case MODULEX_EQ4_GAIN: plugin->eq_gain[3] = data; break;
        case MODULEX_EQ5_GAIN: plugin->eq_gain[4] = data; break;
        case MODULEX_EQ6_GAIN: plugin->eq_gain[5] = data; break;
        case MODULEX_SPECTRUM_ENABLED: plugin->spectrum_analyzer_on = data; break;

        case MODULEX_GATE_MODE: plugin->gate_mode = data; break;
        case MODULEX_GATE_NOTE: plugin->gate_note = data; break;
        case MODULEX_GATE_PITCH: plugin->gate_pitch = data; break;
        case MODULEX_GATE_WET: plugin->gate_wet = data; break;

        case MODULEX_GLITCH_ON: plugin->glitch_on = data; break;
        case MODULEX_GLITCH_NOTE: plugin->glitch_note = data; break;
        case MODULEX_GLITCH_TIME: plugin->glitch_time = data; break;

        case MODULEX_REVERB_DRY: plugin->reverb_dry = data; break;
    }
}

static PYFX_Handle g_modulex_instantiate(PYFX_Descriptor * descriptor,
        int s_rate, fp_get_wavpool_item_from_host a_host_wavpool_func,
        int a_track_num, fp_queue_message a_queue_func)
{
    t_modulex *plugin_data = (t_modulex*)malloc(sizeof(t_modulex));

    plugin_data->fs = s_rate;
    plugin_data->track_num = a_track_num;
    plugin_data->queue_func = a_queue_func;
    return (PYFX_Handle) plugin_data;
}

static void v_modulex_activate(PYFX_Handle instance, float * a_port_table)
{
    t_modulex *plugin_data = (t_modulex *) instance;

    plugin_data->port_table = a_port_table;

    plugin_data->mono_modules =
            v_modulex_mono_init(plugin_data->fs, plugin_data->track_num);

    plugin_data->i_slow_index = MODULEX_SLOW_INDEX_ITERATIONS;
    plugin_data->is_on = 0;
}

static void v_modulex_check_if_on(t_modulex *plugin_data)
{
    if((int)*plugin_data->gate_mode != 0)
    {
        plugin_data->is_on = 1;
    }

    if((int)*plugin_data->glitch_on != 0)
    {
        plugin_data->is_on = 1;
    }

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

static inline void v_modulex_run_eq(t_modulex *plugin_data, int sample_count)
{
    int f_i = 0;

    while(f_i < MODULEX_EQ_COUNT)
    {
        if(*plugin_data->eq_gain[f_i] != 0.0f)
        {
            int f_i2 = 0;

            v_pkq_calc_coeffs(plugin_data->mono_modules->eqs[f_i],
                    *plugin_data->eq_freq[f_i],
                    *plugin_data->eq_res[f_i] * 0.01f,
                    *plugin_data->eq_gain[f_i]);

            while(f_i2 < sample_count)
            {
                v_pkq_run(plugin_data->mono_modules->eqs[f_i],
                        plugin_data->output0[f_i2], plugin_data->output1[f_i2]);
                plugin_data->output0[f_i2] =
                        plugin_data->mono_modules->eqs[f_i]->output0;
                plugin_data->output1[f_i2] =
                        plugin_data->mono_modules->eqs[f_i]->output1;
                f_i2++;
            }
        }
        f_i++;
    }
}

static inline void v_modulex_run_gate(t_modulex *plugin_data,
        float a_in0, float a_in1)
{
    v_sml_run(plugin_data->mono_modules->gate_wet_smoother,
            *plugin_data->gate_wet * 0.01f);
    v_gat_set(plugin_data->mono_modules->gate, *plugin_data->gate_pitch,
            plugin_data->mono_modules->gate_wet_smoother->last_value);
    v_gat_run(plugin_data->mono_modules->gate,
            plugin_data->mono_modules->gate_on, a_in0, a_in1);
}

static inline void v_modulex_run_glitch(t_modulex *plugin_data,
        float a_in0, float a_in1)
{
    v_sml_run(plugin_data->mono_modules->glitch_time_smoother,
            *plugin_data->glitch_time * 0.01f);
    v_glc_glitch_v2_set(plugin_data->mono_modules->glitch,
            plugin_data->mono_modules->glitch_time_smoother->last_value);
    v_glc_glitch_v2_run(plugin_data->mono_modules->glitch, a_in0, a_in1);
}

static void v_modulex_run(PYFX_Handle instance, int sample_count,
		  t_pydaw_seq_event *events, int event_count)
{
    t_modulex *plugin_data = (t_modulex*)instance;

    int event_pos = 0;
    int midi_event_pos = 0;
    plugin_data->midi_event_count = 0;

    int f_gate_note = (int)*plugin_data->gate_note;
    int f_gate_on = (int)*plugin_data->gate_mode;
    int f_glitch_note = (int)*plugin_data->glitch_note;
    int f_glitch_on = (int)*plugin_data->glitch_on;

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
        else if (events[event_pos].type == PYDAW_EVENT_NOTEON)
        {
            if(events[event_pos].note == f_gate_note)
            {
                plugin_data->midi_event_types[plugin_data->midi_event_count] =
                        MODULEX_EVENT_GATE_ON;
                plugin_data->midi_event_ticks[plugin_data->midi_event_count] =
                        events[event_pos].tick;
                float f_db = (0.283464567f *  // 1.0f / 127.0f
                    ((float)events[event_pos].velocity)) - 28.3464567f;
                plugin_data->midi_event_values[plugin_data->midi_event_count] =
                    f_db_to_linear_fast(f_db,
                        plugin_data->mono_modules->amp_ptr);

                plugin_data->midi_event_count++;
            }
            if(events[event_pos].note == f_glitch_note)
            {
                plugin_data->midi_event_types[plugin_data->midi_event_count] =
                        MODULEX_EVENT_GLITCH_ON;
                plugin_data->midi_event_ticks[plugin_data->midi_event_count] =
                        events[event_pos].tick;
                float f_db = (0.283464567f *  // 1.0f / 127.0f
                    ((float)events[event_pos].velocity)) - 28.3464567f;
                plugin_data->midi_event_values[plugin_data->midi_event_count] =
                    f_db_to_linear_fast(f_db,
                        plugin_data->mono_modules->amp_ptr);

                plugin_data->midi_event_count++;
            }
        }
        else if (events[event_pos].type == PYDAW_EVENT_NOTEOFF)
        {
            if(events[event_pos].note == f_gate_note)
            {
                plugin_data->midi_event_types[plugin_data->midi_event_count] =
                        MODULEX_EVENT_GATE_OFF;
                plugin_data->midi_event_ticks[
                        plugin_data->midi_event_count] = events[event_pos].tick;
                plugin_data->midi_event_values[
                        plugin_data->midi_event_count] = 0.0f;
                plugin_data->midi_event_count++;
            }
            if(events[event_pos].note == f_glitch_note)
            {
                plugin_data->midi_event_types[plugin_data->midi_event_count] =
                        MODULEX_EVENT_GLITCH_OFF;
                plugin_data->midi_event_ticks[
                        plugin_data->midi_event_count] = events[event_pos].tick;
                plugin_data->midi_event_values[
                        plugin_data->midi_event_count] = 0.0f;
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

    int f_eq_state = (int)(*plugin_data->eq_on);

    if(f_eq_state == 1)
    {
        v_modulex_run_eq(plugin_data, sample_count);
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
                else if(plugin_data->midi_event_types[midi_event_pos] ==
                        MODULEX_EVENT_GATE_ON)
                {
                    plugin_data->mono_modules->gate_on =
                            plugin_data->midi_event_values[midi_event_pos];
                }
                else if(plugin_data->midi_event_types[midi_event_pos] ==
                        MODULEX_EVENT_GATE_OFF)
                {
                    plugin_data->mono_modules->gate_on =
                            plugin_data->midi_event_values[midi_event_pos];
                }
                else if(plugin_data->midi_event_types[midi_event_pos] ==
                        MODULEX_EVENT_GLITCH_ON)
                {
                    v_glc_glitch_v2_retrigger(
                            plugin_data->mono_modules->glitch);
                    plugin_data->mono_modules->glitch_on =
                            plugin_data->midi_event_values[midi_event_pos];
                }
                else if(plugin_data->midi_event_types[midi_event_pos] ==
                        MODULEX_EVENT_GLITCH_OFF)
                {
                    plugin_data->mono_modules->glitch_on =
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
                    plugin_data->mono_modules->smoothers[f_i][2]->last_value);

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

            if(f_glitch_on && plugin_data->mono_modules->glitch_on > 0.0f)
            {
                v_modulex_run_glitch(plugin_data,
                        plugin_data->mono_modules->current_sample0,
                        plugin_data->mono_modules->current_sample1);
                plugin_data->mono_modules->current_sample0 =
                        plugin_data->mono_modules->glitch->output;
                plugin_data->mono_modules->current_sample1 =
                        plugin_data->mono_modules->glitch->output;
            }

            if(f_gate_on)
            {
                v_modulex_run_gate(plugin_data,
                        plugin_data->mono_modules->current_sample0,
                        plugin_data->mono_modules->current_sample1);
                plugin_data->mono_modules->current_sample0 =
                        plugin_data->mono_modules->gate->output[0];
                plugin_data->mono_modules->current_sample1 =
                        plugin_data->mono_modules->gate->output[1];
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

        float f_dry_vol;
        float f_dry_db = (*plugin_data->reverb_dry);

        if(f_dry_db == 0.0f)
        {
            f_dry_vol = 0.0f;
        }
        else
        {
            v_sml_run(plugin_data->mono_modules->reverb_dry_smoother,
                (*plugin_data->reverb_dry));
            f_dry_vol = f_db_to_linear_fast(
                (plugin_data->mono_modules->reverb_dry_smoother->last_value
                    * 0.4f) - 40.0f,
                plugin_data->mono_modules->amp_ptr);
        }

        int f_i = 0;
        while(f_i < sample_count)
        {
            v_rvb_reverb_run(plugin_data->mono_modules->reverb,
                plugin_data->output0[f_i],
                plugin_data->output1[f_i]);

            plugin_data->output0[f_i] =
                    (plugin_data->output0[f_i] * f_dry_vol) +
                    plugin_data->mono_modules->reverb->output;
            plugin_data->output1[f_i] =
                    (plugin_data->output1[f_i] * f_dry_vol) +
                    plugin_data->mono_modules->reverb->output;

            f_i++;
        }
    }

    if(f_eq_state == 2)
    {
        v_modulex_run_eq(plugin_data, sample_count);
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

    if((int)(*plugin_data->spectrum_analyzer_on))
    {
        v_spa_run(plugin_data->mono_modules->spectrum_analyzer,
                plugin_data->output0, plugin_data->output1, sample_count);
        if(plugin_data->mono_modules->spectrum_analyzer->str_buf[0] != '\0')
        {
            plugin_data->queue_func("ui",
                plugin_data->mono_modules->spectrum_analyzer->str_buf);
            plugin_data->mono_modules->spectrum_analyzer->str_buf[0] = '\0';
        }
    }

}

PYFX_Descriptor *modulex_PYFX_descriptor(int index)
{
    PYFX_Descriptor *LMSLDescriptor =
            pydaw_get_pyfx_descriptor(123456, "Modulex", MODULEX_COUNT);

    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_FX0_KNOB0, 64.0f, 0.0f, 127.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_FX0_KNOB1, 64.0f, 0.0f, 127.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_FX0_KNOB2, 64.0f, 0.0f, 127.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_FX0_COMBOBOX, 0.0f, 0.0f, MULTIFX3KNOB_MAX_INDEX);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_FX1_KNOB0, 64.0f, 0.0f, 127.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_FX1_KNOB1, 64.0f, 0.0f, 127.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_FX1_KNOB2, 64.0f, 0.0f, 127.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_FX1_COMBOBOX, 0.0f, 0.0f, MULTIFX3KNOB_MAX_INDEX);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_FX2_KNOB0, 64.0f, 0.0f, 127.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_FX2_KNOB1, 64.0f, 0.0f, 127.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_FX2_KNOB2, 64.0f, 0.0f, 127.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_FX2_COMBOBOX, 0.0f, 0.0f, MULTIFX3KNOB_MAX_INDEX);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_FX3_KNOB0, 64.0f, 0.0f, 127.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_FX3_KNOB1, 64.0f, 0.0f, 127.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_FX3_KNOB2, 64.0f, 0.0f, 127.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_FX3_COMBOBOX, 0.0f, 0.0f, MULTIFX3KNOB_MAX_INDEX);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_FX4_KNOB0, 64.0f, 0.0f, 127.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_FX4_KNOB1, 64.0f, 0.0f, 127.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_FX4_KNOB2, 64.0f, 0.0f, 127.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_FX4_COMBOBOX, 0.0f, 0.0f, MULTIFX3KNOB_MAX_INDEX);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_FX5_KNOB0, 64.0f, 0.0f, 127.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_FX5_KNOB1, 64.0f, 0.0f, 127.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_FX5_KNOB2, 64.0f, 0.0f, 127.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_FX5_COMBOBOX, 0.0f, 0.0f, MULTIFX3KNOB_MAX_INDEX);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_FX6_KNOB0, 64.0f, 0.0f, 127.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_FX6_KNOB1, 64.0f, 0.0f, 127.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_FX6_KNOB2, 64.0f, 0.0f, 127.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_FX6_COMBOBOX, 0.0f, 0.0f, MULTIFX3KNOB_MAX_INDEX);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_FX7_KNOB0, 64.0f, 0.0f, 127.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_FX7_KNOB1, 64.0f, 0.0f, 127.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_FX7_KNOB2, 64.0f, 0.0f, 127.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_FX7_COMBOBOX, 0.0f, 0.0f, MULTIFX3KNOB_MAX_INDEX);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_DELAY_TIME, 50.0f, 10.0f, 100.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_FEEDBACK, -12.0f, -15.0f, 0.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_DRY, 0.0f, -30.0f, 0.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_WET, -30.0f, -30.0f, 0.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_DUCK, -20.0f, -40.0f, 0.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_CUTOFF, 90.0f, 40.0f, 118.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_STEREO, 100.0f, 0.0f, 100.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_VOL_SLIDER, 0.0f, -50.0f, 0.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_REVERB_TIME, 50.0f, 0.0f, 100.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_REVERB_WET, 0.0f, 0.0f, 100.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_REVERB_COLOR, 50.0f, 0.0f, 100.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_EQ_ON, 0.0f, 0.0f, 2.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_EQ1_FREQ, 24.0f, 20.0f, 120.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_EQ2_FREQ, 42.0f, 20.0f, 120.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_EQ3_FREQ, 60.0f, 20.0f, 120.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_EQ4_FREQ, 78.0f, 20.0f, 120.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_EQ5_FREQ, 96.0f, 20.0f, 120.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_EQ6_FREQ, 114.0f, 20.0f, 120.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_EQ1_RES, 300.0f, 100.0f, 600.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_EQ2_RES, 300.0f, 100.0f, 600.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_EQ3_RES, 300.0f, 100.0f, 600.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_EQ4_RES, 300.0f, 100.0f, 600.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_EQ5_RES, 300.0f, 100.0f, 600.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_EQ6_RES, 300.0f, 100.0f, 600.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_EQ1_GAIN, 0.0f, -24.0f, 24.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_EQ2_GAIN, 0.0f, -24.0f, 24.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_EQ3_GAIN, 0.0f, -24.0f, 24.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_EQ4_GAIN, 0.0f, -24.0f, 24.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_EQ5_GAIN, 0.0f, -24.0f, 24.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_EQ6_GAIN, 0.0f, -24.0f, 24.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_SPECTRUM_ENABLED, 0.0f, 0.0f, 1.0f);

    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_GATE_NOTE, 120.0f, 0.0f, 120.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_GATE_MODE, 0.0f, 0.0f, 2.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_GATE_WET, 0.0f, 0.0f, 100.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_GATE_PITCH, 60.0f, 20.0f, 120.0f);

    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_GLITCH_ON, 0.0f, 0.0f, 1.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_GLITCH_NOTE, 120.0f, 0.0f, 120.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_GLITCH_TIME, 10.0f, 1.0f, 25.0f);
    pydaw_set_pyfx_port(LMSLDescriptor, MODULEX_REVERB_DRY, 100.0f, 0.0f, 100.0f);


    LMSLDescriptor->activate = v_modulex_activate;
    LMSLDescriptor->cleanup = v_modulex_cleanup;
    LMSLDescriptor->connect_port = v_modulex_connect_port;
    LMSLDescriptor->connect_buffer = v_modulex_connect_buffer;
    LMSLDescriptor->deactivate = NULL;
    LMSLDescriptor->instantiate = g_modulex_instantiate;
    LMSLDescriptor->panic = v_modulex_panic;

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