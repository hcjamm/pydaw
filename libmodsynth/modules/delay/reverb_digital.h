/* 
 * File:   reverb_digital.h
 * Author: Jeff Hubbard
 *
 * Created on January 8, 2012, 5:48 PM
 */

#ifndef REVERB_DIGITAL_H
#define	REVERB_DIGITAL_H

#ifdef	__cplusplus
extern "C" {
#endif

/*The number of comb filters in the reverb*/
#define REVERB_DIGITAL_TAPS 8
/*This is used to multiply the amplitude of the output to keep it at a reasonable volume*/
#define REVERB_AMP_RECIP 1/REVERB_DIGITAL_TAPS
    
#define REVERB_DIGITAL_DEBUG_MODE
    
#include "delay.h"
#include "../../lib/amp.h"
#include "../../lib/pitch_core.h"
#include "../../lib/denormal.h"
    
typedef struct st_rvd_reverb
{
    t_delay_simple * buffer0;
    t_delay_simple * buffer1;
    t_delay_tap * taps0 [REVERB_DIGITAL_TAPS];
    float feedback [REVERB_DIGITAL_TAPS];
    float amp [REVERB_DIGITAL_TAPS];
    float hz [REVERB_DIGITAL_TAPS];
    //t_delay_tap taps1 [REVERB_DIGITAL_TAPS];
    float feedback0;  //The feedback sample for channel 0
    float feedback1;  //The feedback sample for channel 1
    float out0;
    float out1 ;    
    int predelay;
    int iterator;    
    float base_pitch;   //The lowest frequency comb filter, in MIDI note number
    float pitch_inc;    //The amount to increment the pitch by for each comb filter
    float reverb_floor;  //The value to consider a reverb tail finished at.  -30 or -50 is a good number
    float reverb_time;
#ifdef REVERB_DIGITAL_DEBUG_MODE
    int debug_counter;
#endif
}t_rvd_reverb;

t_rvd_reverb * g_rvd_get_reverb(float);

/* t_rvd_reverb * g_rvd_get_reverb(
 * float a_sr)
 */
t_rvd_reverb * g_rvd_get_reverb(float a_sr)
{
    t_rvd_reverb * f_result = (t_rvd_reverb*)malloc(sizeof(t_rvd_reverb));
    
    f_result->buffer0 = g_dly_get_delay(1, a_sr);
    f_result->buffer1 = g_dly_get_delay(1, a_sr);    
    f_result->out0 = 0;
    f_result->out1 = 0;
    f_result->predelay = 0;
    f_result->iterator = 0;
    f_result->reverb_floor = -30.0f;
    f_result->reverb_time = 1;
    f_result->feedback0 = 0;
    f_result->feedback1 = 0;
    f_result->base_pitch = 60;
    f_result->pitch_inc = 3;
    
    int f_i = 0;
    
    while(f_i < REVERB_DIGITAL_TAPS)
    {        
        f_result->taps0 [f_i] = g_dly_get_tap();
        //f_result->taps1 [f_i] = g_dly_get_tap();
        f_result->feedback[f_i] = 0;
        f_result->amp[f_i] = 1;
        f_result->hz[f_i] = (10 + (f_i * 42.21367));
        
        f_i++;
    }
        
    return f_result;
}

void v_rvd_run_reverb(t_rvd_reverb*,float,float);
void v_rvd_set_reverb(t_rvd_reverb*,float,float);

/* void v_rvd_run_reverb
 * (t_rvd_reverb* a_rvd, 
 * float a_in0, 
 * float a_in1)
 */
void v_rvd_run_reverb(t_rvd_reverb* a_rvd, float a_in0, float a_in1)
{
    a_rvd->iterator = 0;
    a_rvd->out0 = 0;
    a_rvd->out1 = 0;
    
    v_dly_run_delay(a_rvd->buffer0, (a_in0 + (a_rvd->feedback0)));
    v_dly_run_delay(a_rvd->buffer1, (a_in1 + (a_rvd->feedback1)));
    
    a_rvd->feedback0 = 0;
    a_rvd->feedback1 = 0;
        
    while((a_rvd->iterator) < REVERB_DIGITAL_TAPS)
    {
        /*Channel 0*/
        v_dly_run_tap(a_rvd->buffer0, a_rvd->taps0[(a_rvd->iterator)]);
        a_rvd->out0 = ((a_rvd->out0) + (a_rvd->taps0[(a_rvd->iterator)]->output));
        a_rvd->feedback0 = ((a_rvd->feedback0) + ((a_rvd->out0) * (a_rvd->feedback[(a_rvd->iterator)])));
        
        /*Reuse the same tap for channel 1*/
        v_dly_run_tap(a_rvd->buffer1, a_rvd->taps0[(a_rvd->iterator)]);        
        a_rvd->out1 = ((a_rvd->out1) + (a_rvd->taps0[(a_rvd->iterator)]->output));        
        a_rvd->feedback1 = ((a_rvd->feedback1) + ((a_rvd->out1) * (a_rvd->feedback[(a_rvd->iterator)])));
        
        
        a_rvd->iterator = (a_rvd->iterator) + 1;
    }
    
#ifdef REVERB_DIGITAL_DEBUG_MODE
    a_rvd->debug_counter = (a_rvd->debug_counter) + 1;
    
    if((a_rvd->debug_counter) >= 100000)
    {
        a_rvd->debug_counter = 0;
        printf("\n\nReverb digital info\n");
        printf("a_rvd->base_pitch == %f\n", (a_rvd->base_pitch));
        printf("a_rvd->feedback0 == %f\n", (a_rvd->feedback0));
        printf("a_rvd->feedback1 == %f\n", (a_rvd->feedback1));
        printf("a_rvd->out0 == %f\n", (a_rvd->out0));
        printf("a_rvd->out1 == %f\n", (a_rvd->out1));
        printf("a_rvd->pitch_inc == %f\n", (a_rvd->pitch_inc));
        printf("a_rvd->predelay == %i\n", (a_rvd->predelay));
        printf("a_rvd->reverb_floor == %f\n", (a_rvd->reverb_floor));
        printf("a_rvd->reverb_time == %f\n", (a_rvd->reverb_time));
        
        int f_i = 0;
        
        while(f_i < REVERB_DIGITAL_TAPS)
        {
            printf("\n\nComb %i\n", f_i);
            printf("amp == %f   ", (a_rvd->amp[f_i]));
            printf("feedback == %f  ", (a_rvd->feedback[f_i]));
            printf("hz == %f  ", (a_rvd->hz[f_i]));  
            printf("\n\n");
            
            f_i++;
        }
    }
#endif
}

/*void v_rvd_set_reverb(
 * t_rvd_reverb* a_rvd, 
 * float a_time, //Reverb time in seconds
 * float a_predelay)  //predelay in seconds
 * 
 * This method is based on the following formula I made up:     (**Granted, I'm sure other reverbs use something similar)
 * 
 * Feedback Amp = ((floor/time)/freq)
 * 
 * Where 
 * floor == When to consider the reverb finished (perhaps -30db or -50db
 * time == How long to run the reverb in seconds
 * freq == Frequency in Hz.  This is used because it sounds more musical than using linear time values
 * 
 * The idea being that every pass of the comb filter, it loses Xdb, so from there you can calculate reverb length.
 * If we calculate all comb filters to last the exact same length of time, it will create a smooth sound tail, instead
 * of weird metallic rattling sounds when some of the filters drop off sooner than others.
 * However, adding things like filters to the feedback chain can also affect volume
 * 
 * This function is incredibly expensive to allow for tight sounding reverbs with smooth tails, 
 * you shouldn't modulate these values with LFOs, envelopes or host automation, 
 * they should only be updated when the user moves a knob on the GUI.
 * 
 * TODO:  Experiment with using a 3db/octave curve on the comb filters frequency
 */
void v_rvd_set_reverb(t_rvd_reverb* a_rvd, float a_time, float a_predelay)
{
    if((a_time != (a_rvd->reverb_time)) && (a_predelay != (a_rvd->predelay)))
    {
        a_rvd->iterator = 0;

        while((a_rvd->iterator) < REVERB_DIGITAL_TAPS)
        {
            a_rvd->hz[(a_rvd->iterator)] = f_pit_midi_note_to_hz((a_rvd->base_pitch) + ((a_rvd->pitch_inc) * (a_rvd->iterator)));
            v_dly_set_delay_hz(a_rvd->buffer0, a_rvd->taps0[(a_rvd->iterator)], (a_rvd->hz[(a_rvd->iterator)]));
            a_rvd->feedback[(a_rvd->iterator)] = f_db_to_linear(((a_rvd->reverb_floor)/(a_rvd->reverb_time))/(a_rvd->hz[(a_rvd->iterator)]));
            
            a_rvd->iterator = (a_rvd->iterator) + 1;
        }
    }
}

#ifdef	__cplusplus
}
#endif

#endif	/* REVERB_DIGITAL_H */

