/* 
 * File:   adsr.h
 * 
 * Author: Jeff Hubbard
 * 
 * This file provides t_adsr, a basic ADSR envelope
 * 
 */

#ifndef ADSR_H
#define	ADSR_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include "../../constants.h"
#include "../../lib/amp.h"

#define ADSR_DB 24.0f    
#define ADSR_DB_THRESHOLD -24.0f
#define ADSR_DB_THRESHOLD_LINEAR 0.0625f

#define ADSR_DB_RELEASE 36.0f    
#define ADSR_DB_THRESHOLD_RELEASE -36.0f
#define ADSR_DB_THRESHOLD_LINEAR_RELEASE  0.01575f
    
typedef struct st_adsr
{
    float a_inc;
    float a_time;
    float d_inc;
    float d_time;
    float s_value;    
    //float d_recip;
    //float r_recip;    
    float r_inc;
    float r_time;
    
    float sr_recip;        
    int stage;  //0=a,1=d,2=s,3=r,4=inactive    
    float output;
    
    float output_db;
    float a_inc_db;
    float d_inc_db;
    float r_inc_db;
    
    t_amp * amp_ptr;
}t_adsr;

void v_adsr_set_a_time(t_adsr*, float);
void v_adsr_set_d_time(t_adsr*, float);
void v_adsr_set_s_value(t_adsr*, float);
void v_adsr_set_s_value_db(t_adsr*, float);
void v_adsr_set_r_time(t_adsr*, float);
void v_adsr_set_fast_release(t_adsr*);

void v_adsr_set_adsr_db(t_adsr*, float, float, float, float);
void v_adsr_set_adsr(t_adsr*, float, float, float, float);

void v_adsr_retrigger(t_adsr *);
void v_adsr_release(t_adsr *);
void v_adsr_run(t_adsr *);
void v_adsr_run_db(t_adsr *);

t_adsr * g_adsr_get_adsr(float);

/* void v_adsr_set_a_time(
 * t_adsr* a_adsr_ptr, 
 * float a_time)  //time in seconds
 * 
 * Sets the envelope attack time
 */
void v_adsr_set_a_time(t_adsr*__restrict a_adsr_ptr, float a_time)
{
    if((a_adsr_ptr->a_time) == a_time)
        return;
    
    a_adsr_ptr->a_time = a_time;
    
    if(a_time <= 0.0f)
    {
        a_adsr_ptr->a_inc = 1.0f;
    }
    else
    {
        a_adsr_ptr->a_inc = (a_adsr_ptr->sr_recip) / (a_adsr_ptr->a_time);
    }
        
}

/* void v_adsr_set_d_time(
 * t_adsr* a_adsr_ptr, 
 * float a_time) //time in seconds
 * 
 * Sets the envelope decay time
 */
void v_adsr_set_d_time(t_adsr*__restrict a_adsr_ptr, float a_time)
{
    if((a_adsr_ptr->d_time) == a_time)
        return;
    
    if(a_time <= 0.0f)
    {
        a_adsr_ptr->d_time = .05;
    }
    else
    {
        a_adsr_ptr->d_time = a_time;        
    }
    
    //printf("Setting D time to %f\n", (a_adsr_ptr->d_time));
    
    a_adsr_ptr->d_inc = ((a_adsr_ptr->sr_recip) / (a_adsr_ptr->d_time)) * -1.0f;    
}

/* void v_adsr_set_r_time(
 * t_adsr* a_adsr_ptr, 
 * float a_time) //time in seconds
 * 
 * Sets the envelope release time
 */
void v_adsr_set_r_time(t_adsr*__restrict a_adsr_ptr, float a_time)
{
    if((a_adsr_ptr->r_time) == a_time)
        return;
    
    if(a_time <= 0.0f)
    {
        a_adsr_ptr->r_time = .05f;
    }
    else
    {
        a_adsr_ptr->r_time = a_time;        
    }    
    
    a_adsr_ptr->r_inc = ((a_adsr_ptr->sr_recip) / (a_adsr_ptr->r_time)) * -1.0f;
    
    //printf("Setting R time to %f\n", (a_adsr_ptr->r_time));
    
}

/* void v_adsr_set_fast_release(t_adsr* a_adsr_ptr)
 * 
 * This method is for killing voices by allowing a quick fade-out instead of directly stealing a voice, which should 
 * allow a quick transition without a click
 * TODO:  The total time of the fadeout is not consistent between different sample rates.
 */
