/* 
 * File:   lms_math.h
 * Author: Jeff Hubbard
 * 
 * This file is for math operations that are re-implemented in this library for extra speed,
 * in some instances with a lesser, but still acceptable level of accuracy.  Some of these
 * functions may also have a a finite input range.
 *
 * Created on March 11, 2012, 5:36 PM
 */

#ifndef LMS_MATH_H
#define	LMS_MATH_H

#ifdef	__cplusplus
extern "C" {
#endif

inline float f_lms_abs(float);

/* inline float f_lms_abs(float a_input)
 * 
 * Return the absolute value of a float.  Use this instead of fabs, it's much faster
 */
inline float f_lms_abs(float a_input)
{
    if(a_input > 0)
        return a_input;
    else
        return a_input * -1;
}

#ifdef	__cplusplus
}
#endif

#endif	/* LMS_MATH_H */

