/* -*- c-basic-offset: 4 -*-  vi:set ts=8 sts=4 sw=4: */
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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <stdint.h>

#include <math.h>
#include <stdio.h>

#include "../../include/pydaw3/pydaw_plugin.h"

#include "libmodsynth.h"
#include "../../libmodsynth/lib/amp.h"
#include "../../libmodsynth/modules/filter/svf.h"

#include "../../libmodsynth/modules/delay/reverb.h"

#include "synth.h"
#include "meta.h"


static void v_modulex_run(PYFX_Handle instance, int sample_count,
		  snd_seq_event_t * events, int EventCount);

PYFX_Descriptor *modulex_PYFX_descriptor(int index);
PYINST_Descriptor *modulex_PYINST_descriptor(int index);

static void v_modulex_cleanup(PYFX_Handle instance)
{
    free(instance);
}

static void v_modulex_connect_port(PYFX_Handle instance, int port, PYFX_Data * data)
{
    t_modulex *plugin;

    plugin = (t_modulex *) instance;
    
    switch (port) 
    {
        case MODULEX_INPUT0: plugin->input0 = data; break;
        case MODULEX_INPUT1: plugin->input1 = data; break;
        case MODULEX_OUTPUT0: plugin->output0 = data; break;
        case MODULEX_OUTPUT1: plugin->output1 = data; break;   

        case MODULEX_FX0_KNOB0: plugin->fx_knob0[0] = data; break;
        case MODULEX_FX0_KNOB1:	plugin->fx_knob1[0] = data; break;    
        case MODULEX_FX0_KNOB2: plugin->fx_knob2[0] = data; break;    
        case MODULEX_FX0_COMBOBOX: plugin->fx_combobox[0] = data; break;    

        case MODULEX_FX1_KNOB0: plugin->fx_knob0[1] = data; break;
        case MODULEX_FX1_KNOB1:	plugin->fx_knob1[1] = data; break;    
        case MODULEX_FX1_KNOB2: plugin->fx_knob2[1] = data; break;    
        case MODULEX_FX1_COMBOBOX: plugin->fx_combobox[1] = data; break;    

        case MODULEX_FX2_KNOB0: plugin->fx_knob0[2] = data; break;
        case MODULEX_FX2_KNOB1:	plugin->fx_knob1[2] = data; break;    
        case MODULEX_FX2_KNOB2: plugin->fx_knob2[2] = data; break;    
        case MODULEX_FX2_COMBOBOX: plugin->fx_combobox[2] = data; break;    

        case MODULEX_FX3_KNOB0: plugin->fx_knob0[3] = data; break;
        case MODULEX_FX3_KNOB1:	plugin->fx_knob1[3] = data; break;    
        case MODULEX_FX3_KNOB2: plugin->fx_knob2[3] = data; break;    
        case MODULEX_FX3_COMBOBOX: plugin->fx_combobox[3] = data; break;
        
        case MODULEX_FX4_KNOB0: plugin->fx_knob0[4] = data; break;
        case MODULEX_FX4_KNOB1:	plugin->fx_knob1[4] = data; break;    
        case MODULEX_FX4_KNOB2: plugin->fx_knob2[4] = data; break;    
        case MODULEX_FX4_COMBOBOX: plugin->fx_combobox[4] = data; break;    

        case MODULEX_FX5_KNOB0: plugin->fx_knob0[5] = data; break;
        case MODULEX_FX5_KNOB1:	plugin->fx_knob1[5] = data; break;    
        case MODULEX_FX5_KNOB2: plugin->fx_knob2[5] = data; break;    
        case MODULEX_FX5_COMBOBOX: plugin->fx_combobox[5] = data; break;    

        case MODULEX_FX6_KNOB0: plugin->fx_knob0[6] = data; break;
        case MODULEX_FX6_KNOB1:	plugin->fx_knob1[6] = data; break;    
        case MODULEX_FX6_KNOB2: plugin->fx_knob2[6] = data; break;    
        case MODULEX_FX6_COMBOBOX: plugin->fx_combobox[6] = data; break;    

        case MODULEX_FX7_KNOB0: plugin->fx_knob0[7] = data; break;
        case MODULEX_FX7_KNOB1:	plugin->fx_knob1[7] = data; break;    
        case MODULEX_FX7_KNOB2: plugin->fx_knob2[7] = data; break;    
        case MODULEX_FX7_COMBOBOX: plugin->fx_combobox[7] = data; break;     
        
        case MODULEX_DELAY_TIME: plugin->delay_time = data; break;
        case MODULEX_FEEDBACK: plugin->feedback = data; break;
        case MODULEX_DRY: plugin->dry = data;  break;
        case MODULEX_WET: plugin->wet = data; break;
        case MODULEX_DUCK: plugin->duck = data; break;
        case MODULEX_CUTOFF: plugin->cutoff = data; break;
        case MODULEX_STEREO: plugin->stereo = data; break;
        
        case MODULEX_VOL_SLIDER: plugin->vol_slider = data; break;
        
        case MODULEX_REVERB_TIME: plugin->reverb_time = data; break;
        case MODULEX_REVERB_WET: plugin->reverb_wet = data; break;
        case MODULEX_REVERB_COLOR: plugin->reverb_color = data; break;
    }
}

