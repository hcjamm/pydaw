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

#include <unistd.h>
#include <alsa/asoundlib.h>

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
    t_pydaw_engine *plugin;

    plugin = (t_pydaw_engine *) instance;
    
    /*Add the ports from step 9 to the connectPortLMS event handler*/
    
    if((port >= LMS_INPUT_MIN) && (port < LMS_INPUT_MAX))
    {
        plugin->input_arr[(port - LMS_INPUT_MIN)] = data;
    }
    else
    {        
        switch (port) 
        {     
            case LMS_OUTPUT0: plugin->output0 = data; break;
            case LMS_OUTPUT1: plugin->output1 = data; break;        
        }
    }    
}

static LADSPA_Handle instantiateLMS(const LADSPA_Descriptor * descriptor,
				   unsigned long s_rate)
{
    t_pydaw_engine *plugin_data = (t_pydaw_engine *) malloc(sizeof(t_pydaw_engine));
    
    plugin_data->fs = s_rate;
            
    /*LibModSynth additions*/
    v_init_lms(s_rate);  //initialize any static variables    
    /*End LibModSynth additions*/
    
    return (LADSPA_Handle) plugin_data;
}

static void activateLMS(LADSPA_Handle instance)
{
    t_pydaw_engine *plugin_data = (t_pydaw_engine *) instance;
        
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
    t_pydaw_engine *plugin_data = (t_pydaw_engine *) instance;
    /*Define our inputs*/
    
    /*define our outputs*/
    LADSPA_Data *const output0 = plugin_data->output0;    
    LADSPA_Data *const output1 = plugin_data->output1;    
    
    /*Reset our iterators to 0*/
    plugin_data->pos = 0;
    plugin_data->count= 0;    
    plugin_data->i_mono_out = 0;
        
    while ((plugin_data->pos) < sample_count) 
    {        
	plugin_data->count = (sample_count - (plugin_data->pos)) > STEP_SIZE ? STEP_SIZE : sample_count - (plugin_data->pos);	
        
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
                       
            output0[(plugin_data->buffer_pos)] = (plugin_data->mono_modules->current_sample0);
            output1[(plugin_data->buffer_pos)] = (plugin_data->mono_modules->current_sample1);

            plugin_data->i_mono_out = (plugin_data->i_mono_out) + 1;
        }
        
        plugin_data->pos = (plugin_data->pos) + STEP_SIZE;
    }
}

char *pydaw_configure(LADSPA_Handle instance, const char *key, const char *value)
{
    //t_pydaw_engine *plugin_data = (t_pydaw_engine *)instance;

    if (!strcmp(key, "save")) {	
        printf("configure called with key 'save'\n");
        return NULL; //samplerLoadAll(plugin_data, value);    
    }  else if (!strcmp(key, "lastdir")) {
        //do nothing, this is only so the plugin host will recall the last sample directory
        return NULL;
    }

    return strdup("error: unrecognized configure key");
}


/*This returns MIDI CCs for the different knobs
 TODO:  Try it with non-hex numbers*/
int getControllerLMS(LADSPA_Handle instance, unsigned long port)
{    
    //t_pydaw_engine *plugin_data = (t_pydaw_engine *) instance;
    //return DSSI_CC(i_ccm_get_cc(plugin_data->midi_cc_map, port));
    return 0;
     
}

/*ALSA Sequencer Client Stuff*/

#define MAX_MIDI_PORTS   4

int open_seq(snd_seq_t **seq_handle, int in_ports[], int out_ports[], int num_in, int num_out);
void midi_route(snd_seq_t *seq_handle, int out_ports[], int split_point);

/* Open ALSA sequencer wit num_in writeable ports and num_out readable ports. */
/* The sequencer handle and the port IDs are returned.                        */  
int open_seq(snd_seq_t **seq_handle, int in_ports[], int out_ports[], int num_in, int num_out) {

  int l1;
  char portname[64];

  if (snd_seq_open(seq_handle, "default", SND_SEQ_OPEN_DUPLEX, 0) < 0) {
    fprintf(stderr, "Error opening ALSA sequencer.\n");
    return(-1);
  }
  snd_seq_set_client_name(*seq_handle, "MIDI Router");
  for (l1 = 0; l1 < num_in; l1++) {
    sprintf(portname, "MIDI Router IN %d", l1);
    if ((in_ports[l1] = snd_seq_create_simple_port(*seq_handle, portname,
              SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
              SND_SEQ_PORT_TYPE_APPLICATION)) < 0) {
      fprintf(stderr, "Error creating sequencer port.\n");
      return(-1);
    }
  }
  for (l1 = 0; l1 < num_out; l1++) {
    sprintf(portname, "MIDI Router OUT %d", l1);
    if ((out_ports[l1] = snd_seq_create_simple_port(*seq_handle, portname,
              SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ,
              SND_SEQ_PORT_TYPE_APPLICATION)) < 0) {
      fprintf(stderr, "Error creating sequencer port.\n");
      return(-1);
    }
  }
  return(0);
}

