/*
This file is part of the PyDAW project, Copyright PyDAW Team

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef COMB_FILTER_H
#define	COMB_FILTER_H


#ifdef	__cplusplus
extern "C" {
#endif


#include "../../lib/amp.h"
#include "../../lib/pitch_core.h"
#include "../../lib/interpolate-linear.h"
#include "../../lib/smoother-linear.h"
#include "../../lib/denormal.h"

#define MC_CMB_COUNT 4

typedef struct st_comb_filter
{
    int buffer_size;  //The size of input_buffer
    int input_pointer;  //The index where the current sample is written to
    float delay_pointer; //
    float wet_sample;
    float output_sample;
    float wet_db;
    float wet_linear;
    float feedback_db;
    float feedback_linear;
    float midi_note_number;
    float delay_samples;  //How many samples, including the fractional part, to delay the signal
    float sr;
    float * input_buffer;
    t_lin_interpolater * linear;
    t_pit_pitch_core * pitch_core;
    t_amp * amp_ptr;
    int mc_delay_samples[MC_CMB_COUNT];
    float mc_detune;

}t_comb_filter;

inline void v_cmb_run(t_comb_filter*,float);
inline void v_cmb_set_all(t_comb_filter*, float,float,float);
t_comb_filter * g_cmb_get_comb_filter(float);
void v_cmb_free(t_comb_filter *);

/* v_cmb_run(
 * t_comb_filter*,
 * float input value (audio sample, -1 to 1, typically)
 * );
 * This runs the filter.  You can then use the output sample in your plugin*/
inline void v_cmb_run(t_comb_filter*__restrict a_cmb_ptr, float a_value)
{
    a_cmb_ptr->delay_pointer = (a_cmb_ptr->input_pointer) - (a_cmb_ptr->delay_samples);

    if((a_cmb_ptr->delay_pointer) < 0.0f)
    {
        a_cmb_ptr->delay_pointer = (a_cmb_ptr->delay_pointer) + (a_cmb_ptr->buffer_size);
    }

    a_cmb_ptr->wet_sample = (f_linear_interpolate_arr_wrap(a_cmb_ptr->input_buffer,
            (a_cmb_ptr->buffer_size), (a_cmb_ptr->delay_pointer), a_cmb_ptr->linear));

    a_cmb_ptr->input_buffer[(a_cmb_ptr->input_pointer)] = f_remove_denormal(a_value + ((a_cmb_ptr->wet_sample) * (a_cmb_ptr->feedback_linear)));

    if((a_cmb_ptr->wet_db) <= -20.0f)
    {
        a_cmb_ptr->output_sample = a_value;
    }
    else
    {
        a_cmb_ptr->output_sample = (a_value + ((a_cmb_ptr->wet_sample) * (a_cmb_ptr->wet_linear)));
    }


    a_cmb_ptr->input_pointer = (a_cmb_ptr->input_pointer) + 1;

    if((a_cmb_ptr->input_pointer) >= (a_cmb_ptr->buffer_size))
    {
        a_cmb_ptr->input_pointer = 0;
    }
}

inline void v_cmb_mc_run(t_comb_filter*__restrict a_cmb_ptr, float a_value)
{
    a_cmb_ptr->input_buffer[(a_cmb_ptr->input_pointer)] = a_value;
    a_cmb_ptr->output_sample = a_value;

    if((a_cmb_ptr->wet_db) > -20.0f)
    {
        int f_i = 0;
        while(f_i < MC_CMB_COUNT)
        {
            a_cmb_ptr->delay_pointer = (a_cmb_ptr->input_pointer) - (a_cmb_ptr->mc_delay_samples[f_i]);

            if((a_cmb_ptr->delay_pointer) < 0.0f)
            {
                a_cmb_ptr->delay_pointer = (a_cmb_ptr->delay_pointer) + (a_cmb_ptr->buffer_size);
            }

            a_cmb_ptr->wet_sample = (f_linear_interpolate_arr_wrap(a_cmb_ptr->input_buffer,
                    (a_cmb_ptr->buffer_size), (a_cmb_ptr->delay_pointer), a_cmb_ptr->linear));

            a_cmb_ptr->input_buffer[(a_cmb_ptr->input_pointer)] +=
                    ((a_cmb_ptr->wet_sample) * (a_cmb_ptr->feedback_linear));

            a_cmb_ptr->output_sample += ((a_cmb_ptr->wet_sample) * (a_cmb_ptr->wet_linear));

            f_i++;
        }
    }

    a_cmb_ptr->input_pointer = (a_cmb_ptr->input_pointer) + 1;

    if((a_cmb_ptr->input_pointer) >= (a_cmb_ptr->buffer_size))
    {
        a_cmb_ptr->input_pointer = 0;
    }

    a_cmb_ptr->input_buffer[(a_cmb_ptr->input_pointer)] =
            f_remove_denormal(a_cmb_ptr->input_buffer[(a_cmb_ptr->input_pointer)]);
}

