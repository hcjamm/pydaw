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

#ifndef PYFX_PORTS_H
#define	PYFX_PORTS_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "../../src/pydaw/include/pydaw3/pydaw_plugin.h"
#include <math.h>
    
PYFX_Data get_port_default(const PYFX_Descriptor *plugin, int port, int sample_rate)
{
    PYFX_PortRangeHint hint = plugin->PortRangeHints[port];    
    assert(hint.DefaultValue <= hint.UpperBound && hint.DefaultValue >= hint.LowerBound );
    return hint.DefaultValue;
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