static PYFX_Handle g_modulex_instantiate(const PYFX_Descriptor * descriptor,
				   int s_rate)
{
    t_modulex *plugin_data = (t_modulex *) malloc(sizeof(t_modulex));
    
    plugin_data->fs = s_rate;        
    return (PYFX_Handle) plugin_data;
}

static void v_modulex_activate(PYFX_Handle instance)
{
    t_modulex *plugin_data = (t_modulex *) instance;
        
    plugin_data->mono_modules = v_modulex_mono_init((plugin_data->fs));  //initialize all monophonic modules
    
    plugin_data->i_slow_index = MODULEX_SLOW_INDEX_ITERATIONS;
}

static void v_modulex_run_wrapper(PYFX_Handle instance,
			 int sample_count)
{
    v_modulex_run(instance, sample_count, NULL, 0);
}

static void v_modulex_run(PYFX_Handle instance, int sample_count,
		  snd_seq_event_t *events, int event_count)
{
    t_modulex *plugin_data = (t_modulex *) instance;
    
    int f_i = 0;
    
    if(plugin_data->i_slow_index >= MODULEX_SLOW_INDEX_ITERATIONS)
    {
        plugin_data->i_slow_index = 0;
        plugin_data->is_on = 0;
        
        while(f_i < 8)
        {
            plugin_data->mono_modules->fx_func_ptr[f_i] = g_mf3_get_function_pointer((int)(*(plugin_data->fx_combobox[f_i])));
            
            if(plugin_data->mono_modules->fx_func_ptr[f_i] != v_mf3_run_off)
            {
                plugin_data->is_on = 1;
            }
            
            f_i++;
        }
    }
    else
    {
        plugin_data->i_slow_index = (plugin_data->i_slow_index) + 1;
    }

    f_i = 0;
    
    while(f_i < 8)
    {
        if(plugin_data->mono_modules->fx_func_ptr[f_i] != v_mf3_run_off)
        {
            v_smr_iir_run_fast(plugin_data->mono_modules->smoothers[f_i][0], *plugin_data->fx_knob0[f_i]);
            v_smr_iir_run_fast(plugin_data->mono_modules->smoothers[f_i][1], *plugin_data->fx_knob1[f_i]);
            v_smr_iir_run_fast(plugin_data->mono_modules->smoothers[f_i][2], *plugin_data->fx_knob2[f_i]);
            
            v_mf3_set(plugin_data->mono_modules->multieffect[f_i], 
                    plugin_data->mono_modules->smoothers[f_i][0]->output, 
                    plugin_data->mono_modules->smoothers[f_i][1]->output, 
                    plugin_data->mono_modules->smoothers[f_i][2]->output
                    );
        }
        f_i++;
    }
    
    if(plugin_data->is_on)
    {
        plugin_data->i_mono_out = 0;

        while((plugin_data->i_mono_out) < sample_count)
        {
            plugin_data->mono_modules->current_sample0 = plugin_data->input0[(plugin_data->i_mono_out)];
            plugin_data->mono_modules->current_sample1 = plugin_data->input1[(plugin_data->i_mono_out)];

            f_i = 0;

            while(f_i < 8)
            {
                plugin_data->mono_modules->fx_func_ptr[f_i](plugin_data->mono_modules->multieffect[f_i], (plugin_data->mono_modules->current_sample0), (plugin_data->mono_modules->current_sample1)); 

                plugin_data->mono_modules->current_sample0 = plugin_data->mono_modules->multieffect[f_i]->output0;
                plugin_data->mono_modules->current_sample1 = plugin_data->mono_modules->multieffect[f_i]->output1;
                f_i++;
            }

            plugin_data->output0[(plugin_data->i_mono_out)] = (plugin_data->mono_modules->current_sample0);
            plugin_data->output1[(plugin_data->i_mono_out)] = (plugin_data->mono_modules->current_sample1);

            plugin_data->i_mono_out = (plugin_data->i_mono_out) + 1;
        }
    }
    
    if((*(plugin_data->wet)) > -29.0f)
    {
        plugin_data->i_mono_out = 0;
        
        while((plugin_data->i_mono_out) < sample_count)
        {
            v_smr_iir_run(plugin_data->mono_modules->time_smoother, (*(plugin_data->delay_time) * .01));
            
            v_ldl_set_delay(plugin_data->mono_modules->delay, (plugin_data->mono_modules->time_smoother->output), *(plugin_data->feedback), 
                    *(plugin_data->wet), *(plugin_data->dry), (*(plugin_data->stereo) * .01), (*plugin_data->duck), (*plugin_data->cutoff));

            v_ldl_run_delay(plugin_data->mono_modules->delay, 
                    (plugin_data->output0[(plugin_data->i_mono_out)]), (plugin_data->output0[(plugin_data->i_mono_out)]));        

            plugin_data->output0[(plugin_data->i_mono_out)] = (plugin_data->mono_modules->delay->output0);
            plugin_data->output1[(plugin_data->i_mono_out)] = (plugin_data->mono_modules->delay->output1);            

            plugin_data->i_mono_out = (plugin_data->i_mono_out) + 1;
        }
    }
    
    v_sml_run(plugin_data->mono_modules->reverb_smoother, (*plugin_data->reverb_wet));
    
    if((plugin_data->mono_modules->reverb_smoother->last_value) > 0.005f)
    {                
        v_rvb_reverb_set(plugin_data->mono_modules->reverb, (*plugin_data->reverb_time) * 0.01f, 
                f_db_to_linear_fast(((plugin_data->mono_modules->reverb_smoother->last_value) * 0.4f) - 40.0f, 
                plugin_data->mono_modules->amp_ptr),
                (*plugin_data->reverb_color) * 0.01f);
        
        int f_i = 0;
        while(f_i < sample_count)
        {
            v_rvb_reverb_run(plugin_data->mono_modules->reverb, 
                plugin_data->output0[f_i],
                plugin_data->output1[f_i]);
            
            plugin_data->output0[f_i] += plugin_data->mono_modules->reverb->output;
            plugin_data->output1[f_i] += plugin_data->mono_modules->reverb->output;
            
            f_i++;
        }
    }
    
    if((plugin_data->mono_modules->volume_smoother->last_value) != 0.0f || (*plugin_data->vol_slider != 0.0f))
    {
        f_i = 0;

        while(f_i < sample_count)
        {
            v_sml_run(plugin_data->mono_modules->volume_smoother, (*plugin_data->vol_slider));

            plugin_data->mono_modules->vol_linear = 
                f_db_to_linear_fast((plugin_data->mono_modules->volume_smoother->last_value), 
                plugin_data->mono_modules->amp_ptr);

            plugin_data->output0[f_i] *= (plugin_data->mono_modules->vol_linear);
            plugin_data->output1[f_i] *= (plugin_data->mono_modules->vol_linear);
            f_i++;
        }
    }
    
}

