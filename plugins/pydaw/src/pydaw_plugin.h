/* 
 * File:   pydaw_plugin.h
 * Author: jeffh
 *
 * Created on October 11, 2012, 6:48 PM
 */

#ifndef PYDAW_PLUGIN_H
#define	PYDAW_PLUGIN_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "dssi.h"
#include "pydaw.h"
#include <ladspa.h>
#include <lo/lo.h>
#include <alsa/asoundlib.h>
#include <dlfcn.h>
#include <math.h>
#include <stdlib.h>
    
#define PYDAW_MAX_BUFFER_SIZE 4096
    
LADSPA_Data get_port_default(const LADSPA_Descriptor *plugin, int port, int sample_rate)
{
    LADSPA_PortRangeHint hint = plugin->PortRangeHints[port];
    float lower = hint.LowerBound *
	(LADSPA_IS_HINT_SAMPLE_RATE(hint.HintDescriptor) ? sample_rate : 1.0f);
    float upper = hint.UpperBound *
	(LADSPA_IS_HINT_SAMPLE_RATE(hint.HintDescriptor) ? sample_rate : 1.0f);

    if (!LADSPA_IS_HINT_HAS_DEFAULT(hint.HintDescriptor)) {
	if (!LADSPA_IS_HINT_BOUNDED_BELOW(hint.HintDescriptor) ||
	    !LADSPA_IS_HINT_BOUNDED_ABOVE(hint.HintDescriptor)) {
	    /* No hint, its not bounded, wild guess */
	    return 0.0f;
	}

	if (lower <= 0.0f && upper >= 0.0f) {
	    /* It spans 0.0, 0.0 is often a good guess */
	    return 0.0f;
	}

	/* No clues, return minimum */
	return lower;
    }

    /* Try all the easy ones */
    
    if (LADSPA_IS_HINT_DEFAULT_0(hint.HintDescriptor)) {
	return 0.0f;
    } else if (LADSPA_IS_HINT_DEFAULT_1(hint.HintDescriptor)) {
	return 1.0f;
    } else if (LADSPA_IS_HINT_DEFAULT_100(hint.HintDescriptor)) {
	return 100.0f;
    } else if (LADSPA_IS_HINT_DEFAULT_440(hint.HintDescriptor)) {
	return 440.0f;
    }

    /* All the others require some bounds */

    if (LADSPA_IS_HINT_BOUNDED_BELOW(hint.HintDescriptor)) {
	if (LADSPA_IS_HINT_DEFAULT_MINIMUM(hint.HintDescriptor)) {
	    return lower;
	}
    }
    if (LADSPA_IS_HINT_BOUNDED_ABOVE(hint.HintDescriptor)) {
	if (LADSPA_IS_HINT_DEFAULT_MAXIMUM(hint.HintDescriptor)) {
	    return upper;
	}
	if (LADSPA_IS_HINT_BOUNDED_BELOW(hint.HintDescriptor)) {
            if (LADSPA_IS_HINT_LOGARITHMIC(hint.HintDescriptor) &&
                lower > 0.0f && upper > 0.0f) {
                if (LADSPA_IS_HINT_DEFAULT_LOW(hint.HintDescriptor)) {
                    return expf(logf(lower) * 0.75f + logf(upper) * 0.25f);
                } else if (LADSPA_IS_HINT_DEFAULT_MIDDLE(hint.HintDescriptor)) {
                    return expf(logf(lower) * 0.5f + logf(upper) * 0.5f);
                } else if (LADSPA_IS_HINT_DEFAULT_HIGH(hint.HintDescriptor)) {
                    return expf(logf(lower) * 0.25f + logf(upper) * 0.75f);
                }
            } else {
                if (LADSPA_IS_HINT_DEFAULT_LOW(hint.HintDescriptor)) {
                    return lower * 0.75f + upper * 0.25f;
                } else if (LADSPA_IS_HINT_DEFAULT_MIDDLE(hint.HintDescriptor)) {
                    return lower * 0.5f + upper * 0.5f;
                } else if (LADSPA_IS_HINT_DEFAULT_HIGH(hint.HintDescriptor)) {
                    return lower * 0.25f + upper * 0.75f;
                }
	    }
	}
    }

    /* fallback */
    return 0.0f;
}

