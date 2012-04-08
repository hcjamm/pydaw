/* 
 * File:   amp.h
 * 
 * Purpose:  This file provides functions used for converting linear amplitude values to and
 * from decibels.
 * 
 * Typical usage:
 * 
 * float test1 = f_db_to_linear_fast(-6.0f);  //test1 == .5
 * float test2 = f_linear_to_db_fast(0.25f);  //test2 == -12
 * 
 * You should always prefer the fast versions except in rare circumstances where extreme accuracy
 * is required, or else you need more input range than the fast version can provide
 * 

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

#ifdef	__cplusplus
extern "C" {
#endif
    
#include <math.h>
#include "interpolate-linear.h"
    
#define ARR_AMP_DB2A_COUNT 545
#define ARR_AMP_A2DB_COUNT 400
    
//static float arr_amp_db2a [ARR_AMP_DB2A_COUNT];
//static float arr_amp_a2db [ARR_AMP_A2DB_COUNT];
    
typedef struct st_amp
{
    float result;
    t_lin_interpolater * linear;
}t_amp;

t_amp * g_amp_get();
inline float f_db_to_linear(float,t_amp*);
inline float f_linear_to_db(float,t_amp*); 
inline float f_db_to_linear_fast(float,t_amp*);
inline float f_linear_to_db_fast(float,t_amp*);
inline float f_linear_to_db_linear(float,t_amp*);

#ifdef	__cplusplus
}
#endif

#endif	/* AMP_H */

