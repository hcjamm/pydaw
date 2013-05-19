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



inline void f_rmp_run_ramp(t_ramp_env*__restrict a_rmp_ptr)
{
    if((a_rmp_ptr->output_multiplier) == 0.0f )
    {
        a_rmp_ptr->output_multiplied = 0.0f;
        return;
    }
    
    if((a_rmp_ptr->output) == 1.0f)
    {
        a_rmp_ptr->output_multiplied = (a_rmp_ptr->output_multiplier);
        return;
    }
    
    a_rmp_ptr->output = (a_rmp_ptr->output) + (a_rmp_ptr->ramp_inc);
    
    if((a_rmp_ptr->output) >= 1.0f)
    {
        a_rmp_ptr->output = 1.0f;
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
void v_rmp_set_time(t_ramp_env*__restrict a_rmp_ptr,float a_time)
{
    a_rmp_ptr->ramp_time = a_time;
       
    if((a_rmp_ptr->ramp_time) <= .01f)
    {
        a_rmp_ptr->output = 1.0f;
        a_rmp_ptr->output_multiplied = (a_rmp_ptr->output_multiplier);
        return;
    }
    else
    {
        a_rmp_ptr->output = 0.0f;
        a_rmp_ptr->ramp_inc = (a_rmp_ptr->sr_recip) / (a_rmp_ptr->ramp_time);
    }
}

/*void v_rmp_retrigger(
 * t_ramp_env* a_rmp_ptr, 
 * float a_time, 
 * float a_multiplier)
 */
void v_rmp_retrigger(t_ramp_env*__restrict a_rmp_ptr, float a_time, float a_multiplier)
{
    a_rmp_ptr->output = 0.0f;
    a_rmp_ptr->ramp_time = a_time;
    a_rmp_ptr->output_multiplier = a_multiplier;
        
    if((a_rmp_ptr->ramp_time) <= .05f)
    {
        a_rmp_ptr->output = 1.0f;
        a_rmp_ptr->output_multiplied = (a_rmp_ptr->output_multiplier);
        return;
    }
    else
    {
        a_rmp_ptr->output = 0.0f;
        a_rmp_ptr->ramp_inc = (a_rmp_ptr->sr_recip) / (a_rmp_ptr->ramp_time);
    }
    
}

/*Glide with constant time in seconds*/
void v_rmp_retrigger_glide_t(t_ramp_env*__restrict a_rmp_ptr, float a_time, float a_current_note, float a_next_note)
{    
    a_rmp_ptr->ramp_time = a_time;
    
    a_rmp_ptr->output_multiplier = a_next_note - a_current_note;
        
    /*Turn off if true*/
    if((a_rmp_ptr->ramp_time) <= .05f)
    {
        a_rmp_ptr->output = 1.0f;
        a_rmp_ptr->output_multiplied = (a_rmp_ptr->output_multiplier);
        return;
    }
    else
    {
        a_rmp_ptr->output = 0.0f;
        a_rmp_ptr->ramp_inc = (a_rmp_ptr->sr_recip) / (a_rmp_ptr->ramp_time);
    }
        
}

/*Glide with constant rate in seconds-per-octave*/
void v_rmp_retrigger_glide_r(t_ramp_env*__restrict a_rmp_ptr, float a_time, float a_current_note, float a_next_note)
{
    a_rmp_ptr->output = 0.0f;
    a_rmp_ptr->output_multiplier = a_next_note - a_current_note;
    a_rmp_ptr->ramp_time = a_time * (a_rmp_ptr->output_multiplier) * .083333f;
        
    /*Turn off if true*/
    if((a_rmp_ptr->ramp_time) <= .05f)
    {
        a_rmp_ptr->output = 1.0f;
        a_rmp_ptr->output_multiplied = (a_rmp_ptr->output_multiplier);
        return;
    }
    else
    {
        a_rmp_ptr->output = 0.0f;
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
    f_result->sr_recip = 1.0f/a_sr;
    f_result->output_multiplied = 0.0f;
    f_result->output_multiplier = 1.0f;
    f_result->ramp_inc = .01f;
    f_result->ramp_time = .05f;
    f_result->output = 0.0f;
    
    return f_result;
}

#ifdef	__cplusplus
}
#endif

#endif	/* RAMP_ENV_H */

