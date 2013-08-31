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
#include "svf_stereo.h"

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


static float pydaw_growl_table[25][3][5] = 
{
    {{67.35f, 73.633f, 89.038f, 92.901f, 98.902f}, {1.0f, 0.631f, 0.1f, 0.016f, 0.001f}, {-0.75, -0.556, -0.167, -0.077, 0.0}}, //alto a
    {{55.35f, 79.35f, 88.409f, 91.883f, 98.902f}, {1.0f, 0.063f, 0.032f, 0.018f, 0.001f}, {-1.333, -0.75, -0.167, 0.067, 0.3}}, //alto e
    {{53.038f, 80.4f, 88.409f, 93.863f, 98.902f}, {1.0f, 0.1f, 0.032f, 0.016f, 0.001f}, {-1.8, -0.4, -0.167, 0.067, 0.3}}, //alto i
    {{57.389f, 67.35f, 89.223f, 92.901f, 98.902f}, {1.0f, 0.355f, 0.158f, 0.04f, 0.002f}, {-1.0, -0.75, -0.4, -0.077, -0.037}}, //alto o
    {{51.755f, 65.038f, 87.283f, 92.901f, 98.902f}, {1.0f, 0.251f, 0.032f, 0.01f, 0.001f}, {-1.8, -1.333, 0.176, 0.222, 0.3}}, //alto u
    {{62.37f, 71.892f, 85.252f, 86.726f, 88.726f}, {1.0f, 0.447f, 0.355f, 0.355f, 0.1f}, {-1.333, -1.0, -0.273, -0.167, -0.077}}, //bass a
    {{55.35f, 79.565f, 86.37f, 89.038f, 90.8f}, {1.0f, 0.251f, 0.355f, 0.251f, 0.126f}, {-2.5, -0.75, -0.4, -0.167, -0.167}}, //bass e
    {{47.213f, 80.901f, 87.755f, 90.519f, 92.091f}, {1.0f, 0.032f, 0.158f, 0.079f, 0.04f}, {-1.333, -0.556, -0.4, -0.167, -0.167}}, //bass i
    {{55.35f, 66.233f, 86.37f, 87.755f, 89.646f}, {1.0f, 0.282f, 0.089f, 0.1f, 0.01f}, {-2.5, -0.75, -0.4, -0.167, -0.167}}, //bass o
    {{53.038f, 62.37f, 86.37f, 88.248f, 89.942f}, {1.0f, 0.1f, 0.025f, 0.04f, 0.016f}, {-2.5, -0.75, -0.4, -0.167, -0.167}}, //bass u
    {{64.02f, 73.175f, 88.726f, 90.233f, 92.143f}, {1.0f, 0.501f, 0.071f, 0.063f, 0.013f}, {-0.75, -0.556, -0.167, -0.077, 0.0}}, //countertenor a
    {{57.0f, 81.389f, 88.409f, 90.233f, 91.883f}, {1.0f, 0.2f, 0.126f, 0.1f, 0.1f}, {-1.0, -0.75, -0.4, -0.167, -0.167}}, //countertenor e
    {{48.545f, 81.863f, 89.646f, 92.143f, 93.341f}, {1.0f, 0.063f, 0.063f, 0.016f, 0.016f}, {-2.5, -0.556, -0.4, -0.167, -0.167}}, //countertenor i
    {{56.602f, 67.777f, 88.409f, 90.233f, 91.883f}, {1.0f, 0.316f, 0.05f, 0.079f, 0.02f}, {-2.5, -0.75, -0.4, -0.167, -0.167}}, //countertenor o
    {{54.0f, 63.214f, 88.726f, 90.233f, 92.4f}, {1.0f, 0.1f, 0.071f, 0.032f, 0.02f}, {-2.5, -1.333, -0.4, -0.167, -0.167}}, //countertenor u
    {{67.35f, 73.633f, 89.646f, 94.775f, 98.902f}, {1.0f, 0.501f, 0.025f, 0.1f, 0.003f}, {-0.75, -0.556, -0.167, -0.077, 0.0}}, //soprano a
    {{53.038f, 83.213f, 89.038f, 93.389f, 98.902f}, {1.0f, 0.1f, 0.178f, 0.01f, 0.002f}, {-1.333, -0.4, -0.167, 0.067, 0.3}}, //soprano e
    {{48.545f, 84.384f, 89.942f, 94.775f, 98.902f}, {1.0f, 0.251f, 0.05f, 0.05f, 0.006f}, {-1.333, -0.556, -0.4, -0.167, -0.167}}, //soprano i
    {{57.389f, 67.35f, 89.223f, 94.325f, 98.902f}, {1.0f, 0.282f, 0.079f, 0.079f, 0.003f}, {-1.0, -0.75, -0.4, -0.077, -0.037}}, //soprano o
    {{51.755f, 65.038f, 88.409f, 94.325f, 98.902f}, {1.0f, 0.158f, 0.018f, 0.01f, 0.001f}, {-1.8, -1.333, 0.176, 0.222, 0.3}}, //soprano u
    {{63.755f, 72.545f, 88.085f, 89.646f, 91.618f}, {1.0f, 0.501f, 0.447f, 0.398f, 0.079f}, {-0.75, -0.556, -0.167, -0.077, 0.0}}, //tenor a
    {{55.35f, 80.4f, 87.755f, 91.35f, 93.293f}, {1.0f, 0.2f, 0.251f, 0.2f, 0.1f}, {-1.0, -0.75, -0.4, -0.167, -0.167}}, //tenor e
    {{49.783f, 82.05f, 89.038f, 91.618f, 93.098f}, {1.0f, 0.178f, 0.126f, 0.1f, 0.032f}, {-2.5, -0.556, -0.4, -0.167, -0.167}}, //tenor i
    {{55.35f, 67.35f, 87.755f, 89.038f, 90.233f}, {1.0f, 0.316f, 0.251f, 0.251f, 0.05f}, {-2.5, -0.75, -0.4, -0.167, -0.167}}, //tenor o
    {{53.038f, 62.37f, 88.409f, 89.646f, 91.883f}, {1.0f, 0.1f, 0.141f, 0.2f, 0.05f}, {-2.5, -1.333, -0.4, -0.167, -0.167}} //tenor u
};

