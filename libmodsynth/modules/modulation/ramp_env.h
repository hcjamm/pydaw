/* 
 * File:   ramp_env.h
 * Author: vm-user
 *
 * Created on January 25, 2012, 8:23 PM
 */


#ifndef RAMP_ENV_H
#define	RAMP_ENV_H

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct st_ramp_env
{
    float output;
    float output_multiplied;
    float ramp_time;
    float ramp_inc;
    float sr;
    float sr_recip;
}t_ramp_env;


inline void f_rmp_run_ramp(t_ramp_env*, float);
void v_rmp_retrigger(t_ramp_env*,float);
t_ramp_env * g_rmp_get_ramp_env(float);



inline void f_rmp_run_ramp(t_ramp_env* a_rmp_ptr, float a_multiplier)
{
    if(a_multiplier == 0 )
    {
        a_rmp_ptr->output_multiplied = 0;
        return;
    }
    
    if((a_rmp_ptr->output) == 1)
    {
        a_rmp_ptr->output_multiplied = a_multiplier;
        return;
    }
    
    a_rmp_ptr->output = (a_rmp_ptr->output) + (a_rmp_ptr->ramp_inc);
    
    if((a_rmp_ptr->output) >= 1)
    {
        a_rmp_ptr->output = 1;
        a_rmp_ptr->output_multiplied = a_multiplier;
        return;
    }        
    
    a_rmp_ptr->output_multiplied = (a_rmp_ptr->output) * a_multiplier;
}

//TODO:  an alternate function that takes velocity into account
void v_rmp_retrigger(t_ramp_env* a_rmp_ptr, float a_time)
{
    a_rmp_ptr->output = 0;
    a_rmp_ptr->ramp_time = a_time;
    
    if((a_rmp_ptr->ramp_time = a_time) <= .05)
        a_rmp_ptr->ramp_time = a_time = .05;
    
    a_rmp_ptr->ramp_inc = (a_rmp_ptr->sr_recip) / (a_rmp_ptr->ramp_time = a_time);
}

t_ramp_env * g_rmp_get_ramp_env(float a_sr)
{
    t_ramp_env * f_result = (t_ramp_env*)malloc(sizeof(t_ramp_env));
    f_result->sr = a_sr;
    f_result->sr_recip = 1/a_sr;
    f_result->output_multiplied = 0;
    v_rmp_retrigger(f_result, .2);
    
    return f_result;
}

#ifdef	__cplusplus
}
#endif

#endif	/* RAMP_ENV_H */

