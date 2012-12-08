/* 
 * File:   denormal.h
 * Author: Jeff Hubbard
 * 
 * Purpose:  This file provides functions for removing denormals from plugins.
 * 
 * Denormalization occurs when audio signals containing feedback decay into very small numbers.
 * This causes a huge spike in CPU usage.
 *
 * Created on January 11, 2012, 8:10 PM
 */

#ifndef DENORMAL_H
#define	DENORMAL_H

#ifdef	__cplusplus
extern "C" {
#endif

inline float f_remove_denormal(float);

/* inline float f_remove_denormal(float a_input)
 * 
 * Prevent recursive modules like filters and feedback delays from consuming too much CPU*/
inline float f_remove_denormal(float a_input)
{
    if((a_input < .00001f) && (a_input > -.00001f))
        return 0.0f;
    else
        return a_input;
    
}



#ifdef	__cplusplus
}
#endif

#endif	/* DENORMAL_H */

