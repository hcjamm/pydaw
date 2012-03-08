/* 
 * File:   reverb_digital.h
 * Author: Jeff Hubbard
 *
 * Created on January 8, 2012, 5:48 PM
 */

#ifndef REVERB_DIGITAL_H
#define	REVERB_DIGITAL_H

#ifdef	__cplusplus
extern "C" {
#endif

#define REVERB_DIGITAL_TAPS 8
    
#include "delay.h"
    
typedef struct st_rvd_reverb
{
    t_delay_simple buffer0;
    t_delay_simple buffer1;
    t_delay_tap taps0 [REVERB_DIGITAL_TAPS];
    t_delay_tap taps1 [REVERB_DIGITAL_TAPS];
    float out0;
    float out1;
    float feedback0;
    float feedback1;
    int predelay;
    int iterator;
    float delay_time [REVERB_DIGITAL_TAPS];
}t_rvd_reverb;

t_rvd_reverb * g_rvd_get_reverb(float);

t_rvd_reverb * g_rvd_get_reverb(float a_sr)
{
    t_rvd_reverb * f_result = (t_rvd_reverb*)malloc(sizeof(t_rvd_reverb));
    
    f_result->buffer0 = g_dly_get_delay(1, a_sr);
    f_result->buffer1 = g_dly_get_delay(1, a_sr);    
    f_result->out0 = 0;
    f_result->out1 = 0;
    f_result->feedback0 = 0;
    f_result->feedback1 = 0;
    f_result->predelay = 0;
    f_result->iterator = 0;
    
    int f_i = 0;
    
    while(f_i < REVERB_DIGITAL_TAPS)
    {        
        f_result->taps0 [f_i] = g_dly_get_tap();
        f_result->taps1 [f_i] = g_dly_get_tap();
        f_result->delay_time [f_i] = 0.1;
        
        f_i++;
    }
        
    return f_result;
}

void v_rvd_run_reverb(t_rvd_reverb*,float,float);
void v_rvd_set_reverb(t_rvd_reverb*,float,float);

void v_rvd_run_reverb(t_rvd_reverb* a_rvd, float a_in0, float a_in1)
{
    
    
    a_rvd->iterator = 0;
    
    while((a_rvd->iterator) < REVERB_DIGITAL_TAPS)
    {
        
        a_rvd->iterator = (a_rvd->iterator) + 1;
    }
}

void v_rvd_set_reverb(t_rvd_reverb* a_rvd, float a_time, float a_predelay)
{
    
    
    a_rvd->iterator = 0;
    
    while((a_rvd->iterator) < REVERB_DIGITAL_TAPS)
    {
        
        a_rvd->iterator = (a_rvd->iterator) + 1;
    }
}

#ifdef	__cplusplus
}
#endif

#endif	/* REVERB_DIGITAL_H */

