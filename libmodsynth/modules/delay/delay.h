/* 
 * File:   delay.h
 * Author: Jeff Hubbard
 * 
 * A simple delay line with provisions for settings delay in fractional measures or seconds. 
 * Meant to provide the basis for more complicated delay modules
 * Created on January 28, 2012, 1:39 PM
 */

#ifndef DELAY_H
#define	DELAY_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#define LMS_DELAY_DEBUG_MODE
    
typedef struct st_delay_simple
{    
    int read_head;
    int write_head;
    int delay_samples;
    float delay_seconds;
    float delay_beats;
    float sample_rate;
    float tempo;
    float tempo_recip;
    float output;
    int count;
    float* buffer;
#ifdef LMS_DELAY_DEBUG_MODE
    int debug_counter;
#endif
}t_delay_simple;

t_delay_simple * g_dly_get_delay(float, float);
t_delay_simple * g_dly_get_delay_tempo(float,float,float);
inline void v_dly_set_delay_seconds(t_delay_simple*,float);
inline void v_dly_set_delay_tempo(t_delay_simple*,float);
inline void v_dly_run_delay(t_delay_simple*,float);


/*inline void v_dly_set_delay(
 * t_delay_simple* a_dly,
 * float a_seconds //delay in seconds, typically .1 to 1
 * )
 */
inline void v_dly_set_delay_seconds(t_delay_simple* a_dly,float a_seconds)
{
    if((a_dly->delay_seconds) != a_seconds)
    {
        a_dly->delay_seconds = a_seconds;
        a_dly->delay_samples = (a_dly->sample_rate) * a_seconds;
    }
}
/*inline void v_dly_set_delay_tempo(
 * t_delay_simple* a_dly, 
 * float a_beats //Delay time in beats.  Typical value:  .25, .5, 1, 2....
 * )
 */
inline void v_dly_set_delay_tempo(t_delay_simple* a_dly, float a_beats)
{
    if((a_dly->delay_beats) != a_beats)
    {
        a_dly->delay_beats = a_beats;
        a_dly->delay_samples = (a_dly->tempo_recip) * a_beats * (a_dly->sample_rate);
    }
}

/*Run the delay for one sample, and update the output sample and input buffer
 * To run with feedback, do something like this:
 * //input the delay with the processed feedback signal
 * v_dly_run_delay(f_delay, f_input + ((v_mod->output) * (f_feedback_amt));
 * //This is doing something with the feedback
 * v_mod_function(v_mod, (f_delay->output));
 * //That's where you can run it through a filter, or any other processing
 */
inline void v_dly_run_delay(t_delay_simple* a_dly,float a_input)
{
    
    a_dly->read_head = (a_dly->write_head) - (a_dly->delay_samples);    
    if((a_dly->read_head) < 0)
    {
        a_dly->read_head = (a_dly->read_head) + (a_dly->count);
    }
    
    a_dly->output = a_dly->buffer[(a_dly->read_head)];
    
    a_dly->write_head = (a_dly->write_head) + 1;
    if((a_dly->write_head) >= (a_dly->count))
    {
        a_dly->write_head = 0;
    }
    
    a_dly->buffer[(a_dly->write_head)] = a_input;
    
#ifdef LMS_DELAY_DEBUG_MODE
    a_dly->debug_counter = (a_dly->debug_counter) + 1;
    
    if((a_dly->debug_counter) >= 100000)
    {
        a_dly->debug_counter = 0;
        
        printf("\n\nDelay debug info:\n");
        printf("a_dly->count == %i\n", (a_dly->count));
        printf("a_dly->delay_beats == %f\n", (a_dly->delay_beats));
        printf("a_dly->delay_samples == %i\n", (a_dly->delay_samples));
        printf("a_dly->delay_seconds == %f\n", (a_dly->delay_seconds));
        printf("a_dly->output == %f\n", (a_dly->output));
        printf("a_dly->read_head == %i\n", (a_dly->read_head));
        printf("a_dly->sample_rate == %f\n", (a_dly->sample_rate));
        printf("a_dly->tempo == %f\n", (a_dly->tempo));
        printf("a_dly->tempo_recip == %f\n", (a_dly->tempo_recip));
        printf("a_dly->write_head == %i\n", (a_dly->write_head));
    }
    
#endif
}

/*t_delay_simple * g_dly_get_delay
 * (float a_max_size, //max size in seconds
 * float a_sr //sample rate
 * )
 */
t_delay_simple * g_dly_get_delay(float a_max_size, float a_sr)
{
    t_delay_simple * f_result = (t_delay_simple*)malloc(sizeof(t_delay_simple));
         
    f_result->read_head = 0;
    f_result->write_head = 0;
    f_result->delay_samples = 0;
    f_result->delay_seconds  = 0;
    f_result->delay_beats = 0;
    f_result->sample_rate = a_sr;
    f_result->tempo = 999;
    f_result->tempo_recip = 999;
    f_result->output = 0;
    
    f_result->count = (int)((a_max_size * a_sr) + 2400);   //add 2400 samples to ensure we don't overrun our buffer
    f_result->buffer = (float*)malloc(sizeof(float) * (f_result->count));
    
    return f_result;
}

/*t_delay_simple * g_dly_get_delay_tempo(
 * float a_tempo,  //tempo in BPM
 * float a_max_size, //max size in beats
 * float a_sr //sample rate
 * )
 */
t_delay_simple * g_dly_get_delay_tempo(float a_tempo, float a_max_size,float a_sr)
{
    t_delay_simple * f_result = (t_delay_simple*)malloc(sizeof(t_delay_simple));
         
    f_result->read_head = 0;
    f_result->write_head = 0;
    f_result->delay_samples = 0;
    f_result->delay_seconds  = 0;
    f_result->delay_beats = 0;
    f_result->sample_rate = a_sr;
    f_result->tempo = a_tempo;
    f_result->tempo_recip = (60.0f/a_tempo);  //convert from beats per minute to seconds per beat
    f_result->output = 0;
    
    f_result->count = (int)((a_max_size * (f_result->tempo_recip) * a_sr) + 2400);  //add 2400 samples to ensure we don't overrun our buffer
    f_result->buffer = (float*)malloc(sizeof(float) * (f_result->count));
    
    return f_result;
}

#ifdef	__cplusplus
}
#endif

#endif	/* DELAY_H */

