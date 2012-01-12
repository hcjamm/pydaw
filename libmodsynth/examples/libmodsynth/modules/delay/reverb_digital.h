/* 
 * File:   reverb_digital.h
 * Author: vm-user
 *
 * Created on January 8, 2012, 5:48 PM
 */

#ifndef REVERB_DIGITAL_H
#define	REVERB_DIGITAL_H

#ifdef	__cplusplus
extern "C" {
#endif

const int REVERB_DIGITAL_BUFFER = 300000;
    
typedef struct _reverb_digital{
    float input_buffer [REVERB_DIGITAL_BUFFER];
    int buffer_pos;
    int comb_filters;
    float feedback;
    int spread;
}reverb_digital;

reverb_digital * _rvd_get_reverb_digital();
float _rvd_run(reverb_digital *);

reverb_digital * _rvd_get_reverb_digital()
{
    reverb_digital * _result = (reverb_digital *)malloc(sizeof(reverb_digital));
    
    _result->buffer_pos = 0;
    _result->comb_filters = 7;
    _result->feedback = .6;
    _result->spread = 70;  //TODO:  align this with a pitch value
    
    return _result;
}

float _rvd_run(reverb_digital * _rvd)
{
    float _result = 0;
    
    return _result;
}

#ifdef	__cplusplus
}
#endif

#endif	/* REVERB_DIGITAL_H */

