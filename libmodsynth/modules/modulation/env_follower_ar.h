/* 
 * File:   env_follower_ar.h
 * Author: Jeff Hubbard
 * 
 * An envelope follower with integrated attack and release
 *
 * Created on March 11, 2012, 5:19 PM
 */

#ifndef ENV_FOLLOWER_AR_H
#define	ENV_FOLLOWER_AR_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#define EFR_DEBUG_MODE
    
#include "../../lib/amp.h"
#include "../../lib/denormal.h"
#include "../../lib/lms_math.h"

typedef struct st_efr_env_follower_ar
{
    float input;
    float input_m1;
    float output_m1;
    float attack;
    float release;
    float a_coeff;
    float r_coeff;
    float out_linear;
    float out_db;
    float sample_rate;
#ifdef EFR_DEBUG_MODE
    int debug_counter;
#endif
}t_efr_env_follower_ar;

inline t_efr_env_follower_ar* g_efr_get(float);
inline void v_efr_run(t_efr_env_follower_ar*,float);
inline void v_efr_set(t_efr_env_follower_ar*,float,float);

/* inline t_efr_env_follower_ar* g_efr_get(
 * float a_sr)  //sample rate
 */
inline t_efr_env_follower_ar* g_efr_get(float a_sr)
{
    t_efr_env_follower_ar * f_result = (t_efr_env_follower_ar*)malloc(sizeof(t_efr_env_follower_ar));
    
    f_result->input = 0.0f;
    f_result->input_m1 = 0.0f;
    f_result->output_m1 = 0.0f;
    f_result->attack = 0.2f;
    f_result->release = 0.2f;
    f_result->a_coeff = 1.0f;
    f_result->r_coeff = 1.0f;
    f_result->out_linear = 1.0f;
    f_result->out_db = 0.0f;
    f_result->sample_rate = a_sr;
    
#ifdef EFR_DEBUG_MODE
    f_result->debug_counter = 0;
#endif
    
    return f_result;
}

/* inline void v_efr_run(t_efr_env_follower_ar* a_efr, float a_input)
 */
inline void v_efr_run(t_efr_env_follower_ar* a_efr, float a_input)
{
    a_efr->input_m1 = f_lms_abs(a_input) - a_efr->out_linear;
    if(a_efr->input_m1 > 0)
    {
        //attack
        a_efr->out_linear = ((a_efr->a_coeff) * (a_efr->input_m1)) + (a_efr->out_linear);
    }
    else
    {
        //release
        a_efr->out_linear = ((a_efr->r_coeff) * (a_efr->input_m1)) + (a_efr->out_linear);
    }
    
    a_efr->out_db =  f_db_to_linear_fast((a_efr->out_linear));

}

/* inline void v_efr_set
 * (t_efr_env_follower_ar* a_efr, 
 * float a_attack, //attack time in seconds
 * float a_release) //release time in seconds
 */
inline void v_efr_set(t_efr_env_follower_ar* a_efr, float a_attack, float a_release)
{
    if((a_efr->attack) != a_attack)
    {        
        a_efr->attack = a_attack;
        
        if((a_efr->a_coeff) < 1.0f)
        {
            a_efr->a_coeff = 1.0f;
        }
        
        a_efr->a_coeff = 0.693147/((a_attack * (a_efr->sample_rate)));        
        
#ifdef EFR_DEBUG_MODE
        printf("\nEFR attack info:\n");
        printf("a_efr->a_coeff == %f:\n", a_efr->a_coeff);
        printf("a_efr->attack == %f:\n", a_efr->attack);
#endif
    }
    
    if((a_efr->release) != a_release)
    {
        a_efr->release = a_release;
        
        if((a_efr->r_coeff) < 1.0f)
        {
            a_efr->r_coeff = 1.0f;
        }
        
        a_efr->r_coeff = 0.693147/((a_release * (a_efr->sample_rate)));        
        
        #ifdef EFR_DEBUG_MODE
        printf("\nEFR release info:\n");
        printf("a_efr->r_coeff == %f:\n", a_efr->r_coeff);
        printf("a_efr->release == %f:\n", a_efr->release);
#endif
    }
}

#ifdef	__cplusplus
}
#endif

#endif	/* ENV_FOLLOWER_AR_H */