PYFX_Descriptor *modulex_PYFX_descriptor(int index)
{
    PYFX_Descriptor *LMSLDescriptor = NULL;
    
    char **port_names;
    PYFX_PortDescriptor *port_descriptors;
    PYFX_PortRangeHint *port_range_hints;    
    int * automatable;
    int * value_tranform_hints;

    LMSLDescriptor =
	(PYFX_Descriptor *) malloc(sizeof(PYFX_Descriptor));
    if (LMSLDescriptor) 
    {
        LMSLDescriptor->UniqueID = MODULEX_PLUGIN_UUID;
	LMSLDescriptor->Label = MODULEX_PLUGIN_NAME;
	LMSLDescriptor->Properties = PYFX_PROPERTY_HARD_RT_CAPABLE;
	LMSLDescriptor->Name = MODULEX_PLUGIN_LONG_NAME;
	LMSLDescriptor->Maker = MODULEX_PLUGIN_DEV;
	LMSLDescriptor->Copyright = "GNU GPL v3";
	LMSLDescriptor->PortCount = MODULEX_COUNT;

	port_descriptors = (PYFX_PortDescriptor *)
				calloc(LMSLDescriptor->PortCount, sizeof
						(PYFX_PortDescriptor));
	LMSLDescriptor->PortDescriptors =
	    (const PYFX_PortDescriptor *) port_descriptors;

	port_range_hints = (PYFX_PortRangeHint *)
				calloc(LMSLDescriptor->PortCount, sizeof
						(PYFX_PortRangeHint));
	LMSLDescriptor->PortRangeHints =
	    (const PYFX_PortRangeHint *) port_range_hints;

	port_names = (char **) calloc(LMSLDescriptor->PortCount, sizeof(char *));
	LMSLDescriptor->PortNames = (const char **) port_names;

        automatable = (int*)calloc(LMSLDescriptor->PortCount, sizeof(int));
        LMSLDescriptor->Automatable = automatable;
        
        value_tranform_hints = (int*)calloc(LMSLDescriptor->PortCount, sizeof(int));
        LMSLDescriptor->ValueTransformHint = value_tranform_hints;
        
        /* Parameters for input */
	port_descriptors[MODULEX_INPUT0] = PYFX_PORT_INPUT | PYFX_PORT_AUDIO;
	port_names[MODULEX_INPUT0] = "Input 0";
	port_range_hints[MODULEX_INPUT0].HintDescriptor = 0;

        port_descriptors[MODULEX_INPUT1] = PYFX_PORT_INPUT | PYFX_PORT_AUDIO;
	port_names[MODULEX_INPUT1] = "Input 1";
	port_range_hints[MODULEX_INPUT1].HintDescriptor = 0;
        
	/* Parameters for output */
	port_descriptors[MODULEX_OUTPUT0] = PYFX_PORT_OUTPUT | PYFX_PORT_AUDIO;
	port_names[MODULEX_OUTPUT0] = "Output 0";
	port_range_hints[MODULEX_OUTPUT0].HintDescriptor = 0;

        port_descriptors[MODULEX_OUTPUT1] = PYFX_PORT_OUTPUT | PYFX_PORT_AUDIO;
	port_names[MODULEX_OUTPUT1] = "Output 1";
	port_range_hints[MODULEX_OUTPUT1].HintDescriptor = 0;
        
        
	port_descriptors[MODULEX_FX0_KNOB0] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[MODULEX_FX0_KNOB0] = "FX0 Knob0";
	port_range_hints[MODULEX_FX0_KNOB0].HintDescriptor =
			PYFX_HINT_DEFAULT_MIDDLE |
			PYFX_HINT_BOUNDED_BELOW | PYFX_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX0_KNOB0].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX0_KNOB0].UpperBound =  127.0f;
        automatable[MODULEX_FX0_KNOB0] = 1;
        	
	port_descriptors[MODULEX_FX0_KNOB1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[MODULEX_FX0_KNOB1] = "FX0 Knob1";
	port_range_hints[MODULEX_FX0_KNOB1].HintDescriptor =
			PYFX_HINT_DEFAULT_MIDDLE |
			PYFX_HINT_BOUNDED_BELOW | PYFX_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX0_KNOB1].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX0_KNOB1].UpperBound =  127.0f;
        automatable[MODULEX_FX0_KNOB1] = 1;
        	
	port_descriptors[MODULEX_FX0_KNOB2] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[MODULEX_FX0_KNOB2] = "FX0 Knob2";
	port_range_hints[MODULEX_FX0_KNOB2].HintDescriptor =
			PYFX_HINT_DEFAULT_MIDDLE |
			PYFX_HINT_BOUNDED_BELOW | PYFX_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX0_KNOB2].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX0_KNOB2].UpperBound =  127.0f;
        automatable[MODULEX_FX0_KNOB2] = 1;
        
	port_descriptors[MODULEX_FX0_COMBOBOX] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[MODULEX_FX0_COMBOBOX] = "FX0 Type";
	port_range_hints[MODULEX_FX0_COMBOBOX].HintDescriptor =
                        PYFX_HINT_DEFAULT_MINIMUM | PYFX_HINT_INTEGER |
			PYFX_HINT_BOUNDED_BELOW | PYFX_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX0_COMBOBOX].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX0_COMBOBOX].UpperBound =  MULTIFX3KNOB_MAX_INDEX;
        
        	
	port_descriptors[MODULEX_FX1_KNOB0] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[MODULEX_FX1_KNOB0] = "FX1 Knob0";
	port_range_hints[MODULEX_FX1_KNOB0].HintDescriptor =
			PYFX_HINT_DEFAULT_MIDDLE |
			PYFX_HINT_BOUNDED_BELOW | PYFX_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX1_KNOB0].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX1_KNOB0].UpperBound =  127.0f;        
        automatable[MODULEX_FX1_KNOB0] = 1;
        	
	port_descriptors[MODULEX_FX1_KNOB1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[MODULEX_FX1_KNOB1] = "FX1 Knob1";
	port_range_hints[MODULEX_FX1_KNOB1].HintDescriptor =
			PYFX_HINT_DEFAULT_MIDDLE |
			PYFX_HINT_BOUNDED_BELOW | PYFX_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX1_KNOB1].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX1_KNOB1].UpperBound =  127.0f;
        automatable[MODULEX_FX1_KNOB1] = 1;
        	
	port_descriptors[MODULEX_FX1_KNOB2] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[MODULEX_FX1_KNOB2] = "FX1 Knob2";
	port_range_hints[MODULEX_FX1_KNOB2].HintDescriptor =
			PYFX_HINT_DEFAULT_MIDDLE |
			PYFX_HINT_BOUNDED_BELOW | PYFX_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX1_KNOB2].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX1_KNOB2].UpperBound =  127.0f;
        automatable[MODULEX_FX1_KNOB2] = 1;
        
	port_descriptors[MODULEX_FX1_COMBOBOX] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[MODULEX_FX1_COMBOBOX] = "FX1 Type";
	port_range_hints[MODULEX_FX1_COMBOBOX].HintDescriptor =
                        PYFX_HINT_DEFAULT_MINIMUM | PYFX_HINT_INTEGER |
			PYFX_HINT_BOUNDED_BELOW | PYFX_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX1_COMBOBOX].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX1_COMBOBOX].UpperBound =  MULTIFX3KNOB_MAX_INDEX;               
        
        port_descriptors[MODULEX_FX2_KNOB0] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[MODULEX_FX2_KNOB0] = "FX2 Knob0";
	port_range_hints[MODULEX_FX2_KNOB0].HintDescriptor =
			PYFX_HINT_DEFAULT_MIDDLE |
			PYFX_HINT_BOUNDED_BELOW | PYFX_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX2_KNOB0].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX2_KNOB0].UpperBound =  127.0f;
        automatable[MODULEX_FX2_KNOB0] = 1;
                	
	port_descriptors[MODULEX_FX2_KNOB1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[MODULEX_FX2_KNOB1] = "FX2 Knob1";
	port_range_hints[MODULEX_FX2_KNOB1].HintDescriptor =
			PYFX_HINT_DEFAULT_MIDDLE |
			PYFX_HINT_BOUNDED_BELOW | PYFX_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX2_KNOB1].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX2_KNOB1].UpperBound =  127.0f;
        automatable[MODULEX_FX2_KNOB1] = 1;
        	
	port_descriptors[MODULEX_FX2_KNOB2] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[MODULEX_FX2_KNOB2] = "FX2 Knob2";
	port_range_hints[MODULEX_FX2_KNOB2].HintDescriptor =
			PYFX_HINT_DEFAULT_MIDDLE |
			PYFX_HINT_BOUNDED_BELOW | PYFX_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX2_KNOB2].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX2_KNOB2].UpperBound =  127.0f;
        automatable[MODULEX_FX2_KNOB2] = 1;
        
	port_descriptors[MODULEX_FX2_COMBOBOX] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[MODULEX_FX2_COMBOBOX] = "FX2 Type";
	port_range_hints[MODULEX_FX2_COMBOBOX].HintDescriptor =
                        PYFX_HINT_DEFAULT_MINIMUM | PYFX_HINT_INTEGER |
			PYFX_HINT_BOUNDED_BELOW | PYFX_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX2_COMBOBOX].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX2_COMBOBOX].UpperBound =  MULTIFX3KNOB_MAX_INDEX;        
        	
	port_descriptors[MODULEX_FX3_KNOB0] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[MODULEX_FX3_KNOB0] = "FX3 Knob0";
	port_range_hints[MODULEX_FX3_KNOB0].HintDescriptor =
			PYFX_HINT_DEFAULT_MIDDLE |
			PYFX_HINT_BOUNDED_BELOW | PYFX_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX3_KNOB0].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX3_KNOB0].UpperBound =  127.0f;        
        automatable[MODULEX_FX3_KNOB0] = 1;
        	
	port_descriptors[MODULEX_FX3_KNOB1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[MODULEX_FX3_KNOB1] = "FX3 Knob1";
	port_range_hints[MODULEX_FX3_KNOB1].HintDescriptor =
			PYFX_HINT_DEFAULT_MIDDLE |
			PYFX_HINT_BOUNDED_BELOW | PYFX_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX3_KNOB1].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX3_KNOB1].UpperBound =  127.0f;
        automatable[MODULEX_FX3_KNOB1] = 1;
        	
	port_descriptors[MODULEX_FX3_KNOB2] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[MODULEX_FX3_KNOB2] = "FX3 Knob2";
	port_range_hints[MODULEX_FX3_KNOB2].HintDescriptor =
			PYFX_HINT_DEFAULT_MIDDLE |
			PYFX_HINT_BOUNDED_BELOW | PYFX_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX3_KNOB2].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX3_KNOB2].UpperBound =  127.0f;
        automatable[MODULEX_FX3_KNOB2] = 1;
        
	port_descriptors[MODULEX_FX3_COMBOBOX] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[MODULEX_FX3_COMBOBOX] = "FX3 Type";
	port_range_hints[MODULEX_FX3_COMBOBOX].HintDescriptor =
                        PYFX_HINT_DEFAULT_MINIMUM | PYFX_HINT_INTEGER |
			PYFX_HINT_BOUNDED_BELOW | PYFX_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX3_COMBOBOX].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX3_COMBOBOX].UpperBound =  MULTIFX3KNOB_MAX_INDEX;        
        
	port_descriptors[MODULEX_FX4_KNOB0] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[MODULEX_FX4_KNOB0] = "FX4 Knob0";
	port_range_hints[MODULEX_FX4_KNOB0].HintDescriptor =
			PYFX_HINT_DEFAULT_MIDDLE |
			PYFX_HINT_BOUNDED_BELOW | PYFX_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX4_KNOB0].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX4_KNOB0].UpperBound =  127.0f;
        automatable[MODULEX_FX4_KNOB0] = 1;
                	
	port_descriptors[MODULEX_FX4_KNOB1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[MODULEX_FX4_KNOB1] = "FX4 Knob1";
	port_range_hints[MODULEX_FX4_KNOB1].HintDescriptor =
			PYFX_HINT_DEFAULT_MIDDLE |
			PYFX_HINT_BOUNDED_BELOW | PYFX_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX4_KNOB1].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX4_KNOB1].UpperBound =  127.0f;
        automatable[MODULEX_FX4_KNOB1] = 1;
        	
	port_descriptors[MODULEX_FX4_KNOB2] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[MODULEX_FX4_KNOB2] = "FX4 Knob2";
	port_range_hints[MODULEX_FX4_KNOB2].HintDescriptor =
			PYFX_HINT_DEFAULT_MIDDLE |
			PYFX_HINT_BOUNDED_BELOW | PYFX_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX4_KNOB2].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX4_KNOB2].UpperBound =  127.0f;
        automatable[MODULEX_FX4_KNOB2] = 1;
        
	port_descriptors[MODULEX_FX4_COMBOBOX] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[MODULEX_FX4_COMBOBOX] = "FX0 Type";
	port_range_hints[MODULEX_FX4_COMBOBOX].HintDescriptor =
                        PYFX_HINT_DEFAULT_MINIMUM | PYFX_HINT_INTEGER |
			PYFX_HINT_BOUNDED_BELOW | PYFX_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX4_COMBOBOX].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX4_COMBOBOX].UpperBound =  MULTIFX3KNOB_MAX_INDEX;
                	
	port_descriptors[MODULEX_FX5_KNOB0] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[MODULEX_FX5_KNOB0] = "FX5 Knob0";
	port_range_hints[MODULEX_FX5_KNOB0].HintDescriptor =
			PYFX_HINT_DEFAULT_MIDDLE |
			PYFX_HINT_BOUNDED_BELOW | PYFX_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX5_KNOB0].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX5_KNOB0].UpperBound =  127.0f;
        automatable[MODULEX_FX5_KNOB0] = 1;
        	
	port_descriptors[MODULEX_FX5_KNOB1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[MODULEX_FX5_KNOB1] = "FX5 Knob1";
	port_range_hints[MODULEX_FX5_KNOB1].HintDescriptor =
			PYFX_HINT_DEFAULT_MIDDLE |
			PYFX_HINT_BOUNDED_BELOW | PYFX_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX5_KNOB1].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX5_KNOB1].UpperBound =  127.0f;
        automatable[MODULEX_FX5_KNOB1] = 1;
        	
	port_descriptors[MODULEX_FX5_KNOB2] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[MODULEX_FX5_KNOB2] = "FX5 Knob2";
	port_range_hints[MODULEX_FX5_KNOB2].HintDescriptor =
			PYFX_HINT_DEFAULT_MIDDLE |
			PYFX_HINT_BOUNDED_BELOW | PYFX_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX5_KNOB2].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX5_KNOB2].UpperBound =  127.0f;
        automatable[MODULEX_FX5_KNOB2] = 1;
        
	port_descriptors[MODULEX_FX5_COMBOBOX] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[MODULEX_FX5_COMBOBOX] = "FX5 Type";
	port_range_hints[MODULEX_FX5_COMBOBOX].HintDescriptor =
                        PYFX_HINT_DEFAULT_MINIMUM | PYFX_HINT_INTEGER |
			PYFX_HINT_BOUNDED_BELOW | PYFX_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX5_COMBOBOX].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX5_COMBOBOX].UpperBound =  MULTIFX3KNOB_MAX_INDEX;                
        
        port_descriptors[MODULEX_FX6_KNOB0] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[MODULEX_FX6_KNOB0] = "FX6 Knob0";
	port_range_hints[MODULEX_FX6_KNOB0].HintDescriptor =
			PYFX_HINT_DEFAULT_MIDDLE |
			PYFX_HINT_BOUNDED_BELOW | PYFX_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX6_KNOB0].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX6_KNOB0].UpperBound =  127.0f;
        automatable[MODULEX_FX6_KNOB0] = 1;
        	
	port_descriptors[MODULEX_FX6_KNOB1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[MODULEX_FX6_KNOB1] = "FX6 Knob1";
	port_range_hints[MODULEX_FX6_KNOB1].HintDescriptor =
			PYFX_HINT_DEFAULT_MIDDLE |
			PYFX_HINT_BOUNDED_BELOW | PYFX_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX6_KNOB1].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX6_KNOB1].UpperBound =  127.0f;
        automatable[MODULEX_FX6_KNOB1] = 1;
        	
	port_descriptors[MODULEX_FX6_KNOB2] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[MODULEX_FX6_KNOB2] = "FX6 Knob2";
	port_range_hints[MODULEX_FX6_KNOB2].HintDescriptor =
			PYFX_HINT_DEFAULT_MIDDLE |
			PYFX_HINT_BOUNDED_BELOW | PYFX_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX6_KNOB2].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX6_KNOB2].UpperBound =  127.0f;
        automatable[MODULEX_FX6_KNOB2] = 1;
        
	port_descriptors[MODULEX_FX6_COMBOBOX] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[MODULEX_FX6_COMBOBOX] = "FX6 Type";
	port_range_hints[MODULEX_FX6_COMBOBOX].HintDescriptor =
                        PYFX_HINT_DEFAULT_MINIMUM | PYFX_HINT_INTEGER |
			PYFX_HINT_BOUNDED_BELOW | PYFX_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX6_COMBOBOX].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX6_COMBOBOX].UpperBound =  MULTIFX3KNOB_MAX_INDEX;        
        	
	port_descriptors[MODULEX_FX7_KNOB0] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[MODULEX_FX7_KNOB0] = "FX7 Knob0";
	port_range_hints[MODULEX_FX7_KNOB0].HintDescriptor =
			PYFX_HINT_DEFAULT_MIDDLE |
			PYFX_HINT_BOUNDED_BELOW | PYFX_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX7_KNOB0].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX7_KNOB0].UpperBound =  127.0f;
        automatable[MODULEX_FX7_KNOB0] = 1;
        
	port_descriptors[MODULEX_FX7_KNOB1] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[MODULEX_FX7_KNOB1] = "FX7 Knob1";
	port_range_hints[MODULEX_FX7_KNOB1].HintDescriptor =
			PYFX_HINT_DEFAULT_MIDDLE |
			PYFX_HINT_BOUNDED_BELOW | PYFX_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX7_KNOB1].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX7_KNOB1].UpperBound =  127.0f;
        automatable[MODULEX_FX7_KNOB1] = 1;
        	
	port_descriptors[MODULEX_FX7_KNOB2] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[MODULEX_FX7_KNOB2] = "FX7 Knob2";
	port_range_hints[MODULEX_FX7_KNOB2].HintDescriptor =
			PYFX_HINT_DEFAULT_MIDDLE |
			PYFX_HINT_BOUNDED_BELOW | PYFX_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX7_KNOB2].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX7_KNOB2].UpperBound =  127.0f;
        automatable[MODULEX_FX7_KNOB2] = 1;
        
	port_descriptors[MODULEX_FX7_COMBOBOX] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[MODULEX_FX7_COMBOBOX] = "FX7 Type";
	port_range_hints[MODULEX_FX7_COMBOBOX].HintDescriptor =
                        PYFX_HINT_DEFAULT_MINIMUM | PYFX_HINT_INTEGER |
			PYFX_HINT_BOUNDED_BELOW | PYFX_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FX7_COMBOBOX].LowerBound =  0.0f;
	port_range_hints[MODULEX_FX7_COMBOBOX].UpperBound =  MULTIFX3KNOB_MAX_INDEX;
        
        /* Parameters for delay time */
	port_descriptors[MODULEX_DELAY_TIME] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[MODULEX_DELAY_TIME] = "Delay Time";
	port_range_hints[MODULEX_DELAY_TIME].HintDescriptor =
			PYFX_HINT_DEFAULT_MIDDLE |
			PYFX_HINT_BOUNDED_BELOW | PYFX_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_DELAY_TIME].LowerBound =  10.0f;
	port_range_hints[MODULEX_DELAY_TIME].UpperBound =  100.0f;
        
	port_descriptors[MODULEX_FEEDBACK] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[MODULEX_FEEDBACK] = "Delay Feedback";
	port_range_hints[MODULEX_FEEDBACK].HintDescriptor =
			PYFX_HINT_DEFAULT_HIGH |
			PYFX_HINT_BOUNDED_BELOW | PYFX_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_FEEDBACK].LowerBound =  -15.0f;
	port_range_hints[MODULEX_FEEDBACK].UpperBound =  0.0f;
        automatable[MODULEX_FEEDBACK] = 1;

	port_descriptors[MODULEX_DRY] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[MODULEX_DRY] = "Delay Dry";
	port_range_hints[MODULEX_DRY].HintDescriptor =
			PYFX_HINT_DEFAULT_MAXIMUM |
			PYFX_HINT_BOUNDED_BELOW | PYFX_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_DRY].LowerBound =  -30.0f;
	port_range_hints[MODULEX_DRY].UpperBound =  0.0f;
        automatable[MODULEX_DRY] = 1;
        
	port_descriptors[MODULEX_WET] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[MODULEX_WET] = "Delay Wet";
	port_range_hints[MODULEX_WET].HintDescriptor =
			PYFX_HINT_DEFAULT_MINIMUM |
			PYFX_HINT_BOUNDED_BELOW | PYFX_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_WET].LowerBound =  -30.0f;
	port_range_hints[MODULEX_WET].UpperBound =  0.0f;
        automatable[MODULEX_WET] = 1;
        
	port_descriptors[MODULEX_DUCK] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[MODULEX_DUCK] = "Delay Duck";
	port_range_hints[MODULEX_DUCK].HintDescriptor =
			PYFX_HINT_DEFAULT_MIDDLE |
			PYFX_HINT_BOUNDED_BELOW | PYFX_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_DUCK].LowerBound =  -40.0f;
	port_range_hints[MODULEX_DUCK].UpperBound =  0.0f;
        automatable[MODULEX_DUCK] = 1;
        
	port_descriptors[MODULEX_CUTOFF] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[MODULEX_CUTOFF] = "Delay LP Cutoff";
	port_range_hints[MODULEX_CUTOFF].HintDescriptor =
			PYFX_HINT_DEFAULT_HIGH |
			PYFX_HINT_BOUNDED_BELOW | PYFX_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_CUTOFF].LowerBound =  40.0f;
	port_range_hints[MODULEX_CUTOFF].UpperBound =  118.0f;
        automatable[MODULEX_CUTOFF] = 1;
                
        /* Parameters for stereo */
	port_descriptors[MODULEX_STEREO] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[MODULEX_STEREO] = "Delay Stereo";
	port_range_hints[MODULEX_STEREO].HintDescriptor =
			PYFX_HINT_DEFAULT_MAXIMUM |
			PYFX_HINT_BOUNDED_BELOW | PYFX_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_STEREO].LowerBound =  0.0f;
	port_range_hints[MODULEX_STEREO].UpperBound =  100.0f;        
        
        
        port_descriptors[MODULEX_VOL_SLIDER] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[MODULEX_VOL_SLIDER] = "Volume Slider";
	port_range_hints[MODULEX_VOL_SLIDER].HintDescriptor =
			PYFX_HINT_DEFAULT_MAXIMUM |
			PYFX_HINT_BOUNDED_BELOW | PYFX_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_VOL_SLIDER].LowerBound =  -50.0f;
	port_range_hints[MODULEX_VOL_SLIDER].UpperBound =  0.0f;
        automatable[MODULEX_VOL_SLIDER] = 1;
        
        port_descriptors[MODULEX_REVERB_TIME] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[MODULEX_REVERB_TIME] = "Reverb Time";
	port_range_hints[MODULEX_REVERB_TIME].HintDescriptor =
			PYFX_HINT_DEFAULT_MIDDLE |
			PYFX_HINT_BOUNDED_BELOW | PYFX_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_REVERB_TIME].LowerBound =  0.0f;
	port_range_hints[MODULEX_REVERB_TIME].UpperBound =  100.0f;
        
        port_descriptors[MODULEX_REVERB_WET] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[MODULEX_REVERB_WET] = "Reverb Wet";
	port_range_hints[MODULEX_REVERB_WET].HintDescriptor =
			PYFX_HINT_DEFAULT_MINIMUM |
			PYFX_HINT_BOUNDED_BELOW | PYFX_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_REVERB_WET].LowerBound =  0.0f;
	port_range_hints[MODULEX_REVERB_WET].UpperBound =  100.0f;
        automatable[MODULEX_REVERB_WET] = 1;
        
        port_descriptors[MODULEX_REVERB_COLOR] = PYFX_PORT_INPUT | PYFX_PORT_CONTROL;
	port_names[MODULEX_REVERB_COLOR] = "Reverb Color";
	port_range_hints[MODULEX_REVERB_COLOR].HintDescriptor =
			PYFX_HINT_DEFAULT_MIDDLE |
			PYFX_HINT_BOUNDED_BELOW | PYFX_HINT_BOUNDED_ABOVE;
	port_range_hints[MODULEX_REVERB_COLOR].LowerBound =  0.0f;
	port_range_hints[MODULEX_REVERB_COLOR].UpperBound =  100.0f;
        automatable[MODULEX_REVERB_COLOR] = 1;
                
	LMSLDescriptor->activate = v_modulex_activate;
	LMSLDescriptor->cleanup = v_modulex_cleanup;
	LMSLDescriptor->connect_port = v_modulex_connect_port;
	LMSLDescriptor->deactivate = NULL;
	LMSLDescriptor->instantiate = g_modulex_instantiate;
	LMSLDescriptor->run = v_modulex_run_wrapper;
	LMSLDescriptor->run_adding = NULL;
	LMSLDescriptor->set_run_adding_gain = NULL;
    }
        
    return LMSLDescriptor;    
}


