/* 
 * File:   saturator.h
 * Author: jeffh
 *
 * Created on July 17, 2012, 7:48 PM
 */

#ifndef SATURATOR_H
#define	SATURATOR_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "../../lib/lms_math.h"
#include <math.h>

typedef struct st_sat_saturator
{
    float output0;
    float output1;
    float a;    
    float b;
    float amount;
    t_lin_interpolater * lin_interpolator;
}t_sat_saturator;

t_sat_saturator * g_sat_get();
inline void v_sat_set(t_sat_saturator*,float);
inline void v_sat_run(t_sat_saturator*,float,float);

inline void v_sat_set(t_sat_saturator* a_sat, float a_amt)
{
    if(a_amt != (a_sat->amount))
    {
        a_sat->a=(a_amt*0.005)*3.141592f;
        a_sat->b = sin((a_amt*0.005)*3.141592f);
        a_sat->amount = a_amt;
    }
    
}

inline void v_sat_run(t_sat_saturator* a_sat, float a_in0, float a_in1)
{    	
    a_sat->output0 = f_lms_min(f_lms_max( sin(f_lms_max(f_lms_min(a_in0,1.0f),-1.0f)*(a_sat->a))/(a_sat->b) ,-1.0f) ,1.0f);
    
    a_sat->output1 = f_lms_min(f_lms_max( sin(f_lms_max(f_lms_min(a_in1,1.0f),-1.0f)*(a_sat->a))/(a_sat->b) ,-1.0f) ,1.0f);
}

t_sat_saturator * g_sat_get()
{
    t_sat_saturator * f_result = (t_sat_saturator*)malloc(sizeof(t_sat_saturator));
    
    f_result->lin_interpolator = g_lin_get();
    f_result->a = 0.0f;
    f_result->b = 0.0f;
    f_result->amount = 1.0f;
    f_result->output0 = 0.0f;
    f_result->output1 = 0.0f;
    
    return f_result;
}

#ifdef	__cplusplus
}
#endif

#endif	/* SATURATOR_H */

