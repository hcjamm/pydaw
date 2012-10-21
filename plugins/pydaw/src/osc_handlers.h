/* 
 * File:   osc_handlers.h
 * 
 *  
 */

#ifndef OSC_HANDLERS_H
#define	OSC_HANDLERS_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include "pydaw.h"

/*
int pydaw_osc_midi_handler(t_pydaw_plugin *instance, lo_arg **argv)
{
    static snd_midi_event_t *alsaCoder = NULL;
    static snd_seq_event_t alsaEncodeBuffer[10];
    long count;
    snd_seq_event_t *ev = &alsaEncodeBuffer[0];

    if (verbose) {
	printf("%s: OSC: got midi request for %s "
	       "(%02x %02x %02x %02x)\n", myName, instance->friendly_name,
	       argv[0]->m[0], argv[0]->m[1], argv[0]->m[2], argv[0]->m[3]);
    }

    if (!alsaCoder) {
        if (snd_midi_event_new(10, &alsaCoder)) {
            fprintf(stderr, "%s: Failed to initialise ALSA MIDI coder!\n",
		    myName);
            return 0;
        }
    }

    snd_midi_event_reset_encode(alsaCoder);

    count = snd_midi_event_encode
	(alsaCoder, (argv[0]->m) + 1, 3, alsaEncodeBuffer); // ignore OSC "port id" in argv[0]->m[0]

    if (!count || !snd_seq_ev_is_channel_type(ev)) {
        return 0;
    }

    // substitute correct MIDI channel
    ev->data.note.channel = 0; // instance->channel;
    
    if (ev->type == SND_SEQ_EVENT_NOTEON && ev->data.note.velocity == 0) {
        ev->type =  SND_SEQ_EVENT_NOTEOFF;
    }
        
    pthread_mutex_lock(&midiEventBufferMutex);

    if (midiEventReadIndex == midiEventWriteIndex + 1) {

        fprintf(stderr, "%s: Warning: MIDI event buffer overflow!\n", myName);

    } else if (ev->type == SND_SEQ_EVENT_CONTROLLER &&
               (ev->data.control.param == 0 || ev->data.control.param == 32)) 
    {
        fprintf(stderr, "PyDAW: Warning: %s UI sent bank select controller (should use /program OSC call), ignoring\n", instance->friendly_name);
    } 
    else if (ev->type == SND_SEQ_EVENT_PGMCHANGE) 
    {
        fprintf(stderr, "PyDAW: Warning: %s UI sent program change (should use /program OSC call), ignoring\n", instance->friendly_name);
    } 
    else 
    {
        midiEventBuffer[midiEventWriteIndex] = *ev;
        midiEventWriteIndex = (midiEventWriteIndex + 1) % EVENT_BUFFER_SIZE;

    }

    pthread_mutex_unlock(&midiEventBufferMutex);

    return 0;
}
*/
int pydaw_osc_control_handler(t_pydaw_plugin *instance, lo_arg **argv)
{
    int port = argv[0]->i;
    LADSPA_Data value = argv[1]->f;

    if (port < 0 || port > instance->descriptor->LADSPA_Plugin->PortCount) {
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
    else if(!strcmp(key, "lastdir"))
    {
        instance->euphoria_last_dir_set = 1;
        strcpy(instance->euphoria_last_dir, value);
    }

    if (instance->descriptor->configure) 
    {
        message = instance->descriptor->configure(instance->ladspa_handle, key, value);
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

	if (!strncmp(key, DSSI_RESERVED_CONFIGURE_PREFIX,
		     strlen(DSSI_RESERVED_CONFIGURE_PREFIX))) {
	    fprintf(stderr, "%s: OSC: UI for plugin '%s' attempted to use reserved configure key \"%s\", ignoring\n", myName, instance->friendly_name, key);
	    return 0;
	}

        
	if (instance->plugin->instances > 1 &&
	    !strncmp(key, DSSI_GLOBAL_CONFIGURE_PREFIX,
		     strlen(DSSI_GLOBAL_CONFIGURE_PREFIX))) {
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
    const char *url = (char *)&argv[0]->s;
    const char *path;
    unsigned int i;
    char *host, *port;
    const char *chost, *cport;

    if (instance->uiTarget) lo_address_free(instance->uiTarget);
    host = lo_url_get_hostname(url);
    port = lo_url_get_port(url);
    instance->uiTarget = lo_address_new(host, port);
    free(host);
    free(port);

    if (instance->uiSource) lo_address_free(instance->uiSource);
    chost = lo_address_get_hostname(source);
    cport = lo_address_get_port(source);
    instance->uiSource = lo_address_new(chost, cport);

    path = lo_url_get_path(url);

    if (instance->ui_osc_control_path) free(instance->ui_osc_control_path);
    instance->ui_osc_control_path = (char *)malloc(strlen(path) + 10);
    sprintf(instance->ui_osc_control_path, "%s/control", path);

    if (instance->ui_osc_configure_path) free(instance->ui_osc_configure_path);
    instance->ui_osc_configure_path = (char *)malloc(strlen(path) + 12);
    sprintf(instance->ui_osc_configure_path, "%s/configure", path);

    if (instance->ui_osc_program_path) free(instance->ui_osc_program_path);
    instance->ui_osc_program_path = (char *)malloc(strlen(path) + 10);
    sprintf(instance->ui_osc_program_path, "%s/program", path);

    if (instance->ui_osc_quit_path) free(instance->ui_osc_quit_path);
    instance->ui_osc_quit_path = (char *)malloc(strlen(path) + 10);
    sprintf(instance->ui_osc_quit_path, "%s/quit", path);

    if (instance->ui_osc_rate_path) free(instance->ui_osc_rate_path);
    instance->ui_osc_rate_path = (char *)malloc(strlen(path) + 13);
    sprintf(instance->ui_osc_rate_path, "%s/sample-rate", path);

    if (instance->ui_osc_show_path) free(instance->ui_osc_show_path);
    instance->ui_osc_show_path = (char *)malloc(strlen(path) + 10);
    sprintf(instance->ui_osc_show_path, "%s/show", path);

    free((char *)path);

    /* Send sample rate */
    //Commenting out, AFAIK none of my UIs use the SR handler...
    //lo_send(instance->uiTarget, instance->ui_osc_rate_path, "i", lrintf(sample_rate));

    /* At this point a more substantial host might also call
     * configure() on the UI to set any state that it had remembered
     * for the plugin instance.  But we don't remember state for
     * plugin instances (see our own configure() implementation in
     * osc_configure_handler), and so we have nothing to send except
     * the optional project directory. */

    /*
    if (projectDirectory) {
	lo_send(instance->uiTarget, instance->ui_osc_configure_path, "ss",
		DSSI_PROJECT_DIRECTORY_KEY, projectDirectory);
    }
    */

    
    if (instance->euphoria_last_dir_set) {
	lo_send(instance->uiTarget, instance->ui_osc_configure_path, "ss",
		"lastdir", instance->euphoria_last_dir);
    }
        
    if (instance->euphoria_load_set) {
	lo_send(instance->uiTarget, instance->ui_osc_configure_path, "ss",
		"load", instance->euphoria_load);
    }

    /* Send control ports */
    for (i = 0; i < instance->controlIns; i++) 
    {
        int in = i + instance->firstControlIn;
	int port = instance->pluginControlInPortNumbers[in];
	lo_send(instance->uiTarget, instance->ui_osc_control_path, "if", port, instance->pluginControlIns[in]);
	/* Avoid overloading the GUI if there are lots and lots of ports */
	if ((i+1) % 50 == 0) usleep(50000);
    }

    /* Send 'show' */
    //if (!instance->ui_initial_show_sent) 
    //{
	lo_send(instance->uiTarget, instance->ui_osc_show_path, "");
	//instance->ui_initial_show_sent = 1;
    //}

    return 0;
}

int pydaw_osc_exiting_handler(t_pydaw_plugin *instance, lo_arg **argv)
{
    //int i;

    /*
    if (instance->uiTarget) 
    {
        lo_address_free(instance->uiTarget);
        instance->uiTarget = NULL;
    }
    if (instance->uiSource) 
    {
        lo_address_free(instance->uiSource);
        instance->uiSource = NULL;
    }
    */
    
    /*
    if (instance->plugin) 
    {
	instance->inactive = 1;
    }

    //Do we have any plugins left running?

    for (i = 0; i < instance_count; ++i) {
	if (!instances[i].inactive) return 0;
    }
    */
    //exiting = 1;
    return 0;
}


#ifdef	__cplusplus
}
#endif

#endif	/* OSC_HANDLERS_H */

