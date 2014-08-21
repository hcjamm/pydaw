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

#ifndef PYDAW_PLUGIN_HEADER_INCLUDED
#define PYDAW_PLUGIN_HEADER_INCLUDED

#include <pthread.h>
#include <assert.h>
#include "../src/pydaw_files.h"

#define PYDAW_EVENT_NOTEON     0
#define PYDAW_EVENT_NOTEOFF    1
#define PYDAW_EVENT_PITCHBEND  2
#define PYDAW_EVENT_CONTROLLER 3

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*fp_queue_message)(char*, char*);

typedef float PYFX_Data;

typedef int PYFX_PortDescriptor;


typedef struct _PYFX_PortRangeHint {
  PYFX_Data DefaultValue;
  PYFX_Data LowerBound;
  PYFX_Data UpperBound;
} PYFX_PortRangeHint;

typedef void * PYFX_Handle;

// MIDI event
typedef struct
{
	int type;               /**< event type */
	int tick;	        /**< tick-time */
	unsigned int tv_sec;	/**< seconds */
	unsigned int tv_nsec;	/**< nanoseconds */
        int channel;		/**< channel number */
	int note;		/**< note */
	int velocity;		/**< velocity */
	int duration;		/**< duration until note-off;
                                 * only for #PYDAW_EVENT_NOTEON */
	int param;		/**< control parameter */
        float value;
        float start;
        float length;
        int plugin_index;
        int port;
} t_pydaw_seq_event;

typedef struct
{
    char path[2048];
    int uid;
    float * samples[2];
    float ratio_orig;
    int channels;
    int length;
    float sample_rate;
    int is_loaded;  //wav's are now loaded dynamically when they are first seen
    float host_sr;  //host sample-rate, cached here for easy access
}t_wav_pool_item;

typedef t_wav_pool_item * (*fp_get_wavpool_item_from_host)(int);

/* Descriptor for a Type of Plugin:

   This structure is used to describe a plugin type. It provides a
   number of functions to examine the type, instantiate it, link it to
   buffers and workspaces and to run it. */

typedef struct _PYFX_Descriptor {

  unsigned long UniqueID;

  char * Name;

  char * Maker;

  int PortCount;

  PYFX_PortDescriptor * PortDescriptors;

  /* This member indicates an array of range hints for each port (see
     above). Valid indices vary from 0 to PortCount-1. */
  PYFX_PortRangeHint * PortRangeHints;

  PYFX_Handle (*instantiate)(struct _PYFX_Descriptor * Descriptor,
          int SampleRate, fp_get_wavpool_item_from_host a_host_wavpool_func,
          int a_track_num, fp_queue_message);

   void (*connect_port)(PYFX_Handle Instance, int Port,
           PYFX_Data * DataLocation);

   /* Assign the audio buffer at DataLocation to index a_index
    */
   void (*connect_buffer)(PYFX_Handle Instance, int a_index,
           float * DataLocation);

  void (*cleanup)(PYFX_Handle Instance);

  /* Load the plugin state file at a_file_path
   */
  void (*load)(PYFX_Handle Instance, struct _PYFX_Descriptor * Descriptor,
          char * a_file_path);

void (*set_port_value)(PYFX_Handle Instance, int a_port, float a_value);

  /* When a panic message is sent, do whatever it takes to fix any stuck
   notes. */
  void (*panic)(PYFX_Handle Instance);

  /**
     * PYINST_API_Version
     *
     * This member indicates the DSSI API level used by this plugin.
     * If we're lucky, this will never be needed.  For now all plugins
     * must set it to 1.
     */
    int PYINST_API_Version;

    char *(*configure)(PYFX_Handle Instance,
		       char *Key,
		       char *Value,
                       pthread_spinlock_t * a_spinlock);

    void (*run_synth)(PYFX_Handle Instance, int SampleCount,
		      t_pydaw_seq_event *Events, int EventCount);

    /* Do anything like warming up oscillators, etc...  in preparation
     * for offline rendering.  This must be called after loading
     * the project.
     */
    void (*offline_render_prep)(PYFX_Handle Instance);

    /* Force any notes to off, etc...  and anything else you may want to
     * do when the transport stops
     */
    void (*on_stop)(PYFX_Handle Instance);

} PYFX_Descriptor;

typedef PYFX_Descriptor * (*PYFX_Descriptor_Function)(int Index);

#ifdef __cplusplus
}
#endif

void v_pydaw_ev_clear(t_pydaw_seq_event* a_event)
{
    a_event->type = -1;
    a_event->tick = 0;
}

void v_pydaw_ev_set_pitchbend(t_pydaw_seq_event* a_event,
        int a_channel, int a_value)
{
    a_event->type = PYDAW_EVENT_PITCHBEND;
    a_event->channel = a_channel;
    a_event->value = a_value;
}

