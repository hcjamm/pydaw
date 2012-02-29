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
#include "../../libmodsynth/lib/osc_core.h"
#include "../../libmodsynth/lib/pitch_core.h"
#include "../../libmodsynth/lib/smoother-linear.h"
#include "../../libmodsynth/modules/oscillator/osc_simple.h"
#include "../../libmodsynth/modules/oscillator/noise.h"
#include "../../libmodsynth/modules/filter/svf.h"
#include "../../libmodsynth/modules/distortion/clipper.h"
#include "../../libmodsynth/modules/modulation/adsr.h"
#include "../../libmodsynth/modules/signal_routing/audio_xfade.h"
#include "../../libmodsynth/modules/modulation/ramp_env.h"
#include "../../libmodsynth/lib/smoother-iir.h"
   
/*A call to an audio function that requires no parameters.  Use this for GUI switches when possible, as it will
 require less CPU time than running through if or switch statements.
 Functions from the library that have their own parameters (such as a pointer to 
 their associated struct type as a parameter) should declare their own function pointer types*/
typedef float (*fp_funcptr_audio_generic)();
    
/*Declare any static variables that should be used globally in LibModSynth
 Note that any constants not requiring dynamically generated data should be declared in constants.h
 */
static float va_sr_recip;
static float va_sample_rate;

void v_init_lms(float f_sr);

void v_init_lms(float f_sr)
{
    va_sample_rate = f_sr;
    va_sr_recip = 1/f_sr;    
}

/*Define any modules here that will be used monophonically, ie:  NOT per voice here.  If you are making an effect plugin instead
 of an instrument, you will most likely want to define all of your modules here*/

typedef struct st_mono_modules
{
    t_smoother_iir * filter_smoother;
    t_smoother_iir * pitchbend_smoother;
}t_mono_modules;
    
/*define static variables for libmodsynth modules.  Once instance of this type will be created for each polyphonic voice.*/
typedef struct st_poly_voice
{    
    t_osc_simple_unison * osc_unison1;
    t_osc_simple_unison * osc_unison2;
    
    t_state_variable_filter * svf_filter;
    fp_svf_run_filter svf_function;
    
    t_clipper * clipper1;
    t_audio_xfade * dist_dry_wet;
    
    t_adsr * adsr_filter;
    t_white_noise * white_noise1;
    t_adsr * adsr_amp;       
    float noise_amp;
    
    t_smoother_linear * glide_smoother;
    t_ramp_env * glide_env;
    
    t_ramp_env * pitch_env;
    
    float last_pitch;  //For simplicity, this is used whether glide is turned on or not
    
    float base_pitch;  //base pitch for all oscillators, to avoid redundant calculations
    
    float target_pitch;
    
    float filter_output;  //For assigning the filter output to
    
    float current_sample; //This corresponds to the current sample being processed on this voice.  += this to the output buffer when finished.
    
}t_poly_voice;

#ifdef LMS_DEBUG_MAIN_LOOP

void dump_debug_t_poly_voice(t_poly_voice*);

/*This must be updated if you make any changes to t_poly_voice*/
void dump_debug_t_poly_voice(t_poly_voice* a_data)
{
    printf("\n\nRunning dump_debug_t_poly_voice \n");
    printf("adsr_amp->output == %f \n", a_data->adsr_amp->output);
    printf("adsr_filter->output->output == %f \n", a_data->adsr_filter->output);
    printf("clipper1->clip_high == %f \n", a_data->clipper1->clip_high);
    printf("clipper1->clip_low == %f \n", a_data->clipper1->clip_low);
    
    printf("clipper1->input_gain == %f \n", a_data->clipper1->input_gain);
    printf("current_sample == %f \n", a_data->current_sample);
    printf("dist_dry_wet->in1_mult == %f \n", a_data->dist_dry_wet->in1_mult);
    printf("dist_dry_wet->in2_mult == %f \n", a_data->dist_dry_wet->in2_mult);
    printf("glide_smoother1->last_value == %f \n", a_data->glide_smoother->last_value);
    printf("glide_smoother1->rate == %f \n", a_data->glide_smoother->rate);
    printf("noise_amp == %f \n", a_data->noise_amp);    
    printf("osc_unison1->adjusted_amp == %f \n", a_data->osc_unison1->adjusted_amp);
    printf("osc_unison2->adjusted_amp == %f \n", a_data->osc_unison2->adjusted_amp);
    printf("pitch_env->output_multiplied == %f \n", a_data->pitch_env->output_multiplied);    
    //printf("a_data->real_pitch1 == %f \n", a_data->real_pitch);
    printf("svf_filter->cutoff_filter == %f \n", a_data->svf_filter->cutoff_filter);    
    printf("target_pitch1 == %f \n", a_data->target_pitch);
    printf("white_noise1->sample_array[a_data->white_noise1->read_head] == %f \n", a_data->white_noise1->sample_array[a_data->white_noise1->read_head]);
}

#endif

t_poly_voice * g_poly_init();

/*initialize all of the modules in an instance of poly_voice*/

t_poly_voice * g_poly_init()
{
    t_poly_voice * f_voice = (t_poly_voice*)malloc(sizeof(t_poly_voice));
    
    /*TODO:  Remove some of the set values, they were from the early days and aren't needed anymore*/
    
    f_voice->osc_unison1 = g_osc_get_osc_simple_unison(va_sample_rate);    
    f_voice->osc_unison2 = g_osc_get_osc_simple_unison(va_sample_rate);
    
        
    f_voice->svf_filter = g_svf_get(va_sample_rate);
    f_voice->svf_function = svf_get_run_filter_ptr(1, SVF_FILTER_TYPE_LP);
        
    f_voice->clipper1 = g_clp_get_clipper();    
    f_voice->dist_dry_wet = g_axf_get_audio_xfade(-3);
        
    f_voice->adsr_amp = g_adsr_get_adsr(va_sr_recip);        
    f_voice->adsr_filter = g_adsr_get_adsr(va_sr_recip);
        
    f_voice->white_noise1 = g_get_white_noise(va_sample_rate);    
    f_voice->noise_amp = 0;
        
    f_voice->glide_env = g_rmp_get_ramp_env(va_sample_rate);    
    f_voice->pitch_env = g_rmp_get_ramp_env(va_sample_rate);
    
    //f_voice->real_pitch = 60.0f;
    
    f_voice->target_pitch = 66.0f;
    f_voice->last_pitch = 66.0f;
    f_voice->base_pitch = 66.0f;
    
    f_voice->current_sample = 0.0f;
    
    f_voice->filter_output = 0.0f;
    
    return f_voice;
}


void v_poly_note_off(t_poly_voice * a_voice); //, LTS * _instance);

//Define anything that should happen when the user releases a note here
void v_poly_note_off(t_poly_voice * a_voice) //, LTS * _instance)
{
    v_adsr_release(a_voice->adsr_amp);
    v_adsr_release(a_voice->adsr_filter);    
}

t_mono_modules * v_mono_init();


/*Initialize any modules that will be run monophonically*/
t_mono_modules * v_mono_init()
{
    t_mono_modules * a_mono = (t_mono_modules*)malloc(sizeof(t_mono_modules));
    a_mono->filter_smoother = g_smr_iir_get_smoother();
    a_mono->pitchbend_smoother = g_smr_iir_get_smoother();
    return a_mono;
}


#ifdef	__cplusplus
}
#endif

#endif	/* LIBMODSYNTH_H */

