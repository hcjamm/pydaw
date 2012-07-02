/* 
 * File:   multifx3knob.h
 * Author: Jeff Hubbard
 * 
 * This corresponds to a specific multi-effect unit that accepts 3 knobs as input with the range 0-127
 *
 * Created on March 29, 2012, 6:34 PM
 */

#ifndef MULTIFX3KNOB_H
#define	MULTIFX3KNOB_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "../filter/svf.h"
#include "../filter/comb_filter.h"
#include "../distortion/clipper.h"
#include "../../lib/smoother-linear.h"
    
/*BIG TODO:  Add a function to modify for the modulation sources*/
    
typedef struct st_mf3_multi
{
    int effect_index;    
    int channels;  //Currently only 1 or 2 are supported
    t_state_variable_filter * svf0;
    t_state_variable_filter * svf1;
    t_clipper * clp0;
    t_clipper * clp1;
    float output0, output1;
    fp_mf3_run function_ptr;
    float control0, control1, control2;
    float control_value0, control_value1, control_value2;    
    float mod_value0, mod_value1, mod_value2;
    t_smoother_linear * smoother_linear;
}t_mf3_multi;

/*A function pointer for switching between effect types*/
typedef inline void (*fp_mf3_run)(t_mf3_multi*,float,float,float);

inline void v_mf3_set(t_mf3_multi*,int,int);
inline void v_mf3_run_off(t_mf3_multi*,float,float,float,float,float);
inline void v_mf3_run_lp2(t_mf3_multi*,float,float,float,float,float);
inline void v_mf3_run_lp4(t_mf3_multi*,float,float,float,float,float);
inline void v_mf3_run_hp2(t_mf3_multi*,float,float,float,float,float);
inline void v_mf3_run_hp4(t_mf3_multi*,float,float,float,float,float);
inline void v_mf3_run_bp2(t_mf3_multi*,float,float,float,float,float);
inline void v_mf3_run_bp4(t_mf3_multi*,float,float,float,float,float);
inline void v_mf3_run_eq(t_mf3_multi*,float,float,float,float,float);
inline void v_mf3_run_dist(t_mf3_multi*,float,float,float,float,float);

inline float f_mfx_transform_filter_cutoff(float);
inline float f_mfx_transform_filter_res(float);
inline float f_mfx_transform_eq_gain(float);
inline float f_mfx_transform_dry_wet(float);
inline float f_mfx_transform_comb_cutoff(float);
inline float f_mfx_transform_dist_gain(float);
inline float f_mfx_transform_ignored_knob(float);

//inline float f_mf3_midi_to_pitch(float);

t_mf3_multi g_mf3_get(float);


/* void v_mf3_set(t_fx3_multi* a_mf3, 
 * int a_fx_index, //see below
 * int a_channels) //1 or 2
 * The effect indexes are:
0  "Off"
1  "LP2"
2  "LP4"
3  "HP2"
4  "HP4"
5  "BP2"
6  "BP4"
7  "EQ"
8  "Distortion"
 */
inline void v_mf3_set(t_mf3_multi* a_mf3, int a_fx_index, int a_channels)
{
    if((a_mf3->effect_index) != a_fx_index)
    {
        a_mf3->effect_index = a_fx_index;
        
        switch(a_fx_index)
        {
            case 0:
                a_mf3->function_ptr = v_mf3_run_off;
                break;
            case 1:
                a_mf3->function_ptr = v_mf3_run_lp2;
                break;
            case 2:
                a_mf3->function_ptr = v_mf3_run_lp4;
                break;
            case 3:
                a_mf3->function_ptr = v_mf3_run_hp2;
                break;
            case 4:
                a_mf3->function_ptr = v_mf3_run_hp4;
                break;
            case 5:
                a_mf3->function_ptr = v_mf3_run_bp2;
                break;
            case 6:
                a_mf3->function_ptr = v_mf3_run_bp4;
                break;
            case 7:
                a_mf3->function_ptr = v_mf3_run_eq;
                break;
            case 8:
                a_mf3->function_ptr = v_mf3_run_dist;
                break;
            default:
                /*TODO: Report error*/
                break;
        }
    }
    
    if((a_mf3->channels) != a_channels)
    {
        a_mf3->channels = a_channels;
        /*TODO:  function pointers, etc...*/
    }
}

inline void v_mf3_run_off(t_mf3_multi* a_mf3, float a_in0, float a_in1, float a_knob1,float a_knob2, float a_knob3)
{
    a_mf3->output0 = 0.0f;
    a_mf3->output1 = 0.0f;
}

inline void v_mf3_run_lp2(t_mf3_multi* a_mf3, float a_in0, float a_in1, float a_knob1,float a_knob2, float a_knob3)
{
    a_mf3->control_value0 = f_mfx_transform_filter_cutoff(a_knob1);
    a_mf3->control_value1 = f_mfx_transform_filter_res(a_knob2 );
    
    v_svf_set_cutoff_base(a_mf3->svf0, (a_mf3->control_value0));
    v_svf_set_res(a_mf3->svf0, (a_mf3->control_value1));
    a_mf3->output0 = v_svf_run_2_pole_lp(a_mf3->svf0, a_in0);
    
    v_svf_set_cutoff_base(a_mf3->svf1, (a_mf3->control_value0));
    v_svf_set_res(a_mf3->svf1, (a_mf3->control_value1));
    a_mf3->output1 = v_svf_run_2_pole_lp(a_mf3->svf1, a_in1);
}

inline void v_mf3_run_lp4(t_mf3_multi* a_mf3, float a_in0, float a_in1, float a_knob1,float a_knob2, float a_knob3);
inline void v_mf3_run_hp2(t_mf3_multi* a_mf3, float a_in0, float a_in1, float a_knob1,float a_knob2, float a_knob3);
inline void v_mf3_run_hp4(t_mf3_multi* a_mf3, float a_in0, float a_in1, float a_knob1,float a_knob2, float a_knob3);
inline void v_mf3_run_bp2(t_mf3_multi* a_mf3, float a_in0, float a_in1, float a_knob1,float a_knob2, float a_knob3);
inline void v_mf3_run_bp4(t_mf3_multi* a_mf3, float a_in0, float a_in1, float a_knob1,float a_knob2, float a_knob3);
inline void v_mf3_run_eq(t_mf3_multi* a_mf3, float a_in0, float a_in1, float a_knob1,float a_knob2, float a_knob3);
inline void v_mf3_run_dist(t_mf3_multi* a_mf3, float a_in0, float a_in1, float a_knob1,float a_knob2, float a_knob3);

//TODO:  These functions weren't exact, go back and calculate the desired values
inline float f_mfx_transform_filter_cutoff(float a_knob)
{
    return (a_knob * 0.787401575) + 20;
}

inline float f_mfx_transform_filter_res(float a_knob)
{
    return (a_knob * 0.236220472) - 31;
}

/* t_mf3_multi g_mf3_get(
 * float a_sample_rate)
 */
t_mf3_multi g_mf3_get(float a_sample_rate)
{
    
}

#ifdef	__cplusplus
}
#endif

#endif	/* MULTIFX3KNOB_H */

