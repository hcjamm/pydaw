/* 
 * File:   clipper.h
 * Author: Jeff Hubbard
 * 
 * Purpose:  This file provides a hard-clipper.  This is mostly useful for aggressive
 * sounding distortion effects, although it can be used to clip anything.
 *
 * Created on January 8, 2012, 1:15 PM
 */

#ifndef CLIPPER_H
#define	CLIPPER_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include "../../lib/amp.h"
    
//#define CLP_DEBUG_MODE
    
typedef struct st_clipper
{
    float clip_high, clip_low, input_gain_linear, clip_db, in_db, result;
#ifdef CLP_DEBUG_MODE
    int debug_counter;
#endif
}t_clipper;

/*Set the values of a clipper struct symmetrically, ie: value of .75 clips at .75 and -.75*/
void v_clp_set_clip_sym(t_clipper *, float);
void v_clp_set_in_gain(t_clipper *, float);
t_clipper * g_clp_get_clipper();
inline float f_clp_clip(t_clipper*, float);

/*v_clp_set_clip_sym(
 * t_clipper*,
 * float a_db //Threshold to clip at, in decibel, ie:  -6db = clipping at .5 and -.5
 * )
 */
void v_clp_set_clip_sym(t_clipper * a_clp, float a_db)
{
    /*Already set, don't set again*/
    if(a_db == (a_clp->clip_db))
        return;
    
    a_clp->clip_db = a_db;
    
    float f_value = f_db_to_linear_fast(a_db);
    
#ifdef CLP_DEBUG_MODE
        printf("Clipper value == %f", f_value);
#endif

    a_clp->clip_high = f_value;
    a_clp->clip_low = (f_value * -1);
}

/*void v_clp_set_in_gain(
 * t_clipper*,
 * float a_db   //gain in dB to apply to the input signal before clipping it, usually a value between 0 and 36
 * )
 */
void v_clp_set_in_gain(t_clipper * a_clp, float a_db)
{
    if((a_clp->in_db) == a_db)
        return;
    
    a_clp->in_db = a_db;
    
    a_clp->input_gain_linear = f_db_to_linear_fast(a_db);
}

t_clipper * g_clp_get_clipper()
{
    t_clipper * f_result = (t_clipper*)malloc(sizeof(t_clipper));
    
    f_result->clip_high = 1;
    f_result->clip_low = -1;
    f_result->input_gain_linear = 1;
    f_result->in_db = 0;
    f_result->result = 0;
    
    return f_result;
};

/* inline float f_clp_clip(
 * t_clipper *,
 * float a_input  //value to be clipped
 * )
 * 
 * This function performs the actual clipping, and returns a float
 */
inline float f_clp_clip(t_clipper * a_clp, float a_input)
{
    a_clp->result = a_input * (a_clp->input_gain_linear);
    
    if(a_clp->result > (a_clp->clip_high))
        a_clp->result = (a_clp->clip_high);
    else if(a_clp->result < (a_clp->clip_low))
        a_clp->result = (a_clp->clip_low);
    
#ifdef CLP_DEBUG_MODE
    a_clp->debug_counter = (a_clp->debug_counter) + 1;
    
    if((a_clp->debug_counter) >= 100000)
    {
        a_clp->debug_counter = 0;
        printf("Clipper debug info:\n");
        printf("a_clp->clip_db == %f\n", (a_clp->clip_db));
        printf("a_clp->clip_high == %f\n", (a_clp->clip_high));
        printf("a_clp->clip_low == %f\n", (a_clp->clip_low));
        printf("a_clp->in_db == %f\n", (a_clp->in_db));
        printf("a_clp->input_gain_linear == %f\n", (a_clp->input_gain_linear));
    }
#endif
    
    return (a_clp->result);
}

#ifdef	__cplusplus
}
#endif

#endif	/* CLIPPER_H */

