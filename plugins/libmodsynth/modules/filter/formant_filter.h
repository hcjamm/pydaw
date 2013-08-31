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
    {{55.349957715, 80.39951181, 87.7552343327, 91.349957715, 93.2926099035}, {1.0, 0.199526231497, 0.251188643151, 0.199526231497, 0.1}, {-4.0, -3.5, -2.8, -2.33333333333, -2.33333333333}},
    {{63.7552343327, 72.5454706023, 88.0850031698, 89.6457296565, 91.6183714713}, {1.0, 0.501187233627, 0.446683592151, 0.398107170553, 0.0794328234724}, {-3.5, -3.11111111111, -2.33333333333, -2.15384615385, -2.0}},
    {{55.349957715, 67.349957715, 87.7552343327, 89.0382167797, 90.2326448623}, {1.0, 0.316227766017, 0.251188643151, 0.251188643151, 0.0501187233627}, {-7.0, -3.5, -2.8, -2.33333333333, -2.33333333333}},
    {{49.7825925179, 82.049554095, 89.0382167797, 91.6183714713, 93.0980871773}, {1.0, 0.177827941004, 0.125892541179, 0.1, 0.0316227766017}, {-7.0, -3.11111111111, -2.8, -2.33333333333, -2.33333333333}},
    {{53.0382167797, 62.3695077237, 88.408607741, 89.6457296565, 91.8826871473}, {1.0, 0.1, 0.141253754462, 0.199526231497, 0.0501187233627}, {-7.0, -4.66666666667, -2.8, -2.33333333333, -2.33333333333}},
    {{55.349957715, 66.2326448623, 86.3695077237, 87.7552343327, 89.6457296565}, {1.0, 0.281838293126, 0.0891250938134, 0.1, 0.01}, {-7.0, -3.5, -2.8, -2.33333333333, -2.33333333333}},
    {{67.349957715, 73.6327011877, 89.0382167797, 92.9013539183, 98.902237156}, {1.0, 0.63095734448, 0.1, 0.0158489319246, 0.001}, {-3.5, -3.11111111111, -2.33333333333, -2.15384615385, -2.0}},
    {{47.2130948536, 80.9013539183, 87.7552343327, 90.5188057658, 92.091272086}, {1.0, 0.0316227766017, 0.158489319246, 0.0794328234724, 0.0398107170553}, {-4.66666666667, -3.11111111111, -2.8, -2.33333333333, -2.33333333333}},
    {{55.349957715, 79.349957715, 88.408607741, 91.8826871473, 98.902237156}, {1.0, 0.063095734448, 0.0316227766017, 0.0177827941004, 0.001}, {-4.66666666667, -3.5, -2.33333333333, -1.86666666667, -1.4}},
    {{55.349957715, 79.565020611, 86.3695077237, 89.0382167797, 90.8003134396}, {1.0, 0.251188643151, 0.354813389234, 0.251188643151, 0.125892541179}, {-7.0, -3.5, -2.8, -2.33333333333, -2.33333333333}},
    {{53.0382167797, 80.39951181, 88.408607741, 93.8633981025, 98.902237156}, {1.0, 0.1, 0.0316227766017, 0.0158489319246, 0.001}, {-5.6, -2.8, -2.33333333333, -1.86666666667, -1.4}},
    {{57.3890577323, 67.349957715, 89.2227194903, 92.9013539183, 98.902237156}, {1.0, 0.354813389234, 0.158489319246, 0.0398107170553, 0.00177827941004}, {-4.0, -3.5, -2.8, -2.15384615385, -2.07407407407}},
    {{62.3695077237, 71.892097194, 85.252194871, 86.7264758444, 88.7262742773}, {1.0, 0.446683592151, 0.354813389234, 0.354813389234, 0.1}, {-4.66666666667, -4.0, -2.54545454545, -2.33333333333, -2.15384615385}},
    {{51.7552343327, 65.0382167797, 87.2827434727, 92.9013539183, 98.902237156}, {1.0, 0.251188643151, 0.0316227766017, 0.01, 0.00063095734448}, {-5.6, -4.66666666667, -1.64705882353, -1.55555555556, -1.4}},
    {{53.0382167797, 62.3695077237, 86.3695077237, 88.2475615518, 89.9416743073}, {1.0, 0.1, 0.0251188643151, 0.0398107170553, 0.0158489319246}, {-7.0, -3.5, -2.8, -2.33333333333, -2.33333333333}},
    {{51.7552343327, 65.0382167797, 88.408607741, 94.3250878763, 98.902237156}, {1.0, 0.158489319246, 0.0177827941004, 0.01, 0.001}, {-5.6, -4.66666666667, -1.64705882353, -1.55555555556, -1.4}},
    {{54.0002609639, 63.2141796584, 88.7262742773, 90.2326448623, 92.39951181}, {1.0, 0.1, 0.0707945784384, 0.0316227766017, 0.0199526231497}, {-7.0, -4.66666666667, -2.8, -2.33333333333, -2.33333333333}},
    {{48.5454706023, 81.8633981025, 89.6457296565, 92.1430280005, 93.3409009822}, {1.0, 0.063095734448, 0.063095734448, 0.0158489319246, 0.0158489319246}, {-7.0, -3.11111111111, -2.8, -2.33333333333, -2.33333333333}},
    {{57.3890577323, 67.349957715, 89.2227194903, 94.3250878763, 98.902237156}, {1.0, 0.281838293126, 0.0794328234724, 0.0794328234724, 0.00316227766017}, {-4.0, -3.5, -2.8, -2.15384615385, -2.07407407407}},
    {{48.5454706023, 84.3844244132, 89.9416743073, 94.7747843413, 98.902237156}, {1.0, 0.251188643151, 0.0501187233627, 0.0501187233627, 0.0063095734448}, {-4.66666666667, -3.11111111111, -2.8, -2.33333333333, -2.33333333333}},
    {{56.6019976328, 67.7774446318, 88.408607741, 90.2326448623, 91.8826871473}, {1.0, 0.316227766017, 0.0501187233627, 0.0794328234724, 0.0199526231497}, {-7.0, -3.5, -2.8, -2.33333333333, -2.33333333333}},
    {{53.0382167797, 83.2130948536, 89.0382167797, 93.3890577323, 98.902237156}, {1.0, 0.1, 0.177827941004, 0.01, 0.00158489319246}, {-4.66666666667, -2.8, -2.33333333333, -1.86666666667, -1.4}},
    {{64.0195500087, 73.175079641, 88.7262742773, 90.2326448623, 92.1430280005}, {1.0, 0.501187233627, 0.0707945784384, 0.063095734448, 0.0125892541179}, {-3.5, -3.11111111111, -2.33333333333, -2.15384615385, -2.0}},
    {{67.349957715, 73.6327011877, 89.6457296565, 94.7747843413, 98.902237156}, {1.0, 0.501187233627, 0.0251188643151, 0.1, 0.00316227766017}, {-3.5, -3.11111111111, -2.33333333333, -2.15384615385, -2.0}},
    {{57.0, 81.3890577323, 88.408607741, 90.2326448623, 91.8826871473}, {1.0, 0.199526231497, 0.125892541179, 0.1, 0.1}, {-4.0, -3.5, -2.8, -2.33333333333, -2.33333333333}}
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
    t_lin_interpolater * lin;
    t_audio_xfade * xfade;
}t_grw_growl_filter;

