/*
This file is part of the PyDAW project, Copyright PyDAW Team

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef WAYV_LIBMODSYNTH_H
#define	WAYV_LIBMODSYNTH_H

#ifdef	__cplusplus
extern "C" {
#endif

//Total number of LFOs, ADSRs, other envelopes, etc...  Used for the PolyFX mod matrix
#define WAYV_MODULATOR_COUNT 6
//How many modular PolyFX
#define WAYV_MODULAR_POLYFX_COUNT 4
    
#include "../../libmodsynth/constants.h"
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
#include "../../libmodsynth/modules/oscillator/lfo_simple.h"
#include "../../libmodsynth/modules/oscillator/osc_wavetable.h"
#include "../../libmodsynth/modules/multifx/multifx3knob.h"


typedef struct
{    
    t_smoother_linear * pitchbend_smoother;
    t_amp * amp_ptr;
}t_wayv_mono_modules;
    
/*define static variables for libmodsynth modules.  Once instance of this type will be created for each polyphonic voice.*/
typedef struct
{
    t_osc_wav_unison * osc_wavtable1;
    t_osc_wav_unison * osc_wavtable2;
    t_osc_wav_unison * osc_wavtable3;
    t_wt_wavetables * wavetables;
    
    float osc1_uni_spread, osc2_uni_spread, osc3_uni_spread;
    
    float osc1fm1, osc1fm2, osc1fm3;
    float osc2fm1, osc2fm2, osc2fm3;
    float osc3fm1, osc3fm2, osc3fm3;
    
    float fm1_last, fm2_last, fm3_last;
    
    t_white_noise * white_noise1;
    t_adsr * adsr_main;
    
    t_adsr * adsr_amp1;
    int adsr_amp1_on;
    t_adsr * adsr_amp2;
    int adsr_amp2_on;
    t_adsr * adsr_amp3;
    int adsr_amp3_on;
    
    float noise_amp;
    
    t_smoother_linear * glide_smoother;
    t_ramp_env * glide_env;
        
    float last_pitch;  //For simplicity, this is used whether glide is turned on or not    
    float base_pitch;  //base pitch for all oscillators, to avoid redundant calculations    
    float target_pitch;
    float current_sample; //This corresponds to the current sample being processed on this voice.  += this to the output buffer when finished.    
    
    t_amp * amp_ptr;

    float   amp;
    float note_f;
    int note;
    float osc1_linamp;
    float osc2_linamp;
    float osc3_linamp;
    float noise_linamp;           
    int i_voice;  //for the runVoice function to iterate the current block    
    
    t_mf3_multi * multieffect[WAYV_MODULAR_POLYFX_COUNT]; //[WAYV_MAX_SAMPLE_COUNT];
    fp_mf3_run fx_func_ptr[WAYV_MODULAR_POLYFX_COUNT];        
    float modulex_current_sample[2];        
    float * modulator_outputs[WAYV_MODULATOR_COUNT];    
    fp_noise_func_ptr noise_func_ptr;    
    t_lfs_lfo * lfo1;    
    float lfo_amount_output, lfo_amp_output, lfo_pitch_output;    
    t_adsr * adsr_filter;    
    t_adsr * adsr_amp;    
    t_ramp_env * ramp_env;
    
    float filter_output;
    float noise_sample;
    
    int osc1_on;
    int osc2_on;
    int osc3_on;
    
    float velocity_track;
    float keyboard_track;
    
}t_wayv_poly_voice;

t_wayv_poly_voice * g_wayv_poly_init(float a_sr);

/*initialize all of the modules in an instance of poly_voice*/