typedef struct
{    
    t_svf2_filter * filters[5];
    float formant_amp[5];
    float output0;
    float output1;
    int iter;
    float pitch_tmp;
    float last_pos;
    float last_wet;
    float last_type;
    t_lin_interpolater * lin;
    t_audio_xfade * xfade;
}t_grw_growl_filter;

t_grw_growl_filter * g_grw_growl_filter_get(float);
void v_grw_growl_filter_set(t_grw_growl_filter*, float, float, float);
void v_grw_growl_filter_run(t_grw_growl_filter*, float, float);

t_grw_growl_filter * g_grw_growl_filter_get(float a_sr)
{
    t_grw_growl_filter * f_result;
    
    if(posix_memalign((void**)&f_result, 16, (sizeof(t_grw_growl_filter))) != 0)
    {
        return 0;
    }
    
    int f_i = 0;
    while(f_i < 5)
    {
        f_result->filters[f_i] = g_svf2_get(a_sr);
        v_svf2_set_res(f_result->filters[f_i], -1.5f);
        f_result->formant_amp[f_i] = 1.0f;
        f_i++;
    }
    
    f_result->iter = 0;
    f_result->output0 = 0.0f;
    f_result->output1 = 0.0f;
    f_result->pitch_tmp = 0.0f;
    f_result->lin = g_lin_get();
    f_result->last_pos = -99.0f;
    f_result->last_type = 99.99f;
    f_result->last_wet = 0.0f;
    f_result->xfade = g_axf_get_audio_xfade(-3.0f);
    
    return f_result;
}

void v_grw_growl_filter_set(t_grw_growl_filter* a_grw, float a_pos, float a_wet, float a_type)
{
    if(a_pos != a_grw->last_pos || a_type != a_grw->last_type)
    {
        a_grw->last_pos = a_pos;
        a_grw->last_type = a_type;
        a_grw->iter = 0;
        int f_pos = (int)a_pos + (int)a_type;
        int f_pos_plus_one = f_pos + 1;
        if(f_pos_plus_one > 24)
        {
            f_pos_plus_one = 24;
        }
        
        float f_pos_frac = a_pos - (float)f_pos;

        while(a_grw->iter < 5)
        {
            v_svf2_set_cutoff_base(a_grw->filters[(a_grw->iter)], 
                    f_linear_interpolate(pydaw_growl_table[f_pos][0][(a_grw->iter)], pydaw_growl_table[f_pos_plus_one][0][(a_grw->iter)], f_pos_frac) + 12.0f);
            v_svf2_set_res(a_grw->filters[(a_grw->iter)], 
                    f_linear_interpolate(pydaw_growl_table[f_pos][2][(a_grw->iter)], pydaw_growl_table[f_pos_plus_one][2][(a_grw->iter)], f_pos_frac));
            v_svf2_set_cutoff(a_grw->filters[(a_grw->iter)]);
            a_grw->formant_amp[a_grw->iter] = 
                    f_linear_interpolate(pydaw_growl_table[f_pos][1][(a_grw->iter)], pydaw_growl_table[f_pos_plus_one][1][(a_grw->iter)], f_pos_frac);
            a_grw->iter++;
        }
    }    
    
    if(a_grw->last_wet != a_wet)
    {
        a_grw->last_wet = a_wet;
        v_axf_set_xfade(a_grw->xfade, a_wet);
    }
}

void v_grw_growl_filter_run(t_grw_growl_filter* a_grw, float a_input0, float a_input1)
{
    a_grw->iter = 0;
    a_grw->output0 = 0.0f;
    a_grw->output1 = 0.0f;
    
    while(a_grw->iter < 5)
    {
        v_svf2_run_2_pole_bp(a_grw->filters[(a_grw->iter)], a_input0, a_input1);
        a_grw->output0 += a_grw->filters[(a_grw->iter)]->output0 * a_grw->formant_amp[a_grw->iter];
        a_grw->output1 += a_grw->filters[(a_grw->iter)]->output1 * a_grw->formant_amp[a_grw->iter];
        a_grw->iter++;
    }
    
    a_grw->output0 *= 0.33333f;
    a_grw->output1 *= 0.33333f;
    
    a_grw->output0 = f_axf_run_xfade(a_grw->xfade, a_input0, a_grw->output0);
    a_grw->output1 = f_axf_run_xfade(a_grw->xfade, a_input1, a_grw->output1);
}

#ifdef	__cplusplus
}
#endif

#endif	/* FORMANT_FILTER_H */

