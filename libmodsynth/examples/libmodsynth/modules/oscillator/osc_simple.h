/* 
 * File:   osc_simple.h
 * Author: vm-user
 *
 * Created on January 7, 2012, 8:52 PM
 */

#ifndef OSC_SIMPLE_H
#define	OSC_SIMPLE_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "../../lib/osc_core.h"
#include "../../../constants.h"
#include <math.h>

//Used to switch between values, uses much less CPU than a switch statement
typedef float (*_get_osc_func_ptr)(osc_core*);

float _get_saw(osc_core * _core)
{
    return (((_core->_output) * 2) - 1);
}

float _get_sine(osc_core * _core)
{
    return sin((_core->_output) * PI2);
}

float _get_square(osc_core * _core)
{
    if((_core->_output) >= .5)
    {
        return 1;
    }
    else
    {
        return -1;
    }
}

float _get_triangle(osc_core * _core)
{
    float _ramp = ((_core->_output) * 4) - 2;
    if(_ramp > 1)
    {
        return 2 - _ramp;
    }
    else if(_ramp < -1)
    {
        return (2 + _ramp) * -1;
    }
    else
    {
        return _ramp;
    }
}

#ifdef	__cplusplus
}
#endif

#endif	/* OSC_SIMPLE_H */

