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
    
/*The highest index for selecting the effect type*/
#define MULTIFX3KNOB_MAX_INDEX 17
#define MULTIFX3KNOB_KNOB_COUNT 3

#include "../filter/svf.h"
#include "../filter/comb_filter.h"
#include "../distortion/clipper.h"
#include "../signal_routing/audio_xfade.h"
#include "../../lib/amp.h"
#include "../signal_routing/amp_and_panner.h"
#include "../filter/peak_eq.h"
#include "../dynamics/limiter.h"
#include "../distortion/saturator.h"
#include "../filter/formant_filter.h"
#include "../delay/chorus.h"
#include "../distortion/glitch.h"
#include "../distortion/ring_mod.h"
#include "../distortion/lofi.h"
    
/*BIG TODO:  Add a function to modify for the modulation sources*/
    
typedef struct st_mf3_multi
{
    int effect_index;    
    int channels;  //Currently only 1 or 2 are supported
    t_state_variable_filter * svf0;
    t_state_variable_filter * svf1;
    t_comb_filter * comb_filter0;
    t_comb_filter * comb_filter1;  
    t_pkq_peak_eq * eq0;
    t_clipper * clipper;    
    t_lim_limiter * limiter;
    t_sat_saturator * saturator;
    float output0, output1;
    float control[MULTIFX3KNOB_KNOB_COUNT];
    float control_value[MULTIFX3KNOB_KNOB_COUNT];
    float mod_value[MULTIFX3KNOB_KNOB_COUNT];
    t_audio_xfade * xfader;
    t_amp_and_panner * amp_and_panner;
    float outgain;  //For anything with an outgain knob    
    t_amp * amp_ptr;    
    t_for_formant_filter * formant_filter;
    t_crs_chorus * chorus;
    t_glc_glitch * glitch;
    t_rmd_ring_mod * ring_mod;
    t_lfi_lofi * lofi;
}t_mf3_multi;

/*A function pointer for switching between effect types*/
typedef void (*fp_mf3_run)(t_mf3_multi*,float,float);

inline void v_mf3_set(t_mf3_multi*,float,float,float);
inline void v_mf3_mod(t_mf3_multi*,float,float,float,float);
inline void v_mf3_mod_single(t_mf3_multi*,float,float, int);
inline void v_mf3_commit_mod(t_mf3_multi*);
inline void v_mf3_run_off(t_mf3_multi*,float,float);
inline void v_mf3_run_lp2(t_mf3_multi*,float,float);
inline void v_mf3_run_lp4(t_mf3_multi*,float,float);
inline void v_mf3_run_hp2(t_mf3_multi*,float,float);
inline void v_mf3_run_hp4(t_mf3_multi*,float,float);
inline void v_mf3_run_bp2(t_mf3_multi*,float,float);
inline void v_mf3_run_bp4(t_mf3_multi*,float,float);
inline void v_mf3_run_notch2(t_mf3_multi*,float,float);
inline void v_mf3_run_notch4(t_mf3_multi*,float,float);
inline void v_mf3_run_eq(t_mf3_multi*,float,float);
inline void v_mf3_run_dist(t_mf3_multi*,float,float);
inline void v_mf3_run_comb(t_mf3_multi*,float,float);
inline void v_mf3_run_amp_panner(t_mf3_multi*,float,float);
inline void v_mf3_run_limiter(t_mf3_multi*,float,float);
inline void v_mf3_run_saturator(t_mf3_multi*, float, float);
inline void v_mf3_run_formant_filter(t_mf3_multi*, float, float);
inline void v_mf3_run_chorus(t_mf3_multi*, float, float);
inline void v_mf3_run_glitch(t_mf3_multi*, float, float);
inline void v_mf3_run_ring_mod(t_mf3_multi*, float, float);
inline void v_mf3_run_lofi(t_mf3_multi*, float, float);

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
7  "Notch2"
8  "Notch4"
9  "EQ"
10 "Distortion"
11 "Comb Filter"
12 "Amp/Panner"
13 "Limiter"
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
                return v_mf3_run_notch2;
            case 8:
                return v_mf3_run_notch4;
            case 9:
                return v_mf3_run_eq;
            case 10:
                return v_mf3_run_dist;
            case 11:
                return v_mf3_run_comb;
            case 12:
                return v_mf3_run_amp_panner;
            case 13:
                return v_mf3_run_limiter;
            case 14:
                return v_mf3_run_saturator;
            case 15:
                return v_mf3_run_formant_filter;
            case 16:
                return v_mf3_run_chorus;
            case 17:
                return v_mf3_run_glitch;
            case 18:
                return v_mf3_run_ring_mod;
            case 19:
                return v_mf3_run_lofi;
            default:
                /*TODO: Report error*/
                return v_mf3_run_off;                
        }    
}

