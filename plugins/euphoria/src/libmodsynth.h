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
#include "../../libmodsynth/lib/pitch_core.h"
#include "../../libmodsynth/lib/smoother-linear.h"
#include "../../libmodsynth/modules/modulation/adsr.h"
#include "../../libmodsynth/modules/modulation/ramp_env.h"
#include "../../libmodsynth/lib/smoother-iir.h"
#include "../../libmodsynth/modules/oscillator/lfo_simple.h"
#include "../../libmodsynth/modules/oscillator/noise.h"
#include "../../libmodsynth/modules/multifx/multifx3knob.h"
    
#define LMS_CHANNEL_COUNT 2
   
/*A call to an audio function that requires no parameters.  Use this for GUI switches when possible, as it will
 require less CPU time than running through if or switch statements.
 Functions from the library that have their own parameters (such as a pointer to 
 their associated struct type as a parameter) should declare their own function pointer types*/
typedef float (*fp_funcptr_audio_generic)();
    
/*Define any modules here that will be used monophonically, ie:  NOT per voice here.  If you are making an effect plugin instead
 of an instrument, you will most likely want to define all of your modules here*/

typedef struct st_mono_modules
{
    t_smoother_iir * pitchbend_smoother;
    t_amp * amp_ptr;
}t_mono_modules;
    
/*define static variables for libmodsynth modules.  Once instance of this type will be created for each polyphonic voice.*/
typedef struct st_poly_voice
{   
    t_state_variable_filter * svf_filter[LMS_CHANNEL_COUNT];
    t_clipper * clipper1[LMS_CHANNEL_COUNT];
    t_audio_xfade * dist_dry_wet[LMS_CHANNEL_COUNT];
    t_white_noise * white_noise1[LMS_CHANNEL_COUNT];
    
    fp_svf_run_filter svf_function;
        
    t_adsr * adsr_filter;
    
    t_adsr * adsr_amp;       
    float noise_amp;
    
    t_smoother_linear * glide_smoother;
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
    float osc1_linamp;
    float osc2_linamp;
    float noise_linamp;           
    int i_voice;  //for the runVoice function to iterate the current block
    
    //From Modulex
    
    t_mf3_multi * multieffect[LMS_MODULAR_POLYFX_COUNT];
    fp_mf3_run fx_func_ptr[LMS_MODULAR_POLYFX_COUNT];
        
    float modulex_current_sample[2];    
    
    float * modulator_outputs[LMS_MODULATOR_COUNT];
    
}t_poly_voice;

#ifdef LMS_DEBUG_MAIN_LOOP

void dump_debug_t_poly_voice(t_poly_voice*);

/*This must be updated if you make any changes to t_poly_voice*/
void dump_debug_t_poly_voice(t_poly_voice* a_data)
{
    
}

#endif

t_poly_voice * g_poly_init(float);

/*initialize all of the modules in an instance of poly_voice*/

t_poly_voice * g_poly_init(float a_sr)
{    
    t_poly_voice * f_voice = (t_poly_voice*)malloc(sizeof(t_poly_voice));
    
    int f_i = 0;
    
    while(f_i < LMS_CHANNEL_COUNT)
    {
        f_voice->svf_filter[f_i] = g_svf_get(a_sr);
        f_voice->white_noise1[f_i] = g_get_white_noise(a_sr); 
        f_voice->clipper1[f_i] = g_clp_get_clipper();    
        f_voice->dist_dry_wet[f_i] = g_axf_get_audio_xfade(-3);
        f_i++;
    }
    
    float f_sr_recip = 1/a_sr;
        
    f_voice->svf_function = svf_get_run_filter_ptr(1, SVF_FILTER_TYPE_LP);
    f_voice->adsr_amp = g_adsr_get_adsr(f_sr_recip);        
    f_voice->adsr_filter = g_adsr_get_adsr(f_sr_recip);
        
    
    f_voice->noise_amp = 0;
        
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
    f_voice->osc1_linamp = 1.0f;
    f_voice->osc2_linamp = 1.0f;
    f_voice->noise_linamp = 1.0f;
    f_voice->i_voice = 0;
    
    //From Modulex
    
    for(f_i = 0; f_i < LMS_MODULAR_POLYFX_COUNT; f_i++)
    {
        f_voice->multieffect[f_i] = g_mf3_get(a_sr);    
        f_voice->fx_func_ptr[f_i] = v_mf3_run_off;
    }
    
    f_voice->modulator_outputs[0] = &f_voice->adsr_amp->output;
    f_voice->modulator_outputs[1] = &f_voice->adsr_filter->output;
    f_voice->modulator_outputs[2] = &f_voice->ramp_env->output;
    f_voice->modulator_outputs[3] = &f_voice->lfo1->output;
    
    return f_voice;
}


void v_poly_note_off(t_poly_voice * a_voice); //, LTS * _instance);

//Define anything that should happen when the user releases a note here
void v_poly_note_off(t_poly_voice * a_voice) //, LTS * _instance)
{
    v_adsr_release(a_voice->adsr_amp);
    v_adsr_release(a_voice->adsr_filter);    
}

t_mono_modules * g_mono_init();


/*Initialize any modules that will be run monophonically*/
t_mono_modules * g_mono_init()
{
    t_mono_modules * a_mono = (t_mono_modules*)malloc(sizeof(t_mono_modules));
    a_mono->pitchbend_smoother = g_smr_iir_get_smoother();
    a_mono->amp_ptr = g_amp_get();
    return a_mono;
}


#ifdef	__cplusplus
}
#endif

#endif	/* LIBMODSYNTH_H */

