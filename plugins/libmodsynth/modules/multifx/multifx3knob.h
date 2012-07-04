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
#include "../signal_routing/audio_xfade.h"
    
/*BIG TODO:  Add a function to modify for the modulation sources*/
    
typedef struct st_mf3_multi
{
    int effect_index;    
    int channels;  //Currently only 1 or 2 are supported
    t_state_variable_filter * svf0;
    t_state_variable_filter * svf1;
    t_clipper * clipper;    
    float output0, output1;
    float control0, control1, control2;
    float control_value0, control_value1, control_value2;    
    float mod_value0, mod_value1, mod_value2;
    t_audio_xfade * xfader;
    t_smoother_linear * smoother_linear;
}t_mf3_multi;

/*A function pointer for switching between effect types*/
typedef void (*fp_mf3_run)(t_mf3_multi*,float,float);

inline void v_mf3_set(t_mf3_multi*,float,float,float);
inline void v_mf3_mod(t_mf3_multi*,float,float,float);
inline void v_mf3_commit_mod(t_mf3_multi*);
inline void v_mf3_run_off(t_mf3_multi*,float,float);
inline void v_mf3_run_lp2(t_mf3_multi*,float,float);
inline void v_mf3_run_lp4(t_mf3_multi*,float,float);
inline void v_mf3_run_hp2(t_mf3_multi*,float,float);
inline void v_mf3_run_hp4(t_mf3_multi*,float,float);
inline void v_mf3_run_bp2(t_mf3_multi*,float,float);
inline void v_mf3_run_bp4(t_mf3_multi*,float,float);
inline void v_mf3_run_eq(t_mf3_multi*,float,float);
inline void v_mf3_run_dist(t_mf3_multi*,float,float);

inline void f_mfx_transform_svf_filter(t_mf3_multi*);

//inline float f_mf3_midi_to_pitch(float);

t_mf3_multi * g_mf3_get(float);
inline fp_mf3_run g_mf3_get_function_pointer( int);

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
inline fp_mf3_run g_mf3_get_function_pointer( int a_fx_index)
{    
        switch(a_fx_index)
        {
            case 0:
                return v_mf3_run_off;
            case 1:
                return v_mf3_run_lp2;
            case 2:
                return v_mf3_run_lp4;
            case 3:
                return v_mf3_run_hp2;
            case 4:
                return v_mf3_run_hp4;
            case 5:
                return v_mf3_run_bp2;
            case 6:
                return v_mf3_run_bp4;
            case 7:
                return v_mf3_run_eq;
            case 8:
                return v_mf3_run_dist;
            default:
                /*TODO: Report error*/
                return v_mf3_run_off;
                
        }
    
}

inline void v_mf3_set(t_mf3_multi* a_mf3, float a_control0, float a_control1, float a_control2)
{
    
    a_mf3->control0 = a_control0;
    a_mf3->control1 = a_control1;
    a_mf3->control2 = a_control2;
    
    a_mf3->mod_value0 = 0.0f;
    a_mf3->mod_value1 = 0.0f;
    a_mf3->mod_value2 = 0.0f;
}

inline void v_mf3_mod(t_mf3_multi* a_mf3,float a_control0,float a_control1, float a_control2)
{    
    a_mf3->mod_value0 = (a_mf3->mod_value0) + a_control0;
    a_mf3->mod_value1 = (a_mf3->mod_value1) + a_control1;
    a_mf3->mod_value1 = (a_mf3->mod_value2) + a_control2;
}

inline void v_mf3_commit_mod(t_mf3_multi* a_mf3)
{
    a_mf3->control0 = (a_mf3->control0) + ((a_mf3->mod_value0) * 127);
    
    if((a_mf3->control0) > 127)
    {
        a_mf3->control0 = 127;
    }
    
    if((a_mf3->control0) < 0)
    {
        a_mf3->control0 = 0;
    }
    
    a_mf3->control1 = (a_mf3->control1) + ((a_mf3->mod_value1) * 127);
        
    if((a_mf3->control1) > 127)
    {
        a_mf3->control1 = 127;
    }
    
    if((a_mf3->control1) < 0)
    {
        a_mf3->control1 = 0;
    }
    
    a_mf3->control2 = (a_mf3->control2) + ((a_mf3->mod_value2) * 127);
             
    if((a_mf3->control2) > 127)
    {
        a_mf3->control2 = 127;
    }
    
    if((a_mf3->control2) < 0)
    {
        a_mf3->control2 = 0;
    }
}

inline void v_mf3_run_off(t_mf3_multi* a_mf3, float a_in0, float a_in1)
{
    a_mf3->output0 = a_in0;
    a_mf3->output1 = a_in1;
}

inline void v_mf3_run_lp2(t_mf3_multi* a_mf3, float a_in0, float a_in1)
{
    f_mfx_transform_svf_filter(a_mf3);
    
    a_mf3->output0 = v_svf_run_2_pole_lp(a_mf3->svf0, a_in0);
    a_mf3->output1 = v_svf_run_2_pole_lp(a_mf3->svf1, a_in1);
}

