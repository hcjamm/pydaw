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

#ifndef PEAK_METER_H
#define	PEAK_METER_H

#define PEAK_STEP_SIZE 4

#include "lmalloc.h"

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct
{
    float value[2];
    int buffer_pos;
}t_pkm_peak_meter;


#ifdef	__cplusplus
}
#endif

t_pkm_peak_meter * g_pkm_get()
{
    t_pkm_peak_meter * f_result;

    lmalloc((void**)(&f_result), sizeof(t_pkm_peak_meter));

    f_result->value[0] = 0.0f;
    f_result->value[1] = 0.0f;
    f_result->buffer_pos = 0;

    return f_result;
}

static inline float f_pkm_compare(float a_audio, float a_peak)
{
    float f_value = a_audio;

    if(a_audio < 0.0f)
    {
        f_value = a_audio * -1.0f;
    }

    if(f_value > a_peak)
    {
        return f_value;
    }
    else
    {
        return a_peak;
    }
}

/* For the host to call after reading the peak value
 */
void v_pkm_reset(t_pkm_peak_meter * self)
{
    self->value[0] = 0.0f;
    self->value[1] = 0.0f;
}

void v_pkm_run(t_pkm_peak_meter * self,
        float * a_in0, float * a_in1, int a_count)
{
    self->buffer_pos = 0;
    while(self->buffer_pos < a_count)
    {
        self->value[0] = f_pkm_compare(a_in0[self->buffer_pos], self->value[0]);
        self->value[1] = f_pkm_compare(a_in1[self->buffer_pos], self->value[1]);
        self->buffer_pos += PEAK_STEP_SIZE;
    }
}

#endif	/* PEAK_METER_H */

