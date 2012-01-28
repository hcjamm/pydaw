/* 
 * File:   adsr.h
 This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

 */

#ifndef ADSR_H
#define	ADSR_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include "../../../constants.h"
#include "../../lib/amp.h"

/*TODO:  Add an option to start the attack at -20db or so...*/
typedef struct st_adsr
{
    float a_inc;
    float a_time;
    float d_inc;
    float d_time;
    float s_value;
    
    float d_recip;
    float r_recip;
    
    float r_inc;
    float r_time;
    
    float sr_recip;
        
    int stage;  //0=a,1=d,2=s,3=r,4=inactive
    
    float output;
}t_adsr;

void v_adsr_set_a_time(t_adsr*, float);
void v_adsr_set_d_time(t_adsr*, float);
void v_adsr_set_s_value(t_adsr*, float);
void v_adsr_set_s_value_db(t_adsr*, float);
void v_adsr_set_r_time(t_adsr*, float);

void v_adsr_set_adsr_db(t_adsr*, float, float, float, float);
void v_adsr_set_adsr(t_adsr*, float, float, float, float);

void v_adsr_retrigger(t_adsr *);
void v_adsr_release(t_adsr *);
void v_adsr_run(t_adsr *);

t_adsr * g_adsr_get_adsr(float);

void v_adsr_set_a_time(t_adsr* a_adsr_ptr, float a_time)
{
    if((a_adsr_ptr->a_time) == a_time)
        return;
    
    a_adsr_ptr->a_time = a_time;
    
    if(a_time <= 0)
    {
        a_adsr_ptr->a_inc = 1;
    }
    else
    {
        a_adsr_ptr->a_inc = (a_adsr_ptr->sr_recip) / (a_adsr_ptr->a_time);
    }
        
}

void v_adsr_set_d_time(t_adsr* a_adsr_ptr, float a_time)
{
    if((a_adsr_ptr->d_time) == a_time)
        return;
    
    if(a_time <= 0)
    {
        a_adsr_ptr->d_time = .05;
    }
    else
    {
        a_adsr_ptr->d_time = a_time;        
    }
    
    //printf("Setting D time to %f\n", (a_adsr_ptr->d_time));
    
    a_adsr_ptr->d_inc = ((a_adsr_ptr->sr_recip) / (a_adsr_ptr->d_time)) * -1;    
}

void v_adsr_set_r_time(t_adsr* a_adsr_ptr, float a_time)
{
    if((a_adsr_ptr->r_time) == a_time)
        return;
    
    if(a_time <= 0)
    {
        a_adsr_ptr->r_time = .05;
    }
    else
    {
        a_adsr_ptr->r_time = a_time;        
    }    
    
    a_adsr_ptr->r_inc = ((a_adsr_ptr->sr_recip) / (a_adsr_ptr->r_time)) * -1;
    
    //printf("Setting R time to %f\n", (a_adsr_ptr->r_time));
    
}

void v_adsr_set_s_value(t_adsr* a_adsr_ptr, float a_value)
{
    a_adsr_ptr->s_value = a_value;
    
    if((a_adsr_ptr->s_value) <= 0)
    {
        a_adsr_ptr->s_value = .001;
    }
    
    a_adsr_ptr->d_recip = (1/(1-(a_adsr_ptr->s_value)));
    a_adsr_ptr->r_recip = (1/(a_adsr_ptr->s_value));
    
    //printf("Setting S value to %f\n", (a_adsr_ptr->s_value));
}

void v_adsr_set_s_value_db(t_adsr* a_adsr_ptr, float a_value)
{
    v_adsr_set_s_value(a_adsr_ptr, f_db_to_linear(a_value));
}

void v_adsr_set_adsr(t_adsr* a_adsr_ptr, float a_a, float a_d, float a_s, float a_r)
{
    v_adsr_set_a_time(a_adsr_ptr, a_a);
    v_adsr_set_d_time(a_adsr_ptr, a_d);
    v_adsr_set_s_value(a_adsr_ptr, a_s);
    v_adsr_set_r_time(a_adsr_ptr, a_r);
}

void v_adsr_set_adsr_db(t_adsr* a_adsr_ptr, float a_a, float a_d, float a_s, float a_r)
{
    v_adsr_set_a_time(a_adsr_ptr, a_a);
    v_adsr_set_d_time(a_adsr_ptr, a_d);
    v_adsr_set_s_value_db(a_adsr_ptr, a_s);
    v_adsr_set_r_time(a_adsr_ptr, a_r);
}

void v_adsr_retrigger(t_adsr * a_adsr_ptr)
{
    a_adsr_ptr->stage = 0;
    a_adsr_ptr->output = 0;    
}

void v_adsr_release(t_adsr * a_adsr_ptr)
{
    a_adsr_ptr->stage = 3;    
}

t_adsr * g_adsr_get_adsr(float a_sr_recip)
{
    t_adsr * f_result = (t_adsr*)malloc(sizeof(t_adsr));
    
    f_result->sr_recip = a_sr_recip;
    
    v_adsr_set_a_time(f_result, .05);
    v_adsr_set_d_time(f_result, .5);
    v_adsr_set_s_value_db(f_result, -12);
    v_adsr_set_r_time(f_result, .5);

    v_adsr_retrigger(f_result);
    
    return f_result;
}

void v_adsr_run(t_adsr * a_adsr_ptr)
{
    if((a_adsr_ptr->stage) != 4)
    {         
        switch(a_adsr_ptr->stage)
        {            
            case 0:
                a_adsr_ptr->output = (a_adsr_ptr->output) + (a_adsr_ptr->a_inc);
                if((a_adsr_ptr->output) >= 1)
                {
                    a_adsr_ptr->stage = 1;
                    //printf("ADSR stage1\n");
                }
                break;
            case 1:
                a_adsr_ptr->output =  (a_adsr_ptr->output) + (a_adsr_ptr->d_inc);
                if((a_adsr_ptr->output) <= (a_adsr_ptr->s_value))
                {
                    a_adsr_ptr->stage = 2;
                    //printf("ADSR stage2\n");
                }
                break;
            case 2:
                //Do nothing, we are sustaining
                break;
            case 3:
                /*Currently, this would actually take longer to release if the note off arrives
                 before the decay stage finishes, I may fix it later*/
                a_adsr_ptr->output =  (a_adsr_ptr->output) + (a_adsr_ptr->r_inc);
                if((a_adsr_ptr->output) <= 0)
                {
                    a_adsr_ptr->output = 0;
                    a_adsr_ptr->stage = 4;
                    //printf("ADSR stage4\n");
                }
                break;
        }
    }
}


#ifdef	__cplusplus
}
#endif

#endif	/* ADSR_H */

