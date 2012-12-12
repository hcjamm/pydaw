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

#ifndef EUPHORIA_LIBMODSYNTH_H
#define	EUPHORIA_LIBMODSYNTH_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "../../libmodsynth/constants.h"
    
/*includes for any libmodsynth modules you'll be using*/
#include "../../libmodsynth/lib/pitch_core.h"
#include "../../libmodsynth/modules/modulation/adsr.h"
#include "../../libmodsynth/modules/modulation/ramp_env.h"
#include "../../libmodsynth/lib/smoother-iir.h"
#include "../../libmodsynth/modules/oscillator/lfo_simple.h"
#include "../../libmodsynth/modules/oscillator/noise.h"
#include "../../libmodsynth/modules/multifx/multifx3knob.h"
#include "../../libmodsynth/lib/interpolate-sinc.h"
#include "../../libmodsynth/modules/filter/dc_offset_filter.h"
    
#include "ports.h"
    
#define EUPHORIA_SINC_INTERPOLATION_POINTS 25
#define EUPHORIA_SINC_INTERPOLATION_POINTS_DIV2 13
    
#define EUPHORIA_CHANNEL_COUNT 2
    
//The number of noise modules to use.  This saves a lot of memory vs. having one per voice, since we don't need 100+ unique noise oscillators
#define EUPHORIA_NOISE_COUNT 16
    
/*Define any modules here that will be used monophonically, ie:  NOT per voice here.  If you are making an effect plugin instead
 of an instrument, you will most likely want to define all of your modules here*/

typedef struct st_euphoria_mono_modules
{
    t_smoother_iir * pitchbend_smoother;
    t_amp * amp_ptr;
    t_sinc_interpolator * sinc_interpolator;
    t_dco_dc_offset_filter * dc_offset_filters[EUPHORIA_CHANNEL_COUNT];
    
    t_mf3_multi * multieffect[EUPHORIA_MONO_FX_GROUPS_COUNT][EUPHORIA_MONO_FX_COUNT];
    fp_mf3_run fx_func_ptr[EUPHORIA_MONO_FX_GROUPS_COUNT][EUPHORIA_MONO_FX_COUNT];
    
    t_white_noise * white_noise1[EUPHORIA_NOISE_COUNT];
    int noise_current_index;
}t_euphoria_mono_modules __attribute__((aligned(16)));
    
/*define static variables for libmodsynth modules.  Once instance of this type will be created for each polyphonic voice.*/
typedef struct st_euphoria_poly_voice
{    
    t_adsr * adsr_filter;
    
    t_adsr * adsr_amp;       
    float noise_amp;
    
    t_ramp_env * glide_env;
    
    t_ramp_env * ramp_env;
    
    float last_pitch;  //For simplicity, this is used whether glide is turned on or not
    
    float base_pitch;  //base pitch for all oscillators, to avoid redundant calculations
    
    float target_pitch;
    
    float filter_output;  //For assigning the filter output to
    
    float current_sample; //This corresponds to the current sample being processed on this voice.  += this to the output buffer when finished.
    
    t_lfs_lfo * lfo1;
        
    t_amp * amp_ptr;

    /*Migrated from the now deprecate voice_data struct*/
    //float   amp;
    float note_f;
    float noise_sample;
    float noise_linamp;           
    int i_voice;  //for the runVoice function to iterate the current block
    
    //From Modulex
    
    t_mf3_multi * multieffect[EUPHORIA_MODULAR_POLYFX_COUNT][EUPHORIA_MAX_SAMPLE_COUNT];
    fp_mf3_run fx_func_ptr[EUPHORIA_MODULAR_POLYFX_COUNT];
        
    float modulex_current_sample[2];    
    
    float * modulator_outputs[EUPHORIA_MODULATOR_COUNT];
    
    fp_noise_func_ptr noise_func_ptr;
    
    int noise_index;
    
}t_euphoria_poly_voice __attribute__((aligned(16)));

t_euphoria_poly_voice * g_euphoria_poly_init(float);

/*initialize all of the modules in an instance of poly_voice*/

