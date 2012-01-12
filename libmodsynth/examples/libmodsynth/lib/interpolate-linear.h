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

float _linear_interpolate(float _a, float _b, float _position)
{    
    return (((_a - _b) * _position) + _a);
}


#ifdef	__cplusplus
}
#endif

#endif	/* INTERPOLATE_LINEAR_H */

