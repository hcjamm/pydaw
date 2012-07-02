/* -*- c-basic-offset: 4 -*-  vi:set ts=8 sts=4 sw=4: */

/* synth.c

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <stdint.h>

#include <math.h>
#include <stdio.h>

#include "dssi.h"
#include "ladspa.h"

#include "libmodsynth.h"
#include "../../libmodsynth/lib/amp.h"

#include "synth.h"
#include "meta.h"

static LADSPA_Descriptor *LMSLDescriptor = NULL;
static DSSI_Descriptor *LMSDDescriptor = NULL;

static void runLMS(LADSPA_Handle instance, unsigned long sample_count,
		  snd_seq_event_t * events, unsigned long EventCount);


__attribute__ ((visibility("default")))
const LADSPA_Descriptor *ladspa_descriptor(unsigned long index)
{
    switch (index) {
    case 0:
	return LMSLDescriptor;
    default:
	return NULL;
    }
}

__attribute__ ((visibility("default")))
const DSSI_Descriptor *dssi_descriptor(unsigned long index)
{
    switch (index) {
    case 0:
	return LMSDDescriptor;
    default:
	return NULL;
    }
}

static void cleanupLMS(LADSPA_Handle instance)
{
    free(instance);
}

static void connectPortLMS(LADSPA_Handle instance, unsigned long port,
			  LADSPA_Data * data)
{
    LMS *plugin;

    plugin = (LMS *) instance;
    
    /*Add the ports from step 9 to the connectPortLMS event handler*/
    
    switch (port) {
        case LMS_INPUT0:
            plugin->input0 = data;
            break;
        case LMS_INPUT1:
            plugin->input1 = data;
            break;
        case LMS_OUTPUT0:
            plugin->output0 = data;
            break;
        case LMS_OUTPUT1:
            plugin->output1 = data;
            break;
        /*EQ1*/
        case LMS_PITCH1:
            plugin->pitch1 = data;              
            break;
        case LMS_GAIN1:
            plugin->gain1 = data;              
            break;
        case LMS_RES1:
            plugin->res1 = data;              
            break;
        /*EQ2*/
        case LMS_PITCH2:
            plugin->pitch2 = data;              
            break;
        case LMS_GAIN2:
            plugin->gain2 = data;              
            break;
        case LMS_RES2:
            plugin->res2 = data;              
            break;
        /*EQ3*/
        case LMS_PITCH3:
            plugin->pitch3 = data;              
            break;
        case LMS_GAIN3:
            plugin->gain3 = data;              
            break;
        case LMS_RES3:
            plugin->res3 = data;              
            break;
        /*EQ4*/
        case LMS_PITCH4:
            plugin->pitch4 = data;              
            break;
        case LMS_GAIN4:
            plugin->gain4 = data;              
            break;
        case LMS_RES4:
            plugin->res4 = data;              
            break;
        /*EQ5*/
        case LMS_PITCH5:
            plugin->pitch5 = data;              
            break;
        case LMS_GAIN5:
            plugin->gain5 = data;              
            break;
        case LMS_RES5:
            plugin->res5 = data;              
            break;
            
    }
}

static LADSPA_Handle instantiateLMS(const LADSPA_Descriptor * descriptor,
				   unsigned long s_rate)
{
    LMS *plugin_data = (LMS *) malloc(sizeof(LMS));
    
    plugin_data->fs = s_rate;
    
    plugin_data->midi_cc_map = g_ccm_get();
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_PITCH1, 20, "Cutoff1");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_GAIN1, 21, "Gain1");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_RES1, 22, "Res1");

    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_PITCH2, 23, "Cutoff2");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_GAIN2, 24, "Gain2");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_RES2, 25, "Res2");
    
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_PITCH3, 26, "Cutoff3");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_GAIN3, 27, "Gain3");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_RES3, 28, "Res3");
    
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_PITCH4, 29, "Cutoff4");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_GAIN4, 30, "Gain4");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_RES4, 31, "Res4");
    
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_PITCH2, 32, "Cutoff5");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_GAIN2, 33, "Gain5");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_RES2, 34, "Res5");
    
    v_ccm_read_file_to_array(plugin_data->midi_cc_map, "lms_eq5-cc_map.txt");
    
    /*LibModSynth additions*/
    v_init_lms(s_rate);  //initialize any static variables    
    /*End LibModSynth additions*/
    
    return (LADSPA_Handle) plugin_data;
}

