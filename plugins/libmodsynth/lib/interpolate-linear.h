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
    
typedef struct st_lin_interpolater
{
    int int_pos;
    int int_pos_plus_1;
    float pos;    
}t_lin_interpolater;

t_lin_interpolater * g_lin_get();
inline float f_linear_interpolate(float, float, float);
inline float f_linear_interpolate_arr(float[],float, t_lin_interpolater*);
inline float f_linear_interpolate_arr_wrap(float[],int,float, t_lin_interpolater*);
inline float f_linear_interpolate_ptr_wrap(float*,int,float, t_lin_interpolater*);
   
#ifdef	__cplusplus
}
#endif

#endif	/* INTERPOLATE_LINEAR_H */

