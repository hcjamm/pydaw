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
    int samples_per_point;
    t_lin_interpolater * linear_interpolate;
}t_sinc_interpolator;

float f_sinc_interpolate(t_sinc_interpolator*,float*,float);
//float f_sinc_interpolate(t_sinc_interpolator*,float,int);

float f_sinc_interpolate(t_sinc_interpolator* a_sinc, float * a_array, float a_pos)
{
    int f_pos_int = (int)a_pos;
    float f_pos_frac = a_pos - ((float)f_pos_int);
    float f_sinc_pos_plus = f_pos_frac * (a_sinc->samples_per_point);
    
    float f_float_iterator_down = (a_sinc->samples_per_point) - f_sinc_pos_plus;    
    float f_float_iterator_up = f_sinc_pos_plus;
    
    int f_i;
    float f_result = 0.0f;
        
    f_result +=  
                f_linear_interpolate_ptr_wrap(a_sinc->sinc_table, a_sinc->table_size, (f_float_iterator_up), a_sinc->linear_interpolate)
                *
                a_array[f_pos_int];
    
    for(f_i = 1; f_i <= (((a_sinc->points) -1) / 2); f_i++)
    {
        f_result += 
                f_linear_interpolate_ptr_wrap(a_sinc->sinc_table, a_sinc->table_size, f_float_iterator_down, a_sinc->linear_interpolate)
                *
                a_array[f_pos_int - f_i];
        f_result +=  
                f_linear_interpolate_ptr_wrap(a_sinc->sinc_table, a_sinc->table_size, (f_float_iterator_up), a_sinc->linear_interpolate)
                *
                a_array[f_pos_int + f_i];
        
        f_float_iterator_up += f_sinc_pos_plus;
        f_float_iterator_down += f_sinc_pos_plus;
    }
    
    return f_result;
    
}

t_sinc_interpolator * g_sinc_get(int, int, double);

t_sinc_interpolator * g_sinc_get(int a_points, int a_samples_per_point, double a_fc)
{
    int f_array_size = a_points * a_samples_per_point;
    
    t_sinc_interpolator * f_result = (t_sinc_interpolator*)malloc(sizeof(t_sinc_interpolator));
    f_result->sinc_table = (float*)malloc(sizeof(float) * f_array_size);

    f_result->points = a_points;
    f_result->samples_per_point = a_samples_per_point;
    f_result->table_size = f_array_size;
    
    double f_points = (double)a_points;
    
    double f_inc = (1.0f/((double)f_array_size)) * f_points;

    double i;
    int i_int = 0;
    
    double pi = 3.141592;
    
    for(i = f_inc; i < f_points; i+=f_inc)
    {
        double f_sinc1 = sin((2*pi*a_fc)*i);
        double sinclp = f_sinc1/(pi*i);
        
        double f_bm1 = (0.5*f_points)+(i*0.5);

        double f_bm2 = 0.42 - ( cos((2*pi)/f_points) * 0.5);
        double f_bm3 = (cos((12.5664 * f_bm1)/f_points) * .08) + f_bm2;
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

