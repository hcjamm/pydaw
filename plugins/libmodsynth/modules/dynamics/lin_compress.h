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

#include "../../lib/amp.h"
#include "../../lib/interpolate-linear.h"
#include "../../lib/lms_math.h"
    
typedef struct st_lic_compress
{
    float attack, release, threshold, ratio, ratio_linear;
    float input_db, gain_reduction_db, gain_reduction_linear;
    /*0 == attack, 1 == release*/
    int action;
    t_smoother_linear * attack_smoother, release_smoother;
    t_amp * amp_ptr;
    float output0, output1;
}t_lic_compress;

void v_lic_set(t_lic_compress*,float,float,float,float);
void v_lic_run(t_lic_compress*,float,float);
t_lic_compress * g_lic_get();


void v_lic_set(t_lic_compress* a_lic, float a_threshold, float a_ratio, float a_attack, float a_release)
{
    a_lic->threshold = a_threshold;
        
    a_lic->attack_smoother->rate = a_attack * (a_lic->attack_smoother->sr_recip);
    a_lic->release_smoother->rate = a_release * (a_lic->release_smoother->sr_recip);
    
    a_lic->release = a_release;
    
    if(a_ratio != (a_lic->ratio))
    {
        a_lic->ratio = a_ratio;
        a_lic->ratio_linear = ((1 - (1/a_ratio)) * -1);
    }
}

void v_lic_run(t_lic_compress* a_lic, float a_in0, float a_in1)
{
    a_lic->input_db = f_db_to_linear_fast(f_lms_max(f_lms_abs(a_in0), f_lms_abs(a_in1)), a_lic->amp_ptr);
    
    if((a_lic->input_db) > (a_lic->threshold))
    {
        if((a_lic->action) == 1)
        {
            a_lic->action = 0;
            a_lic->attack_smoother->last_value = (a_lic->release_smoother->last_value);
        }
        
        v_sml_run(a_lic->attack_smoother, (a_lic->input_db));
        
        a_lic->gain_reduction_db = f_lms_ceiling(((a_lic->attack_smoother->last_value) * (a_lic->ratio_linear)), 0);
    }
    else
    {
        if((a_lic->action) == 0)
        {
            a_lic->action = 1;
            a_lic->release_smoother->last_value = (a_lic->attack_smoother->last_value);
        }
        
        v_sml_run(a_lic->release_smoother, (a_lic->input_db));
        
        a_lic->gain_reduction_db = f_lms_ceiling(((a_lic->release_smoother->last_value) * (a_lic->ratio_linear)), 0);
    }
    
    a_lic->gain_reduction_linear = f_db_to_linear_fast((a_lic->gain_reduction_db), a_lic->amp_ptr);
    
    a_lic->output0 = (a_lic->gain_reduction_linear) * a_in0;
    a_lic->output1 = (a_lic->gain_reduction_linear) * a_in1;
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
    
    return f_result;
}

#ifdef	__cplusplus
}
#endif

#endif	/* LIN_COMPRESS_H */

