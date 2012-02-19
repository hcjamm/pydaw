/* 
 * File:   smoother-linear.h
 * Author: vm-user
 *
 * Created on January 9, 2012, 8:02 PM
 */

#ifndef SMOOTHER_LINEAR_H
#define	SMOOTHER_LINEAR_H

#include "math.h"

/*Comment this out when compiling for a release, as it will waste a lot of CPU*/
//#define SML_DEBUG_MODE

typedef struct st_smoother_linear
{
    float rate;
    float last_value;
    float sample_rate;
    float sr_recip;
    
#ifdef SML_DEBUG_MODE
    int debug_counter;
#endif
}t_smoother_linear;

t_smoother_linear * g_sml_get_smoother_linear(float, float, float, float);
inline void v_sml_set_smoother_glide(t_smoother_linear*, float, float, float);
inline void v_sml_run(t_smoother_linear * a_smoother, float);
inline void v_sml_run_glide(t_smoother_linear*, float);

/*There's not much good reason to change this while the synth is running for controls, so you should only set it here.
 If using this for glide or other things that must be smoothed dynamically, you can use the set method below*/
t_smoother_linear * g_sml_get_smoother_linear(float a_sample_rate, float a_high, float a_low, float a_time_in_seconds)
{
    t_smoother_linear * f_result = (t_smoother_linear*)malloc(sizeof(t_smoother_linear));
    /*Start in the middle, the user can manually set the value if this isn't acceptable*/
    f_result->last_value = (((a_high - a_low) * .5) + a_low);
    
    /*Rate is the time it would take to complete if the knob was all the way counter-clockwise, and then instantly moved all the way clockwise*/
    f_result->rate = (((a_high - a_low ) * a_time_in_seconds) / a_sample_rate);
    
    f_result->sample_rate = a_sample_rate;
    f_result->sr_recip = 1/a_sample_rate;
    
    
#ifdef SML_DEBUG_MODE
    f_result->debug_counter = 0;
#endif
    
    return f_result;
}

/*A special function for using a linear smoother as a glide module, run this at note_on
 TODO:  Add pitchbend, etc... as separate arguments and allow intelligent rate smoothing depending on whether
 glide is actually turned on or not*/
inline void v_sml_set_smoother_glide(t_smoother_linear * a_sml_ptr, float a_target, float a_current, float a_time_in_seconds)
{
        
    //We will essentially turn it off by setting the current value as target
    if(a_time_in_seconds < .05)
    {
        a_sml_ptr->last_value = a_target;
    }
    else
    {
        a_sml_ptr->rate = (((a_target - a_current ) * a_time_in_seconds) * (a_sml_ptr->sr_recip));
        a_sml_ptr->last_value = a_current;
    }
    
    
#ifdef SML_DEBUG_MODE
    a_sml_ptr->debug_counter = 0;
    printf("\n\nGlide Set: \n");
    printf("Last Value  %f\n", (a_sml_ptr->last_value));
    printf("Target  %f\n",a_target);
    printf("Rate  %f\n",a_sml_ptr->rate);
    printf("Time in seconds %f\n\n",a_time_in_seconds);
#endif        
}

inline void v_sml_run(t_smoother_linear * a_smoother, float a_current_value)
{
    /*TODO:  evaluate this more thoroughly.  I'm trying to make it as CPU efficient as possible, since almost every port and/or GUI control will have one.*/
    
    /*Evaluated first because most controls won't be moving most of the time, should consume the fewest cycles*/
    if((a_smoother->last_value) == a_current_value)
    {        
        //Do nothing
    } 
    /*This does waste CPU while knobs are being moved, but it will effectively kill the knobs processing
     once it does reach it's destination value*/
    //else if(fabs(a_current_value - (a_smoother->last_value)) < (a_smoother->rate))
    else if((((a_smoother->rate) > 0) && a_current_value - (a_smoother->last_value) <= (a_smoother->rate))
            || (((a_smoother->rate) < 0) && (a_smoother->last_value) - a_current_value <= (a_smoother->rate)))
    {
        a_smoother->last_value = a_current_value;
    }
    
    /*Doing the actual work*/
    
    else if(a_current_value > (a_smoother->last_value))
    {
        a_smoother->last_value = (a_smoother->last_value) + (a_smoother->rate);        
    }
    
    /*Doing the actual work*/
    else
    {
        a_smoother->last_value = (a_smoother->last_value) - (a_smoother->rate);        
    }
}


inline void v_sml_run_glide(t_smoother_linear * a_smoother, float a_target_value)
{    
    /*Evaluated first because most controls won't be moving most of the time, should consume the fewest cycles*/
    if(((a_smoother->last_value) == a_target_value))
    {        
        //Do nothing
    } 
    /*This does waste CPU while knobs are being moved, but it will effectively kill the knobs processing
     once it does reach it's destination value*/    
    else if((((a_smoother->rate) > 0) && a_target_value - (a_smoother->last_value) <= (a_smoother->rate))
            || (((a_smoother->rate) < 0) && (a_smoother->last_value) - a_target_value <= (a_smoother->rate)))
    {
        a_smoother->last_value = a_target_value;
        
#ifdef SML_DEBUG_MODE
        printf("\n\nGlide complete in %i\n", (a_smoother->debug_counter));
#endif
    }
    /*Doing the actual work*/
    else
    {
        a_smoother->last_value = (a_smoother->last_value) + (a_smoother->rate);
#ifdef SML_DEBUG_MODE
        a_smoother->debug_counter = (a_smoother->debug_counter) + 1;
        
        /*Every 2000 samples, print the current value*/
        if(((a_smoother->debug_counter) % 2000) == 0)
        {
            printf("Last glide value == %f\n", (a_smoother->last_value));
        }
#endif
    }
}


#endif	/* SMOOTHER_LINEAR_H */

