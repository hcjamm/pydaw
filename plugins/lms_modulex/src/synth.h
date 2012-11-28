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

#include "ladspa.h"
#include "libmodsynth.h"
#include "../../libmodsynth/lib/cc_map.h"
   
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
/*This is the last control port*/
#define MODULEX_LAST_CONTROL_PORT 42
#define MODULEX_COUNT 43 /* must be 1 + highest value above CHANGE THIS IF YOU ADD OR TAKE AWAY ANYTHING*/
  
/*GUI Step 13:  Add a variable for each control in the LMS type*/
typedef struct {
    LADSPA_Data *input0;
    LADSPA_Data *input1;
    LADSPA_Data *output0;
    LADSPA_Data *output1;
    
    LADSPA_Data *fx0_knob0;
    LADSPA_Data *fx0_knob1;    
    LADSPA_Data *fx0_knob2;
    LADSPA_Data *fx0_combobox;
        
    LADSPA_Data *fx1_knob0;
    LADSPA_Data *fx1_knob1;    
    LADSPA_Data *fx1_knob2;
    LADSPA_Data *fx1_combobox;
        
    LADSPA_Data *fx2_knob0;
    LADSPA_Data *fx2_knob1;    
    LADSPA_Data *fx2_knob2;
    LADSPA_Data *fx2_combobox;
        
    LADSPA_Data *fx3_knob0;
    LADSPA_Data *fx3_knob1;    
    LADSPA_Data *fx3_knob2;
    LADSPA_Data *fx3_combobox;
        
    LADSPA_Data *fx4_knob0;
    LADSPA_Data *fx4_knob1;    
    LADSPA_Data *fx4_knob2;
    LADSPA_Data *fx4_combobox;
        
    LADSPA_Data *fx5_knob0;
    LADSPA_Data *fx5_knob1;    
    LADSPA_Data *fx5_knob2;
    LADSPA_Data *fx5_combobox;
        
    LADSPA_Data *fx6_knob0;
    LADSPA_Data *fx6_knob1;    
    LADSPA_Data *fx6_knob2;
    LADSPA_Data *fx6_combobox;
        
    LADSPA_Data *fx7_knob0;
    LADSPA_Data *fx7_knob1;    
    LADSPA_Data *fx7_knob2;
    LADSPA_Data *fx7_combobox;
    
    LADSPA_Data *delay_time;
    LADSPA_Data *feedback;
    LADSPA_Data *dry;
    LADSPA_Data *wet;
    LADSPA_Data *duck;
    LADSPA_Data *cutoff;
    LADSPA_Data *stereo;
        
    float fs;    
    t_modulex_mono_modules * mono_modules;
    
    t_ccm_midi_cc_map * midi_cc_map;
    
    int i_mono_out;
    int i_buffer_clear;    
    
} t_modulex;

#ifdef	__cplusplus
}
#endif

#endif	/* SYNTH_H */

