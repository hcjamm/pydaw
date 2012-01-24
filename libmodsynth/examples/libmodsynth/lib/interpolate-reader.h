/* 
 * File:   interpolate-reader.h
 * Author: vm-user
 *
 * Created on January 10, 2012, 7:29 PM
 */

#ifndef INTERPOLATE_READER_H
#define	INTERPOLATE_READER_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "pitch_core.h"
    
/*This gives us full numerical accuracy throughout an entire file, rather than losing accuracy over the course of longer files*/
    
typedef struct _interpolate_reader
{
    uint sample_count;
    uint samples_to_return_half; //how many samples to read in each direction.  For example, a 15 point interpolator should be 7, -7,0,+7;  
    uint samples_to_return;  //the total number of samples, it is done this way to prevent an even number of samples being selected by the user
    float samples [];
    float midi_note;
    float hz;
    float original_pitch;
    float f_inc;
    uint i_inc;
    
    float f_pos;
    uint i_pos;
    
    float increment;
    
}interpolate_reader;

interpolate_reader * _inr_get_interpolate_reader();

void _inr_set_pitch_sampler(interpolate_reader *, float);

void _inr_set_pitch_osc(interpolate_reader *, float);

void _inr_set_orig_pitch(interpolate_reader *, float);

void _inr_run(interpolate_reader);

float * _inr_run_osc(interpolate_reader *);

float * _inr_run_sampler(interpolate_reader *);

void _inr_set_orig_pitch(interpolate_reader * _reader, float _note)
{
    _reader->midi_note = _note;
    _reader->hz = f_pit_midi_note_to_hz(_note);
}

interpolate_reader * _inr_get_interpolate_reader()
{
    interpolate_reader * _result = (interpolate_reader*)malloc(sizeof(interpolate_reader));
    
    _result->i_pos = 0;
    _result->f_pos = 0;
    
    _inr_set_orig_pitch(_result, 60); //This is to prevent any disasters later if the user doesn't initialize it correctly
    
    _result->f_inc = 0;
    _result->i_inc = 1;
    
    return _result;
}

/*Increments the reader*/
void _inr_run(interpolate_reader * _reader)
{
    _reader->i_pos = (_reader->i_pos) + (_reader->i_inc);
    _reader->f_pos = (_reader->f_pos) + (_reader->f_inc);
    
    if((_reader->f_pos) >= 1)
    {
        _reader->f_pos = (_reader->f_pos) - 1;
        _reader->i_pos = (_reader->i_pos) + 1;
    }
    
    if((_reader->i_pos) >= (_reader->sample_count))
    {
        _reader->i_pos = (_reader->i_pos) - (_reader->sample_count);
    }
}

/*Returns an array suitable for oscillator output*/
float * _inr_run_osc(interpolate_reader * _reader)
{
    _inr_run(_reader);
    
    
}

/*Returns an array suitable for sampler output*/
float * _inr_run_sampler(interpolate_reader * _reader)
{
    _inr_run(_reader);
}

float _inr_get_samples(float _array_source [], int _source_count, float * _array_result [], int _pos, int _offset)
{
    
}

#ifdef	__cplusplus
}
#endif

#endif	/* INTERPOLATE_READER_H */

