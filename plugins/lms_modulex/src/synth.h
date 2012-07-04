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
    
/*Comment these out when compiling a stable, production-ready plugin.  
 The debugging code wastes a lot of CPU, and end users don't really need to see it*/
//#define LMS_DEBUG_NOTE
//#define LMS_DEBUG_MAIN_LOOP
//#define LMS_DEBUG_MODE_QT


/*Then you can print debug information like this:
#ifdef LMS_DEBUG_NOTE
printf("debug information");
#endif
*/
   
#define LMS_INPUT0  0
#define LMS_INPUT1  1    
#define LMS_OUTPUT0  2
#define LMS_OUTPUT1  3
/*GUI Step 11:  Add ports to the main synthesizer file that the GUI can talk to */
    
/*LMS_FIRST_CONTROL_PORT is the first port used for controls such as knobs.  All control ports must be numbered continuously,
 as they are iterated through*/
#define LMS_FIRST_CONTROL_PORT 4
#define LMS_FX1_KNOB1  4
#define LMS_FX1_KNOB2  5
#define LMS_FX1_KNOB3  6
#define LMS_FX1_COMBOBOX 7
/*This is the last control port*/
#define LMS_LAST_CONTROL_PORT 7
#define LMS_COUNT 8 /* must be 1 + highest value above CHANGE THIS IF YOU ADD OR TAKE AWAY ANYTHING*/

#define STEP_SIZE   16
    

/*GUI Step 12:  Add a variable for each control in the synth_vals type*/
typedef struct {    
    /*The variables below this line correspond to GUI controls*/
    
    LADSPA_Data fx1_knob1;
    LADSPA_Data fx1_knob2;    
    LADSPA_Data fx1_knob3;
    LADSPA_Data fx1_combobox;
} synth_vals;


/*GUI Step 13:  Add a variable for each control in the LMS type*/
typedef struct {
    LADSPA_Data *input0;
    LADSPA_Data *input1;
    LADSPA_Data *output0;
    LADSPA_Data *output1;
    LADSPA_Data *fx1_knob1;
    LADSPA_Data *fx1_knob2;    
    LADSPA_Data *fx1_knob3;
    LADSPA_Data *fx1_combobox;
    
    float fs;    
    t_mono_modules * mono_modules;
    synth_vals vals;
    t_ccm_midi_cc_map * midi_cc_map;
    int pos;
    int count;
    int buffer_pos;
    int i_mono_out;
    int i_buffer_clear;    
    
} LMS;

#ifdef	__cplusplus
}
#endif

#endif	/* SYNTH_H */

