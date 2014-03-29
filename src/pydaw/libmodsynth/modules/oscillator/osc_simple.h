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

#ifndef OSC_SIMPLE_H
#define	OSC_SIMPLE_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "../../lib/osc_core.h"
#include "../../constants.h"
#include "../../lib/pitch_core.h"
#include "../../lib/fast_sine.h"


#define OSC_UNISON_MAX_VOICES 7

//Used to switch between values, uses much less CPU than a switch statement
typedef float (*fp_get_osc_func_ptr)(t_osc_core*);

typedef struct
{
    float sr_recip;
    //Set this to the max number of voices, not to exceed OSC_UNISON_MAX_VOICES
    int voice_count;
    fp_get_osc_func_ptr osc_type;
    float bottom_pitch;
    float pitch_inc;
    float voice_inc [OSC_UNISON_MAX_VOICES];
    t_osc_core * osc_cores [OSC_UNISON_MAX_VOICES];
    //Restart the oscillators at the same phase on each note-on
    float phases [OSC_UNISON_MAX_VOICES];
    float uni_spread;
    //Set this with unison voices to prevent excessive volume
    float adjusted_amp;
    int i_run_unison;  //iterator for running the unison oscillator
    float current_sample;  //current output sample for the entire oscillator
    int i_unison_pitch;
    int i_sync_phase;
    t_pit_pitch_core * pitch_core;
    int is_resetting;  //For oscillator sync
}t_osc_simple_unison;


inline void v_osc_set_uni_voice_count(t_osc_simple_unison*, int);
inline void v_osc_set_unison_pitch(t_osc_simple_unison * a_osc_ptr,
        float a_spread, float a_pitch);
inline float f_osc_run_unison_osc(t_osc_simple_unison * a_osc_ptr);
inline float f_get_saw(t_osc_core *);
inline float f_get_sine(t_osc_core *);
inline float f_get_square(t_osc_core *);
inline float f_get_triangle(t_osc_core *);
inline float f_get_osc_off(t_osc_core *);
inline void v_osc_set_simple_osc_unison_type(t_osc_simple_unison *, int);
inline void v_osc_note_on_sync_phases(t_osc_simple_unison *);
t_osc_simple_unison * g_osc_get_osc_simple_unison(float);


/* void v_osc_set_uni_voice_count(
 * t_osc_simple_unison* a_osc_ptr,
 * int a_value) //the number of unison voices this oscillator should use
 */
inline void v_osc_set_uni_voice_count(t_osc_simple_unison* a_osc_ptr,
        int a_value)
{
    if(a_value > (OSC_UNISON_MAX_VOICES))
    {
        a_osc_ptr->voice_count = OSC_UNISON_MAX_VOICES;
    }
    else if(a_value < 1)
    {
        a_osc_ptr->voice_count = 1;
    }
    else
    {
        a_osc_ptr->voice_count = a_value;
    }

    a_osc_ptr->adjusted_amp = (1.0f / (float)(a_osc_ptr->voice_count)) +
            (((float)(a_osc_ptr->voice_count - 1)) * .06f);
}


/* void v_osc_set_unison_pitch(
 * t_osc_simple_unison * a_osc_ptr,
 * float a_spread, //the total spread of the unison pitches, the distance in
 *                 //semitones from bottom pitch to top.  Typically .1 to .5
 * float a_pitch)  //the pitch of the oscillator in MIDI note number,
 *                 //typically 32 to 70
 */
inline void v_osc_set_unison_pitch(t_osc_simple_unison * a_osc_ptr,
        float a_spread, float a_pitch)
{
    if((a_osc_ptr->voice_count) == 1)
    {
        a_osc_ptr->voice_inc[0] =
                f_pit_midi_note_to_hz_fast(a_pitch, a_osc_ptr->pitch_core) *
                a_osc_ptr->sr_recip;
    }
    else
    {
        if(a_spread != (a_osc_ptr->uni_spread))
        {
            a_osc_ptr->uni_spread = a_spread;
            a_osc_ptr->bottom_pitch = -0.5f * a_spread;
            a_osc_ptr->pitch_inc = a_spread / ((float)(a_osc_ptr->voice_count));
        }

        a_osc_ptr->i_unison_pitch = 0;

        while((a_osc_ptr->i_unison_pitch) < (a_osc_ptr->voice_count))
        {
            a_osc_ptr->voice_inc[(a_osc_ptr->i_unison_pitch)] =
                    f_pit_midi_note_to_hz_fast(
                    (a_pitch + (a_osc_ptr->bottom_pitch) +
                    (a_osc_ptr->pitch_inc *
                    ((float)(a_osc_ptr->i_unison_pitch)))),
                    a_osc_ptr->pitch_core) * (a_osc_ptr->sr_recip);
            a_osc_ptr->i_unison_pitch = (a_osc_ptr->i_unison_pitch) + 1;
        }
    }

}