/*v_cmb_set_all(
 * t_comb_filter*,
 * float wet (decibels -20 to 0)
 * float feedback (decibels -20 to 0)
 * float pitch (midi note number, 20 to 120)
 * );
 *
 * Sets all parameters of the comb filter.
 */
inline void v_cmb_set_all(t_comb_filter*__restrict a_cmb_ptr, float a_wet_db, float a_feedback_db, float a_midi_note_number)
{
    /*Set wet_linear, but only if it's changed since last time*/
    if((a_cmb_ptr->wet_db) != a_wet_db)
    {
        a_cmb_ptr->wet_db = a_wet_db;
        a_cmb_ptr->wet_linear = f_db_to_linear_fast(a_wet_db, a_cmb_ptr->amp_ptr);
    }

    /*Set feedback_linear, but only if it's changed since last time*/
    if((a_cmb_ptr->feedback_db) != a_feedback_db)
    {
        if(a_feedback_db > -0.1f)
        {
            a_cmb_ptr->feedback_db = -0.1f;
        }
        else
        {
            a_cmb_ptr->feedback_db = a_feedback_db;
        }

        a_cmb_ptr->feedback_linear = f_db_to_linear_fast((a_cmb_ptr->feedback_db), a_cmb_ptr->amp_ptr); // * -1;  //negative feedback, gives a comb-ier sound
    }

    if((a_cmb_ptr->midi_note_number) != a_midi_note_number)
    {
        a_cmb_ptr->midi_note_number = a_midi_note_number;
        a_cmb_ptr->delay_samples = f_pit_midi_note_to_samples(a_midi_note_number, (a_cmb_ptr->sr), a_cmb_ptr->pitch_core);
    }

}

inline void v_cmb_mc_set_all(t_comb_filter*__restrict a_cmb, float a_wet_db, float a_midi_note_number, float a_detune)
{
    if(a_cmb->mc_detune != a_detune || (a_cmb->midi_note_number) != a_midi_note_number)
    {
        a_cmb->mc_detune = a_detune;
        int f_i = 0;

        while(f_i < MC_CMB_COUNT)
        {
            a_cmb->mc_delay_samples[f_i] = f_pit_midi_note_to_samples(
                    a_midi_note_number + (a_detune * (float)f_i),
                    (a_cmb->sr), a_cmb->pitch_core);
            f_i++;
        }
    }

    v_cmb_set_all(a_cmb, a_wet_db, a_wet_db - 13.0f, a_midi_note_number);
}

/* t_comb_filter * g_cmb_get_comb_filter(
 * float a_sr) //sample rate
 */
t_comb_filter * g_cmb_get_comb_filter(float a_sr)
{
    t_comb_filter * f_result;// = (t_comb_filter*)malloc(sizeof(t_comb_filter));

    if(posix_memalign((void**)&f_result, 16, (sizeof(t_comb_filter))) != 0)
    {
        return 0;
    }

    int f_i = 0;

    f_result->buffer_size = (int)((a_sr / 20.0f) + 300);  //Allocate enough memory to accomodate 20hz filter frequency

    if(posix_memalign((void**)(&(f_result->input_buffer)), 16, (sizeof(float) * (f_result->buffer_size))) != 0)
    {
        return 0;
    }

    while(f_i < (f_result->buffer_size))
    {
        f_result->input_buffer[f_i] = 0.0f;
        f_i++;
    }

    f_result->input_pointer = 0;
    f_result->delay_pointer = 0;
    f_result->wet_sample = 0.0f;
    f_result->output_sample = 0.0f;
    f_result->wet_db = -1.0f;
    f_result->wet_linear = .75f;
    f_result->feedback_db = -6.0f;
    f_result->feedback_linear = 0.5f;
    f_result->midi_note_number = 60.0f;
    f_result->delay_samples = 150.0f;
    f_result->sr = a_sr;
    f_result->linear = g_lin_get();
    f_result->pitch_core = g_pit_get();
    f_result->amp_ptr = g_amp_get();
    f_result->mc_detune = 999.999f;


    v_cmb_set_all(f_result,-6.0f, -6.0f, 66.0f);
    v_cmb_run(f_result, 0.0f);

    return f_result;
}


void v_cmb_free(t_comb_filter * a_cmb)
{
    v_amp_free(a_cmb->amp_ptr);
    free(a_cmb->linear);
    free(a_cmb->input_buffer);
    v_pit_free(a_cmb->pitch_core);
    free(a_cmb);
}

#ifdef	__cplusplus
}
#endif


#endif	/* COMB_FILTER_H */

