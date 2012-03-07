/* 
 * File:   one_pole.h
 * Author: jeffh
 *
 * Created on March 5, 2012, 11:06 PM
 */

#ifndef ONE_POLE_H
#define	ONE_POLE_H

#ifdef	__cplusplus
extern "C" {
#endif
    
//#define OPL_DEBUG_MODE
    
#include "../../constants.h"
#include "../../lib/denormal.h"

typedef struct st_opl_one_pole
{
    float a0, a1, b1, x; 
    float output;
    float cutoff;
    float sample_rate;
    float sr_recip;
    float hp;
    
#ifdef OPL_DEBUG_MODE
    int debug_counter;
#endif
}t_opl_one_pole;

inline void v_opl_set_coeff(t_opl_one_pole*, float);
inline void v_opl_run(t_opl_one_pole*, float);
t_opl_one_pole * g_opl_get_one_pole(float);

/*inline void v_opl_set_lowpass(
 * t_opl_one_pole* a_opl, 
 * float a_cutoff //Cutoff in MIDI note number.  Typically 30 to 120
 * )
 */
inline void v_opl_set_coeff(t_opl_one_pole* a_opl, float a_cutoff)
{
    a_opl->cutoff = f_pit_midi_note_to_hz_fast(a_cutoff);
    a_opl->x = exp(-2.0*PI*((a_opl->cutoff)*(a_opl->sr_recip)));
    a_opl->a0 = 1.0-(a_opl->x);
    a_opl->b1 = -(a_opl->x);
}


inline void v_opl_run(t_opl_one_pole* a_opl, float a_input)
{
    a_opl->output = f_remove_denormal(((a_opl->a0)*a_input) - ((a_opl->b1)*(a_opl->output))); 
    a_opl->hp = a_input - (a_opl->output);
    
#ifdef OPL_DEBUG_MODE
    a_opl->debug_counter = (a_opl->debug_counter) + 1;
    
    if((a_opl->debug_counter) >= 100000)
    {
        a_opl->debug_counter = 0;
        
        printf("\n\nOne pole info\n");
        printf("a_opl->a0 == %f\n", a_opl->a0);
        printf("a_opl->a1 == %f\n", a_opl->a1);
        printf("a_opl->b1 == %f\n", a_opl->b1);
        printf("a_opl->cutoff == %f\n", a_opl->cutoff);
        printf("a_opl->hp == %f\n", a_opl->hp);
        printf("a_opl->output == %f\n", a_opl->output);
        printf("a_opl->x == %f\n", a_opl->x);
                
    }
#endif
}

/*t_opl_one_pole * g_opl_get_one_pole(
 * float a_sr  //sample rate
 * ) * 
 */
t_opl_one_pole * g_opl_get_one_pole(float a_sr)
{
    t_opl_one_pole * f_result = (t_opl_one_pole*)malloc(sizeof(t_opl_one_pole));
    
    f_result->a0 = 0;    
    f_result->b1 = 0;
    f_result->output = 0;
    f_result->cutoff = 1000;
    f_result->sample_rate = a_sr;
    f_result->sr_recip = 1/a_sr;
    
#ifdef OPL_DEBUG_MODE
    f_result->debug_counter = 0;
#endif
    
    return f_result;
}

#ifdef	__cplusplus
}
#endif

#endif	/* ONE_POLE_H */

