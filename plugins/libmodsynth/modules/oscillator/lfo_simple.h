/* 
 * File:   lfo_simple.h
 * Author: Jeff Hubbard
 * 
 * A simple LFO
 *
 * Created on March 29, 2012, 6:20 PM
 */

#ifndef LFO_SIMPLE_H
#define	LFO_SIMPLE_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "osc_simple.h"
#include "../../lib/osc_core.h"
    
typedef struct st_lfs_lfo
{
    float inc, sr, sr_recip, output;
    t_osc_core * osc_core;
    fp_get_osc_func_ptr osc_ptr;
}t_lfs_lfo;

inline void v_lfs_sync(t_lfs_lfo *,float,int);
inline void v_lfs_set(t_lfs_lfo *, float);
inline void v_lfs_run(t_lfs_lfo *);
t_lfs_lfo * g_lfs_get(float);

/* inline void v_lfs_sync(
 * t_lfs_lfo * a_lfs, 
 * float a_phase,  //the phase to resync to.  Range:  0 to .9999
 * int a_type)  //The type of LFO.  See types below
 * 
 * Types:
 * 0 : Off
 * 1 : Sine
 * 2 : Triangle
 */
inline void v_lfs_sync(t_lfs_lfo * a_lfs, float a_phase, int a_type)
{
    a_lfs->osc_core->output = a_phase;
    
    switch(a_type)
    {
        case 0:
            a_lfs->osc_ptr = f_get_osc_off;            
            break;
        case 1:
            a_lfs->osc_ptr = f_get_sine;            
            break;
        case 2:
            a_lfs->osc_ptr = f_get_triangle;     
            break;
    }    
}


/* void v_osc_set_hz(
 * t_lfs_lfo * a_lfs_ptr,  
 * float a_hz)  //the pitch of the oscillator in hz, typically 0.1 to 10000
 * 
 * For setting LFO frequency.
 */
inline void v_lfs_set(t_lfs_lfo * a_lfs_ptr, float a_hz)
{    
    a_lfs_ptr->inc =  a_hz * a_lfs_ptr->sr_recip;    
}

/* inline void v_lfs_run(t_lfs_lfo *)
 */
inline void v_lfs_run(t_lfs_lfo * a_lfs)
{
    v_run_osc(a_lfs->osc_core, (a_lfs->inc));
    a_lfs->output = a_lfs->osc_ptr(a_lfs->osc_core);
}

t_lfs_lfo * g_lfs_get(float a_sr)
{
    t_lfs_lfo * f_result;
    
    if(posix_memalign((void**)&f_result, 16, (sizeof(t_lfs_lfo))) != 0)
    {
        return 0;
    }
    
    f_result->inc = 0;
    f_result->osc_core = g_get_osc_core();
    f_result->osc_ptr = f_get_osc_off;
    f_result->output = 0;
    f_result->sr = a_sr;
    f_result->sr_recip = 1.0f/a_sr;
    
    return f_result;
}


#ifdef	__cplusplus
}
#endif

#endif	/* LFO_SIMPLE_H */

