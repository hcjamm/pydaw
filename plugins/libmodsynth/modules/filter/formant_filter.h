/* 
 * File:   formant_filter.h
 * Author: JeffH
 * 
 * A simple formant filter
 *
 * Created on April 10, 2013, 8:33 AM
 */

#ifndef FORMANT_FILTER_H
#define	FORMANT_FILTER_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "svf.h"
#include "../../lib/interpolate-linear.h"

static float f_formant_pitches[3][10] =
{
    {61.4815007463, 50.3695077237, 57.0, 65.7647152829, 59.892097194, 58.8633387057, 64.0195500087, 60.2218660311, 54.9116472027, 48.5454706023},
    {68.1946296497, 68.8021425265, 71.55592468, 72.7050324737, 74.224633736, 76.408607741, 80.6019976328, 81.769564049, 83.1263160229, 85.5572660335},
    {86.4414926111, 85.175079641, 85.175079641, 86.6556686271, 86.2972222721, 80.2973738117, 86.4414926111, 86.937176301, 87.4190618187, 90.2902566975}
};

/* ^^^^ Generated with this Python script:
from math import log
f_list = []
f_list.append([570, 840, 2410, "ow"])
f_list.append([300, 870, 2240, "oo"])
f_list.append([440, 1020, 2240, "u"])
f_list.append([730, 1090, 2440, "a"])
f_list.append([520, 1190, 2390, "uh"])
f_list.append([490, 1350, 1690, "er"])
f_list.append([660, 1720, 2410, "ae"])
f_list.append([530, 1840, 2480, "e"])
f_list.append([390, 1990, 2550, "i"])
f_list.append([270, 2290, 3010, "iy"])

def pydaw_hz_to_pitch(a_hz):
    return ((12.0 * log(a_hz * (1.0/440.0), 2.0)) + 57.0)

print("static float f_formant_pitches[3][" + str(len(f_list)) + "] =\n{")
for i in range(3):
    f_print = "    {"
    for f_item in f_list:
        f_print += str(pydaw_hz_to_pitch(f_item[i]))
        f_print += ", "
    print(f_print + "}, //" + f_item[3])
print "};"
 */
    
typedef struct
{
    t_state_variable_filter * filters[3][2];
    float output0;
    float output1;
    int iter;
    float pitch_tmp;
    float last_pos;
    t_lin_interpolater * lin;
}t_for_formant_filter;

t_for_formant_filter * g_for_formant_filter_get(float);
void v_for_formant_filter_set(t_for_formant_filter*, float);
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
    
    return f_result;
}

void v_for_formant_filter_set(t_for_formant_filter* a_for, float a_pos)
{
    if(a_pos != a_for->last_pos)
    {
        a_for->last_pos = a_pos;
        a_for->iter = 0;

        while(a_for->iter < 3)
        {
            a_for->pitch_tmp = f_linear_interpolate_ptr(f_formant_pitches[(a_for->iter)], a_pos, a_for->lin);
            float f_temp = f_pit_midi_note_to_hz(a_for->pitch_tmp);            
            v_svf_set_cutoff_base(a_for->filters[(a_for->iter)][0], (a_for->pitch_tmp));
            v_svf_set_cutoff_base(a_for->filters[(a_for->iter)][1], (a_for->pitch_tmp));
            v_svf_set_cutoff(a_for->filters[(a_for->iter)][0]);
            v_svf_set_cutoff(a_for->filters[(a_for->iter)][1]);
            a_for->iter++;
        }
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
}

#ifdef	__cplusplus
}
#endif

#endif	/* FORMANT_FILTER_H */

