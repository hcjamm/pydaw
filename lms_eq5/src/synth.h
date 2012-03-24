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
   
#define LMS_INPUT0  0
#define LMS_INPUT1  1    
#define LMS_OUTPUT0  2
#define LMS_OUTPUT1  3
/*GUI Step 11:  Add ports to the main synthesizer file that the GUI can talk to */
    
/*LMS_FIRST_CONTROL_PORT is the first port used for controls such as knobs.  All control ports must be numbered continuously,
 as they are iterated through*/
#define LMS_FIRST_CONTROL_PORT 4
/*EQ1*/
#define LMS_PITCH1  4
#define LMS_GAIN1  5
#define LMS_RES1  6
/*EQ2*/
#define LMS_PITCH2  7
#define LMS_GAIN2  8
#define LMS_RES2  9
/*EQ1*/
#define LMS_PITCH3  10
#define LMS_GAIN3  11
#define LMS_RES3  12
/*EQ1*/
#define LMS_PITCH4  13
#define LMS_GAIN4  14
#define LMS_RES4  15
/*EQ1*/
#define LMS_PITCH5  16
#define LMS_GAIN5  17
#define LMS_RES5  18

/*This is the last control port*/
#define LMS_LAST_CONTROL_PORT 18
#define LMS_COUNT 19 /* must be 1 + highest value above CHANGE THIS IF YOU ADD OR TAKE AWAY ANYTHING*/


#define POLYPHONY   8  //maximum voices played at one time
#define MIDI_NOTES  128  //Maximum MIDI note.  You probably don't want to change this
#define STEP_SIZE   16
    

/*GUI Step 12:  Add a variable for each control in the synth_vals type*/
typedef struct {    
    /*The variables below this line correspond to GUI controls*/
    /*EQ1*/
    LADSPA_Data pitch1;
    LADSPA_Data gain1;
    LADSPA_Data res1;
    /*EQ2*/
    LADSPA_Data gain2;
    LADSPA_Data pitch2;
    LADSPA_Data res2;
    /*EQ3*/
    LADSPA_Data gain3;
    LADSPA_Data pitch3;
    LADSPA_Data res3;
    /*EQ4*/
    LADSPA_Data gain4;
    LADSPA_Data pitch4;
    LADSPA_Data res4;
    /*EQ5*/
    LADSPA_Data gain5;
    LADSPA_Data pitch5;
    LADSPA_Data res5;
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
    /*EQ1*/
    LADSPA_Data *gain1;
    LADSPA_Data *pitch1;
    LADSPA_Data *res1;
    /*EQ2*/
    LADSPA_Data *gain2;
    LADSPA_Data *pitch2;
    LADSPA_Data *res2;
    /*EQ3*/
    LADSPA_Data *gain3;
    LADSPA_Data *pitch3;
    LADSPA_Data *res3;
    /*EQ4*/
    LADSPA_Data *gain4;
    LADSPA_Data *pitch4;
    LADSPA_Data *res4;
    /*EQ5*/
    LADSPA_Data *gain5;
    LADSPA_Data *pitch5;
    LADSPA_Data *res5;
    
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

