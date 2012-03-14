/* 
 * File:   reverb_digital.h
 * Author: Jeff Hubbard
 * 
 * This file is not yet production ready, it still has bugs, flaws and inconsistencies.
 * 
 * A complete reverb effect.
 *
 * Created on January 8, 2012, 5:48 PM
 */

#ifndef REVERB_DIGITAL_H
#define	REVERB_DIGITAL_H

#ifdef	__cplusplus
extern "C" {
#endif

/*The number of comb filters in the reverb*/
#define REVERB_DIGITAL_TAPS 40
/*Buffer size in seconds*/
#define REVERB_DIGITAL_BUFFER_SIZE 2
/*This is used to multiply the amplitude of the output to keep it at a reasonable volume*/
#define REVERB_AMP_RECIP 1.0f/((float)REVERB_DIGITAL_TAPS)
    
#define REVERB_DIGITAL_DEBUG_MODE
    
#include "delay.h"
#include "../../lib/amp.h"
#include "../../lib/pitch_core.h"
#include "../../lib/denormal.h"
#include "../../modules/filter/svf.h"
#include "../../modules/modulation/env_follower.h"
    
typedef struct st_rvd_reverb
{
    t_delay_simple * buffer0;
    t_delay_simple * buffer1;
    t_state_variable_filter * svf_lp0;
    t_state_variable_filter * svf_lp1;
    t_state_variable_filter * svf_hp0;
    t_state_variable_filter * svf_hp1;
    t_enf_env_follower * envf0;
    t_enf_env_follower * envf1;
    t_delay_tap * predelay_tap;
    t_delay_tap * taps [REVERB_DIGITAL_TAPS];
    float feedback [REVERB_DIGITAL_TAPS];
    float amp [REVERB_DIGITAL_TAPS];
    float hz [REVERB_DIGITAL_TAPS];
    //t_delay_tap taps1 [REVERB_DIGITAL_TAPS];
    float feedback0;  //The feedback sample for channel 0
    float feedback1;  //The feedback sample for channel 1
    float out0;
    float out1 ;    
    float predelay;
    int iterator;    
    float base_pitch;   //The lowest frequency comb filter, in MIDI note number
    float pitch_inc;    //The amount to increment the pitch by for each comb filter
    float reverb_floor;  //The value to consider a reverb tail finished at.  -30 or -50 is a good number
    float reverb_time;
    float lp_cutoff;  //Lowpass filter cutoff frequency in MIDI note number
    float hp_cutoff;  //Highpass filter cutoff frequency in MIDI note number
    float lp_out0;  //Outputs for the lowpass filter
    float lp_out1;
    float hp_out0;  //Outputs for the highpass filter
    float hp_out1;
    
    t_amp * amp_ptr;
    t_pit_pitch_core * pitch_core;
#ifdef REVERB_DIGITAL_DEBUG_MODE
    int debug_counter;
#endif
}t_rvd_reverb;

t_rvd_reverb * g_rvd_get_reverb(float);
inline void v_rvd_run_reverb(t_rvd_reverb*,float,float);
inline void v_rvd_set_reverb(t_rvd_reverb*,float,float);
inline void v_rvd_set_svf(t_rvd_reverb*,float,float);

/* t_rvd_reverb * g_rvd_get_reverb(
 * float a_sr)  //sample rate
 */
t_rvd_reverb * g_rvd_get_reverb(float a_sr)
{
    t_rvd_reverb * f_result = (t_rvd_reverb*)malloc(sizeof(t_rvd_reverb));
    
    f_result->buffer0 = g_dly_get_delay(4, a_sr);
    f_result->buffer1 = g_dly_get_delay(4, a_sr);
    f_result->predelay_tap = g_dly_get_tap();
    f_result->svf_lp0 = g_svf_get(a_sr);
    f_result->svf_lp1 = g_svf_get(a_sr);
    f_result->svf_hp0 = g_svf_get(a_sr);
    f_result->svf_hp1 = g_svf_get(a_sr);
    f_result->envf0 = g_enf_get_env_follower(a_sr);
    f_result->envf1 = g_enf_get_env_follower(a_sr);
    f_result->out0 = 0;
    f_result->out1 = 0;
    f_result->predelay = 0;
    f_result->iterator = 0;
    f_result->reverb_floor = -50.0f;
    f_result->reverb_time = 1;
    f_result->feedback0 = 0;
    f_result->feedback1 = 0;
    f_result->base_pitch = -20;
    f_result->pitch_inc = 0.2f;
    f_result->lp_cutoff = 110;
    f_result->hp_cutoff = 40;
    f_result->lp_out0 = 0;
    f_result->lp_out1 = 0;
    f_result->hp_out0 = 0;
    f_result->hp_out1 = 0;
    
    f_result->amp_ptr = g_amp_get();
    f_result->pitch_core = g_pit_get();
    
    int f_i = 0;
    
    while(f_i < REVERB_DIGITAL_TAPS)
    {        
        f_result->taps [f_i] = g_dly_get_tap();
        //f_result->taps1 [f_i] = g_dly_get_tap();
        f_result->feedback[f_i] = 0;
        f_result->amp[f_i] = 1;
        f_result->hz[f_i] = (10 + (f_i * 42.21367));
        
        f_i++;
    }
    
    v_svf_set_res(f_result->svf_lp0, -18.0f);
    v_svf_set_res(f_result->svf_lp1, -18.0f);
    v_svf_set_res(f_result->svf_hp0, -18.0f);
    v_svf_set_res(f_result->svf_hp1, -18.0f);
    
    v_rvd_set_svf(f_result, 20, 120);
        
    return f_result;
}

/* void v_rvd_run_reverb
 * (t_rvd_reverb* a_rvd, 
 * float a_in0, 
 * float a_in1)
 */
inline void v_rvd_run_reverb(t_rvd_reverb* a_rvd, float a_in0, float a_in1)
{    
    a_rvd->hp_out0 = v_svf_run_2_pole_hp(a_rvd->svf_hp0, (a_in0 + ((a_rvd->feedback0) * (REVERB_AMP_RECIP) * (a_rvd->feedback[(a_rvd->iterator)]))));
    a_rvd->hp_out1 = v_svf_run_2_pole_hp(a_rvd->svf_hp1, (a_in1 + ((a_rvd->feedback1) * (REVERB_AMP_RECIP) * (a_rvd->feedback[(a_rvd->iterator)]))));
    
    a_rvd->lp_out0 = v_svf_run_2_pole_lp(a_rvd->svf_lp0, (a_rvd->hp_out0));
    a_rvd->lp_out1 = v_svf_run_2_pole_lp(a_rvd->svf_lp1, (a_rvd->hp_out1));
    
    v_dly_run_delay(a_rvd->buffer0, f_remove_denormal((a_rvd->lp_out0)));
    v_dly_run_delay(a_rvd->buffer1, f_remove_denormal((a_rvd->lp_out0)));
 
    a_rvd->iterator = 0;    
    a_rvd->out0 = 0;
    a_rvd->out1 = 0;        
    a_rvd->feedback0 = 0;
    a_rvd->feedback1 = 0;
        
    while((a_rvd->iterator) < REVERB_DIGITAL_TAPS)
    {
        /*Channel 0*/
        v_dly_run_tap(a_rvd->buffer0, a_rvd->taps[(a_rvd->iterator)]);
        a_rvd->out0 = ((a_rvd->out0) + (a_rvd->taps[(a_rvd->iterator)]->output));        
        a_rvd->feedback0 = ((a_rvd->feedback0) + ((a_rvd->taps[(a_rvd->iterator)]->output) * (a_rvd->feedback[(a_rvd->iterator)])));
        
        /*Reuse the same tap for channel 1*/
        v_dly_run_tap(a_rvd->buffer1, a_rvd->taps[(a_rvd->iterator)]);        
        a_rvd->out1 = ((a_rvd->out1) + (a_rvd->taps[(a_rvd->iterator)]->output));        
        a_rvd->feedback1 = ((a_rvd->feedback1) + ((a_rvd->taps[(a_rvd->iterator)]->output) * (a_rvd->feedback[(a_rvd->iterator)])));        
        
        a_rvd->iterator = (a_rvd->iterator) + 1;
    }
    
    /*
    v_dly_run_delay(a_rvd->predelay_buffer0, (a_rvd->out0));
    v_dly_run_delay(a_rvd->predelay_buffer1, (a_rvd->out1));
    
    v_dly_run_tap(a_rvd->predelay_buffer0, a_rvd->predelay_tap);
    
    a_rvd->out0 = (a_rvd->predelay_tap->output);
    
    v_dly_run_tap(a_rvd->predelay_buffer1, a_rvd->predelay_tap);
    
    a_rvd->out1 = (a_rvd->predelay_tap->output);
    */
    
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
        printf("a_rvd->predelay == %f\n", (a_rvd->predelay));
        printf("a_rvd->reverb_floor == %f\n", (a_rvd->reverb_floor));
        printf("a_rvd->reverb_time == %f\n", (a_rvd->reverb_time));
        
        int f_i = 0;
        
        while(f_i < REVERB_DIGITAL_TAPS)
        {
            printf("Comb %i\n", f_i);
            printf("amp == %f   ", (a_rvd->amp[f_i]));
            printf("feedback == %f  ", (a_rvd->feedback[f_i]));
            printf("hz == %f  ", (a_rvd->hz[f_i]));  
            printf("\n");
            
            f_i++;
        }
    }
#endif
}

