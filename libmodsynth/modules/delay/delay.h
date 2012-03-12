/* 
 * File:   delay.h
 * Author: Jeff Hubbard
 * 
 * A simple delay line with provisions for settings delay in fractional measures or seconds. 
 * Meant to provide the basis for more complicated delay modules
 * 
 * Created on January 28, 2012, 1:39 PM
 */

#ifndef DELAY_H
#define	DELAY_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include "../../lib/interpolate-linear.h"
#include "../../lib/pitch_core.h"
        
//#define DLY_DEBUG_MODE

/* A tap is used to read from a delay.  You can have as many taps as you want per
 * delay line.
 */
typedef struct st_delay_tap
{
    int read_head;
    int read_head_p1;
    float fraction;
    int delay_samples;
    float delay_seconds;
    float delay_beats;
    float delay_pitch;
    float delay_hz;
    float output;
}t_delay_tap;

   
/* A delay is just a buffer to write audio to, that also maintains information like tempo,
 * sample rate, etc...  Taps are used to read from the delay line, either 
 * non-interpolated (CPU friendly, not suitable for modulation) or interpolated (suitable for modulation)
 */
typedef struct st_delay_simple
{   
    int write_head;    
    float sample_rate;
    float tempo;
    float tempo_recip;    
    int sample_count;    
    float* buffer;
#ifdef DLY_DEBUG_MODE
    int debug_counter;
#endif
}t_delay_simple;


t_delay_simple * g_dly_get_delay(float, float);
t_delay_simple * g_dly_get_delay_tempo(float,float,float);
t_delay_tap * g_dly_get_tap();
inline void v_dly_set_delay_seconds(t_delay_simple*,t_delay_tap*,float);
inline void v_dly_set_delay_lin(t_delay_simple*,t_delay_tap*,float);
inline void v_dly_set_delay_tempo(t_delay_simple*,t_delay_tap*,float);
inline void v_dly_set_delay_pitch(t_delay_simple*,t_delay_tap*,float);
inline void v_dly_set_delay_pitch_fast(t_delay_simple*,t_delay_tap*,float);
inline void v_dly_set_delay_hz(t_delay_simple*,t_delay_tap*,float);
inline void v_dly_run_delay(t_delay_simple*,float);
inline void v_dly_run_tap(t_delay_simple*,t_delay_tap*);
inline void v_dly_run_tap_lin(t_delay_simple*,t_delay_tap*);


/*inline void v_dly_set_delay(
 * t_delay_simple* a_dly,
 * float a_seconds //delay in seconds, typically .1 to 1
 * )
 */
inline void v_dly_set_delay_seconds(t_delay_simple* a_dly, t_delay_tap* a_tap,float a_seconds)
{
    if((a_tap->delay_seconds) != a_seconds)
    {        
        a_tap->delay_seconds = a_seconds;
        a_tap->delay_samples = (int)((a_dly->sample_rate) * a_seconds);
    }
}

/* inline void v_dly_set_delay_lin(
 * t_delay_simple* a_dly,
 * t_delay_tap* a_tap,
 * float a_seconds
 * )
 * 
 * This must be run if running the tap as linear, otherwise you will segfault
 */
inline void v_dly_set_delay_lin(t_delay_simple* a_dly, t_delay_tap* a_tap,float a_seconds)
{
    if((a_tap->delay_seconds) != a_seconds)
    {
        
        a_tap->delay_seconds = a_seconds;
        a_tap->delay_samples = (int)((a_dly->sample_rate) * a_seconds);
        a_tap->fraction = ((a_dly->sample_rate) * a_seconds) - (a_tap->delay_samples);
    }
}


/*inline void v_dly_set_delay_tempo(
 * t_delay_simple* a_dly, 
 * t_delay_tap* a_tap, 
 * float a_beats //Delay time in beats.  Typical value:  .25, .5, 1, 2....
 * )
 */
inline void v_dly_set_delay_tempo(t_delay_simple* a_dly, t_delay_tap* a_tap, float a_beats)
{
    if((a_tap->delay_beats) != a_beats)
    {
        a_tap->delay_beats = a_beats;
        a_tap->delay_samples = (a_dly->tempo_recip) * a_beats * (a_dly->sample_rate);
    }
}

/*inline void v_dly_set_delay_pitch(
 * t_delay_simple* a_dly, 
 * t_delay_tap* a_tap, 
 * float a_pitch)  //Pitch in MIDI note number
 * 
 * This method is very slow because it calculates a more accurate result that the fast method, 
 * it should only be used in things like reverbs, where feedback and pitch are tightly coupled together, and
 * require accuracy.
 */
inline void v_dly_set_delay_pitch(t_delay_simple* a_dly, t_delay_tap* a_tap, float a_pitch)
{
    if((a_tap->delay_pitch) != a_pitch)
    {
        a_tap->delay_pitch = a_pitch;
        a_tap->delay_samples = ((a_dly->sample_rate)/(f_pit_midi_note_to_hz(a_pitch)));
    }
}



/*inline void v_dly_set_delay_pitch(
 * t_delay_simple* a_dly, 
 * t_delay_tap* a_tap, 
 * float a_pitch)  //Pitch in MIDI note number
 * 
 * This method is very slow because it calculates a more accurate result that the fast method, 
 * it should only be used in things like reverbs, where feedback and pitch are tightly coupled together, and
 * require accuracy.
 */
inline void v_dly_set_delay_pitch_fast(t_delay_simple* a_dly, t_delay_tap* a_tap, float a_pitch)
{
    if((a_tap->delay_pitch) != a_pitch)
    {
        a_tap->delay_pitch = a_pitch;
        a_tap->delay_samples = ((a_dly->sample_rate)/(f_pit_midi_note_to_hz(a_pitch)));
    }
}


