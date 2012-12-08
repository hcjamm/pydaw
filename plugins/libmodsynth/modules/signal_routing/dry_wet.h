/* 
 * File:   dry_wet.h
 * Author: jeffh
 *
 * A control for mixing dry and wet signals.  This is a 2-knob solution, whereas a crossfade is a single knob.
 * 
 * Created on March 5, 2012, 11:22 PM
 */

#ifndef DRY_WET_H
#define	DRY_WET_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include "../../lib/amp.h"

typedef struct st_dw_dry_wet
{
    float wet_db;
    float wet_linear;
    float dry_db;
    float dry_linear;
    float output;
    t_amp * amp_ptr;
}t_dw_dry_wet;

inline void v_dw_set_dry_wet(t_dw_dry_wet*,float,float);
inline void v_dw_run_dry_wet(t_dw_dry_wet*,float,float);
t_dw_dry_wet* g_dw_get_dry_wet();

/*inline void v_dw_set_dry_wet(
 * t_dw_dry_wet* a_dw,
 * float a_dry_db, //dry value in decibels, typically -50 to 0
 * float a_wet_db) //wet value in decibels, typically -50 to 0
 */
inline void v_dw_set_dry_wet(t_dw_dry_wet* a_dw,float a_dry_db,float a_wet_db)
{
    if((a_dw->dry_db) != (a_dry_db))
    {
        a_dw->dry_db = a_dry_db;
        a_dw->dry_linear = f_db_to_linear(a_dry_db, a_dw->amp_ptr);
    }
    
    if((a_dw->wet_db) != (a_wet_db))
    {
        a_dw->wet_db = a_wet_db;
        a_dw->wet_linear = f_db_to_linear(a_wet_db, a_dw->amp_ptr);
    }
}

/* inline void v_dw_run_dry_wet(
 * t_dw_dry_wet* a_dw, 
 * float a_dry, //dry signal
 * float a_wet) //wet signal
 */
inline void v_dw_run_dry_wet(t_dw_dry_wet* a_dw, float a_dry, float a_wet)
{
    a_dw->output = ((a_dw->dry_linear) * a_dry) + ((a_dw->wet_linear) * a_wet);
}

t_dw_dry_wet* g_dw_get_dry_wet()
{
    t_dw_dry_wet* f_result;// = (t_dw_dry_wet*)malloc(sizeof(t_dw_dry_wet));
    
    if(posix_memalign((void**)&f_result, 16, (sizeof(t_dw_dry_wet))) != 0)
    {
        return 0;
    }
    
    f_result->wet_db = -50.0f;
    f_result->wet_linear = 0.0f;
    f_result->dry_db = 0.0f;
    f_result->dry_linear = 1.0f;
    f_result->output = 0.0f;
    f_result->amp_ptr = g_amp_get();
    
    return f_result;
}


#ifdef	__cplusplus
}
#endif

#endif	/* DRY_WET_H */

