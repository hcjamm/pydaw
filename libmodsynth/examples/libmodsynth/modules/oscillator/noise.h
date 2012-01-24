/* 
 * File:   noise.h
 * 
 This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

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
    uint array_count, read_head;
    float * sample_array;     
    
}t_white_noise;

t_white_noise * g_get_white_noise(float);

t_white_noise * g_get_white_noise(float a_sample_rate)
{
    srand((unsigned)time(NULL));
    
    t_white_noise * f_result = (t_white_noise*)malloc(sizeof(t_white_noise));
    
    f_result->array_count = a_sample_rate;
    
    f_result->read_head = 0;
    
    f_result->sample_array = (float*)malloc(sizeof(float) * a_sample_rate);
    
    uint f_i = 0;
    
    uint f_i_s_r = (uint)a_sample_rate;
    
    while(f_i < f_i_s_r)
    {
        /*Mixing 3 random numbers together gives a more natural sounding white noise,
         instead of a "brick" of noise, as seen on an oscilloscope*/
        float _sample1 = ((float)rand() / (float)RAND_MAX) - .5;
        float _sample2 = ((float)rand() / (float)RAND_MAX) - .5;
        float _sample3 = ((float)rand() / (float)RAND_MAX) - .5;
        
        f_result->sample_array[f_i] = (_sample1 + _sample2 + _sample3) * .5;
        f_i++;
    }
    
    return f_result;
}


float f_run_w_noise(t_white_noise *);

float f_run_w_noise(t_white_noise * a_w_noise)
{
    a_w_noise->read_head = (a_w_noise->read_head) + 1;
    
    if((a_w_noise->read_head) >= (a_w_noise->array_count))
    {
        a_w_noise->read_head = 0;
    }
    
    return a_w_noise->sample_array[(a_w_noise->read_head)];
}

#ifdef	__cplusplus
}
#endif

#endif	/* NOISE_H */