void v_adsr_set_fast_release(t_adsr*__restrict a_adsr_ptr)
{
    a_adsr_ptr->r_time = .05f;
    a_adsr_ptr->r_inc = -.0002f;
    a_adsr_ptr->stage = 3;
}

/* void v_adsr_set_s_value(
 * t_adsr* a_adsr_ptr, 
 * float a_value) //The sustain value, range: 0 to 1
 */
void v_adsr_set_s_value(t_adsr*__restrict a_adsr_ptr, float a_value)
{
    a_adsr_ptr->s_value = a_value;
    
    if((a_adsr_ptr->s_value) <= 0.0f)
    {
        a_adsr_ptr->s_value = .001f;
    }
    
    //a_adsr_ptr->d_recip = (1.0f/(1.0f-(a_adsr_ptr->s_value)));
    //a_adsr_ptr->r_recip = (1.0f/(a_adsr_ptr->s_value));
    
    //printf("Setting S value to %f\n", (a_adsr_ptr->s_value));
}

/* void v_adsr_set_s_value_db(
 * t_adsr* a_adsr_ptr, 
 * float a_value)  //The decibel value of sustain, typically -30 to 0
 */
void v_adsr_set_s_value_db(t_adsr*__restrict a_adsr_ptr, float a_value)
{
    v_adsr_set_s_value(a_adsr_ptr, f_db_to_linear_fast(a_value, a_adsr_ptr->amp_ptr));
}

/* void v_adsr_set_adsr(
 * t_adsr* a_adsr_ptr, 
 * float a_a, //attack
 * float a_d, //decay
 * float a_s, //sustain
 * float a_r) //release
 * 
 * Set allADSR values, with a range of 0 to 1 for sustain
 */
void v_adsr_set_adsr(t_adsr*__restrict a_adsr_ptr, float a_a, float a_d, float a_s, float a_r)
{
    v_adsr_set_a_time(a_adsr_ptr, a_a);
    v_adsr_set_d_time(a_adsr_ptr, a_d);
    v_adsr_set_s_value(a_adsr_ptr, a_s);
    v_adsr_set_r_time(a_adsr_ptr, a_r);
}


/* void v_adsr_set_adsr(
 * t_adsr* a_adsr_ptr, 
 * float a_a, //attack
 * float a_d, //decay
 * float a_s, //sustain
 * float a_r) //release
 * 
 * Set all ADSR values, with a range of -30 to 0 for sustain
 */
void v_adsr_set_adsr_db(t_adsr*__restrict a_adsr_ptr, float a_a, float a_d, float a_s, float a_r)
{
    v_adsr_set_a_time(a_adsr_ptr, a_a);
    v_adsr_set_d_time(a_adsr_ptr, a_d);
    v_adsr_set_s_value_db(a_adsr_ptr, a_s);
    v_adsr_set_r_time(a_adsr_ptr, a_r);
    
    a_adsr_ptr->a_inc_db = (a_adsr_ptr->a_inc) * ADSR_DB;
    a_adsr_ptr->d_inc_db = (a_adsr_ptr->d_inc) * (ADSR_DB);
    a_adsr_ptr->r_inc_db = (a_adsr_ptr->r_inc) * (ADSR_DB_RELEASE);    
}

/* void v_adsr_retrigger(t_adsr * a_adsr_ptr)
 * 
 * Reset the ADSR to the beginning of the attack phase
 */
void v_adsr_retrigger(t_adsr *__restrict a_adsr_ptr)
{
    a_adsr_ptr->stage = 0;
    a_adsr_ptr->output = 0.0f;
    a_adsr_ptr->output_db = ADSR_DB_THRESHOLD;
}

/* void v_adsr_release(t_adsr * a_adsr_ptr)
 * 
 * Set the ADSR to the release phase
 */
void v_adsr_release(t_adsr *__restrict a_adsr_ptr)
{
    if(a_adsr_ptr->stage < 3)
    {
        a_adsr_ptr->stage = 3;
    }
}

/* t_adsr * g_adsr_get_adsr(
 * float a_sr_recip) // 1.0f/sample_rate (TODO: use sample_rate instead)
 * 
 */
