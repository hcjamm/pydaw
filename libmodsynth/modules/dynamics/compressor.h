/* 
 * File:   compressor.h
 * Author: Jeff Hubbard
 *
 * Created on March 9, 2012, 9:18 PM
 */

#ifndef COMPRESSOR_H
#define	COMPRESSOR_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#define CPR_DEBUG_MODE

#include "../modulation/env_follower.h"
#include "../modulation/ramp_env.h"
#include "../../lib/amp.h"
 
/*This is a stereo compressor.  It compresses both channels equally if either of them exceeds the threshold*/
typedef struct st_cpr_compressor
{    
    t_enf_env_follower * env_f0;
    t_enf_env_follower * env_f1;
    
    t_ramp_env * ramp_env_attack;
    t_ramp_env * ramp_env_release;
    
    int stage;  //0 = off, 1 = attack, 2 = compress, 3 = release    
    float threshold;
    float difference;
    float difference_release;  //lock in the difference when releasing, otherwise it would jump quite a bit
    float level_db;  //The current level in db of the highest channel
    float ratio;  //compression ratio
    float ratio_linear;  //The actual multiplier
    float attack;
    float release;
    /*This is a multiplier, not the audio output, for multiplying other audio signals.
     * This allows for effortless side-chaining in the main loop*/
    float output;
    
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
    v_enf_run_env_follower(a_cpr->env_f0, in0);
    v_enf_run_env_follower(a_cpr->env_f1, in1);
    
    /*Compress based on the highest value*/
    if((a_cpr->env_f0->output_smoothed) > (a_cpr->env_f1->output_smoothed))
    {
        a_cpr->level_db = (a_cpr->env_f0->output_smoothed);
        a_cpr->difference = (a_cpr->env_f0->output_smoothed) - (a_cpr->threshold);
    }
    else
    {
        a_cpr->level_db = (a_cpr->env_f1->output_smoothed);
        a_cpr->difference = (a_cpr->env_f1->output_smoothed) - (a_cpr->threshold);
    }
    
    /* The key principles here are:
     * When going directly from attack to release, don't retrigger the envelope, set it to the same phase
     * as the envelope this tick of the sample rate clock is at.
     * 
     * This implementation doesn't currently have a variable knee, but it may eventually.
     */
    
    //switch_statement:
    
    switch((a_cpr->stage))
    {
        case 0:
            if((a_cpr->level_db) > (a_cpr->threshold))
            {
                a_cpr->stage = 1;
                v_rmp_retrigger(a_cpr->ramp_env_attack, (a_cpr->attack), 1);
                //goto switch_statement;
            }
            else
            {
                a_cpr->output = 1;
            }
            break;
        case 1:            
            if((a_cpr->level_db) < (a_cpr->threshold))
            {
                a_cpr->stage = 3;
                a_cpr->ramp_env_release->output = (a_cpr->ramp_env_release->output);
                a_cpr->difference_release = (a_cpr->difference) * -1.0f;
                //goto switch_statement;
            }
            
            f_rmp_run_ramp(a_cpr->ramp_env_attack);            
            a_cpr->output = f_db_to_linear_fast((a_cpr->ramp_env_attack->output) * (a_cpr->difference) * (a_cpr->ratio_linear));
            
            if((a_cpr->ramp_env_attack->output) == 1)
            {
                a_cpr->stage = 2;
            }            
            break;
        case 2:
            if((a_cpr->level_db) > (a_cpr->threshold))
            {
                a_cpr->output = f_db_to_linear_fast((a_cpr->difference) * (a_cpr->ratio_linear));
            }
            else
            {                
                a_cpr->stage = 3;
                v_rmp_retrigger(a_cpr->ramp_env_release, (a_cpr->release), 1);
                a_cpr->difference_release = (a_cpr->difference) * -1.0f;
                //goto switch_statement;
            }
            break;
        case 3:            
            if((a_cpr->level_db) > (a_cpr->threshold))
            {
                a_cpr->stage = 1;
                a_cpr->ramp_env_attack->output = (a_cpr->ramp_env_release->output);
                
                //goto switch_statement;
            }
            
            f_rmp_run_ramp(a_cpr->ramp_env_release);
            
            a_cpr->output = f_db_to_linear_fast((a_cpr->ramp_env_release->output) * (a_cpr->difference_release));
            
            if((a_cpr->ramp_env_attack->output) == 1)
            {
                a_cpr->stage = 0;
            }
            break;
    }
    
#ifdef CPR_DEBUG_MODE
    a_cpr->debug_counter = (a_cpr->debug_counter) + 1;
    
    if((a_cpr->debug_counter) >= 100000)
    {
        a_cpr->debug_counter = 0;
        
        printf("\n\nCompressor info:\n");
        printf("a_cpr->attack == %f\n", (a_cpr->attack));
        printf("a_cpr->difference == %f\n", (a_cpr->difference));        
        printf("a_cpr->level_db == %f\n", (a_cpr->level_db));
        printf("a_cpr->output == %f\n", (a_cpr->output));
        printf("a_cpr->ratio == %f\n", (a_cpr->ratio));
        printf("a_cpr->ratio_linear == %f\n", (a_cpr->ratio_linear));
        printf("a_cpr->release == %f\n", (a_cpr->release));
        printf("a_cpr->stage == %i\n", (a_cpr->stage));
        printf("a_cpr->threshold == %f\n", (a_cpr->threshold));
    }
#endif
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
        a_cpr->ratio_linear = ((1 - (1/a_ratio)) * -1);
    }
    
    a_cpr->threshold = a_threshold;
    
    if(a_attack != (a_cpr->attack))
    {
        a_cpr->attack = a_attack;
        v_rmp_set_time(a_cpr->ramp_env_attack, a_attack);
    }
    
    if(a_release != (a_cpr->release))
    {
        a_cpr->release = a_release;
        v_rmp_set_time(a_cpr->ramp_env_release, a_release);
    }    
}

/*t_cpr_compressor * g_crp_get_compressor(
 * float a_sr  //sample rate
 * )*/
t_cpr_compressor * g_cpr_get_compressor(float a_sr)
{
    t_cpr_compressor * f_result = (t_cpr_compressor*)malloc(sizeof(t_cpr_compressor));
    
    f_result->env_f0 = g_enf_get_env_follower(a_sr);
    f_result->env_f1 = g_enf_get_env_follower(a_sr);
    
    f_result->ramp_env_attack = g_rmp_get_ramp_env(a_sr);
    f_result->ramp_env_release = g_rmp_get_ramp_env(a_sr);
    
    f_result->difference = 0.0f;
    f_result->difference_release = 0.0f;
    f_result->stage = 0;
    f_result->threshold = -3.0f;
    f_result->ratio = 3.0f;
    f_result->attack = 0.1f;
    f_result->release = 0.2f;
    f_result->output = 1.0f;
    
    return f_result;
}


#ifdef	__cplusplus
}
#endif

#endif	/* COMPRESSOR_H */

