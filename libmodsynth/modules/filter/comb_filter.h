/* 
 * File:   comb_filter.h
 * Author: vm-user
 *
 * Created on January 25, 2012, 8:14 PM
 */

#ifndef COMB_FILTER_H
#define	COMB_FILTER_H


#ifdef	__cplusplus
extern "C" {
#endif
    
/*Comment this out when compiling a release, it will waste a lot of CPU*/
//#define LMS_CMB_DEBUG_MODE

#define CMB_BUFFER_SIZE 2000
    
#include "../../lib/amp.h"
#include "../../lib/pitch_core.h"
#include "../../lib/interpolate-linear.h"
#include "../../lib/smoother-linear.h"
    
typedef struct st_comb_filter
{
    float input_buffer [CMB_BUFFER_SIZE];
    int input_pointer;
    float delay_pointer;
    float wet_sample;
    float output_sample;
    float wet_db;
    float wet_linear;
    float feedback_db;
    float feedback_linear;
    float midi_note_number;
    float delay_samples;
    float sr;
#ifdef LMS_CMB_DEBUG_MODE
    int debug_counter;    
#endif
}t_comb_filter;

inline void v_cmb_set_input(t_comb_filter*,float);
inline void v_cmb_set_all(t_comb_filter*, float,float,float);
t_comb_filter * g_cmb_get_comb_filter(float);

/* v_cmb_set_input(
 * t_comb_filter*,
 * float input value (audio sample, -1 to 1, typically)
 * );
 * This runs the filter.  You can then use the output sample in your plugin*/
inline void v_cmb_set_input(t_comb_filter* a_cmb_ptr,float a_value)
{
    a_cmb_ptr->input_pointer = (a_cmb_ptr->input_pointer) + 1;
    
    if((a_cmb_ptr->input_pointer) >= CMB_BUFFER_SIZE)
    {
        a_cmb_ptr->input_pointer = 0;
    }
    
    a_cmb_ptr->delay_pointer = (a_cmb_ptr->input_pointer) - (a_cmb_ptr->delay_samples);
    
    if((a_cmb_ptr->delay_pointer) < 0)
    {
        a_cmb_ptr->delay_pointer = (a_cmb_ptr->delay_pointer) + CMB_BUFFER_SIZE;
    }
    
    a_cmb_ptr->wet_sample = (f_linear_interpolate_arr_wrap(a_cmb_ptr->input_buffer, 
            CMB_BUFFER_SIZE, (a_cmb_ptr->delay_pointer)));
    
    a_cmb_ptr->input_buffer[(a_cmb_ptr->input_pointer)] = a_value + ((a_cmb_ptr->wet_sample) * (a_cmb_ptr->feedback_linear));
    
    if((a_cmb_ptr->wet_linear) <= -20.0f)
    {
        a_cmb_ptr->output_sample = a_value;
    }
    else
    {
        a_cmb_ptr->output_sample = a_value + ((a_cmb_ptr->wet_sample) * (a_cmb_ptr->wet_linear));
    }
    
#ifdef LMS_CMB_DEBUG_MODE
    a_cmb_ptr->debug_counter = (a_cmb_ptr->debug_counter) + 1;
    
    if((a_cmb_ptr->debug_counter) > 100000)
    {
        a_cmb_ptr->debug_counter = 0;
        printf("\nComb Filter Debug Output:\n");
        printf("a_cmb_ptr->feedback_db == %f\n", (a_cmb_ptr->feedback_db));
        printf("a_cmb_ptr->feedback_linear == %f\n", (a_cmb_ptr->feedback_linear));
        printf("a_cmb_ptr->input_feeback == %f\n", (a_cmb_ptr->delay_pointer));
        printf("a_cmb_ptr->input_pointer == %i\n", (a_cmb_ptr->input_pointer));
        printf("a_cmb_ptr->midi_note_number == %f\n", (a_cmb_ptr->midi_note_number));
        printf("a_cmb_ptr->output_sample == %f\n", (a_cmb_ptr->output_sample));
        printf("a_cmb_ptr->samples == %f\n", (a_cmb_ptr->delay_samples));
        printf("a_cmb_ptr->wet_db == %f\n", (a_cmb_ptr->wet_db));
        printf("a_cmb_ptr->wet_linear == %f\n", (a_cmb_ptr->wet_linear));
        printf("a_cmb_ptr->wet_sample == %f\n", (a_cmb_ptr->wet_sample));
        printf("\n\n");
    }
#endif
    
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
inline void v_cmb_set_all(t_comb_filter* a_cmb_ptr, float a_wet_db, float a_feedback_db, float a_midi_note_number)
{
    /*Set wet_linear, but only if it's changed since last time*/    
    if((a_cmb_ptr->wet_db) != a_wet_db)
    {
        a_cmb_ptr->wet_db = a_wet_db;
        a_cmb_ptr->wet_linear = f_db_to_linear_fast(a_wet_db);
    }
    
    /*Set feedback_linear, but only if it's changed since last time*/    
    if((a_cmb_ptr->feedback_db) != a_feedback_db)
    {
        if(a_feedback_db > -1.0f)
        {
            a_cmb_ptr->feedback_db = -1.0f;
        }
        else
        {
            a_cmb_ptr->feedback_db = a_feedback_db;
        }
        
        a_cmb_ptr->feedback_linear = f_db_to_linear_fast((a_cmb_ptr->feedback_db)); // * -1;  //negative feedback, gives a comb-ier sound
    }
    
    /*Set wet_linear, but only if it's changed since last time*/    
    if((a_cmb_ptr->midi_note_number) != a_midi_note_number)
    {
        a_cmb_ptr->midi_note_number = a_midi_note_number;
        a_cmb_ptr->delay_samples = f_pit_midi_note_to_samples(a_midi_note_number, (a_cmb_ptr->sr));
    }
    
}


t_comb_filter * g_cmb_get_comb_filter(float a_sr)
{
    t_comb_filter * f_result = (t_comb_filter*)malloc(sizeof(t_comb_filter));
    
    int f_i = 0;
    
    while(f_i < CMB_BUFFER_SIZE)
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
    f_result->delay_samples = 150;
    f_result->sr = a_sr;
#ifdef LMS_CMB_DEBUG_MODE
    f_result->debug_counter = 0;
#endif
    
    v_cmb_set_all(f_result,-6.0f, -6.0f, 66.0f);
    v_cmb_set_input(f_result, 0);
            
    return f_result;
}


#ifdef	__cplusplus
}
#endif


#endif	/* COMB_FILTER_H */

