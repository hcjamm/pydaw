/* 
 * File:   lin_compress.h
 * Author: Jeff Hubbard
 *
 * A stereo compressor based on my ramp envelope.  Compresses both channels equally based on
 *  the higher of the 2 channels.
 * 
 * Created on April 15, 2012, 7:36 PM
 */

#ifndef RAMP_COMPRESS_H
#define	RAMP_COMPRESS_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#define LMS_RAC_DEBUG_MODE

#include "../../lib/amp.h"
#include "../modulation/ramp_env.h"
#include "../../lib/lms_math.h"
#include "../../lib/smoother-linear.h"
    
typedef struct st_rac_compress
{
    float attack, release, threshold, ratio, ratio_linear;
    float input_db, gain_reduction_db, gain_reduction_linear;
    /*0 == attack, 1 == release*/
    int action;
    t_ramp_env * attack_env;
    t_ramp_env * release_env;
    t_smoother_linear * audio_smoother;
    float threshold_difference;
    t_amp * amp_ptr;
    float output0, output1;
#ifdef LMS_RAC_DEBUG_MODE
    int debug_counter;
#endif
}t_rac_compress;

void v_rac_set(t_rac_compress*,float,float,float,float);
void v_rac_run(t_rac_compress*,float,float);
t_rac_compress * g_rac_get(float);

/*void v_rac_set(t_rac_compress* a_rac, float a_threshold, float a_ratio, float a_attack, float a_release)*/
void v_rac_set(t_rac_compress* a_rac, float a_threshold, float a_ratio, float a_attack, float a_release)
{
    a_rac->threshold = a_threshold;
        
    //a_rac->attack_smoother->rate = a_attack * (a_rac->attack_smoother->sr_recip);
    //a_rac->release_smoother->rate = a_release * (a_rac->release_smoother->sr_recip);
    v_rmp_set_time(a_rac->attack_env, a_attack);
    v_rmp_set_time(a_rac->release_env, a_release);
        
    a_rac->release = a_release;
    
    if(a_ratio != (a_rac->ratio))
    {
        a_rac->ratio = a_ratio;
        a_rac->ratio_linear = ((-1.0f + (1/a_ratio)));
    }
}

/*void v_rac_run(t_rac_compress* a_rac, float a_in0, float a_in1)*/
void v_rac_run(t_rac_compress* a_rac, float a_in0, float a_in1)
{
    v_sml_run(a_rac->audio_smoother, f_lms_max(f_lms_abs(a_in0), f_lms_abs(a_in1)));
    a_rac->input_db = f_linear_to_db_fast((a_rac->audio_smoother->last_value), a_rac->amp_ptr);
    
    if((a_rac->input_db) > (a_rac->threshold))
    {
        if((a_rac->action) == 1)
        {
            a_rac->action = 0;
            a_rac->attack_env->output = (a_rac->release_env->output);
        }
        
        f_rmp_run_ramp(a_rac->attack_env);
        
        a_rac->threshold_difference = (a_rac->attack_env->output) * ((a_rac->input_db) - (a_rac->threshold));
        
    }
    else
    {
        if((a_rac->action) == 0)
        {
            a_rac->action = 1;
            a_rac->release_env->output = (a_rac->attack_env->output);
        }
        
        //v_sml_run(a_rac->release_smoother, (a_rac->input_db));
        f_rmp_run_ramp(a_rac->release_env);
        
        a_rac->threshold_difference =  (1.0f - (a_rac->release_env->output)) * ((a_rac->input_db) - (a_rac->threshold));
    }
    
    a_rac->gain_reduction_db = f_lms_ceiling(((a_rac->threshold_difference) * (a_rac->ratio_linear)), 0.0f);
    
    a_rac->gain_reduction_linear = f_db_to_linear_fast((a_rac->gain_reduction_db), a_rac->amp_ptr);
    
    a_rac->output0 = (a_rac->gain_reduction_linear) * a_in0;
    a_rac->output1 = (a_rac->gain_reduction_linear) * a_in1;
    
#ifdef LMS_RAC_DEBUG_MODE
    a_rac->debug_counter = (a_rac->debug_counter + 1);
    
    if(a_rac->debug_counter >= 100000)
    {
        a_rac->debug_counter = 0;
        
        printf("a_rac->gain_reduction_db = %f\n", (a_rac->gain_reduction_db));
        printf("a_rac->gain_reduction_linear = %f\n", (a_rac->gain_reduction_linear));
        printf("a_rac->threshold = %f\n", (a_rac->threshold));
        printf("a_rac->ratio = %f\n", (a_rac->ratio));
    }
#endif
}

/* t_rac_compress * g_rac_get(float a_sr)
 */
t_rac_compress * g_rac_get(float a_sr)
{
    t_rac_compress * f_result = (t_rac_compress*)malloc(sizeof(t_rac_compress));
    
    f_result->action = 1;
    f_result->amp_ptr = g_amp_get();
    f_result->attack = 0.05f;    
    f_result->attack_env = g_rmp_get_ramp_env(a_sr);
    f_result->audio_smoother = g_sml_get_smoother_linear(a_sr, 0, 1, 0.02f);
    f_result->gain_reduction_db = 0.0f;
    f_result->gain_reduction_linear = 1.0f;
    f_result->input_db = -50.0f;
    f_result->output0 = 0.0f;
    f_result->output1 = 0.0f;
    f_result->ratio = 2.0f;
    f_result->ratio_linear = -0.5f;    
    f_result->release = 0.2f;
    f_result->release_env = g_rmp_get_ramp_env(a_sr);    
    f_result->threshold = 0.0f;
    f_result->threshold_difference = 0.0f;
#ifdef LMS_RAC_DEBUG_MODE
    f_result->debug_counter = 0;
#endif
    return f_result;
}

#ifdef	__cplusplus
}
#endif

#endif	/* RAMP_COMPRESS_H */

