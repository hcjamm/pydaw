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

static void v_modulex_run(LADSPA_Handle instance, unsigned long sample_count,
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

static void v_modulex_cleanup(LADSPA_Handle instance)
{
    free(instance);
}

static void v_modulex_connect_port(LADSPA_Handle instance, unsigned long port,
			  LADSPA_Data * data)
{
    t_modulex *plugin;

    plugin = (t_modulex *) instance;
    
    /*Add the ports from step 9 to the connectPortLMS event handler*/
    
    switch (port) 
    {
        case MODULEX_INPUT0: plugin->input0 = data; break;
        case MODULEX_INPUT1: plugin->input1 = data; break;
        case MODULEX_OUTPUT0: plugin->output0 = data; break;
        case MODULEX_OUTPUT1: plugin->output1 = data; break;   

        case MODULEX_FX0_KNOB0: plugin->fx0_knob0 = data; break;
        case MODULEX_FX0_KNOB1:	plugin->fx0_knob1 = data; break;    
        case MODULEX_FX0_KNOB2: plugin->fx0_knob2 = data; break;    
        case MODULEX_FX0_COMBOBOX: plugin->fx0_combobox = data; break;    

        case MODULEX_FX1_KNOB0: plugin->fx1_knob0 = data; break;
        case MODULEX_FX1_KNOB1:	plugin->fx1_knob1 = data; break;    
        case MODULEX_FX1_KNOB2: plugin->fx1_knob2 = data; break;    
        case MODULEX_FX1_COMBOBOX: plugin->fx1_combobox = data; break;    

        case MODULEX_FX2_KNOB0: plugin->fx2_knob0 = data; break;
        case MODULEX_FX2_KNOB1:	plugin->fx2_knob1 = data; break;    
        case MODULEX_FX2_KNOB2: plugin->fx2_knob2 = data; break;    
        case MODULEX_FX2_COMBOBOX: plugin->fx2_combobox = data; break;    

        case MODULEX_FX3_KNOB0: plugin->fx3_knob0 = data; break;
        case MODULEX_FX3_KNOB1:	plugin->fx3_knob1 = data; break;    
        case MODULEX_FX3_KNOB2: plugin->fx3_knob2 = data; break;    
        case MODULEX_FX3_COMBOBOX: plugin->fx3_combobox = data; break;
        
        case MODULEX_FX4_KNOB0: plugin->fx4_knob0 = data; break;
        case MODULEX_FX4_KNOB1:	plugin->fx4_knob1 = data; break;    
        case MODULEX_FX4_KNOB2: plugin->fx4_knob2 = data; break;    
        case MODULEX_FX4_COMBOBOX: plugin->fx4_combobox = data; break;    

        case MODULEX_FX5_KNOB0: plugin->fx5_knob0 = data; break;
        case MODULEX_FX5_KNOB1:	plugin->fx5_knob1 = data; break;    
        case MODULEX_FX5_KNOB2: plugin->fx5_knob2 = data; break;    
        case MODULEX_FX5_COMBOBOX: plugin->fx5_combobox = data; break;    

        case MODULEX_FX6_KNOB0: plugin->fx6_knob0 = data; break;
        case MODULEX_FX6_KNOB1:	plugin->fx6_knob1 = data; break;    
        case MODULEX_FX6_KNOB2: plugin->fx6_knob2 = data; break;    
        case MODULEX_FX6_COMBOBOX: plugin->fx6_combobox = data; break;    

        case MODULEX_FX7_KNOB0: plugin->fx7_knob0 = data; break;
        case MODULEX_FX7_KNOB1:	plugin->fx7_knob1 = data; break;    
        case MODULEX_FX7_KNOB2: plugin->fx7_knob2 = data; break;    
        case MODULEX_FX7_COMBOBOX: plugin->fx7_combobox = data; break;     
        
        case MODULEX_DELAY_TIME: plugin->delay_time = data; break;
        case MODULEX_FEEDBACK: plugin->feedback = data; break;
        case MODULEX_DRY: plugin->dry = data;  break;
        case MODULEX_WET: plugin->wet = data; break;
        case MODULEX_DUCK: plugin->duck = data; break;
        case MODULEX_CUTOFF: plugin->cutoff = data; break;
        case MODULEX_STEREO: plugin->stereo = data; break;
    }
}

static LADSPA_Handle g_modulex_instantiate(const LADSPA_Descriptor * descriptor,
				   unsigned long s_rate)
{
    t_modulex *plugin_data = (t_modulex *) malloc(sizeof(t_modulex));
    
    plugin_data->fs = s_rate;
    
    plugin_data->midi_cc_map = g_ccm_get();
    
    v_ccm_set_cc(plugin_data->midi_cc_map, MODULEX_FX0_KNOB0, 75, "FX0Knob0");
    v_ccm_set_cc(plugin_data->midi_cc_map, MODULEX_FX0_KNOB1, 76, "FX0Knob1");
    v_ccm_set_cc(plugin_data->midi_cc_map, MODULEX_FX0_KNOB2, 97, "FX0Knob2");
    
    v_ccm_set_cc(plugin_data->midi_cc_map, MODULEX_FX1_KNOB0, 77, "FX1Knob0");
    v_ccm_set_cc(plugin_data->midi_cc_map, MODULEX_FX1_KNOB1, 7, "FX1Knob1");
    v_ccm_set_cc(plugin_data->midi_cc_map, MODULEX_FX1_KNOB2, 98, "FX1Knob2");
    
    v_ccm_set_cc(plugin_data->midi_cc_map, MODULEX_FX2_KNOB0, 99, "FX2Knob0");
    v_ccm_set_cc(plugin_data->midi_cc_map, MODULEX_FX2_KNOB1, 100, "FX2Knob1");
    v_ccm_set_cc(plugin_data->midi_cc_map, MODULEX_FX2_KNOB2, 101, "FX2Knob2");
    
    v_ccm_set_cc(plugin_data->midi_cc_map, MODULEX_FX3_KNOB0, 102, "FX3Knob0");
    v_ccm_set_cc(plugin_data->midi_cc_map, MODULEX_FX3_KNOB1, 103, "FX3Knob1");
    v_ccm_set_cc(plugin_data->midi_cc_map, MODULEX_FX3_KNOB2, 104, "FX3Knob2");
    
    v_ccm_set_cc(plugin_data->midi_cc_map, MODULEX_FX4_KNOB0, 105, "FX0Knob0");
    v_ccm_set_cc(plugin_data->midi_cc_map, MODULEX_FX4_KNOB1, 106, "FX0Knob1");
    v_ccm_set_cc(plugin_data->midi_cc_map, MODULEX_FX4_KNOB2, 107, "FX0Knob2");
    
    v_ccm_set_cc(plugin_data->midi_cc_map, MODULEX_FX5_KNOB0, 108, "FX1Knob0");
    v_ccm_set_cc(plugin_data->midi_cc_map, MODULEX_FX5_KNOB1, 109, "FX1Knob1");
    v_ccm_set_cc(plugin_data->midi_cc_map, MODULEX_FX5_KNOB2, 110, "FX1Knob2");
    
    v_ccm_set_cc(plugin_data->midi_cc_map, MODULEX_FX6_KNOB0, 111, "FX2Knob0");
    v_ccm_set_cc(plugin_data->midi_cc_map, MODULEX_FX6_KNOB1, 112, "FX2Knob1");
    v_ccm_set_cc(plugin_data->midi_cc_map, MODULEX_FX6_KNOB2, 113, "FX2Knob2");
    
    v_ccm_set_cc(plugin_data->midi_cc_map, MODULEX_FX7_KNOB0, 114, "FX3Knob0");
    v_ccm_set_cc(plugin_data->midi_cc_map, MODULEX_FX7_KNOB1, 115, "FX3Knob1");
    v_ccm_set_cc(plugin_data->midi_cc_map, MODULEX_FX7_KNOB2, 116, "FX3Knob2");
    
    v_ccm_set_cc(plugin_data->midi_cc_map, MODULEX_WET, 117, "Delay Wet");
    v_ccm_set_cc(plugin_data->midi_cc_map, MODULEX_DRY, 118, "Delay Dry");
    v_ccm_set_cc(plugin_data->midi_cc_map, MODULEX_FEEDBACK, 117, "Delay Feedback");
    v_ccm_set_cc(plugin_data->midi_cc_map, MODULEX_CUTOFF, 118, "Delay Cutoff");
    
    
    v_ccm_read_file_to_array(plugin_data->midi_cc_map, "lms_modulex-cc_map.txt");
    
    return (LADSPA_Handle) plugin_data;
}

static void v_modulex_activate(LADSPA_Handle instance)
{
    t_modulex *plugin_data = (t_modulex *) instance;
        
    plugin_data->mono_modules = v_modulex_mono_init((plugin_data->fs));  //initialize all monophonic modules    
}

static void v_modulex_run_wrapper(LADSPA_Handle instance,
			 unsigned long sample_count)
{
    v_modulex_run(instance, sample_count, NULL, 0);
}

static void v_modulex_run(LADSPA_Handle instance, unsigned long sample_count,
		  snd_seq_event_t *events, unsigned long event_count)
{
    t_modulex *plugin_data = (t_modulex *) instance;
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
    
    plugin_data->mono_modules->fx_func_ptr4 = g_mf3_get_function_pointer((int)(*(plugin_data->fx4_combobox)));
    plugin_data->mono_modules->fx_func_ptr5 = g_mf3_get_function_pointer((int)(*(plugin_data->fx5_combobox)));
    plugin_data->mono_modules->fx_func_ptr6 = g_mf3_get_function_pointer((int)(*(plugin_data->fx6_combobox)));
    plugin_data->mono_modules->fx_func_ptr7 = g_mf3_get_function_pointer((int)(*(plugin_data->fx7_combobox)));

    v_mf3_set(plugin_data->mono_modules->multieffect0, 
            *(plugin_data->fx0_knob0), *(plugin_data->fx0_knob1), *(plugin_data->fx0_knob2));

    v_mf3_set(plugin_data->mono_modules->multieffect1, 
            *(plugin_data->fx1_knob0), *(plugin_data->fx1_knob1), *(plugin_data->fx1_knob2));

    v_mf3_set(plugin_data->mono_modules->multieffect2, 
            *(plugin_data->fx2_knob0), *(plugin_data->fx2_knob1), *(plugin_data->fx2_knob2));

    v_mf3_set(plugin_data->mono_modules->multieffect3, 
            *(plugin_data->fx3_knob0), *(plugin_data->fx3_knob1), *(plugin_data->fx3_knob2));
    
    v_mf3_set(plugin_data->mono_modules->multieffect4, 
            *(plugin_data->fx4_knob0), *(plugin_data->fx4_knob1), *(plugin_data->fx4_knob2));

    v_mf3_set(plugin_data->mono_modules->multieffect5, 
            *(plugin_data->fx5_knob0), *(plugin_data->fx5_knob1), *(plugin_data->fx5_knob2));

    v_mf3_set(plugin_data->mono_modules->multieffect6, 
            *(plugin_data->fx6_knob0), *(plugin_data->fx6_knob1), *(plugin_data->fx6_knob2));

    v_mf3_set(plugin_data->mono_modules->multieffect7, 
            *(plugin_data->fx7_knob0), *(plugin_data->fx7_knob1), *(plugin_data->fx7_knob2));
    
    
    v_svf_set_cutoff_base(plugin_data->mono_modules->svf0, *(plugin_data->cutoff));
    v_svf_set_cutoff_base(plugin_data->mono_modules->svf1, *(plugin_data->cutoff));
            
    v_svf_set_cutoff(plugin_data->mono_modules->svf0);
    v_svf_set_cutoff(plugin_data->mono_modules->svf1);
    
    plugin_data->i_buffer_clear = 0;
    /*Clear the output buffer*/
    while((plugin_data->i_buffer_clear) < sample_count)
    {
        output0[(plugin_data->i_buffer_clear)] = 0.0f;                        
        output1[(plugin_data->i_buffer_clear)] = 0.0f;     
        plugin_data->i_buffer_clear = (plugin_data->i_buffer_clear) + 1;
    }

    plugin_data->i_mono_out = 0;

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
        
        plugin_data->mono_modules->fx_func_ptr4(plugin_data->mono_modules->multieffect4, (plugin_data->mono_modules->current_sample0), (plugin_data->mono_modules->current_sample1)); 

        plugin_data->mono_modules->current_sample0 = plugin_data->mono_modules->multieffect4->output0;
        plugin_data->mono_modules->current_sample1 = plugin_data->mono_modules->multieffect4->output1;

        plugin_data->mono_modules->fx_func_ptr5(plugin_data->mono_modules->multieffect5, (plugin_data->mono_modules->current_sample0), (plugin_data->mono_modules->current_sample1)); 

        plugin_data->mono_modules->current_sample0 = plugin_data->mono_modules->multieffect5->output0;
        plugin_data->mono_modules->current_sample1 = plugin_data->mono_modules->multieffect5->output1;

        plugin_data->mono_modules->fx_func_ptr6(plugin_data->mono_modules->multieffect6, (plugin_data->mono_modules->current_sample0), (plugin_data->mono_modules->current_sample1)); 

        plugin_data->mono_modules->current_sample0 = plugin_data->mono_modules->multieffect6->output0;
        plugin_data->mono_modules->current_sample1 = plugin_data->mono_modules->multieffect6->output1;

        plugin_data->mono_modules->fx_func_ptr7(plugin_data->mono_modules->multieffect7, (plugin_data->mono_modules->current_sample0), (plugin_data->mono_modules->current_sample1)); 

        plugin_data->mono_modules->current_sample0 = plugin_data->mono_modules->multieffect7->output0;
        plugin_data->mono_modules->current_sample1 = plugin_data->mono_modules->multieffect7->output1;
        
        /*TODO
         * 
         * Optimize all of these * .01s...
         * 
         */
        
        if((*(plugin_data->wet)) < -29.0f)
        {
            output0[(plugin_data->i_mono_out)] = (plugin_data->mono_modules->current_sample0);
            output1[(plugin_data->i_mono_out)] = (plugin_data->mono_modules->current_sample1);
        }
        else
        {
            v_smr_iir_run(plugin_data->mono_modules->time_smoother, (*(plugin_data->delay_time) * .01));

            //Not using ducking for now...  Need to do a better version that uses the new limiter
            //v_enf_run_env_follower(plugin_data->mono_modules->env_follower, ((plugin_data->mono_modules->current_sample0) + (plugin_data->mono_modules->current_sample1)));

            /*If above the ducking threshold, reduce the wet amount by the same*/
            /*if((plugin_data->mono_modules->env_follower->output_smoothed) > *(plugin_data->duck))
            {
                v_ldl_set_delay(plugin_data->mono_modules->delay, (plugin_data->mono_modules->time_smoother->output), 
                        *(plugin_data->feedback),
                        (*(plugin_data->wet) - ((plugin_data->mono_modules->env_follower->output_smoothed) - *(plugin_data->duck))), 
                        *(plugin_data->dry), (*(plugin_data->stereo) * .01));
            }
            else
            {*/
                v_ldl_set_delay(plugin_data->mono_modules->delay, (plugin_data->mono_modules->time_smoother->output), *(plugin_data->feedback), 
                        *(plugin_data->wet), *(plugin_data->dry), (*(plugin_data->stereo) * .01));
            //}

            v_ldl_run_delay(plugin_data->mono_modules->delay, (plugin_data->mono_modules->current_sample0), (plugin_data->mono_modules->current_sample1));        

            output0[(plugin_data->i_mono_out)] = (plugin_data->mono_modules->delay->output0);
            output1[(plugin_data->i_mono_out)] = (plugin_data->mono_modules->delay->output1);

            plugin_data->mono_modules->delay->feedback0 = v_svf_run_2_pole_lp(plugin_data->mono_modules->svf0, (plugin_data->mono_modules->delay->feedback0));
            plugin_data->mono_modules->delay->feedback1 = v_svf_run_2_pole_lp(plugin_data->mono_modules->svf1, (plugin_data->mono_modules->delay->feedback1));

        }
        
        plugin_data->i_mono_out = (plugin_data->i_mono_out) + 1;
    }
}