t_adsr * g_adsr_get_adsr(float a_sr_recip)
{
    t_adsr * f_result = (t_adsr*)malloc(sizeof(t_adsr));
    
    f_result->sr_recip = a_sr_recip;
    f_result->amp_ptr = g_amp_get();
        
    f_result->output = 0.0f;
    f_result->stage = 4;
    
    //Set these to nonsensical values so that comparisons aren't happening with invalid numbers
    f_result->a_inc = -100.5f;
    f_result->a_time = -100.5f;
    f_result->d_inc = -100.5f;
    //f_result->d_recip = -100.5f;
    f_result->d_time =  -100.5f;
    f_result->r_inc = -100.5f;
    //f_result->r_recip = -100.5f;
    f_result->r_time = -100.5f;
    f_result->s_value = -100.5f;
    
    f_result->output_db = -100.5f;
    f_result->a_inc_db = 0.1f;
    f_result->d_inc_db = 0.1f;
    f_result->r_inc_db = 0.1f;
    
    v_adsr_set_a_time(f_result, .05);
    v_adsr_set_d_time(f_result, .5);
    v_adsr_set_s_value_db(f_result, -12.0f);
    v_adsr_set_r_time(f_result, .5);
    
    return f_result;
}

/* void v_adsr_run(t_adsr * a_adsr_ptr)
 * 
 * Run the ADSR envelope
 */
void v_adsr_run(t_adsr *__restrict a_adsr_ptr)
{
    if((a_adsr_ptr->stage) != 4)
    {         
        switch(a_adsr_ptr->stage)
        {            
            case 0:
                a_adsr_ptr->output = (a_adsr_ptr->output) + (a_adsr_ptr->a_inc);
                if((a_adsr_ptr->output) >= 1.0f)
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
                if((a_adsr_ptr->output) <= 0.0f)
                {
                    a_adsr_ptr->output = 0.0f;
                    a_adsr_ptr->stage = 4;
                    //printf("ADSR stage4\n");
                }
                break;
        }
    }
}

void v_adsr_run_db(t_adsr *__restrict a_adsr_ptr)
{
    if((a_adsr_ptr->stage) != 4)
    {         
        switch(a_adsr_ptr->stage)
        {            
            case 0:
                if((a_adsr_ptr->output) < ADSR_DB_THRESHOLD_LINEAR)
                {
                    a_adsr_ptr->output = (a_adsr_ptr->output) + (a_adsr_ptr->a_inc);
                }
                else
                {
                    a_adsr_ptr->output_db = (a_adsr_ptr->output_db) + (a_adsr_ptr->a_inc_db);
                    a_adsr_ptr->output = f_db_to_linear((a_adsr_ptr->output_db), a_adsr_ptr->amp_ptr);
                    
                    if((a_adsr_ptr->output) >= 1.0f)
                    {
                        a_adsr_ptr->stage = 1;                    
                    }
                }
                
                break;
            case 1:
                if((a_adsr_ptr->output) < ADSR_DB_THRESHOLD_LINEAR)
                {
                    a_adsr_ptr->output =  (a_adsr_ptr->output) + (a_adsr_ptr->d_inc);
                }
                else
                {
                    a_adsr_ptr->output_db = (a_adsr_ptr->output_db) + (a_adsr_ptr->d_inc_db);
                    a_adsr_ptr->output = f_db_to_linear((a_adsr_ptr->output_db), a_adsr_ptr->amp_ptr);
                }
                
                if((a_adsr_ptr->output) <= (a_adsr_ptr->s_value))
                {
                    a_adsr_ptr->stage = 2;                    
                }
                break;
            case 2:
                //Do nothing, we are sustaining
                break;
            case 3:
                if((a_adsr_ptr->output) < ADSR_DB_THRESHOLD_LINEAR_RELEASE)
                {
                    a_adsr_ptr->output =  (a_adsr_ptr->output) + (a_adsr_ptr->r_inc);
                    
                    if((a_adsr_ptr->output) <= 0.0f)
                    {
                        a_adsr_ptr->output = 0.0f;
                        a_adsr_ptr->stage = 4;
                        //printf("ADSR stage4\n");
                    }
                }
                else
                {
                    a_adsr_ptr->output_db = (a_adsr_ptr->output_db) + (a_adsr_ptr->r_inc_db);
                    a_adsr_ptr->output = f_db_to_linear((a_adsr_ptr->output_db), a_adsr_ptr->amp_ptr);
                }
                
                break;
        }
    }
}


#ifdef	__cplusplus
}
#endif

#endif	/* ADSR_H */

