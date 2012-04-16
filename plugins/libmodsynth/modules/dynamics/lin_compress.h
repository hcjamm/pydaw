/* 
 * File:   lin_compress.h
 * Author: Jeff Hubbard
 *
 * A stereo compressor based on my linear smoother algorithm.  Compresses both channels equally based on
 *  the higher of the 2 channels.
 * 
 * Created on April 15, 2012, 12:28 PM
 */

#ifndef LIN_COMPRESS_H
#define	LIN_COMPRESS_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#define LMS_LIC_DEBUG_MODE

#include "../../lib/amp.h"
#include "../../lib/interpolate-linear.h"
#include "../../lib/lms_math.h"
#include "../../lib/smoother-linear.h"
    
typedef struct st_lic_compress
{
    float attack, release, threshold, ratio, ratio_linear;
    float input_db, gain_reduction_db, gain_reduction_linear;
    /*0 == attack, 1 == release*/
    int action;
    t_smoother_linear * attack_smoother;
    t_smoother_linear * release_smoother;
    t_smoother_linear * audio_smoother;
    float threshold_difference;
    t_amp * amp_ptr;
    float output0, output1;
#ifdef LMS_LIC_DEBUG_MODE
    int debug_counter;
#endif
}t_lic_compress;

void v_lic_set(t_lic_compress*,float,float,float,float);
void v_lic_run(t_lic_compress*,float,float);
t_lic_compress * g_lic_get(float);

/*void v_lic_set(t_lic_compress* a_lic, float a_threshold, float a_ratio, float a_attack, float a_release)*/
void v_lic_set(t_lic_compress* a_lic, float a_threshold, float a_ratio, float a_attack, float a_release)
{
    a_lic->threshold = a_threshold;
        
    a_lic->attack_smoother->rate = a_attack * (a_lic->attack_smoother->sr_recip);
    a_lic->release_smoother->rate = a_release * (a_lic->release_smoother->sr_recip);
    
    a_lic->release = a_release;
    
    if(a_ratio != (a_lic->ratio))
    {
        a_lic->ratio = a_ratio;
        a_lic->ratio_linear = ((-1.0f + (1/a_ratio)));
    }
}

/*void v_lic_run(t_lic_compress* a_lic, float a_in0, float a_in1)*/
void v_lic_run(t_lic_compress* a_lic, float a_in0, float a_in1)
{
    v_sml_run(a_lic->audio_smoother, f_lms_max(f_lms_abs(a_in0), f_lms_abs(a_in1)));
    a_lic->input_db = f_linear_to_db_fast((a_lic->audio_smoother->last_value), a_lic->amp_ptr);
    
    if((a_lic->input_db) > (a_lic->threshold))
    {
        if((a_lic->action) == 1)
        {
            a_lic->action = 0;
            a_lic->attack_smoother->last_value = (a_lic->release_smoother->last_value);
        }
        
        v_sml_run(a_lic->attack_smoother, (a_lic->input_db));
        a_lic->threshold_difference = (a_lic->attack_smoother->last_value) - (a_lic->threshold);
        
    }
    else
    {
        if((a_lic->action) == 0)
        {
            a_lic->action = 1;
            a_lic->release_smoother->last_value = (a_lic->attack_smoother->last_value);
        }
        
        v_sml_run(a_lic->release_smoother, (a_lic->input_db));
        
        a_lic->threshold_difference = (a_lic->release_smoother->last_value) - (a_lic->threshold);        
    }
    
    a_lic->gain_reduction_db = f_lms_ceiling(((a_lic->threshold_difference) * (a_lic->ratio_linear)), 0.0f);
    
    a_lic->gain_reduction_linear = f_db_to_linear_fast((a_lic->gain_reduction_db), a_lic->amp_ptr);
    
    a_lic->output0 = (a_lic->gain_reduction_linear) * a_in0;
    a_lic->output1 = (a_lic->gain_reduction_linear) * a_in1;
    
#ifdef LMS_LIC_DEBUG_MODE
    a_lic->debug_counter = (a_lic->debug_counter + 1);
    
    if(a_lic->debug_counter >= 100000)
    {
        a_lic->debug_counter = 0;
        
        printf("a_lic->gain_reduction_db = %f\n", (a_lic->gain_reduction_db));
        printf("a_lic->gain_reduction_linear = %f\n", (a_lic->gain_reduction_linear));
        printf("a_lic->threshold = %f\n", (a_lic->threshold));
        printf("a_lic->ratio = %f\n", (a_lic->ratio));
    }
#endif
}

/* t_lic_compress * g_lic_get(float a_sr)
 */
t_lic_compress * g_lic_get(float a_sr)
{
    t_lic_compress * f_result = (t_lic_compress*)malloc(sizeof(t_lic_compress));
    
    f_result->action = 1;
    f_result->amp_ptr = g_amp_get();
    f_result->attack = 0.05f;    
    f_result->attack_smoother = g_sml_get_smoother_linear(a_sr, 36,-50,0.05f);
    f_result->audio_smoother = g_sml_get_smoother_linear(a_sr, 0, 1, 0.02f);
    f_result->gain_reduction_db = 0.0f;
    f_result->gain_reduction_linear = 1.0f;
    f_result->input_db = -50.0f;
    f_result->output0 = 0.0f;
    f_result->output1 = 0.0f;
    f_result->ratio = 2.0f;
    f_result->ratio_linear = -0.5f;    
    f_result->release = 0.2f;
    f_result->release_smoother = g_sml_get_smoother_linear(a_sr, 36,-50,0.2f);
    f_result->threshold = 0.0f;
    f_result->threshold_difference = 0.0f;
#ifdef LMS_LIC_DEBUG_MODE
    f_result->debug_counter = 0;
#endif
    return f_result;
}

#ifdef	__cplusplus
}
#endif

#endif	/* LIN_COMPRESS_H */

