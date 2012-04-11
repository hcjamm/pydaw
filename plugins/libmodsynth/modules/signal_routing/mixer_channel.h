/* 
 * File:   mixer_channel.h
 * Author: Jeff Hubbard
 *
 * This file provides a mixer channel for a multichannel mixer.
 * 
 * Pan is calculated by a sine ranging from .5 to 1.5 radians, with pan law added.
 * 
 * Created on April 11, 2012, 6:01 PM
 */

#ifndef MIXER_CHANNEL_H
#define	MIXER_CHANNEL_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "../../lib/smoother-linear.h"
#include "../../lib/amp.h"
#include "../../lib/fast_sine.h"
#include "../../lib/interpolate-linear.h"
    
typedef struct st_mxc_mixer_channel
{
    t_amp * amp_ptr;    
    float amp_linear;
    t_smoother_linear * amp_smoother;
    float gain_db;
    float gain_linear;
    float master_gain0;
    float master_gain1;    
    t_smoother_linear * pan_smoother;
    float pan0;
    float pan1;
    t_lin_interpolater * sine_interpolator;
    float pan_law_gain_linear;
    fp_mix_function mix_function;
    float in0, in1;
    float out0, out1;
}t_mxc_mixer_channel;

void (*fp_mix_function)(t_mxc_mixer_channel*,float,float,float,float,float);
t_mxc_mixer_channel * g_mxc_get();
void v_mxc_mix_stereo_to_stereo(t_mxc_mixer_channel*,float,float,float,float,float,float);
void v_mxc_mix_stereo_to_mono(t_mxc_mixer_channel*,float,float,float,float,float,float);
static void v_mxc_run_smoothers(t_mxc_mixer_channel*, float, float);

static void v_mxc_run_smoothers(t_mxc_mixer_channel* a_mxc, float a_amp, float a_pan, float a_pan_law)
{
    v_sml_run(a_mxc->amp_smoother, a_amp);
    v_sml_run(a_mxc->pan_smoother, a_pan);
        
    if(a_pan > 0.5f)
    {
        a_mxc->pan_law_gain_linear = f_db_to_linear_fast(( a_pan_law * a_pan * 2), a_mxc->amp_ptr);
    }
    else
    {
        a_mxc->pan_law_gain_linear = f_db_to_linear_fast((a_pan_law * (1 - a_pan) * 2), a_mxc->amp_ptr);        
    }
    
    a_mxc->pan0 = f_sine_fast_run((((1 - a_pan) * 0.5) + 0.25f), a_mxc->sine_interpolator) * (a_mxc->pan_law_gain_linear);
    a_mxc->pan1 = f_sine_fast_run(((a_pan * 0.5) + 0.25f), a_mxc->sine_interpolator) * (a_mxc->pan_law_gain_linear);
    
    a_mxc->amp_linear = f_db_to_linear_fast((a_mxc->amp_smoother->last_value), a_mxc->amp_ptr);
    
    a_mxc->master_gain0 = (a_mxc->amp_linear) * (a_mxc->pan0);
    a_mxc->master_gain1 = (a_mxc->amp_linear) * (a_mxc->pan1);        
}

void v_mxc_mix_stereo_to_stereo(t_mxc_mixer_channel* a_mxc, float a_in0, float a_in1, float a_amp, float a_gain, float a_pan, float a_pan_law)
{
    v_mxc_run_smoothers(a_mxc, a_amp, a_pan);
    
    a_mxc->out0 = (a_mxc->master_gain0) * a_in0;
    a_mxc->out1 = (a_mxc->master_gain1) * a_in1;
}

void v_mxc_mix_stereo_to_mono(t_mxc_mixer_channel* a_mxc, float a_in0, float a_in1, float a_amp, float a_gain, float a_pan, float a_pan_law)
{
    v_mxc_run_smoothers(a_mxc, a_amp, a_pan);
    
    a_mxc->in0 = (a_in0 + a_in1) * 0.5f;
    a_mxc->in1 = (a_mxc->in0);
    
    a_mxc->out0 = (a_mxc->master_gain0) * a_mxc->in0;
    a_mxc->out1 = (a_mxc->master_gain1) * a_mxc->in1;
}

t_mxc_mixer_channel * g_mxc_get(float a_sr)
{
    t_mxc_mixer_channel f_result = (t_mxc_mixer_channel*)malloc(sizeof(t_mxc_mixer_channel));
    
    f_result->amp_linear = 1;
    f_result->amp_ptr = g_amp_get();
    f_result->in0 = 0;
    f_result->in1 = 0;
    f_result->gain_db = 0;
    f_result->gain_linear = 1;
    f_result->mix_function = v_mxc_mix_stereo_to_stereo;
    f_result->out0 = 0;
    f_result->out1 = 0;
    f_result->master_gain0 = 1;
    f_result->master_gain1 = 1;
    f_result->sine_interpolator = g_lin_get();
    f_result->amp_smoother = g_sml_get_smoother_linear(a_sr, 0, -48, 0.5f);
    f_result->pan_smoother = g_sml_get_smoother_linear(a_sr, 1.0f, -1.0f, 0.5f);
    f_result->pan_law_gain_linear = 3.0f;
    f_result->pan0 = 1;
    f_result->pan1 = 1;
    
    return f_result;
}

#ifdef	__cplusplus
}
#endif

#endif	/* MIXER_CHANNEL_H */

