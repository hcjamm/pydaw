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

#ifndef MODULEX_LIBMODSYNTH_H
#define	MODULEX_LIBMODSYNTH_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "../../libmodsynth/constants.h"
    
/*includes for any libmodsynth modules you'll be using*/
#include "../../libmodsynth/modules/multifx/multifx3knob.h"
#include "../../libmodsynth/lib/osc_core.h"
#include "../../libmodsynth/lib/amp.h"
#include "../../libmodsynth/lib/pitch_core.h"
#include "../../libmodsynth/lib/smoother-linear.h"
#include "../../libmodsynth/lib/smoother-iir.h"
#include "../../libmodsynth/modules/delay/lms_delay.h"
#include "../../libmodsynth/modules/filter/svf.h"
#include "../../libmodsynth/modules/modulation/env_follower.h"
    
#include "../../libmodsynth/modules/delay/reverb.h"
//#include "../../libmodsynth/modules/dynamics/compressor.h"

typedef struct st_modulex_mono_modules
{
    t_mf3_multi * multieffect[8];
    fp_mf3_run fx_func_ptr[8];
        
    t_lms_delay * delay;
    t_state_variable_filter * svf0;
    t_state_variable_filter * svf1;
    t_smoother_iir * time_smoother;
    t_enf_env_follower * env_follower;
    
    t_rvb_reverb * reverb;
    //t_cmp_compressor * compressor;
    
    float current_sample0;
    float current_sample1;
    
    t_smoother_linear * volume_smoother;
    t_smoother_linear * reverb_smoother;
    
    t_amp * amp_ptr;    
}t_modulex_mono_modules;
    

t_modulex_mono_modules * v_modulex_mono_init(float);


/*Initialize any modules that will be run monophonically*/
t_modulex_mono_modules * v_modulex_mono_init(float a_sr)
{
    t_modulex_mono_modules * a_mono = (t_modulex_mono_modules*)malloc(sizeof(t_modulex_mono_modules));
    
    int f_i = 0;
    
    while(f_i < 8)
    {
        a_mono->multieffect[f_i] = g_mf3_get(a_sr);    
        a_mono->fx_func_ptr[f_i] = v_mf3_run_off;
        f_i++;
    }
    
    a_mono->amp_ptr = g_amp_get();
    
    a_mono->delay = g_ldl_get_delay(1, a_sr);
    a_mono->svf0 = g_svf_get(a_sr);
    a_mono->svf1 = g_svf_get(a_sr);
    v_svf_set_res(a_mono->svf0, -18);
    v_svf_set_res(a_mono->svf1, -18);
    a_mono->time_smoother = g_smr_iir_get_smoother();
    a_mono->env_follower = g_enf_get_env_follower(a_sr);
    
    a_mono->reverb = g_rvb_reverb_get(a_sr);
    //a_mono->compressor = g_cmp_get(a_sr);
    
    a_mono->volume_smoother = g_sml_get_smoother_linear(a_sr, 0.0f, -50.0f, 0.001f);
    a_mono->volume_smoother->last_value = 0.0f;
    a_mono->reverb_smoother = g_sml_get_smoother_linear(a_sr, 100.0f, 0.0f, 0.001f);
    a_mono->reverb_smoother->last_value = 0.0f;
    
    return a_mono;
}

/*Define any custom functions here*/



#ifdef	__cplusplus
}
#endif

#endif	/* LIBMODSYNTH_H */

