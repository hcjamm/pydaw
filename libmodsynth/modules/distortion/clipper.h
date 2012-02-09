/* 
 * File:   clipper.h
 * Author: vm-user
 *
 * Created on January 8, 2012, 1:15 PM
 */

#ifndef CLIPPER_H
#define	CLIPPER_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include "../../lib/amp.h"
    
typedef struct st_clipper
{
    float clip_high, clip_low, input_gain, clip_db, in_db;
}t_clipper;

/*Set the values of a clipper struct symmetrically, ie: value of .75 clips at .75 and -.75*/
void v_clp_set_clip_sym(t_clipper *, float);
void v_clp_set_in_gain(t_clipper *, float);
t_clipper * g_clp_get_clipper();
inline float f_clp_clip(t_clipper*, float);


void v_clp_set_clip_sym(t_clipper * a_clp, float a_db)
{
    /*Already set, don't set again*/
    if(a_db == (a_clp->clip_db))
        return;
    
    a_clp->clip_db = a_db;
    
    float f_value = f_db_to_linear_fast(a_db);
    
#ifdef LMS_DEBUG_MODE
        printf("Clipper value == %f", f_value);
#endif

    a_clp->clip_high = f_value;
    a_clp->clip_low = (f_value * -1);
}

void v_clp_set_in_gain(t_clipper * a_clp, float a_db)
{
    if((a_clp->in_db) == a_db)
        return;
    
    a_clp->in_db = a_db;
    
    a_clp->input_gain = f_db_to_linear(a_db);
}

t_clipper * g_clp_get_clipper()
{
    t_clipper * f_result = (t_clipper*)malloc(sizeof(t_clipper));
    
    f_result->clip_high = 1;
    f_result->clip_low = -1;
    f_result->input_gain = 1;
    f_result->in_db = 0;
    
    return f_result;
};


inline float f_clp_clip(t_clipper * a_clp, float a_input)
{
    float f_result = a_input * (a_clp->input_gain);
    
    if(f_result > (a_clp->clip_high))
        f_result = (a_clp->clip_high);
    else if(f_result < (a_clp->clip_low))
        f_result = (a_clp->clip_low);
    
    return f_result;
}

#ifdef	__cplusplus
}
#endif

#endif	/* CLIPPER_H */

