/* 
 * File:   osc_wavetable.h
 * Author: Jeff Hubbard
 * 
 *
 * Created on January 8, 2012, 11:15 AM
 */

#ifndef OSC_WAVETABLE_H
#define	OSC_WAVETABLE_H

#ifdef	__cplusplus
extern "C" {
#endif


#include "../../lib/osc_core.h"
#include "../../constants.h"
#include "../../lib/pitch_core.h"
#include "../../lib/interpolate-linear.h"
#include "wavetables.h"
    

/*C doesn't like dynamic arrays, so we have to define this at compile time.  
 Realistically, we're doing the users a favor by limiting it to 7, because having
 200 sawtooth oscillators playing at the same time doesn't sound fundamentally better than 7*/
#define OSC_UNISON_MAX_VOICES 7
    
/*The wavetable oscillator.  This does not included the wavetables themselves*/    
typedef struct st_osc_wav
{
    /*The increment to advance the pointer each tick of the sample rate clock.  For example:
     Base note of wavetable:  C1
     Playing C2:
     inc == 2.0f == 12 semitones up*/
    float inc;
    /*The last pitch set.  Used to determine whether to re-calculate t_osc_wav->inc*/
    float last_pitch;
    float output;  //the output sample
}t_osc_wav;

/*A unison oscillator. */
typedef struct st_osc_wav_unison
{
    float sr_recip;
    int voice_count;  //Set this to the max number of voices, not to exceed OSC_UNISON_MAX_VOICES      
    float bottom_pitch;
    float pitch_inc;
    float voice_inc [OSC_UNISON_MAX_VOICES];
    t_osc_core * osc_cores [OSC_UNISON_MAX_VOICES];
    float * selected_wavetable;    
    int selected_wavetable_sample_count;
    float selected_wavetable_sample_count_float;
    float phases [OSC_UNISON_MAX_VOICES];  //Restart the oscillators at the same phase on each note-on    
    float uni_spread;                
    float adjusted_amp;  //Set this with unison voices to prevent excessive volume     
    int i_run_unison;  //iterator for running the unison oscillator
    int i_uni_pitch;  //iterator for setting unison pitch
    float current_sample;  //current output sample for the entire oscillator
    t_pit_pitch_core * pitch_core;
    /*This will be replaced with a SINC interpolator once the wavetable oscillator is working*/
    t_lin_interpolater * linear_interpolator;
}t_osc_wav_unison;


typedef float (*fp_get_osc_wav_func_ptr)(t_osc_wav*,t_wavetable*);

inline void v_osc_wav_set_uni_voice_count(t_osc_wav_unison*, int);
inline void v_osc_wav_set_unison_pitch(t_osc_wav_unison *, float, float);
inline float f_osc_wav_run_unison(t_osc_wav_unison *);
inline float f_osc_wav_run_unison_off(t_osc_wav_unison *);
inline void v_osc_wav_run(t_osc_core *,t_wavetable*,t_lin_interpolater*);
inline void v_osc_wav_note_on_sync_phases(t_osc_wav_unison *);
inline void v_osc_wav_set_waveform(t_osc_wav_unison*, float *, int);
t_osc_wav * g_osc_get_osc_wav();
t_osc_wav_unison * g_osc_get_osc_wav_unison(float);


inline void v_osc_wav_set_waveform(t_osc_wav_unison* a_osc_wav, float * a_waveform, int a_sample_count)
{
    a_osc_wav->selected_wavetable = a_waveform;
    a_osc_wav->selected_wavetable_sample_count = a_sample_count;
    a_osc_wav->selected_wavetable_sample_count_float = (float)(a_sample_count);
}

/* void v_osc_wav_set_uni_voice_count(
 * t_osc_simple_unison* a_osc_ptr, 
 * int a_value) //the number of unison voices this oscillator should use
 */
void v_osc_wav_set_uni_voice_count(t_osc_wav_unison* a_osc_ptr, int a_value)
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
    
    a_osc_ptr->adjusted_amp = (1.0f / (float)(a_osc_ptr->voice_count)) + ((a_osc_ptr->voice_count - 1) * .06f);
}


/* void v_osc_set_unison_pitch(
 * t_osc_simple_unison * a_osc_ptr, 
 * float a_spread, //the total spread of the unison pitches, the distance in semitones from bottom pitch to top.  Typically .1 to .5
 * float a_pitch,  //the pitch of the oscillator in MIDI note number, typically 32 to 70
 * float a_wav_recip) //The the reciprocal of the wavetable's base pitch
 * 
 * This uses the formula:
 * (1.0f/(sample_rate/table_length)) * osc_hz = ratio
 * 
 * This avoids division in the main loop
 */
