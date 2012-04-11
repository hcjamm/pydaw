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
    
/*Comment these out when compiling a stable, production-ready plugin.  
 The debugging code wastes a lot of CPU, and end users don't really need to see it*/
//#define LMS_DEBUG_NOTE
//#define LMS_DEBUG_MAIN_LOOP
//#define LMS_DEBUG_MODE_QT
//#define LMS_DEBUGGER_PROJECT

/*Then you can print debug information like this:
#ifdef LMS_DEBUG_NOTE
printf("debug information");
#endif
*/
   
#define POLYPHONY   8  //maximum voices played at one time
#define MIDI_NOTES  128  //Maximum MIDI note.  You probably don't want to change this
#define STEP_SIZE   16
    

/*GUI Step 12:  Add a variable for each control in the synth_vals type*/
typedef struct {    
    /*The variables below this line correspond to GUI controls*/
    
    LADSPA_Data gain;
    LADSPA_Data wet;    
    LADSPA_Data out_gain;
    /*The variables below this line do NOT correspond to GUI controls*/
#ifdef LMS_DEBUG_MAIN_LOOP
    int debug_counter;
#endif
} synth_vals;

#ifdef LMS_DEBUG_MAIN_LOOP

void dump_debug_synth_vals(synth_vals*);

/*Any changes to voice_data require this to be changed*/
void dump_debug_synth_vals(synth_vals * a_data)
{
    a_data->debug_counter = (a_data->debug_counter) + 1;
    
    if((a_data->debug_counter) >= 100000)
    {
        a_data->debug_counter = 0;
        printf("\n\nRunning dump_debug_synth_vals\n");
        printf("gain == %f\n", a_data->gain);
        printf("wet == %f\n", a_data->wet);
    }
}

#endif

/*GUI Step 13:  Add a variable for each control in the LMS type*/
typedef struct {
    LADSPA_Data *input0;
    LADSPA_Data *input1;
    LADSPA_Data *output0;
    LADSPA_Data *output1;
    LADSPA_Data *gain;
    LADSPA_Data *wet;
    LADSPA_Data *out_gain;
    
    float fs;    
    t_mono_modules * mono_modules;
    synth_vals vals;
    
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