inline void v_mf3_set(t_mf3_multi*__restrict a_mf3, float a_control0, float a_control1, float a_control2)
{    
    a_mf3->control[0] = a_control0;
    a_mf3->control[1] = a_control1;
    a_mf3->control[2] = a_control2;
    
    a_mf3->mod_value[0] = 0.0f;
    a_mf3->mod_value[1] = 0.0f;
    a_mf3->mod_value[2] = 0.0f;
}

/* inline void v_mf3_mod(t_mf3_multi* a_mf3,
 * float a_mod, //Expects 0 to 1 or -1 to 1 range from an LFO, envelope, etc...
 * float a_amt0, float a_amt1, float a_amt2)  //Amount, from the GUI.  Range:  -100 to 100
 */
inline void v_mf3_mod(t_mf3_multi*__restrict a_mf3,float a_mod, float a_amt0, float a_amt1, float a_amt2)
{    
    a_mf3->mod_value[0] = (a_mf3->mod_value[0]) + (a_mod * a_amt0 * .01f);
    a_mf3->mod_value[1] = (a_mf3->mod_value[1]) + (a_mod * a_amt1 * .01f);
    a_mf3->mod_value[2] = (a_mf3->mod_value[2]) + (a_mod * a_amt2 * .01f);
}

/* inline void v_mf3_mod_single(
 * t_mf3_multi* a_mf3,
 * float a_mod, //The output of the LFO, etc...  -1.0f to 1.0f
 * float a_amt, //amount, -1.0f to 1.0f
 * int a_index)  //control index
 */
inline void v_mf3_mod_single(t_mf3_multi*__restrict a_mf3,float a_mod, float a_amt, int a_index)
{    
    a_mf3->mod_value[a_index] = (a_mf3->mod_value[a_index]) + (a_mod * a_amt);    //not  * .01 because it's expected you did this at note_on
}

inline void v_mf3_commit_mod(t_mf3_multi*__restrict a_mf3)
{
    a_mf3->control[0] = (a_mf3->control[0]) + ((a_mf3->mod_value[0]) * 127.0f);
    
    if((a_mf3->control[0]) > 127.0f)
    {
        a_mf3->control[0] = 127.0f;
    }
    
    if((a_mf3->control[0]) < 0.0f)
    {
        a_mf3->control[0] = 0.0f;
    }
    
    a_mf3->control[1] = (a_mf3->control[1]) + ((a_mf3->mod_value[1]) * 127.0f);
        
    if((a_mf3->control[1]) > 127.0f)
    {
        a_mf3->control[1] = 127.0f;
    }
    
    if((a_mf3->control[1]) < 0.0f)
    {
        a_mf3->control[1] = 0.0f;
    }
    
    a_mf3->control[2] = (a_mf3->control[2]) + ((a_mf3->mod_value[2]) * 127.0f);
             
    if((a_mf3->control[2]) > 127.0f)
    {
        a_mf3->control[2] = 127.0f;
    }
    
    if((a_mf3->control[2]) < 0.0f)
    {
        a_mf3->control[2] = 0.0f;
    }
}

