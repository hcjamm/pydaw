/* 
 * File:   PYFX_ports.h
 * Author: JeffH
 *
 * Created on November 4, 2012, 12:58 AM
 */

#ifndef PYFX_PORTS_H
#define	PYFX_PORTS_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "../../plugins/include/pydaw3/pydaw_plugin.h"
#include <math.h>
    
PYFX_Data get_port_default(const PYFX_Descriptor *plugin, int port, int sample_rate)
{
    PYFX_PortRangeHint hint = plugin->PortRangeHints[port];
    float lower = hint.LowerBound *
	(PYFX_IS_HINT_SAMPLE_RATE(hint.HintDescriptor) ? sample_rate : 1.0f);
    float upper = hint.UpperBound *
	(PYFX_IS_HINT_SAMPLE_RATE(hint.HintDescriptor) ? sample_rate : 1.0f);

    if (!PYFX_IS_HINT_HAS_DEFAULT(hint.HintDescriptor)) {
	if (!PYFX_IS_HINT_BOUNDED_BELOW(hint.HintDescriptor) ||
	    !PYFX_IS_HINT_BOUNDED_ABOVE(hint.HintDescriptor)) {
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
    
    if (PYFX_IS_HINT_DEFAULT_0(hint.HintDescriptor)) {
	return 0.0f;
    } else if (PYFX_IS_HINT_DEFAULT_1(hint.HintDescriptor)) {
	return 1.0f;
    } else if (PYFX_IS_HINT_DEFAULT_100(hint.HintDescriptor)) {
	return 100.0f;
    } else if (PYFX_IS_HINT_DEFAULT_440(hint.HintDescriptor)) {
	return 440.0f;
    }

    /* All the others require some bounds */

    if (PYFX_IS_HINT_BOUNDED_BELOW(hint.HintDescriptor)) {
	if (PYFX_IS_HINT_DEFAULT_MINIMUM(hint.HintDescriptor)) {
	    return lower;
	}
    }
    if (PYFX_IS_HINT_BOUNDED_ABOVE(hint.HintDescriptor)) {
	if (PYFX_IS_HINT_DEFAULT_MAXIMUM(hint.HintDescriptor)) {
	    return upper;
	}
	if (PYFX_IS_HINT_BOUNDED_BELOW(hint.HintDescriptor)) {
            if (PYFX_IS_HINT_LOGARITHMIC(hint.HintDescriptor) &&
                lower > 0.0f && upper > 0.0f) {
                if (PYFX_IS_HINT_DEFAULT_LOW(hint.HintDescriptor)) {
                    return expf(logf(lower) * 0.75f + logf(upper) * 0.25f);
                } else if (PYFX_IS_HINT_DEFAULT_MIDDLE(hint.HintDescriptor)) {
                    return expf(logf(lower) * 0.5f + logf(upper) * 0.5f);
                } else if (PYFX_IS_HINT_DEFAULT_HIGH(hint.HintDescriptor)) {
                    return expf(logf(lower) * 0.25f + logf(upper) * 0.75f);
                }
            } else {
                if (PYFX_IS_HINT_DEFAULT_LOW(hint.HintDescriptor)) {
                    return lower * 0.75f + upper * 0.25f;
                } else if (PYFX_IS_HINT_DEFAULT_MIDDLE(hint.HintDescriptor)) {
                    return lower * 0.5f + upper * 0.5f;
                } else if (PYFX_IS_HINT_DEFAULT_HIGH(hint.HintDescriptor)) {
                    return lower * 0.25f + upper * 0.75f;
                }
	    }
	}
    }

    /* fallback */
    return 0.0f;
}

void set_PYFX_ports(const PYINST_Descriptor * a_ddesc, PYFX_Handle * a_handle, float * a_control_ins)
{   
    int in, out, controlIn, controlOut, j;
    
    in = out = controlIn = controlOut, j = 0;
    
    for (j = 0; j < a_ddesc->PYFX_Plugin->PortCount; j++) 
    {
        PYFX_PortDescriptor pod =
            a_ddesc->PYFX_Plugin->PortDescriptors[j];

        a_control_ins[j] = -1;

        if (PYFX_IS_PORT_AUDIO(pod)) {

            if (PYFX_IS_PORT_INPUT(pod)) 
            {
                //f_result->pluginInputBuffers[in] = (float*)calloc(8192, sizeof(float));
                //a_ddesc->PYFX_Plugin->connect_port(f_result->PYFX_handle, j, f_result->pluginInputBuffers[in]);                                
                in++;
            } 
            else if (PYFX_IS_PORT_OUTPUT(pod)) 
            {
                //f_result->pluginOutputBuffers[out] = (float*)calloc(8192, sizeof(float));
                //a_ddesc->PYFX_Plugin->connect_port(f_result->PYFX_handle, j, f_result->pluginOutputBuffers[out]);                
                out++;
            }

        } 
        else if (PYFX_IS_PORT_CONTROL(pod)) 
        {
            if (PYFX_IS_PORT_INPUT(pod)) {
                //f_result->pluginControlInPortNumbers[controlIn] = j;
                //f_result->pluginPortControlInNumbers[j] = controlIn;

                a_control_ins[controlIn] = get_port_default(a_ddesc->PYFX_Plugin, j, 44100);

                a_ddesc->PYFX_Plugin->connect_port(a_handle, j, &a_control_ins[controlIn++]);

            } else if (PYFX_IS_PORT_OUTPUT(pod)) {
                //a_ddesc->PYFX_Plugin->connect_port(f_result->PYFX_handle, j, &f_result->pluginControlOuts[controlOut++]);
            }
        }
    }
}
    


#ifdef	__cplusplus
}
#endif

#endif	/* PYFX_PORTS_H */
