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
//TODO a function that accepts a number of table elements and checks the length of sample + 1 ??? 


inline float f_linear_interpolate(float a_a, float a_b, float a_position)
{    
    return (((a_a - a_b) * a_position) + a_a);
}

//Accepts an array as input
inline float f_linear_interpolate_array(float a_table[], float a_ptr)
{        
    int f_int_pos = (int)a_ptr;
    int f_int_pos_plus_1 = f_int_pos + 1;
    float f_pos = a_ptr - f_int_pos;
    
    return ((a_table[f_int_pos] - a_table[f_int_pos_plus_1]) * f_pos) + a_table[f_int_pos_plus_1];
}


    
#ifdef	__cplusplus
}
#endif

#endif	/* INTERPOLATE_LINEAR_H */

