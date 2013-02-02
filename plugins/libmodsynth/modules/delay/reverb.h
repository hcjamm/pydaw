/* 
 * File:   reverb.h
 * Author: JeffH
 * 
 * A simple digital reverb built with off-the-shelf parts from the rest of the library.
 * The focus is on quality, CPU efficiency, and a minimal number of controls(targeting only time, wet and color knobs)
 * 
 * A memory and CPU optimized version will come after the overall sound is good...
 *
 * Created on January 10, 2013, 8:29 AM
 */

#ifndef PYDAW_REVERB_H
#define	PYDAW_REVERB_H

#define PYDAW_REVERB_CHANNELS 2
#define PYDAW_REVERB_DIFFUSER_COUNT 4
#define PYDAW_REVERB_TAP_COUNT 8

#ifdef	__cplusplus
extern "C" {
#endif

#include "../../lib/amp.h"
#include "../../lib/pitch_core.h"
#include "../filter/comb_filter.h"
#include "../filter/svf.h"
    
typedef struct 
{
    int iter1;
    float color;
    float wet;
    float wet_linear;
    float time;
    float output[PYDAW_REVERB_CHANNELS];    
    t_state_variable_filter * diffusers[PYDAW_REVERB_CHANNELS][PYDAW_REVERB_DIFFUSER_COUNT];
    t_state_variable_filter * hp[PYDAW_REVERB_CHANNELS];
    t_state_variable_filter * lp[PYDAW_REVERB_CHANNELS][PYDAW_REVERB_TAP_COUNT];
    t_comb_filter * taps[PYDAW_REVERB_CHANNELS][PYDAW_REVERB_TAP_COUNT];
    t_pit_pitch_core * pitch_core;
    t_amp * amp;
    float comb_tunings[PYDAW_REVERB_TAP_COUNT];
    float allpass_tunings[PYDAW_REVERB_DIFFUSER_COUNT];
} t_rvb_reverb;

t_rvb_reverb * g_rvb_reverb_get(float);
void v_rvb_reverb_set(t_rvb_reverb *, float, float, float);
inline void v_rvb_reverb_run(t_rvb_reverb *, float, float);

void v_rvb_reverb_set(t_rvb_reverb * a_reverb, float a_time, float a_wet, float a_color)
{
    if(a_time != (a_reverb->time))
    {
        a_reverb->time = a_time;        
    }
    
    if(a_wet != (a_reverb->wet))
    {
        a_reverb->wet = a_wet;
        a_reverb->wet_linear = f_db_to_linear_fast(a_wet, a_reverb->amp);
    }
    
    if(a_color != (a_reverb->color))
    {
        
    }
}

inline void v_rvb_reverb_run(t_rvb_reverb * a_reverb, float a_input0, float a_input1)
{
    a_reverb->iter1 = 0;
    
    a_reverb->output[0] = 0.0f;
    a_reverb->output[1] = 0.0f;
    
    while((a_reverb->iter1) < PYDAW_REVERB_TAP_COUNT)
    {
        v_cmb_set_input(a_reverb->taps[0][(a_reverb->iter1)], a_input0);
        v_cmb_set_input(a_reverb->taps[1][(a_reverb->iter1)], a_input1);
    
        a_reverb->output[0] += (a_reverb->taps[0][(a_reverb->iter1)]->output_sample);
        a_reverb->output[1] += (a_reverb->taps[1][(a_reverb->iter1)]->output_sample);
        
        a_reverb->iter1 = (a_reverb->iter1) + 1;
    }
    
    a_reverb->iter1 = 0;
    
    while((a_reverb->iter1) < PYDAW_REVERB_DIFFUSER_COUNT)
    {
        a_reverb->output[0] = v_svf_run_2_pole_allpass(a_reverb->diffusers[0][(a_reverb->iter1)], a_reverb->output[0]);
        a_reverb->output[1] = v_svf_run_2_pole_allpass(a_reverb->diffusers[1][(a_reverb->iter1)], a_reverb->output[1]);
        
        a_reverb->iter1 = (a_reverb->iter1) + 1;
    }        
}

t_rvb_reverb * g_rvb_reverb_get(float a_sr)
{
    t_rvb_reverb * f_result;
    
    if(posix_memalign((void**)&f_result, 16, (sizeof(t_rvb_reverb))) != 0)
    {
        return 0;
    }
    
    int f_i = 0;
        
    f_result->color = 0.0f;
    f_result->iter1 = 0;
    f_result->time = 1.0f;
    f_result->wet = -40.0f;
    f_result->wet_linear = 0.0f;
    
    f_result->amp = g_amp_get();
    f_result->pitch_core = g_pit_get();
    
    f_result->comb_tunings[0] = f_pit_hz_to_midi_note(1557.0f);
    f_result->comb_tunings[1] = f_pit_hz_to_midi_note(1617.0f);
    f_result->comb_tunings[2] = f_pit_hz_to_midi_note(1491.0f);
    f_result->comb_tunings[3] = f_pit_hz_to_midi_note(1422.0f);
    f_result->comb_tunings[4] = f_pit_hz_to_midi_note(1277.0f);
    f_result->comb_tunings[5] = f_pit_hz_to_midi_note(1356.0f);
    f_result->comb_tunings[6] = f_pit_hz_to_midi_note(1188.0f);
    f_result->comb_tunings[7] = f_pit_hz_to_midi_note(1116.0f);
        
    f_result->allpass_tunings[0] = f_pit_hz_to_midi_note(225.0f);
    f_result->allpass_tunings[1] = f_pit_hz_to_midi_note(556.0f);
    f_result->allpass_tunings[2] = f_pit_hz_to_midi_note(441.0f);
    f_result->allpass_tunings[3] = f_pit_hz_to_midi_note(341.0f);
    
    while(f_i < PYDAW_REVERB_CHANNELS)
    {
        f_result->output[f_i] = 0.0f;
        
        f_result->hp[f_i] = g_svf_get(a_sr);
                
        int f_i2 = 0;
        
        while(f_i2 < PYDAW_REVERB_TAP_COUNT)
        {
            f_result->taps[f_i][f_i2] = g_cmb_get_comb_filter(a_sr);
            f_result->lp[f_i][f_i2] = g_svf_get(a_sr);
            v_svf_set_cutoff_base(f_result->lp[f_i][f_i2], 10000.0f);
            v_svf_set_cutoff(f_result->lp[f_i][f_i2]);
            
            v_cmb_set_all(f_result->taps[f_i][f_i2], 0.0f, -3.0f, f_result->comb_tunings[f_i2]);
            f_i2++;
        }
        
        f_i2 = 0;
        
        while(f_i2 < PYDAW_REVERB_DIFFUSER_COUNT)
        {
            f_result->diffusers[f_i][f_i2] = g_svf_get(a_sr);
            v_svf_set_cutoff_base(f_result->diffusers[f_i][f_i2], f_result->allpass_tunings[f_i2]);
            v_svf_set_cutoff(f_result->diffusers[f_i][f_i2]);
            f_i2++;
        }
        
        f_i++;
    }
    
    return f_result;    
}

#ifdef	__cplusplus
}
#endif

#endif	/* PYDAW_REVERB_H */

