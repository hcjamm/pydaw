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

#define CMB_BUFFER_SIZE 2000
    
#include "../../lib/amp.h"
#include "../../lib/pitch_core.h"
#include "../../lib/interpolate-linear.h"
#include "../../lib/smoother-linear.h"
    
typedef struct st_comb_filter
{
    float input_buffer [CMB_BUFFER_SIZE];
    int input_pointer;
    float note_number;
    float wet_db;
    float linear_wet;
    float feedback_db;
    float feedback_linear;
    float sample_fraction;
    int sample_offset;
    t_smoother_linear * pitch_smoother;
}t_comb_filter;

inline void v_cmb_set_input(t_comb_filter*,float);
inline float f_cmb_get_wet_output(t_comb_filter*, float, float, float);
t_comb_filter g_cmb_get_comb_filter(float);

inline void v_cmb_set_input(t_comb_filter* a_cmb_ptr,float a_value)
{
    a_cmb_ptr->input_buffer[(a_cmb_ptr->input_pointer)] = a_value;
    
    a_cmb_ptr->input_pointer = (a_cmb_ptr->input_pointer) + 1;
    
    if((a_cmb_ptr->input_pointer) >= (CMB_BUFFER_SIZE))
    {
        a_cmb_ptr->input_pointer = 0;
    }
}

inline float f_cmb_get_wet_output(t_comb_filter* a_cmb_ptr, float a_midi_note_number, float a_wet_db, float a_feedback_db)
{
    /*This assumes the buffer was already written to before the read was called*/
    if(a_wet_db <= -30)
        return (a_cmb_ptr->input_buffer[(a_cmb_ptr->input_pointer)]);
    
    if(a_wet_db != (a_cmb_ptr->wet_db))
    {
        a_cmb_ptr->wet_db = a_wet_db;
        a_cmb_ptr->linear_wet = f_db_to_linear(a_wet_db);
    }
    
    v_sml_run(a_cmb_ptr->pitch_smoother, a_midi_note_number);
    
    if(a_midi_note_number != (a_cmb_ptr->pitch_smoother->last_value))
    {
        float f_note_to_samples = f_pit_midi_note_to_hz((a_cmb_ptr->pitch_smoother->last_value));
        
        a_cmb_ptr->sample_offset = (int)f_note_to_samples;
        a_cmb_ptr->sample_fraction = f_note_to_samples - (a_cmb_ptr->sample_offset);
    }
    
    if((a_cmb_ptr->feedback_db) != a_feedback_db)
    {
        float f_feedback_db = a_feedback_db;
        
        if(f_feedback_db > -2)
            f_feedback_db = -2;
        
        //TODO:  a less-than logic for not doing feedback at all
        
        a_cmb_ptr->feedback_linear = f_db_to_linear(a_feedback_db);
    }
    
      
    float f_wet_sample;
    float f_wet_sample_minus_1;
    
    int f_wet_sample_index = (a_cmb_ptr->input_pointer) - (a_cmb_ptr->sample_offset);
    int f_wet_sample_index_minus_1 = f_wet_sample_index - 1;
    
    if(f_wet_sample_index < 0)
    {
        f_wet_sample = a_cmb_ptr->input_buffer[(f_wet_sample_index + CMB_BUFFER_SIZE)];
    }
    else
    {
        f_wet_sample = a_cmb_ptr->input_buffer[f_wet_sample_index];
    }
    
    if(f_wet_sample_index_minus_1 < 0)
    {
        f_wet_sample_minus_1 = a_cmb_ptr->input_buffer[(f_wet_sample_index_minus_1 + CMB_BUFFER_SIZE)];
    }
    else
    {
        f_wet_sample_minus_1 = a_cmb_ptr->input_buffer[f_wet_sample_index_minus_1];
    }
    
    float f_wet_result = f_linear_interpolate(f_wet_sample, f_wet_sample_minus_1, (a_cmb_ptr->sample_fraction));
    
    //perform feedback    
    a_cmb_ptr->input_buffer[(a_cmb_ptr->input_pointer)] = (a_cmb_ptr->input_buffer[(a_cmb_ptr->input_pointer)]) + 
            (f_wet_result * (a_cmb_ptr->feedback_linear));
    
    return  (f_wet_result * (a_cmb_ptr->linear_wet)) + (a_cmb_ptr->input_buffer[(a_cmb_ptr->input_pointer)]);    
    
}

t_comb_filter * g_cmb_get_comb_filter(float a_sr)
{
    t_comb_filter * f_result = (t_comb_filter*)malloc(sizeof(t_comb_filter));
    
    f_result->pitch_smoother = g_sml_get_smoother_linear(a_sr, 132, 20, 1);
    f_result->input_pointer = 0;
    f_result->note_number = 60;
    f_result->sample_fraction = 0;
    f_result->wet_db = -30;
        
    return f_result;
}


#ifdef	__cplusplus
}
#endif


#endif	/* COMB_FILTER_H */

