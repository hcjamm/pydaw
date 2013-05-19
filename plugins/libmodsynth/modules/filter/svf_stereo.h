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

#ifndef PYDAW_SVF_STEREO_H
#define	PYDAW_SVF_STEREO_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "../../lib/pitch_core.h"
#include "../../lib/amp.h"
#include "../../lib/interpolate-linear.h"
#include "../../constants.h"
#include "../../lib/smoother-linear.h"
#include "../../lib/denormal.h"

/*Define filter types for changing the function pointer*/
#define SVF_FILTER_TYPE_LP 0
#define SVF_FILTER_TYPE_HP 1
#define SVF_FILTER_TYPE_BP 2
#define SVF_FILTER_TYPE_EQ 3
#define SVF_FILTER_TYPE_NOTCH 4
    
/*The maximum number of filter kernels to cascade.
 */
#define SVF_MAX_CASCADE 2

/*Changing this only affects initialization of the filter, you must still change the code in v_svf2_run()*/
#define SVF_OVERSAMPLE_MULTIPLIER 4
#define SVF_OVERSAMPLE_STEP_SIZE 0.25f
    
/*Provides data storage for the inner-workings of the filter*/
typedef struct
{
    float filter_input, filter_last_input, bp_m1, lp_m1, hp, lp, bp;    
}t_svf2_kernel;

typedef struct
{
    float cutoff_note, cutoff_hz, cutoff_filter, pi2_div_sr, sr, filter_res, filter_res_db, velocity_cutoff; //, velocity_cutoff_amt;    
    float cutoff_base, cutoff_mod, cutoff_last,  velocity_mod_amt;  //New additions to fine-tune the modulation process    
    float oversample_iterator;    
    t_svf2_kernel * filter_kernels [SVF_MAX_CASCADE][2];
    t_amp * amp_ptr;
    t_pit_pitch_core * pitch_core;
    float output0, output1;
} t_svf2_filter; 

//Used to switch between values, uses much less CPU than a switch statement at every tick of the samplerate clock
typedef void (*fp_svf2_run_filter)(t_svf2_filter*,float, float);

/*The int is the number of cascaded filter kernels*/
inline fp_svf2_run_filter fp_svf2_get_run_filter_ptr(int,int);

inline void v_svf2_run(t_svf2_filter*, t_svf2_kernel *, float);

inline void v_svf2_run_2_pole_lp(t_svf2_filter*, float, float);
inline void v_svf2_run_4_pole_lp(t_svf2_filter*, float, float);

inline void v_svf2_run_2_pole_hp(t_svf2_filter*, float, float);
inline void v_svf2_run_4_pole_hp(t_svf2_filter*, float, float);

inline void v_svf2_run_2_pole_bp(t_svf2_filter*, float, float);
inline void v_svf2_run_4_pole_bp(t_svf2_filter*, float, float);

inline void v_svf2_run_2_pole_notch(t_svf2_filter*, float, float);
inline void v_svf2_run_4_pole_notch(t_svf2_filter*, float, float);

inline void v_svf2_run_no_filter(t_svf2_filter*, float, float);

inline void v_svf2_run_2_pole_allpass(t_svf2_filter*, float, float);

/* inline void v_svf2_run_no_filter(
 * t_svf2_filter* a_svf, 
 * float a_in) //audio input
 * 
 * This is for allowing a filter to be turned off by running a function pointer.  a_in is returned unmodified. 
 */
inline void v_svf2_run_no_filter(t_svf2_filter*__restrict a_svf, float a_in0, float a_in1)
{
    a_svf->output0 = a_in0;
    a_svf->output1 = a_in1;
}

t_svf2_kernel * g_svf2_get_filter_kernel();

