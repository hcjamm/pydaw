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

#ifndef SMOOTHER_IIR_H
#define	SMOOTHER_IIR_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "../lib/denormal.h"

typedef struct s_smoother_iir
{
    float output;
}t_smoother_iir;

inline void v_smr_iir_run(t_smoother_iir*, float);
inline void v_smr_iir_run_fast(t_smoother_iir*, float);

/* inline void v_smr_iir_run(
 * t_smoother_iir *
 * a_smoother, float a_in)  //The input to be smoothed
 *
 * Use t_smoother_iir->output as your new control value after running this
 */
inline void v_smr_iir_run(t_smoother_iir * a_smoother, float a_in)
{
    a_smoother->output =
            f_remove_denormal((a_in * 0.01f) + ((a_smoother->output) * 0.99f));
}

/* inline void v_smr_iir_run_fast(
 * t_smoother_iir *
 * a_smoother, float a_in)  //The input to be smoothed
 *
 * Use t_smoother_iir->output as your new control value after running this
 */
inline void v_smr_iir_run_fast(t_smoother_iir * a_smoother, float a_in)
{
    a_smoother->output =
            f_remove_denormal((a_in * .2f) + ((a_smoother->output) * .8f));
}

t_smoother_iir * g_smr_iir_get_smoother();

t_smoother_iir * g_smr_iir_get_smoother()
{
    t_smoother_iir * f_result;

    if(posix_memalign((void**)&f_result, 16, sizeof(t_smoother_iir)) != 0)
    {
        return 0;
    }

    f_result->output = 0.0f;
    return f_result;
}

#ifdef	__cplusplus
}
#endif

#endif	/* SMOOTHER_IIR_H */

