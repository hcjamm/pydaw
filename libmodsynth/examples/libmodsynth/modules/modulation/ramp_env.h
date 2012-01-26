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
    float ramp_time;
    float ramp_inc;
    float sr;
    float sr_recip;
}t_ramp_env;


inline void v_rmp_set_ramp_time(t_ramp_env*,float);
inline float f_rmp_run_ramp(t_ramp_env*);
void v_rmp_retrigger(t_ramp_env*);
t_ramp_env g_rmp_get_ramp_env(float);


inline void v_rmp_set_ramp_time(t_ramp_env* a_rmp_ptr,float a_time)
{
    a_rmp_ptr->ramp_time = a_time;
    a_rmp_ptr->ramp_inc = (a_rmp_ptr->sr_recip) * a_time;
}

inline float f_rmp_run_ramp(t_ramp_env* a_rmp_ptr)
{
    if((a_rmp_ptr->output) == 1)
        return 1;
    
    a_rmp_ptr->output = (a_rmp_ptr->output) * (a_rmp_ptr->ramp_inc);
    
    if((a_rmp_ptr->output) >= 1)
    {
        a_rmp_ptr->output = 1;
        return 1;
    }        
    
    return (a_rmp_ptr->output);
}

void v_rmp_retrigger(t_ramp_env* a_rmp_ptr)
{
    a_rmp_ptr->output = 0;
}

t_ramp_env g_rmp_get_ramp_env(float a_sr)
{
    t_ramp_env * f_result = (t_ramp_env*)malloc(sizeof(t_ramp_env));
    f_result->sr = a_sr;
    f_result->sr_recip = 1/a_sr;
    
    v_rmp_set_ramp_time(f_result, .1);
    
    return f_result;
}

#ifdef	__cplusplus
}
#endif

#endif	/* RAMP_ENV_H */


