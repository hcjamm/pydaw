/* 
 * File:   audio_xfade.h
 * Author: vm-user
 * Description:  Performs a smooth crossfade suitable for a 100% Dry to 100% Wet knob
 * 
 * This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

 * Created on January 17, 2012, 8:59 PM
 */

#ifndef AUDIO_XFADE_H
#define	AUDIO_XFADE_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "../../lib/amp.h"
    
typedef struct _audio_xfade
{
    float _mid_point;
    float _mid_point_50_minus;
    float _in1_mult;
    float _in2_mult;
}audio_xfade;

void _axf_set_xfade(audio_xfade *, float);

void _axf_set_xfade(audio_xfade * _axf_ptr, float _point)
{
    //TODO:  go back and fix this
    
    _axf_ptr->_in1_mult = 1- _point;
    _axf_ptr->_in2_mult = _point;
    
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

float _axf_run_xfade(audio_xfade *, float, float);

audio_xfade * _axf_get_audio_xfade(float __mid_point)
{
    audio_xfade * _result = (audio_xfade*)malloc(sizeof(audio_xfade));
    _result->_mid_point = __mid_point;
    _result->_mid_point_50_minus = 50 - _result->_mid_point;
    
    return _result;
}

float _axf_run_xfade(audio_xfade * _axf_ptr, float _in1, float _in2)
{
    float _result = ((_axf_ptr->_in1_mult) * _in1) + ((_axf_ptr->_in2_mult) * _in2);
    return _result;
}

#ifdef	__cplusplus
}
#endif

#endif	/* AUDIO_XFADE_H */