int i_modulex_get_controller(LADSPA_Handle instance, unsigned long port)
{    
    t_modulex *plugin_data = (t_modulex *) instance;
    return DSSI_CC(i_ccm_get_cc(plugin_data->midi_cc_map, port));     
}

#ifdef __GNUC__
__attribute__((constructor)) void v_modulex_init()
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
        LMSLDescriptor->UniqueID = MODULEX_PLUGIN_UUID;
	LMSLDescriptor->Label = MODULEX_PLUGIN_NAME;
	LMSLDescriptor->Properties = LADSPA_PROPERTY_HARD_RT_CAPABLE | LADSPA_PROPERTY_INPLACE_BROKEN;
	LMSLDescriptor->Name = MODULEX_PLUGIN_LONG_NAME;
	LMSLDescriptor->Maker = MODULEX_PLUGIN_DEV;
	LMSLDescriptor->Copyright = "GNU GPL v3";
	LMSLDescriptor->PortCount = MODULEX_COUNT;

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
	port_descriptors[MODULEX_INPUT0] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
	port_names[MODULEX_INPUT0] = "Input 0";
	port_range_hints[MODULEX_INPUT0].HintDescriptor = 0;

        port_descriptors[MODULEX_INPUT1] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
	port_names[MODULEX_INPUT1] = "Input 1";
	port_range_hints[MODULEX_INPUT1].HintDescriptor = 0;
        
	/* Parameters for output */
	port_descriptors[MODULEX_OUTPUT0] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
	port_names[MODULEX_OUTPUT0] = "Output 0";
	port_range_hints[MODULEX_OUTPUT0].HintDescriptor = 0;

        port_descriptors[MODULEX_OUTPUT1] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
	port_names[MODULEX_OUTPUT1] = "Output 1";
	port_range_hints[MODULEX_OUTPUT1].HintDescriptor = 0;
        
        
	port_descriptors[MODULEX_FX0_KNOB0] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[MODULEX_FX0_KNOB0] = "FX0 Knob0";
	port_range_hints[MODULEX_FX0_KNOB0].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX0_KNOB0].LowerBound =  0;
	port_range_hints[MODULEX_FX0_KNOB0].UpperBound =  127;
        
        	
	port_descriptors[MODULEX_FX0_KNOB1] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[MODULEX_FX0_KNOB1] = "FX0 Knob1";
	port_range_hints[MODULEX_FX0_KNOB1].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX0_KNOB1].LowerBound =  0;
	port_range_hints[MODULEX_FX0_KNOB1].UpperBound =  127;
        	
	port_descriptors[MODULEX_FX0_KNOB2] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[MODULEX_FX0_KNOB2] = "FX0 Knob2";
	port_range_hints[MODULEX_FX0_KNOB2].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX0_KNOB2].LowerBound =  0;
	port_range_hints[MODULEX_FX0_KNOB2].UpperBound =  127;
        
	port_descriptors[MODULEX_FX0_COMBOBOX] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[MODULEX_FX0_COMBOBOX] = "FX0 Type";
	port_range_hints[MODULEX_FX0_COMBOBOX].HintDescriptor =
                        LADSPA_HINT_DEFAULT_MINIMUM | LADSPA_HINT_INTEGER |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX0_COMBOBOX].LowerBound =  0;
	port_range_hints[MODULEX_FX0_COMBOBOX].UpperBound =  MULTIFX3KNOB_MAX_INDEX;
        
        	
	port_descriptors[MODULEX_FX1_KNOB0] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[MODULEX_FX1_KNOB0] = "FX1 Knob0";
	port_range_hints[MODULEX_FX1_KNOB0].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX1_KNOB0].LowerBound =  0;
	port_range_hints[MODULEX_FX1_KNOB0].UpperBound =  127;
        
        	
	port_descriptors[MODULEX_FX1_KNOB1] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[MODULEX_FX1_KNOB1] = "FX1 Knob1";
	port_range_hints[MODULEX_FX1_KNOB1].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX1_KNOB1].LowerBound =  0;
	port_range_hints[MODULEX_FX1_KNOB1].UpperBound =  127;
        	
	port_descriptors[MODULEX_FX1_KNOB2] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[MODULEX_FX1_KNOB2] = "FX1 Knob2";
	port_range_hints[MODULEX_FX1_KNOB2].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX1_KNOB2].LowerBound =  0;
	port_range_hints[MODULEX_FX1_KNOB2].UpperBound =  127;
        
	port_descriptors[MODULEX_FX1_COMBOBOX] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[MODULEX_FX1_COMBOBOX] = "FX1 Type";
	port_range_hints[MODULEX_FX1_COMBOBOX].HintDescriptor =
                        LADSPA_HINT_DEFAULT_MINIMUM | LADSPA_HINT_INTEGER |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX1_COMBOBOX].LowerBound =  0;
	port_range_hints[MODULEX_FX1_COMBOBOX].UpperBound =  MULTIFX3KNOB_MAX_INDEX;
        
        
        
        port_descriptors[MODULEX_FX2_KNOB0] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[MODULEX_FX2_KNOB0] = "FX2 Knob0";
	port_range_hints[MODULEX_FX2_KNOB0].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX2_KNOB0].LowerBound =  0;
	port_range_hints[MODULEX_FX2_KNOB0].UpperBound =  127;
        
        	
	port_descriptors[MODULEX_FX2_KNOB1] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[MODULEX_FX2_KNOB1] = "FX2 Knob1";
	port_range_hints[MODULEX_FX2_KNOB1].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX2_KNOB1].LowerBound =  0;
	port_range_hints[MODULEX_FX2_KNOB1].UpperBound =  127;
        	
	port_descriptors[MODULEX_FX2_KNOB2] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[MODULEX_FX2_KNOB2] = "FX2 Knob2";
	port_range_hints[MODULEX_FX2_KNOB2].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX2_KNOB2].LowerBound =  0;
	port_range_hints[MODULEX_FX2_KNOB2].UpperBound =  127;
        
	port_descriptors[MODULEX_FX2_COMBOBOX] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[MODULEX_FX2_COMBOBOX] = "FX2 Type";
	port_range_hints[MODULEX_FX2_COMBOBOX].HintDescriptor =
                        LADSPA_HINT_DEFAULT_MINIMUM | LADSPA_HINT_INTEGER |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX2_COMBOBOX].LowerBound =  0;
	port_range_hints[MODULEX_FX2_COMBOBOX].UpperBound =  MULTIFX3KNOB_MAX_INDEX;
        
        	
	port_descriptors[MODULEX_FX3_KNOB0] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[MODULEX_FX3_KNOB0] = "FX3 Knob0";
	port_range_hints[MODULEX_FX3_KNOB0].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX3_KNOB0].LowerBound =  0;
	port_range_hints[MODULEX_FX3_KNOB0].UpperBound =  127;
        
        	
	port_descriptors[MODULEX_FX3_KNOB1] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[MODULEX_FX3_KNOB1] = "FX3 Knob1";
	port_range_hints[MODULEX_FX3_KNOB1].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX3_KNOB1].LowerBound =  0;
	port_range_hints[MODULEX_FX3_KNOB1].UpperBound =  127;
        	
	port_descriptors[MODULEX_FX3_KNOB2] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[MODULEX_FX3_KNOB2] = "FX3 Knob2";
	port_range_hints[MODULEX_FX3_KNOB2].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX3_KNOB2].LowerBound =  0;
	port_range_hints[MODULEX_FX3_KNOB2].UpperBound =  127;
        
	port_descriptors[MODULEX_FX3_COMBOBOX] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[MODULEX_FX3_COMBOBOX] = "FX3 Type";
	port_range_hints[MODULEX_FX3_COMBOBOX].HintDescriptor =
                        LADSPA_HINT_DEFAULT_MINIMUM | LADSPA_HINT_INTEGER |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX3_COMBOBOX].LowerBound =  0;
	port_range_hints[MODULEX_FX3_COMBOBOX].UpperBound =  MULTIFX3KNOB_MAX_INDEX;
        
        
        
        
        
	port_descriptors[MODULEX_FX4_KNOB0] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[MODULEX_FX4_KNOB0] = "FX4 Knob0";
	port_range_hints[MODULEX_FX4_KNOB0].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX4_KNOB0].LowerBound =  0;
	port_range_hints[MODULEX_FX4_KNOB0].UpperBound =  127;
        
        	
	port_descriptors[MODULEX_FX4_KNOB1] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[MODULEX_FX4_KNOB1] = "FX4 Knob1";
	port_range_hints[MODULEX_FX4_KNOB1].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX4_KNOB1].LowerBound =  0;
	port_range_hints[MODULEX_FX4_KNOB1].UpperBound =  127;
        	
	port_descriptors[MODULEX_FX4_KNOB2] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[MODULEX_FX4_KNOB2] = "FX4 Knob2";
	port_range_hints[MODULEX_FX4_KNOB2].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX4_KNOB2].LowerBound =  0;
	port_range_hints[MODULEX_FX4_KNOB2].UpperBound =  127;
        
	port_descriptors[MODULEX_FX4_COMBOBOX] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[MODULEX_FX4_COMBOBOX] = "FX0 Type";
	port_range_hints[MODULEX_FX4_COMBOBOX].HintDescriptor =
                        LADSPA_HINT_DEFAULT_MINIMUM | LADSPA_HINT_INTEGER |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX4_COMBOBOX].LowerBound =  0;
	port_range_hints[MODULEX_FX4_COMBOBOX].UpperBound =  MULTIFX3KNOB_MAX_INDEX;
        
        	
	port_descriptors[MODULEX_FX5_KNOB0] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[MODULEX_FX5_KNOB0] = "FX5 Knob0";
	port_range_hints[MODULEX_FX5_KNOB0].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX5_KNOB0].LowerBound =  0;
	port_range_hints[MODULEX_FX5_KNOB0].UpperBound =  127;
        
        	
	port_descriptors[MODULEX_FX5_KNOB1] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[MODULEX_FX5_KNOB1] = "FX5 Knob1";
	port_range_hints[MODULEX_FX5_KNOB1].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX5_KNOB1].LowerBound =  0;
	port_range_hints[MODULEX_FX5_KNOB1].UpperBound =  127;
        	
	port_descriptors[MODULEX_FX5_KNOB2] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[MODULEX_FX5_KNOB2] = "FX5 Knob2";
	port_range_hints[MODULEX_FX5_KNOB2].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX5_KNOB2].LowerBound =  0;
	port_range_hints[MODULEX_FX5_KNOB2].UpperBound =  127;
        
	port_descriptors[MODULEX_FX5_COMBOBOX] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[MODULEX_FX5_COMBOBOX] = "FX5 Type";
	port_range_hints[MODULEX_FX5_COMBOBOX].HintDescriptor =
                        LADSPA_HINT_DEFAULT_MINIMUM | LADSPA_HINT_INTEGER |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX5_COMBOBOX].LowerBound =  0;
	port_range_hints[MODULEX_FX5_COMBOBOX].UpperBound =  MULTIFX3KNOB_MAX_INDEX;
        
        
        
        port_descriptors[MODULEX_FX6_KNOB0] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[MODULEX_FX6_KNOB0] = "FX6 Knob0";
	port_range_hints[MODULEX_FX6_KNOB0].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX6_KNOB0].LowerBound =  0;
	port_range_hints[MODULEX_FX6_KNOB0].UpperBound =  127;
        
        	
	port_descriptors[MODULEX_FX6_KNOB1] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[MODULEX_FX6_KNOB1] = "FX6 Knob1";
	port_range_hints[MODULEX_FX6_KNOB1].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX6_KNOB1].LowerBound =  0;
	port_range_hints[MODULEX_FX6_KNOB1].UpperBound =  127;
        	
	port_descriptors[MODULEX_FX6_KNOB2] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[MODULEX_FX6_KNOB2] = "FX6 Knob2";
	port_range_hints[MODULEX_FX6_KNOB2].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX6_KNOB2].LowerBound =  0;
	port_range_hints[MODULEX_FX6_KNOB2].UpperBound =  127;
        
	port_descriptors[MODULEX_FX6_COMBOBOX] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[MODULEX_FX6_COMBOBOX] = "FX6 Type";
	port_range_hints[MODULEX_FX6_COMBOBOX].HintDescriptor =
                        LADSPA_HINT_DEFAULT_MINIMUM | LADSPA_HINT_INTEGER |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX6_COMBOBOX].LowerBound =  0;
	port_range_hints[MODULEX_FX6_COMBOBOX].UpperBound =  MULTIFX3KNOB_MAX_INDEX;
        
        	
	port_descriptors[MODULEX_FX7_KNOB0] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[MODULEX_FX7_KNOB0] = "FX7 Knob0";
	port_range_hints[MODULEX_FX7_KNOB0].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX7_KNOB0].LowerBound =  0;
	port_range_hints[MODULEX_FX7_KNOB0].UpperBound =  127;
        
        	
	port_descriptors[MODULEX_FX7_KNOB1] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[MODULEX_FX7_KNOB1] = "FX7 Knob1";
	port_range_hints[MODULEX_FX7_KNOB1].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX7_KNOB1].LowerBound =  0;
	port_range_hints[MODULEX_FX7_KNOB1].UpperBound =  127;
        	
	port_descriptors[MODULEX_FX7_KNOB2] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[MODULEX_FX7_KNOB2] = "FX7 Knob2";
	port_range_hints[MODULEX_FX7_KNOB2].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX7_KNOB2].LowerBound =  0;
	port_range_hints[MODULEX_FX7_KNOB2].UpperBound =  127;
        
	port_descriptors[MODULEX_FX7_COMBOBOX] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[MODULEX_FX7_COMBOBOX] = "FX7 Type";
	port_range_hints[MODULEX_FX7_COMBOBOX].HintDescriptor =
                        LADSPA_HINT_DEFAULT_MINIMUM | LADSPA_HINT_INTEGER |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX7_COMBOBOX].LowerBound =  0;
	port_range_hints[MODULEX_FX7_COMBOBOX].UpperBound =  MULTIFX3KNOB_MAX_INDEX;
        
        /* Parameters for delay time */
	port_descriptors[MODULEX_DELAY_TIME] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[MODULEX_DELAY_TIME] = "Delay Time";
	port_range_hints[MODULEX_DELAY_TIME].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_DELAY_TIME].LowerBound =  10;
	port_range_hints[MODULEX_DELAY_TIME].UpperBound =  100;
        
        /* Parameters for feedback */
	port_descriptors[MODULEX_FEEDBACK] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[MODULEX_FEEDBACK] = "Feedback";
	port_range_hints[MODULEX_FEEDBACK].HintDescriptor =
			LADSPA_HINT_DEFAULT_HIGH |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FEEDBACK].LowerBound =  -15;
	port_range_hints[MODULEX_FEEDBACK].UpperBound =  0;

        /* Parameters for dry */
	port_descriptors[MODULEX_DRY] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[MODULEX_DRY] = "Dry";
	port_range_hints[MODULEX_DRY].HintDescriptor =
			LADSPA_HINT_DEFAULT_MAXIMUM |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_DRY].LowerBound =  -30;
	port_range_hints[MODULEX_DRY].UpperBound =  0;
        
        /* Parameters for wet */
	port_descriptors[MODULEX_WET] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[MODULEX_WET] = "Wet";
	port_range_hints[MODULEX_WET].HintDescriptor =
			LADSPA_HINT_DEFAULT_MINIMUM |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_WET].LowerBound =  -30;
	port_range_hints[MODULEX_WET].UpperBound =  0;
        
        /* Parameters for duck */
	port_descriptors[MODULEX_DUCK] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[MODULEX_DUCK] = "Duck";
	port_range_hints[MODULEX_DUCK].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_DUCK].LowerBound =  -40;
	port_range_hints[MODULEX_DUCK].UpperBound =  0;
        
        /* Parameters for cutoff */
	port_descriptors[MODULEX_CUTOFF] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[MODULEX_CUTOFF] = "Cutoff";
	port_range_hints[MODULEX_CUTOFF].HintDescriptor =
			LADSPA_HINT_DEFAULT_HIGH |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_CUTOFF].LowerBound =  40;
	port_range_hints[MODULEX_CUTOFF].UpperBound =  118;
        
        
        /* Parameters for stereo */
	port_descriptors[MODULEX_STEREO] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[MODULEX_STEREO] = "Stereo";
	port_range_hints[MODULEX_STEREO].HintDescriptor =
			LADSPA_HINT_DEFAULT_MAXIMUM |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_STEREO].LowerBound =  0;
	port_range_hints[MODULEX_STEREO].UpperBound =  100;
        
        
	LMSLDescriptor->activate = v_modulex_activate;
	LMSLDescriptor->cleanup = v_modulex_cleanup;
	LMSLDescriptor->connect_port = v_modulex_connect_port;
	LMSLDescriptor->deactivate = NULL;
	LMSLDescriptor->instantiate = g_modulex_instantiate;
	LMSLDescriptor->run = v_modulex_run_wrapper;
	LMSLDescriptor->run_adding = NULL;
	LMSLDescriptor->set_run_adding_gain = NULL;
    }

    LMSDDescriptor = (DSSI_Descriptor *) malloc(sizeof(DSSI_Descriptor));
    if (LMSDDescriptor) {
	LMSDDescriptor->DSSI_API_Version = 1;
	LMSDDescriptor->LADSPA_Plugin = LMSLDescriptor;
	LMSDDescriptor->configure = NULL;  //TODO:  I think this is where the host can set plugin state, etc...
	LMSDDescriptor->get_program = NULL;  //TODO:  This is where program change is read, plugin state retrieved, etc...
	LMSDDescriptor->get_midi_controller_for_port = i_modulex_get_controller;
	LMSDDescriptor->select_program = NULL;  //TODO:  This is how the host can select programs, not sure how it differs from a MIDI program change
	LMSDDescriptor->run_synth = v_modulex_run;
	LMSDDescriptor->run_synth_adding = NULL;
	LMSDDescriptor->run_multiple_synths = NULL;
	LMSDDescriptor->run_multiple_synths_adding = NULL;
    }
}

#ifdef __GNUC__
__attribute__((destructor)) void v_modulex_destructor()
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
