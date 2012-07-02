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
    case LMS_DELAY_TIME:
	plugin->delay_time = data;              
	break;
    case LMS_FEEDBACK:
	plugin->feedback = data;              
	break;
    case LMS_DRY:
	plugin->dry = data;              
	break;
    case LMS_WET:
	plugin->wet = data;              
	break;
    case LMS_DUCK:
	plugin->duck = data;              
	break;
    case LMS_CUTOFF:
	plugin->cutoff = data;              
	break;
    case LMS_STEREO:
        plugin->stereo = data;
        break;
    }
}

static LADSPA_Handle instantiateLMS(const LADSPA_Descriptor * descriptor,
				   unsigned long s_rate)
{
    LMS *plugin_data = (LMS *) malloc(sizeof(LMS));
    
    plugin_data->fs = s_rate;
    
    plugin_data->midi_cc_map = g_ccm_get();
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_DELAY_TIME, 20, "Delay Time");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_FEEDBACK, 21, "Feedback");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_DRY, 22, "Dry");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_WET, 23, "Wet");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_DUCK, 24, "Duck");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_CUTOFF, 25, "Cutoff");
    v_ccm_set_cc(plugin_data->midi_cc_map, LMS_STEREO, 26, "Stereo");
    

    v_ccm_read_file_to_array(plugin_data->midi_cc_map, "lms_delay-cc_map.txt");

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
    plugin_data->event_pos = 0;
#ifdef LMS_DEBUGGER_PROJECT
    plugin_data->vals.delay_time = 0.2f;    
    plugin_data->vals.feedback = -10.0f;
    plugin_data->vals.dry = -6;    
    plugin_data->vals.wet = -12;
    plugin_data->vals.duck = -20;    
    plugin_data->vals.cutoff = 100;
    plugin_data->vals.stereo = 0.5f;
#else
    /*Set the values from synth_vals in RunLMS*/
    plugin_data->vals.delay_time = *(plugin_data->delay_time) * .01;    
    plugin_data->vals.feedback = *(plugin_data->feedback);
    plugin_data->vals.dry = *(plugin_data->dry);    
    plugin_data->vals.wet = *(plugin_data->wet);
    plugin_data->vals.duck = *(plugin_data->duck);    
    plugin_data->vals.cutoff = *(plugin_data->cutoff);
    plugin_data->vals.stereo = *(plugin_data->stereo) * .01;