typedef struct st_pydaw_plugin
{
    void * lib_handle;
    float buffer[2][PYDAW_MAX_BUFFER_SIZE];  //Buffer for writing the audio output to(and reading from???)
    
    DSSI_Descriptor_Function descfn;
    
    LADSPA_Handle ladspa_handle;
    
    const DSSI_Descriptor *descriptor;
    int    ins;
    int    outs;
    int    controlIns;
    int    controlOuts;    
    int    firstControlIn;                       /* the offset to translate instance control in # to global control in # */
    int    *pluginPortControlInNumbers;           /* maps instance LADSPA port # to global control in # */
    long   controllerMap[128]; /* maps MIDI controller to global control in # */
    DSSI_Program_Descriptor *pluginPrograms;
        
    lo_address       uiTarget;
    lo_address       uiSource;
    int              ui_initial_show_sent;    
    char            *ui_osc_control_path;
    char            *ui_osc_configure_path;
    char            *ui_osc_program_path;
    char            *ui_osc_quit_path;
    char            *ui_osc_rate_path;
    char            *ui_osc_show_path;
        
    float **pluginInputBuffers, **pluginOutputBuffers;

    float *pluginControlIns, *pluginControlOuts;
    unsigned long *pluginControlInPortNumbers;          /* maps global control in # to instance LADSPA port # */
    
    int * pluginPortUpdated;
    /* Since there are no plans of allowing 3rd party DSSI plugins, the host will only remember known configure keys
     that should be recalled by the UI*/
    char euphoria_load[16384];
    int euphoria_load_set;
    //char euphoria_last_dir[512];
    //int euphoria_last_dir_set;
    
    int ui_visible;
}t_pydaw_plugin;

void setControl(t_pydaw_plugin *instance, long controlIn, snd_seq_event_t *event)
{
    long port = instance->pluginControlInPortNumbers[controlIn];

    const LADSPA_Descriptor *p = instance->descriptor->LADSPA_Plugin;

    LADSPA_PortRangeHintDescriptor d = p->PortRangeHints[port].HintDescriptor;

    LADSPA_Data lb = p->PortRangeHints[port].LowerBound ;
        /* * (LADSPA_IS_HINT_SAMPLE_RATE(p->PortRangeHints[port].HintDescriptor) ?
	 instance->sample_rate : 1.0f); */

    LADSPA_Data ub = p->PortRangeHints[port].UpperBound ;
         /*   * (LADSPA_IS_HINT_SAMPLE_RATE(p->PortRangeHints[port].HintDescriptor) ?
	 sample_rate : 1.0f); */

    float value = (float)event->data.control.value;

    if (!LADSPA_IS_HINT_BOUNDED_BELOW(d)) 
    {
	if (!LADSPA_IS_HINT_BOUNDED_ABOVE(d)) {
	    /* unbounded: might as well leave the value alone. */
            return;
	} else {
	    /* bounded above only. just shift the range. */
	    value = ub - 127.0f + value;
	}
    } 
    else 
    {
	if (!LADSPA_IS_HINT_BOUNDED_ABOVE(d)) {
	    /* bounded below only. just shift the range. */
	    value = lb + value;
	} else {
	    /* bounded both ends.  more interesting. */
            if (LADSPA_IS_HINT_LOGARITHMIC(d) && lb > 0.0f && ub > 0.0f) {
		const float llb = logf(lb);
		const float lub = logf(ub);

		value = expf(llb + ((lub - llb) * value / 127.0f));
	    } else {
		value = lb + ((ub - lb) * value / 127.0f);
	    }
	}
    }
    if (LADSPA_IS_HINT_INTEGER(d)) 
    {
        value = lrintf(value);
    }

    instance->pluginControlIns[controlIn] = value;
    instance->pluginPortUpdated[controlIn] = 1;
}


