/* 
 * File:   libmodsynth.h
 This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

 */

#ifndef LIBMODSYNTH_H
#define	LIBMODSYNTH_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "../libmodsynth/constants.h"
    
/*includes for any libmodsynth modules you'll be using*/
#include "../../libmodsynth/modules/multifx/multifx3knob.h"

typedef struct st_modulex_mono_modules
{
    t_mf3_multi * multieffect0;
    fp_mf3_run fx_func_ptr0;    
    
    t_mf3_multi * multieffect1;
    fp_mf3_run fx_func_ptr1;    
    
    t_mf3_multi * multieffect2;
    fp_mf3_run fx_func_ptr2;    
    
    t_mf3_multi * multieffect3;
    fp_mf3_run fx_func_ptr3;    
    
    float current_sample0;
    float current_sample1;
}t_modulex_mono_modules;
    

t_modulex_mono_modules * v_modulex_mono_init(float);


/*Initialize any modules that will be run monophonically*/
t_modulex_mono_modules * v_modulex_mono_init(float a_sr)
{
    t_modulex_mono_modules * a_mono = (t_modulex_mono_modules*)malloc(sizeof(t_modulex_mono_modules));
    a_mono->multieffect0 = g_mf3_get(a_sr);    
    a_mono->fx_func_ptr0 = v_mf3_run_off;
    
    a_mono->multieffect1 = g_mf3_get(a_sr);    
    a_mono->fx_func_ptr1 = v_mf3_run_off;
    
    a_mono->multieffect2 = g_mf3_get(a_sr);    
    a_mono->fx_func_ptr2 = v_mf3_run_off;
    
    a_mono->multieffect3 = g_mf3_get(a_sr);    
    a_mono->fx_func_ptr3 = v_mf3_run_off;
    
    return a_mono;
}

/*Define any custom functions here*/



#ifdef	__cplusplus
}
#endif

#endif	/* LIBMODSYNTH_H */

