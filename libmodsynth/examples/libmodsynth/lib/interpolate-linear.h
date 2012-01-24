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
    
inline float f_linear_interpolate(float a_a, float a_b, float a_position)
{    
    return (((a_a - a_b) * a_position) + a_a);
}


#ifdef	__cplusplus
}
#endif

#endif	/* INTERPOLATE_LINEAR_H */

