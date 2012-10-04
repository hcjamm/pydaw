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
#include "../../libmodsynth/modules/filter/svf.h"

#include "synth.h"
#include "meta.h"

static LADSPA_Descriptor *LMSLDescriptor = NULL;
static DSSI_Descriptor *LMSDDescriptor = NULL;

static void run_lms_modulex(LADSPA_Handle instance, unsigned long sample_count,
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
    
    switch (port) 
    {
        case LMS_INPUT0: plugin->input0 = data; break;
        case LMS_INPUT1: plugin->input1 = data; break;
        case LMS_OUTPUT0: plugin->output0 = data; break;
        case LMS_OUTPUT1: plugin->output1 = data; break;   

        case LMS_FX0_KNOB0: plugin->fx0_knob0 = data; break;
        case LMS_FX0_KNOB1:	plugin->fx0_knob1 = data; break;    
        case LMS_FX0_KNOB2: plugin->fx0_knob2 = data; break;    
        case LMS_FX0_COMBOBOX: plugin->fx0_combobox = data; break;    

        case LMS_FX1_KNOB0: plugin->fx1_knob0 = data; break;
        case LMS_FX1_KNOB1:	plugin->fx1_knob1 = data; break;    
        case LMS_FX1_KNOB2: plugin->fx1_knob2 = data; break;    
        case LMS_FX1_COMBOBOX: plugin->fx1_combobox = data; break;    

        case LMS_FX2_KNOB0: plugin->fx2_knob0 = data; break;
        case LMS_FX2_KNOB1:	plugin->fx2_knob1 = data; break;    
        case LMS_FX2_KNOB2: plugin->fx2_knob2 = data; break;    
        case LMS_FX2_COMBOBOX: plugin->fx2_combobox = data; break;    

        case LMS_FX3_KNOB0: plugin->fx3_knob0 = data; break;
        case LMS_FX3_KNOB1:	plugin->fx3_knob1 = data; break;    
        case LMS_FX3_KNOB2: plugin->fx3_knob2 = data; break;    
        case LMS_FX3_COMBOBOX: plugin->fx3_combobox = data; break;    
    }
}

static LADSPA_Handle instantiateLMS(const LADSPA_Descriptor * descriptor,
				   unsigned long s_rate)
{
    LMS *plugin_data = (LMS *) malloc(sizeof(LMS));
    
    plugin_data->fs = s_rate;
    
    plugin_data->midi_cc_map = g_ccm_get();
    
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_FX0_KNOB0, 74, "FX0Knob0");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_FX0_KNOB1, 71, "FX0Knob1");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_FX0_KNOB2, 75, "FX0Knob2");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_FX0_COMBOBOX, 92, "FX0Combobox");
    
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_FX1_KNOB0, 70, "FX1Knob0");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_FX1_KNOB1, 91, "FX1Knob1");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_FX1_KNOB2, 28, "FX1Knob2");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_FX1_COMBOBOX, 23, "FX1Combobox");
    
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_FX2_KNOB0, 20, "FX2Knob0");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_FX2_KNOB1, 21, "FX2Knob1");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_FX2_KNOB2, 26, "FX2Knob2");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_FX2_COMBOBOX, 27, "FX2Combobox");
    
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_FX3_KNOB0, 22, "FX3Knob0");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_FX3_KNOB1, 5, "FX3Knob1");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_FX3_KNOB2, 37, "FX3Knob2");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_FX3_COMBOBOX, 38, "FX3Combobox");
    
    v_ccm_read_file_to_array(plugin_data->midi_cc_map, "lms_modulex-cc_map.txt");
    
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
    run_lms_modulex(instance, sample_count, NULL, 0);
}

