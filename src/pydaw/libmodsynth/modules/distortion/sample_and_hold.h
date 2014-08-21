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
#ifndef SAMPLE_AND_HOLD_H
#define	SAMPLE_AND_HOLD_H

#include "../../lib/pitch_core.h"
#include "../../lib/lmalloc.h"
#include "../signal_routing/audio_xfade.h"

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct
{
    float output0, output1, hold0, hold1;
    int hold_count, hold_counter;
    float last_pitch, last_wet, sr;
    t_pit_pitch_core * pitch;
    t_audio_xfade * xfade;
} t_sah_sample_and_hold;

t_sah_sample_and_hold * g_sah_sample_and_hold_get(float);
void v_sah_sample_and_hold_set(t_sah_sample_and_hold*, float, float);
void v_sah_sample_and_hold_run(t_sah_sample_and_hold*, float, float);
void v_sah_free(t_sah_sample_and_hold*);

void v_sah_free(t_sah_sample_and_hold * a_sah)
{
    v_pit_free(a_sah->pitch);
    free(a_sah->xfade);
    free(a_sah);
}

t_sah_sample_and_hold * g_sah_sample_and_hold_get(float a_sr)
{
    t_sah_sample_and_hold * f_result;

    lmalloc((void**)&f_result, sizeof(t_sah_sample_and_hold));

    f_result->hold_count = 1;
    f_result->hold_counter = 0;
    f_result->output0 = 0.0f;
    f_result->output1 = 0.0f;
    f_result->hold0 = 0.0f;
    f_result->hold1 = 0.0f;
    f_result->last_pitch = -99.999f;
    f_result->sr = a_sr;
    f_result->pitch = g_pit_get();
    f_result->last_wet = -99.00088f;
    f_result->xfade = g_axf_get_audio_xfade(-3.0f);

    return f_result;
}

void v_sah_sample_and_hold_set(t_sah_sample_and_hold* a_sah, float a_pitch,
        float a_wet)
{
    if(a_sah->last_pitch != a_pitch)
    {
        a_sah->last_pitch = a_pitch;
        a_sah->hold_count = (int)(a_sah->sr /
                f_pit_midi_note_to_hz_fast(a_pitch, a_sah->pitch));
    }

    if(a_sah->last_wet != a_wet)
    {
        a_sah->last_wet = a_wet;
        v_axf_set_xfade(a_sah->xfade, a_wet);
    }
}

void v_sah_sample_and_hold_run(t_sah_sample_and_hold* a_sah,
        float a_in0, float a_in1)
{
    a_sah->output0 = f_axf_run_xfade(a_sah->xfade, a_in0, a_sah->hold0);
    a_sah->output1 = f_axf_run_xfade(a_sah->xfade, a_in1, a_sah->hold1);

    a_sah->hold_counter++;

    if(a_sah->hold_counter >= a_sah->hold_count)
    {
        a_sah->hold_counter = 0;
        a_sah->hold0 = a_in0;
        a_sah->hold1 = a_in1;
    }
}

#ifdef	__cplusplus
}
#endif

#endif	/* SAMPLE_AND_HOLD_H */

