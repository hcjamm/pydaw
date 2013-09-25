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

#ifndef AUDIO_XFADE_H
#define	AUDIO_XFADE_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "../../lib/amp.h"
    
//#define AXF_DEBUG_MODE
    
typedef struct st_audio_xfade
{
    float mid_point;
    float mid_point_50_minus;
    float in1_mult;
    float in2_mult;
#ifdef AXF_DEBUG_MODE
    int debug_counter;
#endif
}t_audio_xfade;

void v_axf_set_xfade(t_audio_xfade *, float);

/*void v_axf_set_xfade(
 * t_audio_xfade *,
 * float a_point // 0 to 1
 * )
 */
void v_axf_set_xfade(t_audio_xfade * a_axf_ptr, float a_point)
{
    //TODO:  go back and fix this
    
    a_axf_ptr->in1_mult = 1.0f - a_point;
    a_axf_ptr->in2_mult = a_point;
    
    /*
    if(_point < .5)
    {
        _axf_ptr->_in1_mult = _db_to_linear(_point * (_axf_ptr->_mid_point) * 2 );
        _axf_ptr->_in2_mult = _db_to_linear(_point * (_axf_ptr->_mid_point_50_minus));
    }
    else
    {
        _axf_ptr->_in1_mult = _db_to_linear((1 - _point) * (_axf_ptr->_mid_point_50_minus));
        _axf_ptr->_in2_mult = _db_to_linear((1 - _point) * (_axf_ptr->_mid_point) * 2 );        
    }
     */
}

float f_axf_run_xfade(t_audio_xfade *, float, float);

/*t_audio_xfade * g_axf_get_audio_xfade
 * (
 * float a_mid_point //This is the negative gain at 0.5 for both channels.  -3 or -6 is a good value
 * )
 */
t_audio_xfade * g_axf_get_audio_xfade(float a_mid_point)
{
    t_audio_xfade * f_result = (t_audio_xfade*)malloc(sizeof(t_audio_xfade));
    f_result->mid_point = a_mid_point;
    f_result->mid_point_50_minus = 50.0f - f_result->mid_point;
    f_result->in1_mult = .5f;
    f_result->in2_mult = .5f;
        
    return f_result;
}

/* float f_axf_run_xfade(t_audio_xfade * a_axf_ptr, float a_in1, float a_in2)
 */
float f_axf_run_xfade(t_audio_xfade * a_axf_ptr, float a_in1, float a_in2)
{
    float f_result = ((a_axf_ptr->in1_mult) * a_in1) + ((a_axf_ptr->in2_mult) * a_in2);
#ifdef AXF_DEBUG_MODE
    a_axf_ptr->debug_counter = (a_axf_ptr->debug_counter) + 1;
    
    if((a_axf_ptr->debug_counter) >= 100000)
    {
        a_axf_ptr->debug_counter = 0;
        printf("Audio Xfade debug info\n");
        printf("a_axf_ptr->in1_mult == %f\n", (a_axf_ptr->in1_mult));
        printf("a_axf_ptr->in1_mult == %f\n", (a_axf_ptr->in2_mult));
        printf("a_axf_ptr->mid_point == %f\n", (a_axf_ptr->mid_point));
        printf("a_axf_ptr->mid_point_50_minus == %f\n", (a_axf_ptr->mid_point_50_minus));
    }
#endif
    return f_result;
}

#ifdef	__cplusplus
}
#endif

#endif	/* AUDIO_XFADE_H */
