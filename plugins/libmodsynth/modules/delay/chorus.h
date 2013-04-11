/* 
 * File:   chorus.h
 * Author: JeffH
 * 
 * My attempt to recreate a chorus plugin I wrote 10 years ago, simple
 * and very stereo-ized...
 *
 * Created on April 10, 2013, 8:26 AM
 */

#ifndef CHORUS_H
#define	CHORUS_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "../../lib/interpolate-cubic.h"
#include "../../lib/amp.h"
#include "../oscillator/lfo_simple.h"

typedef struct 
{
    float * buffer;    
    float wet_lin, wet_db, freq_last, mod_amt;
    float output0, output1;
    float delay_offset_amt, delay_offset;
    float pos_left, pos_right;
    int buffer_size, buffer_ptr;
    float buffer_size_float;
    t_lfs_lfo * lfo;
    t_cubic_interpolater * cubic;
    t_amp * amp;
}t_crs_chorus;

t_crs_chorus * g_crs_chorus_get(float);
void v_crs_chorus_set(t_crs_chorus*, float, float);
void v_crs_chorus_run(t_crs_chorus*, float, float);

t_crs_chorus* g_crs_chorus_get(float a_sr)
{
    t_crs_chorus * f_result;
    
    if(posix_memalign((void**)&f_result, 16, (sizeof(t_crs_chorus))) != 0)
    {
        return 0;
    }
    
    f_result->buffer_size = (int)(a_sr * 0.050f);
    f_result->buffer_size_float = ((float)(f_result->buffer_size));
    
    if(posix_memalign((void**)&f_result->buffer, 16, (sizeof(float) * f_result->buffer_size)) != 0)
    {
        return 0;
    } 
    
    int f_i = 0;
    while(f_i < f_result->buffer_size)
    {
        f_result->buffer[f_i] = 0.0f;
        f_i++;
    }
    
    f_result->pos_left = 0.0f;
    f_result->pos_right = 0.0f;
    f_result->buffer_ptr = 0;
    f_result->delay_offset_amt =  a_sr * 0.03f;
    f_result->delay_offset = 0.0f;
    f_result->cubic = g_cubic_get();
    f_result->lfo = g_lfs_get(a_sr);
    f_result->wet_lin = 0.0f;
    f_result->wet_db = -99.99f;
    f_result->amp = g_amp_get();
    f_result->mod_amt = a_sr * 0.01f;
    f_result->freq_last = -99.99f;
    f_result->output0 = 0.0f;
    f_result->output1 = 0.0f;
    v_lfs_sync(f_result->lfo, 0.0f, 1);
    
    return f_result;
}

void v_crs_chorus_set(t_crs_chorus* a_crs, float a_freq, float a_wet)
{
    if(a_wet != (a_crs->wet_db))
    {
        a_crs->wet_db = a_wet;
        a_crs->wet_lin = f_db_to_linear_fast(a_wet, a_crs->amp);
    }
    
    if(a_freq != (a_crs->freq_last))
    {
        a_crs->freq_last = a_freq;
        v_lfs_set(a_crs->lfo, a_freq);
    }
}

void v_crs_chorus_run(t_crs_chorus* a_crs, float a_input0, float a_input1)
{
    a_crs->buffer[(a_crs->buffer_ptr)] = (a_input0 + a_input1) * 0.5f;
    
    a_crs->delay_offset = ((float)(a_crs->buffer_ptr)) - (a_crs->delay_offset_amt);
            
    v_lfs_run(a_crs->lfo);
    
    a_crs->pos_left = ((a_crs->delay_offset) + ((a_crs->lfo->output) * (a_crs->mod_amt)));
    
    if((a_crs->pos_left) >= (a_crs->buffer_size_float))
    {
        a_crs->pos_left -= (a_crs->buffer_size_float);
    }
    else if((a_crs->pos_left) < 0.0f)
    {
        a_crs->pos_left += (a_crs->buffer_size_float);
    }
    
    a_crs->pos_right = ((a_crs->delay_offset) + ((a_crs->lfo->output) * (a_crs->mod_amt) * -1.0f));
    
    if((a_crs->pos_right) >= (a_crs->buffer_size_float))
    {
        a_crs->pos_right -= (a_crs->buffer_size_float);
    }
    else if((a_crs->pos_right) < 0.0f)
    {
        a_crs->pos_right += (a_crs->buffer_size_float);
    }
    
    a_crs->output0 = a_input0 + (f_cubic_interpolate_ptr_wrap(a_crs->buffer, (a_crs->buffer_size), 
            (a_crs->pos_left), a_crs->cubic) * (a_crs->wet_lin));
    a_crs->output1 = a_input1 +  (f_cubic_interpolate_ptr_wrap(a_crs->buffer, (a_crs->buffer_size), 
            (a_crs->pos_right), a_crs->cubic) * (a_crs->wet_lin));
    
    a_crs->buffer_ptr++;
    if((a_crs->buffer_ptr) >= (a_crs->buffer_size))
    {
        a_crs->buffer_ptr = 0;
    }
}

#ifdef	__cplusplus
}
#endif

#endif	/* CHORUS_H */

