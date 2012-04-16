/* 
 * File:   ramp_env.h
 * Author: Jeff Hubbard
 * 
 * Purpose:  This file provides t_ramp_env, a ramp envelope.
 * 
 * The envelope goes from 0 at retrigger to 1 when finished.
 * 
 * The 0 to 1 value can be read from t_ramp_env->output, or you can read a multiplied value from t_ramp_env->output_multiplied
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
    float output;  //if == 1, the ramp can be considered finished running
    float output_multiplied;
    float ramp_time;
    float ramp_inc;
    float sr;
    float sr_recip;
    float output_multiplier;
}t_ramp_env;


inline void f_rmp_run_ramp(t_ramp_env*);
void v_rmp_retrigger(t_ramp_env*,float,float);
void v_rmp_retrigger_glide_t(t_ramp_env*,float,float,float);
void v_rmp_retrigger_glide_r(t_ramp_env*,float,float,float);
void v_rmp_set_time(t_ramp_env*,float);
t_ramp_env * g_rmp_get_ramp_env(float);



inline void f_rmp_run_ramp(t_ramp_env* a_rmp_ptr)
{
    if((a_rmp_ptr->output_multiplier) == 0 )
    {
        a_rmp_ptr->output_multiplied = 0;
        return;
    }
    
    if((a_rmp_ptr->output) == 1)
    {
        a_rmp_ptr->output_multiplied = (a_rmp_ptr->output_multiplier);
        return;
    }
    
    a_rmp_ptr->output = (a_rmp_ptr->output) + (a_rmp_ptr->ramp_inc);
    
    if((a_rmp_ptr->output) >= 1)
    {
        a_rmp_ptr->output = 1;
        a_rmp_ptr->output_multiplied = (a_rmp_ptr->output_multiplier);
        return;
    }        
    
    a_rmp_ptr->output_multiplied = (a_rmp_ptr->output) * (a_rmp_ptr->output_multiplier);
}

/* void v_rmp_set_time(
 * t_ramp_env* a_rmp_ptr,
 * float a_time)  //time in seconds
 * 
 * Set envelope time without retriggering the envelope
 */
void v_rmp_set_time(t_ramp_env* a_rmp_ptr,float a_time)
{
    a_rmp_ptr->ramp_time = a_time;
       
    if((a_rmp_ptr->ramp_time) <= .01)
    {
        a_rmp_ptr->output = 1;
        a_rmp_ptr->output_multiplied = (a_rmp_ptr->output_multiplier);
        return;
    }
    else
    {
        a_rmp_ptr->output = 0;
        a_rmp_ptr->ramp_inc = (a_rmp_ptr->sr_recip) / (a_rmp_ptr->ramp_time);
    }
}

/*void v_rmp_retrigger(
 * t_ramp_env* a_rmp_ptr, 
 * float a_time, 
 * float a_multiplier)
 */
void v_rmp_retrigger(t_ramp_env* a_rmp_ptr, float a_time, float a_multiplier)
{
    a_rmp_ptr->output = 0;
    a_rmp_ptr->ramp_time = a_time;
    a_rmp_ptr->output_multiplier = a_multiplier;
        
    if((a_rmp_ptr->ramp_time) <= .05)
    {
        a_rmp_ptr->output = 1;
        a_rmp_ptr->output_multiplied = (a_rmp_ptr->output_multiplier);
        return;
    }
    else
    {
        a_rmp_ptr->output = 0;
        a_rmp_ptr->ramp_inc = (a_rmp_ptr->sr_recip) / (a_rmp_ptr->ramp_time);
    }
    
}

/*Glide with constant time in seconds*/
void v_rmp_retrigger_glide_t(t_ramp_env* a_rmp_ptr, float a_time, float a_current_note, float a_next_note)
{    
    a_rmp_ptr->ramp_time = a_time;
    
    a_rmp_ptr->output_multiplier = a_next_note - a_current_note;
        
    /*Turn off if true*/
    if((a_rmp_ptr->ramp_time) <= .05)
    {
        a_rmp_ptr->output = 1;
        a_rmp_ptr->output_multiplied = (a_rmp_ptr->output_multiplier);
        return;
    }
    else
    {
        a_rmp_ptr->output = 0;
        a_rmp_ptr->ramp_inc = (a_rmp_ptr->sr_recip) / (a_rmp_ptr->ramp_time);
    }
        
}

/*Glide with constant rate in seconds-per-octave*/
void v_rmp_retrigger_glide_r(t_ramp_env* a_rmp_ptr, float a_time, float a_current_note, float a_next_note)
{
    a_rmp_ptr->output = 0;
    a_rmp_ptr->output_multiplier = a_next_note - a_current_note;
    a_rmp_ptr->ramp_time = a_time * (a_rmp_ptr->output_multiplier) * .083333;
        
    /*Turn off if true*/
    if((a_rmp_ptr->ramp_time) <= .05)
    {
        a_rmp_ptr->output = 1;
        a_rmp_ptr->output_multiplied = (a_rmp_ptr->output_multiplier);
        return;
    }
    else
    {
        a_rmp_ptr->output = 0;
        a_rmp_ptr->ramp_inc = (a_rmp_ptr->sr_recip) / (a_rmp_ptr->ramp_time);
    }
}

/*t_ramp_env * g_rmp_get_ramp_env(
 * float a_sr  //sample rate
 * )
 */
t_ramp_env * g_rmp_get_ramp_env(float a_sr)
{
    t_ramp_env * f_result = (t_ramp_env*)malloc(sizeof(t_ramp_env));
    f_result->sr = a_sr;
    f_result->sr_recip = 1/a_sr;
    f_result->output_multiplied = 0;
    f_result->output_multiplier = 1;
    f_result->ramp_inc = .01;
    f_result->ramp_time = .05;
    f_result->output = 0;
    
    return f_result;
}

#ifdef	__cplusplus
}
#endif

#endif	/* RAMP_ENV_H */

