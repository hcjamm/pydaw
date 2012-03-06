/* 
 * File:   one_pole.h
 * Author: jeffh
 *
 * Created on March 5, 2012, 11:06 PM
 */

#ifndef ONE_POLE_H
#define	ONE_POLE_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include "../../constants.h"

typedef struct st_opl_one_pole
{
    float a0, a1, b1;
    float w, norm;
    float input_m1;
    float output;
    float cutoff;
    float sample_rate;
}t_opl_one_pole;

inline void v_opl_set_lowpass(t_opl_one_pole*, float);
inline void v_opl_run(t_opl_one_pole*, float);
t_opl_one_pole * g_opl_get_one_pole(float);

/*TODO: This has some glaring flaws, don't try to run it like this*/
inline void v_opl_set_lowpass(t_opl_one_pole* a_opl, float a_cutoff)
{
    a_opl->w = 2.0f * (a_opl->sample_rate);
    a_opl->cutoff = a_cutoff;

    a_opl->cutoff *= 2.0f * PI;
    a_opl->norm = 1.0 / ((a_opl->cutoff) + (a_opl->w));
    a_opl->b1 = ((a_opl->w) - (a_opl->cutoff)) * (a_opl->norm);
    a_opl->a0 = a_opl->a1 = ((a_opl->cutoff) * (a_opl->norm));
}

/*
void SetHPF(float fCut, float fSampling)
{
    float w = 2.0 * fSampling;
    float Norm;

    fCut *= 2.0F * PI;
    Norm = 1.0 / (fCut + w);
    a0 = w * Norm;
    a1 = -a0;
    b1 = (w - fCut) * Norm;
}
*/

inline void v_opl_run(t_opl_one_pole* a_opl, float a_input)
{
    a_opl->output = (a_input*(a_opl->a0)) + ((a_opl->input_m1) * (a_opl->a1)) + ((a_opl->output) *(a_opl->b1));
    a_opl->input_m1 = a_input;
}

t_opl_one_pole * g_opl_get_one_pole(float a_sr)
{
    t_opl_one_pole * f_result = (t_opl_one_pole*)malloc(sizeof(t_opl_one_pole));
    
    f_result->a0 = 0;
    f_result->a1 = 0;
    f_result->b1 = 0;
    f_result->w = 0;
    f_result->norm = 0;
    f_result->input_m1 = 0;
    f_result->output = 0;
    f_result->cutoff = 0;
    f_result->sample_rate = a_sr;
    
    return f_result;
}

/*Borrowed from:
 * 
 void SetLPF(float fCut, float fSampling)
{
    float w = 2.0 * fSampling;
    float Norm;

    fCut *= 2.0F * PI;
    Norm = 1.0 / (fCut + w);
    b1 = (w - fCut) * Norm;
    a0 = a1 = fCut * Norm;
}

void SetHPF(float fCut, float fSampling)
{
    float w = 2.0 * fSampling;
    float Norm;

    fCut *= 2.0F * PI;
    Norm = 1.0 / (fCut + w);
    a0 = w * Norm;
    a1 = -a0;
    b1 = (w - fCut) * Norm;
}

Where
out[n] = in[n]*a0 + in[n-1]*a1 + out[n-1]*b1;
 */

#ifdef	__cplusplus
}
#endif

#endif	/* ONE_POLE_H */

