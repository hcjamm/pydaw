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

#ifndef RING_MOD_H
#define	RING_MOD_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "../oscillator/osc_simple.h"
#include "../signal_routing/audio_xfade.h"

typedef struct
{
    float pitch;
    float last_wet;
    float output0, output1;
    t_osc_simple_unison * osc;
    float osc_output;
    t_amp * amp;
    t_audio_xfade * xfade;
}t_rmd_ring_mod;

t_rmd_ring_mod * g_rmd_ring_mod_get(float);
void v_rmd_ring_mod_set(t_rmd_ring_mod*, float, float);
void v_rmd_ring_mod_run(t_rmd_ring_mod*, float, float);
void v_rmd_ring_mod_free(t_rmd_ring_mod*);

void v_rmd_ring_mod_free(t_rmd_ring_mod* a_rmd)
{
    if(a_rmd)
    {
        v_amp_free(a_rmd->amp);
        free(a_rmd->xfade);
        //TODO:  Free the unison osc
        free(a_rmd);
    }
}

t_rmd_ring_mod * g_rmd_ring_mod_get(float a_sr)
{
    t_rmd_ring_mod * f_result;

    if(posix_memalign((void**)&f_result, 16, (sizeof(t_rmd_ring_mod))) != 0)
    {
        return 0;
    }

    f_result->osc = g_osc_get_osc_simple_single(a_sr);
    v_osc_set_simple_osc_unison_type(f_result->osc, 3);
    v_osc_set_uni_voice_count(f_result->osc, 1);
    f_result->output0 = 0.0f;
    f_result->output1 = 0.0f;
    f_result->last_wet = -110.0f;
    f_result->pitch = -99.99f;
    f_result->amp = g_amp_get();
    f_result->osc_output = 0.0f;
    f_result->xfade = g_axf_get_audio_xfade(0.5f);

    return f_result;
}

void v_rmd_ring_mod_set(t_rmd_ring_mod* a_rmd, float a_pitch, float a_wet)
{
    if(a_rmd->last_wet != a_wet)
    {
        a_rmd->last_wet = a_wet;
        v_axf_set_xfade(a_rmd->xfade, a_wet);
    }

    if(a_rmd->pitch != a_pitch)
    {
        a_rmd->pitch = a_pitch;
        v_osc_set_unison_pitch(a_rmd->osc, 0.0f, a_pitch);
    }
}

void v_rmd_ring_mod_run(t_rmd_ring_mod* a_rmd, float a_input0, float a_input1)
{
    a_rmd->osc_output = f_osc_run_unison_osc(a_rmd->osc);

    a_rmd->output0 = f_axf_run_xfade(a_rmd->xfade, a_input0, (a_input0 * (a_rmd->osc_output)));
    a_rmd->output1 = f_axf_run_xfade(a_rmd->xfade, a_input1, (a_input1 * (a_rmd->osc_output)));
}


//f_get_sine



#ifdef	__cplusplus
}
#endif

#endif	/* RING_MOD_H */

