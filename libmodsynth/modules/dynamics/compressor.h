/* 
 * File:   compressor.h
 * Author: Jeff Hubbard
 * 
 * Purpose:  A complete compressor plugin.  This file is not yet production-ready.
 *
 * Created on March 9, 2012, 9:18 PM
 */

#ifndef COMPRESSOR_H
#define	COMPRESSOR_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#define CPR_DEBUG_MODE

#include "../modulation/env_follower_ar.h"
#include "../../lib/amp.h"
 
/*This is a stereo compressor.  It compresses both channels independently*/
typedef struct st_cpr_compressor
{    
    t_efr_env_follower_ar * efr0;
    t_efr_env_follower_ar * efr1;
    float threshold;
    float ratio;  //compression ratio
    float ratio_linear;  //The actual multiplier
    /*This is a multiplier, not the audio output, for multiplying other audio signals.
     * This allows for effortless side-chaining in the main loop*/
    float output;
    float output_db;  //Output in decibels
    
#ifdef CPR_DEBUG_MODE
    int debug_counter;
#endif
}t_cpr_compressor;

inline void v_cpr_run_compressor(t_cpr_compressor *, float, float);
inline void v_cpr_set_compressor(t_cpr_compressor *, float, float, float, float);
t_cpr_compressor * g_cpr_get_compressor(float);


/* inline void v_cpr_run_compressor(
 * t_cpr_compressor * a_cpr, 
 * float in0, //audio in 0
 * float in1) //audio in 1
 * 
 * This calculates how to compress a stereo signal by the highest channel amplitude.
 * a_cpr->output is populated with a number to multiply by.  Multiply the inputs by this for a compressor,
 * or multiply some other audio signal by it to side-chain.
 */
inline void v_cpr_run_compressor(t_cpr_compressor * a_cpr, float in0, float in1)
{
    v_efr_run(a_cpr->efr0, in0);
    
    if((a_cpr->efr0->out_db) > (a_cpr->threshold))
    {
        a_cpr->output_db = (((a_cpr->threshold) - (a_cpr->efr0->out_db)) * (a_cpr->ratio_linear));
        a_cpr->output = f_db_to_linear_fast((a_cpr->output_db));
    }
    
    v_efr_run(a_cpr->efr1, in1);
    
    if((a_cpr->efr1->out_db) > (a_cpr->threshold))
    {
        a_cpr->output_db = (((a_cpr->threshold) - (a_cpr->efr1->out_db)) * (a_cpr->ratio_linear));
        a_cpr->output = f_db_to_linear_fast((a_cpr->output_db));
    }
}

/* inline void v_cpr_set_compressor(
 * t_cpr_compressor * a_cpr, 
 * float a_ratio, //Ratio, ie:  10 == 10:1
 * float a_threshold,  //In decibels, ie:  -6 or -12
 * float a_attack,   //attack in seconds, ie:  .1
 * float a_release)  //release in seconds, ie:  .3
 */
inline void v_cpr_set_compressor(t_cpr_compressor * a_cpr, float a_ratio, float a_threshold, float a_attack, float a_release)
{
    if(a_ratio != (a_cpr->ratio))
    {
        a_cpr->ratio = a_ratio;
        a_cpr->ratio_linear = (1 - (1/a_ratio));
    }
    
    a_cpr->threshold = a_threshold;
    
    v_efr_set(a_cpr->efr0, a_attack, a_release);
    v_efr_set(a_cpr->efr1, a_attack, a_release);
}

/*t_cpr_compressor * g_crp_get_compressor(
 * float a_sr  //sample rate
 * )*/
t_cpr_compressor * g_cpr_get_compressor(float a_sr)
{
    t_cpr_compressor * f_result = (t_cpr_compressor*)malloc(sizeof(t_cpr_compressor));
    
    f_result->efr0 = g_efr_get(a_sr);
    f_result->efr1 = g_efr_get(a_sr);
    
    f_result->threshold = -3.0f;
    f_result->ratio = 3.0f;
    f_result->output = 1.0f;
    f_result->output_db = 0;
    f_result->ratio_linear = 0.9f;
    
    return f_result;
}


#ifdef	__cplusplus
}
#endif

#endif	/* COMPRESSOR_H */

