/* 
 * File:   svf.h
 * Author: vm-user
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

typedef struct st_state_variable_filter
{
    t_smoother_linear * cutoff_smoother;
    float bp_m1, lp_m1, cutoff_note, cutoff_hz, cutoff_filter, pi2_div_sr, sr, oversample_div, filter_res, filter_res_db, filter_input, filter_last_input, hp, lp, bp;
    float cutoff_base, cutoff_mod, cutoff_last;  //New additions to fine-tune the modulation process
    int oversample_mult;
    
#ifdef SVF_DEBUG_MODE
    int samples_ran;
#endif
    
} t_state_variable_filter; 


inline void v_svf_set_cutoff(t_state_variable_filter*);
void v_svf_set_res(t_state_variable_filter*,  float);
t_state_variable_filter * g_svf_get(float, int);
inline void v_svf_set_input_value(t_state_variable_filter*, float);
inline void v_svf_set_cutoff_base(t_state_variable_filter*, float);
inline void v_svf_add_cutoff_mod(t_state_variable_filter*, float);

inline void v_svf_set_cutoff_base(t_state_variable_filter* a_svf, float a_midi_note_number)
{
    a_svf->cutoff_base = a_midi_note_number;
}

inline void v_svf_add_cutoff_mod(t_state_variable_filter* a_svf, float a_midi_note_number)
{
    a_svf->cutoff_mod = (a_svf->cutoff_mod) + a_midi_note_number;
}

/*This should be called every sample, otherwise the smoothing and modulation doesn't work properly*/
inline void v_svf_set_cutoff(t_state_variable_filter * a_svf)
{
             
    v_sml_run(a_svf->cutoff_smoother, (a_svf->cutoff_base));
    
    a_svf->cutoff_note = (a_svf->cutoff_smoother->last_value) + (a_svf->cutoff_mod);
     
    /*It hasn't changed since last time, return*/    
    if((a_svf->cutoff_note) == (a_svf->cutoff_last))
        return; 
    
    a_svf->cutoff_last = (a_svf->cutoff_note);
         
    a_svf->cutoff_mod = 0;
    
    a_svf->cutoff_hz = f_pit_midi_note_to_hz_fast((a_svf->cutoff_note)); //_svf->cutoff_smoother->last_value);
    
    a_svf->cutoff_filter = (a_svf->pi2_div_sr) * (a_svf->cutoff_hz) * (a_svf->oversample_div);

    /*prevent the filter from exploding numerically, this does artificially cap the cutoff frequency to below what you set it to
     if you lower the oversampling rate of the filter.*/
    if((a_svf->cutoff_filter) > .8)
        a_svf->cutoff_filter = .8;  
}

void v_svf_set_res(
    t_state_variable_filter * a_svf,
    float a_db  //-100 to 0 is the expected range
    )
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

       a_svf->filter_res = (1 - (f_db_to_linear_fast(a_db))) * 2;
}



