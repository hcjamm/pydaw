/* 
 * File:   osc_core.h
 * Author: vm-user
 *
 * Created on January 7, 2012, 7:19 PM
 */

#ifndef OSC_CORE_H
#define	OSC_CORE_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "pitch_core.h"
#include <stdlib.h>
   
    
typedef struct _osc_core
{    
    float _output;
    
}osc_core;


float _run_osc(osc_core *, float);
osc_core * _get_osc_core(float);


osc_core * _get_osc_core(float _sr)
{
    osc_core * _result = (osc_core*)malloc(sizeof(osc_core)); 
    _result->_output = 0;    
    return _result;
}

float _run_osc(osc_core *_core, float _inc)
{
    _core->_output += _inc;
    
    if(_core->_output >= 1)
    {
        _core->_output -= 1;
    }
    
    return _core->_output;
}


#ifdef	__cplusplus
}
#endif

#endif	/* OSC_CORE_H */