inline void v_mf3_run_lp4(t_mf3_multi* a_mf3, float a_in0, float a_in1)
{
    f_mfx_transform_svf_filter(a_mf3);
    
    a_mf3->output0 = v_svf_run_4_pole_lp(a_mf3->svf0, a_in0);
    a_mf3->output1 = v_svf_run_4_pole_lp(a_mf3->svf1, a_in1);
}

inline void v_mf3_run_hp2(t_mf3_multi* a_mf3, float a_in0, float a_in1)
{
    f_mfx_transform_svf_filter(a_mf3);
    
    a_mf3->output0 = v_svf_run_2_pole_hp(a_mf3->svf0, a_in0);
    a_mf3->output1 = v_svf_run_2_pole_hp(a_mf3->svf1, a_in1);
}

inline void v_mf3_run_hp4(t_mf3_multi* a_mf3, float a_in0, float a_in1)
{
    f_mfx_transform_svf_filter(a_mf3);
    
    a_mf3->output0 = v_svf_run_4_pole_hp(a_mf3->svf0, a_in0);
    a_mf3->output1 = v_svf_run_4_pole_hp(a_mf3->svf1, a_in1);
}

inline void v_mf3_run_bp2(t_mf3_multi* a_mf3, float a_in0, float a_in1)
{
    f_mfx_transform_svf_filter(a_mf3);
    
    a_mf3->output0 = v_svf_run_2_pole_bp(a_mf3->svf0, a_in0);
    a_mf3->output1 = v_svf_run_2_pole_bp(a_mf3->svf1, a_in1);
}

inline void v_mf3_run_bp4(t_mf3_multi* a_mf3, float a_in0, float a_in1)
{
    f_mfx_transform_svf_filter(a_mf3);
    
    a_mf3->output0 = v_svf_run_4_pole_bp(a_mf3->svf0, a_in0);
    a_mf3->output1 = v_svf_run_4_pole_bp(a_mf3->svf1, a_in1);
}

inline void v_mf3_run_eq(t_mf3_multi* a_mf3, float a_in0, float a_in1)
{    
    f_mfx_transform_svf_filter(a_mf3);
    v_svf_set_eq(a_mf3->svf0, (a_mf3->control_value2));
    v_svf_set_eq(a_mf3->svf1, (a_mf3->control_value2));
    
    a_mf3->output0 = v_svf_run_2_pole_eq(a_mf3->svf0, a_in0);
    a_mf3->output1 = v_svf_run_2_pole_eq(a_mf3->svf1, a_in1);
}

inline void v_mf3_run_dist(t_mf3_multi* a_mf3, float a_in0, float a_in1)
{
    v_mf3_commit_mod(a_mf3);
    a_mf3->control_value0 = ((a_mf3->control0) * 0.283464567);
    v_clp_set_in_gain(a_mf3->clipper, (a_mf3->control_value0));
    
    a_mf3->output0 = f_clp_clip(a_mf3->clipper, a_in0);
    a_mf3->output1 = f_clp_clip(a_mf3->clipper, a_in1);
}

inline void f_mfx_transform_svf_filter(t_mf3_multi* a_mf3)
{
    v_mf3_commit_mod(a_mf3);
    v_sml_run(a_mf3->smoother_linear, (((a_mf3->control0) * 0.818897638) + 20));
    //cutoff
    a_mf3->control_value0 = (a_mf3->smoother_linear->last_value);
    //res
    a_mf3->control_value1 = ((a_mf3->control1) * 0.236220472) - 30;
    
    v_svf_set_cutoff_base(a_mf3->svf0, (a_mf3->control_value0));
    v_svf_set_res(a_mf3->svf0, (a_mf3->control_value1));    
    
    v_svf_set_cutoff_base(a_mf3->svf1, (a_mf3->control_value0));
    v_svf_set_res(a_mf3->svf1, (a_mf3->control_value1));    
}


/* t_mf3_multi g_mf3_get(
 * float a_sample_rate)
 */
t_mf3_multi * g_mf3_get(float a_sample_rate)
{
    t_mf3_multi * f_result = (t_mf3_multi*)malloc(sizeof(t_mf3_multi));
    f_result->effect_index = 0; 
    f_result->channels = 2;
    f_result->svf0 = g_svf_get(a_sample_rate);
    f_result->svf1 = g_svf_get(a_sample_rate);
    f_result->clipper = g_clp_get_clipper();
    v_clp_set_clip_sym(f_result->clipper, 0.9f);
    f_result->output0 = 0.0f;
    f_result->output1 = 0.0f;
    f_result->control0 = 0.0f;
    f_result->control1 = 0.0f;
    f_result->control2 = 0.0f;
    f_result->control_value0 = 65.0f;
    f_result->control_value1 = 65.0f;
    f_result->control_value2 = 65.0f;
    f_result->mod_value0 = 0.0f;
    f_result->mod_value1 = 0.0f;
    f_result->mod_value2 = 0.0f;
    f_result->xfader = g_axf_get_audio_xfade(-3.0f);
    f_result->smoother_linear = g_sml_get_smoother_linear(a_sample_rate, 127.0f, 0.0f, 0.1f);
    
    return f_result;
}

#ifdef	__cplusplus
}
#endif

#endif	/* MULTIFX3KNOB_H */