inline void v_mf3_run_off(t_mf3_multi*__restrict a_mf3, float a_in0, float a_in1)
{
    a_mf3->output0 = a_in0;
    a_mf3->output1 = a_in1;
}

inline void v_mf3_run_lp2(t_mf3_multi*__restrict a_mf3, float a_in0, float a_in1)
{
    f_mfx_transform_svf_filter(a_mf3);
    
    a_mf3->output0 = v_svf_run_2_pole_lp(a_mf3->svf0, a_in0);
    a_mf3->output1 = v_svf_run_2_pole_lp(a_mf3->svf1, a_in1);
}

inline void v_mf3_run_lp4(t_mf3_multi*__restrict a_mf3, float a_in0, float a_in1)
{
    f_mfx_transform_svf_filter(a_mf3);
    
    a_mf3->output0 = v_svf_run_4_pole_lp(a_mf3->svf0, a_in0);
    a_mf3->output1 = v_svf_run_4_pole_lp(a_mf3->svf1, a_in1);
}

inline void v_mf3_run_hp2(t_mf3_multi*__restrict a_mf3, float a_in0, float a_in1)
{
    f_mfx_transform_svf_filter(a_mf3);
    
    a_mf3->output0 = v_svf_run_2_pole_hp(a_mf3->svf0, a_in0);
    a_mf3->output1 = v_svf_run_2_pole_hp(a_mf3->svf1, a_in1);
}

inline void v_mf3_run_hp4(t_mf3_multi*__restrict a_mf3, float a_in0, float a_in1)
{
    f_mfx_transform_svf_filter(a_mf3);
    
    a_mf3->output0 = v_svf_run_4_pole_hp(a_mf3->svf0, a_in0);
    a_mf3->output1 = v_svf_run_4_pole_hp(a_mf3->svf1, a_in1);
}

inline void v_mf3_run_bp2(t_mf3_multi*__restrict a_mf3, float a_in0, float a_in1)
{
    f_mfx_transform_svf_filter(a_mf3);
    
    a_mf3->output0 = v_svf_run_2_pole_bp(a_mf3->svf0, a_in0);
    a_mf3->output1 = v_svf_run_2_pole_bp(a_mf3->svf1, a_in1);
}

inline void v_mf3_run_bp4(t_mf3_multi*__restrict a_mf3, float a_in0, float a_in1)
{
    f_mfx_transform_svf_filter(a_mf3);
    
    a_mf3->output0 = v_svf_run_4_pole_bp(a_mf3->svf0, a_in0);
    a_mf3->output1 = v_svf_run_4_pole_bp(a_mf3->svf1, a_in1);
}

inline void v_mf3_run_notch2(t_mf3_multi*__restrict a_mf3, float a_in0, float a_in1)
{
    f_mfx_transform_svf_filter(a_mf3);
    
    a_mf3->output0 = v_svf_run_2_pole_notch(a_mf3->svf0, a_in0);
    a_mf3->output1 = v_svf_run_2_pole_notch(a_mf3->svf1, a_in1);
}

inline void v_mf3_run_notch4(t_mf3_multi*__restrict a_mf3, float a_in0, float a_in1)
{
    f_mfx_transform_svf_filter(a_mf3);
    
    a_mf3->output0 = v_svf_run_4_pole_notch(a_mf3->svf0, a_in0);
    a_mf3->output1 = v_svf_run_4_pole_notch(a_mf3->svf1, a_in1);
}

inline void v_mf3_run_eq(t_mf3_multi*__restrict a_mf3, float a_in0, float a_in1)
{   
    v_mf3_commit_mod(a_mf3);    
    //cutoff
    a_mf3->control_value[0] = (((a_mf3->control[0]) * 0.818897638) + 20.0f);
    //width
    a_mf3->control_value[1] = ((a_mf3->control[1]) * 0.047244094) + 2.5f;
    //gain
    a_mf3->control_value[2] = (a_mf3->control[2]) * 0.377952756 - 24.0f;
    
    v_pkq_calc_coeffs(a_mf3->eq0, a_mf3->control_value[0], a_mf3->control_value[1], a_mf3->control_value[2]);
    
    v_pkq_run(a_mf3->eq0, a_in0, a_in1);
    
    a_mf3->output0 = (a_mf3->eq0->output0);
    a_mf3->output1 = (a_mf3->eq0->output1);
}

