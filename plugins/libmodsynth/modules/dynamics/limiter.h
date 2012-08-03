/* 
 * File:   limiter.h
 * Author: jeffh
 *
 * Created on August 1, 2012, 6:18 PM
 */

#ifndef LIMITER_H
#define	LIMITER_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include "../../lib/lms_math.h"
#include "../../lib/amp.h"
#include <math.h>
    
#define LMS_HOLD_TIME_DIVISOR 500.0f

typedef struct st_lim_limiter
{
    int holdtime, r1Timer, r2Timer;
    float output0, output1;
    float sr, sr_recip;
    float thresh, ceiling, volume, release, r;
    float maxSpls, max1Block, max2Block, envT, env, gain;
    float last_thresh, last_ceiling, last_release;
    float autogain;
    float *buffer0, *buffer1;
    int buffer_size, buffer_index, buffer_read_index;
    t_amp * amp_ptr;
}t_lim_limiter;

t_lim_limiter * g_lim_get(float);
void v_lim_set(t_lim_limiter*,float, float, float);
void v_lim_run(t_lim_limiter*,float, float);

void v_lim_set(t_lim_limiter*a_lim, float a_thresh, float a_ceiling, float a_release)
{
    if(a_thresh != (a_lim->last_thresh))
    {
        a_lim->thresh = f_db_to_linear_fast((a_thresh), a_lim->amp_ptr); 
        a_lim->autogain = (1.0f/(a_lim->thresh));
        a_lim->last_thresh = a_thresh;
    }
    
    if(a_ceiling != (a_lim->last_ceiling))
    {
        a_lim->volume = f_db_to_linear_fast((a_ceiling), a_lim->amp_ptr);
        a_lim->last_ceiling = a_ceiling;
    }
    
    if(a_release != (a_lim->last_release))
    {
        if(a_release <= 0)
        {
            a_lim->release = 0.005f;
        }
        else
        {
            a_lim->release = a_release * 0.001f;
        }
        
        
        a_lim->r = ((a_lim->sr_recip) / (a_lim->release)) * LMS_HOLD_TIME_DIVISOR;        
        a_lim->last_release = a_release;
    }
}

void v_lim_run(t_lim_limiter *a_lim, float a_in0, float a_in1)
{
    a_lim->maxSpls=f_lms_max(f_lms_abs(a_in0),f_lms_abs(a_in1));

    a_lim->r1Timer = (a_lim->r1Timer) + 1;
    
    if((a_lim->r1Timer) > (a_lim->holdtime))
    {
        a_lim->r1Timer = 0; 
        a_lim->max1Block = (a_lim->max1Block) - (a_lim->r);
                
        if((a_lim->max1Block) < 0.0f)
        {
            a_lim->max1Block = 0.0f;
        }
    }
     
    a_lim->max1Block = f_lms_max((a_lim->max1Block),(a_lim->maxSpls));
    
    a_lim->r2Timer = (a_lim->r2Timer) + 1;
    
    if((a_lim->r2Timer) > (a_lim->holdtime))
    {
        a_lim->r2Timer = 0; 
        a_lim->max2Block = (a_lim->max2Block) - (a_lim->r);
        
        if((a_lim->max2Block) < 0.0f)
        {
            a_lim->max2Block = 0.0f;
        }
    }
    
    a_lim->max2Block = f_lms_max((a_lim->max2Block),(a_lim->maxSpls));
    
    a_lim->env = f_lms_max((a_lim->max1Block),(a_lim->max2Block));

    if(a_lim->env > a_lim->thresh)
    {
        a_lim->gain = ((a_lim->thresh) / (a_lim->env))*(a_lim->volume);
    }
    else
    {
        a_lim->gain= (a_lim->volume);
    }
    
    a_lim->buffer0[(a_lim->buffer_index)] = a_in0;
    a_lim->buffer1[(a_lim->buffer_index)] = a_in1;
    
    a_lim->buffer_index = (a_lim->buffer_index) + 1;
    
    if((a_lim->buffer_index) >= (a_lim->buffer_size))
    {
        a_lim->buffer_index = 0;
    }

    a_lim->output0 = (a_lim->buffer0[(a_lim->buffer_index)]) * (a_lim->gain) * (a_lim->autogain);
    a_lim->output1 = (a_lim->buffer0[(a_lim->buffer_index)]) * (a_lim->gain) * (a_lim->autogain);
}

t_lim_limiter * g_lim_get(float srate)
{
    t_lim_limiter * f_result;
    posix_memalign((void**)&f_result, 16, sizeof(t_lim_limiter));
    
    f_result->holdtime = ((int)(srate/LMS_HOLD_TIME_DIVISOR));
    
    f_result->buffer_size = (f_result->holdtime); // (int)(srate*0.003f);
    f_result->buffer_index = 0;
    
    posix_memalign((void**)&f_result->buffer0, 16, (sizeof(float) * (f_result->buffer_size)));
    posix_memalign((void**)&f_result->buffer1, 16, (sizeof(float) * (f_result->buffer_size)));
    
    f_result->amp_ptr = g_amp_get();
    
    int f_i;
    
    for(f_i = 0; f_i < (f_result->buffer_size); f_i++)
    {
        f_result->buffer0[f_i] = 0.0f;
        f_result->buffer1[f_i] = 0.0f;
    }
    
    f_result->r1Timer = 0;
    f_result->r2Timer = (f_result->holdtime)/2;
    
    f_result->ceiling = 0.0f;
    f_result->env = 0.0f;
    f_result->envT = 0.0f;
    f_result->gain = 0.0f;
    f_result->max1Block = 0.0f;
    f_result->max2Block = 0.0f;
    f_result->maxSpls = 0.0f;
    f_result->output0 = 0.0f;
    f_result->output1 = 0.0f;
    f_result->r = 0.0f;
    f_result->release = 0.0f;
    f_result->thresh = 0.0f;
    f_result->volume = 0.0f;
    f_result->sr = srate;
    f_result->sr_recip = 1.0f/srate;
    
    //nonsensical values that it won't evaluate to on the first run
    f_result->last_ceiling = 1234.4522f;
    f_result->last_release = 1234.4522f;
    f_result->last_thresh = 1234.4532f;
    
    return f_result;
}

#ifdef	__cplusplus
}
#endif

#endif	/* LIMITER_H */