/* float f_osc_run_unison_osc(t_osc_simple_unison * a_osc_ptr)
 *
 * Returns one sample of an oscillator's output
 */
inline float f_osc_run_unison_osc(t_osc_simple_unison * a_osc_ptr)
{
    a_osc_ptr->i_run_unison = 0;
    a_osc_ptr->current_sample = 0.0f;

    while((a_osc_ptr->i_run_unison) < (a_osc_ptr->voice_count))
    {
        v_run_osc((a_osc_ptr->osc_cores[(a_osc_ptr->i_run_unison)]),
                (a_osc_ptr->voice_inc[(a_osc_ptr->i_run_unison)]));
        a_osc_ptr->current_sample = (a_osc_ptr->current_sample) +
                a_osc_ptr->osc_type(
                (a_osc_ptr->osc_cores[(a_osc_ptr->i_run_unison)]));
        a_osc_ptr->i_run_unison = (a_osc_ptr->i_run_unison) + 1;
    }

    return (a_osc_ptr->current_sample) * (a_osc_ptr->adjusted_amp);
}

/* float f_osc_run_unison_osc(t_osc_simple_unison * a_osc_ptr)
 *
 * For warming up oscillators
 */
inline void f_osc_run_unison_osc_core_only(t_osc_simple_unison * a_osc_ptr)
{
    a_osc_ptr->i_run_unison = 0;
    a_osc_ptr->current_sample = 0.0f;

    while((a_osc_ptr->i_run_unison) < (a_osc_ptr->voice_count))
    {
        v_run_osc((a_osc_ptr->osc_cores[(a_osc_ptr->i_run_unison)]),
                (a_osc_ptr->voice_inc[(a_osc_ptr->i_run_unison)]));
        a_osc_ptr->i_run_unison = (a_osc_ptr->i_run_unison) + 1;
    }
}

inline float f_osc_run_unison_osc_sync(t_osc_simple_unison * a_osc_ptr)
{
    a_osc_ptr->i_run_unison = 0;
    a_osc_ptr->current_sample = 0.0f;

    a_osc_ptr->is_resetting =
            v_run_osc_sync((a_osc_ptr->osc_cores[(a_osc_ptr->i_run_unison)]),
            (a_osc_ptr->voice_inc[(a_osc_ptr->i_run_unison)]));
    a_osc_ptr->current_sample =
            (a_osc_ptr->current_sample) +
            a_osc_ptr->osc_type(
            (a_osc_ptr->osc_cores[(a_osc_ptr->i_run_unison)]));

    return (a_osc_ptr->current_sample) * (a_osc_ptr->adjusted_amp);
}

inline float f_get_saw(t_osc_core * a_core)
{
    return (((a_core->output) * 2.0f) - 1.0f);
}

inline float f_get_sine(t_osc_core * a_core)
{
    //return sin((a_core->output) * PI2);
    return f_sine_fast_run((a_core->output), a_core->linear);
}

inline float f_get_square(t_osc_core * a_core)
{
    if((a_core->output) >= 0.5f)
    {
        return 1.0f;
    }
    else
    {
        return -1.0f;
    }
}

inline float f_get_triangle(t_osc_core * a_core)
{
    float f_ramp = ((a_core->output) * 4.0f) - 2.0f;
    if(f_ramp > 1.0f)
    {
        return 2.0f - f_ramp;
    }
    else if(f_ramp < -1.0f)
    {
        return (2.0f + f_ramp) * -1.0f;
    }
    else
    {
        return f_ramp;
    }
}


//Return zero if the oscillator is turned off.
inline float f_get_osc_off(t_osc_core * a_core)
{
    return 0.0f;
}

fp_get_osc_func_ptr SIMPLE_OSC_TYPES[] =
{
    f_get_saw, f_get_square, f_get_triangle, f_get_sine, f_get_osc_off
};

/* void v_osc_set_simple_osc_unison_type(
 * t_osc_simple_unison * a_osc_ptr, int a_index)
 *
 * Set the oscillator type.  Current valid types are:
 * 0. Saw
 * 1. Square
 * 2. Triangle
 * 3. Sine
 * 4. Off
 */
inline void v_osc_set_simple_osc_unison_type(
        t_osc_simple_unison * a_osc_ptr, int a_index)
{
    a_osc_ptr->osc_type = SIMPLE_OSC_TYPES[a_index];
}


