/* 
 * File:   interpolate-linear.h
 * Author: Jeff Hubbard
 * 
 * Purpose:  This file provides several different functions for performing linear interpolation,
 * depending on your source material.
 * 
 * Created on January 8, 2012, 11:14 AM
 */

#ifndef INTERPOLATE_LINEAR_H
#define	INTERPOLATE_LINEAR_H

#ifdef	__cplusplus
extern "C" {
#endif

    
inline float f_linear_interpolate(float, float, float);
inline float f_linear_interpolate_arr(float[],float);
inline float f_linear_interpolate_arr_wrap(float[],int,float);

/*These are meant to prevent allocating objects constantly, without having to immediately
 re-write everything that uses them.  If LMS ever becomes multi-threaded, these will need
 to be moved to their own struct and passed to the functions*/
static int f_int_pos;
static int f_int_pos_plus_1;
static float f_pos;

/* inline float f_linear_interpolate(
 * float a_a, //item 0
 * float a_b, //item 1
 * float a_position)  //position between the 2, range:  0 to 1
 */
inline float f_linear_interpolate(float a_a, float a_b, float a_position)
{    
    return (((a_a - a_b) * a_position) + a_a);
}

/* inline float f_linear_interpolate_arr(
 * float a_table[], //an array of floats 
 * float a_ptr) //the position in a_table to interpolate
 * 
 * You will typically want to use f_linear_interpolate_arr_wrap instead, unless you already know ahead of time
 * that you either won't be outside the bounds of your table, or else that wrapping is not acceptable behavior
 */
inline float f_linear_interpolate_arr(float a_table[], float a_ptr)
{        
    f_int_pos = (int)a_ptr;
    f_int_pos_plus_1 = f_int_pos + 1;
    f_pos = a_ptr - f_int_pos;
    
    return ((a_table[f_int_pos] - a_table[f_int_pos_plus_1]) * f_pos) + a_table[f_int_pos_plus_1];
}

/* inline float f_linear_interpolate_arr_wrap(
 * float a_table[], //an array of floats
 * int a_table_size, //the size of a_table
 * float a_ptr)  //The position on the table you are interpolating
 * 
 * example:
 * 
 * f_linear_interpolate_arr_wrap(a_table[], 10, 5.5);  //interpolates halfway between a_table[5] and a_table[6]
 * 
 * This function wraps if a_ptr exceeds a_table_size or is less than 0.
 */
inline float f_linear_interpolate_arr_wrap(float a_table[], int a_table_size, float a_ptr)
{        
    f_int_pos = (int)a_ptr;
    f_int_pos_plus_1 = f_int_pos + 1;
    
    if(f_int_pos_plus_1 > a_table_size)
        f_int_pos_plus_1 = 0;
    
    f_pos = a_ptr - f_int_pos;
    
    return (((a_table[f_int_pos]) - (a_table[f_int_pos_plus_1])) * f_pos) + (a_table[f_int_pos_plus_1]);
}

    
#ifdef	__cplusplus
}
#endif

#endif	/* INTERPOLATE_LINEAR_H */

