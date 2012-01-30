/* 
 * File:   sample_reader.h
 * Author: vm-user
 *
 * Created on January 26, 2012, 7:20 PM
 */

#ifndef SAMPLE_READER_H
#define	SAMPLE_READER_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "interpolate-linear.h"

typedef struct st_sample_reader
{
    int samples_to_read;
    int sample_offset;  //usually a negative number
}t_sample_reader;

inline void v_smr_run(t_sample_reader*,float[],int,float,int,float[],int);
t_sample_reader * g_smr_sample_reader(int);

/*Perform sinc interpolation*/
inline void v_smr_run(t_sample_reader* a_smr_ptr, 
        float a_sample [], int a_int_sample_ptr, 
        float a_float_sample_ptr, int a_sample_length,
        float a_sinc_impulse [], int a_samples_per_point)
{
    int i = 0;
    
    float f_result = 0;
    
    float f_sinc_fraction = a_samples_per_point * a_float_sample_ptr;
    
    while(i < (a_smr_ptr->samples_to_read))
    {
        //calculate sinc
        int f_sample_pos = (a_smr_ptr->sample_offset) + a_int_sample_ptr + i;
        
        float f_sinc_pos = f_sinc_fraction + (a_samples_per_point * i);
                
        float f_sinc_coefficient = f_linear_interpolate(a_sinc_impulse, f_sinc_pos);
        
        /*TODO: create a choice of whether to wrap or not.*/
        if(f_sample_pos < 0)
        {
            f_result += a_sample[f_sample_pos + a_sample_length] * f_sinc_coefficient;
        }
        else if(f_sample_pos > a_sample_length)
        {
            f_result  += a_sample[f_sample_pos - a_sample_length] * f_sinc_coefficient;
        }
        else
        {
            f_result  += a_sample[f_sample_pos] * f_sinc_coefficient;
        }
        
        i++;
    }
    
    return f_result;
        
}

t_sample_reader * g_smr_sample_reader(int a_samples)
{
    t_sample_reader f_result = (t_sample_reader*)malloc(sizeof(t_sample_reader));
    
    f_result->samples_to_read = a_samples;
    
    if (a_samples % 2) /* a_samples is odd */ 
    { 
        f_result->sample_offset = ((a_samples - 1) /2) * -1;
    }
    else /* a_samples is even */ 
    {
        f_result->sample_offset = (a_samples/2) * -1;
    }
    
    return f_result;
}




#ifdef	__cplusplus
}
#endif

#endif	/* SAMPLE_READER_H */

