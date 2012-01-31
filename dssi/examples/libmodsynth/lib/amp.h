/* 
 * File:   amp.h

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

 */

#ifndef AMP_H
#define	AMP_H

#include <math.h>

#ifdef	__cplusplus
extern "C" {
#endif

inline float f_db_to_linear(float);
inline float f_linear_to_db(float);    


inline float f_db_to_linear(float a_db)
{
    float f_result = pow ( 10.0, (0.05 * a_db) );
    return f_result;
}

inline float f_linear_to_db(float a_linear)
{
    float f_result = 20.0 * log10 ( a_linear );
    return f_result;
}


#ifdef	__cplusplus
}
#endif

#endif	/* AMP_H */

