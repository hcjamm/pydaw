/* 
 * File:   osc_core.h
 * Author: Jeff Hubbard
 * 
 * Purpose:  This file provides the t_osc_core type, which is meant to provide base functionality
 * for oscillators, that can be easily extended in later versions of LibModSynth.
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
    float output;   //range:  0 to 1    
    t_lin_interpolater * linear;
}t_osc_core;


void v_run_osc(t_osc_core *, float);
t_osc_core * g_get_osc_core();


t_osc_core * g_get_osc_core()
{
    t_osc_core * f_result = (t_osc_core*)malloc(sizeof(t_osc_core)); 
    f_result->output = 0;    
    f_result->linear = g_lin_get();
    return f_result;
}

/* void v_run_osc(
 * t_osc_core *a_core, 
 * float a_inc) //The increment to run the oscillator by.
 * The oscillator will increment until it reaches 1, then resets to (value - 1), for each oscillation
 */
void v_run_osc(t_osc_core *a_core, float a_inc)
{
    a_core->output = (a_core->output) + a_inc;
    
    if(a_core->output >= 1)
    {
        a_core->output = (a_core->output - 1);
    }    
}


#ifdef	__cplusplus
}
#endif

#endif	/* OSC_CORE_H */

