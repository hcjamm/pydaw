/* 
 * File:   glide.h
 * Author: vm-user
 *
 * Created on January 20, 2012, 9:55 AM
 */

#ifndef GLIDE_H
#define	GLIDE_H

#ifdef	__cplusplus
extern "C" {
#endif

    
typedef enum _gld_glide_mode
{
    gld_constant_rate, gld_constant_time, gld_off
} gld_glide_mode;

//This is for future use, the user will have the option to glide only when notes move in a particular direction
typedef enum _gld_glide_filter
{
    gld_all, gld_only_up, gld_only_down
} gld_glide_filter;

    
typedef struct _gld_glide
{
    gld_glide_mode _mode;
    float _time;
    int _is_running;
    float _inc;
    float _pitchbend_amt;
    float _sr, _sr_recip;
    float _last_pitch;  //where we were last tick of the SR clock
    float _current_pitch;
    float _target_pitch;  //where we intend to get to
    
    float twelve_div_sr;  //for speed calculations
    
}gld_glide;


/*Forward Declarations*/
gld_glide * _gld_get_glide(float _sample_rate);
void _gld_set_inc_time(gld_glide * _gld_ptr);
void _gld_set_inc_speed(gld_glide * _gld_ptr);
void _gld_set_glide(gld_glide * _gld_ptr, float _glide_time, gld_glide_mode glide_mode);
float _gld_run_glide(gld_glide * _gld_ptr);
void _gld_set_pitch(gld_glide * _gld_ptr, float _pitch, float _previous_pitch);
void _gld_set_pitchbend(gld_glide * _gld_ptr, float _pb_amt);



gld_glide * _gld_get_glide(float _sample_rate)
{
    gld_glide * _result = (gld_glide*)malloc(sizeof(gld_glide));
    
    _result->_mode = gld_constant_time;
    _result->_is_running = 0;
    _result->_sr = _sample_rate;
    _result->_sr_recip = 1/_sample_rate;
    _result->twelve_div_sr = 12 /_sample_rate;
    _result->_last_pitch = 60;   //TODO:  Get rid of this
    return _result;
}


void _gld_set_inc_time(gld_glide * _gld_ptr)
{
    _gld_ptr->_target_pitch = (_gld_ptr->_target_pitch) + (_gld_ptr->_pitchbend_amt);
    
    _gld_ptr->_inc = ((_gld_ptr->_current_pitch) - (_gld_ptr->_target_pitch)) * -1 * _gld_ptr->_sr_recip *  _gld_ptr->_time;
}

void _gld_set_inc_speed(gld_glide * _gld_ptr)
{
    _gld_ptr->_target_pitch = (_gld_ptr->_target_pitch) + (_gld_ptr->_pitchbend_amt);
    
    float _diff = (_gld_ptr->_pitchbend_amt) - (_gld_ptr->_target_pitch);
    
    if(_diff > 0)
    {
        _gld_ptr->_inc = -1 * (_gld_ptr->twelve_div_sr) * (_gld_ptr->_time);
    }
    else if(_diff < 0)
    {
        _gld_ptr->_inc = (_gld_ptr->twelve_div_sr) * (_gld_ptr->_time);
    }
    else
    {
        _gld_ptr->_inc = 0;
        _gld_ptr->_is_running = 0;
    }
    
}


void _gld_set_pitchbend(gld_glide * _gld_ptr, float _pb_amt)
{
    /*Quantize small values to zero so not to screw up tuning*/
    if((_pb_amt < .05) && (_pb_amt > -.05))
    {
        _gld_ptr->_pitchbend_amt = 0;
        return;
    }
    
    _gld_ptr->_pitchbend_amt = _pb_amt;
    
    
    _gld_ptr->_is_running = 1;
    
    switch(_gld_ptr->_mode)
    {
        case gld_constant_rate:
            /*The knob goes zero to 2, which would mean 2 seconds per octave this way*/            
            _gld_set_inc_speed(_gld_ptr);
            break;
        case gld_constant_time:
            /*The knob goes zero to 2, which would mean 2 seconds total time this way*/            
            _gld_set_inc_time(_gld_ptr);
            break;
        case gld_off:
            _gld_set_inc_time(_gld_ptr);  //TODO:  Check the actual increment time
            break;
    }
}

void _gld_note_on(gld_glide * _gld_ptr, float _pitch, float _previous_pitch, float _glide_time, gld_glide_mode _glide_mode)
{    
    _gld_ptr->_last_pitch = _previous_pitch;
    _gld_ptr->_target_pitch = _pitch + (_gld_ptr->_pitchbend_amt);
    
    
    _gld_ptr->_mode = _glide_mode;
    _gld_ptr->_time = _glide_time;
    
    if(_glide_time <= .03)  //TODO: figure out an exact arbitrary cutoff time where it no longer matters
    {
        _gld_ptr->_mode = gld_off;
    }        
    
    switch(_gld_ptr->_mode)
    {
        case gld_constant_rate:
            /*The knob goes zero to 2, which would mean 2 seconds per octave this way*/
            _gld_ptr->_is_running = 1;
            _gld_set_inc_speed(_gld_ptr);
            break;
        case gld_constant_time:
            /*The knob goes zero to 2, which would mean 2 seconds total time this way*/
            _gld_ptr->_is_running = 1;            
            _gld_set_inc_time(_gld_ptr);
            break;
        case gld_off:
            _gld_ptr->_is_running = 0;
            _gld_ptr->_current_pitch = _pitch;            
            break;
    }
}


float _gld_run_glide(gld_glide * _gld_ptr)
{
    if((_gld_ptr->_is_running) == 1)
    {
        _gld_ptr->_current_pitch = (_gld_ptr->_current_pitch) + (_gld_ptr->_inc);
        
        if(((_gld_ptr->_inc) < 0) && ((_gld_ptr->_current_pitch) <= (_gld_ptr->_target_pitch)))
        {
            _gld_ptr->_current_pitch = _gld_ptr->_target_pitch;
            _gld_ptr->_is_running = 0;
        }
        else if(((_gld_ptr->_inc) > 0) && ((_gld_ptr->_current_pitch) >= (_gld_ptr->_target_pitch)))
        {
            _gld_ptr->_current_pitch = _gld_ptr->_target_pitch;
            _gld_ptr->_is_running = 0;
        }
        
        return (_gld_ptr->_current_pitch);
    }
    else
    {
        return (_gld_ptr->_current_pitch);
    }
}

#ifdef	__cplusplus
}
#endif

#endif	/* GLIDE_H */

