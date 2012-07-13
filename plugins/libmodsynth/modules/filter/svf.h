/* 
 * File:   svf.h
 * Author: Jeff Hubbard
 * 
 * Purpose:  provides t_state_variable_filter, a multimode filter with lowpass, highpass and bandpass types,
 * in 2 or 4 pole configurations
 * 
 * Usage:
 * 
 * t_state_variable_filter * svf = g_svf_get(44100);
 * //set resonance
 * void v_svf_set_res(svf, -6);
 * //Set the value from your control (probably better to run it through a smoother first)
 * inline void v_svf_set_cutoff_base(svf, (plugin_data->vals.cutoff))
 * //modify it with an envelope
 * inline void v_svf_add_cutoff_mod(svf, ...->adsr->output);
 * //calculate your filter coefficients based on the cutoff base and all mods
 * inline void v_svf_set_cutoff(svf);
 * //run the filter
 * float output = fp_svf_run_filter(svf, input);
 * 
 * The above does not include creating the function pointer or setting up controls to change it.  See LMS Filter for an example
 * 
 *
 * Created on January 8, 2012, 11:35 AM
 */

#ifndef SVF_H
#define	SVF_H

#ifdef	__cplusplus
extern "C" {
#endif

/*This should be commented out if releasing a plugin, it will waste a lot of CPU printing debug information to the console that users shouldn't need.*/
//#define SVF_DEBUG_MODE

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
    
/*The maximum number of filter kernels to cascade.  Multiply this times 2 to get the number of poles the filter will have.
 To people using the library or forking the plugins I've written:  
 */
#define SVF_MAX_CASCADE 2
/*I set this to only 2 for a reason;  People don't want or need more than 4 poles in their filter, it's just not musical.
 You may also be wondering why I used separate methods for getting 2 or 4 poles, rather than using language features like iteration for unlimited scalability.
 There reasons are:
 a.  People really don't need more than 4 poles if they actually want to sound good
 b.  It saves a whole lot of CPU not using these features, especially since no reasonable person is going to create a 16 pole filter
 
 The official aim of LibModSynth is to create quality plugins that are easy to to make music with, not monstrosities with
 way too many knobs and buttons.  You are strongly discouraged (but still completely within your rights) from making
 plugins that are overly complicated, and encouraged to make plugins that sound good, and offer simplied, easy-to-use controls*/


/*Changing this only affects initialization of the filter, you must still change the code in v_svf_set_input_value()*/
#define SVF_OVERSAMPLE_MULTIPLIER 4
#define SVF_OVERSAMPLE_STEP_SIZE 0.25f
    
/*Provides data storage for the inner-workings of the filter*/
typedef struct st_svf_kernel
{
    float filter_input, filter_last_input, bp_m1, lp_m1, hp, lp, bp;
    
}t_svf_kernel;

/*Provides both filter coefficients and filter kernels to create a complete filter*/
typedef struct st_state_variable_filter
{
    //t_smoother_linear * cutoff_smoother;
    float cutoff_note, cutoff_hz, cutoff_filter, pi2_div_sr, sr, filter_res, filter_res_db, velocity_cutoff; //, velocity_cutoff_amt;
    
    float cutoff_base, cutoff_mod, cutoff_last,  velocity_mod_amt;  //New additions to fine-tune the modulation process
    
    /*For the eq*/
    float gain_db, gain_linear;
    float oversample_iterator;
    /*To create a stereo or greater filter, you would create an additional array of filters for each channel*/
    t_svf_kernel * filter_kernels [SVF_MAX_CASCADE];
    t_amp * amp_ptr;
    t_pit_pitch_core * pitch_core;
#ifdef SVF_DEBUG_MODE
    int samples_ran;
#endif
    
} t_state_variable_filter; 

//Used to switch between values, uses much less CPU than a switch statement at every tick of the samplerate clock
typedef float (*fp_svf_run_filter)(t_state_variable_filter*,float);

/*TODO:  The function pointer and functions do not comply to the naming standard*/

/*The int is the number of cascaded filter kernels*/
inline fp_svf_run_filter svf_get_run_filter_ptr(int,int);

inline void v_svf_set_input_value(t_state_variable_filter*, t_svf_kernel *, float);

inline float v_svf_run_2_pole_lp(t_state_variable_filter*, float);
inline float v_svf_run_4_pole_lp(t_state_variable_filter*, float);

inline float v_svf_run_2_pole_hp(t_state_variable_filter*, float);
inline float v_svf_run_4_pole_hp(t_state_variable_filter*, float);

inline float v_svf_run_2_pole_bp(t_state_variable_filter*, float);
inline float v_svf_run_4_pole_bp(t_state_variable_filter*, float);

inline float v_svf_run_2_pole_eq(t_state_variable_filter*, float);
inline float v_svf_run_4_pole_eq(t_state_variable_filter*, float);

inline float v_svf_run_no_filter(t_state_variable_filter*, float);

inline void v_svf_set_eq(t_state_variable_filter*, float);
inline void v_svf_set_eq4(t_state_variable_filter*, float);

/* inline float v_svf_run_no_filter(
 * t_state_variable_filter* a_svf, 
 * float a_in) //audio input
 * 
 * This is for allowing a filter to be turned off by running a function pointer.  a_in is returned unmodified. 
 */
inline float v_svf_run_no_filter(t_state_variable_filter* a_svf, float a_in)
{
    return a_in;
}

inline void v_svf_set_eq(t_state_variable_filter* a_svf, float a_gain)
{
    if(a_gain != (a_svf->gain_db))
    {
        a_svf->gain_db = a_gain;
        a_svf->gain_linear = f_db_to_linear_fast(a_gain, a_svf->amp_ptr);
    }
}

inline void v_svf_set_eq4(t_state_variable_filter* a_svf, float a_gain)
{
    if(a_gain != (a_svf->gain_db))
    {
        a_svf->gain_db = a_gain;
        a_svf->gain_linear = f_db_to_linear_fast((a_gain * .05), a_svf->amp_ptr);
    }
}

t_svf_kernel * g_svf_get_filter_kernel();

t_svf_kernel * g_svf_get_filter_kernel()
{
    t_svf_kernel * f_result = (t_svf_kernel*)malloc(sizeof(t_svf_kernel));
        
        f_result->bp = 0;    
        f_result->hp = 0;
        f_result->lp = 0;    
        f_result->lp_m1 = 0;
        f_result->filter_input = 0;
        f_result->filter_last_input = 0;  
        f_result->bp_m1 = 0;
        
        return f_result;
}

/* inline fp_svf_run_filter svf_get_run_filter_ptr(
 * int a_cascades, 
 * int a_filter_type)
 * 
 * The int refers to the number of cascaded filter kernels, ie:  a value of 2 == 4 pole filter
 * 
 * Filter types:
 * 
 * SVF_FILTER_TYPE_LP 0
 * SVF_FILTER_TYPE_HP 1
 * SVF_FILTER_TYPE_BP 2 
 */
inline fp_svf_run_filter svf_get_run_filter_ptr(int a_cascades, int a_filter_type)
{
    /*Lowpass*/
    if((a_cascades == 1) && (a_filter_type == SVF_FILTER_TYPE_LP))
    {
        return v_svf_run_2_pole_lp;
    }
    else if((a_cascades == 2) && (a_filter_type == SVF_FILTER_TYPE_LP))
    {
        return v_svf_run_4_pole_lp;
    }
    /*Highpass*/
    else if((a_cascades == 1) && (a_filter_type == SVF_FILTER_TYPE_HP))
    {
        return v_svf_run_2_pole_hp;
    }
    else if((a_cascades == 2) && (a_filter_type == SVF_FILTER_TYPE_HP))
    {
        return v_svf_run_4_pole_hp;
    }
    /*Bandpass*/
    else if((a_cascades == 1) && (a_filter_type == SVF_FILTER_TYPE_BP))
    {
        return v_svf_run_2_pole_bp;
    }
    else if((a_cascades == 2) && (a_filter_type == SVF_FILTER_TYPE_BP))
    {
        return v_svf_run_4_pole_bp;
    }
    /*EQ*/
    else if((a_cascades == 1) && (a_filter_type == SVF_FILTER_TYPE_EQ))
    {
        return v_svf_run_2_pole_eq;
    }
    else if((a_cascades == 2) && (a_filter_type == SVF_FILTER_TYPE_EQ))
    {
        return v_svf_run_4_pole_eq;
    }
    /*This means that you entered invalid settings, if your filter unexpectedly returns a 2 pole lowpass, 
     then check the  settings being fed to it*/
    else
    {
        return v_svf_run_2_pole_lp;
    }
}

/* inline void v_svf_set_input_value(
 * t_state_variable_filter * a_svf, 
 * t_svf_kernel * a_kernel, 
 * float a_input_value) //the audio input to filter
 * 
 * The main action to run the filter kernel*/
inline void v_svf_set_input_value(t_state_variable_filter * a_svf, t_svf_kernel * a_kernel, float a_input_value)
{        
    a_kernel->filter_input = a_input_value;
    
    a_svf->oversample_iterator = 0;
    
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
     
    
#ifdef SVF_DEBUG_MODE
   
#endif
}




inline float v_svf_run_2_pole_lp(t_state_variable_filter* a_svf, float a_input)
{
    v_svf_set_input_value(a_svf, (a_svf->filter_kernels[0]), a_input);
    
    return (a_svf->filter_kernels[0]->lp);
}


inline float v_svf_run_4_pole_lp(t_state_variable_filter* a_svf, float a_input)
{
    v_svf_set_input_value(a_svf, (a_svf->filter_kernels[0]), a_input);
    v_svf_set_input_value(a_svf, (a_svf->filter_kernels[1]), (a_svf->filter_kernels[0]->lp));
    
    return (a_svf->filter_kernels[1]->lp);
}

inline float v_svf_run_2_pole_hp(t_state_variable_filter* a_svf, float a_input)
{
    v_svf_set_input_value(a_svf, (a_svf->filter_kernels[0]), a_input);
    
    return (a_svf->filter_kernels[0]->hp);
}


inline float v_svf_run_4_pole_hp(t_state_variable_filter* a_svf, float a_input)
{
    v_svf_set_input_value(a_svf, (a_svf->filter_kernels[0]), a_input);
    v_svf_set_input_value(a_svf, (a_svf->filter_kernels[1]), (a_svf->filter_kernels[0]->hp));
    
    return (a_svf->filter_kernels[1]->hp);
}


inline float v_svf_run_2_pole_bp(t_state_variable_filter* a_svf, float a_input)
{
    v_svf_set_input_value(a_svf, (a_svf->filter_kernels[0]), a_input);
    
    return (a_svf->filter_kernels[0]->bp);
}


inline float v_svf_run_4_pole_bp(t_state_variable_filter* a_svf, float a_input)
{
    v_svf_set_input_value(a_svf, (a_svf->filter_kernels[0]), a_input);
    v_svf_set_input_value(a_svf, (a_svf->filter_kernels[1]), (a_svf->filter_kernels[0]->bp));
    
    return (a_svf->filter_kernels[1]->bp);
}

inline float v_svf_run_2_pole_eq(t_state_variable_filter* a_svf, float a_input)
{
    v_svf_set_input_value(a_svf, (a_svf->filter_kernels[0]), a_input);
    
    return (((a_svf->filter_kernels[0]->lp) + (a_svf->filter_kernels[0]->hp)) + ((a_svf->filter_kernels[0]->bp) * (a_svf->gain_linear)));
}


inline float v_svf_run_4_pole_eq(t_state_variable_filter* a_svf, float a_input)
{
    v_svf_set_input_value(a_svf, (a_svf->filter_kernels[0]), a_input);
    v_svf_set_input_value(a_svf, (a_svf->filter_kernels[1]), (((a_svf->filter_kernels[0]->lp) + (a_svf->filter_kernels[0]->hp)) + ((a_svf->filter_kernels[0]->bp) * (a_svf->gain_linear))));
    
    return (((a_svf->filter_kernels[1]->lp) + (a_svf->filter_kernels[1]->hp)) + ((a_svf->filter_kernels[1]->bp) * (a_svf->gain_linear)));
}

inline void v_svf_set_cutoff(t_state_variable_filter*);
void v_svf_set_res(t_state_variable_filter*,  float);
t_state_variable_filter * g_svf_get(float);
inline void v_svf_set_cutoff_base(t_state_variable_filter*, float);
inline void v_svf_add_cutoff_mod(t_state_variable_filter*, float);
inline void v_svf_velocity_mod(t_state_variable_filter*,float);

/* inline void v_svf_velocity_mod(t_state_variable_filter* a_svf, float a_velocity)
 */
inline void v_svf_velocity_mod(t_state_variable_filter* a_svf, float a_velocity)
{
    a_svf->velocity_cutoff = ((a_velocity) * .2) - 24;
    a_svf->velocity_mod_amt = a_velocity * 0.007874016;
#ifdef SVF_DEBUG_MODE
    printf("svf->velocity:  %f\n", (a_svf->velocity_cutoff));
#endif
}

/* inline void v_svf_set_cutoff_base(t_state_variable_filter* a_svf, float a_midi_note_number)
 * Set the base pitch of the filter, this will usually correspond to a single GUI knob*/
inline void v_svf_set_cutoff_base(t_state_variable_filter* a_svf, float a_midi_note_number)
{
    a_svf->cutoff_base = a_midi_note_number;
}

/* inline void v_svf_add_cutoff_mod(t_state_variable_filter* a_svf, float a_midi_note_number)
 * Modulate the filters cutoff with an envelope, LFO, etc...*/
inline void v_svf_add_cutoff_mod(t_state_variable_filter* a_svf, float a_midi_note_number)
{
    a_svf->cutoff_mod = (a_svf->cutoff_mod) + a_midi_note_number;
}

/* inline void v_svf_set_cutoff(t_state_variable_filter * a_svf)
 * This should be called every sample, otherwise the smoothing and modulation doesn't work properly*/
inline void v_svf_set_cutoff(t_state_variable_filter * a_svf)
{             
    a_svf->cutoff_note = (a_svf->cutoff_base) + ((a_svf->cutoff_mod) * (a_svf->velocity_mod_amt)) + (a_svf->velocity_cutoff);
     
    /*It hasn't changed since last time, return*/    
    if((a_svf->cutoff_note) == (a_svf->cutoff_last))
        return; 
    
    a_svf->cutoff_last = (a_svf->cutoff_note);
         
    a_svf->cutoff_mod = 0;
    
    a_svf->cutoff_hz = f_pit_midi_note_to_hz_fast((a_svf->cutoff_note), a_svf->pitch_core); //_svf->cutoff_smoother->last_value);
    
    a_svf->cutoff_filter = (a_svf->pi2_div_sr) * (a_svf->cutoff_hz);

    /*prevent the filter from exploding numerically, this does artificially cap the cutoff frequency to below what you set it to
     if you lower the oversampling rate of the filter.*/
    if((a_svf->cutoff_filter) > .8)
        a_svf->cutoff_filter = .8;  
}

/* void v_svf_set_res(
 * t_state_variable_filter * a_svf, 
 * float a_db)   //-100 to 0 is the expected range
 * 
 */
void v_svf_set_res(t_state_variable_filter * a_svf, float a_db)
{
    /*Don't calculate it again if it hasn't changed*/
    if((a_svf->filter_res_db) == a_db)
        return;
    
    
    
    if(a_db < -100)
    {
        a_svf->filter_res_db = -100;
    }
    else if (a_db > -.5)
    {
        a_svf->filter_res_db = -.5;
    }
    else
    {
        a_svf->filter_res_db = a_db;
    }

       a_svf->filter_res = (1 - (f_db_to_linear_fast((a_svf->filter_res_db), a_svf->amp_ptr))) * 2;
}



/* t_state_variable_filter * g_svf_get(float a_sample_rate)
 */
t_state_variable_filter * g_svf_get(float a_sample_rate)
{
    t_state_variable_filter * f_svf = (t_state_variable_filter*)malloc(sizeof(t_state_variable_filter));
    f_svf->sr = a_sample_rate * ((float)(SVF_OVERSAMPLE_MULTIPLIER));
    f_svf->pi2_div_sr = (PI2 / (f_svf->sr));
    
    int f_i = 0;
    
    while(f_i < SVF_MAX_CASCADE)
    {
        f_svf->filter_kernels[f_i] = g_svf_get_filter_kernel();
        
        f_i++;
    }
    
    f_svf->cutoff_note = 60;
    f_svf->cutoff_hz = 1000;
    f_svf->cutoff_filter = .7;
    f_svf->filter_res = .25;
    f_svf->filter_res_db = -12;        
      
    f_svf->cutoff_base = 78; 
    f_svf->cutoff_mod = 0;
    f_svf->cutoff_last = 81;
    f_svf->filter_res_db = -21;
    f_svf->filter_res = .5;
    f_svf->velocity_cutoff = 0;    
    f_svf->velocity_mod_amt = 1;
    
    f_svf->gain_db = 0.0f;
    f_svf->gain_linear = 1.0f;
    
    f_svf->amp_ptr = g_amp_get();
    f_svf->pitch_core = g_pit_get();
    f_svf->oversample_iterator = 0.0f;
#ifdef SVF_DEBUG_MODE    
        f_svf->samples_ran = 0;    
#endif
    
    v_svf_set_cutoff_base(f_svf, 75);
    v_svf_add_cutoff_mod(f_svf, 0);
    v_svf_set_res(f_svf, -12);
    v_svf_set_cutoff(f_svf);
        
    return f_svf;
}


#ifdef	__cplusplus
}
#endif

#endif	/* SVF_H */