inline void v_mf3_run_dist(t_mf3_multi*__restrict a_mf3, float a_in0, float a_in1)
{
    v_mf3_commit_mod(a_mf3);
    a_mf3->control_value[0] = ((a_mf3->control[0]) * 0.283464567f);
    a_mf3->control_value[1] = ((a_mf3->control[1]) * 0.007874016f);
    a_mf3->control_value[2] = (((a_mf3->control[2]) * 0.094488189f) - 12.0f);
    a_mf3->outgain = f_db_to_linear((a_mf3->control_value[2]), a_mf3->amp_ptr);
    v_clp_set_in_gain(a_mf3->clipper, (a_mf3->control_value[0]));
    v_axf_set_xfade(a_mf3->xfader, (a_mf3->control_value[1]));
    
    a_mf3->output0 = f_axf_run_xfade(a_mf3->xfader, a_in0, ((a_mf3->outgain) * f_clp_clip(a_mf3->clipper, a_in0)));
    a_mf3->output1 = f_axf_run_xfade(a_mf3->xfader, a_in1, ((a_mf3->outgain) * f_clp_clip(a_mf3->clipper, a_in1)));
}


inline void v_mf3_run_comb(t_mf3_multi*__restrict a_mf3, float a_in0, float a_in1)
{
    v_mf3_commit_mod(a_mf3);
    
    //cutoff
    a_mf3->control_value[0] = (((a_mf3->control[0]) * 0.692913386) + 20.0f);
    //res
    a_mf3->control_value[1] = ((a_mf3->control[1]) * 0.157480315) - 20.0f;
    
    v_cmb_set_all(a_mf3->comb_filter0, (a_mf3->control_value[1]), (a_mf3->control_value[1]), 
                    (a_mf3->control_value[0]));

    v_cmb_set_all(a_mf3->comb_filter1, (a_mf3->control_value[1]), (a_mf3->control_value[1]), 
            (a_mf3->control_value[0]));

    v_cmb_set_input(a_mf3->comb_filter0, a_in0);
    v_cmb_set_input(a_mf3->comb_filter1, a_in1);

    a_mf3->output0 = (a_mf3->comb_filter0->output_sample);
    a_mf3->output1 = (a_mf3->comb_filter1->output_sample);
}

inline void v_mf3_run_amp_panner(t_mf3_multi*__restrict a_mf3, float a_in0, float a_in1)
{
    v_mf3_commit_mod(a_mf3);
    
    a_mf3->control_value[0] = ((a_mf3->control[0]) * 0.007874016f);    
    a_mf3->control_value[1] = ((a_mf3->control[1]) * 0.283464567f) - 30.0f;
    
    v_app_set(a_mf3->amp_and_panner, (a_mf3->control_value[1]), (a_mf3->control_value[0]));
    v_app_run(a_mf3->amp_and_panner, a_in0, a_in1);
    
    a_mf3->output0 = (a_mf3->amp_and_panner->output0);
    a_mf3->output1 = (a_mf3->amp_and_panner->output1);
}

inline void f_mfx_transform_svf_filter(t_mf3_multi*__restrict a_mf3)
{
    v_mf3_commit_mod(a_mf3);    
    //cutoff
    a_mf3->control_value[0] = (((a_mf3->control[0]) * 0.818897638) + 20.0f);
    //res
    a_mf3->control_value[1] = ((a_mf3->control[1]) * 0.236220472) - 30.0f;
    
    v_svf_set_cutoff_base(a_mf3->svf0, (a_mf3->control_value[0]));    
    v_svf_set_res(a_mf3->svf0, (a_mf3->control_value[1]));    
    v_svf_set_cutoff(a_mf3->svf0);
    
    v_svf_set_cutoff_base(a_mf3->svf1, (a_mf3->control_value[0]));
    v_svf_set_res(a_mf3->svf1, (a_mf3->control_value[1]));    
    v_svf_set_cutoff(a_mf3->svf1);
}


