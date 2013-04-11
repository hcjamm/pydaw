/* 
 * File:   glitch.h
 * Author: JeffH
 * 
 * Buffer underflow type effect, like having serious XRuns on purpose, with 
 * hints of granular-done-wrong...
 *
 * Created on April 10, 2013, 10:19 PM
 */

#ifndef PYDAW_GLITCH_H
#define	PYDAW_GLITCH_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "../signal_routing/audio_xfade.h"
#include "../../lib/pitch_core.h"
    
typedef struct 
{
    float * buffer;
    int buffer_size, buffer_ptr;
    float last_pitch, last_repeat, last_wet;
    int sample_count, repeat_count, is_repeating, current_repeat_count;
    float sr, sample_tmp;
    float output0, output1;
    t_audio_xfade * xfade;
    t_pit_pitch_core * pitch;
}t_glc_glitch;

t_glc_glitch * g_glc_glitch_get(float);
void v_glc_glitch_set(t_glc_glitch*, float, float, float);
void v_glc_glitch_run(t_glc_glitch*, float, float);

t_glc_glitch * g_glc_glitch_get(float a_sr)
{
    t_glc_glitch * f_result;
    
    if(posix_memalign((void**)&f_result, 16, (sizeof(t_glc_glitch))) != 0)
    {
        return 0;
    }
    
    f_result->buffer_size = (int)(a_sr * (1.0f/19.0f));
    
    if(posix_memalign((void**)&f_result->buffer, 16, (sizeof(float) * f_result->buffer_size)) != 0)
    {
        return 0;
    }
    
    f_result->buffer_ptr = 0;
    f_result->current_repeat_count = 0;
    f_result->is_repeating = 0;
    f_result->last_pitch = 55.5555f;    
    f_result->last_repeat = 99.99f;
    f_result->last_wet = -1.111f;
    f_result->pitch = g_pit_get();
    f_result->repeat_count = 42;    
    f_result->sample_count = 99.99f;
    f_result->sample_tmp = 0.0f;
    f_result->sr = a_sr;
    f_result->xfade = g_axf_get_audio_xfade(-3.0f);
    
    return f_result;
}

void v_glc_glitch_set(t_glc_glitch* a_glc, float a_pitch, float a_repeat, float a_wet)
{
    if(a_glc->last_pitch != a_pitch)
    {
        a_glc->last_pitch = a_pitch;
        a_glc->sample_count = (int)((a_glc->sr)/(f_pit_midi_note_to_hz_fast(a_pitch, a_glc->pitch)));
    }
    
    if(a_glc->last_repeat != a_repeat)
    {
        a_glc->last_repeat = a_repeat;
        a_glc->repeat_count = (int)(a_repeat);
    }
    
    if(a_glc->last_wet != a_wet)
    {
        a_glc->last_wet = a_wet;
        v_axf_set_xfade(a_glc->xfade, a_wet);
    }
}

void v_glc_glitch_run(t_glc_glitch* a_glc, float a_input0, float a_input1)
{
    a_glc->output0 = f_axf_run_xfade(a_glc->xfade, a_input0, a_glc->buffer[a_glc->buffer_ptr]);
    a_glc->output1 = f_axf_run_xfade(a_glc->xfade, a_input1, a_glc->buffer[a_glc->buffer_ptr]);
    
    if(!a_glc->is_repeating)
    {
        a_glc->buffer[a_glc->buffer_ptr] = (a_input0 + a_input1) * 0.5f;
    }
    
    a_glc->buffer_ptr++;
    
    if(a_glc->buffer_ptr >= a_glc->sample_count)
    {
        a_glc->buffer_ptr = 0;
        if(a_glc->is_repeating)
        {
            a_glc->current_repeat_count++;
            
            if(a_glc->current_repeat_count >= a_glc->repeat_count)
            {
                a_glc->current_repeat_count = 0;
                a_glc->is_repeating = 0;
            }
        }
        else
        {
            a_glc->is_repeating = 1;            
        }
    }
}

#ifdef	__cplusplus
}
#endif

#endif	/* PYDAW_GLITCH_H */

