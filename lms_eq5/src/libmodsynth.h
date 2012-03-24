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
#include "../../libmodsynth/constants.h"
    
/*includes for any libmodsynth modules you'll be using*/
#include "../../libmodsynth/lib/pitch_core.h"
#include "../../libmodsynth/lib/amp.h"
    #include "../../libmodsynth/lib/smoother-linear.h"
#include "../../libmodsynth/modules/filter/svf.h"
   
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
    /*EQ1*/
    t_state_variable_filter * eq1_0;
    t_state_variable_filter * eq1_1;
    t_smoother_linear * smoother1;            
    /*EQ2*/
    t_state_variable_filter * eq2_0;
    t_state_variable_filter * eq2_1;
    t_smoother_linear * smoother2;
    /*EQ3*/
    t_state_variable_filter * eq3_0;
    t_state_variable_filter * eq3_1;
    t_smoother_linear * smoother3;
    /*EQ4*/
    t_state_variable_filter * eq4_0;
    t_state_variable_filter * eq4_1;
    t_smoother_linear * smoother4;
    /*EQ5*/
    t_state_variable_filter * eq5_0;
    t_state_variable_filter * eq5_1;
    t_smoother_linear * smoother5;
    
    float out_sample0, out_sample1;
}t_mono_modules;
    

t_mono_modules * v_mono_init(float);


/*Initialize any modules that will be run monophonically*/
t_mono_modules * v_mono_init(float a_sr)
{
    t_mono_modules * a_mono = (t_mono_modules*)malloc(sizeof(t_mono_modules));
    /*EQ1*/
    a_mono->eq1_0 = g_svf_get(a_sr);
    a_mono->eq1_1 = g_svf_get(a_sr);
    a_mono->smoother1 = g_sml_get_smoother_linear(a_sr, 124.0f, 20.0f, 1.0f);
    /*EQ2*/
    a_mono->eq2_0 = g_svf_get(a_sr);
    a_mono->eq2_1 = g_svf_get(a_sr);
    a_mono->smoother2 = g_sml_get_smoother_linear(a_sr, 124.0f, 20.0f, 1.0f);
    /*EQ3*/
    a_mono->eq3_0 = g_svf_get(a_sr);
    a_mono->eq3_1 = g_svf_get(a_sr);
    a_mono->smoother3 = g_sml_get_smoother_linear(a_sr, 124.0f, 20.0f, 1.0f);
    /*EQ4*/
    a_mono->eq4_0 = g_svf_get(a_sr);
    a_mono->eq4_1 = g_svf_get(a_sr);
    a_mono->smoother4 = g_sml_get_smoother_linear(a_sr, 124.0f, 20.0f, 1.0f);
    /*EQ5*/
    a_mono->eq5_0 = g_svf_get(a_sr);
    a_mono->eq5_1 = g_svf_get(a_sr);
    a_mono->smoother5 = g_sml_get_smoother_linear(a_sr, 124.0f, 20.0f, 1.0f);
    
    a_mono->out_sample0 = 0.0f;
    a_mono->out_sample1 = 0.0f;
    
    return a_mono;
    
}

/*Define any custom functions here*/



#ifdef	__cplusplus
}
#endif

#endif	/* LIBMODSYNTH_H */