inline void v_mf3_run_limiter(t_mf3_multi*__restrict a_mf3, float a_in0, float a_in1)
{
    v_mf3_commit_mod(a_mf3);
    a_mf3->control_value[0] = (((a_mf3->control[0]) * 0.236220472f) - 30.0f);
    a_mf3->control_value[1] = (((a_mf3->control[1]) * 0.093700787f) - 11.9f);
    a_mf3->control_value[2] = (((a_mf3->control[2]) * 1.968503937f) + 150.0f);
    
    v_lim_set(a_mf3->limiter, (a_mf3->control_value[0]), (a_mf3->control_value[1]), (a_mf3->control_value[2]));
    v_lim_run(a_mf3->limiter, a_in0, a_in1);
    
    a_mf3->output0 = a_mf3->limiter->output0;
    a_mf3->output1 = a_mf3->limiter->output1;
}

inline void v_mf3_run_saturator(t_mf3_multi*__restrict a_mf3, float a_in0, float a_in1)
{
    v_mf3_commit_mod(a_mf3);
    a_mf3->control_value[0] = ((a_mf3->control[0]) * 0.188976378) - 12.0f;
    a_mf3->control_value[1] = ((a_mf3->control[1]) * 0.748031496f) + 5.0f;
    a_mf3->control_value[2] = ((a_mf3->control[2]) * 0.188976378) - 12.0f;
    
    v_sat_set(a_mf3->saturator, (a_mf3->control_value[0]), (a_mf3->control_value[1]), (a_mf3->control_value[2]));
    
    v_sat_run(a_mf3->saturator, a_in0, a_in1);
    
    a_mf3->output0 = a_mf3->saturator->output0;
    a_mf3->output1 = a_mf3->saturator->output1;
}

inline void v_mf3_run_formant_filter(t_mf3_multi*__restrict a_mf3, float a_in0, float a_in1)
{
    v_mf3_commit_mod(a_mf3);
    a_mf3->control_value[0] = ((a_mf3->control[0]) * 0.07086f);
    a_mf3->control_value[1] = ((a_mf3->control[1]) * 0.007874016);
    
    v_for_formant_filter_set(a_mf3->formant_filter, a_mf3->control_value[0], a_mf3->control_value[1]);
    v_for_formant_filter_run(a_mf3->formant_filter, a_in0, a_in1);
    
    a_mf3->output0 = a_mf3->formant_filter->output0;
    a_mf3->output1 = a_mf3->formant_filter->output1;
}

inline void v_mf3_run_chorus(t_mf3_multi*__restrict a_mf3, float a_in0, float a_in1)
{
    v_mf3_commit_mod(a_mf3);
    a_mf3->control_value[0] = ((a_mf3->control[0]) * 0.04488189f) + 0.3f;
    a_mf3->control_value[1] = ((a_mf3->control[1]) * 0.1889f) - 24.0f;
    
    v_crs_chorus_set(a_mf3->chorus, a_mf3->control_value[0], a_mf3->control_value[1]);
    v_crs_chorus_run(a_mf3->chorus, a_in0, a_in1);
    
    a_mf3->output0 = a_mf3->chorus->output0;
    a_mf3->output1 = a_mf3->chorus->output1;
}

inline void v_mf3_run_glitch(t_mf3_multi*__restrict a_mf3, float a_in0, float a_in1)
{
    v_mf3_commit_mod(a_mf3);
    a_mf3->control_value[0] = ((a_mf3->control[0]) * 0.62992126f) + 5.0f;
    a_mf3->control_value[1] = ((a_mf3->control[1]) * 0.08661f) + 1.1f;
    a_mf3->control_value[2] = ((a_mf3->control[2]) * 0.007874016f);
    
    v_glc_glitch_set(a_mf3->glitch, a_mf3->control_value[0], a_mf3->control_value[1], a_mf3->control_value[2]);
    v_glc_glitch_run(a_mf3->glitch, a_in0, a_in1);
    
    a_mf3->output0 = a_mf3->glitch->output0;
    a_mf3->output1 = a_mf3->glitch->output1;
}

