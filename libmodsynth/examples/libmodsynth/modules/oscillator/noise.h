/* 
 * File:   noise.h
 * Author: vm-user
 *
 * Created on January 10, 2012, 9:04 PM
 */

#ifndef NOISE_H
#define	NOISE_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include <stdlib.h>
#include <time.h>

typedef struct _white_noise
{
    uint array_count, read_head;
    float * sample_array;     
    
}white_noise;

white_noise * _get_white_noise(float);

white_noise * _get_white_noise(float _sample_rate)
{
    srand((unsigned)time(NULL));
    
    white_noise * _result = (white_noise*)malloc(sizeof(white_noise));
    
    _result->array_count = _sample_rate;
    
    _result->read_head = 0;
    
    _result->sample_array = (float*)malloc(sizeof(float) * _sample_rate);
    
    uint i = 0;
    
    uint _i_s_r = (uint)_sample_rate;
    
    while(i < _i_s_r)
    {
        /*Mixing 3 random numbers together gives a more natural sounding white noise,
         instead of a "brick" of noise, as seen on an oscilloscope*/
        float _sample1 = (float)rand() / (float)RAND_MAX;
        float _sample2 = (float)rand() / (float)RAND_MAX;
        float _sample3 = (float)rand() / (float)RAND_MAX;
        
        _result->sample_array[i] = (_sample1 + _sample2 + _sample3) * .33;
        i++;
    }
    
    return _result;
}


float _run_w_noise(white_noise *);

float _run_w_noise(white_noise * _w_noise)
{
    _w_noise->read_head = (_w_noise->read_head) + 1;
    
    if((_w_noise->read_head) >= (_w_noise->array_count))
    {
        _w_noise->read_head = 0;
    }
    
    return _w_noise->sample_array[(_w_noise->read_head)];
}

#ifdef	__cplusplus
}
#endif

#endif	/* NOISE_H */

