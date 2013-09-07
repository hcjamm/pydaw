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

#ifndef OSC_HANDLERS_H
#define	OSC_HANDLERS_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include "pydaw.h"

int pydaw_osc_control_handler(t_pydaw_plugin *instance, lo_arg **argv)
{
    int port = argv[0]->i;
    PYFX_Data value = argv[1]->f;

    if (port < 0 || port > instance->descriptor->PYFX_Plugin->PortCount) {
	fprintf(stderr, "PyDAW: OSC: port number (%d) is out of range\n", port);
	return 0;
    }
    if (instance->pluginPortControlInNumbers[port] == -1) {
	fprintf(stderr, "PyDAW: OSC: port %d is not a control in\n", port);
	return 0;
    }
    instance->pluginControlIns[instance->pluginPortControlInNumbers[port]] = value;
        
    return 0;
}

int pydaw_osc_configure_handler(t_pydaw_plugin *instance, lo_arg **argv)
{
    const char *key = (const char *)&argv[0]->s;
    const char *value = (const char *)&argv[1]->s;
    char *message;
        
    if(!strcmp(key, "load"))
    {
        instance->euphoria_load_set = 1;
        strcpy(instance->euphoria_load, value);
    }
    /*else if(!strcmp(key, "lastdir"))
    {
        instance->euphoria_last_dir_set = 1;
        strcpy(instance->euphoria_last_dir, value);
    }*/

    if (instance->descriptor->configure) 
    {
        message = instance->descriptor->configure(instance->PYFX_handle, key, value);
        if (message) 
        {
            printf("PyDAW: on configure '%s' '%s', plugin returned error '%s'\n",key, value, message);
            free(message);
        }
        
        //TODO:  Would this need to be sent to the UI?  Probably not?
        //lo_send(instance->uiTarget, instance->ui_osc_configure_path, "ss", key, value);        
        
        
        /*
	int n = instance->number;
	int m = n;

	if (!strncmp(key, PYINST_RESERVED_CONFIGURE_PREFIX,
		     strlen(PYINST_RESERVED_CONFIGURE_PREFIX))) {
	    fprintf(stderr, "%s: OSC: UI for plugin '%s' attempted to use reserved configure key \"%s\", ignoring\n", myName, instance->friendly_name, key);
	    return 0;
	}

        
	if (instance->plugin->instances > 1 &&
	    !strncmp(key, PYINST_GLOBAL_CONFIGURE_PREFIX,
		     strlen(PYINST_GLOBAL_CONFIGURE_PREFIX))) {
	    while (n > 0 && instances[n-1].plugin == instances[m].plugin) --n;
	    m = n + instances[n].plugin->instances - 1;
	}
	*/
        
        /*
	while (n <= m) {

	    message = instances[n].plugin->descriptor->configure(instanceHandles[n], key, value);
	    if (message) 
            {
		printf("PyDAW: on configure '%s' '%s', plugin '%s' returned error '%s'\n",
		       key, value, instance->friendly_name, message);
		free(message);
	    }

	    // also call back on UIs for plugins other than the one
	    // that requested this:
	    if (n != instance->number && instances[n].uiTarget) 
            {
		lo_send(instances[n].uiTarget, instances[n].ui_osc_configure_path, "ss", key, value);
	    }
	*/	
	    /* configure invalidates bank and program information, so
	       we should do this again now: */
	    //query_programs(&instances[n]);
	    //++n;
	//}	    
    }

    return 0;
}

int pydaw_osc_update_handler(t_pydaw_plugin *instance, lo_arg **argv, lo_address source)
{    
    return 0;
}

int pydaw_osc_exiting_handler(t_pydaw_plugin *instance, lo_arg **argv)
{
    return 0;
}


#ifdef	__cplusplus
}
#endif

#endif	/* OSC_HANDLERS_H */

