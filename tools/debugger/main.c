/* 
 * File:   main.c
 * Author: Jeff Hubbard
 * 
 * A debugger project for running plugins in a synthetic manner without actually sending
 * any audio to the soundcard.  This is primarily intended for use in IDEs that have
 * GDB integration, such as Netbeans, Eclipse or Anjuta.
 * 
 * Usage:  Set breakpoints anywhere you'd like to see them, and debug the debugger project.  Be sure to edit these
 * lines:
 * 
 * #include "../lms_comb/src/synth.h"
 * #include "../lms_comb/src/synth.c" 
 * 
 * below includes to the project you want to debug, replacing 'lms_comb' with the name of the project folder you wish
 * to debug.  You can also set breakpoints in any file within "../libmodsynth/lib or ../libmodsynth/modules"
 *
 * Created on March 12, 2012, 7:46 PM
 */

#include <stdio.h>
#include <stdlib.h>

/*Change these to the project you would like to debug*/
#include "../../plugins/libmodsynth/lib/interpolate-sinc.h"

/*This must be defined in synth.h for the project to be debugged, otherwise you'll get a segfault.
#define LMS_DEBUGGER_PROJECT

Also:
 * 
 * The following apparatus must be added to the runLMS function in synth.c for the project you'll be debugging.
 * Not all of the LMS plugins have been retrofitted with this yet:
 * 
#include "synth.h"
#include "meta.h"

#ifdef LMS_DEBUGGER_PROJECT
#include "../../debugger/input_gen.h"
#endif

 ...
  
 static void runLMS(LADSPA_Handle instance, unsigned long sample_count,
		  snd_seq_event_t *events, unsigned long event_count)
{
    LMS *plugin_data = (LMS *) instance;
//Add this define, and create dummy inputs when debugging
#ifdef LMS_DEBUGGER_PROJECT
    LADSPA_Data *const input0 = (LADSPA_Data*)malloc(sizeof(LADSPA_Data) * (plugin_data->count));    
    LADSPA_Data *const input1 = (LADSPA_Data*)malloc(sizeof(LADSPA_Data) * (plugin_data->count));

    LADSPA_Data *const output0 = (LADSPA_Data*)malloc(sizeof(LADSPA_Data) * (plugin_data->count));
    LADSPA_Data *const output1 = (LADSPA_Data*)malloc(sizeof(LADSPA_Data) * (plugin_data->count));
#else
    LADSPA_Data *const input0 = plugin_data->input0;    
    LADSPA_Data *const input1 = plugin_data->input1;

    LADSPA_Data *const output0 = plugin_data->output0;    
    LADSPA_Data *const output1 = plugin_data->output1;    
#endif    

    plugin_data->pos = 0;
    plugin_data->count= 0;    
    plugin_data->i_mono_out = 0;
    plugin_data->event_pos = 0;

 //add this define, and set your control values manually
#ifdef LMS_DEBUGGER_PROJECT
    plugin_data->vals.threshold = -24;    
    plugin_data->vals.ratio = 5;
    plugin_data->vals.attack = 0.1f;    
    plugin_data->vals.release = 0.2f;
    plugin_data->vals.gain = 0;
#else
    plugin_data->vals.threshold = *(plugin_data->threshold);    
    plugin_data->vals.ratio = *(plugin_data->ratio) * 0.1f;
    plugin_data->vals.attack = *(plugin_data->attack) * 0.01f;    
    plugin_data->vals.release = *(plugin_data->release) * 0.01f;
    plugin_data->vals.gain = *(plugin_data->gain);
#endif
 
 */

/* int main(
 * int argc, //ignored
 * char** argv) //ignored
 * 
 */
int main(int argc, char** argv) {
    
    //LADSPA_Descriptor * f_descriptor = *ladspa_descriptor(0);
    //LADSPA_Handle f_plugin = instantiateLMS(LMSLDescriptor, 44100);
    //activateLMS(f_plugin);
    
    //snd_seq_event_t events;    
    
    //runLMS(f_plugin, 1000, &events, 0);
    
    //free(f_plugin);
    
    //t_sinc_interpolator * test = g_sinc_get(15, 1000);
    
    t_sinc_interpolator * f_test = g_sinc_get(7, 100, 0.15);
    
    float * test_square = (float*)malloc(sizeof(float) * 50);
        
    int i;
    
    for(i = 0; i < 10; i++)
    {
        test_square[i] = 0.0f;
    }
    
    test_square[10] = 1.0f;
    test_square[11] = 1.0f;
    test_square[12] = 1.0f;
    test_square[13] = 1.0f;
    test_square[14] = 1.0f;
    test_square[15] = 1.0f;
    test_square[16] = 1.0f;
    test_square[17] = 1.0f;
    test_square[18] = 1.0f;
    test_square[19] = -1.0f;
    test_square[20] = -1.0f;
    test_square[21] = -1.0f;
    test_square[22] = -1.0f;
    test_square[23] = -1.0f;
    test_square[24] = -1.0f;
    test_square[25] = -1.0f;
    test_square[26] = -1.0f;
    test_square[27] = -1.0f;
    
    for(i = 28; i < 50; i++)
    {
        test_square[i] = 0.0f;
    }
    
    float f_iterate;
    
    for(f_iterate  = 10.0f; f_iterate < 40; f_iterate+=0.369f)
    {
        printf("%f\n", f_sinc_interpolate(f_test,test_square,f_iterate));
    }
    
    return 0; //(EXIT_SUCCESS);
}

