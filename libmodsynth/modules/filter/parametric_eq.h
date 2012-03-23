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
#include "../../constants.h"
    
typedef struct st_peq_para_eq
{
    float gain_db;
    float gain_linear;
    float quality;
    float c0, c1, c2;
    float ldelay1, ldelay2, rdelay1, rdelay2;
    float li1, li2, ri1, ri2, tmp, a;
    float arc;
    float sample_rate;
    float pitch;
    float hz;
    float output0, output1;
    t_pit_pitch_core * pitch_core;
}t_peq_para_eq;

void v_peq_set_eq(t_peq_para_eq*,float,float,float);
void v_peq_run(t_peq_para_eq*,float);
t_peq_para_eq * g_peq_get(float);

void v_peq_set_eq(t_peq_para_eq* a_peq, float a_pitch, float a_gain,float a_quality)
{
    a_peq->pitch = a_pitch;
    a_peq->hz = f_pit_midi_note_to_hz_fast(a_pitch, a_peq->pitch_core);
    a_peq->arc=a_peq->pitch * PI/(a_peq->sample_rate*0.5);
    a_peq->gain_db = a_gain;
    a_peq->gain_linear=(2 ^ (a_peq->gain_db/6));
    a_peq->quality = a_quality;
    a_peq->a=(sin(a_peq->arc)*a_quality) * (a_peq->gain_linear < 1 ? 1 : 0.25);
    a_peq->tmp=1/(1+a_peq->a);  

    a_peq->c0=a_peq->tmp*a_peq->a*(a_peq->gain_linear-1);
    a_peq->c1=a_peq->tmp*2*cos(a_peq->arc);
    a_peq->c2=a_peq->tmp*(a_peq->a-1);
}

void v_peq_run(t_peq_para_eq* a_peq,float a_input0, float a_input1)
{
    a_peq->tmp=a_peq->c0*(a_input0-a_peq->ldelay2) + a_peq->c1*a_peq->li1 + a_peq->c2*a_peq->li2;
    a_peq->ldelay2=a_peq->ldelay1; a_peq->ldelay1=a_input0;
    a_peq->li2=a_peq->li1; a_peq->output0 += (a_peq->li1=a_peq->tmp);

    a_peq->tmp=a_peq->c0*(a_input1-a_peq->rdelay2) + a_peq->c1*a_peq->ri1 + a_peq->c2*a_peq->ri2;
    a_peq->rdelay2=a_peq->rdelay1; a_peq->rdelay1=a_input1; 
    a_peq->ri2=a_peq->ri1; a_peq->output1 += (a_peq->ri1=a_peq->tmp);
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
    f_result->tmp = 0;
    f_result->a = 0;
    f_result->sample_rate = a_sample_rate;
    f_result->output0 = 0;
    f_result->output1 = 0;
    
    f_result->pitch_core = g_pit_get();
    
    return f_result;
}

#ifdef	__cplusplus
}
#endif

#endif	/* PARAMETRIC_EQ_H */

