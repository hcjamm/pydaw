/* 
 * File:   libmodsynth.h
 * Author: vm-user
 *
 * Created on January 7, 2012, 7:26 PM
 */

#ifndef LIBMODSYNTH_H
#define	LIBMODSYNTH_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "constants.h"
    
/*includes for any libmodsynth components*/
#include "libmodsynth/lib/osc_core.h"
#include "libmodsynth/lib/pitch_core.h"
#include "libmodsynth/modules/oscillator/osc_simple.h"
#include "libmodsynth/modules/oscillator/noise.h"
#include "libmodsynth/modules/filter/svf.h"
#include "libmodsynth/modules/distortion/clipper.h"
#include "libmodsynth/modules/modulation/adsr.h"
   

    
/*Declare any static variables that should be used globally in LibModSynth
 Note that any constants not requiring dynamically generated data should 
 */
static float _sr_recip;
static float _sample_rate;

void _init_lms(float _sr);

void _init_lms(float _sr)
{
    _sample_rate = _sr;
    _sr_recip = 1/_sr;
    
}
    
/*define static variables for libmodsynth modules.  Once instance of this type will be created for each polyphonic voice.*/
typedef struct _poly_voice
{    
    osc_core * _osc_core_test;    
    state_variable_filter * _svf_filter;
    clipper * _clipper1;
    adsr * _adsr_filter;
    white_noise * _w_noise;
    adsr * _adsr_amp;
}poly_voice;

poly_voice * _poly_init();

/*initialize all of the modules in an instance of poly_voice*/

poly_voice * _poly_init()
{
    poly_voice * _voice = (poly_voice*)malloc(sizeof(poly_voice));
    
    _voice->_osc_core_test = _get_osc_core(_sample_rate);    
    _voice->_svf_filter = _svf_get(_sample_rate,4);
    _svf_set_res(_voice->_svf_filter, -3);
    _svf_set_cutoff(_voice->_svf_filter, 72);
    _voice->_clipper1 = _clp_get_clipper();
    _clp_set_clip_sym(_voice->_clipper1, -6);
    _clp_set_in_gain(_voice->_clipper1, 12);
    
    _voice->_adsr_amp = _adsr_get_adsr(_sr_recip);
    
    
    _voice->_adsr_filter = _adsr_get_adsr(_sr_recip);
    
    _adsr_set_a_time(_voice->_adsr_filter, 0.01);
    _adsr_set_d_time(_voice->_adsr_filter, .5);
    printf("\nD %f\n\n", (_voice->_adsr_filter->d_inc));
    _adsr_set_s_value(_voice->_adsr_filter, .2);
    _adsr_set_r_time(_voice->_adsr_filter, 1);
    
    _voice->_w_noise = _get_white_noise(_sample_rate);
    
    /*
    printf("A %f\n", (_voice->_adsr_filter->a_inc));
    printf("D %f\n", (_voice->_adsr_filter->d_inc));
    printf("S %f\n", (_voice->_adsr_filter->s_value));
    printf("R %f\n", (_voice->_adsr_filter->r_inc));
    
    printf("\n_poly_init() has run\n");
    */
    return _voice;
}

void _poly_note_on(poly_voice * _voice);

//Define anything that should happen when the user presses a note here
void _poly_note_on(poly_voice * _voice)
{
    /*Retrigger ADSR envelopes*/
    _adsr_retrigger(_voice->_adsr_amp);
    _adsr_retrigger(_voice->_adsr_filter);
    
    /*Set ADSR envelope(doing it here prevents the user for modulating it for notes that are already playing)*/
    //_adsr_set_adsr(_voice->_adsr_amp, (_instance->attack), (_instance->decay), (_instance->sustain), (_instance->release));
}

void _poly_note_off(poly_voice * _voice); //, LTS * _instance);

//Define anything that should happen when the user releases a note here
void _poly_note_off(poly_voice * _voice) //, LTS * _instance)
{
    _adsr_release(_voice->_adsr_amp);
    _adsr_release(_voice->_adsr_filter);    
}

void _mono_init();


/*Initialize any modules that will be run monophonically*/
void _mono_init()
{
    
}


#ifdef	__cplusplus
}
#endif

#endif	/* LIBMODSYNTH_H */

