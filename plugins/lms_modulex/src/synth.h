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
/*GUI Step 11:  Add ports to the main synthesizer file that the GUI can talk to */
    
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
/*This is the last control port*/
#define MODULEX_LAST_CONTROL_PORT 19
#define MODULEX_COUNT 20 /* must be 1 + highest value above CHANGE THIS IF YOU ADD OR TAKE AWAY ANYTHING*/
  
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

