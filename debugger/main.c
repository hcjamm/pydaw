/* 
 * File:   main.c
 * Author: Jeff Hubbard
 * 
 * An experimental debugger for running plugins in a synthetic manner without actually sending
 * any audio to the soundcard.  This is primarily intended for use in IDEs that have
 * GDB integration, such as Netbeans, Eclipse or Anjuta.
 * 
 * Usage:  Set breakpoints anywhere you'd like to see them, and debug the debugger project.  Be sure to edit the 
 * below includes to the project you want to debug.
 *
 * Created on March 12, 2012, 7:46 PM
 */

#include <stdio.h>
#include <stdlib.h>

/*Change these to the project you would like to debug*/
#include "../lms_dynamics/src/synth.h"
#include "../lms_dynamics/src/synth.c"

/*This defines certain behaviors within projects that are being debugged.  There's no need to comment this out, if you're
 not running the debugger project, then this won't be defined anyways...*/
#define LMS_DEBUGGER_PROJECT

/* int main(
 * int argc, //ignored
 * char** argv) //ignored
 * 
 */
int main(int argc, char** argv) {
    
    //LADSPA_Descriptor * f_descriptor = *ladspa_descriptor(0);
    LADSPA_Handle f_plugin = instantiateLMS(LMSLDescriptor, 44100);
    activateLMS(f_plugin);
    
    snd_seq_event_t events;    
    
    runLMS(f_plugin, 1000, &events, 0);
    
    return (EXIT_SUCCESS);
}