#endif
    
    v_svf_set_cutoff_base(plugin_data->mono_modules->svf0, (plugin_data->vals.cutoff));
    v_svf_set_cutoff_base(plugin_data->mono_modules->svf1, (plugin_data->vals.cutoff));
            
    v_svf_set_cutoff(plugin_data->mono_modules->svf0);
    v_svf_set_cutoff(plugin_data->mono_modules->svf1);
    
    while ((plugin_data->pos) < sample_count) 
    {	
        /*
        while ((plugin_data->event_pos) < event_count)
        {
	    if (events[(plugin_data->event_pos)].type == SND_SEQ_EVENT_TEMPO) 
            {
                //events[(plugin_data->event_pos)].data.raw32
            }
        }
        */
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
            
            v_smr_iir_run(plugin_data->mono_modules->time_smoother, (plugin_data->vals.delay_time));
            
            v_enf_run_env_follower(plugin_data->mono_modules->env_follower, ((input0[(plugin_data->buffer_pos)]) + (input1[(plugin_data->buffer_pos)])));
            
            /*If above the ducking threshold, reduce the wet amount by the same*/
            if((plugin_data->mono_modules->env_follower->output_smoothed) > (plugin_data->vals.duck))
            {
                v_ldl_set_delay(plugin_data->mono_modules->delay, (plugin_data->mono_modules->time_smoother->output), 
                        (plugin_data->vals.feedback),
                        ((plugin_data->vals.wet) - ((plugin_data->mono_modules->env_follower->output_smoothed) - (plugin_data->vals.duck))), 
                        (plugin_data->vals.dry), (plugin_data->vals.stereo));
            }
            else
            {
                v_ldl_set_delay(plugin_data->mono_modules->delay, (plugin_data->mono_modules->time_smoother->output), (plugin_data->vals.feedback), 
                        (plugin_data->vals.wet), (plugin_data->vals.dry), (plugin_data->vals.stereo));
            }
    
            
    
            v_ldl_run_delay(plugin_data->mono_modules->delay, (input0[(plugin_data->buffer_pos)]), (input1[(plugin_data->buffer_pos)]));
            
            output0[(plugin_data->buffer_pos)] = (plugin_data->mono_modules->delay->output0);
            output1[(plugin_data->buffer_pos)] = (plugin_data->mono_modules->delay->output1);
            
            plugin_data->mono_modules->delay->feedback0 = v_svf_run_2_pole_lp(plugin_data->mono_modules->svf0, (plugin_data->mono_modules->delay->feedback0));
            plugin_data->mono_modules->delay->feedback1 = v_svf_run_2_pole_lp(plugin_data->mono_modules->svf1, (plugin_data->mono_modules->delay->feedback1));
                 
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
	LMSLDescriptor->Properties = LADSPA_PROPERTY_HARD_RT_CAPABLE; // LADSPA_PROPERTY_INPLACE_BROKEN; //0;
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
        
	
	/* Parameters for delay time */
	port_descriptors[LMS_DELAY_TIME] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_DELAY_TIME] = "Delay Time";
	port_range_hints[LMS_DELAY_TIME].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_DELAY_TIME].LowerBound =  10;
	port_range_hints[LMS_DELAY_TIME].UpperBound =  100;
        
        /* Parameters for feedback */
	port_descriptors[LMS_FEEDBACK] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_FEEDBACK] = "Feedback";
	port_range_hints[LMS_FEEDBACK].HintDescriptor =
			LADSPA_HINT_DEFAULT_HIGH |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_FEEDBACK].LowerBound =  -15;
	port_range_hints[LMS_FEEDBACK].UpperBound =  0;

        /* Parameters for dry */
	port_descriptors[LMS_DRY] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_DRY] = "Dry";
	port_range_hints[LMS_DRY].HintDescriptor =
			LADSPA_HINT_DEFAULT_MAXIMUM |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_DRY].LowerBound =  -30;
	port_range_hints[LMS_DRY].UpperBound =  0;
        
        /* Parameters for wet */
	port_descriptors[LMS_WET] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_WET] = "Wet";
	port_range_hints[LMS_WET].HintDescriptor =
			LADSPA_HINT_DEFAULT_HIGH |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_WET].LowerBound =  -30;
	port_range_hints[LMS_WET].UpperBound =  0;
        
        /* Parameters for duck */
	port_descriptors[LMS_DUCK] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_DUCK] = "Duck";
	port_range_hints[LMS_DUCK].HintDescriptor =
			LADSPA_HINT_DEFAULT_MIDDLE |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_DUCK].LowerBound =  -40;
	port_range_hints[LMS_DUCK].UpperBound =  0;
        
        /* Parameters for cutoff */
	port_descriptors[LMS_CUTOFF] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_CUTOFF] = "Cutoff";
	port_range_hints[LMS_CUTOFF].HintDescriptor =
			LADSPA_HINT_DEFAULT_HIGH |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_CUTOFF].LowerBound =  40;
	port_range_hints[LMS_CUTOFF].UpperBound =  118;
        
        
        /* Parameters for stereo */
	port_descriptors[LMS_STEREO] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[LMS_STEREO] = "Stereo";
	port_range_hints[LMS_STEREO].HintDescriptor =
			LADSPA_HINT_DEFAULT_MAXIMUM |
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[LMS_STEREO].LowerBound =  0;
	port_range_hints[LMS_STEREO].UpperBound =  100;
        
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
