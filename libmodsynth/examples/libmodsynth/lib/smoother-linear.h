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
    float rate;
    float last_value;
    float _sr;
    float _sr_recip;
}smoother_linear;

/*There's not much good reason to change this while the synth is running for controls, so you should only set it here.
 If using this for glide or other things that must be smoothed dynamically, you can use the set method below*/
smoother_linear * _sml_get_smoother_linear(float _sample_rate, float _high, float _low, float _time_in_seconds)
{
    smoother_linear * _result = (smoother_linear*)malloc(sizeof(smoother_linear));
    /*Start in the middle, the user can manually set the value if this isn't acceptable*/
    _result->last_value = (((_high - _low) * .5) + _low);
    
    /*Rate is the time it would take to complete if the knob was all the way counter-clockwise, and then instantly moved all the way clockwise*/
    _result->rate = (((_high - _low ) * _time_in_seconds) / _sample_rate);
    
    _result->_sr = _sample_rate;
    _result->_sr_recip = 1/_sample_rate;
    
    return _result;
}

/*A special function for using a linear smoother as a glide module, run this at note_on
 TODO:  Add pitchbend, etc... as separate arguments and allow intelligent rate smoothing depending on whether
 glide is actually turned on or not*/
void _sml_set_smoother_glide(smoother_linear * _sml_ptr, float _target, float _current, float _time_in_seconds)
{
        
    //We will essentially turn it off by setting the current value as target
    if(_time_in_seconds < .05)
    {
        _sml_ptr->last_value = _target;
    }
    else
    {
        _sml_ptr->rate = (((_target - _current ) * _time_in_seconds) * (_sml_ptr->_sr_recip));
        _sml_ptr->last_value = _current;
    }
    
    printf("Last Value  %f\n", (_sml_ptr->last_value));
    printf("Target  %f\n",_target);
    printf("Rate  %f\n",_sml_ptr->rate);
    printf("Time in seconds %f\n",_time_in_seconds);
}

float _sml_run(smoother_linear * _smoother, float _current_value)
{
    /*TODO:  evaluate this more thoroughly.  I'm trying to make it as CPU efficient as possible, since almost every port and/or GUI control will have one.*/
    
    /*Evaluated first because most controls won't be moving most of the time, should consume the fewest cycles*/
    if((_smoother->last_value) == _current_value)
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
        _smoother->last_value = (_smoother->last_value) + (_smoother->rate);
        return (_smoother->last_value);
    }
    /*Doing the actual work*/
    else
    {
        _smoother->last_value = (_smoother->last_value) - (_smoother->rate);
        return (_smoother->last_value);
    }
}


void _sml_run_glide(smoother_linear * _smoother, float _target_value)
{    
    /*Evaluated first because most controls won't be moving most of the time, should consume the fewest cycles*/
    if(((_smoother->last_value) == _target_value))
    {        
        //Do nothing
    } 
    /*This does waste CPU while knobs are being moved, but it will effectively kill the knobs processing
     once it does reach it's destination value*/
    else if(fabs(_target_value - (_smoother->last_value)) < (_smoother->rate))
    {
        _smoother->last_value = _target_value;
        
    }
    /*Doing the actual work*/
    else if(_target_value > (_smoother->last_value))
    {
        _smoother->last_value = (_smoother->last_value) + (_smoother->rate);
    }
    /*Doing the actual work*/
    else
    {
        _smoother->last_value = (_smoother->last_value) - (_smoother->rate);
    }
}


#endif	/* SMOOTHER_LINEAR_H */