t_svf2_kernel * g_svf2_get_filter_kernel()
{
    t_svf2_kernel * f_result;
    
    if(posix_memalign((void**)&f_result, 16, (sizeof(t_svf2_kernel))) != 0)
    {
        return 0;
    }
        
    f_result->bp = 0.0f;    
    f_result->hp = 0.0f;
    f_result->lp = 0.0f;    
    f_result->lp_m1 = 0.0f;
    f_result->filter_input = 0.0f;
    f_result->filter_last_input = 0.0f;  
    f_result->bp_m1 = 0.0f;
    return f_result;
}

/* inline fp_svf2_run_filter fp_svf2_get_run_filter_ptr(
 * int a_cascades, 
 * int a_filter_type)
 * 
 * The int refers to the number of cascaded filter kernels, ie:  a value of 2 == 4 pole filter
 *  
 */
inline fp_svf2_run_filter fp_svf2_get_run_filter_ptr(int a_cascades, int a_filter_type)
{
    /*Lowpass*/
    if((a_cascades == 1) && (a_filter_type == SVF_FILTER_TYPE_LP))
    {
        return v_svf2_run_2_pole_lp;
    }
    else if((a_cascades == 2) && (a_filter_type == SVF_FILTER_TYPE_LP))
    {
        return v_svf2_run_4_pole_lp;
    }
    /*Highpass*/
    else if((a_cascades == 1) && (a_filter_type == SVF_FILTER_TYPE_HP))
    {
        return v_svf2_run_2_pole_hp;
    }
    else if((a_cascades == 2) && (a_filter_type == SVF_FILTER_TYPE_HP))
    {
        return v_svf2_run_4_pole_hp;
    }
    /*Bandpass*/
    else if((a_cascades == 1) && (a_filter_type == SVF_FILTER_TYPE_BP))
    {
        return v_svf2_run_2_pole_bp;
    }
    else if((a_cascades == 2) && (a_filter_type == SVF_FILTER_TYPE_BP))
    {
        return v_svf2_run_4_pole_bp;
    }
    /*Notch*/
    else if((a_cascades == 1) && (a_filter_type == SVF_FILTER_TYPE_NOTCH))
    {
        return v_svf2_run_2_pole_notch;
    }
    else if((a_cascades == 2) && (a_filter_type == SVF_FILTER_TYPE_NOTCH))
    {
        return v_svf2_run_4_pole_notch;
    }
    /*Sane-ish default...*/
    else
    {
        return v_svf2_run_2_pole_lp;
    }
}

/* inline void v_svf2_run(
 * t_svf2_filter * a_svf, 
 * t_svf2_kernel * a_kernel, 
 * float a_input_value) //the audio input to filter
 * 
 * The main action to run the filter kernel*/
inline void v_svf2_run(t_svf2_filter *__restrict a_svf, t_svf2_kernel *__restrict a_kernel, float a_input_value)
{        
    a_kernel->filter_input = a_input_value;
    
    a_svf->oversample_iterator = 0.0f;
    
    while((a_svf->oversample_iterator) < 1.0f)
    {
        a_kernel->hp = f_linear_interpolate((a_kernel->filter_last_input), (a_kernel->filter_input), (a_svf->oversample_iterator))
        - (((a_kernel->bp_m1) * (a_svf->filter_res)) + (a_kernel->lp_m1));
        a_kernel->bp = ((a_kernel->hp) * (a_svf->cutoff_filter)) + (a_kernel->bp_m1);
        a_kernel->lp = ((a_kernel->bp) * (a_svf->cutoff_filter)) + (a_kernel->lp_m1);
        
        a_svf->oversample_iterator = (a_svf->oversample_iterator) + SVF_OVERSAMPLE_STEP_SIZE;
    }    
    
    a_kernel->bp_m1 = f_remove_denormal((a_kernel->bp));
    a_kernel->lp_m1 = f_remove_denormal((a_kernel->lp));    
    a_kernel->filter_last_input = a_input_value;     
}

