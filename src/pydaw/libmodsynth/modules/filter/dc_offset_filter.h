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

#ifndef DC_OFFSET_FILTER_H
#define	DC_OFFSET_FILTER_H

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct st_dco_dc_offset_filter
{
    float in_n_m1, out_n_m1, coeff, output;
}t_dco_dc_offset_filter;

t_dco_dc_offset_filter * g_dco_get(float);
inline float f_dco_run(t_dco_dc_offset_filter*,float);
inline void v_dco_reset(t_dco_dc_offset_filter*);

inline float f_dco_run(t_dco_dc_offset_filter*__restrict a_dco,float a_in)
{
    a_dco->output = (a_in - (a_dco->in_n_m1)) + ((a_dco->out_n_m1) * (a_dco->coeff));
    
    a_dco->in_n_m1 = a_in;
    a_dco->out_n_m1 = (a_dco->output);
    
    return (a_dco->output);
}

inline void v_dco_reset(t_dco_dc_offset_filter*__restrict a_dco)
{
    a_dco->in_n_m1 = 0.0f;
    a_dco->out_n_m1 = 0.0f;
    a_dco->output = 0.0f;
}

t_dco_dc_offset_filter * g_dco_get(float a_sr)
{
    t_dco_dc_offset_filter * f_result = (t_dco_dc_offset_filter*)malloc(sizeof(t_dco_dc_offset_filter));
    
    f_result->coeff = (1.0f - (6.6f/a_sr));
    v_dco_reset(f_result);
    
    return f_result;
}

#ifdef	__cplusplus
}
#endif

#endif	/* DC_OFFSET_FILTER_H */

