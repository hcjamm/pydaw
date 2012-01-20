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
#include "../../lib/pitch_core.h"
#include <math.h>

/*C doesn't like dynamic arrays, so we have to define this at compile time.  
 Realistically, we're doing the users a favor by limiting it to 7, because having
 200 sawtooth oscillators playing at the same time doesn't sound fundamentally better than 7*/
#define OSC_UNISON_MAX_VOICES 7
    
//Used to switch between values, uses much less CPU than a switch statement
typedef float (*_get_osc_func_ptr)(osc_core*);


/*A unison oscillator.  The _osc_type pointer determines which waveform it uses, it defaults to saw unless you set it to something else*/
typedef struct _osc_simple_unison
{
    float _sr_recip;
    int _voice_count;  //Set this to the max number of voices, not to exceed OSC_UNISON_MAX_VOICES
    _get_osc_func_ptr _osc_type;        
    float _bottom_pitch;
    float _pitch_inc;
    float _voice_inc [OSC_UNISON_MAX_VOICES];
    osc_core * _cores [OSC_UNISON_MAX_VOICES];
    
    float _phases [OSC_UNISON_MAX_VOICES];  //Restart the oscillators at the same phase on each note-on
    
    /*Not yet implemented, this is intended to 
     * paired with the glide and pitchbend functionality*/
    float _current_pitch;
    float _destination_pitch;
}osc_simple_unison;


void _osc_set_uni_voice_count(osc_simple_unison*, int);

void _osc_set_uni_voice_count(osc_simple_unison* _osc_ptr, int _value)
{
    if(_value > (OSC_UNISON_MAX_VOICES))
    {
        _osc_ptr->_voice_count = OSC_UNISON_MAX_VOICES;
    }
    else if(_value < 1)
    {
        _osc_ptr->_voice_count = 1;
    }
    else
    {
        _osc_ptr->_voice_count = _value;
    }
}

float _osc_run_unison_osc(osc_simple_unison * _osc_ptr);

//Return one sample of the oscillator running.
float _osc_run_unison_osc(osc_simple_unison * _osc_ptr)
{
    int i = 0;
    float _result = 0;
    while(i < (_osc_ptr->_voice_count))
    {
        _run_osc((_osc_ptr->_cores[i]), (_osc_ptr->_voice_inc[i]));
        _result += _osc_ptr->_osc_type((_osc_ptr->_cores[i]));
        i++;
    }
    
    return _result;
}

void _osc_set_unison_osc_spread(osc_simple_unison * _osc_ptr, float _amount);


void _osc_set_unison_osc_spread(osc_simple_unison * _osc_ptr, float _amount)
{
    //TODO:  review this logic for when there is only one voice, and make sure that the spread is accurate and
    //doesn't need to be voice_count + 1
    _osc_ptr->_bottom_pitch = -.5 * _amount;
    _osc_ptr->_pitch_inc = _amount / ((float)(_osc_ptr->_voice_count));
}

void _osc_set_unison_osc_pitch(osc_simple_unison * _osc_ptr, float _pitch);


void _osc_set_unison_osc_pitch(osc_simple_unison * _osc_ptr, float _pitch)
{
    int i = 0;
    
    while(i < (_osc_ptr->_voice_count))
    {
        _osc_ptr->_voice_inc[i] =  _pit_midi_note_to_hz(_pitch + (_osc_ptr->_bottom_pitch) + (_osc_ptr->_pitch_inc * ((float)i))) * _osc_ptr->_sr_recip;
        i++;
    }
}


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

//Return zero if the oscillator is turned off
float _get_osc_off(osc_core * _core)
{
    return 0;
}


void _osc_set_simple_osc_unison_type(osc_simple_unison *, int);


/*Set the oscillator type.  Current valid types are:
 * 0. Saw
 * 1. Square
 * 2. Triangle
 * 3. Sine
 * 4. Off
 */
void _osc_set_simple_osc_unison_type(osc_simple_unison * _osc_ptr, int _index)
{
    switch(_index)
    {
        case 0:
            _osc_ptr->_osc_type = _get_saw;
            break;
        case 1:
            _osc_ptr->_osc_type = _get_square;
            break;
        case 2:
            _osc_ptr->_osc_type = _get_triangle;
            break;
        case 3:
            _osc_ptr->_osc_type = _get_sine;
            break;
        case 4:
            _osc_ptr->_osc_type = _get_osc_off;
            break;
    }
    
}

void _osc_note_on_sync_phases(osc_simple_unison *);

/*Resync the oscillators at note_on to hopefully avoid phasing artifacts*/
void _osc_note_on_sync_phases(osc_simple_unison * _osc_ptr)
{
    int i = 0;
    
    while(i < _osc_ptr->_voice_count)
    {
        _osc_ptr->_cores[i]->_output = _osc_ptr->_phases[i];
        
        i++;
    }
}


osc_simple_unison * _osc_get_osc_simple_unison(float);

osc_simple_unison * _osc_get_osc_simple_unison(float __sr_recip)
{
    osc_simple_unison * _result = (osc_simple_unison*)malloc(sizeof(osc_simple_unison));
    
    _result->_voice_count = OSC_UNISON_MAX_VOICES;    
    _result->_osc_type = _get_saw;
    _result->_sr_recip = __sr_recip;
    
    int i = 0;
    
    while(i < (OSC_UNISON_MAX_VOICES))
    {
        _result->_cores[i] = (osc_core*)malloc(sizeof(osc_core));
        i++;
    }
    
    _osc_set_unison_osc_spread(_result, .5);
    _osc_set_unison_osc_pitch(_result, 60);
    
    i = 0;
    
    //Prevent phasing artifacts from the oscillators starting at the same phase.
    while(i < 200000)
    {
        _osc_run_unison_osc(_result);
        i++;
    }
    
    i = 0;
        
    while(i < (OSC_UNISON_MAX_VOICES))
    {
        _result->_phases[i] = _result->_cores[i]->_output;
        i++;
    }
    
    _osc_set_unison_osc_spread(_result, .2);
    
    return _result;
}


#ifdef	__cplusplus
}
#endif

#endif	/* OSC_SIMPLE_H */

