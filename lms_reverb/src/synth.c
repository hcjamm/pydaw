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

static void runLMS(LADSPA_Handle instance, unsigned long sample_count,
		  snd_seq_event_t * events, unsigned long EventCount);


const LADSPA_Descriptor *ladspa_descriptor(unsigned long index)
{
    switch (index) {
    case 0:
	return LMSLDescriptor;
    default:
	return NULL;
    }
}

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
    case LMS_PREDELAY:
	plugin->predelay = data;              
	break;
    case LMS_TIME:
	plugin->time = data;              
	break;
    case LMS_HIGHPASS:
	plugin->highpass = data;              
	break;
    case LMS_LOWPASS:
	plugin->lowpass = data;              
	break;
    case LMS_DRYWET:
	plugin->drywet = data;              
	break;
    }
}

static LADSPA_Handle instantiateLMS(const LADSPA_Descriptor * descriptor,
				   unsigned long s_rate)
{
    LMS *plugin_data = (LMS *) malloc(sizeof(LMS));
    
    plugin_data->fs = s_rate;
    
    /*LibModSynth additions*/
    v_init_lms(s_rate);  //initialize any static variables    
    /*End LibModSynth additions*/
    
    return (LADSPA_Handle) plugin_data;
}

static void activateLMS(LADSPA_Handle instance)
{
    LMS *plugin_data = (LMS *) instance;
        
    plugin_data->mono_modules = v_mono_init((plugin_data->fs), 90.0f);  //initialize all monophonic modules
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
    /*Define our inputs*/
    LADSPA_Data *const input0 = plugin_data->input0;    
    LADSPA_Data *const input1 = plugin_data->input1;
    /*define our outputs*/
    LADSPA_Data *const output0 = plugin_data->output0;    
    LADSPA_Data *const output1 = plugin_data->output1;    
    
    /*Reset our iterators to 0*/
    plugin_data->pos = 0;
    plugin_data->count= 0;    
    plugin_data->i_mono_out = 0;
    plugin_data->event_pos = 0;
    
    /*Set the values from synth_vals in RunLMS*/
    plugin_data->vals.predelay = *(plugin_data->predelay) * .01;    
    plugin_data->vals.time = *(plugin_data->time) * .01;
    plugin_data->vals.highpass = *(plugin_data->highpass);    
    plugin_data->vals.lowpass = *(plugin_data->lowpass);
    plugin_data->vals.drywet = *(plugin_data->drywet) * .01;
    
    v_rvd_set_reverb(plugin_data->mono_modules->reverb, (plugin_data->vals.time), (plugin_data->vals.predelay));
    v_rvd_set_svf(plugin_data->mono_modules->reverb, (plugin_data->vals.highpass), (plugin_data->vals.lowpass));
    
    while ((plugin_data->pos) < sample_count) 
    {	
        
        plugin_data->count = (sample_count - (plugin_data->pos)) > STEP_SIZE ? STEP_SIZE :	sample_count - (plugin_data->pos);
	        
        plugin_data->i_buffer_clear = 0;
        /*Clear the output buffer*/
        while((plugin_data->i_buffer_clear)<(plugin_data->count))
        {
	    output0[((plugin_data->pos) + (plugin_data->i_buffer_clear))] = 0.0f;
            output1[((plugin_data->pos) + (plugin_data->i_buffer_clear))] = 0.0f;
            plugin_data->i_buffer_clear = (plugin_data->i_buffer_clear) + 1;
	}
        
        plugin_data->i_mono_out = 0;
        
        /*The main loop where processing happens*/
        while((plugin_data->i_mono_out) < (plugin_data->count))
        {   
            plugin_data->buffer_pos = (plugin_data->pos) + (plugin_data->i_mono_out);
    
            v_rvd_run_reverb(plugin_data->mono_modules->reverb, (input0[(plugin_data->buffer_pos)]), (input1[(plugin_data->buffer_pos)]));
            
            output0[(plugin_data->buffer_pos)] = f_axf_run_xfade(plugin_data->mono_modules->axf0, 
                    (input0[(plugin_data->buffer_pos)]), (plugin_data->mono_modules->reverb->out0));
            output1[(plugin_data->buffer_pos)] = f_axf_run_xfade(plugin_data->mono_modules->axf1, 
                    (input1[(plugin_data->buffer_pos)]), (plugin_data->mono_modules->reverb->out1));
            
            plugin_data->i_mono_out = (plugin_data->i_mono_out) + 1;
        }
        
        plugin_data->pos = (plugin_data->pos) + STEP_SIZE;
    }
}



/*This returns MIDI CCs for the different knobs
 TODO:  Try it with non-hex numbers*/
int getControllerLMS(LADSPA_Handle instance, unsigned long port)
{
    switch (port) {    
    case LMS_PREDELAY:
        return DSSI_CC(0x14);  //20
    case LMS_TIME:
        return DSSI_CC(0x15);  //21
    case LMS_HIGHPASS:
        return DSSI_CC(0x16);  //22
    case LMS_LOWPASS:
        return DSSI_CC(0x17);  //23
    case LMS_DRYWET:
        return DSSI_CC(0x18);  //24    
    default:
        return DSSI_NONE;
    }
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
	LMSLDescriptor->Label = "LMS";  
	LMSLDescriptor->Properties = 0;
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
        
	
	/* Parameters for predelay */
	port_descriptors[LMS_PREDELAY] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_PREDELAY] = "Predelay";
	port_range_hints[LMS_PREDELAY].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_PREDELAY].LowerBound =  0;
	port_range_hints[LMS_PREDELAY].UpperBound =  30;
        
        /* Parameters for time */
	port_descriptors[LMS_TIME] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_TIME] = "Time";
	port_range_hints[LMS_TIME].HintDescriptor =
			LADSPA_HINT_DEFAULT_HIGH |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_TIME].LowerBound =  0;
	port_range_hints[LMS_TIME].UpperBound =  400;

        /* Parameters for highpass */
	port_descriptors[LMS_HIGHPASS] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_HIGHPASS] = "Highpass";
	port_range_hints[LMS_HIGHPASS].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_HIGHPASS].LowerBound =  20;
	port_range_hints[LMS_HIGHPASS].UpperBound =  100;
        
        /* Parameters for lowpass */
	port_descriptors[LMS_LOWPASS] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_LOWPASS] = "Lowpass";
	port_range_hints[LMS_LOWPASS].HintDescriptor =
			LADSPA_HINT_DEFAULT_HIGH |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_LOWPASS].LowerBound =  40;
	port_range_hints[LMS_LOWPASS].UpperBound =  120;
        
        /* Parameters for dry/wet */
	port_descriptors[LMS_DRYWET] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_DRYWET] = "Dry/Wet";
	port_range_hints[LMS_DRYWET].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_DRYWET].LowerBound =  0;
	port_range_hints[LMS_DRYWET].UpperBound =  100;
                
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
	LMSDDescriptor->run_synth = runLMS;
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
