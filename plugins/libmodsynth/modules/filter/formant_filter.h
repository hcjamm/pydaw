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

#ifndef FORMANT_FILTER_H
#define	FORMANT_FILTER_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "svf.h"
#include "../../lib/interpolate-linear.h"
#include "../signal_routing/audio_xfade.h"

static float f_formant_pitches[3][10] =
{
    {65.7647152829, 64.0195500087, 60.2218660311, 48.5454706023, 54.9116472027, 58.8633387057, 61.4815007463, 50.3695077237, 59.892097194, 57.0},
    {72.7050324737, 80.6019976328, 81.769564049, 85.5572660335, 83.1263160229, 76.408607741, 68.1946296497, 68.8021425265, 74.224633736, 71.55592468},
    {86.6556686271, 86.4414926111, 86.937176301, 90.2902566975, 87.4190618187, 80.2973738117, 86.4414926111, 85.175079641, 86.2972222721, 85.175079641}
};

/* ^^^^ Generated with this Python script:
from math import log
f_list = []
f_list.append([730, 1090, 2440]) #"a"
f_list.append([660, 1720, 2410]) #"ae"
f_list.append([530, 1840, 2480]) #"e"
f_list.append([270, 2290, 3010]) #"iy"
f_list.append([390, 1990, 2550]) #"i"
f_list.append([490, 1350, 1690]) #"er"
f_list.append([570, 840, 2410]) #"ow"
f_list.append([300, 870, 2240]) #"oo"
f_list.append([520, 1190, 2390]) #"uh"
f_list.append([440, 1020, 2240]) #"u"

def pydaw_hz_to_pitch(a_hz):
    return ((12.0 * log(a_hz * (1.0/440.0), 2.0)) + 57.0)

print("static float f_formant_pitches[3][" + str(len(f_list)) + "] =\n{")
for i in range(3):
    f_print = "    {"
    for f_item in f_list:
        f_print += str(pydaw_hz_to_pitch(f_item[i]))
        f_print += ", "
    print(f_print + "},")
print "};"

 *  */
    
typedef struct
{
    t_state_variable_filter * filters[3][2];
    float output0;
    float output1;
    int iter;
    float pitch_tmp;
    float last_pos;
    float last_wet;
    t_lin_interpolater * lin;
    t_audio_xfade * xfade;
}t_for_formant_filter;

t_for_formant_filter * g_for_formant_filter_get(float);
void v_for_formant_filter_set(t_for_formant_filter*, float, float);
void v_for_formant_filter_run(t_for_formant_filter*, float, float);

t_for_formant_filter * g_for_formant_filter_get(float a_sr)
{
    t_for_formant_filter * f_result;
    
    if(posix_memalign((void**)&f_result, 16, (sizeof(t_for_formant_filter))) != 0)
    {
        return 0;
    }
    
    int f_i = 0;
    while(f_i < 3)    
    {
        int f_i2 = 0;
        while(f_i2 < 2)
        {
            f_result->filters[f_i][f_i2] = g_svf_get(a_sr);
            v_svf_set_res(f_result->filters[f_i][f_i2], -1.5f);
            f_i2++;
        }
        f_i++;
    }
    
    f_result->iter = 0;
    f_result->output0 = 0.0f;
    f_result->output1 = 0.0f;
    f_result->pitch_tmp = 0.0f;
    f_result->lin = g_lin_get();
    f_result->last_pos = -99.0f;
    f_result->last_wet = 0.0f;
    f_result->xfade = g_axf_get_audio_xfade(-3.0f);
    
    return f_result;
}

void v_for_formant_filter_set(t_for_formant_filter* a_for, float a_pos, float a_wet)
{
    if(a_pos != a_for->last_pos)
    {
        a_for->last_pos = a_pos;
        a_for->iter = 0;

        while(a_for->iter < 3)
        {
            a_for->pitch_tmp = f_linear_interpolate_ptr(f_formant_pitches[(a_for->iter)], a_pos, a_for->lin);                     
            v_svf_set_cutoff_base(a_for->filters[(a_for->iter)][0], (a_for->pitch_tmp));
            v_svf_set_cutoff_base(a_for->filters[(a_for->iter)][1], (a_for->pitch_tmp));
            v_svf_set_cutoff(a_for->filters[(a_for->iter)][0]);
            v_svf_set_cutoff(a_for->filters[(a_for->iter)][1]);
            a_for->iter++;
        }
    }
    
    
    if(a_for->last_wet != a_wet)
    {
        a_for->last_wet = a_wet;
        v_axf_set_xfade(a_for->xfade, a_wet);
    }
}

void v_for_formant_filter_run(t_for_formant_filter* a_for, float a_input0, float a_input1)
{
    a_for->iter = 0;
    a_for->output0 = 0.0f;
    a_for->output1 = 0.0f;
    
    while(a_for->iter < 3)
    {
        a_for->output0 += v_svf_run_4_pole_bp(a_for->filters[(a_for->iter)][0], a_input0);
        a_for->output1 += v_svf_run_4_pole_bp(a_for->filters[(a_for->iter)][1], a_input1);
        a_for->iter++;
    }
    
    a_for->output0 *= 0.33333f;
    a_for->output1 *= 0.33333f;
    
    a_for->output0 = f_axf_run_xfade(a_for->xfade, a_input0, a_for->output0);
    a_for->output1 = f_axf_run_xfade(a_for->xfade, a_input1, a_for->output1);
}

#ifdef	__cplusplus
}
#endif

#endif	/* FORMANT_FILTER_H */