//The main action to run the filter
inline void v_svf_set_input_value(t_state_variable_filter * a_svf, float a_input_value)
{
    a_svf->filter_input = a_input_value;
    float f_position = 0;

    int f_i = 0;
    
    while(f_i < (a_svf->oversample_mult))
    {
        float f_interpolated_sample = f_linear_interpolate((a_svf->filter_last_input), (a_svf->filter_input), f_position);

        a_svf->hp = f_interpolated_sample - (((a_svf->bp_m1) * (a_svf->filter_res)) + (a_svf->lp_m1));
        a_svf->bp = ((a_svf->hp) * (a_svf->cutoff_filter)) + (a_svf->bp_m1);
        a_svf->lp = ((a_svf->bp) * (a_svf->cutoff_filter)) + (a_svf->lp_m1);

        a_svf->bp_m1 = f_remove_denormal((a_svf->bp));
        a_svf->lp_m1 = f_remove_denormal((a_svf->lp));

        f_position += (a_svf->oversample_mult);
        
        f_i++;
    }
    
#ifdef SVF_DEBUG_MODE
    
    
    if(((a_svf->lp) > 1000) || ((a_svf->lp) < -1000)    
    || ((a_svf->bp) > 1000) || ((a_svf->bp) < -1000)    
    ||  ((a_svf->hp) > 1000) || ((a_svf->hp) < -1000))
    {
        //printf("hp > 1000, resetting to 0. samples run: %i\n\n", (a_svf->samples_ran));
        //a_svf->hp = 0;
        printf("sr == %f\n", a_svf->sr);
        printf("pi2_div_sr == %f\n", a_svf->pi2_div_sr);
        printf("oversample_mult == %i\n", a_svf->oversample_mult);
        printf("oversample_div == %f\n\n", a_svf->oversample_div);
        printf("cutoff_smoother->last_value == %f\n", a_svf->cutoff_smoother->last_value);
        printf("bp == %f\n", a_svf->bp);
        printf("bp_m1 == %f\n", a_svf->bp_m1);
        printf("hp == %f\n", a_svf->hp);
        printf("lp == %f\n", a_svf->lp);
        printf("lp_m1 == %f\n", a_svf->lp_m1);
        printf("cutoff_note == %f\n", a_svf->cutoff_note);
        printf("cutoff_hz == %f\n", a_svf->cutoff_hz);
        printf("cutoff_filter == %f\n", a_svf->cutoff_filter);
        printf("filter_res == %f\n", a_svf->filter_res);
        printf("filter_res_db == %f\n", a_svf->filter_res_db);
        printf("filter_input == %f\n", a_svf->filter_input);
        printf("filter_last_input == %f\n", a_svf->filter_last_input);
        printf("hp == %f\n", a_svf->hp);
        printf("lp == %f\n", a_svf->lp);
        printf("bp == %f\n", a_svf->bp);
        printf("cutoff_base == %f\n", a_svf->cutoff_base);
        printf("cutoff_mod == %f\n", a_svf->cutoff_mod);
        printf("cutoff_last == %f\n", a_svf->cutoff_last);

    }
    
    if((a_svf->samples_ran) == INT_MAX)
        a_svf->samples_ran = 0;
    else
        a_svf->samples_ran++;    
#endif
}


/*instantiate a new pointer to a state variable filter*/
t_state_variable_filter * g_svf_get(float a_sample_rate, int a_oversample)
{
    t_state_variable_filter * f_svf = (t_state_variable_filter*)malloc(sizeof(t_state_variable_filter));
    f_svf->sr = a_sample_rate * ((float)(a_oversample));
    f_svf->pi2_div_sr = (PI2 / (f_svf->sr));
    f_svf->oversample_mult = a_oversample;
    f_svf->oversample_div = (1/((float)(f_svf->oversample_mult))); 
    f_svf->cutoff_smoother = g_sml_get_smoother_linear(a_sample_rate, 130, 30, 2);
    f_svf->bp = 0;
    f_svf->bp_m1 = 0;
    f_svf->hp = 0;
    f_svf->lp = 0;
    f_svf->lp_m1 = 0;
    f_svf->cutoff_note = 60;
    f_svf->cutoff_hz = 1000;
    f_svf->cutoff_filter = .7;
    f_svf->filter_res = .25;
    f_svf->filter_res_db = -12;        
    f_svf->filter_input = 0;
    f_svf->filter_last_input = 0;    
    f_svf->cutoff_base = 78; 
    f_svf->cutoff_mod = 0;
    f_svf->cutoff_last = 81;
    f_svf->filter_res_db = -21;
    f_svf->filter_res = .5;
    
#ifdef SVF_DEBUG_MODE    
        f_svf->samples_ran = 0;    
#endif
    
    v_svf_set_cutoff_base(f_svf, 75);
    v_svf_add_cutoff_mod(f_svf, 0);
    v_svf_set_res(f_svf, -12);
    v_svf_set_cutoff(f_svf);
    
    v_svf_set_input_value(f_svf, 0);
    
    return f_svf;
}


#ifdef	__cplusplus
}
#endif

#endif	/* SVF_H */