/* Read events from writeable port and route them to readable port 0  */
/* if NOTEON / OFF event with note < split_point. NOTEON / OFF events */
/* with note >= split_point are routed to readable port 1. All other  */
/* events are routed to both readable ports.                          */
void midi_route(snd_seq_t *seq_handle, int out_ports[], int split_point) {

  snd_seq_event_t *ev;

  do {
    snd_seq_event_input(seq_handle, &ev);
    snd_seq_ev_set_subs(ev);  
    snd_seq_ev_set_direct(ev);
    
    if ((ev->type == SND_SEQ_EVENT_NOTEON)||(ev->type == SND_SEQ_EVENT_NOTEOFF)) 
    {
      if (ev->data.note.note < split_point) {
        snd_seq_ev_set_source(ev, out_ports[0]);
      } else {
        snd_seq_ev_set_source(ev, out_ports[1]);
      }
      snd_seq_event_output_direct(seq_handle, ev);
    } 
    else 
    {
      snd_seq_ev_set_source(ev, out_ports[0]);
      snd_seq_event_output_direct(seq_handle, ev);
      snd_seq_ev_set_source(ev, out_ports[1]);
      snd_seq_event_output_direct(seq_handle, ev);
    }
    snd_seq_free_event(ev);
  } while (snd_seq_event_input_pending(seq_handle, 0) > 0);
}

int init_alsa_seq(int argc, char *argv[]) {

  snd_seq_t *seq_handle;
  int out_ports[MAX_MIDI_PORTS];
  int npfd, split_point;
  struct pollfd *pfd;
  
  if (argc < 2) {
    fprintf(stderr, "\nmidiroute <split_point>\n\n");
    exit(1);
  } else {
    split_point = atoi(argv[1]);  
  }  
  if (open_seq(&seq_handle, 0, out_ports, 1, 2) < 0) {
    fprintf(stderr, "ALSA Error.\n");
    exit(1);
  }
  npfd = snd_seq_poll_descriptors_count(seq_handle, POLLIN);
  pfd = (struct pollfd *)alloca(npfd * sizeof(struct pollfd));
  snd_seq_poll_descriptors(seq_handle, pfd, npfd, POLLIN);
  while (1) {
    if (poll(pfd, npfd, 100000) > 0) {
      midi_route(seq_handle, out_ports, split_point);
    }  
  }
}


/*End ALSA Sequencer Client Stuff*/

/*Stuff from midi-listen.c*/


static snd_seq_t *seq_handle;
static int in_port;

#define CHK(stmt, msg) if((stmt) < 0) {puts("ERROR: "#msg); exit(1);}
void midi_open(void)
{
    CHK(snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_INPUT, 0),
            "Could not open sequencer");

    CHK(snd_seq_set_client_name(seq_handle, "Midi Listener"),
            "Could not set client name");
    CHK(in_port = snd_seq_create_simple_port(seq_handle, "listen:in",
                SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
                SND_SEQ_PORT_TYPE_APPLICATION),
            "Could not open port");
}

snd_seq_event_t *midi_read(void)
{
    snd_seq_event_t *ev = NULL;
    snd_seq_event_input(seq_handle, &ev);
    return ev;
}

void midi_process(const snd_seq_event_t *ev)
{
    snd_seq_addr_t * test;
    snd_seq_port_subscribe_t * test2;
    
    //snd_seq_subscribe_port(seq_handle, )
    if((ev->type == SND_SEQ_EVENT_NOTEON)
            ||(ev->type == SND_SEQ_EVENT_NOTEOFF)) {
        const char *type = (ev->type==SND_SEQ_EVENT_NOTEON) ? "on " : "off";
        printf("[%d] Note %s: %2x vel(%2x)\n", ev->time.tick, type,
                                               ev->data.note.note,
                                               ev->data.note.velocity);
    }
    else if(ev->type == SND_SEQ_EVENT_CONTROLLER)
        printf("[%d] Control:  %2x val(%2x)\n", ev->time.tick,
                                                ev->data.control.param,
                                                ev->data.control.value);
    else
        printf("[%d] Unknown:  Unhandled Event Received\n", ev->time.tick);
}

int midi_listen_main()
{
    midi_open();
    while(1)
        midi_process(midi_read());
    return -1;
}

/*End stuff from midi-listen.c*/

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
        int f_i;
        
        for(f_i = LMS_INPUT_MIN; f_i < LMS_INPUT_MAX; f_i++)
        {
            port_descriptors[f_i] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
            port_names[f_i] = "Input ";  //TODO:  Give a more descriptive port name
            port_range_hints[f_i].HintDescriptor = 0;
        }
        
	/* Parameters for output */
	port_descriptors[LMS_OUTPUT0] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
	port_names[LMS_OUTPUT0] = "Output 0";
	port_range_hints[LMS_OUTPUT0].HintDescriptor = 0;

        port_descriptors[LMS_OUTPUT1] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
	port_names[LMS_OUTPUT1] = "Output 1";
	port_range_hints[LMS_OUTPUT1].HintDescriptor = 0;
               
        
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
	LMSDDescriptor->configure = pydaw_configure;
	LMSDDescriptor->get_program = NULL;
	LMSDDescriptor->get_midi_controller_for_port = getControllerLMS;
	LMSDDescriptor->select_program = NULL;
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