inline void v_svf2_run_2_pole_lp(t_svf2_filter*__restrict a_svf, float a_in0, float a_in1)
{
    v_svf2_run(a_svf, (a_svf->filter_kernels[0][0]), a_in0);
    v_svf2_run(a_svf, (a_svf->filter_kernels[0][1]), a_in1);
    
    a_svf->output0 = a_svf->filter_kernels[0][0]->lp;
    a_svf->output1 = a_svf->filter_kernels[0][1]->lp;    
}

inline void v_svf2_run_4_pole_lp(t_svf2_filter*__restrict a_svf, float a_in0, float a_in1)
{
    v_svf2_run(a_svf, (a_svf->filter_kernels[0][0]), a_in0);
    v_svf2_run(a_svf, (a_svf->filter_kernels[1][0]), (a_svf->filter_kernels[0][0]->lp));
    
    v_svf2_run(a_svf, (a_svf->filter_kernels[0][1]), a_in1);
    v_svf2_run(a_svf, (a_svf->filter_kernels[1][1]), (a_svf->filter_kernels[0][1]->lp));
    
    a_svf->output0 = a_svf->filter_kernels[1][0]->lp;
    a_svf->output1 = a_svf->filter_kernels[1][1]->lp;
}

inline void v_svf2_run_2_pole_hp(t_svf2_filter*__restrict a_svf, float a_in0, float a_in1)
{
    v_svf2_run(a_svf, (a_svf->filter_kernels[0][0]), a_in0);
    v_svf2_run(a_svf, (a_svf->filter_kernels[0][1]), a_in1);
    
    a_svf->output0 = a_svf->filter_kernels[0][0]->hp;
    a_svf->output1 = a_svf->filter_kernels[0][1]->hp;    
}

inline void v_svf2_run_4_pole_hp(t_svf2_filter*__restrict a_svf, float a_in0, float a_in1)
{
    v_svf2_run(a_svf, (a_svf->filter_kernels[0][0]), a_in0);
    v_svf2_run(a_svf, (a_svf->filter_kernels[1][0]), (a_svf->filter_kernels[0][0]->hp));
    
    v_svf2_run(a_svf, (a_svf->filter_kernels[0][1]), a_in1);
    v_svf2_run(a_svf, (a_svf->filter_kernels[1][1]), (a_svf->filter_kernels[0][1]->hp));
    
    a_svf->output0 = a_svf->filter_kernels[1][0]->hp;
    a_svf->output1 = a_svf->filter_kernels[1][1]->hp;
}

inline void v_svf2_run_2_pole_bp(t_svf2_filter*__restrict a_svf, float a_in0, float a_in1)
{
    v_svf2_run(a_svf, (a_svf->filter_kernels[0][0]), a_in0);
    v_svf2_run(a_svf, (a_svf->filter_kernels[0][1]), a_in1);
    
    a_svf->output0 = a_svf->filter_kernels[0][0]->bp;
    a_svf->output1 = a_svf->filter_kernels[0][1]->bp;   
}

inline void v_svf2_run_4_pole_bp(t_svf2_filter*__restrict a_svf, float a_in0, float a_in1)
{
    v_svf2_run(a_svf, (a_svf->filter_kernels[0][0]), a_in0);
    v_svf2_run(a_svf, (a_svf->filter_kernels[1][0]), (a_svf->filter_kernels[0][0]->bp));
    
    v_svf2_run(a_svf, (a_svf->filter_kernels[0][1]), a_in1);
    v_svf2_run(a_svf, (a_svf->filter_kernels[1][1]), (a_svf->filter_kernels[0][1]->bp));
    
    a_svf->output0 = a_svf->filter_kernels[1][0]->bp;
    a_svf->output1 = a_svf->filter_kernels[1][1]->bp;
}

