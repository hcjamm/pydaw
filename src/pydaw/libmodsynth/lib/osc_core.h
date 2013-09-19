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

#ifndef OSC_CORE_H
#define	OSC_CORE_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "pitch_core.h"
#include <stdlib.h>
   
    
typedef struct st_osc_core
{    
    float output;   //range:  0 to 1    
    t_lin_interpolater * linear;
}t_osc_core;


void v_run_osc(t_osc_core *, float);
t_osc_core * g_get_osc_core();
void v_osc_core_free(t_osc_core *);


t_osc_core * g_get_osc_core()
{
    t_osc_core * f_result = (t_osc_core*)malloc(sizeof(t_osc_core)); 
    f_result->output = 0.0f;    
    f_result->linear = g_lin_get();
    return f_result;
}

void v_osc_core_free(t_osc_core * a_osc)
{
    free(a_osc->linear);
    free(a_osc);
}

/* void v_run_osc(
 * t_osc_core *a_core, 
 * float a_inc) //The increment to run the oscillator by.
 * The oscillator will increment until it reaches 1, then resets to (value - 1), for each oscillation
 */
void v_run_osc(t_osc_core *a_core, float a_inc)
{
    a_core->output = (a_core->output) + a_inc;
    
    if(a_core->output >= 1.0f)
    {
        a_core->output = (a_core->output - 1.0f);
    }    
}

int v_run_osc_sync(t_osc_core *a_core, float a_inc)
{
    a_core->output = (a_core->output) + a_inc;
    
    if(a_core->output >= 1.0f)
    {
        a_core->output = (a_core->output - 1.0f);
        return 1;
    }    
    else
    {
        return 0;
    }
}


#ifdef	__cplusplus
}
#endif

#endif	/* OSC_CORE_H */