void v_pydaw_ev_set_noteoff(t_pydaw_seq_event* a_event,
        int a_channel, int a_note, int a_velocity)
{
    a_event->type = PYDAW_EVENT_NOTEOFF;
    a_event->channel = a_channel;
    a_event->note = a_note;
    a_event->velocity = a_velocity;
}

void v_pydaw_ev_set_noteon(t_pydaw_seq_event* a_event,
        int a_channel, int a_note, int a_velocity)
{
    a_event->type = PYDAW_EVENT_NOTEON;
    a_event->channel = a_channel;
    a_event->note = a_note;
    a_event->velocity = a_velocity;
}

void v_pydaw_ev_set_controller(t_pydaw_seq_event* a_event,
        int a_channel, int a_cc_num, int a_value)
{
    a_event->type = PYDAW_EVENT_CONTROLLER;
    a_event->channel = a_channel;
    a_event->param = a_cc_num;
    a_event->value = a_value;
}

PYFX_Descriptor * pydaw_get_pyfx_descriptor(int a_uid, char * a_name,
        int a_port_count)
{
    PYFX_Descriptor *f_result =
            (PYFX_Descriptor*)malloc(sizeof(PYFX_Descriptor));

    f_result->UniqueID = a_uid;
    f_result->Name = a_name;
    f_result->Maker = "PyDAW Team";
    f_result->PortCount = a_port_count;

    f_result->PortDescriptors =
        (PYFX_PortDescriptor*)calloc(f_result->PortCount,
            sizeof(PYFX_PortDescriptor));

    f_result->PortRangeHints =
        (PYFX_PortRangeHint*)calloc(f_result->PortCount,
            sizeof(PYFX_PortRangeHint));

    return f_result;
}

void pydaw_set_pyfx_port(PYFX_Descriptor * a_desc, int a_port,
        float a_default, float a_min, float a_max)
{
    assert(a_port >= 0 && a_port < a_desc->PortCount);
    assert(!a_desc->PortDescriptors[a_port]);
    assert(a_min < a_max);
    assert(a_default >= a_min && a_default <= a_max);

    a_desc->PortDescriptors[a_port] = 1;
    a_desc->PortRangeHints[a_port].DefaultValue = a_default;
    a_desc->PortRangeHints[a_port].LowerBound = a_min;
    a_desc->PortRangeHints[a_port].UpperBound = a_max;
}



PYFX_Data g_pydaw_get_port_default(PYFX_Descriptor *plugin, int port)
{
    PYFX_PortRangeHint hint = plugin->PortRangeHints[port];
    assert(hint.DefaultValue <= hint.UpperBound &&
            hint.DefaultValue >= hint.LowerBound );
    return hint.DefaultValue;
}

float * g_pydaw_get_port_table(PYFX_Handle * handle,
        PYFX_Descriptor * descriptor)
{
    float * pluginControlIns;
    int j;

    int f_i = 0;

    if(posix_memalign((void**)(&pluginControlIns), 16,
        (sizeof(float) * descriptor->PortCount)) != 0)
    {
        return 0;
    }

    f_i = 0;
    while(f_i < descriptor->PortCount)
    {
        pluginControlIns[f_i] = 0.0f;
        f_i++;
    }

    for (j = 0; j < descriptor->PortCount; j++)
    {
        PYFX_PortDescriptor pod = descriptor->PortDescriptors[j];

        if(pod)
        {
            pluginControlIns[j] = g_pydaw_get_port_default(descriptor, j);

            descriptor->connect_port(handle, j, &pluginControlIns[j]);
        }
    }

    return pluginControlIns;
}

void pydaw_generic_file_loader(PYFX_Handle Instance,
        PYFX_Descriptor * Descriptor, char * a_path, float * a_table)
{
    t_2d_char_array * f_2d_array = g_get_2d_array_from_file(a_path,
                PYDAW_LARGE_STRING);

    while(1)
    {
        char * f_key = c_iterate_2d_char_array(f_2d_array);

        if(f_2d_array->eof)
        {
            free(f_key);
            break;
        }

        assert(strcmp(f_key, ""));

        if(f_key[0] == 'c')
        {
            char * f_config_key = c_iterate_2d_char_array(f_2d_array);
            char * f_value =
                c_iterate_2d_char_array_to_next_line(f_2d_array);

            char * message =
                Descriptor->configure(Instance, f_config_key, f_value, 0);

            if (message)
            {
                //TODO:  delete this
                free(message);
            }

        }
        else
        {
            char * f_value =
                c_iterate_2d_char_array_to_next_line(f_2d_array);
            int f_port_key = atoi(f_key);
            float f_port_value = atof(f_value);

            assert(f_port_key >= 0);
            assert(f_port_key <= Descriptor->PortCount);

            a_table[f_port_key] = f_port_value;
        }
    }

    g_free_2d_char_array(f_2d_array);

}


#endif /* PYDAW_PLUGIN_HEADER_INCLUDED */