inline void v_svf2_run_2_pole_notch(t_svf2_filter*__restrict a_svf, float a_in0, float a_in1)
{
    v_svf2_run(a_svf, (a_svf->filter_kernels[0][0]), a_in0);
    v_svf2_run(a_svf, (a_svf->filter_kernels[0][1]), a_in1);    
    
    a_svf->output0 = (a_svf->filter_kernels[0][0]->hp) + (a_svf->filter_kernels[0][0]->lp);
    a_svf->output1 = (a_svf->filter_kernels[0][1]->hp) + (a_svf->filter_kernels[0][1]->lp);
}

inline void v_svf2_run_4_pole_notch(t_svf2_filter*__restrict a_svf, float a_in0, float a_in1)
{
    v_svf2_run(a_svf, (a_svf->filter_kernels[0][0]), a_in0);    
    v_svf2_run(a_svf, (a_svf->filter_kernels[1][0]), (a_svf->filter_kernels[0][0]->hp) + (a_svf->filter_kernels[0][0]->lp));    
    a_svf->output0 = (a_svf->filter_kernels[1][0]->hp) + (a_svf->filter_kernels[1][0]->lp);
    
    v_svf2_run(a_svf, (a_svf->filter_kernels[0][1]), a_in1);    
    v_svf2_run(a_svf, (a_svf->filter_kernels[1][1]), (a_svf->filter_kernels[0][0]->hp) + (a_svf->filter_kernels[0][1]->lp));    
    a_svf->output1 = (a_svf->filter_kernels[1][1]->hp) + (a_svf->filter_kernels[1][1]->lp);
}

inline void v_svf2_run_2_pole_allpass(t_svf2_filter*__restrict a_svf, float a_in0, float a_in1)
{
    v_svf2_run(a_svf, (a_svf->filter_kernels[0][0]), a_in0);    
    a_svf->output0 = (a_svf->filter_kernels[0][0]->hp) + (a_svf->filter_kernels[0][0]->lp)  + (a_svf->filter_kernels[0][0]->bp);
    
    v_svf2_run(a_svf, (a_svf->filter_kernels[0][1]), a_in1);    
    a_svf->output1 = (a_svf->filter_kernels[0][1]->hp) + (a_svf->filter_kernels[0][1]->lp)  + (a_svf->filter_kernels[0][1]->bp);
}

inline void v_svf2_set_cutoff(t_svf2_filter*);
void v_svf2_set_res(t_svf2_filter*,  float);
t_svf2_filter * g_svf2_get(float);
inline void v_svf2_set_cutoff_base(t_svf2_filter*, float);
inline void v_svf2_add_cutoff_mod(t_svf2_filter*, float);
inline void v_svf2_velocity_mod(t_svf2_filter*,float);

/* inline void v_svf2_velocity_mod(t_svf2_filter* a_svf, float a_velocity)
 */
inline void v_svf2_velocity_mod(t_svf2_filter*__restrict a_svf, float a_velocity)
{
    a_svf->velocity_cutoff = ((a_velocity) * .2f) - 24.0f;
    a_svf->velocity_mod_amt = a_velocity * 0.007874016f;
}

/* inline void v_svf2_set_cutoff_base(t_svf2_filter* a_svf, float a_midi_note_number)
 * Set the base pitch of the filter, this will usually correspond to a single GUI knob*/
inline void v_svf2_set_cutoff_base(t_svf2_filter*__restrict a_svf, float a_midi_note_number)
{
    a_svf->cutoff_base = a_midi_note_number;
}

/* inline void v_svf2_add_cutoff_mod(t_svf2_filter* a_svf, float a_midi_note_number)
 * Modulate the filters cutoff with an envelope, LFO, etc...*/
inline void v_svf2_add_cutoff_mod(t_svf2_filter*__restrict a_svf, float a_midi_note_number)
{
    a_svf->cutoff_mod = (a_svf->cutoff_mod) + a_midi_note_number;
}

/* inline void v_svf2_set_cutoff(t_svf2_filter * a_svf)
 * This should be called every sample, otherwise the smoothing and modulation doesn't work properly*/
