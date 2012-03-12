/* 
 * File:   eq_biquad.h
 * Author: Jeff Hubbard
 * 
 * This file is not yet implemented.
 *
 * Created on January 28, 2012, 1:36 PM
 */

#ifndef EQ_BIQUAD_H
#define	EQ_BIQUAD_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#define BEQ_DEBUG_MODE
    
#include "../../lib/amp.h"
#include "../../lib/pitch_core.h"

typedef struct st_beq_eq
{
    //TODO
#ifdef BEQ_DEBUG_MODE
    int debug_counter;
#endif
}t_beq_eq;

inline void v_beq_set_eq(t_beq_eq*,float,float,float);
inline void v_beq_run_eq(t_beq_eq*,float);
t_beq_eq * g_beq_get_eq(float);

/* inline void v_beq_set_eq(
 * t_beq_eq* a_beq, 
 * float a_freq, //Frequency in MIDI note number, typically 20 to 120
 * float a_q,  //Q, in decibels, typically -30 to -1
 * float a_gain)  //Gain in decibels, typically -18 to 18
 */
inline void v_beq_set_eq(t_beq_eq* a_beq, float a_freq, float a_q,float a_gain)
{
    
}

inline void v_beq_run_eq(t_beq_eq* a_beq, float a_in)
{
 
#ifdef BEQ_DEBUG_MODE
    a_beq->debug_counter = (a_beq->debug_counter) + 1;
    
    if((a_beq->debug_counter) >= 100000)
    {
        a_beq->debug_counter = 0;
        printf("\n\nBiquad EQ info:\n");
    }
#endif
}

t_beq_eq * g_beq_get_eq(float)
{
    t_beq_eq * f_result = (t_beq_eq*)malloc(sizeof(t_beq_eq));
    
    //TODO
    
    return f_result;
}

#ifdef	__cplusplus
}
#endif

#endif	/* EQ_BIQUAD_H */

