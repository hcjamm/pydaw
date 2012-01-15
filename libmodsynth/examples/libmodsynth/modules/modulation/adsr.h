/* 
 * File:   adsr.h
 This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

 */

#ifndef ADSR_H
#define	ADSR_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include "../../../constants.h"
#include "../../lib/amp.h"

/*TODO:  Add an option to start the attack at -20db or so...*/
typedef struct _adsr
{
    float a_inc;
    float a_time;
    float d_inc;
    float d_time;
    float s_value;
    
    float d_recip;
    float r_recip;
    
    float r_inc;
    float r_time;
    
    float _sr_recip;
        
    int stage;  //0=a,1=d,2=s,3=r,4=inactive
    
    float output;
}adsr;

void _adsr_set_a_time(adsr*, float);
void _adsr_set_d_time(adsr*, float);
void _adsr_set_s_value(adsr*, float);
void _adsr_set_s_value_db(adsr*, float);
void _adsr_set_r_time(adsr*, float);

void _adsr_set_adsr_db(adsr*, float, float, float, float);
void _adsr_set_adsr(adsr*, float, float, float, float);

void _adsr_retrigger(adsr *);
void _adsr_release(adsr *);
void _adsr_run(adsr *);

adsr * _adsr_get_adsr(float);

void _adsr_set_a_time(adsr* _adsr_ptr, float _time)
{
    if((_adsr_ptr->a_time) == _time)
        return;
    
    _adsr_ptr->a_time = _time;
    
    if(_time <= 0)
    {
        _adsr_ptr->a_inc = 1;
    }
    else
    {
        _adsr_ptr->a_inc = (_adsr_ptr->_sr_recip) / (_adsr_ptr->a_time);
    }
        
}

void _adsr_set_d_time(adsr* _adsr_ptr, float _time)
{
    if((_adsr_ptr->d_time) == _time)
        return;
    
    if(_time <= 0)
    {
        _adsr_ptr->d_time = .05;
    }
    else
    {
        _adsr_ptr->d_time = _time;        
    }
    
    printf("Setting D time to %f\n", (_adsr_ptr->d_time));
    
    _adsr_ptr->d_inc = ((_adsr_ptr->_sr_recip) / (_adsr_ptr->d_time)) * -1;    
}

void _adsr_set_r_time(adsr* _adsr_ptr, float _time)
{
    if((_adsr_ptr->r_time) == _time)
        return;
    
    if(_time <= 0)
    {
        _adsr_ptr->r_time = .05;
    }
    else
    {
        _adsr_ptr->r_time = _time;        
    }    
    
    _adsr_ptr->r_inc = ((_adsr_ptr->_sr_recip) / (_adsr_ptr->r_time)) * -1;
    
    printf("Setting R time to %f\n", (_adsr_ptr->r_time));
    
}

void _adsr_set_s_value(adsr* _adsr_ptr, float _value)
{
    _adsr_ptr->s_value = _value;
    
    if((_adsr_ptr->s_value) <= 0)
    {
        _adsr_ptr->s_value = .001;
    }
    
    _adsr_ptr->d_recip = (1/(1-(_adsr_ptr->s_value)));
    _adsr_ptr->r_recip = (1/(_adsr_ptr->s_value));
    
    printf("Setting S value to %f\n", (_adsr_ptr->s_value));
}

void _adsr_set_s_value_db(adsr* _adsr_ptr, float _value)
{
    _adsr_set_s_value(_adsr_ptr, _db_to_linear(_value));
}

void _adsr_set_adsr(adsr* _adsr_ptr, float _a, float _d, float _s, float _r)
{
    _adsr_set_a_time(_adsr_ptr, _a);
    _adsr_set_d_time(_adsr_ptr, _d);
    _adsr_set_s_value(_adsr_ptr, _s);
    _adsr_set_r_time(_adsr_ptr, _r);
}

void _adsr_set_adsr_db(adsr* _adsr_ptr, float _a, float _d, float _s, float _r)
{
    _adsr_set_a_time(_adsr_ptr, _a);
    _adsr_set_d_time(_adsr_ptr, _d);
    _adsr_set_s_value_db(_adsr_ptr, _s);
    _adsr_set_r_time(_adsr_ptr, _r);
}

void _adsr_retrigger(adsr * _adsr_ptr)
{
    _adsr_ptr->stage = 0;
    _adsr_ptr->output = 0;    
}

void _adsr_release(adsr * _adsr_ptr)
{
    _adsr_ptr->stage = 3;    
}

adsr * _adsr_get_adsr(float __sr_recip)
{
    adsr * _result = (adsr*)malloc(sizeof(adsr));
    
    _result->_sr_recip = __sr_recip;
    
    _adsr_set_a_time(_result, .05);
    _adsr_set_d_time(_result, .5);
    _adsr_set_s_value_db(_result, -12);
    _adsr_set_r_time(_result, .5);

    _adsr_retrigger(_result);
    
    return _result;
}

void _adsr_run(adsr * _adsr_ptr)
{
    if((_adsr_ptr->stage) != 4)
    {         
        switch(_adsr_ptr->stage)
        {            
            case 0:
                _adsr_ptr->output = (_adsr_ptr->output) + (_adsr_ptr->a_inc);
                if((_adsr_ptr->output) >= 1)
                {
                    _adsr_ptr->stage = 1;
                    //printf("ADSR stage1\n");
                }
                break;
            case 1:
                _adsr_ptr->output =  (_adsr_ptr->output) + (_adsr_ptr->d_inc);
                if((_adsr_ptr->output) <= (_adsr_ptr->s_value))
                {
                    _adsr_ptr->stage = 2;
                    //printf("ADSR stage2\n");
                }
                break;
            case 2:
                //Do nothing, we are sustaining
                break;
            case 3:
                /*Currently, this would actually take longer to release if the note off arrives
                 before the decay stage finishes, I may fix it later*/
                _adsr_ptr->output =  (_adsr_ptr->output) + (_adsr_ptr->r_inc);
                if((_adsr_ptr->output) <= 0)
                {
                    _adsr_ptr->output = 0;
                    _adsr_ptr->stage = 4;
                    //printf("ADSR stage4\n");
                }
                break;
        }
    }
}


#ifdef	__cplusplus
}
#endif

#endif	/* ADSR_H */

