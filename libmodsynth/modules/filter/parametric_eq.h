/* 
 * File:   parametric_eq.h
 * Author: Jeff Hubbard
 *
 * Simple parametric EQ.  Completely unoptimized in it's present state, more work to follow.
 * 
 * Created on March 22, 2012, 10:00 PM
 */

#ifndef PARAMETRIC_EQ_H
#define	PARAMETRIC_EQ_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include "../../lib/pitch_core.h"
#include "../../lib/fast_sine.h"
#include "../../lib/amp.h"
#include "../../constants.h"
#include <math.h>
    
typedef struct st_peq_para_eq
{
    float gain_db;
    float gain_linear;
    float quality;
    float c0, c1, c2;
    float ldelay1, ldelay2, rdelay1, rdelay2;
    float li1, li2, ri1, ri2, tmp_set, tmp_run, a;
    float arc;
    float sample_rate;
    float pitch;
    float hz;
    float output0, output1;
    t_pit_pitch_core * pitch_core;
    t_amp * amp_ptr;
}t_peq_para_eq;

void v_peq_set_eq(t_peq_para_eq*,float,float,float);
void v_peq_run(t_peq_para_eq*,float,float);
t_peq_para_eq * g_peq_get(float);

/* void v_peq_set_eq(
 * t_peq_para_eq* a_peq, 
 * float a_pitch, //EQ frequency in MIDI note number, typically 20 to 120
 * float a_gain,  //Gain in decibels.  Typically -24 to 24
 * float a_quality) //Resonance or Q.  TODO: Range
 */
void v_peq_set_eq(t_peq_para_eq* a_peq, float a_pitch, float a_gain,float a_quality)
{
    if((a_peq->pitch) != a_pitch)
    {
        a_peq->pitch = a_pitch;
        a_peq->hz = f_pit_midi_note_to_hz_fast(a_pitch, a_peq->pitch_core);
        a_peq->arc=a_peq->hz * PI/(a_peq->sample_rate*0.5); /*TODO:  Only calculate the last part at initialization*/
    }
    
    if((a_peq->gain_db) != a_gain)
    {
        a_peq->gain_db = a_gain;
        a_peq->gain_linear= f_db_to_linear_fast(a_gain, a_peq->amp_ptr);  //(2 ^ (a_peq->gain_db/6));        
    }
    
    if((a_peq->quality) != a_quality)
    {
        a_peq->quality = a_quality;
        a_peq->a=(sin(a_peq->arc)*a_quality) * (a_peq->gain_linear < 1 ? 1 : 0.25);
        a_peq->tmp_set=1/(1+a_peq->a);
    }

    a_peq->c0=a_peq->tmp_set*a_peq->a*(a_peq->gain_linear-1);
    a_peq->c1=a_peq->tmp_set*2*cos(a_peq->arc);
    a_peq->c2=a_peq->tmp_set*(a_peq->a-1);
}

void v_peq_run(t_peq_para_eq* a_peq,float a_input0, float a_input1)
{
    a_peq->tmp_run=a_peq->c0*(a_input0-a_peq->ldelay2) + a_peq->c1*a_peq->li1 + a_peq->c2*a_peq->li2;
    a_peq->ldelay2=a_peq->ldelay1; 
    a_peq->ldelay1=a_input0;
    a_peq->li2=a_peq->li1; 
    a_peq->output0 += (a_peq->li1=a_peq->tmp_run);

    a_peq->tmp_run=a_peq->c0*(a_input1-a_peq->rdelay2) + a_peq->c1*a_peq->ri1 + a_peq->c2*a_peq->ri2;
    a_peq->rdelay2=a_peq->rdelay1; 
    a_peq->rdelay1=a_input1; 
    a_peq->ri2=a_peq->ri1; 
    a_peq->output1 += (a_peq->ri1=a_peq->tmp_run);
}

t_peq_para_eq * g_peq_get(float a_sample_rate)
{
    t_peq_para_eq * f_result = (t_peq_para_eq*)malloc(sizeof(t_peq_para_eq));
    
    f_result->arc = 0;
    f_result->c0 = 0;
    f_result->c1 = 0;
    f_result->c2 = 0;
    f_result->gain_db = 0;
    f_result->gain_linear = 1;
    f_result->ldelay1 = 0;
    f_result->ldelay2 = 0;
    f_result->li1 = 0;
    f_result->li2 = 0;
    f_result->quality = 0;
    f_result->rdelay1 = 0;
    f_result->rdelay2 = 0;
    f_result->ri1 = 0;
    f_result->ri2 = 0;
    f_result->tmp_set = 0;
    f_result->tmp_run = 0;
    f_result->a = 0;
    f_result->sample_rate = a_sample_rate;
    f_result->output0 = 0;
    f_result->output1 = 0;        
    f_result->pitch_core = g_pit_get();
    f_result->amp_ptr = g_amp_get();
    f_result->pitch = 6;
    f_result->hz = 2;
    f_result->tmp_set = 0;
    
    v_peq_set_eq(f_result, 82, -6.0f, 0.5);
    
    
    return f_result;
}

#ifdef	__cplusplus
}
#endif

#endif	/* PARAMETRIC_EQ_H */

