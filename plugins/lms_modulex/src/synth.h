/* 
 * File:   synth.h
 * Author: Jeff Hubbard
 *
 * Created on February 26, 2012, 8:48 PM
 */

#ifndef SYNTH_H
#define	SYNTH_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "../../include/pydaw2/pydaw_plugin.h"
#include "libmodsynth.h"
#include "../../libmodsynth/lib/cc_map.h"
   
#define MODULEX_SLOW_INDEX_ITERATIONS 50
    
#define MODULEX_INPUT0  0
#define MODULEX_INPUT1  1    
#define MODULEX_OUTPUT0  2
#define MODULEX_OUTPUT1  3

/*LMS_FIRST_CONTROL_PORT is the first port used for controls such as knobs.  All control ports must be numbered continuously,
 as they are iterated through*/
#define MODULEX_FIRST_CONTROL_PORT 4
#define MODULEX_FX0_KNOB0  4
#define MODULEX_FX0_KNOB1  5
#define MODULEX_FX0_KNOB2  6
#define MODULEX_FX0_COMBOBOX 7
#define MODULEX_FX1_KNOB0  8
#define MODULEX_FX1_KNOB1  9
#define MODULEX_FX1_KNOB2  10
#define MODULEX_FX1_COMBOBOX 11
#define MODULEX_FX2_KNOB0  12
#define MODULEX_FX2_KNOB1  13
#define MODULEX_FX2_KNOB2  14
#define MODULEX_FX2_COMBOBOX 15
#define MODULEX_FX3_KNOB0  16
#define MODULEX_FX3_KNOB1  17
#define MODULEX_FX3_KNOB2  18
#define MODULEX_FX3_COMBOBOX 19
#define MODULEX_FX4_KNOB0  20
#define MODULEX_FX4_KNOB1  21
#define MODULEX_FX4_KNOB2  22
#define MODULEX_FX4_COMBOBOX 23
#define MODULEX_FX5_KNOB0  24
#define MODULEX_FX5_KNOB1  25
#define MODULEX_FX5_KNOB2  26
#define MODULEX_FX5_COMBOBOX 27
#define MODULEX_FX6_KNOB0  28
#define MODULEX_FX6_KNOB1  29
#define MODULEX_FX6_KNOB2  30
#define MODULEX_FX6_COMBOBOX 31
#define MODULEX_FX7_KNOB0  32
#define MODULEX_FX7_KNOB1  33
#define MODULEX_FX7_KNOB2  34
#define MODULEX_FX7_COMBOBOX 35
#define MODULEX_DELAY_TIME  36
#define MODULEX_FEEDBACK  37
#define MODULEX_DRY  38
#define MODULEX_WET  39
#define MODULEX_DUCK  40
#define MODULEX_CUTOFF  41
#define MODULEX_STEREO 42
    
#define MODULEX_VOL_SLIDER 43
    
#define MODULEX_REVERB_TIME 44
#define MODULEX_REVERB_WET 45
#define MODULEX_REVERB_COLOR 46
    
#define MODULEX_COMPRESSOR_THRESH 47
#define MODULEX_COMPRESSOR_ATTACK 48
#define MODULEX_COMPRESSOR_RATIO 49
#define MODULEX_COMPRESSOR_RELEASE 50
    
/*This is the last control port*/
#define MODULEX_LAST_CONTROL_PORT 50
#define MODULEX_COUNT 51 /* must be 1 + highest value above CHANGE THIS IF YOU ADD OR TAKE AWAY ANYTHING*/
  
/*GUI Step 13:  Add a variable for each control in the LMS type*/
typedef struct {
    LADSPA_Data *input0;
    LADSPA_Data *input1;
    LADSPA_Data *output0;
    LADSPA_Data *output1;
    
    LADSPA_Data *fx_knob0[8];
    LADSPA_Data *fx_knob1[8];    
    LADSPA_Data *fx_knob2[8];
    LADSPA_Data *fx_combobox[8];
    
    LADSPA_Data *delay_time;
    LADSPA_Data *feedback;
    LADSPA_Data *dry;
    LADSPA_Data *wet;
    LADSPA_Data *duck;
    LADSPA_Data *cutoff;
    LADSPA_Data *stereo;
    
    LADSPA_Data *vol_slider;
    
    LADSPA_Data *reverb_time;
    LADSPA_Data *reverb_wet;
    LADSPA_Data *reverb_color;
    
    LADSPA_Data *compressor_thresh;
    LADSPA_Data *compressor_attack;
    LADSPA_Data *compressor_ratio;
    LADSPA_Data * compressor_release;
        
    float fs;    
    t_modulex_mono_modules * mono_modules;
    
    t_ccm_midi_cc_map * midi_cc_map;
    
    int i_mono_out;
    int i_buffer_clear;    
    
    int i_slow_index;
    int is_on;
    
} t_modulex;

#ifdef	__cplusplus
}
#endif

#endif	/* SYNTH_H */

