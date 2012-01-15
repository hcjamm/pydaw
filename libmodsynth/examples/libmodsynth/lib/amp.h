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

float _db_to_linear(float _db)
{
    float _result = pow ( 10.0, (0.05 * _db) );
    return _result;
}

float _linear_to_db(float _linear)
{
    float _result = 20.0 * log10 ( _linear );
    return _result;
}



#ifdef	__cplusplus
}
#endif

#endif	/* AMP_H */

