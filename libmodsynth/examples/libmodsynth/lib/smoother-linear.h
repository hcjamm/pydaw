/* 
 * File:   smoother-linear.h
 * Author: vm-user
 *
 * Created on January 9, 2012, 8:02 PM
 */

#ifndef SMOOTHER_LINEAR_H
#define	SMOOTHER_LINEAR_H

#include "math.h"

typedef struct _smoother_linear
{
    float rate, last_value;
}smoother_linear;

/*There's not much good reason to change this while the synth is running, so all properties are set on instantiation*/
smoother_linear * _sml_get_smoother_linear(float _sample_rate, float _high, float _low, float _time_in_seconds)
{
    smoother_linear * _result = (smoother_linear*)malloc(sizeof(smoother_linear));
    /*Start in the middle, the user can manually set the value if this isn't acceptable*/
    _result->last_value = (((_high - _low) * .5) + _low);
    
    /*Rate is the time it would take to complete if the knob was all the way counter-clockwise, and then instantly moved all the way clockwise*/
    _result->rate = (((_high - _low ) * _time_in_seconds) / _sample_rate);
    
    return _result;
}

float _sml_run(smoother_linear * _smoother, float _current_value)
{
    /*TODO:  evaluate this more thoroughly.  I'm trying to make it as CPU efficient as possible, since almost every port and/or GUI control will have one.*/
    
    /*Evaluated first because most controls won't be moving most of the time, should consume the fewest cycles*/
    if(((_smoother->last_value) == _current_value))
    {        
        return (_smoother->last_value);
    } 
    /*This does waste CPU while knobs are being moved, but it will effectively kill the knobs processing
     once it does reach it's destination value*/
    else if(fabs(_current_value - (_smoother->last_value)) < (_smoother->rate))
    {
        _smoother->last_value = _current_value;
        return (_smoother->last_value);
    }
    /*Doing the actual work*/
    else if(_current_value > (_smoother->last_value))
    {
        return (_smoother->last_value) + _smoother->rate;
    }
    /*Doing the actual work*/
    else
    {
        return (_smoother->last_value) - _smoother->rate;
    }
}

#endif	/* SMOOTHER_LINEAR_H */