t_grw_growl_filter * g_grw_growl_filter_get(float);
void v_grw_growl_filter_set(t_grw_growl_filter*, float, float);
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
    f_result->last_wet = 0.0f;
    f_result->xfade = g_axf_get_audio_xfade(-3.0f);
    
    return f_result;
}

void v_grw_growl_filter_set(t_grw_growl_filter* a_grw, float a_pos, float a_wet)
{
    if(a_pos != a_grw->last_pos)
    {
        a_grw->last_pos = a_pos;
        a_grw->iter = 0;
        int f_pos = (int)a_pos;

        while(a_grw->iter < 5)
        {            
            //a_grw->pitch_tmp = f_linear_interpolate(pydaw_growl_table[f_pos][0][(a_grw->iter)], a_pos, a_grw->lin);                     
            //v_svf_set_cutoff_base(a_grw->filters[(a_grw->iter)], (a_grw->pitch_tmp));
            v_svf2_set_cutoff_base(a_grw->filters[(a_grw->iter)], (pydaw_growl_table[f_pos][0][(a_grw->iter)]));            
            v_svf2_set_res(a_grw->filters[(a_grw->iter)], pydaw_growl_table[f_pos][2][(a_grw->iter)]);
            v_svf2_set_cutoff(a_grw->filters[(a_grw->iter)]);
            a_grw->formant_amp[a_grw->iter] = pydaw_growl_table[f_pos][1][(a_grw->iter)];
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

