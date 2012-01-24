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

inline float f_pit_midi_note_to_hz(float);    
inline float f_pit_hz_to_midi_note(float);


/*Functions*/

inline float f_pit_midi_note_to_hz(float a_midi_note_number)
{
    float f_result;
    f_result = base_a4*pow(2,(a_midi_note_number-57)*.0833333);
    return f_result;
}

inline float f_pit_hz_to_midi_note(float _hz)
{
    float f_result=(12*log2(_hz*base_a4_recip))+57;
    return f_result;
}


#ifdef	__cplusplus
}
#endif

#endif	/* PITCH_CORE_H */

