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

#ifndef PYDAW_SYNTH_H
#define	PYDAW_SYNTH_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "../../include/pydaw3/pydaw_plugin.h"
  
#define PYDAW_INPUT_COUNT 0 //10
    
#define PYDAW_INPUT_MIN 0
#define PYDAW_INPUT_MAX  (PYDAW_INPUT_MIN + PYDAW_INPUT_COUNT)
    
#define PYDAW_OUTPUT0  (PYDAW_INPUT_COUNT)
#define PYDAW_OUTPUT1  (PYDAW_OUTPUT0 + 1)
/*GUI Step 11:  Add ports to the main synthesizer file that the GUI can talk to */
    
/*LMS_FIRST_CONTROL_PORT is the first port used for controls such as knobs.  All control ports must be numbered continuously,
 as they are iterated through*/
#define PYDAW_FIRST_CONTROL_PORT (PYDAW_OUTPUT1 + 1)
/*This is the last control port*/
#define PYDAW_LAST_CONTROL_PORT (PYDAW_FIRST_CONTROL_PORT)
#define PYDAW_COUNT  (PYDAW_LAST_CONTROL_PORT) /* must be 1 + highest value above CHANGE THIS IF YOU ADD OR TAKE AWAY ANYTHING*/
  
/*GUI Step 13:  Add a variable for each control in the LMS type*/
typedef struct {
    PYFX_Data **input_arr;
    
    PYFX_Data *output0;
    PYFX_Data *output1;
    
    float fs;
    
    int i_mono_out;
    int i_buffer_clear;    
    
} t_pydaw_engine;

#ifdef	__cplusplus
}
#endif

#endif	/* SYNTH_H */

