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

#ifndef INTERPOLATE_CUBIC_H
#define	INTERPOLATE_CUBIC_H

#include "lmalloc.h"

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct
{
    float a0,a1,a2,a3,mu,mu2;
    int int_pos, int_pos_plus1, int_pos_minus1, int_pos_minus2;
}t_cubic_interpolater;

inline float f_cubic_interpolate_ptr_wrap(float*, int, float,
        t_cubic_interpolater*);
inline float f_cubic_interpolate_ptr(float*, float, t_cubic_interpolater*);

t_cubic_interpolater * g_cubic_get();

#ifdef	__cplusplus
}
#endif


t_cubic_interpolater * g_cubic_get()
{
    t_cubic_interpolater * f_result;

    lmalloc((void**)&f_result, sizeof(t_cubic_interpolater));

    f_result->a0 = 0.0f;
    f_result->a1 = 0.0f;
    f_result->a2 = 0.0f;
    f_result->a3 = 0.0f;
    f_result->mu = 0.0f;
    f_result->mu2 = 0.0f;

    f_result->int_pos = 0;
    f_result->int_pos_minus1 = 0;
    f_result->int_pos_minus2 = 0;
    f_result->int_pos_plus1 = 0;

    return f_result;
}


/* inline float f_cubic_interpolate(
 * float a_a, //item 0
 * float a_b, //item 1
 * float a_position)  //position between the 2, range:  0 to 1
 */
/*
inline float f_cubic_interpolate(float a_a, float a_b, float a_position)
{
    return (((a_a - a_b) * a_position) + a_a);
}
*/


/* inline float f_cubic_interpolate_ptr_wrap(
 * float * a_table,
 * int a_table_size,
 * float a_ptr,
 * t_cubic_interpolater * a_cubic)
 *
 * This method uses a pointer instead of an array the float* must be malloc'd
 * to (sizeof(float) * a_table_size)
 */
inline float f_cubic_interpolate_ptr_wrap(float * a_table, int a_table_size,
        float a_ptr, t_cubic_interpolater * a_cubic)
{
    a_cubic->int_pos = (int)a_ptr;
    a_cubic->mu = a_ptr - ((float)a_cubic->int_pos);
    a_cubic->mu2 = (a_cubic->mu) * (a_cubic->mu);
    a_cubic->int_pos_plus1 = (a_cubic->int_pos) + 1;
    a_cubic->int_pos_minus1 = (a_cubic->int_pos) - 1;
    a_cubic->int_pos_minus2 = (a_cubic->int_pos) - 2;

    if(a_cubic->int_pos >= a_table_size)
    {
        a_cubic->int_pos = (a_cubic->int_pos) - a_table_size;
    }
    else if(a_cubic->int_pos < 0)
    {
        a_cubic->int_pos = (a_cubic->int_pos) + a_table_size;
    }

    if(a_cubic->int_pos_plus1 >= a_table_size)
    {
        a_cubic->int_pos_plus1 = (a_cubic->int_pos_plus1) - a_table_size;
    }
    else if(a_cubic->int_pos_plus1 < 0)
    {
        a_cubic->int_pos_plus1 = (a_cubic->int_pos_plus1) + a_table_size;
    }

    if(a_cubic->int_pos_minus1 >= a_table_size)
    {
        a_cubic->int_pos_minus1 = (a_cubic->int_pos_minus1) - a_table_size;
    }
    else if(a_cubic->int_pos_minus1 < 0)
    {
        a_cubic->int_pos_minus1 = (a_cubic->int_pos_minus1) + a_table_size;
    }

    if(a_cubic->int_pos_minus2 >= a_table_size)
    {
        a_cubic->int_pos_minus2 = (a_cubic->int_pos_minus2) - a_table_size;
    }
    else if(a_cubic->int_pos_minus2 < 0)
    {
        a_cubic->int_pos_minus2 = (a_cubic->int_pos_minus2) + a_table_size;
    }

    a_cubic->a0 = a_table[a_cubic->int_pos_plus1] - a_table[a_cubic->int_pos] -
            a_table[a_cubic->int_pos_minus2] + a_table[a_cubic->int_pos_minus1];
    a_cubic->a1 = a_table[a_cubic->int_pos_minus2] -
            a_table[a_cubic->int_pos_minus1] - a_cubic->a0;
    a_cubic->a2 = a_table[a_cubic->int_pos] - a_table[a_cubic->int_pos_minus2];
    a_cubic->a3 = a_table[a_cubic->int_pos_minus1];

    return(a_cubic->a0 * a_cubic->mu * a_cubic->mu2 + a_cubic->a1 *
            a_cubic->mu2 + a_cubic->a2 * a_cubic->mu + a_cubic->a3);
}

