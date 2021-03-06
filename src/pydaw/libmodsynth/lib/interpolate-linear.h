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

#ifndef INTERPOLATE_LINEAR_H
#define	INTERPOLATE_LINEAR_H

#include "lmalloc.h"

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct
{
    int int_pos;
    int int_pos_plus_1;
    float pos;
}t_lin_interpolater;

inline float f_linear_interpolate(float, float, float);
inline float f_linear_interpolate_arr(float[],float, t_lin_interpolater*);
inline float f_linear_interpolate_arr_wrap(float[],int,float,
        t_lin_interpolater*);
inline float f_linear_interpolate_ptr_wrap(float*,int,float,
        t_lin_interpolater*);
inline float f_linear_interpolate_ptr(float*, float, t_lin_interpolater*);
inline float f_linear_interpolate_ptr_ifh(float * a_table, int a_whole_number,
        float a_frac, t_lin_interpolater * a_lin);

#ifdef	__cplusplus
}
#endif


t_lin_interpolater * g_lin_get();

t_lin_interpolater * g_lin_get()
{
    t_lin_interpolater * f_result;

    lmalloc((void**)&f_result, sizeof(t_lin_interpolater));

    f_result->int_pos = 0;
    f_result->int_pos_plus_1 = 1;
    f_result->pos = 0.0f;

    return f_result;
}


/* inline float f_linear_interpolate(
 * float a_a, //item 0
 * float a_b, //item 1
 * float a_position)  //position between the 2, range:  0 to 1
 */
inline float f_linear_interpolate(float a_a, float a_b, float a_position)
{
    return (((a_a - a_b) * a_position) + a_b);
}

/* inline float f_linear_interpolate_arr(
 * float a_table[], //an array of floats
 * float a_ptr, //the position in a_table to interpolate
 * t_lin_interpolater * a_lin)
 *
 * You will typically want to use f_linear_interpolate_arr_wrap instead,
 * unless you already know ahead of time
 * that you either won't be outside the bounds of your table, or else that
 * wrapping is not acceptable behavior
 */
inline float f_linear_interpolate_arr(float a_table[], float a_ptr,
        t_lin_interpolater * a_lin)
{
    a_lin->int_pos = (int)a_ptr;
    a_lin->int_pos_plus_1 = a_lin->int_pos + 1;
    a_lin->pos = a_ptr - a_lin->int_pos;

    return ((a_table[(a_lin->int_pos)] - a_table[(a_lin->int_pos_plus_1)]) *
            (a_lin->pos)) + a_table[(a_lin->int_pos_plus_1)];
}

/* inline float f_linear_interpolate_arr_wrap(
 * float a_table[], //an array of floats
 * int a_table_size, //the size of a_table
 * float a_ptr)  //The position on the table you are interpolating
 *
 * example:
 * //interpolates halfway between a_table[5] and a_table[6]
 * f_linear_interpolate_arr_wrap(a_table[], 10, 5.5);
 *
 * This function wraps if a_ptr exceeds a_table_size or is less than 0.
 */
inline float f_linear_interpolate_arr_wrap(float a_table[], int a_table_size,
        float a_ptr, t_lin_interpolater * a_lin)
{
    a_lin->int_pos = (int)a_ptr;
    a_lin->int_pos_plus_1 = (a_lin->int_pos) + 1;

    if((a_lin->int_pos_plus_1) >= a_table_size)
        a_lin->int_pos_plus_1 = 0;

    a_lin->pos = a_ptr - (a_lin->int_pos);

    return (((a_table[(a_lin->int_pos)]) - (a_table[(a_lin->int_pos_plus_1)])) *
            (a_lin->pos)) + (a_table[(a_lin->int_pos_plus_1)]);
}

/* inline float f_linear_interpolate_ptr_wrap(
 * float * a_table,
 * int a_table_size,
 * float a_ptr,
 * t_lin_interpolater * a_lin)
 *
 * This method uses a pointer instead of an array the float* must be malloc'd
 * to (sizeof(float) * a_table_size)
 */
inline float f_linear_interpolate_ptr_wrap(float * a_table, int a_table_size,
        float a_ptr, t_lin_interpolater * a_lin)
{
    a_lin->int_pos = (int)a_ptr;
    a_lin->int_pos_plus_1 = (a_lin->int_pos) + 1;

    if((a_lin->int_pos) >= a_table_size)
    {
        a_lin->int_pos -= a_table_size;
    }

    if((a_lin->int_pos_plus_1) >= a_table_size)
    {
        a_lin->int_pos_plus_1 -= a_table_size;
    }

    if((a_lin->int_pos) < 0)
    {
        a_lin->int_pos += a_table_size;
    }

    if((a_lin->int_pos_plus_1) < 0)
    {
        a_lin->int_pos_plus_1 += a_table_size;
    }

    a_lin->pos = a_ptr - (a_lin->int_pos);

    return (((a_table[(a_lin->int_pos)]) - (a_table[(a_lin->int_pos_plus_1)])) *
            (a_lin->pos)) + (a_table[(a_lin->int_pos_plus_1)]);
}

/* inline float f_linear_interpolate_ptr_wrap(
 * float * a_table,
 * float a_ptr,
 * t_lin_interpolater * a_lin)
 *
 * This method uses a pointer instead of an array the float* must be malloc'd
 * to (sizeof(float) * a_table_size)
 *
 * THIS DOES NOT CHECK THAT YOU PROVIDED A VALID POSITION
 */
inline float f_linear_interpolate_ptr(float * a_table, float a_ptr,
        t_lin_interpolater * a_lin)
{
    a_lin->int_pos = (int)a_ptr;
    a_lin->int_pos_plus_1 = (a_lin->int_pos) + 1;

    a_lin->pos = a_ptr - (a_lin->int_pos);

    return (((a_table[(a_lin->int_pos)]) - (a_table[(a_lin->int_pos_plus_1)])) *
            (a_lin->pos)) + (a_table[(a_lin->int_pos_plus_1)]);
}

/* inline float f_linear_interpolate_ptr_ifh(
 * float * a_table,
 * int a_table_size,
 * int a_whole_number,
 * float a_frac,
 * t_lin_interpolater * a_lin)
 *
 * For use with the read_head type in Euphoria Sampler
 */
inline float f_linear_interpolate_ptr_ifh(float * a_table, int a_whole_number,
        float a_frac, t_lin_interpolater * a_lin)
{
    a_lin->int_pos = a_whole_number;
    a_lin->int_pos_plus_1 = (a_lin->int_pos) + 1;

    a_lin->pos = a_frac;

    return (((a_table[(a_lin->int_pos)]) - (a_table[(a_lin->int_pos_plus_1)])) *
            (a_lin->pos)) + (a_table[(a_lin->int_pos_plus_1)]);
}

#endif	/* INTERPOLATE_LINEAR_H */

