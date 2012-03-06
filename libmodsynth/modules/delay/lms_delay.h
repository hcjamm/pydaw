/* 
 * File:   lms_delay.h
 * Author: Jeff Hubbard
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
    int is_ducking;
    t_dw_dry_wet * dw0;
    t_dw_dry_wet * dw1;
}t_lms_delay;

//Used to switch between delay types, uses much less CPU than a switch statement
typedef float (*fp_ldl_run_ptr)(t_lms_delay*,float,float);

t_lms_delay * g_ldl_get_delay(float,float);

inline void v_ldl_set_delay(t_lms_delay*,float,float,int,float,float);

inline void v_ldl_run_delay_ping_pong(t_lms_delay*,float,float);
inline void v_ldl_run_delay_stereo(t_lms_delay*,float,float,float,float);

/*t_lms_delay * g_ldl_get_delay(
 * float a_tempo, //tempo in BPM
 * float a_sr  //sample rate
 * )
 */
t_lms_delay * g_ldl_get_delay(float a_tempo, float a_sr)
{
    t_lms_delay* f_result = (t_lms_delay*)malloc(sizeof(t_lms_delay));
    
    f_result->delay0 = g_dly_get_delay_tempo(a_tempo, 4, a_sr);
    f_result->delay1 = g_dly_get_delay_tempo(a_tempo, 4, a_sr);
    f_result->tap0 = g_dly_get_tap();
    f_result->tap1 = g_dly_get_tap();
    f_result->output0 = 0;
    f_result->output1 = 0;
    f_result->feedback0 = 0;
    f_result->feedback1 = 0;
    f_result->feedback_db = -50;
    f_result->feedback_linear = 0;
    f_result->is_ducking = 0;
    f_result->dw0 = g_dw_get_dry_wet();
    f_result->dw1 = g_dw_get_dry_wet();
    return f_result;
}

/* inline void v_ldl_run_delay_ping_pong(
 * t_lms_delay* a_dly, 
 * float a_in0, //input 0  - audio input 0
 * float a_in1, //input 1
 * float a_fb0, //feedback 0 - This is for allowing external modules to modify the feedback, like a dampening filter
 * float a_fb1) //feedback 1
 */
inline void v_ldl_run_delay_ping_pong(t_lms_delay* a_dly, float a_in0, float a_in1)
{
    v_dly_run_delay(a_dly->delay0, ((((a_dly->feedback1) * (a_dly->feedback_linear)) + ((a_in0 + a_in1) * 0.5f))));        
    v_dly_run_tap(a_dly->delay0, a_dly->tap0);
    
    v_dly_run_delay(a_dly->delay1, (a_dly->tap0->output));        
    v_dly_run_tap(a_dly->delay1, a_dly->tap1);    
    
    a_dly->feedback0 = (a_dly->tap0->output);
    a_dly->feedback1 = (a_dly->tap1->output);
    
    v_dw_run_dry_wet(a_dly->dw0, a_in0, (a_dly->feedback0));
    v_dw_run_dry_wet(a_dly->dw1, a_in1, (a_dly->feedback1));
    
    a_dly->output0 = (a_dly->dw0->output);
    a_dly->output1 = (a_dly->dw1->output);
}


inline void v_ldl_set_delay(t_lms_delay* a_dly,float a_beats, float a_feeback_db, int a_is_ducking,float a_wet, float a_dry)
{
    v_dly_set_delay_tempo(a_dly->delay0, a_dly->tap0, a_beats);
    v_dly_set_delay_tempo(a_dly->delay1, a_dly->tap1, a_beats);
    
    a_dly->is_ducking = a_is_ducking;
    
    if(a_feeback_db != (a_dly->feedback_db))
    {
        a_dly->feedback_db = a_feeback_db;
        a_dly->feedback_linear = f_db_to_linear_fast(a_feeback_db);
    }
    
    v_dw_set_dry_wet(a_dly->dw0, a_dry, a_wet);
    v_dw_set_dry_wet(a_dly->dw1, a_dry, a_wet);
    
}

#ifdef	__cplusplus
}
#endif

#endif	/* LMS_DELAY_H */