static void activateLMS(LADSPA_Handle instance)
{
    LMS *plugin_data = (LMS *) instance;
        
    plugin_data->mono_modules = v_mono_init((plugin_data->fs));  //initialize all monophonic modules
}

static void runLMSWrapper(LADSPA_Handle instance,
			 unsigned long sample_count)
{
    runLMS(instance, sample_count, NULL, 0);
}

/*This is where parameters are update and the main loop is run.*/
static void runLMS(LADSPA_Handle instance, unsigned long sample_count,
		  snd_seq_event_t *events, unsigned long event_count)
{
    LMS *plugin_data = (LMS *) instance;
    
#ifdef LMS_DEBUGGER_PROJECT
    LADSPA_Data *const input0 = (LADSPA_Data*)malloc(sizeof(LADSPA_Data) * (plugin_data->count));    
    LADSPA_Data *const input1 = (LADSPA_Data*)malloc(sizeof(LADSPA_Data) * (plugin_data->count));

    LADSPA_Data *const output0 = (LADSPA_Data*)malloc(sizeof(LADSPA_Data) * (plugin_data->count));
    LADSPA_Data *const output1 = (LADSPA_Data*)malloc(sizeof(LADSPA_Data) * (plugin_data->count));
#else
    /*Define our inputs*/
    LADSPA_Data *const input0 = plugin_data->input0;    
    LADSPA_Data *const input1 = plugin_data->input1;
    /*define our outputs*/
    LADSPA_Data *const output0 = plugin_data->output0;    
    LADSPA_Data *const output1 = plugin_data->output1;    
#endif
    /*Reset our iterators to 0*/
    plugin_data->pos = 0;
    plugin_data->count= 0;    
    plugin_data->i_mono_out = 0;

#ifdef LMS_DEBUGGER_PROJECT
    plugin_data->vals.gain1 = 12;    
    plugin_data->vals.pitch1 = 100.0f;
    plugin_data->vals.res1 = -15;
#else
    /*Set the values from synth_vals in RunLMS*/
    /*EQ1*/
    plugin_data->vals.pitch1 = *(plugin_data->pitch1);
    plugin_data->vals.gain1 = *(plugin_data->gain1);    
    plugin_data->vals.res1 = *(plugin_data->res1);
    /*EQ2*/
    plugin_data->vals.pitch2 = *(plugin_data->pitch2);
    plugin_data->vals.gain2 = *(plugin_data->gain2);    
    plugin_data->vals.res2 = *(plugin_data->res2);
    /*EQ3*/
    plugin_data->vals.pitch3 = *(plugin_data->pitch3);
    plugin_data->vals.gain3 = *(plugin_data->gain3);    
    plugin_data->vals.res3 = *(plugin_data->res3);
    /*EQ4*/
    plugin_data->vals.pitch4 = *(plugin_data->pitch4);
    plugin_data->vals.gain4 = *(plugin_data->gain4);    
    plugin_data->vals.res4 = *(plugin_data->res4);
    /*EQ5*/
    plugin_data->vals.pitch5 = *(plugin_data->pitch5);
    plugin_data->vals.gain5 = *(plugin_data->gain5);    
    plugin_data->vals.res5 = *(plugin_data->res5);
        
#endif
    /*EQ1 Left Channel*/    
    v_svf_set_res(plugin_data->mono_modules->eq1_0, (plugin_data->vals.res1));    
    v_svf_set_eq(plugin_data->mono_modules->eq1_0, (plugin_data->vals.gain1));
    /*EQ1 Right Channel*/
    v_svf_set_res(plugin_data->mono_modules->eq1_1, (plugin_data->vals.res1));    
    v_svf_set_eq(plugin_data->mono_modules->eq1_1, (plugin_data->vals.gain1));
    /*EQ2 Left Channel*/    
    v_svf_set_res(plugin_data->mono_modules->eq2_0, (plugin_data->vals.res2));    
    v_svf_set_eq(plugin_data->mono_modules->eq2_0, (plugin_data->vals.gain2));
    /*EQ2 Right Channel*/  
    v_svf_set_res(plugin_data->mono_modules->eq2_1, (plugin_data->vals.res2));    
    v_svf_set_eq(plugin_data->mono_modules->eq2_1, (plugin_data->vals.gain2));
    /*EQ3 Left Channel*/
    v_svf_set_res(plugin_data->mono_modules->eq3_0, (plugin_data->vals.res3));    
    v_svf_set_eq(plugin_data->mono_modules->eq3_0, (plugin_data->vals.gain3));
    /*EQ3 Right Channel*/
    v_svf_set_res(plugin_data->mono_modules->eq3_1, (plugin_data->vals.res3));    
    v_svf_set_eq(plugin_data->mono_modules->eq3_1, (plugin_data->vals.gain3));
    /*EQ4 Left Channel*/
    v_svf_set_res(plugin_data->mono_modules->eq4_0, (plugin_data->vals.res4));    
    v_svf_set_eq(plugin_data->mono_modules->eq4_0, (plugin_data->vals.gain4));
    /*EQ4 Right Channel*/
    v_svf_set_res(plugin_data->mono_modules->eq4_1, (plugin_data->vals.res4));    
    v_svf_set_eq(plugin_data->mono_modules->eq4_1, (plugin_data->vals.gain4));
    /*EQ5 Left Channel*/
    v_svf_set_res(plugin_data->mono_modules->eq5_0, (plugin_data->vals.res5));    
    v_svf_set_eq(plugin_data->mono_modules->eq5_0, (plugin_data->vals.gain5));
    /*EQ5 Right Channel*/    
    v_svf_set_res(plugin_data->mono_modules->eq5_1, (plugin_data->vals.res5));    
    v_svf_set_eq(plugin_data->mono_modules->eq5_1, (plugin_data->vals.gain5));
        
    while ((plugin_data->pos) < sample_count) 
    {
	plugin_data->count = (sample_count - (plugin_data->pos)) > STEP_SIZE ? STEP_SIZE : sample_count - (plugin_data->pos);
	
        plugin_data->i_buffer_clear = 0;
        /*Clear the output buffer*/
        while((plugin_data->i_buffer_clear)<(plugin_data->count))
        {
	    output0[((plugin_data->pos) + (plugin_data->i_buffer_clear))] = 0.0f;
            output1[((plugin_data->pos) + (plugin_data->i_buffer_clear))] = 0.0f;
            plugin_data->i_buffer_clear = (plugin_data->i_buffer_clear) + 1;
            
#ifdef LMS_DEBUG_MAIN_LOOP
            dump_debug_synth_vals(&plugin_data->vals);
#endif
	}
        
        plugin_data->i_mono_out = 0;
        
        /*The main loop where processing happens*/
        while((plugin_data->i_mono_out) < (plugin_data->count))
        {                           
            plugin_data->buffer_pos = (plugin_data->pos) + (plugin_data->i_mono_out);
            
            plugin_data->mono_modules->out_sample0 = (input0[(plugin_data->buffer_pos)]);
            plugin_data->mono_modules->out_sample1 = (input1[(plugin_data->buffer_pos)]);
            
            /*Run cutoff smoothers*/
            v_sml_run(plugin_data->mono_modules->smoother1, (plugin_data->vals.pitch1));
            v_sml_run(plugin_data->mono_modules->smoother2, (plugin_data->vals.pitch2));
            v_sml_run(plugin_data->mono_modules->smoother3, (plugin_data->vals.pitch3));
            v_sml_run(plugin_data->mono_modules->smoother4, (plugin_data->vals.pitch4));
            v_sml_run(plugin_data->mono_modules->smoother5, (plugin_data->vals.pitch5));
            /*Set cutoff frequencies*/

            /*EQ1*/
            v_svf_set_cutoff_base(plugin_data->mono_modules->eq1_0, (plugin_data->mono_modules->smoother1->last_value));
            v_svf_set_cutoff(plugin_data->mono_modules->eq1_0);
            v_svf_set_cutoff_base(plugin_data->mono_modules->eq1_1, (plugin_data->mono_modules->smoother1->last_value));
            v_svf_set_cutoff(plugin_data->mono_modules->eq1_1);
            /*EQ2*/
            v_svf_set_cutoff_base(plugin_data->mono_modules->eq2_0, (plugin_data->mono_modules->smoother2->last_value));
            v_svf_set_cutoff(plugin_data->mono_modules->eq2_0);
            v_svf_set_cutoff_base(plugin_data->mono_modules->eq2_1, (plugin_data->mono_modules->smoother2->last_value));
            v_svf_set_cutoff(plugin_data->mono_modules->eq2_1);
            /*EQ3*/
            v_svf_set_cutoff_base(plugin_data->mono_modules->eq3_0, (plugin_data->mono_modules->smoother3->last_value));
            v_svf_set_cutoff(plugin_data->mono_modules->eq3_0);        
            v_svf_set_cutoff_base(plugin_data->mono_modules->eq3_1, (plugin_data->mono_modules->smoother3->last_value));
            v_svf_set_cutoff(plugin_data->mono_modules->eq3_1);    
            /*EQ4*/    
            v_svf_set_cutoff_base(plugin_data->mono_modules->eq4_0, (plugin_data->mono_modules->smoother4->last_value));
            v_svf_set_cutoff(plugin_data->mono_modules->eq4_0);    
            v_svf_set_cutoff_base(plugin_data->mono_modules->eq4_1, (plugin_data->mono_modules->smoother4->last_value));
            v_svf_set_cutoff(plugin_data->mono_modules->eq4_1);            
            /*EQ5*/
            v_svf_set_cutoff_base(plugin_data->mono_modules->eq5_0, (plugin_data->mono_modules->smoother5->last_value));
            v_svf_set_cutoff(plugin_data->mono_modules->eq5_0);
            v_svf_set_cutoff_base(plugin_data->mono_modules->eq5_1, (plugin_data->mono_modules->smoother5->last_value));
            v_svf_set_cutoff(plugin_data->mono_modules->eq5_1);

            /*EQ1*/
            plugin_data->mono_modules->out_sample0 = v_svf_run_2_pole_eq(plugin_data->mono_modules->eq1_0, (plugin_data->mono_modules->out_sample0));
            plugin_data->mono_modules->out_sample1 = v_svf_run_2_pole_eq(plugin_data->mono_modules->eq1_1, (plugin_data->mono_modules->out_sample1));
            /*EQ2*/
            plugin_data->mono_modules->out_sample0 = v_svf_run_2_pole_eq(plugin_data->mono_modules->eq2_0, (plugin_data->mono_modules->out_sample0));
            plugin_data->mono_modules->out_sample1 = v_svf_run_2_pole_eq(plugin_data->mono_modules->eq2_1, (plugin_data->mono_modules->out_sample1));            
            /*EQ3*/
            plugin_data->mono_modules->out_sample0 = v_svf_run_2_pole_eq(plugin_data->mono_modules->eq3_0, (plugin_data->mono_modules->out_sample0));
            plugin_data->mono_modules->out_sample1 = v_svf_run_2_pole_eq(plugin_data->mono_modules->eq3_1, (plugin_data->mono_modules->out_sample1));            
            /*EQ4*/
            plugin_data->mono_modules->out_sample0 = v_svf_run_2_pole_eq(plugin_data->mono_modules->eq4_0, (plugin_data->mono_modules->out_sample0));
            plugin_data->mono_modules->out_sample1 = v_svf_run_2_pole_eq(plugin_data->mono_modules->eq4_1, (plugin_data->mono_modules->out_sample1));            
            /*EQ5*/
            plugin_data->mono_modules->out_sample0 = v_svf_run_2_pole_eq(plugin_data->mono_modules->eq5_0, (plugin_data->mono_modules->out_sample0));
            plugin_data->mono_modules->out_sample1 = v_svf_run_2_pole_eq(plugin_data->mono_modules->eq5_1, (plugin_data->mono_modules->out_sample1));            
            
            output0[(plugin_data->buffer_pos)] = (plugin_data->mono_modules->out_sample0);
            
            output1[(plugin_data->buffer_pos)] = (plugin_data->mono_modules->out_sample1);
                    
            plugin_data->i_mono_out = (plugin_data->i_mono_out) + 1;
        }
        
        plugin_data->pos = (plugin_data->pos) + STEP_SIZE;
    }
}



