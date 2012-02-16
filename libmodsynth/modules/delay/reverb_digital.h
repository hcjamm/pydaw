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

#define REVERB_DIGITAL_BUFFER 100000
#define REVERB_DIGITAL_COMB_FILTER_COUNT 7
    
typedef struct st_reverb_digital{
    float input_buffer [REVERB_DIGITAL_BUFFER];
    int buffer_pos;
    int current_buffer_pos;  //for arbitrarily iterating through the array    
    int pre_delay;
    float feedback_linear;
    float feedback_db;
    int spread_linear;
    float spread_semitones;  //Semitones converted to samples-per-second in spread_linear, because it sounds more musical
    float output;
    float wet_db;
    float wet_linear;
    int comb_iterator;
    float sample_rate;
    float comb_tilt;  //The tilting of the feedback, in decibels-per-tap.  Be careful with positive values
    float comb_amp [REVERB_DIGITAL_COMB_FILTER_COUNT];
    float last_time;  //For checking the last time set, to avoid recalculating reverb time
}t_reverb_digital;

t_reverb_digital * g_rvd_get_reverb_digital(float);
inline void v_rvd_run(t_reverb_digital *, float);

inline void v_rvd_set_time(t_reverb_digital*, float);
inline void v_rvd_set_wet_db(t_reverb_digital*, float);
inline void v_rvd_set_pre_delay(t_reverb_digital*, float);


t_reverb_digital * g_rvd_get_reverb_digital(float a_sample_rate)
{
    t_reverb_digital * f_result = (t_reverb_digital *)malloc(sizeof(t_reverb_digital));
    
    
    f_result->buffer_pos = 0;    
    f_result->current_buffer_pos = 0;    
    f_result->comb_iterator = 0;  
    f_result->comb_tilt = -2;
    f_result->feedback_db = -6;
    f_result->feedback_linear = .5;
    f_result->output = 0;
    f_result->pre_delay = 0;
    f_result->sample_rate = a_sample_rate;    
    f_result->spread_linear = 150;  //TODO:  align this with a pitch value
    f_result->spread_semitones = 1;
    f_result->wet_db = -6;
    f_result->wet_linear = 0.5;
    
    
    int f_i = 0;
    
    while(f_i < REVERB_DIGITAL_BUFFER)
    {
        f_result->input_buffer[f_i] = 0;
        f_i++;
    }
    
    f_i = 0;
    
    while(f_i < REVERB_DIGITAL_COMB_FILTER_COUNT)
    {
        f_result->comb_amp[f_i] = 0.5;
        f_i++;
    }
    
    
    return f_result;
}

void v_rvd_run(t_reverb_digital * a_rvd, float a_input)
{
    a_rvd->input_buffer[(a_rvd->buffer_pos)] = a_input;
    
    a_rvd->comb_iterator = 0;
    a_rvd->current_buffer_pos = (a_rvd->buffer_pos) - (a_rvd->spread_linear);
    
    while((a_rvd->comb_iterator) < REVERB_DIGITAL_COMB_FILTER_COUNT)
    {
        if((a_rvd->input_buffer[(a_rvd->current_buffer_pos)]) < 0)
        {
            a_rvd->input_buffer[(a_rvd->current_buffer_pos)] = (a_rvd->input_buffer[(a_rvd->current_buffer_pos)]) + REVERB_DIGITAL_BUFFER;
        }
        
        if((a_rvd->input_buffer[(a_rvd->current_buffer_pos)]) >= REVERB_DIGITAL_BUFFER)
        {
            a_rvd->input_buffer[(a_rvd->current_buffer_pos)] = (a_rvd->input_buffer[(a_rvd->current_buffer_pos)]) - REVERB_DIGITAL_BUFFER;
        }
        
        a_rvd->input_buffer[(a_rvd->buffer_pos)] = (a_rvd->input_buffer[(a_rvd->current_buffer_pos)]) * (a_rvd->comb_amp[(a_rvd->comb_iterator)]);
                
        a_rvd->comb_iterator = (a_rvd->comb_iterator) + 1;
        a_rvd->current_buffer_pos = (a_rvd->current_buffer_pos) - (a_rvd->spread_linear);
    }
    
    
    if((a_rvd->buffer_pos) >= REVERB_DIGITAL_BUFFER)
    {
        a_rvd->buffer_pos = 0;
    }
    else
    {
        a_rvd->buffer_pos += 1;
    }
}

/*Currently just a loose estimation, as digital reverbs are a very difficult thing to do right*/
inline void v_rvd_set_time(t_reverb_digital* a_rvd, float a_seconds)
{
    
}

inline void v_rvd_set_wet_db(t_reverb_digital* a_rvd, float a_db)
{
    
}

inline void v_rvd_set_pre_delay(t_reverb_digital* a_rvd, float a_seconds)
{
    
}

#ifdef	__cplusplus
}
#endif

#endif	/* REVERB_DIGITAL_H */

