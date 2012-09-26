/* 
 * File:   main.c
 * Author: Jeff Hubbard
 * 
 * 
 *
 * Created on March 12, 2012, 7:46 PM
 */

#include <stdio.h>
#include <stdlib.h>

/*
#include <alsa/asoundlib.h>
#include <alsa/seq.h>
#include <alsa/seq_event.h>
#include <alsa/seq_midi_event.h>
#include <alsa/seqmid.h>
*/
/*Change these to the project you would like to debug*/
//#include "../../plugins/libmodsynth/lib/lms_sequencer.h"
#include "../../plugins/pydaw/src/pydaw.h"

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
    
    
    /*t_osc_simple_unison * f_osc = g_osc_get_osc_simple_unison(44100.0f);
    v_osc_set_simple_osc_unison_type(f_osc, 0);
    
    v_osc_set_unison_pitch(f_osc, 0.1f, 66.0f);
    
    v_lim_set(f_limiter, -6.0f, -9.0f, 200.0f);
    
    float * test_square; // = (float*)malloc(sizeof(float) * 50);
    posix_memalign((void**)&test_square, 16, (sizeof(float) * 50));
        
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
    
    int f_i;
    
    for(f_i = 0; f_i < 500000; f_i++)
    {
        float f_sample = f_osc_run_unison_osc(f_osc);
        
        v_lim_run(f_limiter, f_sample, f_sample);
    }
    
    
    for(f_i = 0; f_i < 500000; f_i++)
    {
        float f_sample = f_osc_run_unison_osc(f_osc);
        
        v_lim_run(f_limiter, f_sample, f_sample);
    }
    
    
    for(f_i = 0; f_i < 500000; f_i++)
    {
        float f_sample = f_osc_run_unison_osc(f_osc);
        
        v_lim_run(f_limiter, f_sample, f_sample);
    }
    */

    //midi_open();
    //snd_seq_close(seq);
    
    t_pydaw_data * f_data = g_pydaw_data_get();
    f_data->region_folder = "/home/bob/dssi/pydaw/default-project/regions/";
    v_pydaw_parse_configure_message(f_data, "sr", "region-1");
    v_pydaw_parse_configure_message(f_data, "sr", "region-1");
    
    return 0; //(EXIT_SUCCESS);
}