/*This returns MIDI CCs for the different knobs
 TODO:  Try it with non-hex numbers*/
int getControllerLMS(LADSPA_Handle instance, unsigned long port)
{
    LMS *plugin_data = (LMS *) instance;
    return DSSI_CC(i_ccm_get_cc(plugin_data->midi_cc_map, port));
}


/*Here we define how all of the LADSPA and DSSI header stuff is setup,
 we also define the ports and the GUI.*/
#ifdef __GNUC__
__attribute__((constructor)) void init()
#else
void _init()
#endif
{
    char **port_names;
    LADSPA_PortDescriptor *port_descriptors;
    LADSPA_PortRangeHint *port_range_hints;

    LMSLDescriptor =
	(LADSPA_Descriptor *) malloc(sizeof(LADSPA_Descriptor));
    if (LMSLDescriptor) {
        LMSLDescriptor->UniqueID = LMS_PLUGIN_UUID;
	LMSLDescriptor->Label = LMS_PLUGIN_NAME;
	LMSLDescriptor->Properties = LADSPA_PROPERTY_INPLACE_BROKEN; //0;
	LMSLDescriptor->Name = LMS_PLUGIN_LONG_NAME;
	LMSLDescriptor->Maker = LMS_PLUGIN_DEV;
	LMSLDescriptor->Copyright = "GNU GPL v3";
	LMSLDescriptor->PortCount = LMS_COUNT;

	port_descriptors = (LADSPA_PortDescriptor *)
				calloc(LMSLDescriptor->PortCount, sizeof
						(LADSPA_PortDescriptor));
	LMSLDescriptor->PortDescriptors =
	    (const LADSPA_PortDescriptor *) port_descriptors;

	port_range_hints = (LADSPA_PortRangeHint *)
				calloc(LMSLDescriptor->PortCount, sizeof
						(LADSPA_PortRangeHint));
	LMSLDescriptor->PortRangeHints =
	    (const LADSPA_PortRangeHint *) port_range_hints;

	port_names = (char **) calloc(LMSLDescriptor->PortCount, sizeof(char *));
	LMSLDescriptor->PortNames = (const char **) port_names;

        /* Parameters for input */
	port_descriptors[LMS_INPUT0] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
	port_names[LMS_INPUT0] = "Input 0";
	port_range_hints[LMS_INPUT0].HintDescriptor = 0;

        port_descriptors[LMS_INPUT1] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
	port_names[LMS_INPUT1] = "Input 1";
	port_range_hints[LMS_INPUT1].HintDescriptor = 0;
        
	/* Parameters for output */
	port_descriptors[LMS_OUTPUT0] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
	port_names[LMS_OUTPUT0] = "Output 0";
	port_range_hints[LMS_OUTPUT0].HintDescriptor = 0;

        port_descriptors[LMS_OUTPUT1] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
	port_names[LMS_OUTPUT1] = "Output 1";
	port_range_hints[LMS_OUTPUT1].HintDescriptor = 0;
        
        /*Define the LADSPA ports for the plugin in the class constructor*/
        
	/*EQ1*/
        /* Parameters for pitch1 */
	port_descriptors[LMS_PITCH1] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_PITCH1] = "Freq1";
	port_range_hints[LMS_PITCH1].HintDescriptor =
			LADSPA_HINT_DEFAULT_MINIMUM |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PITCH1].LowerBound =  20;
	port_range_hints[LMS_PITCH1].UpperBound =  124;
        
	/* Parameters for gain1 */
	port_descriptors[LMS_GAIN1] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_GAIN1] = "Gain1";
	port_range_hints[LMS_GAIN1].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_GAIN1].LowerBound =  -24;
	port_range_hints[LMS_GAIN1].UpperBound =  24;
        
        /* Parameters for res1 */
	port_descriptors[LMS_RES1] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_RES1] = "Res1";
	port_range_hints[LMS_RES1].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_RES1].LowerBound =  -24;
	port_range_hints[LMS_RES1].UpperBound =  -1;

        /*EQ2*/
        /* Parameters for pitch2 */
	port_descriptors[LMS_PITCH2] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_PITCH2] = "Freq2";
	port_range_hints[LMS_PITCH2].HintDescriptor =
			LADSPA_HINT_DEFAULT_LOW |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PITCH2].LowerBound =  20;
	port_range_hints[LMS_PITCH2].UpperBound =  124;
        
	/* Parameters for gain2 */
	port_descriptors[LMS_GAIN2] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_GAIN2] = "Gain2";
	port_range_hints[LMS_GAIN2].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_GAIN2].LowerBound =  -24;
	port_range_hints[LMS_GAIN2].UpperBound =  24;
        
        /* Parameters for res2 */
	port_descriptors[LMS_RES2] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_RES2] = "Res2";
	port_range_hints[LMS_RES2].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_RES2].LowerBound =  -24;
	port_range_hints[LMS_RES2].UpperBound =  -1;
                
        /*EQ3*/
        /* Parameters for pitch3 */
	port_descriptors[LMS_PITCH3] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_PITCH3] = "Freq3";
	port_range_hints[LMS_PITCH3].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PITCH3].LowerBound =  20;
	port_range_hints[LMS_PITCH3].UpperBound =  124;
        
	/* Parameters for gain3 */
	port_descriptors[LMS_GAIN3] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_GAIN3] = "Gain3";
	port_range_hints[LMS_GAIN3].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_GAIN3].LowerBound =  -24;
	port_range_hints[LMS_GAIN3].UpperBound =  24;
        
        /* Parameters for res3 */
	port_descriptors[LMS_RES3] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_RES3] = "Res3";
	port_range_hints[LMS_RES3].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_RES3].LowerBound =  -24;
	port_range_hints[LMS_RES3].UpperBound =  -1;
        
        /*EQ4*/
        /* Parameters for pitch4 */
	port_descriptors[LMS_PITCH4] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_PITCH4] = "Freq4";
	port_range_hints[LMS_PITCH4].HintDescriptor =
			LADSPA_HINT_DEFAULT_HIGH |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PITCH4].LowerBound =  20;
	port_range_hints[LMS_PITCH4].UpperBound =  124;
        
	/* Parameters for gain4 */
	port_descriptors[LMS_GAIN4] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_GAIN4] = "Gain4";
	port_range_hints[LMS_GAIN4].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_GAIN4].LowerBound =  -24;
	port_range_hints[LMS_GAIN4].UpperBound =  24;
        
        /* Parameters for res4 */
	port_descriptors[LMS_RES4] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_RES4] = "Res4";
	port_range_hints[LMS_RES4].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_RES4].LowerBound =  -24;
	port_range_hints[LMS_RES4].UpperBound =  -1;
        
        /*EQ5*/
        /* Parameters for pitch5 */
	port_descriptors[LMS_PITCH5] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_PITCH5] = "Freq5";
	port_range_hints[LMS_PITCH5].HintDescriptor =
			LADSPA_HINT_DEFAULT_MAXIMUM |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PITCH5].LowerBound =  20;
	port_range_hints[LMS_PITCH5].UpperBound =  124;
        
	/* Parameters for gain5 */
	port_descriptors[LMS_GAIN5] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_GAIN5] = "Gain5";
	port_range_hints[LMS_GAIN5].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_GAIN5].LowerBound =  -24;
	port_range_hints[LMS_GAIN5].UpperBound =  24;
        
        /* Parameters for res5 */
	port_descriptors[LMS_RES5] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_RES5] = "Res5";
	port_range_hints[LMS_RES5].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_RES5].LowerBound =  -24;
	port_range_hints[LMS_RES5].UpperBound =  -1;
        
        /*Step 17:  Add LADSPA ports*/
        
        
        /*Here is where the functions in synth.c get pointed to for the host to call*/
	LMSLDescriptor->activate = activateLMS;
	LMSLDescriptor->cleanup = cleanupLMS;
	LMSLDescriptor->connect_port = connectPortLMS;
	LMSLDescriptor->deactivate = NULL;
	LMSLDescriptor->instantiate = instantiateLMS;
	LMSLDescriptor->run = runLMSWrapper;
	LMSLDescriptor->run_adding = NULL;
	LMSLDescriptor->set_run_adding_gain = NULL;
    }

    LMSDDescriptor = (DSSI_Descriptor *) malloc(sizeof(DSSI_Descriptor));
    if (LMSDDescriptor) {
	LMSDDescriptor->DSSI_API_Version = 1;
	LMSDDescriptor->LADSPA_Plugin = LMSLDescriptor;
	LMSDDescriptor->configure = NULL;  //TODO:  I think this is where the host can set plugin state, etc...
	LMSDDescriptor->get_program = NULL;  //TODO:  This is where program change is read, plugin state retrieved, etc...
	LMSDDescriptor->get_midi_controller_for_port = getControllerLMS;
	LMSDDescriptor->select_program = NULL;  //TODO:  This is how the host can select programs, not sure how it differs from a MIDI program change
	LMSDDescriptor->run_synth = NULL;
	LMSDDescriptor->run_synth_adding = NULL;
	LMSDDescriptor->run_multiple_synths = NULL;
	LMSDDescriptor->run_multiple_synths_adding = NULL;
    }
}

#ifdef __GNUC__
__attribute__((destructor)) void fini()
#else
void _fini()
#endif
{
    if (LMSLDescriptor) {
	free((LADSPA_PortDescriptor *) LMSLDescriptor->PortDescriptors);
	free((char **) LMSLDescriptor->PortNames);
	free((LADSPA_PortRangeHint *) LMSLDescriptor->PortRangeHints);
	free(LMSLDescriptor);
    }
    if (LMSDDescriptor) {
	free(LMSDDescriptor);
    }
}