/* inline float f_cubic_interpolate_ptr_wrap(
 * float * a_table,
 * float a_ptr,
 * t_cubic_interpolater * a_lin)
 *
 * This method uses a pointer instead of an array the float* must be
 * malloc'd to (sizeof(float) * a_table_size)
 *
 * THIS DOES NOT CHECK THAT YOU PROVIDED A VALID POSITION
 */

inline float f_cubic_interpolate_ptr(float * a_table, float a_ptr,
        t_cubic_interpolater * a_cubic)
{
    a_cubic->int_pos = (int)a_ptr;
    a_cubic->int_pos_plus1 = (a_cubic->int_pos) + 1;
    a_cubic->int_pos_minus1 = (a_cubic->int_pos) - 1;
    a_cubic->int_pos_minus2 = (a_cubic->int_pos) - 2;

#ifdef PYDAW_NO_HARDWARE
    //Check this when run with no hardware, but otherwise save the CPU.
    //Anything sending a position to this should already know that the position is valid.
    assert(a_cubic->int_pos_minus1 >= 0);
    assert(a_cubic->int_pos_minus2 >= 0);
#endif

    a_cubic->mu = a_ptr - (a_cubic->int_pos);

    a_cubic->mu2 = (a_cubic->mu) * (a_cubic->mu);
    a_cubic->a0 = a_table[a_cubic->int_pos_plus1] - a_table[a_cubic->int_pos] -
            a_table[a_cubic->int_pos_minus2] + a_table[a_cubic->int_pos_minus1];
    a_cubic->a1 = a_table[a_cubic->int_pos_minus2] -
            a_table[a_cubic->int_pos_minus1] - a_cubic->a0;
    a_cubic->a2 = a_table[a_cubic->int_pos] - a_table[a_cubic->int_pos_minus2];
    a_cubic->a3 = a_table[a_cubic->int_pos_minus1];

    return (a_cubic->a0 * a_cubic->mu * a_cubic->mu2 + a_cubic->a1 *
            a_cubic->mu2 + a_cubic->a2 * a_cubic->mu + a_cubic->a3);
}


/* inline float f_cubic_interpolate_ptr_ifh(
 * float * a_table,
 * int a_table_size,
 * int a_whole_number,
 * float a_frac,
 * t_lin_interpolater * a_lin)
 *
 * For use with the read_head type in Euphoria Sampler
 */
inline float f_cubic_interpolate_ptr_ifh(float * a_table, int a_whole_number,
        float a_frac, t_cubic_interpolater * a_cubic)
{
    a_cubic->int_pos = a_whole_number;
    a_cubic->int_pos_plus1 = (a_cubic->int_pos) + 1;
    a_cubic->int_pos_minus1 = (a_cubic->int_pos) - 1;
    a_cubic->int_pos_minus2 = (a_cubic->int_pos) - 2;

    a_cubic->mu = a_frac;

    a_cubic->mu2 = (a_cubic->mu) * (a_cubic->mu);
    a_cubic->a0 = a_table[a_cubic->int_pos_plus1] - a_table[a_cubic->int_pos] -
            a_table[a_cubic->int_pos_minus2] + a_table[a_cubic->int_pos_minus1];
    a_cubic->a1 = a_table[a_cubic->int_pos_minus2] -
            a_table[a_cubic->int_pos_minus1] - a_cubic->a0;
    a_cubic->a2 = a_table[a_cubic->int_pos] - a_table[a_cubic->int_pos_minus2];
    a_cubic->a3 = a_table[a_cubic->int_pos_minus1];

    return (a_cubic->a0 * a_cubic->mu * a_cubic->mu2 + a_cubic->a1 *
            a_cubic->mu2 + a_cubic->a2 * a_cubic->mu + a_cubic->a3);
}

#endif	/* INTERPOLATE_CUBIC_H */

