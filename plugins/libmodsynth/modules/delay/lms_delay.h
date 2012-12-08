/* 
 * File:   lms_delay.h
 * Author: Jeff Hubbard
 * 
 * This file contains t_lms_delay, which is the basis of LMS_Delay version 1.  It is a complete
 * mono/stereo delay effect written with delay.h
 * 
 * Created on March 5, 2012, 9:16 PM
 */

#ifndef LMS_DELAY_H
#define	LMS_DELAY_H

#ifdef	__cplusplus
extern "C" {
#endif

//#define LMS_DELAY_DEBUG_MODE
    
#include "delay.h"
#include "../../lib/amp.h"
#include "../signal_routing/dry_wet.h"
#include "../../lib/denormal.h"
#include "../signal_routing/audio_xfade.h"
#include "../modulation/env_follower.h"
/* A multi-mode delay module.  This is a complete delay with stereo, ping-pong, etc... modes
 * feeback can be routed out and back into the module.
 * 
 * The delay in delay.h is for building your own custom delays.  This is an implementation of that.
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
    
    t_amp * amp_ptr;    
}t_lms_delay;

t_lms_delay * g_ldl_get_delay(float,float);
inline void v_ldl_set_delay(t_lms_delay*,float,float,float,float,float);
inline void v_ldl_run_delay(t_lms_delay*,float,float);

/*t_lms_delay * g_ldl_get_delay(
 * float a_seconds, //The maximum amount of time for the delay to buffer, it should never be asked to delay longer than this number
 * float a_sr  //sample rate
 * )
 */
t_lms_delay * g_ldl_get_delay(float a_seconds, float a_sr)
{
    t_lms_delay* f_result = (t_lms_delay*)malloc(sizeof(t_lms_delay));
    
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
    
    return f_result;
}

/* inline void v_ldl_run_delay_ping_pong(
 * t_lms_delay* a_dly, 
 * float a_in0, //input 0  - audio input 0
 * float a_in1, //input 1
 * float a_fb0, //feedback 0 - This is for allowing external modules to modify the feedback, like a dampening filter
 * float a_fb1) //feedback 1
 */
inline void v_ldl_run_delay(t_lms_delay* a_dly, float a_in0, float a_in1)
{    
    v_enf_run_env_follower(a_dly->feeback_env_follower, ((a_dly->feedback0) + (a_dly->feedback1)));
    /*Automatically reduce feedback if delay volume is too high*/
    if((a_dly->feeback_env_follower->output_smoothed) > -3.0f)
    {
        a_dly->feedback_linear = f_db_to_linear(((-3.0f - (a_dly->feeback_env_follower->output_smoothed)) * 0.3f), a_dly->amp_ptr);
    }
    
    v_dly_run_delay(a_dly->delay0, 
            f_axf_run_xfade(a_dly->stereo_xfade0, 
            (a_in0 + ((a_dly->feedback0) * (a_dly->feedback_linear))),
            (((a_dly->feedback1) * (a_dly->feedback_linear)) + ((a_in0 + a_in1) * 0.5f))
            ));
    
    v_dly_run_tap(a_dly->delay0, a_dly->tap0);
    
    v_dly_run_delay(a_dly->delay1, 
            f_axf_run_xfade(a_dly->stereo_xfade0 ,
            (a_in1 + ((a_dly->feedback1) * (a_dly->feedback_linear))),
            (a_dly->tap0->output))); 
    
    v_dly_run_tap(a_dly->delay1, a_dly->tap1);    
    
    a_dly->feedback0 = f_remove_denormal((a_dly->tap0->output));
    a_dly->feedback1 = f_remove_denormal((a_dly->tap1->output));
    
    v_dw_run_dry_wet(a_dly->dw0, a_in0, (a_dly->feedback0));
    v_dw_run_dry_wet(a_dly->dw1, a_in1, (a_dly->feedback1));
    
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
inline void v_ldl_set_delay(t_lms_delay* a_dly,float a_seconds, float a_feeback_db, float a_wet, float a_dry, float a_stereo)
{
    v_dly_set_delay_seconds(a_dly->delay0, a_dly->tap0, a_seconds);
    v_dly_set_delay_seconds(a_dly->delay1, a_dly->tap1, a_seconds);
 
    v_axf_set_xfade(a_dly->stereo_xfade0, a_stereo);
    v_axf_set_xfade(a_dly->stereo_xfade1, a_stereo);
    
    if(a_feeback_db != (a_dly->feedback_db))
    {
        a_dly->feedback_db = a_feeback_db;
        
        a_dly->feedback_linear = f_db_to_linear_fast(a_feeback_db, a_dly->amp_ptr);
    }
    
    v_dw_set_dry_wet(a_dly->dw0, a_dry, a_wet);
    v_dw_set_dry_wet(a_dly->dw1, a_dry, a_wet);
    
}

#ifdef	__cplusplus
}
#endif

#endif	/* LMS_DELAY_H */

