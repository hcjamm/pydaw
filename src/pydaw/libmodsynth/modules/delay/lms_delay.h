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
#ifndef LMS_DELAY_H
#define	LMS_DELAY_H

#ifdef	__cplusplus
extern "C" {
#endif

//#define LMS_DELAY_DEBUG_MODE

#include "delay.h"
#include "../../lib/amp.h"
#include "../../lib/lmalloc.h"
#include "../signal_routing/dry_wet.h"
#include "../../lib/denormal.h"
#include "../signal_routing/audio_xfade.h"
#include "../modulation/env_follower.h"
#include "../dynamics/limiter.h"

/* A multi-mode delay module.  This is a complete delay with stereo,
 * ping-pong, etc... modes
 * feeback can be routed out and back into the module.
 */
typedef struct st_lms_delay
{
    t_delay_simple * delay0;
    t_delay_simple * delay1;
    t_delay_tap * tap0;
    t_delay_tap * tap1;
    float output0;  //mixed signal out
    float output1;  //mixed signal out
    float feedback0;  //feedback out/in
    float feedback1;  //feedback out/in
    float feedback_db;
    float feedback_linear;
    t_dw_dry_wet * dw0;
    t_dw_dry_wet * dw1;
    t_audio_xfade * stereo_xfade0;
    t_audio_xfade * stereo_xfade1;

    t_enf_env_follower * feeback_env_follower;  //Checks for overflow
    t_enf_env_follower * input_env_follower;  //Checks for overflow
    float wet_dry_diff;  //difference between wet and dry output volume
    float combined_inputs;  //Add output 0 and 1
    t_lim_limiter * limiter;
    float last_duck;
    float limiter_gain;

    t_state_variable_filter * svf0;
    t_state_variable_filter * svf1;

    t_amp * amp_ptr;
}t_lms_delay;

t_lms_delay * g_ldl_get_delay(float,float);
inline void v_ldl_set_delay(t_lms_delay*,float,float,float,float,float,
        float,float);
inline void v_ldl_run_delay(t_lms_delay*,float,float);

/*t_lms_delay * g_ldl_get_delay(
 * float a_seconds, //The maximum amount of time for the delay to buffer
 * float a_sr  //sample rate
 * )
 */
t_lms_delay * g_ldl_get_delay(float a_seconds, float a_sr)
{
    t_lms_delay* f_result;
    lmalloc((void**)&f_result, sizeof(t_lms_delay));

    f_result->delay0 = g_dly_get_delay(a_seconds, a_sr);
    f_result->delay1 = g_dly_get_delay(a_seconds, a_sr);
    f_result->tap0 = g_dly_get_tap();
    f_result->tap1 = g_dly_get_tap();
    f_result->output0 = 0.0f;
    f_result->output1 = 0.0f;
    f_result->feedback0 = 0.0f;
    f_result->feedback1 = 0.0f;
    f_result->feedback_db = -50.0f;
    f_result->feedback_linear = 0.0f;
    f_result->dw0 = g_dw_get_dry_wet();
    f_result->dw1 = g_dw_get_dry_wet();
    f_result->stereo_xfade0 = g_axf_get_audio_xfade(-3.0f);
    f_result->stereo_xfade1 = g_axf_get_audio_xfade(-3.0f);

    f_result->feeback_env_follower = g_enf_get_env_follower(a_sr);
    f_result->input_env_follower = g_enf_get_env_follower(a_sr);
    f_result->combined_inputs = 0.0f;

    f_result->amp_ptr = g_amp_get();
    f_result->limiter = g_lim_get(a_sr);
    f_result->last_duck = -99.999f;

    f_result->limiter_gain = 0.0f;

    f_result->svf0 = g_svf_get(a_sr);
    f_result->svf1 = g_svf_get(a_sr);
    v_svf_set_res(f_result->svf0, -18.0f);
    v_svf_set_res(f_result->svf1, -18.0f);

    return f_result;
}

inline void v_ldl_run_delay(t_lms_delay* a_dly, float a_in0, float a_in1)
{
    v_lim_run(a_dly->limiter, a_in0, a_in1);

    v_dly_run_delay(a_dly->delay0,
            f_axf_run_xfade(a_dly->stereo_xfade0,
            (a_in0 + ((a_dly->feedback0) * (a_dly->feedback_linear))),
            (((a_dly->feedback1) * (a_dly->feedback_linear)) +
            ((a_in0 + a_in1) * 0.5f))
            ));

    v_dly_run_tap(a_dly->delay0, a_dly->tap0);

    v_dly_run_delay(a_dly->delay1,
            f_axf_run_xfade(a_dly->stereo_xfade0 ,
            (a_in1 + ((a_dly->feedback1) * (a_dly->feedback_linear))),
            (a_dly->tap0->output)));

    v_dly_run_tap(a_dly->delay1, a_dly->tap1);

    a_dly->feedback0 = v_svf_run_2_pole_lp(a_dly->svf0, (a_dly->tap0->output));
    a_dly->feedback1 = v_svf_run_2_pole_lp(a_dly->svf1, (a_dly->tap1->output));

    a_dly->limiter_gain =  (a_dly->limiter->gain)  * (a_dly->limiter->autogain);

    if(a_dly->limiter_gain > 1.0f)
    {
        a_dly->limiter_gain = 1.0f;
    }

    v_dw_run_dry_wet(a_dly->dw0, a_in0, (a_dly->feedback0) *
            (a_dly->limiter_gain));
    v_dw_run_dry_wet(a_dly->dw1, a_in1, (a_dly->feedback1) *
            (a_dly->limiter_gain));

    a_dly->output0 = (a_dly->dw0->output);
    a_dly->output1 = (a_dly->dw1->output);
}


/*inline void v_ldl_set_delay(
 * t_lms_delay* a_dly,
 * float a_seconds,
 * float a_feeback_db, //This should not exceed -2 or it could explode
 * int a_is_ducking,
 * float a_wet,
 * float a_dry,
 * float a_stereo)  //Crossfading between dual-mono and stereo.  0 to 1
 */
inline void v_ldl_set_delay(t_lms_delay* a_dly,float a_seconds,
        float a_feeback_db, float a_wet, float a_dry,
        float a_stereo, float a_duck, float a_damp)
{
    v_dly_set_delay_seconds(a_dly->delay0, a_dly->tap0, a_seconds);
    v_dly_set_delay_seconds(a_dly->delay1, a_dly->tap1, a_seconds);

    v_axf_set_xfade(a_dly->stereo_xfade0, a_stereo);
    v_axf_set_xfade(a_dly->stereo_xfade1, a_stereo);

    if(a_feeback_db != (a_dly->feedback_db))
    {
        a_dly->feedback_db = a_feeback_db;
        a_dly->feedback_linear =
                f_db_to_linear_fast(a_feeback_db, a_dly->amp_ptr);
        if(a_dly->feedback_linear > 0.9f)
        {
            a_dly->feedback_linear = 0.9f;
        }
    }

    if(a_dly->last_duck != a_duck)
    {
        a_dly->last_duck = a_duck;
        v_lim_set(a_dly->limiter, a_duck, 0.0f, 400.0f);
    }

    v_svf_set_cutoff_base(a_dly->svf0, a_damp);
    v_svf_set_cutoff_base(a_dly->svf1, a_damp);
    v_svf_set_cutoff(a_dly->svf0);
    v_svf_set_cutoff(a_dly->svf1);

    v_dw_set_dry_wet(a_dly->dw0, a_dry, a_wet);
    v_dw_set_dry_wet(a_dly->dw1, a_dry, a_wet);

}

#ifdef	__cplusplus
}
#endif

#endif	/* LMS_DELAY_H */

