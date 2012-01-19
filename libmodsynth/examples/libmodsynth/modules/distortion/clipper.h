/* 
 * File:   clipper.h
 * Author: vm-user
 *
 * Created on January 8, 2012, 1:15 PM
 */

#ifndef CLIPPER_H
#define	CLIPPER_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include "../../lib/amp.h"
    
typedef struct _clipper
{
    float clip_high, clip_low, input_gain, clip_db, in_db;
}clipper;

/*Set the values of a clipper struct symmetrically, ie: value of .75 clips at .75 and -.75*/
void _clp_set_clip_sym(clipper *, float);
void _clp_set_in_gain(clipper *, float);


void _clp_set_clip_sym(clipper * _clp, float _db)
{
    /*Already set, don't set again*/
    if(_db == (_clp->clip_db))
        return;
    
    _clp->clip_db = _db;
    
    float _value = _db_to_linear(_db);
    
#ifdef LMS_DEBUG_MODE
        printf("Clipper value == %f", _value);
#endif

    _clp->clip_high = _value;
    _clp->clip_low = (_value * -1);
}

void _clp_set_in_gain(clipper * _clp, float _db)
{
    if((_clp->in_db) == _db)
        return;
    
    _clp->in_db = _db;
    
    _clp->input_gain = _db_to_linear(_db);
}

clipper * _clp_get_clipper()
{
    clipper * _result = (clipper*)malloc(sizeof(clipper));
    
    _result->clip_high = 1;
    _result->clip_low = -1;
    _result->input_gain = 1;
    
    return _result;
};

float _clp_clip(clipper*, float);

float _clp_clip(clipper * _clp, float _input)
{
    float _result = _input * (_clp->input_gain);
    
    if(_result > (_clp->clip_high))
        _result = (_clp->clip_high);
    else if(_result < (_clp->clip_low))
        _result = (_clp->clip_low);
    
    return _result;
}

#ifdef	__cplusplus
}
#endif

#endif	/* CLIPPER_H */