inline void v_svf2_set_cutoff(t_svf2_filter *__restrict a_svf)
{             
    a_svf->cutoff_note = (a_svf->cutoff_base) + ((a_svf->cutoff_mod) * (a_svf->velocity_mod_amt)) + (a_svf->velocity_cutoff);
    a_svf->cutoff_mod = 0.0f;
    
    /*It hasn't changed since last time, return*/    
    if((a_svf->cutoff_note) == (a_svf->cutoff_last))
        return; 
    
    a_svf->cutoff_last = (a_svf->cutoff_note);
    
    a_svf->cutoff_hz = f_pit_midi_note_to_hz_fast((a_svf->cutoff_note), a_svf->pitch_core); //_svf->cutoff_smoother->last_value);
    
    a_svf->cutoff_filter = (a_svf->pi2_div_sr) * (a_svf->cutoff_hz) * 4.0f;

    /*prevent the filter from exploding numerically, this does artificially cap the cutoff frequency to below what you set it to
     if you lower the oversampling rate of the filter.*/
    if((a_svf->cutoff_filter) > 0.8f)
        a_svf->cutoff_filter = 0.8f;  
}

/* void v_svf2_set_res(
 * t_svf2_filter * a_svf, 
 * float a_db)   //-100 to 0 is the expected range
 * 
 */
void v_svf2_set_res(t_svf2_filter *__restrict a_svf, float a_db)
{
    /*Don't calculate it again if it hasn't changed*/
    if((a_svf->filter_res_db) == a_db)
        return;
    
    if(a_db < -100.0f)
    {
        a_svf->filter_res_db = -100.0f;
    }
    else if (a_db > -.5f)
    {
        a_svf->filter_res_db = -.5f;
    }
    else
    {
        a_svf->filter_res_db = a_db;
    }

       a_svf->filter_res = (1.0f - (f_db_to_linear_fast((a_svf->filter_res_db), a_svf->amp_ptr))) * 2.0f;
}

/* t_svf2_filter * g_svf2_get(float a_sample_rate)
 */
t_svf2_filter * g_svf2_get(float a_sample_rate)
{
    t_svf2_filter * f_svf;
    
    if(posix_memalign((void**)&f_svf, 16, (sizeof(t_svf2_filter))) != 0)
    {
        return 0;
    }
    
    f_svf->sr = a_sample_rate * ((float)(SVF_OVERSAMPLE_MULTIPLIER));
    f_svf->pi2_div_sr = (PI2 / (f_svf->sr));
    
    int f_i = 0;
    
    while(f_i < SVF_MAX_CASCADE)
    {       
        f_svf->filter_kernels[f_i][0] = g_svf2_get_filter_kernel();
        f_svf->filter_kernels[f_i][1] = g_svf2_get_filter_kernel();        
        f_i++;
    }
    
    f_svf->cutoff_note = 60.0f;
    f_svf->cutoff_hz = 1000.0f;
    f_svf->cutoff_filter = 0.7f;
    f_svf->filter_res = 0.25f;
    f_svf->filter_res_db = -12.0f;        
      
    f_svf->cutoff_base = 78.0f; 
    f_svf->cutoff_mod = 0.0f;
    f_svf->cutoff_last = 81.0f;
    f_svf->filter_res_db = -21.0f;
    f_svf->filter_res = 0.5f;
    f_svf->velocity_cutoff = 0.0f;    
    f_svf->velocity_mod_amt = 1.0f;
    
    f_svf->amp_ptr = g_amp_get();
    f_svf->pitch_core = g_pit_get();
    f_svf->oversample_iterator = 0.0f;
    
    v_svf2_set_cutoff_base(f_svf, 75.0f);
    v_svf2_add_cutoff_mod(f_svf, 0.0f);
    v_svf2_set_res(f_svf, -12.0f);
    v_svf2_set_cutoff(f_svf);
        
    return f_svf;
}


#ifdef	__cplusplus
}
#endif

#endif	/* PYDAW_SVF_STEREO_H */