PYINST_Descriptor *modulex_PYINST_descriptor(int index)
{
    PYINST_Descriptor *LMSDDescriptor = NULL;
    
    LMSDDescriptor = (PYINST_Descriptor *) malloc(sizeof(PYINST_Descriptor));
    if (LMSDDescriptor) 
    {
	LMSDDescriptor->PYINST_API_Version = 1;
	LMSDDescriptor->PYFX_Plugin = modulex_PYFX_descriptor(0);
	LMSDDescriptor->configure = NULL;
	LMSDDescriptor->get_program = NULL;
	LMSDDescriptor->get_midi_controller_for_port = NULL;
	LMSDDescriptor->select_program = NULL;
	LMSDDescriptor->run_synth = v_modulex_run;
	LMSDDescriptor->run_synth_adding = NULL;
	LMSDDescriptor->run_multiple_synths = NULL;
	LMSDDescriptor->run_multiple_synths_adding = NULL;
    }
        
    return LMSDDescriptor;    
}


/*
void v_modulex_destructor()
{
    if (LMSLDescriptor) {
	free((PYFX_PortDescriptor *) LMSLDescriptor->PortDescriptors);
	free((char **) LMSLDescriptor->PortNames);
	free((PYFX_PortRangeHint *) LMSLDescriptor->PortRangeHints);
	free(LMSLDescriptor);
    }
    if (LMSDDescriptor) {
	free(LMSDDescriptor);
    }
}
*/