/* 
 * File:   soft_clipper.h
 * Author: Jeff Hubbard
 * 
 * Performs linear soft clipping of an input signal
 *
 * Created on April 13, 2012, 2:46 PM
 */

#ifndef SOFT_CLIPPER_H
#define	SOFT_CLIPPER_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include "../../lib/amp.h"

typedef struct st_scl_soft_clipper
{
    float threshold_db;
    float threshold_linear, threshold_linear_neg;
    float amount;
    float output;
    float temp;
    t_amp * amp_ptr;
}t_soft_clipper;

void v_scl_set(t_soft_clipper*,float,float);
void v_scl_run(t_soft_clipper*,float);
t_soft_clipper * g_scl_get();

void v_scl_set(t_soft_clipper* a_scl, float a_threshold_db, float a_amount)
{
    if(a_threshold_db != (a_scl->threshold_db))
    {
        a_scl->threshold_db = a_threshold_db;
        a_scl->threshold_linear = f_db_to_linear_fast(a_threshold_db, a_scl->amp_ptr);
        a_scl->threshold_linear_neg = (a_scl->threshold_linear) * -1;
    }
    
    a_scl->amount = a_amount;
}

void v_scl_run(t_soft_clipper* a_scl,float a_input)
{
    if(a_input > (a_scl->threshold_linear))
    {
        a_scl->temp = a_input - (a_scl->threshold_linear);
        a_scl->output = ((a_scl->temp) * (a_scl->amount)) + (a_scl->threshold_linear);
    }
    else if(a_input > (a_scl->threshold_linear_neg))
    {
        a_scl->temp = a_input + (a_scl->threshold_linear_neg);
        a_scl->output = ((a_scl->temp) * (a_scl->amount)) - (a_scl->threshold_linear);        
    }
}

t_soft_clipper * g_scl_get()
{
    t_soft_clipper * f_result = (t_soft_clipper*)malloc(sizeof(t_soft_clipper));
    
    f_result->amount = 1;
    f_result->amp_ptr = g_amp_get();
    f_result->output = 0;
    f_result->temp = 0;
    f_result->threshold_db = 0;
    f_result->threshold_linear = 1;
    f_result->threshold_linear_neg = -1;
    
    return f_result;
}

#ifdef	__cplusplus
}
#endif

#endif	/* SOFT_CLIPPER_H */

