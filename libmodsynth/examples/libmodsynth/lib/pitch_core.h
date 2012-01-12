/* 
 * File:   pitch_core.h
 * Author: vm-user
 *
 * Created on January 7, 2012, 7:21 PM
 */

#ifndef PITCH_CORE_H
#define	PITCH_CORE_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include <math.h>
#include "../../constants.h"


/*Forward declaration of functions*/

float _pit_midi_note_to_hz(float);
    
float _pit_hz_to_midi_note(float);


/*Functions*/

float _pit_midi_note_to_hz(float _midi_note_number)
{
    float _result;
    _result = base_a4*pow(2,(_midi_note_number-57)*.0833333);
    return _result;
}

float _pit_hz_to_midi_note(float _hz)
{
    float _result=(12*log2(_hz*base_a4_recip))+57;
    return _result;
}


#ifdef	__cplusplus
}
#endif

#endif	/* PITCH_CORE_H */

