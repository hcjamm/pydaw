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
#include "../../libmodsynth/modules/oscillator/lfo_simple.h"

typedef struct
{
    t_smoother_linear * filter_smoother;
    t_smoother_linear * pitchbend_smoother;
    t_smoother_linear * lfo_smoother;
    t_amp * amp_ptr;
}t_rayv_mono_modules;


typedef struct
{
    t_osc_simple_unison * osc_unison1;
    t_osc_simple_unison * osc_unison2;

    float osc1_pitch_adjust, osc2_pitch_adjust;

    t_state_variable_filter * svf_filter;
    fp_svf_run_filter svf_function;

    t_clipper * clipper1;
    t_audio_xfade * dist_dry_wet;

    t_adsr * adsr_filter;
    t_white_noise * white_noise1;
    t_adsr * adsr_amp;
    float noise_amp;

    float filter_keytrk;

    t_smoother_linear * glide_smoother;
    t_ramp_env * glide_env;

    t_ramp_env * pitch_env;
    //For glide
    float last_pitch;
      //base pitch for all oscillators, to avoid redundant calculations
    float base_pitch;

    float target_pitch;

    float filter_output;  //For assigning the filter output to
    // This corresponds to the current sample being processed on this voice.
    // += this to the output buffer when finished.
    float current_sample;

    t_lfs_lfo * lfo1;

    float lfo_amp_output, lfo_pitch_output, lfo_filter_output;

    t_amp * amp_ptr;

    /*Migrated from the now deprecate voice_data struct*/
    float   amp;
    float note_f;
    int note;
    float osc1_linamp;
    float osc2_linamp;
    float noise_linamp;
    int i_voice;  //for the runVoice function to iterate the current block

    int hard_sync;
    int adsr_prefx;
    float unison_spread;
}t_rayv_poly_voice;

t_rayv_poly_voice * g_rayv_poly_init(float);



t_rayv_poly_voice * g_rayv_poly_init(float a_sr)
{
    t_rayv_poly_voice * f_voice =
            (t_rayv_poly_voice*)malloc(sizeof(t_rayv_poly_voice));

    f_voice->osc_unison1 = g_osc_get_osc_simple_unison(a_sr);
    f_voice->osc_unison2 = g_osc_get_osc_simple_unison(a_sr);

    f_voice->osc1_pitch_adjust = 0.0f;
    f_voice->osc2_pitch_adjust = 0.0f;

    f_voice->svf_filter = g_svf_get(a_sr);
    f_voice->svf_function = svf_get_run_filter_ptr(1, SVF_FILTER_TYPE_LP);

    f_voice->filter_keytrk = 0.0f;

    f_voice->clipper1 = g_clp_get_clipper();
    f_voice->dist_dry_wet = g_axf_get_audio_xfade(-3);

    f_voice->adsr_amp = g_adsr_get_adsr(a_sr);
    f_voice->adsr_filter = g_adsr_get_adsr(a_sr);

    f_voice->white_noise1 = g_get_white_noise(a_sr);
    f_voice->noise_amp = 0;

    f_voice->glide_env = g_rmp_get_ramp_env(a_sr);
    f_voice->pitch_env = g_rmp_get_ramp_env(a_sr);

    //f_voice->real_pitch = 60.0f;

    f_voice->target_pitch = 66.0f;
    f_voice->last_pitch = 66.0f;
    f_voice->base_pitch = 66.0f;

    f_voice->current_sample = 0.0f;

    f_voice->filter_output = 0.0f;

    f_voice->lfo1 = g_lfs_get(a_sr);

    f_voice->lfo_amp_output = 0.0f;
    f_voice->lfo_filter_output = 0.0f;
    f_voice->lfo_pitch_output = 0.0f;

    f_voice->amp_ptr = g_amp_get();

    f_voice->amp = 1.0f;
    f_voice->note_f = 1.0f;
    f_voice->osc1_linamp = 1.0f;
    f_voice->osc2_linamp = 1.0f;
    f_voice->noise_linamp = 1.0f;
    f_voice->i_voice = 0;

    f_voice->hard_sync = 0;
    f_voice->adsr_prefx = 0;
    f_voice->unison_spread = 0.5f;

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

    v_adsr_release(a_voice->adsr_filter);
}

t_rayv_mono_modules * v_rayv_mono_init(float);


/*Initialize any modules that will be run monophonically*/
t_rayv_mono_modules * v_rayv_mono_init(float a_sr)
{
    t_rayv_mono_modules * a_mono =
            (t_rayv_mono_modules*)malloc(sizeof(t_rayv_mono_modules));
    a_mono->filter_smoother =
            g_sml_get_smoother_linear(a_sr, 124.0f, 20.0f, 0.2f);
    a_mono->lfo_smoother =
            g_sml_get_smoother_linear(a_sr, 1600.0f, 10.0f, 0.2f);
    //To prevent low volume and brightness at the first note-on(s)
    a_mono->filter_smoother->last_value = 100.0f;
    a_mono->pitchbend_smoother =
            g_sml_get_smoother_linear(a_sr, 1.0f, -1.0f, 0.1f);
    a_mono->amp_ptr = g_amp_get();
    return a_mono;
}


#ifdef	__cplusplus
}
#endif

#endif	/* LIBMODSYNTH_H */

