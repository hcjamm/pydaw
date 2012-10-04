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
   
#define LMS_INPUT_COUNT 16
    
#define LMS_INPUT_MIN 0
#define LMS_INPUT_MAX  (LMS_INPUT_MIN + LMS_INPUT_COUNT)
    
#define LMS_OUTPUT0  16
#define LMS_OUTPUT1  17
/*GUI Step 11:  Add ports to the main synthesizer file that the GUI can talk to */
    
/*LMS_FIRST_CONTROL_PORT is the first port used for controls such as knobs.  All control ports must be numbered continuously,
 as they are iterated through*/
#define LMS_FIRST_CONTROL_PORT 18
/*This is the last control port*/
#define LMS_LAST_CONTROL_PORT 18
#define LMS_COUNT 19 /* must be 1 + highest value above CHANGE THIS IF YOU ADD OR TAKE AWAY ANYTHING*/
  
/*GUI Step 13:  Add a variable for each control in the LMS type*/
typedef struct {
    LADSPA_Data *input_arr[LMS_INPUT_COUNT];
    
    LADSPA_Data *output0;
    LADSPA_Data *output1;
    
    float fs;    
    t_mono_modules * mono_modules;
    
    int i_mono_out;
    int i_buffer_clear;    
    
} t_pydaw_engine;

#ifdef	__cplusplus
}
#endif

#endif	/* SYNTH_H */