inline void v_mf3_run_ring_mod(t_mf3_multi*__restrict a_mf3, float a_in0, float a_in1)
{
    v_mf3_commit_mod(a_mf3);
    a_mf3->control_value[0] = ((a_mf3->control[0]) * 0.40944f) + 31.0f;
    a_mf3->control_value[1] = ((a_mf3->control[1]) * 0.18897f) -24.0f;
    
    v_rmd_ring_mod_set(a_mf3->ring_mod, a_mf3->control_value[0], a_mf3->control_value[1]);
    v_rmd_ring_mod_run(a_mf3->ring_mod, a_in0, a_in1);
    
    a_mf3->output0 = a_mf3->ring_mod->output0;
    a_mf3->output1 = a_mf3->ring_mod->output1;
}

inline void v_mf3_run_lofi(t_mf3_multi*__restrict a_mf3, float a_in0, float a_in1)
{
    v_mf3_commit_mod(a_mf3);
    a_mf3->control_value[0] = ((a_mf3->control[0]) * 0.094488) + 4.0f;
        
    v_lfi_lofi_set(a_mf3->lofi, a_mf3->control_value[0]);
    v_lfi_lofi_run(a_mf3->lofi, a_in0, a_in1);
        
    a_mf3->output0 = a_mf3->lofi->output0;
    a_mf3->output1 = a_mf3->lofi->output1;
}

/* t_mf3_multi g_mf3_get(
 * float a_sample_rate)
 */
t_mf3_multi * g_mf3_get(float a_sample_rate)
{
    t_mf3_multi * f_result;
    
    if(posix_memalign((void**)&f_result, 16, (sizeof(t_mf3_multi))) != 0)
    {
        return 0;
    }
    
    f_result->effect_index = 0; 
    f_result->channels = 2;
    f_result->svf0 = g_svf_get(a_sample_rate);
    f_result->svf1 = g_svf_get(a_sample_rate);
    f_result->comb_filter0 = g_cmb_get_comb_filter(a_sample_rate);
    f_result->comb_filter1 = g_cmb_get_comb_filter(a_sample_rate);
    f_result->eq0 = g_pkq_get(a_sample_rate);
    f_result->clipper = g_clp_get_clipper();
    v_clp_set_clip_sym(f_result->clipper, -3.0f);
    f_result->limiter = g_lim_get(a_sample_rate);
    f_result->output0 = 0.0f;
    f_result->output1 = 0.0f;
    f_result->control[0] = 0.0f;
    f_result->control[1] = 0.0f;
    f_result->control[2] = 0.0f;
    f_result->control_value[0] = 65.0f;
    f_result->control_value[1] = 65.0f;
    f_result->control_value[2] = 65.0f;
    f_result->mod_value[0] = 0.0f;
    f_result->mod_value[1] = 0.0f;
    f_result->mod_value[2] = 0.0f;
    f_result->xfader = g_axf_get_audio_xfade(-3.0f);
    f_result->outgain = 1.0f;
    f_result->amp_ptr = g_amp_get();
    f_result->amp_and_panner = g_app_get();
    f_result->saturator = g_sat_get();
    f_result->formant_filter = g_for_formant_filter_get(a_sample_rate);
    f_result->chorus = g_crs_chorus_get(a_sample_rate);
    f_result->glitch = g_glc_glitch_get(a_sample_rate);
    f_result->ring_mod = g_rmd_ring_mod_get(a_sample_rate);
    f_result->lofi = g_lfi_lofi_get();
    
    return f_result;
}

#ifdef	__cplusplus
}
#endif

#endif	/* MULTIFX3KNOB_H */

