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
#define LMS_CMB_DEBUG_MODE

#define CMB_BUFFER_SIZE 2000
    
#include "../../lib/amp.h"
#include "../../lib/pitch_core.h"
#include "../../lib/interpolate-linear.h"
#include "../../lib/smoother-linear.h"
    
typedef struct st_comb_filter
{
    float input_buffer [CMB_BUFFER_SIZE];
    int input_pointer;
    int input_feeback;
    float input_feeback_f;
    float output_sample;
    float wet_db;
    float wet_linear;
    float feedback_db;
    float feedback_linear;
#ifdef LMS_CMB_DEBUG_MODE
    int debug_counter;    
#endif
}t_comb_filter;

inline void v_cmb_set_input(t_comb_filter*,float);
inline void v_cmb_set_all(t_comb_filter*, float,float,float);
t_comb_filter * g_cmb_get_comb_filter(float);

/*This runs the filter.  You can then use the output sample in your plugin*/
inline void v_cmb_set_input(t_comb_filter* a_cmb_ptr,float a_value)
{
    a_cmb_ptr->input_pointer = (a_cmb_ptr->input_pointer) + 1;
    
    if((a_cmb_ptr->input_pointer) >= CMB_BUFFER_SIZE)
    {
        a_cmb_ptr->input_pointer = 0;
    }
    
    a_cmb_ptr->input_feeback = (a_cmb_ptr->input_pointer) - 150;
    
    if((a_cmb_ptr->input_feeback) < 0)
    {
        a_cmb_ptr->input_feeback = (a_cmb_ptr->input_feeback) + CMB_BUFFER_SIZE;
    }
    
    a_cmb_ptr->input_buffer[(a_cmb_ptr->input_pointer)] = a_value + ((a_cmb_ptr->input_buffer[(a_cmb_ptr->input_feeback)]) * .75);
    
    a_cmb_ptr->output_sample = a_value + (a_cmb_ptr->input_buffer[(a_cmb_ptr->input_feeback)]);
}


inline void v_cmb_set_all(t_comb_filter* a_cmb_ptr, float a_wet_db, float a_feedback_db, float a_midi_note_number)
{
    
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
    f_result->output_sample = 0.0f;
    f_result->input_feeback = 150;
            
    return f_result;
}


#ifdef	__cplusplus
}
#endif


#endif	/* COMB_FILTER_H */