/*Resync the oscillators at note_on to hopefully avoid phasing artifacts*/
inline void v_osc_note_on_sync_phases(t_osc_simple_unison * a_osc_ptr)
{
    a_osc_ptr->i_sync_phase = 0;

    while((a_osc_ptr->i_sync_phase) < (a_osc_ptr->voice_count))
    {
        a_osc_ptr->osc_cores[(a_osc_ptr->i_sync_phase)]->output =
                a_osc_ptr->phases[(a_osc_ptr->i_sync_phase)];

        a_osc_ptr->i_sync_phase = (a_osc_ptr->i_sync_phase) + 1;
    }
}

inline void v_osc_note_on_sync_phases_hard(t_osc_simple_unison * a_osc_ptr)
{
    a_osc_ptr->i_sync_phase = 0;

    while((a_osc_ptr->i_sync_phase) < (a_osc_ptr->voice_count))
    {
        a_osc_ptr->osc_cores[(a_osc_ptr->i_sync_phase)]->output = 0.0f;

        a_osc_ptr->i_sync_phase = (a_osc_ptr->i_sync_phase) + 1;
    }
}

/* t_osc_simple_unison * g_osc_get_osc_simple_unison(float a_sample_rate)
 */
t_osc_simple_unison * g_osc_get_osc_simple_unison(float a_sample_rate)
{
    t_osc_simple_unison * f_result;

    if(posix_memalign((void**)&f_result, 16, (sizeof(t_osc_simple_unison)))
            != 0)
    {
        return 0;
    }

    v_osc_set_uni_voice_count(f_result, OSC_UNISON_MAX_VOICES);
    f_result->osc_type = f_get_saw;
    f_result->sr_recip = 1.0 / a_sample_rate;
    f_result->adjusted_amp = 1.0;
    f_result->bottom_pitch = -0.1;
    f_result->current_sample = 0.0;
    f_result->i_run_unison = 0;
    f_result->osc_type = f_get_osc_off;
    f_result->pitch_inc = 0.1f;
    f_result->uni_spread = 0.1f;
    f_result->voice_count = 1;
    f_result->i_unison_pitch = 0;
    f_result->i_sync_phase = 0;
    f_result->is_resetting = 0;

    f_result->pitch_core = g_pit_get();

    int f_i = 0;

    while(f_i < (OSC_UNISON_MAX_VOICES))
    {
        f_result->osc_cores[f_i] =  g_get_osc_core();
        f_i++;
    }

    v_osc_set_unison_pitch(f_result, .5f, 60.0f);

    f_i = 0;

    //Prevent phasing artifacts from the oscillators starting at the same phase.
    while(f_i < 200000)
    {
        f_osc_run_unison_osc(f_result);
        f_i++;
    }

    f_i = 0;

    while(f_i < (OSC_UNISON_MAX_VOICES))
    {
        f_result->phases[f_i] = f_result->osc_cores[f_i]->output;
        f_i++;
    }

    v_osc_set_unison_pitch(f_result, .2, 60.0f);

    return f_result;
}


t_osc_simple_unison * g_osc_get_osc_simple_single(float a_sample_rate)
{
    t_osc_simple_unison * f_result;

    if(posix_memalign((void**)&f_result, 16, (sizeof(t_osc_simple_unison)))
            != 0)
    {
        return 0;
    }

    v_osc_set_uni_voice_count(f_result, 1);
    f_result->osc_type = f_get_saw;
    f_result->sr_recip = 1.0 / a_sample_rate;
    f_result->adjusted_amp = 1.0;
    f_result->bottom_pitch = -0.1;
    f_result->current_sample = 0.0;
    f_result->i_run_unison = 0;
    f_result->osc_type = f_get_osc_off;
    f_result->pitch_inc = 0.1f;
    f_result->uni_spread = 0.1f;
    f_result->voice_count = 1;
    f_result->i_unison_pitch = 0;
    f_result->i_sync_phase = 0;

    f_result->pitch_core = g_pit_get();

    int f_i = 0;

    while(f_i < (OSC_UNISON_MAX_VOICES))
    {
        f_result->osc_cores[f_i] =  g_get_osc_core();
        f_i++;
    }

    v_osc_set_unison_pitch(f_result, .5f, 60.0f);

    f_i = 0;

    while(f_i < (OSC_UNISON_MAX_VOICES))
    {
        f_result->phases[f_i] = 0.0f;
        f_i++;
    }

    v_osc_set_unison_pitch(f_result, .2, 60.0f);

    return f_result;
}


#ifdef	__cplusplus
}
#endif

#endif	/* OSC_SIMPLE_H */