/*void v_rvd_set_reverb(
 * t_rvd_reverb* a_rvd, 
 * float a_time, //Reverb time in seconds
 * float a_predelay)  //predelay in seconds 
 */
inline void v_rvd_set_reverb(t_rvd_reverb* a_rvd, float a_time, float a_predelay)
{
    
    
    
    if((a_time != (a_rvd->reverb_time)) || (a_predelay != (a_rvd->predelay)))
    {        
#ifdef REVERB_DIGITAL_DEBUG_MODE
        printf("\nif(a_time != (a_rvd->reverb_time))\n");
        printf("a_time == %f\n", a_time);
        printf("a_rvd->reverb_time == %f\n", a_rvd->reverb_time);
#endif
        a_rvd->reverb_time = a_time;
        a_rvd->predelay = a_predelay;
        
        a_rvd->iterator = 0;
        
        while((a_rvd->iterator) < REVERB_DIGITAL_TAPS)
        {
            a_rvd->hz[(a_rvd->iterator)] = f_pit_midi_note_to_hz((a_rvd->base_pitch) + ((a_rvd->pitch_inc) * (a_rvd->iterator)));
            v_dly_set_delay_hz(a_rvd->buffer0, a_rvd->taps[(a_rvd->iterator)], ((a_rvd->hz[(a_rvd->iterator)])));
            
            a_rvd->feedback[(a_rvd->iterator)] = //a_time *
                    f_db_to_linear((((a_rvd->reverb_floor)/(a_rvd->reverb_time))/(a_rvd->hz[(a_rvd->iterator)])), a_rvd->amp_ptr);
            
            a_rvd->iterator = (a_rvd->iterator) + 1;
        }
    }
}

