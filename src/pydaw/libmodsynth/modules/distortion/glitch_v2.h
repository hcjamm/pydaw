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

#ifndef PYDAW_GLITCH_V2_H
#define	PYDAW_GLITCH_V2_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "../signal_routing/audio_xfade.h"

typedef struct
{
    float * buffer;
    int buffer_size, read_head, write_head;
    float last_wet, last_time;
    int sample_count;
    float sr;
    float output0, output1;
    t_audio_xfade * xfade;
}t_glc_glitch_v2;

t_glc_glitch_v2 * g_glc_glitch_v2_get(float);
void v_glc_glitch_v2_set(t_glc_glitch_v2*, float, float);
void v_glc_glitch_v2_run(t_glc_glitch_v2*, float, float);
void v_glc_glitch_v2_free(t_glc_glitch_v2*);

void v_glc_glitch_v2_free(t_glc_glitch_v2 * a_glc)
{
    if(a_glc)
    {
        free(a_glc->buffer);
        free(a_glc);
    }
}

t_glc_glitch_v2 * g_glc_glitch_v2_get(float a_sr)
{
    t_glc_glitch_v2 * f_result;

    if(posix_memalign((void**)&f_result, 16, (sizeof(t_glc_glitch_v2))) != 0)
    {
        return 0;
    }

    f_result->buffer_size = (int)(a_sr * 0.27f);

    if(posix_memalign((void**)&f_result->buffer, 16, (sizeof(float) *
            f_result->buffer_size)) != 0)
    {
        return 0;
    }

    int f_i = 0;

    while(f_i < f_result->buffer_size)
    {
        f_result->buffer[f_i] = 0.0f;
        f_i++;
    }

    f_result->read_head = 0;
    f_result->write_head = 0;
    f_result->last_time = 654654.89f;
    f_result->last_wet = -1.111f;
    f_result->sample_count = 99.99f;
    f_result->sr = a_sr;
    f_result->xfade = g_axf_get_audio_xfade(-3.0f);

    return f_result;
}

void v_glc_glitch_v2_set(t_glc_glitch_v2* a_glc, float a_time, float a_wet)
{
    if(a_glc->last_time != a_time)
    {
        a_glc->last_time = a_time;
        a_glc->sample_count = (int)((a_glc->sr) * a_time);
    }

    if(a_glc->last_wet != a_wet)
    {
        a_glc->last_wet = a_wet;
        v_axf_set_xfade(a_glc->xfade, a_wet);
    }
}

inline void v_glc_glitch_v2_retrigger(t_glc_glitch_v2* a_glc)
{
    a_glc->read_head = 0;
    a_glc->write_head = 0;
}

void v_glc_glitch_v2_run(t_glc_glitch_v2* a_glc, float a_input0, float a_input1)
{
    if(a_glc->write_head < a_glc->buffer_size)
    {
        a_glc->buffer[a_glc->write_head] = (a_input0 + a_input1) * 0.5f;
        a_glc->write_head++;
    }

    a_glc->output0 = f_axf_run_xfade(a_glc->xfade, a_input0,
            a_glc->buffer[a_glc->read_head]);
    a_glc->output1 = f_axf_run_xfade(a_glc->xfade, a_input1,
            a_glc->buffer[a_glc->read_head]);

    a_glc->read_head++;

    if(a_glc->read_head >= a_glc->sample_count)
    {
        a_glc->read_head = 0;
    }
}

#ifdef	__cplusplus
}
#endif

#endif	/* PYDAW_GLITCH_V2_H */

