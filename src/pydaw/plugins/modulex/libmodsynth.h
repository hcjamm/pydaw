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

#ifndef MODULEX_LIBMODSYNTH_H
#define	MODULEX_LIBMODSYNTH_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "../../libmodsynth/constants.h"
#include "../../libmodsynth/modules/multifx/multifx3knob.h"
#include "../../libmodsynth/lib/osc_core.h"
#include "../../libmodsynth/lib/amp.h"
#include "../../libmodsynth/lib/pitch_core.h"
#include "../../libmodsynth/lib/spectrum_analyzer.h"
#include "../../libmodsynth/lib/smoother-linear.h"
#include "../../libmodsynth/modules/delay/lms_delay.h"
#include "../../libmodsynth/modules/filter/svf.h"
#include "../../libmodsynth/modules/modulation/env_follower.h"
#include "../../libmodsynth/modules/delay/reverb.h"
#include "../../libmodsynth/modules/filter/peak_eq.h"
#include "../../libmodsynth/modules/modulation/gate.h"
#include "../../libmodsynth/modules/distortion/glitch_v2.h"

#define MODULEX_EQ_COUNT 6

typedef struct
{
    t_mf3_multi * multieffect[8];
    fp_mf3_run fx_func_ptr[8];

    t_lms_delay * delay;
    t_smoother_linear * time_smoother;
    t_enf_env_follower * env_follower;

    t_rvb_reverb * reverb;

    float current_sample0;
    float current_sample1;

    float vol_linear;

    t_smoother_linear * volume_smoother;
    t_smoother_linear * reverb_smoother;
    t_smoother_linear * gate_wet_smoother;
    t_smoother_linear * glitch_time_smoother;

    t_smoother_linear * smoothers[8][3];

    t_amp * amp_ptr;
    t_pkq_peak_eq * eqs[MODULEX_EQ_COUNT];
    t_spa_spectrum_analyzer * spectrum_analyzer;
    t_gat_gate * gate;
    float gate_on;

    t_glc_glitch_v2 * glitch;
    float glitch_on;
}t_modulex_mono_modules;

t_modulex_mono_modules * v_modulex_mono_init(float, int);

t_modulex_mono_modules * v_modulex_mono_init(float a_sr, int a_track_num)
{
    t_modulex_mono_modules * a_mono =
            (t_modulex_mono_modules*)malloc(sizeof(t_modulex_mono_modules));

    int f_i = 0;

    while(f_i < 8)
    {
        a_mono->multieffect[f_i] = g_mf3_get(a_sr);
        a_mono->fx_func_ptr[f_i] = v_mf3_run_off;
        int f_i2 = 0;
        while(f_i2 < 3)
        {
            a_mono->smoothers[f_i][f_i2] =
                    g_sml_get_smoother_linear(a_sr, 127.0f, 0.0f, 0.1f);
            f_i2++;
        }
        f_i++;
    }

    f_i = 0;

    while(f_i < MODULEX_EQ_COUNT)
    {
        a_mono->eqs[f_i] = g_pkq_get(a_sr);
        f_i++;
    }

    a_mono->amp_ptr = g_amp_get();

    a_mono->delay = g_ldl_get_delay(1, a_sr);
    a_mono->time_smoother =
            g_sml_get_smoother_linear(a_sr, 100.0f, 10.0f, 0.1f);
    a_mono->env_follower = g_enf_get_env_follower(a_sr);

    a_mono->reverb = g_rvb_reverb_get(a_sr);

    a_mono->volume_smoother =
            g_sml_get_smoother_linear(a_sr, 0.0f, -50.0f, 0.1f);
    a_mono->volume_smoother->last_value = 0.0f;
    a_mono->reverb_smoother =
            g_sml_get_smoother_linear(a_sr, 100.0f, 0.0f, 0.001f);
    a_mono->reverb_smoother->last_value = 0.0f;

    a_mono->gate_wet_smoother =
            g_sml_get_smoother_linear(a_sr, 100.0f, 0.0f, 0.01f);
    a_mono->gate_wet_smoother->last_value = 0.0f;

    a_mono->vol_linear = 1.0f;

    a_mono->spectrum_analyzer =
            g_spa_spectrum_analyzer_get(4096, a_track_num, 0);

    a_mono->gate = g_gat_get(a_sr);
    a_mono->gate_on = 0.0f;

    a_mono->glitch = g_glc_glitch_v2_get(a_sr);
    a_mono->glitch_on = 0.0f;
    a_mono->glitch_time_smoother =
            g_sml_get_smoother_linear(a_sr, 0.10f, 0.01f, 0.01f);

    return a_mono;
}



#ifdef	__cplusplus
}
#endif

#endif	/* LIBMODSYNTH_H */

