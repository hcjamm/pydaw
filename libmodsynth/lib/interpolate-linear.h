/* 
 * File:   interpolate-linear.h
 * Author: vm-user
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

inline float f_linear_interpolate(float a_a, float a_b, float a_position)
{    
    return (((a_a - a_b) * a_position) + a_a);
}

//Accepts an array as input
inline float f_linear_interpolate_arr(float a_table[], float a_ptr)
{        
    f_int_pos = (int)a_ptr;
    f_int_pos_plus_1 = f_int_pos + 1;
    f_pos = a_ptr - f_int_pos;
    
    return ((a_table[f_int_pos] - a_table[f_int_pos_plus_1]) * f_pos) + a_table[f_int_pos_plus_1];
}

/*Wrap if the next sample is past the array boundary*/
inline float f_linear_interpolate_arr_wrap(float a_table[], int a_table_size, float a_ptr)
{        
    f_int_pos = (int)a_ptr;
    f_int_pos_plus_1 = f_int_pos + 1;
    
    if(f_int_pos_plus_1 > a_table_size)
        f_int_pos_plus_1 = 0;
    
    f_pos = a_ptr - f_int_pos;
    
    return ((a_table[f_int_pos] - a_table[f_int_pos_plus_1]) * f_pos) + a_table[f_int_pos_plus_1];
}

    
#ifdef	__cplusplus
}
#endif

#endif	/* INTERPOLATE_LINEAR_H */

