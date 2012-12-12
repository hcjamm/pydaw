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

#ifndef RAYV_LIBMODSYNTH_H
#define	RAYV_LIBMODSYNTH_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "../../libmodsynth/constants.h"
    
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
#include "../../libmodsynth/modules/oscillator/lfo_simple.h"
#include "../../libmodsynth/modules/oscillator/osc_wavetable.h"

static float va_rayv_sr_recip;
static float va_rayv_sample_rate;

void v_rayv_init_lms(float f_sr);

void v_rayv_init_lms(float f_sr)
{
    va_rayv_sample_rate = f_sr;
    va_rayv_sr_recip = 1.0f/f_sr;    
}

/*Define any modules here that will be used monophonically, ie:  NOT per voice here.  If you are making an effect plugin instead
 of an instrument, you will most likely want to define all of your modules here*/

typedef struct st_rayv_mono_modules
{    
    t_smoother_iir * pitchbend_smoother;
    t_amp * amp_ptr;
}t_rayv_mono_modules;
    
/*define static variables for libmodsynth modules.  Once instance of this type will be created for each polyphonic voice.*/
typedef struct st_rayv_poly_voice
{
    t_osc_wav_unison * osc_wavtable1;
    t_osc_wav_unison * osc_wavtable2;
    t_wt_wavetables * wavetables;
    
    t_white_noise * white_noise1;
    t_adsr * adsr_amp;       
    float noise_amp;
    
    t_smoother_linear * glide_smoother;
    t_ramp_env * glide_env;
        
    float last_pitch;  //For simplicity, this is used whether glide is turned on or not    
    float base_pitch;  //base pitch for all oscillators, to avoid redundant calculations    
    float target_pitch;
    float current_sample; //This corresponds to the current sample being processed on this voice.  += this to the output buffer when finished.    
    
    t_amp * amp_ptr;

    /*Migrated from the now deprecate voice_data struct*/
    float   amp;
    float note_f;
    int note;
    float osc1_linamp;
    float osc2_linamp;
    float noise_linamp;           
    int i_voice;  //for the runVoice function to iterate the current block
}t_rayv_poly_voice;

t_rayv_poly_voice * g_rayv_poly_init();

/*initialize all of the modules in an instance of poly_voice*/

t_rayv_poly_voice * g_rayv_poly_init()
{
    t_rayv_poly_voice * f_voice = (t_rayv_poly_voice*)malloc(sizeof(t_rayv_poly_voice));
    
    f_voice->osc_wavtable1 = g_osc_get_osc_wav_unison(va_rayv_sample_rate);
    f_voice->osc_wavtable2 = g_osc_get_osc_wav_unison(va_rayv_sample_rate);
    f_voice->wavetables = g_wt_wavetables_get();    
    
    f_voice->adsr_amp = g_adsr_get_adsr(va_rayv_sr_recip);        
            
    f_voice->white_noise1 = g_get_white_noise(va_rayv_sample_rate);    
    f_voice->noise_amp = 0;
        
    f_voice->glide_env = g_rmp_get_ramp_env(va_rayv_sample_rate);    
        
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
    f_voice->noise_linamp = 1.0f;
    f_voice->i_voice = 0;
    
    return f_voice;
}


void v_rayv_poly_note_off(t_rayv_poly_voice * a_voice, int a_fast);

void v_rayv_poly_note_off(t_rayv_poly_voice * a_voice, int a_fast)
{
    if(a_fast)
    {
        v_adsr_set_fast_release(a_voice->adsr_amp);
    }
    else
    {
        v_adsr_release(a_voice->adsr_amp);
    }
}

t_rayv_mono_modules * v_rayv_mono_init();


/*Initialize any modules that will be run monophonically*/
t_rayv_mono_modules * v_rayv_mono_init()
{
    t_rayv_mono_modules * a_mono = (t_rayv_mono_modules*)malloc(sizeof(t_rayv_mono_modules));    
    a_mono->pitchbend_smoother = g_smr_iir_get_smoother();
    a_mono->amp_ptr = g_amp_get();
    return a_mono;
}


#ifdef	__cplusplus
}
#endif

#endif	/* LIBMODSYNTH_H */

