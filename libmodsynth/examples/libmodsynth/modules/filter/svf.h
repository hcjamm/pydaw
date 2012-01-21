/* 
 * File:   svf.h
 * Author: vm-user
 *
 * Created on January 8, 2012, 11:35 AM
 */

#ifndef SVF_H
#define	SVF_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "../../lib/pitch_core.h"
#include "../../lib/amp.h"
#include "../../lib/interpolate-linear.h"
#include "../../../constants.h"
#include "../../lib/smoother-linear.h"
#include "../../lib/denormal.h"

typedef struct _state_variable_filter
{
    smoother_linear * cutoff_smoother;
    float _bp_m1, _lp_m1, _cutoff_note, _cutoff_hz, _cutoff_filter, _pi2_div_sr, _sr, _oversample_div, _filter_res, _filter_res_db, _input, _last_input, _hp, _lp, _bp;
    int _oversample_mult;

} state_variable_filter; 


state_variable_filter * _svf_get(float, int);

/*This should be called every sample, otherwise the smoothing doesn't work properly*/
void _svf_set_cutoff(state_variable_filter * _svf, float _midi_note_number)
{
             
    //_sml_run(_svf->cutoff_smoother, _midi_note_number);
    
     /*It hasn't changed since last time, return*/
    
    /*if((_svf->cutoff_smoother->last_value) == _midi_note_number)
        return;  */
    
    
     
    //_svf->_cutoff_note = _midi_note_number;
    _svf->_cutoff_hz = _pit_midi_note_to_hz(_midi_note_number); //_svf->cutoff_smoother->last_value);
    
    if(_svf->_cutoff_hz >= 24000)
        _svf->_cutoff_hz = 24000;

    _svf->_cutoff_filter = _svf->_pi2_div_sr * _svf->_cutoff_hz * _svf->_oversample_div;

    /*prevent the filter from exploding numerically, this does artificially cap the cutoff frequency to below what you set it to
     if you lower the oversampling rate of the filter.*/
    if(_svf->_cutoff_filter > .8)
        _svf->_cutoff_filter = .8;  
}

void _svf_set_res(
    state_variable_filter * _svf,
    float _db  //-100 to 0 is the expected range
    )
{
    /*Don't calculate it again if it hasn't changed*/
    if((_svf->_filter_res_db) == _db)
        return;
    
    _svf->_filter_res_db = _db;
    
    if(_db < -100)
        _db = -100;
    else if (_db > -.5)
    {
        _db = -.5;
    }

       _svf->_filter_res = (1 - _db_to_linear(_db)) * 2;
}

/*instantiate a new pointer to a state variable filter*/
state_variable_filter * _svf_get(float _sample_rate, int _oversample)
{
    state_variable_filter * _svf = (state_variable_filter*)malloc(sizeof(state_variable_filter));
    _svf->_sr = _sample_rate * ((float)_oversample);
    _svf->_pi2_div_sr = (PI2 / (_svf->_sr));
    _svf->_oversample_mult = _oversample;
    _svf->_oversample_div = (1/((float)_svf->_oversample_mult)); 
    _svf->cutoff_smoother = _sml_get_smoother_linear(_sample_rate, 130, 30, 2);
    
    return _svf;
}

//The main action to run the filter
void _svf_set_input_value(state_variable_filter * _svf, float _input_value)
{
    _svf->_input = _input_value;
    float _position = 0;

    int i = 0;
    
    while(i < (_svf->_oversample_mult))
    {
        float _interpolated_sample = _linear_interpolate(_svf->_last_input, _svf->_input, _position);

        _svf->_hp = _interpolated_sample - ((_svf->_bp_m1 * _svf->_filter_res) + _svf->_lp_m1);
        _svf->_bp = (_svf->_hp * _svf->_cutoff_filter) + _svf->_bp_m1;
        _svf->_lp = (_svf->_bp * _svf->_cutoff_filter) + _svf->_lp_m1;

        _svf->_bp_m1 = _remove_denormal((_svf->_bp));
        _svf->_lp_m1 = _remove_denormal((_svf->_lp));

        _position += _svf->_oversample_mult;
        
        i++;
    }
}

#ifdef	__cplusplus
}
#endif

#endif	/* SVF_H */

