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

#ifndef AMP_AND_PANNER_H
#define	AMP_AND_PANNER_H

#include "../../lib/amp.h"
#include "../../lib/lms_math.h"
#include "../../lib/fast_sine.h"
#include "../../lib/lmalloc.h"

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct st_amp_and_panner
{
    float amp_db;
    float amp_linear;
    float pan; //0 to 1
    float amp_linear0;
    float amp_linear1;
    float output0;
    float output1;
    t_amp * amp_ptr;
    t_lin_interpolater * linear_interpolator;
}t_amp_and_panner;

void v_app_set(t_amp_and_panner*,float,float);
void v_app_run(t_amp_and_panner*,float,float);
t_amp_and_panner * g_app_get();

void v_app_set(t_amp_and_panner* a_app,float a_db,float a_pan)
{
    a_app->amp_db = a_db;
    a_app->pan = a_pan;

    a_app->amp_linear = f_db_to_linear_fast(a_db , a_app->amp_ptr);

    a_app->amp_linear0 = (f_sine_fast_run(((a_pan * .5f) + 0.25f),
            a_app->linear_interpolator) * 0.5f + 1.0f) * (a_app->amp_linear);
    a_app->amp_linear1 = (f_sine_fast_run((0.75f - (a_pan * 0.5f)),
            a_app->linear_interpolator) * 0.5f + 1.0f) * (a_app->amp_linear);
}

void v_app_run(t_amp_and_panner* a_app, float a_in0, float a_in1)
{
    a_app->output0 = a_in0 * (a_app->amp_linear0);
    a_app->output1 = a_in1 * (a_app->amp_linear1);
}

void v_app_run_monofier(t_amp_and_panner* a_app, float a_in0, float a_in1)
{
    v_app_run(a_app, a_in0, a_in1);
    a_app->output0 = a_app->output0 + a_app->output1;
    a_app->output1 = a_app->output0;
}

t_amp_and_panner * g_app_get()
{
    t_amp_and_panner * f_result;

    lmalloc((void**)&f_result, sizeof(t_amp_and_panner));

    f_result->amp_db = 0.0f;
    f_result->pan = 0.0f;
    f_result->amp_linear0 = 1.0f;
    f_result->amp_linear1 = 1.0f;
    f_result->output0 = 0.0f;
    f_result->output1 = 0.0f;
    f_result->amp_ptr = g_amp_get();
    f_result->linear_interpolator = g_lin_get();

    return f_result;
}

void v_app_free(t_amp_and_panner * a_app)
{
    v_amp_free(a_app->amp_ptr);
    free(a_app->linear_interpolator);
    free(a_app);
}

#ifdef	__cplusplus
}
#endif

#endif	/* AMP_AND_PANNER_H */

