/* 
 * File:   input_gen.h
 * Author: Jeff Hubbard
 * 
 * Generate synthetic input for debugger projects
 *
 * Created on March 13, 2012, 6:15 PM
 */

#ifndef INPUT_GEN_H
#define	INPUT_GEN_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "../../src/libmodsynth/modules/oscillator/osc_simple.h"
#include "../../src/libmodsynth/lib/amp.h"
#include "../../src/libmodsynth/lib/pitch_core.h"
#include <ladspa.h>
    
PYFX_Data * g_get_input(int,float);

/*PYFX_Data * g_get_input(
 * int a_buffer_size,  //The size of the PYFX_Data*
 * float a_note_pitch,  //The pitch of the oscillator
 * int osc_type) //oscillator type.  See below
 * 
 * Create an input buffer filled with a saw-tooth oscillator output.  This allows you
 * to run the debugger project with simulated sound coming into the plugin.
 * 
 * Valid osc_types are:
 * -1.   Empty (all zeroes)
 *  0.   Saw
 *  1.   Square
 *  2.   Triangle
 *  3.   Sine
 *  4.   Off
 */
PYFX_Data * g_dbg_get_input_buffer(int a_buffer_size,float a_note_pitch, int a_osc_type)
{
    PYFX_Data * f_result = (PYFX_Data*)malloc(sizeof(PYFX_Data) * (a_buffer_size));
    
    int f_i = 0;
    
    t_osc_simple_unison * f_osc = g_osc_get_osc_simple_unison(44100);
    v_osc_set_simple_osc_unison_type(f_osc, 0);
    v_osc_set_uni_voice_count(f_osc, 1);
    v_osc_set_unison_pitch(f_osc, .1, a_note_pitch);
    
    while(f_i < a_buffer_size)
    {
        float f_sample = 0.0f;
        
        if(a_osc_type != -1)
        {
            f_sample = f_osc_run_unison_osc(f_osc);
        }        
        
        f_result[f_i] = f_sample;
        f_i++;
    }
    
    free(f_osc);
    
    return f_result;
}

#ifdef	__cplusplus
}
#endif

#endif	/* INPUT_GEN_H */

