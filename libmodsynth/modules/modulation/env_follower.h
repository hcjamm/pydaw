/* 
 * File:   env_follower.h
 * Author: jeffh
 *
 * Created on March 5, 2012, 8:35 PM
 */

#ifndef ENV_FOLLOWER_H
#define	ENV_FOLLOWER_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include "../filter/one_pole.h"

typedef struct st_enf_env_follower
{
    float input;
    float output_smoothed;    
    t_opl_one_pole * smoother;
}t_enf_env_follower;

t_enf_env_follower * g_enf_get_env_follower(float);
inline void v_enf_run_env_follower(t_enf_env_follower*, float);

inline void v_enf_run_env_follower(t_enf_env_follower * a_enf, float a_input)
{
    
    //Get absolute value.  This is much faster than fabs
    if(a_input < 0)
    {
        a_enf->input = a_input * -1;
    }
    else
    {
        a_enf->input = a_input;
    }
    
    v_opl_run(a_enf->smoother, (a_enf->input));
    
    a_enf->output_smoothed = (a_enf->smoother->output);
}

t_enf_env_follower * g_enf_get_env_follower(float a_sr)
{
    t_enf_env_follower * f_result = (t_enf_env_follower*)malloc(sizeof(t_enf_env_follower));
    
    f_result->input = 0;
    f_result->output_smoothed = 0;
    f_result->smoother = g_opl_get_one_pole(a_sr);
    
    v_opl_set_lowpass(f_result->smoother, 100);  //Set the smoother to 100hz
    
    return f_result;
}

#ifdef	__cplusplus
}
#endif

#endif	/* ENV_FOLLOWER_H */