/*This is where parameters are update and the main loop is run.*/
static void run_lms_modulex(LADSPA_Handle instance, unsigned long sample_count,
		  snd_seq_event_t *events, unsigned long event_count)
{
    LMS *plugin_data = (LMS *) instance;
    /*Define our inputs*/
    LADSPA_Data *const input0 = plugin_data->input0;    
    LADSPA_Data *const input1 = plugin_data->input1;
    /*define our outputs*/
    LADSPA_Data *const output0 = plugin_data->output0;    
    LADSPA_Data *const output1 = plugin_data->output1;    
    
    plugin_data->mono_modules->fx_func_ptr0 = g_mf3_get_function_pointer((int)(*(plugin_data->fx0_combobox)));
    plugin_data->mono_modules->fx_func_ptr1 = g_mf3_get_function_pointer((int)(*(plugin_data->fx1_combobox)));
    plugin_data->mono_modules->fx_func_ptr2 = g_mf3_get_function_pointer((int)(*(plugin_data->fx2_combobox)));
    plugin_data->mono_modules->fx_func_ptr3 = g_mf3_get_function_pointer((int)(*(plugin_data->fx3_combobox)));

    v_mf3_set(plugin_data->mono_modules->multieffect0, 
            *(plugin_data->fx0_knob0), *(plugin_data->fx0_knob1), *(plugin_data->fx0_knob2));

    v_mf3_set(plugin_data->mono_modules->multieffect1, 
            *(plugin_data->fx1_knob0), *(plugin_data->fx1_knob1), *(plugin_data->fx1_knob2));

    v_mf3_set(plugin_data->mono_modules->multieffect2, 
            *(plugin_data->fx2_knob0), *(plugin_data->fx2_knob1), *(plugin_data->fx2_knob2));

    v_mf3_set(plugin_data->mono_modules->multieffect3, 
            *(plugin_data->fx3_knob0), *(plugin_data->fx3_knob1), *(plugin_data->fx3_knob2));

    plugin_data->i_buffer_clear = 0;
    /*Clear the output buffer*/
    while((plugin_data->i_buffer_clear) < sample_count)
    {
        output0[(plugin_data->i_buffer_clear)] = 0.0f;                        
        output1[(plugin_data->i_buffer_clear)] = 0.0f;     
        plugin_data->i_buffer_clear = (plugin_data->i_buffer_clear) + 1;
    }

    plugin_data->i_mono_out = 0;

    /*The main loop where processing happens*/
    while((plugin_data->i_mono_out) < sample_count)
    {
        plugin_data->mono_modules->current_sample0 = input0[(plugin_data->i_mono_out)];
        plugin_data->mono_modules->current_sample1 = input1[(plugin_data->i_mono_out)];

        plugin_data->mono_modules->fx_func_ptr0(plugin_data->mono_modules->multieffect0, (plugin_data->mono_modules->current_sample0), (plugin_data->mono_modules->current_sample1)); 

        plugin_data->mono_modules->current_sample0 = plugin_data->mono_modules->multieffect0->output0;
        plugin_data->mono_modules->current_sample1 = plugin_data->mono_modules->multieffect0->output1;

        plugin_data->mono_modules->fx_func_ptr1(plugin_data->mono_modules->multieffect1, (plugin_data->mono_modules->current_sample0), (plugin_data->mono_modules->current_sample1)); 

        plugin_data->mono_modules->current_sample0 = plugin_data->mono_modules->multieffect1->output0;
        plugin_data->mono_modules->current_sample1 = plugin_data->mono_modules->multieffect1->output1;

        plugin_data->mono_modules->fx_func_ptr2(plugin_data->mono_modules->multieffect2, (plugin_data->mono_modules->current_sample0), (plugin_data->mono_modules->current_sample1)); 

        plugin_data->mono_modules->current_sample0 = plugin_data->mono_modules->multieffect2->output0;
        plugin_data->mono_modules->current_sample1 = plugin_data->mono_modules->multieffect2->output1;

        plugin_data->mono_modules->fx_func_ptr3(plugin_data->mono_modules->multieffect3, (plugin_data->mono_modules->current_sample0), (plugin_data->mono_modules->current_sample1)); 

        plugin_data->mono_modules->current_sample0 = plugin_data->mono_modules->multieffect3->output0;
        plugin_data->mono_modules->current_sample1 = plugin_data->mono_modules->multieffect3->output1;

        output0[(plugin_data->i_mono_out)] = (plugin_data->mono_modules->current_sample0);
        output1[(plugin_data->i_mono_out)] = (plugin_data->mono_modules->current_sample1);

        plugin_data->i_mono_out = (plugin_data->i_mono_out) + 1;
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
	LMSLDescriptor->Properties = LADSPA_PROPERTY_HARD_RT_CAPABLE | LADSPA_PROPERTY_INPLACE_BROKEN;
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
        	
	port_descriptors[LMS_FX0_KNOB0] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_FX0_KNOB0] = "FX0 Knob0";
	port_range_hints[LMS_FX0_KNOB0].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_FX0_KNOB0].LowerBound =  0;
	port_range_hints[LMS_FX0_KNOB0].UpperBound =  127;
        
        	
	port_descriptors[LMS_FX0_KNOB1] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_FX0_KNOB1] = "FX0 Knob1";
	port_range_hints[LMS_FX0_KNOB1].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_FX0_KNOB1].LowerBound =  0;
	port_range_hints[LMS_FX0_KNOB1].UpperBound =  127;
        	
	port_descriptors[LMS_FX0_KNOB2] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_FX0_KNOB2] = "FX0 Knob2";
	port_range_hints[LMS_FX0_KNOB2].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_FX0_KNOB2].LowerBound =  0;
	port_range_hints[LMS_FX0_KNOB2].UpperBound =  127;
        
	port_descriptors[LMS_FX0_COMBOBOX] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_FX0_COMBOBOX] = "FX0 Type";
	port_range_hints[LMS_FX0_COMBOBOX].HintDescriptor =
                        LADSPA_HINT_DEFAULT_MINIMUM | LADSPA_HINT_INTEGER |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_FX0_COMBOBOX].LowerBound =  0;
	port_range_hints[LMS_FX0_COMBOBOX].UpperBound =  MULTIFX3KNOB_MAX_INDEX;
        
        	
	port_descriptors[LMS_FX1_KNOB0] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_FX1_KNOB0] = "FX1 Knob0";
	port_range_hints[LMS_FX1_KNOB0].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_FX1_KNOB0].LowerBound =  0;
	port_range_hints[LMS_FX1_KNOB0].UpperBound =  127;
        
        	
	port_descriptors[LMS_FX1_KNOB1] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_FX1_KNOB1] = "FX1 Knob1";
	port_range_hints[LMS_FX1_KNOB1].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_FX1_KNOB1].LowerBound =  0;
	port_range_hints[LMS_FX1_KNOB1].UpperBound =  127;
        	
	port_descriptors[LMS_FX1_KNOB2] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_FX1_KNOB2] = "FX1 Knob2";
	port_range_hints[LMS_FX1_KNOB2].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_FX1_KNOB2].LowerBound =  0;
	port_range_hints[LMS_FX1_KNOB2].UpperBound =  127;
        
	port_descriptors[LMS_FX1_COMBOBOX] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_FX1_COMBOBOX] = "FX1 Type";
	port_range_hints[LMS_FX1_COMBOBOX].HintDescriptor =
                        LADSPA_HINT_DEFAULT_MINIMUM | LADSPA_HINT_INTEGER |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_FX1_COMBOBOX].LowerBound =  0;
	port_range_hints[LMS_FX1_COMBOBOX].UpperBound =  MULTIFX3KNOB_MAX_INDEX;
        
        
        
        port_descriptors[LMS_FX2_KNOB0] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_FX2_KNOB0] = "FX2 Knob0";
	port_range_hints[LMS_FX2_KNOB0].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_FX2_KNOB0].LowerBound =  0;
	port_range_hints[LMS_FX2_KNOB0].UpperBound =  127;
        
        	
	port_descriptors[LMS_FX2_KNOB1] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_FX2_KNOB1] = "FX2 Knob1";
	port_range_hints[LMS_FX2_KNOB1].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_FX2_KNOB1].LowerBound =  0;
	port_range_hints[LMS_FX2_KNOB1].UpperBound =  127;
        	
	port_descriptors[LMS_FX2_KNOB2] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_FX2_KNOB2] = "FX2 Knob2";
	port_range_hints[LMS_FX2_KNOB2].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_FX2_KNOB2].LowerBound =  0;
	port_range_hints[LMS_FX2_KNOB2].UpperBound =  127;
        
	port_descriptors[LMS_FX2_COMBOBOX] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_FX2_COMBOBOX] = "FX2 Type";
	port_range_hints[LMS_FX2_COMBOBOX].HintDescriptor =
                        LADSPA_HINT_DEFAULT_MINIMUM | LADSPA_HINT_INTEGER |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_FX2_COMBOBOX].LowerBound =  0;
	port_range_hints[LMS_FX2_COMBOBOX].UpperBound =  MULTIFX3KNOB_MAX_INDEX;
        
        	
	port_descriptors[LMS_FX3_KNOB0] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_FX3_KNOB0] = "FX3 Knob0";
	port_range_hints[LMS_FX3_KNOB0].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_FX3_KNOB0].LowerBound =  0;
	port_range_hints[LMS_FX3_KNOB0].UpperBound =  127;
        
        	
	port_descriptors[LMS_FX3_KNOB1] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_FX3_KNOB1] = "FX3 Knob1";
	port_range_hints[LMS_FX3_KNOB1].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_FX3_KNOB1].LowerBound =  0;
	port_range_hints[LMS_FX3_KNOB1].UpperBound =  127;
        	
	port_descriptors[LMS_FX3_KNOB2] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_FX3_KNOB2] = "FX3 Knob2";
	port_range_hints[LMS_FX3_KNOB2].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_FX3_KNOB2].LowerBound =  0;
	port_range_hints[LMS_FX3_KNOB2].UpperBound =  127;
        
	port_descriptors[LMS_FX3_COMBOBOX] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_FX3_COMBOBOX] = "FX3 Type";
	port_range_hints[LMS_FX3_COMBOBOX].HintDescriptor =
                        LADSPA_HINT_DEFAULT_MINIMUM | LADSPA_HINT_INTEGER |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_FX3_COMBOBOX].LowerBound =  0;
	port_range_hints[LMS_FX3_COMBOBOX].UpperBound =  MULTIFX3KNOB_MAX_INDEX;
        
        
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
