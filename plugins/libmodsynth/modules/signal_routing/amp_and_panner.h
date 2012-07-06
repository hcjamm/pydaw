/* 
 * File:   amp_and_panner.h
 * Author: jeffh
 *
 * Created on July 5, 2012, 8:58 PM
 */

#ifndef AMP_AND_PANNER_H
#define	AMP_AND_PANNER_H

#include "../../lib/amp.h"
#include "../../lib/lms_math.h"

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct st_amp_and_panner
{
    float amp_db;
    float pan_law;
    float pan; //0 to 1
    float pan_amt;
    float amp_linear0;
    float amp_linear1;
    float output0;
    float output1;
    t_amp * amp_ptr;
}t_amp_and_panner;

void v_app_set(t_amp_and_panner*,float,float);
void v_app_run(t_amp_and_panner*,float,float);
t_amp_and_panner * g_app_get();

void v_app_set(t_amp_and_panner* a_app,float a_db,float a_pan)
{
    a_app->amp_db = a_db;
    a_app->pan = a_pan;
    
    a_app->amp_linear0 = f_db_to_linear_fast(
            ((f_lms_ceiling(((a_pan * 2) - 1), 1) * (a_app->pan_amt)) + a_db)
            , a_app->amp_ptr);
    
    a_app->amp_linear1 = f_db_to_linear_fast(
            ((f_lms_ceiling((1 - (a_pan * 2)), 1) * (a_app->pan_amt)) + a_db)
            , a_app->amp_ptr);
}

void v_app_run(t_amp_and_panner* a_app, float a_in0, float a_in1)
{
    a_app->output0 = a_in0 * (a_app->amp_linear0);
    a_app->output1 = a_in1 * (a_app->amp_linear1);
}

t_amp_and_panner * g_app_get()
{
    t_amp_and_panner * f_result = (t_amp_and_panner*)malloc(sizeof(t_amp_and_panner));
    f_result->amp_db = 0.0f;
    f_result->pan_law = 0.0f;
    f_result->pan = 0.0f;
    f_result->pan_amt = 0.0f;
    f_result->amp_linear0 = 1.0f;
    f_result->amp_linear1 = 1.0f;
    f_result->output0 = 0.0f;
    f_result->output1 = 0.0f;
    f_result->amp_ptr = g_amp_get();
}

#ifdef	__cplusplus
}
#endif

#endif	/* AMP_AND_PANNER_H */

