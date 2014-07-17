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

#ifndef PYDAW_GATE_H
#define	PYDAW_GATE_H

#include "../filter/svf.h"
#include "../signal_routing/audio_xfade.h"

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct
{
    t_state_variable_filter * svf;
    t_audio_xfade * xfade;
    float last_cutoff;
    float last_wet;
    float output[2];
    float value;
}t_gat_gate;

t_gat_gate * g_gat_get(float);
void v_gat_set(t_gat_gate*, float);
void v_gat_run(t_gat_gate*, float, float, float);

/*
 *  void v_gat_set(t_gat_gate* a_gat, float a_pitch)
 */
void v_gat_set(t_gat_gate* a_gat, float a_pitch, float a_wet)
{
    if(a_pitch != a_gat->last_cutoff)
    {
        a_gat->last_cutoff = a_pitch;
        v_svf_set_cutoff_base(a_gat->svf, 66.0f);
        v_svf_set_cutoff(a_gat->svf);
    }

    if(a_wet != a_gat->last_wet)
    {
        a_gat->last_wet = a_wet;
        v_axf_set_xfade(a_gat->xfade, a_wet);
    }
}

/*
 * void v_gat_run(t_gat_gate * a_gat, float a_on, float a_in0, float a_in1)
 */
void v_gat_run(t_gat_gate * a_gat, float a_on, float a_in0, float a_in1)
{
    a_gat->value = v_svf_run_2_pole_lp(a_gat->svf, a_on);

    a_gat->output[0] = f_axf_run_xfade(a_in0, a_gat->value * a_in0);
    a_gat->output[1] = f_axf_run_xfade(a_in0, a_gat->value * a_in1);
}

/*
 * t_gat_gate * g_gat_get(float a_sr)
 */
t_gat_gate * g_gat_get(float a_sr)
{
    t_gat_gate * f_result = (t_gat_gate*)malloc(sizeof(t_gat_gate));

    f_result->value = 0.0f;
    f_result->output[0] = 0.0f;
    f_result->output[1] = 0.0f;
    f_result->last_cutoff = 66.0f;
    f_result->last_wet = 0.0f;
    f_result->svf = g_svf_get(a_sr);

    f_result->xfade = g_axf_get_audio_xfade(-3.0f);

    v_svf_set_cutoff_base(f_result->svf, 66.0f);
    v_svf_set_res(f_result->svf, -12.0f);
    v_svf_set_cutoff(f_result->svf);

    return f_result;
}

#ifdef	__cplusplus
}
#endif

#endif	/* PYDAW_GATE_H */

