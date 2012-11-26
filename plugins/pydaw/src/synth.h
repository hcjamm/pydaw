/* 
 * File:   synth.h
 * Author: Jeff Hubbard
 *
 * Created on February 26, 2012, 8:48 PM
 */

#ifndef PYDAW_SYNTH_H
#define	PYDAW_SYNTH_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "ladspa.h"
#include "libmodsynth.h"
#include "../../libmodsynth/lib/cc_map.h"
   
//#define PYDAW_INPUT_COUNT 16
    
//#define PYDAW_INPUT_MIN 0
//#define PYDAW_INPUT_MAX  (PYDAW_INPUT_MIN + PYDAW_INPUT_COUNT)
    
#define PYDAW_OUTPUT0  0 //16
#define PYDAW_OUTPUT1  1 //17
/*GUI Step 11:  Add ports to the main synthesizer file that the GUI can talk to */
    
/*LMS_FIRST_CONTROL_PORT is the first port used for controls such as knobs.  All control ports must be numbered continuously,
 as they are iterated through*/
#define PYDAW_FIRST_CONTROL_PORT 2
/*This is the last control port*/
#define PYDAW_LAST_CONTROL_PORT 2
#define PYDAW_COUNT 3 /* must be 1 + highest value above CHANGE THIS IF YOU ADD OR TAKE AWAY ANYTHING*/
  
/*GUI Step 13:  Add a variable for each control in the LMS type*/
typedef struct {
    //LADSPA_Data *input_arr[PYDAW_INPUT_COUNT];
    
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

