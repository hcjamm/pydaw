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

#ifndef SYNTH_H
#define	SYNTH_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "../../include/pydaw_plugin.h"
#include "libmodsynth.h"

#define MODULEX_SLOW_INDEX_ITERATIONS 30

#define MODULEX_INPUT0  0
#define MODULEX_INPUT1  1
#define MODULEX_OUTPUT0  2
#define MODULEX_OUTPUT1  3

#define MODULEX_FIRST_CONTROL_PORT 4
#define MODULEX_FX0_KNOB0  4
#define MODULEX_FX0_KNOB1  5
#define MODULEX_FX0_KNOB2  6
#define MODULEX_FX0_COMBOBOX 7
#define MODULEX_FX1_KNOB0  8
#define MODULEX_FX1_KNOB1  9
#define MODULEX_FX1_KNOB2  10
#define MODULEX_FX1_COMBOBOX 11
#define MODULEX_FX2_KNOB0  12
#define MODULEX_FX2_KNOB1  13
#define MODULEX_FX2_KNOB2  14
#define MODULEX_FX2_COMBOBOX 15
#define MODULEX_FX3_KNOB0  16
#define MODULEX_FX3_KNOB1  17
#define MODULEX_FX3_KNOB2  18
#define MODULEX_FX3_COMBOBOX 19
#define MODULEX_FX4_KNOB0  20
#define MODULEX_FX4_KNOB1  21
#define MODULEX_FX4_KNOB2  22
#define MODULEX_FX4_COMBOBOX 23
#define MODULEX_FX5_KNOB0  24
#define MODULEX_FX5_KNOB1  25
#define MODULEX_FX5_KNOB2  26
#define MODULEX_FX5_COMBOBOX 27
#define MODULEX_FX6_KNOB0  28
#define MODULEX_FX6_KNOB1  29
#define MODULEX_FX6_KNOB2  30
#define MODULEX_FX6_COMBOBOX 31
#define MODULEX_FX7_KNOB0  32
#define MODULEX_FX7_KNOB1  33
#define MODULEX_FX7_KNOB2  34
#define MODULEX_FX7_COMBOBOX 35
#define MODULEX_DELAY_TIME  36
#define MODULEX_FEEDBACK  37
#define MODULEX_DRY  38
#define MODULEX_WET  39
#define MODULEX_DUCK  40
#define MODULEX_CUTOFF  41
#define MODULEX_STEREO 42

#define MODULEX_VOL_SLIDER 43

#define MODULEX_REVERB_TIME 44
#define MODULEX_REVERB_WET 45
#define MODULEX_REVERB_COLOR 46

#define MODULEX_EQ_ON 47
#define MODULEX_EQ1_FREQ 48
#define MODULEX_EQ1_RES 49
#define MODULEX_EQ1_GAIN 50
#define MODULEX_EQ1_FREQ 48
#define MODULEX_EQ1_RES 49
#define MODULEX_EQ1_GAIN 50
#define MODULEX_EQ2_FREQ 51
#define MODULEX_EQ2_RES 52
#define MODULEX_EQ2_GAIN 53
#define MODULEX_EQ3_FREQ 54
#define MODULEX_EQ3_RES 55
#define MODULEX_EQ3_GAIN 56
#define MODULEX_EQ4_FREQ 57
#define MODULEX_EQ4_RES 58
#define MODULEX_EQ4_GAIN 59
#define MODULEX_EQ5_FREQ 60
#define MODULEX_EQ5_RES 61
#define MODULEX_EQ5_GAIN 62
#define MODULEX_EQ6_FREQ 63
#define MODULEX_EQ6_RES 64
#define MODULEX_EQ6_GAIN 65
#define MODULEX_SPECTRUM_ENABLED 66
#define MODULEX_GATE_NOTE 67
#define MODULEX_GATE_MODE 68
#define MODULEX_GATE_WET 69
#define MODULEX_GATE_PITCH 70
#define MODULEX_GLITCH_ON 71
#define MODULEX_GLITCH_NOTE 72
#define MODULEX_GLITCH_TIME 73
#define MODULEX_REVERB_DRY 74

#define MODULEX_LAST_CONTROL_PORT 74
/* must be 1 + highest value above
 * CHANGE THIS IF YOU ADD OR TAKE AWAY ANYTHING*/
#define MODULEX_COUNT 75

typedef struct
{
    PYFX_Data *output0;
    PYFX_Data *output1;

    PYFX_Data *fx_knob0[8];
    PYFX_Data *fx_knob1[8];
    PYFX_Data *fx_knob2[8];
    PYFX_Data *fx_combobox[8];

    PYFX_Data *delay_time;
    PYFX_Data *feedback;
    PYFX_Data *dry;
    PYFX_Data *wet;
    PYFX_Data *duck;
    PYFX_Data *cutoff;
    PYFX_Data *stereo;

    PYFX_Data *vol_slider;

    PYFX_Data *reverb_time;
    PYFX_Data *reverb_dry;
    PYFX_Data *reverb_wet;
    PYFX_Data *reverb_color;

    PYFX_Data *eq_on;

    PYFX_Data *eq_freq[6];
    PYFX_Data *eq_res[6];
    PYFX_Data *eq_gain[6];
    PYFX_Data *spectrum_analyzer_on;

    PYFX_Data *gate_note;
    PYFX_Data *gate_mode;
    PYFX_Data *gate_wet;
    PYFX_Data *gate_pitch;

    PYFX_Data *glitch_on;
    PYFX_Data *glitch_note;
    PYFX_Data *glitch_time;

    float fs;
    t_modulex_mono_modules * mono_modules;

    int i_mono_out;
    int i_buffer_clear;

    int i_slow_index;
    int is_on;

    int midi_event_types[200];
    int midi_event_ticks[200];
    float midi_event_values[200];
    int midi_event_ports[200];
    int midi_event_count;
    int track_num;
    fp_queue_message queue_func;

    float * port_table;
} t_modulex;

#ifdef	__cplusplus
}
#endif

#endif	/* SYNTH_H */