t_euphoria_poly_voice * g_euphoria_poly_init(float a_sr)
{    
    t_euphoria_poly_voice * f_voice;
    if(posix_memalign((void**)&(f_voice), 16, (sizeof(t_euphoria_poly_voice))) != 0)
    {
        return 0;
    }
    
    int f_i = 0;
   
    float f_sr_recip = 1.0f/a_sr;

    f_voice->adsr_amp = g_adsr_get_adsr(f_sr_recip);        
    f_voice->adsr_filter = g_adsr_get_adsr(f_sr_recip);
    
    f_voice->noise_amp = 0.0f;
        
    f_voice->glide_env = g_rmp_get_ramp_env(a_sr);    
    f_voice->ramp_env = g_rmp_get_ramp_env(a_sr);
    
    //f_voice->real_pitch = 60.0f;
    
    f_voice->target_pitch = 66.0f;
    f_voice->last_pitch = 66.0f;
    f_voice->base_pitch = 66.0f;
    
    f_voice->current_sample = 0.0f;
    
    f_voice->filter_output = 0.0f;
    
    f_voice->lfo1 = g_lfs_get(a_sr);
        
    f_voice->amp_ptr = g_amp_get();
    
    f_voice->note_f = 1.0f;
    f_voice->noise_sample = 0.0f;
    f_voice->noise_linamp = 1.0f;
    f_voice->i_voice = 0;
    
    //From Modulex
    
    for(f_i = 0; f_i < EUPHORIA_MODULAR_POLYFX_COUNT; f_i++)
    {
        int f_i2;
        for(f_i2 = 0; f_i2 < EUPHORIA_MAX_SAMPLE_COUNT; f_i2++)
        {
            f_voice->multieffect[f_i][f_i2] = g_mf3_get(a_sr);    
        }
        
        f_voice->fx_func_ptr[f_i] = v_mf3_run_off;
    }
    
    f_voice->modulator_outputs[0] = &(f_voice->adsr_amp->output);
    f_voice->modulator_outputs[1] = &(f_voice->adsr_filter->output);
    f_voice->modulator_outputs[2] = &(f_voice->ramp_env->output);
    f_voice->modulator_outputs[3] = &(f_voice->lfo1->output);
    
    f_voice->noise_func_ptr = f_run_noise_off;
    f_voice->noise_index = 0;
    
    return f_voice;
}


void v_euphoria_poly_note_off(t_euphoria_poly_voice * a_voice, int a_fast_release);

void v_euphoria_poly_note_off(t_euphoria_poly_voice * a_voice, int a_fast_release) //, LTS * _instance)
{
    if(a_fast_release)
    {
        v_adsr_set_fast_release(a_voice->adsr_amp);
    }
    else
    {
        v_adsr_release(a_voice->adsr_amp);
    }
    
    v_adsr_release(a_voice->adsr_filter);    
}

t_euphoria_mono_modules * g_euphoria_mono_init(float a_sr);


/*Initialize any modules that will be run monophonically*/
t_euphoria_mono_modules * g_euphoria_mono_init(float a_sr)
{
    t_euphoria_mono_modules * a_mono;
    
    if(posix_memalign((void**)&(a_mono), 16, (sizeof(t_euphoria_mono_modules))) != 0)
    {
        return 0;
    }
    
    a_mono->pitchbend_smoother = g_smr_iir_get_smoother();
    a_mono->amp_ptr = g_amp_get();
    a_mono->sinc_interpolator = g_sinc_get(EUPHORIA_SINC_INTERPOLATION_POINTS, 2000, 8000.0f, a_sr, 0.42f);
    a_mono->noise_current_index = 0;
    
    int f_i;
    
    for(f_i = 0; f_i < EUPHORIA_CHANNEL_COUNT; f_i++)
    {
        a_mono->dc_offset_filters[f_i] = g_dco_get(a_sr);
    }
    
    int f_i2;
    
    for(f_i = 0; f_i < EUPHORIA_MONO_FX_GROUPS_COUNT; f_i++)
    {
        for(f_i2 = 0; f_i2 < EUPHORIA_MONO_FX_COUNT; f_i2++)
        {
            a_mono->multieffect[f_i][f_i2] = g_mf3_get(a_sr);
            a_mono->fx_func_ptr[f_i][f_i2] = v_mf3_run_off;
        }
    }
    
    for(f_i = 0; f_i < EUPHORIA_NOISE_COUNT; f_i++)
    {
        a_mono->white_noise1[f_i] = g_get_white_noise(a_sr);
    }
    
    return a_mono;
}


#ifdef	__cplusplus
}
#endif

#endif	/* LIBMODSYNTH_H */