t_wayv_poly_voice * g_wayv_poly_init(float a_sr)
{
    float f_sr_recip = 1.0f / a_sr;
    
    t_wayv_poly_voice * f_voice = (t_wayv_poly_voice*)malloc(sizeof(t_wayv_poly_voice));
    
    f_voice->osc_wavtable1 = g_osc_get_osc_wav_unison(a_sr);
    f_voice->osc_wavtable2 = g_osc_get_osc_wav_unison(a_sr);
    f_voice->osc_wavtable3 = g_osc_get_osc_wav_unison(a_sr);
    f_voice->wavetables = g_wt_wavetables_get();    
    
    f_voice->osc1_uni_spread = 0.0f;
    f_voice->osc2_uni_spread = 0.0f;
    f_voice->osc3_uni_spread = 0.0f;
    
    f_voice->osc1fm1 = 0.0;
    f_voice->osc1fm2 = 0.0;
    f_voice->osc1fm3 = 0.0;
    f_voice->osc2fm1 = 0.0;
    f_voice->osc2fm2 = 0.0;
    f_voice->osc2fm3 = 0.0;
    f_voice->osc3fm1 = 0.0;
    f_voice->osc3fm2 = 0.0;
    f_voice->osc3fm3 = 0.0;
    
    f_voice->fm1_last = 0.0;
    f_voice->fm2_last = 0.0;
    f_voice->fm3_last = 0.0;
    
    f_voice->adsr_main = g_adsr_get_adsr(f_sr_recip);
    f_voice->adsr_amp1 = g_adsr_get_adsr(f_sr_recip);
    f_voice->adsr_amp2 = g_adsr_get_adsr(f_sr_recip);
    f_voice->adsr_amp3 = g_adsr_get_adsr(f_sr_recip);
            
    f_voice->white_noise1 = g_get_white_noise(a_sr);    
    f_voice->noise_amp = 0;
        
    f_voice->glide_env = g_rmp_get_ramp_env(a_sr);    
        
    //f_voice->real_pitch = 60.0f;
    
    f_voice->target_pitch = 66.0f;
    f_voice->last_pitch = 66.0f;
    f_voice->base_pitch = 66.0f;
    
    f_voice->current_sample = 0.0f;
    
    f_voice->amp_ptr = g_amp_get();
    
    f_voice->amp = 1.0f;
    f_voice->note_f = 1.0f;
    f_voice->osc1_linamp = 1.0f;
    f_voice->osc2_linamp = 1.0f;
    f_voice->osc3_linamp = 1.0f;
    f_voice->noise_linamp = 1.0f;
    f_voice->i_voice = 0;
    
    f_voice->osc1_on = 0;
    f_voice->osc2_on = 0;
    f_voice->osc3_on = 0;
    
    f_voice->lfo_amount_output = 0.0f;
    f_voice->lfo_amp_output = 0.0f;
    f_voice->lfo_pitch_output = 0.0f;
    
    int f_i = 0;
   
    f_voice->adsr_amp = g_adsr_get_adsr(f_sr_recip);        
    f_voice->adsr_filter = g_adsr_get_adsr(f_sr_recip);
    
    f_voice->noise_amp = 0.0f;
        
    f_voice->glide_env = g_rmp_get_ramp_env(a_sr);    
    f_voice->ramp_env = g_rmp_get_ramp_env(a_sr);
        
    f_voice->filter_output = 0.0f;
    
    f_voice->lfo1 = g_lfs_get(a_sr);
        
    f_voice->noise_sample = 0.0f;
        
    f_voice->adsr_amp1_on = 0;
    f_voice->adsr_amp2_on = 0;
    f_voice->adsr_amp3_on = 0;
        
    for(f_i = 0; f_i < WAYV_MODULAR_POLYFX_COUNT; f_i++)
    {        
        f_voice->multieffect[f_i] = g_mf3_get(a_sr);        
        f_voice->fx_func_ptr[f_i] = v_mf3_run_off;
    }
    
    f_voice->modulator_outputs[0] = &(f_voice->adsr_main->output);
    f_voice->modulator_outputs[1] = &(f_voice->adsr_filter->output);
    f_voice->modulator_outputs[2] = &(f_voice->ramp_env->output);
    f_voice->modulator_outputs[3] = &(f_voice->lfo_amount_output);
    f_voice->modulator_outputs[4] = &(f_voice->keyboard_track);
    f_voice->modulator_outputs[5] = &(f_voice->velocity_track);
    
    f_voice->noise_func_ptr = f_run_noise_off;
    
    return f_voice;
}


void v_wayv_poly_note_off(t_wayv_poly_voice * a_voice, int a_fast);

void v_wayv_poly_note_off(t_wayv_poly_voice * a_voice, int a_fast)
{
    if(a_fast)
    {
        v_adsr_set_fast_release(a_voice->adsr_main);
    }
    else
    {
        v_adsr_release(a_voice->adsr_main);
    }
    
    v_adsr_release(a_voice->adsr_amp1);
    v_adsr_release(a_voice->adsr_amp2);
    v_adsr_release(a_voice->adsr_amp3);
}

t_wayv_mono_modules * v_wayv_mono_init(float);


/*Initialize any modules that will be run monophonically*/
t_wayv_mono_modules * v_wayv_mono_init(float a_sr)
{
    t_wayv_mono_modules * a_mono = (t_wayv_mono_modules*)malloc(sizeof(t_wayv_mono_modules));    
    a_mono->pitchbend_smoother = g_sml_get_smoother_linear(a_sr, 1.0f, -1.0f, 0.2f);
    a_mono->amp_ptr = g_amp_get();
    return a_mono;
}


#ifdef	__cplusplus
}
#endif

#endif	/* LIBMODSYNTH_H */

