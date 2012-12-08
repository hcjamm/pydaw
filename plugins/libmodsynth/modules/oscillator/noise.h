/* 
 * File:   noise.h
 * Author: Jeff Hubbard
 * 
 * This file provides t_white_noise, a white noise oscillator.
 * 
 * There is also a function to return pink noise from white noise.
 * 
 */

#ifndef NOISE_H
#define	NOISE_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include <stdlib.h>
#include <time.h>
typedef struct st_white_noise
{
    int array_count, read_head;
    float * sample_array;        
    float b0,b1,b2,b3,b4,b5,b6;  //pink noise coefficients
}t_white_noise;

typedef float (*fp_noise_func_ptr)(t_white_noise*);

t_white_noise * g_get_white_noise(float);
inline float f_run_white_noise(t_white_noise *);
inline float f_run_pink_noise(t_white_noise *);
inline float f_run_noise_off(t_white_noise *);
inline fp_noise_func_ptr fp_get_noise_func_ptr(int);

inline fp_noise_func_ptr fp_get_noise_func_ptr(int a_index)
{
    switch(a_index)
    {
        case 0:
            return f_run_noise_off;
        case 1:
            return f_run_white_noise;
        case 2:
            return f_run_pink_noise;
        default:
            return f_run_noise_off;
    }    
}

static unsigned int seed_helper = 18;

/* t_white_noise * g_get_white_noise(float a_sample_rate)
 */
t_white_noise * g_get_white_noise(float a_sample_rate)
{
    time_t f_clock = time(NULL);
    srand(((unsigned)f_clock) + (seed_helper));
    
    seed_helper *= 7;
    
    t_white_noise * f_result = (t_white_noise*)malloc(sizeof(t_white_noise));
    
    f_result->array_count = a_sample_rate;
    
    f_result->read_head = 0;
    
    f_result->sample_array = (float*)malloc(sizeof(float) * a_sample_rate);
    
    int f_i = 0;
    
    int f_i_s_r = (int)a_sample_rate;
    
    while(f_i < f_i_s_r)
    {
        /*Mixing 3 random numbers together gives a more natural sounding white noise,
         instead of a "brick" of noise, as seen on an oscilloscope*/
        float _sample1 = ((double)rand() / (double)RAND_MAX) - .5f;
        float _sample2 = ((double)rand() / (double)RAND_MAX) - .5f;
        float _sample3 = ((double)rand() / (double)RAND_MAX) - .5f;
        
        f_result->sample_array[f_i] = (_sample1 + _sample2 + _sample3) * .5f;
        f_i++;
    }
    
    return f_result;
}

/* inline float f_run_white_noise(t_white_noise * a_w_noise)
 * 
 * returns a single sample of white noise
 */
inline float f_run_white_noise(t_white_noise * a_w_noise)
{
    a_w_noise->read_head = (a_w_noise->read_head) + 1;
    
    if((a_w_noise->read_head) >= (a_w_noise->array_count))
    {
        a_w_noise->read_head = 0;
    }
    
    return a_w_noise->sample_array[(a_w_noise->read_head)];
}

/* inline float f_run_pink_noise(t_white_noise * a_w_noise)
 * 
 * returns a single sample of pink noise
 */
inline float f_run_pink_noise(t_white_noise * a_w_noise)
{
    a_w_noise->read_head = (a_w_noise->read_head) + 1;
      
    if((a_w_noise->read_head) >= (a_w_noise->array_count))
    {
        a_w_noise->read_head = 0;
    }

    float f_white = a_w_noise->sample_array[(a_w_noise->read_head)];

    (a_w_noise->b0) = 0.99886f * (a_w_noise->b0) + f_white * 0.0555179f;
    (a_w_noise->b1) = 0.99332f * (a_w_noise->b1) + f_white * 0.0750759f;
    (a_w_noise->b2) = 0.96900f * (a_w_noise->b2) + f_white * 0.1538520f;
    (a_w_noise->b3) = 0.86650f * (a_w_noise->b3) + f_white * 0.3104856f;
    (a_w_noise->b4) = 0.55000f * (a_w_noise->b4) + f_white * 0.5329522f;
    (a_w_noise->b5) = -0.7616f * (a_w_noise->b5) - f_white * 0.0168980f;
    (a_w_noise->b6) = f_white * 0.115926f;
    return (a_w_noise->b0) + (a_w_noise->b1) + (a_w_noise->b2) + (a_w_noise->b3) + 
            (a_w_noise->b4) + (a_w_noise->b5) + (a_w_noise->b6) + f_white * 0.5362f;      
}

inline float f_run_noise_off(t_white_noise * a_w_noise)
{
    return 0.0f;
}


#ifdef	__cplusplus
}
#endif

#endif	/* NOISE_H */

