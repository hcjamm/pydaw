/* 
 * File:   osc_core.h
 * Author: vm-user
 *
 * Created on January 7, 2012, 7:19 PM
 */

#ifndef OSC_CORE_H
#define	OSC_CORE_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "pitch_core.h"
#include <stdlib.h>
   
    
typedef struct st_osc_core
{    
    float output;
    
}t_osc_core;


float f_run_osc(t_osc_core *, float);
t_osc_core * g_get_osc_core(float);


t_osc_core * g_get_osc_core(float a_sr)
{
    t_osc_core * f_result = (t_osc_core*)malloc(sizeof(t_osc_core)); 
    f_result->output = 0;    
    return f_result;
}

float f_run_osc(t_osc_core *a_core, float a_inc)
{
    a_core->output += a_inc;
    
    if(a_core->output >= 1)
    {
        a_core->output -= 1;
    }
    
    return a_core->output;
}


#ifdef	__cplusplus
}
#endif

#endif	/* OSC_CORE_H */