/* void v_rvd_set_svf(
 * t_rvd_reverb*,
 * float a_hp, //The highpass filter frequency in MIDI note number, typical range:  20 to 70
 * float a_lp) //The lowpass filter frequency in MIDI note number, typical range:  70 to 124
 */
inline void v_rvd_set_svf(t_rvd_reverb* a_rvd,float a_hp, float a_lp)
{
    if(a_hp != (a_rvd->hp_cutoff))
    {
        a_rvd->hp_cutoff = a_hp;
        v_svf_set_cutoff_base(a_rvd->svf_hp0, a_hp);
        v_svf_set_cutoff(a_rvd->svf_hp0);
        v_svf_set_cutoff_base(a_rvd->svf_hp1, a_hp);
        v_svf_set_cutoff(a_rvd->svf_hp1);
    }
    
    if(a_lp != (a_rvd->lp_cutoff))
    {
        a_rvd->lp_cutoff = a_lp;
        v_svf_set_cutoff_base(a_rvd->svf_lp0, a_lp);
        v_svf_set_cutoff(a_rvd->svf_lp0);
        v_svf_set_cutoff_base(a_rvd->svf_lp1, a_lp);
        v_svf_set_cutoff(a_rvd->svf_lp1);
    }
}

#ifdef	__cplusplus
}
#endif

#endif	/* REVERB_DIGITAL_H */

