/*
This file is part of the PyDAW project, Copyright PyDAW Team

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef DENORMAL_H
#define	DENORMAL_H

#ifdef	__cplusplus
extern "C" {
#endif

inline float f_remove_denormal(float);

#ifdef	__cplusplus
}
#endif

/* inline float f_remove_denormal(float a_input)
 *
 * Prevent recursive modules like filters and feedback delays from
 * consuming too much CPU
 */
inline float f_remove_denormal(float a_input)
{
    if((a_input < .00001f) && (a_input > -.00001f))
        return 0.0f;
    else
        return a_input;

}

#endif	/* DENORMAL_H */