void v_osc_wav_set_unison_pitch(t_osc_wav_unison * a_osc_ptr, float a_spread, float a_pitch)
{
    if((a_osc_ptr->voice_count) == 1)
    {
        a_osc_ptr->voice_inc[0] =  f_pit_midi_note_to_hz_fast(a_pitch, a_osc_ptr->pitch_core) * (a_osc_ptr->sr_recip);
    }
    else
    {        
        if(a_spread != (a_osc_ptr->uni_spread))
        {
            a_osc_ptr->uni_spread = a_spread;
            a_osc_ptr->bottom_pitch = -0.5f * a_spread;
            a_osc_ptr->pitch_inc = a_spread / ((float)(a_osc_ptr->voice_count));
        }
        
        a_osc_ptr->i_uni_pitch = 0;

        while((a_osc_ptr->i_uni_pitch) < (a_osc_ptr->voice_count))
        {
            a_osc_ptr->voice_inc[(a_osc_ptr->i_uni_pitch)] =  f_pit_midi_note_to_hz_fast(
                    (a_pitch + (a_osc_ptr->bottom_pitch) + (a_osc_ptr->pitch_inc * ((float)(a_osc_ptr->i_uni_pitch))))
                    , a_osc_ptr->pitch_core) 
                    * (a_osc_ptr->sr_recip);
            
            a_osc_ptr->i_uni_pitch = (a_osc_ptr->i_uni_pitch) + 1;
        }
    }
    
}



//Return zero if the oscillator is turned off.  A function pointer should point here if the oscillator is turned off.
float f_osc_wav_run_unison_off(t_osc_wav_unison * a_osc_ptr)
{
    return 0.0f;
}

/* float f_osc_run_unison_osc(t_osc_simple_unison * a_osc_ptr)
 * 
 * Returns one sample of an oscillator's output
 */
float f_osc_wav_run_unison(t_osc_wav_unison * a_osc_ptr)
{
    a_osc_ptr->i_run_unison = 0;
    a_osc_ptr->current_sample = 0.0f;
        
    while((a_osc_ptr->i_run_unison) < (a_osc_ptr->voice_count))
    {
        //v_run_osc((a_osc_ptr->osc_cores[(a_osc_ptr->i_run_unison)]), (a_osc_ptr->voice_inc[(a_osc_ptr->i_run_unison)]));
        //a_osc_ptr->current_sample = (a_osc_ptr->current_sample) + a_osc_ptr->osc_type((a_osc_ptr->osc_cores[(a_osc_ptr->i_run_unison)]));
        v_run_osc(a_osc_ptr->osc_cores[(a_osc_ptr->i_run_unison)], (a_osc_ptr->voice_inc[(a_osc_ptr->i_run_unison)]));
        a_osc_ptr->current_sample = (a_osc_ptr->current_sample) + 
                f_linear_interpolate_ptr_wrap(a_osc_ptr->selected_wavetable, a_osc_ptr->selected_wavetable_sample_count, 
                ((a_osc_ptr->osc_cores[(a_osc_ptr->i_run_unison)]->output) * (a_osc_ptr->selected_wavetable_sample_count_float)), a_osc_ptr->linear_interpolator);
    
        a_osc_ptr->i_run_unison = (a_osc_ptr->i_run_unison) + 1;
    }
    
    return (a_osc_ptr->current_sample) * (a_osc_ptr->adjusted_amp);
}


/*Resync the oscillators at note_on to hopefully avoid phasing artifacts*/
void v_osc_wav_note_on_sync_phases(t_osc_wav_unison * a_osc_ptr)
{
    int f_i = 0;
    
    while(f_i < a_osc_ptr->voice_count)
    {
        a_osc_ptr->osc_cores[f_i]->output = a_osc_ptr->phases[f_i];        
        f_i++;
    }
}

t_osc_wav * g_osc_get_osc_wav()
{
    t_osc_wav * f_result = (t_osc_wav*)malloc(sizeof(t_osc_wav));
    
    f_result->inc = 0.0f;
    f_result->last_pitch = 20.0f;
    f_result->output = 0.0f;
        
    return f_result;
}

/* t_osc_simple_unison * g_osc_get_osc_simple_unison(float a_sample_rate)
 */
t_osc_wav_unison * g_osc_get_osc_wav_unison(float a_sample_rate)
{
    t_osc_wav_unison * f_result = (t_osc_wav_unison*)malloc(sizeof(t_osc_wav_unison));
    
    v_osc_wav_set_uni_voice_count(f_result, OSC_UNISON_MAX_VOICES);        
    f_result->sr_recip = 1.0f / a_sample_rate;
    f_result->adjusted_amp = 1.0f;
    f_result->bottom_pitch = -0.1f;
    f_result->current_sample = 0.0f;
    f_result->i_run_unison = 0;
    f_result->pitch_inc = 0.1f;
    f_result->uni_spread = 0.1f;
    f_result->voice_count = 1;
    f_result->i_uni_pitch = 0;
        
    f_result->pitch_core = g_pit_get();
    f_result->linear_interpolator = g_lin_get();
        
    int f_i = 0;
    
    while(f_i < (OSC_UNISON_MAX_VOICES))
    {
        f_result->osc_cores[f_i] =  g_get_osc_core(); 
        //f_result->osc_wavs[f_i] = 
        f_i++;
    }
           
    f_i = 0;
        
    while(f_i < (OSC_UNISON_MAX_VOICES))
    {
        f_result->phases[f_i] = 0.0f;  //TODO:  get some good values for this
        f_i++;
    }
    
    v_osc_wav_set_unison_pitch(f_result, .2f, 60.0f);
    
    f_result->selected_wavetable = 0;
    f_result->selected_wavetable_sample_count = 0;
    f_result->selected_wavetable_sample_count_float = 0.0f;
    
    return f_result;
}


#ifdef	__cplusplus
}
#endif

#endif	/* OSC_WAVETABLE_H */