t_pydaw_plugin * g_pydaw_plugin_get(int a_sample_rate, int a_index)
{
    t_pydaw_plugin * f_result = (t_pydaw_plugin*)malloc(sizeof(t_pydaw_plugin));  //TODO: posix_memalign instead...
    
    switch(a_index)
    {
        case 1:
            f_result->lib_handle = dlopen("/usr/lib/dssi/euphoria.so", RTLD_NOW | RTLD_LOCAL);                  
            break;
        case 2:
            f_result->lib_handle = dlopen("/usr/lib/dssi/ray_v.so", RTLD_NOW | RTLD_LOCAL);                  
            break;
    }
    
    f_result->ui_visible = 0;
    
    //f_result->euphoria_last_dir_set = 0;
    f_result->euphoria_load_set = 0;
        
    f_result->pluginPrograms = NULL;    
    f_result->uiTarget = NULL;
    f_result->uiSource = NULL;
    f_result->ui_initial_show_sent = 0;
    f_result->ui_osc_control_path = NULL;
    f_result->ui_osc_configure_path = NULL;
    f_result->ui_osc_program_path = NULL;
    f_result->ui_osc_quit_path = NULL;
    f_result->ui_osc_rate_path = NULL;
    f_result->ui_osc_show_path = NULL;
    
    f_result->descfn = (DSSI_Descriptor_Function)dlsym(f_result->lib_handle, "dssi_descriptor");
    
    f_result->descriptor = f_result->descfn(0);
    
    f_result->ins = 0;
    f_result->outs = 0;
    f_result->controlIns = 0;
    f_result->controlOuts = 0;

    int j;
    
    for (j = 0; j < f_result->descriptor->LADSPA_Plugin->PortCount; j++) 
    {
        LADSPA_PortDescriptor pod = f_result->descriptor->LADSPA_Plugin->PortDescriptors[j];

        if (LADSPA_IS_PORT_AUDIO(pod)) 
        {
            if (LADSPA_IS_PORT_INPUT(pod)) ++f_result->ins;
            else if (LADSPA_IS_PORT_OUTPUT(pod)) ++f_result->outs;
        } 
        else if (LADSPA_IS_PORT_CONTROL(pod)) 
        {
            if (LADSPA_IS_PORT_INPUT(pod)) 
            {
                ++f_result->controlIns;
            }
            else if (LADSPA_IS_PORT_OUTPUT(pod)) 
            {
                ++f_result->controlOuts;
            }
        }
    }
   
    /* finish up new plugin */
    f_result->pluginPortControlInNumbers =
            (int*)malloc(f_result->descriptor->LADSPA_Plugin->PortCount *
                          sizeof(int));

    printf("f_result->in %i\n", f_result->ins);
    printf("f_result->outs %i\n", f_result->outs);
    printf("f_result->controlIns %i\n", f_result->controlIns);
    printf("f_result->controlOuts %i\n", f_result->controlOuts);    
    
    //f_result->inputPorts = (jack_port_t **)malloc(f_result->insTotal * sizeof(jack_port_t *));
    f_result->pluginInputBuffers = (float**)malloc((f_result->ins) * sizeof(float*));
    f_result->pluginControlIns = (float*)calloc(f_result->controlIns, sizeof(float));
    //f_result->pluginControlInInstances = (d3h_instance_t **)malloc(f_result->controlInsTotal * sizeof(d3h_instance_t *));
    f_result->pluginControlInPortNumbers = (unsigned long *)malloc(f_result->controlIns * sizeof(unsigned long));
    f_result->pluginPortUpdated = (int*)malloc(f_result->controlIns * sizeof(int));
    //f_result->outputPorts = (jack_port_t **)malloc(f_result->outsTotal * sizeof(jack_port_t *));
    f_result->pluginOutputBuffers = (float**)malloc((f_result->outs) * sizeof(float*));
    f_result->pluginControlOuts = (float *)calloc(f_result->controlOuts, sizeof(float));
    
    //TODO:  Count ins and outs from the loop at line 1142.  Or just rely on that we already know it
    
    f_result->ladspa_handle = f_result->descriptor->LADSPA_Plugin->instantiate(f_result->descriptor->LADSPA_Plugin, a_sample_rate);
        
    f_result->firstControlIn = 0;
    
    for (j = 0; j < 128; j++) 
    {
        f_result->controllerMap[j] = -1;
    }
    
    int in, out, controlIn, controlOut;
    
    in = out = controlIn = controlOut = 0;
    
    for (j = 0; j < f_result->descriptor->LADSPA_Plugin->PortCount; j++) 
    {
        LADSPA_PortDescriptor pod =
            f_result->descriptor->LADSPA_Plugin->PortDescriptors[j];

        f_result->pluginPortControlInNumbers[j] = -1;

        if (LADSPA_IS_PORT_AUDIO(pod)) {

            if (LADSPA_IS_PORT_INPUT(pod)) 
            {
                f_result->pluginInputBuffers[in] = (float*)calloc(8192, sizeof(float));
                f_result->descriptor->LADSPA_Plugin->connect_port(f_result->ladspa_handle, j, f_result->pluginInputBuffers[in]);                                
                in++;
            } 
            else if (LADSPA_IS_PORT_OUTPUT(pod)) 
            {
                f_result->pluginOutputBuffers[out] = (float*)calloc(8192, sizeof(float));
                f_result->descriptor->LADSPA_Plugin->connect_port(f_result->ladspa_handle, j, f_result->pluginOutputBuffers[out]);                
                out++;
            }

        } 
        else if (LADSPA_IS_PORT_CONTROL(pod)) 
        {

            if (LADSPA_IS_PORT_INPUT(pod)) {

                if (f_result->descriptor->get_midi_controller_for_port) {

                    int controller = f_result->descriptor->get_midi_controller_for_port(f_result->ladspa_handle, j);

                    /*if (controller == 0) {
                        MB_MESSAGE
                            ("Buggy plugin: wants mapping for bank MSB\n");
                    } else if (controller == 32) {
                        MB_MESSAGE
                            ("Buggy plugin: wants mapping for bank LSB\n");
                    } else*/
                    if (DSSI_IS_CC(controller)) 
                    {
                        f_result->controllerMap[DSSI_CC_NUMBER(controller)] = controlIn;
                    }
                }

                //TODO:  Most of these haven't been alloc'd, this is going to SEGFAULT
                //also, plugin input/output buffers need to be initialized too...

                /*
                f_result->pluginControlInInstances[controlIn] = instance;
                 */
                f_result->pluginControlInPortNumbers[controlIn] = j;
                f_result->pluginPortControlInNumbers[j] = controlIn;

                f_result->pluginControlIns[controlIn] = get_port_default
                    (f_result->descriptor->LADSPA_Plugin, j, a_sample_rate);

                f_result->descriptor->LADSPA_Plugin->connect_port
                    (f_result->ladspa_handle, j, &f_result->pluginControlIns[controlIn++]);

            } else if (LADSPA_IS_PORT_OUTPUT(pod)) {
                f_result->descriptor->LADSPA_Plugin->connect_port
                    (f_result->ladspa_handle, j, &f_result->pluginControlOuts[controlOut++]);
            }
        }
    }
    
    f_result->descriptor->LADSPA_Plugin->activate(f_result->ladspa_handle);
    
    return f_result;
}

void v_free_pydaw_plugin(t_pydaw_plugin * a_plugin)
{
    /*TODO:  This isn't all of the pointers to be free'd*/
    //free(a_plugin->pluginControlInPortNumbers);
    //free(a_plugin->pluginControlIns);
    //free(a_plugin->pluginControlOuts);
    //free(a_plugin->pluginPortControlInNumbers);
    //free(a_plugin->pluginPortUpdated);
    if(a_plugin)
    {
        free(a_plugin);
    }
}

void v_run_plugin(t_pydaw_plugin * a_plugin, int a_sample_count, snd_seq_event_t * a_event_buffer, 
        int a_event_count)
{
    a_plugin->descriptor->run_synth(a_plugin->ladspa_handle, a_sample_count, a_event_buffer, a_event_count); 
}

#ifdef	__cplusplus
}
#endif

#endif	/* PYDAW_PLUGIN_H */