/*inline void v_dly_set_delay_hz(
 * t_delay_simple* a_dly, 
 * t_delay_tap* a_tap, 
 * float a_hz)  //Frequency in hz.  1/a_hz == the delay time
 * 
 */
inline void v_dly_set_delay_hz(t_delay_simple* a_dly, t_delay_tap* a_tap, float a_hz)
{
    if((a_tap->delay_hz) != a_hz)
    {
        a_tap->delay_hz = a_hz;
        a_tap->delay_samples = ((a_dly->sample_rate)/(a_hz));
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
    a_dly->buffer[(a_dly->write_head)] = a_input;
    
    a_dly->write_head = (a_dly->write_head) + 1;
    if((a_dly->write_head) >= (a_dly->sample_count))
    {
        a_dly->write_head = 0;
    }
    
    
#ifdef DLY_DEBUG_MODE
    a_dly->debug_counter = (a_dly->debug_counter) + 1;
    
    if((a_dly->debug_counter) >= 100000)
    {
        a_dly->debug_counter = 0;
        
        printf("\n\nDelay debug info:\n");
        printf("a_dly->count == %i\n", (a_dly->sample_count));        
        printf("a_dly->sample_rate == %f\n", (a_dly->sample_rate));
        printf("a_dly->tempo == %f\n", (a_dly->tempo));
        printf("a_dly->tempo_recip == %f\n", (a_dly->tempo_recip));
        printf("a_dly->write_head == %i\n", (a_dly->write_head));
    }
    
#endif
}

inline void v_dly_run_tap(t_delay_simple* a_dly,t_delay_tap* a_tap)
{
    a_tap->read_head = (a_dly->write_head) - (a_tap->delay_samples);
    
    if((a_tap->read_head) < 0)
    {
        a_tap->read_head = (a_tap->read_head) + (a_dly->sample_count);
    }
    
    a_tap->output = (a_dly->buffer[(a_tap->read_head)]);
    
#ifdef DLY_DEBUG_MODE
    if((a_dly->debug_counter) == 50000)
    {
        printf("\n\nTap debug info:\n");
        printf("a_tap->delay_beats == %f\n", (a_tap->delay_beats));
        printf("a_tap->delay_samples == %i\n", (a_tap->delay_samples));
        printf("a_tap->delay_seconds == %f\n", (a_tap->delay_seconds));
        printf("a_tap->output == %f\n", (a_tap->output));
        printf("a_tap->read_head == %i\n", (a_tap->read_head));
    }
#endif
}

/* inline void v_dly_run_tap_lin(t_delay_simple* a_dly,t_delay_tap* a_tap)
 * 
 * Run a delay line using linear interpolation.  The delay must have been set using:
 * v_dly_set_delay_lin();
 */
inline void v_dly_run_tap_lin(t_delay_simple* a_dly,t_delay_tap* a_tap)
{
    a_tap->read_head = (a_dly->write_head) - (a_tap->delay_samples);    
        
    if((a_tap->read_head) < 0)
    {
        a_tap->read_head = (a_tap->read_head) + (a_dly->sample_count);
    }
    
    a_tap->read_head_p1 = (a_tap->read_head) + 1;
    
    if((a_tap->read_head_p1) >= (a_dly->sample_count))
    {
        a_tap->read_head_p1 = (a_tap->read_head_p1) - (a_dly->sample_count);
    }
    
    a_tap->output = f_linear_interpolate(
            a_dly->buffer[(a_tap->read_head)], a_dly->buffer[(a_tap->read_head_p1)], (a_tap->fraction));
    
    
#ifdef DLY_DEBUG_MODE
    if((a_dly->debug_counter) == 50000)
    {
        printf("\n\nTap debug info:\n");
        printf("a_tap->delay_beats == %f\n", (a_tap->delay_beats));
        printf("a_tap->delay_samples == %i\n", (a_tap->delay_samples));
        printf("a_tap->delay_seconds == %f\n", (a_tap->delay_seconds));
        printf("a_tap->output == %f\n", (a_tap->output));
        printf("a_tap->read_head == %i\n", (a_tap->read_head));
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
    
    f_result->write_head = 0;
    f_result->sample_rate = a_sr;
    f_result->tempo = 999;
    f_result->tempo_recip = 999;
        
    f_result->sample_count = (int)((a_max_size * a_sr) + 2400);   //add 2400 samples to ensure we don't overrun our buffer
    f_result->buffer = (float*)malloc(sizeof(float) * (f_result->sample_count));
    
    int f_i = 0;
    
    while(f_i < (f_result->sample_count))
    {
        f_result->buffer[f_i] = 0.0f;
        f_i++;
    }
    
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
         
    
    f_result->write_head = 0;    
    f_result->sample_rate = a_sr;
    f_result->tempo = a_tempo;
    f_result->tempo_recip = (60.0f/a_tempo);  //convert from beats per minute to seconds per beat
        
    f_result->sample_count = (int)((a_max_size * (f_result->tempo_recip) * a_sr) + 2400);  //add 2400 samples to ensure we don't overrun our buffer
    f_result->buffer = (float*)malloc(sizeof(float) * (f_result->sample_count));
    
    return f_result;
}

t_delay_tap * g_dly_get_tap()
{
    t_delay_tap * f_result = (t_delay_tap*)malloc(sizeof(t_delay_tap));
    
    f_result->read_head = 0;
    f_result->delay_samples = 0;
    f_result->delay_seconds  = 0;
    f_result->delay_beats = 0;
    f_result->output = 0;
    f_result->delay_pitch = 20.0123f;  //To ensure it doesn't accidentally match the first time
    f_result->delay_hz = 20.2021f;
    
    return f_result;
}

#ifdef	__cplusplus
}
#endif

#endif	/* DELAY_H */

