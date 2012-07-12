/* 
 * File:   interpolate-sinc.h
 * Author: jeffh
 *
 * Created on July 10, 2012, 7:16 PM
 */

#ifndef INTERPOLATE_SINC_H
#define	INTERPOLATE_SINC_H

#include <math.h>
#include "interpolate-linear.h"

#ifdef	__cplusplus
extern "C" {
#endif


typedef struct st_sinc_interpolator
{
    float * sinc_table;
    int table_size;
    int points;
    int points_div2;
    int samples_per_point;
    int i;
    float result;
    float float_iterator_up;
    float float_iterator_down;
    int pos_int;
    float pos_frac;
    t_lin_interpolater * linear_interpolate;
}t_sinc_interpolator;

float f_sinc_interpolate(t_sinc_interpolator*,float*,float);
//float f_sinc_interpolate(t_sinc_interpolator*,float,int);

/* float f_sinc_interpolate(
 * t_sinc_interpolator* a_sinc, 
 * float * a_array, //The array to interpolate from
 * float a_pos) //The position in the array to interpolate
 * 
 * This function assumes you added adequate 0.0f's to the beginning and end of the array, it does not check array boundaries
 */
float f_sinc_interpolate(t_sinc_interpolator* a_sinc, float * a_array, float a_pos)
{
    a_sinc->pos_int = (int)a_pos;
    a_sinc->pos_frac = a_pos - ((float)(a_sinc->pos_int));
    a_sinc->float_iterator_up = (a_sinc->pos_frac) * (a_sinc->samples_per_point);
    
    a_sinc->float_iterator_down = (a_sinc->samples_per_point) - (a_sinc->float_iterator_up);    
        
    a_sinc->result = 0.0f;
        
    a_sinc->result +=  
                f_linear_interpolate_ptr_wrap(a_sinc->sinc_table, a_sinc->table_size, (a_sinc->float_iterator_up), a_sinc->linear_interpolate)
                *
                a_array[(a_sinc->pos_int)];
    
    for(a_sinc->i = 1; (a_sinc->i) <= (a_sinc->points_div2); a_sinc->i = (a_sinc->i) + 1)
    {
        a_sinc->result += 
                f_linear_interpolate_ptr_wrap(a_sinc->sinc_table, a_sinc->table_size, a_sinc->float_iterator_down, a_sinc->linear_interpolate)
                *
                a_array[(a_sinc->pos_int) - (a_sinc->i)];
        a_sinc->result +=  
                f_linear_interpolate_ptr_wrap(a_sinc->sinc_table, a_sinc->table_size, (a_sinc->float_iterator_up), a_sinc->linear_interpolate)
                *
                a_array[(a_sinc->pos_int) + (a_sinc->i)];
        
        a_sinc->float_iterator_up += (a_sinc->samples_per_point);
        a_sinc->float_iterator_down += (a_sinc->samples_per_point);
    }
    
    return (a_sinc->result);
    
}

t_sinc_interpolator * g_sinc_get(int, int, double, double);

/* t_sinc_interpolator * g_sinc_get(
 * int a_points, //The number of points to use
 * int a_samples_per_point, //how many array elements per a_point
 * double a_fc,  //cutoff, usually 0.1 to 0.5
 * double a_amp) //Adjusting for the loss of amplitude in the SINC LP.  This is a linear value, normal range:  1.0 to 8.0
 */
t_sinc_interpolator * g_sinc_get(int a_points, int a_samples_per_point, double a_fc, double a_amp)
{
    int f_array_size = a_points * a_samples_per_point;
    
    t_sinc_interpolator * f_result = (t_sinc_interpolator*)malloc(sizeof(t_sinc_interpolator));
    f_result->sinc_table = (float*)malloc(sizeof(float) * f_array_size);

    f_result->points = a_points;
    f_result->samples_per_point = a_samples_per_point;
    f_result->table_size = f_array_size;
    f_result->points_div2 = (((a_points) + 1) / 2);
    
    double f_points = (double)a_points;
    
    double f_inc = (1.0f/((double)f_array_size)) * f_points;

    double i;
    int i_int = 0;
    
    double pi = 3.141592;
    
    double f_i_test = 0;
    
    for(i = (f_points*0.5); i < f_points; i+=f_inc)
    {
        double f_sinc1 = sin((pi*a_fc*i));
        double sinclp = (f_sinc1/(pi*i)) * a_amp;
        
        double f_bm1 = (f_points)+(i);

        double f_bm2 = 0.42659 - (0.49656 *(cos((2*pi*i)/(f_points - 1))));
        double f_bm3 = (cos((12.5664 * i)/(f_points - 1) ) * .076849) + f_bm2;
        float f_out = sinclp * f_bm3;
        
        f_result->sinc_table[i_int] = f_out;
        i_int++;
    }
    
    f_result->linear_interpolate = g_lin_get();
    
    return f_result;

}




#ifdef	__cplusplus
}
#endif

#endif	/* INTERPOLATE_SINC_H */

