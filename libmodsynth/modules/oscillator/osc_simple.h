/* 
 * File:   osc_simple.h
 * Author: vm-user
 *
 * Created on January 7, 2012, 8:52 PM
 */

#ifndef OSC_SIMPLE_H
#define	OSC_SIMPLE_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "../../lib/osc_core.h"
#include "../../constants.h"
#include "../../lib/pitch_core.h"
#include <math.h>
    

/*C doesn't like dynamic arrays, so we have to define this at compile time.  
 Realistically, we're doing the users a favor by limiting it to 7, because having
 200 sawtooth oscillators playing at the same time doesn't sound fundamentally better than 7*/
#define OSC_UNISON_MAX_VOICES 7
    
//Used to switch between values, uses much less CPU than a switch statement
typedef float (*fp_get_osc_func_ptr)(t_osc_core*);


/*A unison oscillator.  The _osc_type pointer determines which waveform it uses, it defaults to saw unless you set it to something else*/
typedef struct st_osc_simple_unison
{
    float sr_recip;
    int voice_count;  //Set this to the max number of voices, not to exceed OSC_UNISON_MAX_VOICES
    fp_get_osc_func_ptr osc_type;        
    float bottom_pitch;
    float pitch_inc;
    float voice_inc [OSC_UNISON_MAX_VOICES];
    t_osc_core * osc_cores [OSC_UNISON_MAX_VOICES];
    
    float phases [OSC_UNISON_MAX_VOICES];  //Restart the oscillators at the same phase on each note-on
    
    float uni_spread;
                
    float adjusted_amp;  //Set this with unison voices to prevent excessive volume     
    
}t_osc_simple_unison;


void v_osc_set_uni_voice_count(t_osc_simple_unison*, int);
void v_osc_set_unison_pitch(t_osc_simple_unison * a_osc_ptr, float a_spread, float a_pitch);
float f_osc_run_unison_osc(t_osc_simple_unison * a_osc_ptr);
float f_get_saw(t_osc_core *);
float f_get_sine(t_osc_core *);
float f_get_square(t_osc_core *);
float f_get_triangle(t_osc_core *);
float f_get_osc_off(t_osc_core *);
void v_osc_set_simple_osc_unison_type(t_osc_simple_unison *, int);
void v_osc_note_on_sync_phases(t_osc_simple_unison *);
t_osc_simple_unison * g_osc_get_osc_simple_unison(float);



void v_osc_set_uni_voice_count(t_osc_simple_unison* a_osc_ptr, int a_value)
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
    
    a_osc_ptr->adjusted_amp = (1 / (float)(a_osc_ptr->voice_count)) + ((a_osc_ptr->voice_count - 1) * .06);
}



void v_osc_set_unison_pitch(t_osc_simple_unison * a_osc_ptr, float a_spread, float a_pitch)
{
    if((a_osc_ptr->voice_count) == 1)
    {
        a_osc_ptr->voice_inc[0] =  f_pit_midi_note_to_hz_fast(a_pitch) * a_osc_ptr->sr_recip;
    }
    else
    {        
        if(a_spread != (a_osc_ptr->uni_spread))
        {
            a_osc_ptr->uni_spread = a_spread;
            a_osc_ptr->bottom_pitch = -.5 * a_spread;
            a_osc_ptr->pitch_inc = a_spread / ((float)(a_osc_ptr->voice_count));
        }
        
        int i = 0;

        while(i < (a_osc_ptr->voice_count))
        {
            a_osc_ptr->voice_inc[i] =  f_pit_midi_note_to_hz_fast(a_pitch + (a_osc_ptr->bottom_pitch) + (a_osc_ptr->pitch_inc * ((float)i))) * a_osc_ptr->sr_recip;
            i++;
        }
    }
    
}




//Return one sample of the oscillator running.
float f_osc_run_unison_osc(t_osc_simple_unison * a_osc_ptr)
{
    int f_i = 0;
    float f_result = 0;
    
    
    while(f_i < (a_osc_ptr->voice_count))
    {
        f_run_osc((a_osc_ptr->osc_cores[f_i]), (a_osc_ptr->voice_inc[f_i]));
        f_result += a_osc_ptr->osc_type((a_osc_ptr->osc_cores[f_i]));
        f_i++;
    }
    
    return f_result * (a_osc_ptr->adjusted_amp);
}



float f_get_saw(t_osc_core * a_core)
{
    return (((a_core->output) * 2) - 1);
}

float f_get_sine(t_osc_core * a_core)
{
    return sin((a_core->output) * PI2);
}

float f_get_square(t_osc_core * a_core)
{
    if((a_core->output) >= .5)
    {
        return 1;
    }
    else
    {
        return -1;
    }
}

float f_get_triangle(t_osc_core * a_core)
{
    float f_ramp = ((a_core->output) * 4) - 2;
    if(f_ramp > 1)
    {
        return 2 - f_ramp;
    }
    else if(f_ramp < -1)
    {
        return (2 + f_ramp) * -1;
    }
    else
    {
        return f_ramp;
    }
}


//Return zero if the oscillator is turned off.  A function pointer should point here if the oscillator is turned off.
float f_get_osc_off(t_osc_core * a_core)
{
    return 0;
}

/*Set the oscillator type.  Current valid types are:
 * 0. Saw
 * 1. Square
 * 2. Triangle
 * 3. Sine
 * 4. Off
 * 5. Pulse  //Added after 'off' because the developer may not add a pulse knob
 */
void v_osc_set_simple_osc_unison_type(t_osc_simple_unison * a_osc_ptr, int a_index)
{
    switch(a_index)
    {
        case 0:
            a_osc_ptr->osc_type = f_get_saw;
            break;
        case 1:
            a_osc_ptr->osc_type = f_get_square;
            break;
        case 2:
            a_osc_ptr->osc_type = f_get_triangle;
            break;
        case 3:
            a_osc_ptr->osc_type = f_get_sine;
            break;
        case 4:
            a_osc_ptr->osc_type = f_get_osc_off;
            break;
    }
    
}


/*Resync the oscillators at note_on to hopefully avoid phasing artifacts*/
void v_osc_note_on_sync_phases(t_osc_simple_unison * a_osc_ptr)
{
    int f_i = 0;
    
    while(f_i < a_osc_ptr->voice_count)
    {
        a_osc_ptr->osc_cores[f_i]->output = a_osc_ptr->phases[f_i];
        
        f_i++;
    }
}





t_osc_simple_unison * g_osc_get_osc_simple_unison(float a_sample_rate)
{
    t_osc_simple_unison * f_result = (t_osc_simple_unison*)malloc(sizeof(t_osc_simple_unison));
    
    v_osc_set_uni_voice_count(f_result, OSC_UNISON_MAX_VOICES);    
    f_result->osc_type = f_get_saw;
    f_result->sr_recip = 1 / a_sample_rate;
    
    
    int f_i = 0;
    
    while(f_i < (OSC_UNISON_MAX_VOICES))
    {
        f_result->osc_cores[f_i] = (t_osc_core*)malloc(sizeof(t_osc_core));
        f_i++;
    }
        
    v_osc_set_unison_pitch(f_result, .5, 60);
    
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
    
    v_osc_set_unison_pitch(f_result, .2, 60);
    
    return f_result;
}


#ifdef	__cplusplus
}
#endif

#endif	/* OSC_SIMPLE_H */

