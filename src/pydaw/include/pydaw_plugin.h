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

#ifdef __cplusplus
extern "C" {
#endif

#define PYDAW_EVENT_NOTEON     0
#define PYDAW_EVENT_NOTEOFF    1
#define PYDAW_EVENT_PITCHBEND  2
#define PYDAW_EVENT_CONTROLLER 3

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
	int duration;		/**< duration until note-off; only for #PYDAW_EVENT_NOTEON */
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

void v_pydaw_ev_clear(t_pydaw_seq_event* a_event)
{
    a_event->type = -1;
    a_event->tick = 0;
}

void v_pydaw_ev_set_pitchbend(t_pydaw_seq_event* a_event, int a_channel, int a_value)
{
    a_event->type = PYDAW_EVENT_PITCHBEND;
    a_event->channel = a_channel;
    a_event->value = a_value;
}

void v_pydaw_ev_set_noteoff(t_pydaw_seq_event* a_event, int a_channel, int a_note, int a_velocity)
{
    a_event->type = PYDAW_EVENT_NOTEOFF;
    a_event->channel = a_channel;
    a_event->note = a_note;
    a_event->velocity = a_velocity;
}

void v_pydaw_ev_set_noteon(t_pydaw_seq_event* a_event, int a_channel, int a_note, int a_velocity)
{
    a_event->type = PYDAW_EVENT_NOTEON;
    a_event->channel = a_channel;
    a_event->note = a_note;
    a_event->velocity = a_velocity;
}

void v_pydaw_ev_set_controller(t_pydaw_seq_event* a_event, int a_channel, int a_cc_num, int a_value)
{
    a_event->type = PYDAW_EVENT_CONTROLLER;
    a_event->channel = a_channel;
    a_event->param = a_cc_num;
    a_event->value = a_value;
}


typedef t_wav_pool_item * (*fp_get_wavpool_item_from_host)(int);

/*****************************************************************************/

/* Fundamental data type passed in and out of plugin. This data type
   is used to communicate audio samples and control values. It is
   assumed that the plugin will work sensibly given any numeric input
   value although it may have a preferred range (see hints below).

   For audio it is generally assumed that 1.0f is the `0dB' reference
   amplitude and is a `normal' signal level. */

typedef float PYFX_Data;

/*****************************************************************************/

/* Plugin Ports:
 * Set this to 1 if the port is valid, 0 if not
 *  */

typedef int PYFX_PortDescriptor;


typedef struct _PYFX_PortRangeHint {

  /* Hints about the port. */
  //PYFX_PortRangeHintDescriptor HintDescriptor;

  PYFX_Data DefaultValue;

  /* Meaningful when hint PYFX_HINT_BOUNDED_BELOW is active. When
     PYFX_HINT_SAMPLE_RATE is also active then this value should be
     multiplied by the relevant sample rate. */
  PYFX_Data LowerBound;

  /* Meaningful when hint PYFX_HINT_BOUNDED_ABOVE is active. When
     PYFX_HINT_SAMPLE_RATE is also active then this value should be
     multiplied by the relevant sample rate. */
  PYFX_Data UpperBound;

} PYFX_PortRangeHint;

/*****************************************************************************/

/* Plugin Handles:

   This plugin handle indicates a particular instance of the plugin
   concerned. It is valid to compare this to NULL (0 for C++) but
   otherwise the host should not attempt to interpret it. The plugin
   may use it to reference internal instance data. */

typedef void * PYFX_Handle;

/*****************************************************************************/

/* Descriptor for a Type of Plugin:

   This structure is used to describe a plugin type. It provides a
   number of functions to examine the type, instantiate it, link it to
   buffers and workspaces and to run it. */

typedef struct _PYFX_Descriptor {

  /* This numeric identifier indicates the plugin type
     uniquely. Plugin programmers may reserve ranges of IDs from a
     central body to avoid clashes. Hosts may assume that IDs are
     below 0x1000000. */
  unsigned long UniqueID;

  /* This member points to the null-terminated name of the plugin
     (e.g. "Sine Oscillator"). */
  char * Name;

  /* This member points to the null-terminated string indicating the
     maker of the plugin. This can be an empty string but not NULL. */
  char * Maker;

  /* This member points to the null-terminated string indicating any
     copyright applying to the plugin. If no Copyright applies the
     string "None" should be used. */
  char * Copyright;

  /* This indicates the number of ports (input AND output) present on
     the plugin. */
  int PortCount;

  /* This member indicates an array of port descriptors. Valid indices
     vary from 0 to PortCount-1. */
  PYFX_PortDescriptor * PortDescriptors;

  /* This member indicates an array of range hints for each port (see
     above). Valid indices vary from 0 to PortCount-1. */
  PYFX_PortRangeHint * PortRangeHints;

  /* This member is a function pointer that instantiates a plugin. A
     handle is returned indicating the new plugin instance. The
     instantiation function accepts a sample rate as a parameter. The
     plugin descriptor from which this instantiate function was found
     must also be passed. This function must return NULL if
     instantiation fails.

     Note that instance initialisation should generally occur in
     activate() rather than here. */
  PYFX_Handle (*instantiate)(struct _PYFX_Descriptor * Descriptor,
          int SampleRate, fp_get_wavpool_item_from_host a_host_wavpool_func,
          int a_track_num);

  /* This member is a function pointer that connects a port on an
     instantiated plugin to a memory location at which a block of data
     for the port will be read/written. The data location is expected
     to be an array of PYFX_Data for audio ports or a single
     PYFX_Data value for control ports. Memory issues will be
     managed by the host. The plugin must read/write the data at these
     locations every time run() or run_adding() is called and the data
     present at the time of this connection call should not be
     considered meaningful.

     connect_port() may be called more than once for a plugin instance
     to allow the host to change the buffers that the plugin is
     reading or writing. These calls may be made before or after
     activate() or deactivate() calls.

     connect_port() must be called at least once for each port before
     run() or run_adding() is called. When working with blocks of
     PYFX_Data the plugin should pay careful attention to the block
     size passed to the run function as the block allocated may only
     just be large enough to contain the block of samples.

     Plugin writers should be aware that the host may elect to use the
     same buffer for more than one port and even use the same buffer
     for both input and output (see PYFX_PROPERTY_INPLACE_BROKEN).
     However, overlapped buffers or use of a single buffer for both
     audio and control data may result in unexpected behaviour. */
   void (*connect_port)(PYFX_Handle Instance, int Port, PYFX_Data * DataLocation);

   /* Assign the audio buffer at DataLocation to index a_index
    */
   void (*connect_buffer)(PYFX_Handle Instance, int a_index, float * DataLocation);

  /* This member is a function pointer that initialises a plugin
     instance and activates it for use. This is separated from
     instantiate() to aid real-time support and so that hosts can
     reinitialise a plugin instance by calling deactivate() and then
     activate(). In this case the plugin instance must reset all state
     information dependent on the history of the plugin instance
     except for any data locations provided by connect_port() and any
     gain set by set_run_adding_gain(). If there is nothing for
     activate() to do then the plugin writer may provide a NULL rather
     than an empty function.

     When present, hosts must call this function once before run() (or
     run_adding()) is called for the first time. This call should be
     made as close to the run() call as possible and indicates to
     real-time plugins that they are now live. Plugins should not rely
     on a prompt call to run() after activate(). activate() may not be
     called again unless deactivate() is called first. Note that
     connect_port() may be called before or after a call to
     activate(). */
  void (*activate)(PYFX_Handle Instance, float * a_port_table);

  /* This is the counterpart to activate() (see above). If there is
     nothing for deactivate() to do then the plugin writer may provide
     a NULL rather than an empty function.

     Hosts must deactivate all activated units after they have been
     run() (or run_adding()) for the last time. This call should be
     made as close to the last run() call as possible and indicates to
     real-time plugins that they are no longer live. Plugins should
     not rely on prompt deactivation. Note that connect_port() may be
     called before or after a call to deactivate().

     Deactivation is not similar to pausing as the plugin instance
     will be reinitialised when activate() is called to reuse it. */
  void (*deactivate)(PYFX_Handle Instance);

  /* Once an instance of a plugin has been finished with it can be
     deleted using the following function. The instance handle passed
     ceases to be valid after this call.

     If activate() was called for a plugin instance then a
     corresponding call to deactivate() must be made before cleanup()
     is called. */
  void (*cleanup)(PYFX_Handle Instance);


  /* When a panic message is sent, do whatever it takes to fix any stuck
   notes. */
  void (*panic)(PYFX_Handle Instance);

} PYFX_Descriptor;

/**********************************************************************/

/* Accessing a Plugin: */

/* The exact mechanism by which plugins are loaded is host-dependent,
   however all most hosts will need to know is the name of shared
   object file containing the plugin types. To allow multiple hosts to
   share plugin types, hosts may wish to check for environment
   variable PYFX_PATH. If present, this should contain a
   colon-separated path indicating directories that should be searched
   (in order) when loading plugin types.

   A plugin programmer must include a function called
   "ladspa_descriptor" with the following function prototype within
   the shared object file. This function will have C-style linkage (if
   you are using C++ this is taken care of by the `extern "C"' clause
   at the top of the file).

   A host will find the plugin shared object file by one means or
   another, find the ladspa_descriptor() function, call it, and
   proceed from there.

   Plugin types are accessed by index (not ID) using values from 0
   upwards. Out of range indexes must result in this function
   returning NULL, so the plugin count can be determined by checking
   for the least index that results in NULL being returned. */

PYFX_Descriptor * ladspa_descriptor(int Index);

/* Datatype corresponding to the ladspa_descriptor() function. */
typedef PYFX_Descriptor *
(*PYFX_Descriptor_Function)(int Index);

/**********************************************************************/


typedef struct _PYINST_Descriptor {

    /**
     * PYINST_API_Version
     *
     * This member indicates the DSSI API level used by this plugin.
     * If we're lucky, this will never be needed.  For now all plugins
     * must set it to 1.
     */
    int PYINST_API_Version;

    /**
     * PYFX_Plugin
     *
     * A DSSI synth plugin consists of a LADSPA plugin plus an
     * additional framework for controlling program settings and
     * transmitting MIDI events.  A plugin must fully implement the
     * LADSPA descriptor fields as well as the required LADSPA
     * functions including instantiate() and (de)activate().  It
     * should also implement run(), with the same behaviour as if
     * run_synth() (below) were called with no synth events.
     *
     * In order to instantiate a synth the host calls the LADSPA
     * instantiate function, passing in this PYFX_Descriptor
     * pointer.  The returned PYFX_Handle is used as the argument
     * for the DSSI functions below as well as for the LADSPA ones.
     */
    PYFX_Descriptor *PYFX_Plugin;

    /**
     * configure()
     *
     * This member is a function pointer that sends a piece of
     * configuration data to the plugin.  The key argument specifies
     * some aspect of the synth's configuration that is to be changed,
     * and the value argument specifies a new value for it.  A plugin
     * that does not require this facility at all may set this member
     * to NULL.
     *
     * This call is intended to set some session-scoped aspect of a
     * plugin's behaviour, for example to tell the plugin to load
     * sample data from a particular file.  The plugin should act
     * immediately on the request.  The call should return NULL on
     * success, or an error string that may be shown to the user.  The
     * host will free the returned value after use if it is non-NULL.
     *
     * Calls to configure() are not automated as timed events.
     * Instead, a host should remember the last value associated with
     * each key passed to configure() during a given session for a
     * given plugin instance, and should call configure() with the
     * correct value for each key the next time it instantiates the
     * "same" plugin instance, for example on reloading a project in
     * which the plugin was used before.  Plugins should note that a
     * host may typically instantiate a plugin multiple times with the
     * same configuration values, and should share data between
     * instances where practical.
     *
     * Calling configure() completely invalidates the program and bank
     * information last obtained from the plugin.
     */
    char *(*configure)(PYFX_Handle Instance,
		       char *Key,
		       char *Value,
                       pthread_mutex_t * a_mutex);

    /**
     * run_synth()
     *
     * This member is a function pointer that runs a synth for a
     * block.  This is identical in function to the LADSPA run()
     * function, except that it also supplies events to the synth.
     *
     * A plugin may provide this function, run_multiple_synths() (see
     * below), both, or neither (if it is not in fact a synth).  A
     * plugin that does not provide this function must set this member
     * to NULL.  Authors of synth plugins are encouraged to provide
     * this function if at all possible.
     *
     * The Events pointer points to a block of EventCount ALSA
     * sequencer events, which is used to communicate MIDI and related
     * events to the synth.  Each event is timestamped relative to the
     * start of the block, (mis)using the ALSA "tick time" field as a
     * frame count. The host is responsible for ensuring that events
     * with differing timestamps are already ordered by time.
     *
     * See also the notes on activation, port connection etc in
     * ladpsa.h, in the context of the LADSPA run() function.
     *
     * Note Events
     * ~~~~~~~~~~~
     * There are two minor requirements aimed at making the plugin
     * writer's life as simple as possible:
     *
     * 1. A host must never send events of type SND_SEQ_EVENT_NOTE.
     * Notes should always be sent as separate SND_SEQ_EVENT_NOTE_ON
     * and NOTE_OFF events.  A plugin should discard any one-point
     * NOTE events it sees.
     *
     * 2. A host must not attempt to switch notes off by sending
     * zero-velocity NOTE_ON events.  It should always send true
     * NOTE_OFFs.  It is the host's responsibility to remap events in
     * cases where an external MIDI source has sent it zero-velocity
     * NOTE_ONs.
     */
    void (*run_synth)(PYFX_Handle    Instance,
		      int    SampleCount,
		      t_pydaw_seq_event *Events,
		      int    EventCount);

    /* Do anything like warming up oscillators, etc...  in preparation
     * for offline rendering.  This must be called after loading
     * the project.
     */
    void (*offline_render_prep)(PYFX_Handle Instance);

} PYINST_Descriptor;

/**
 * DSSI supports a plugin discovery method similar to that of LADSPA:
 *
 * - DSSI hosts may wish to locate DSSI plugin shared object files by
 *    searching the paths contained in the PYINST_PATH and PYFX_PATH
 *    environment variables, if they are present.  Both are expected
 *    to be colon-separated lists of directories to be searched (in
 *    order), and PYINST_PATH should be searched first if both variables
 *    are set.
 *
 * - Each shared object file containing DSSI plugins must include a
 *   function dssi_descriptor(), with the following function prototype
 *   and C-style linkage.  Hosts may enumerate the plugin types
 *   available in the shared object file by repeatedly calling
 *   this function with successive Index values (beginning from 0),
 *   until a return value of NULL indicates no more plugin types are
 *   available.  Each non-NULL return is the PYINST_Descriptor
 *   of a distinct plugin type.
 */

PYINST_Descriptor *dssi_descriptor(int Index);

typedef PYINST_Descriptor *(*PYINST_Descriptor_Function)(int Index);

PYFX_Descriptor * pydaw_get_pyfx_descriptor(int a_uid, char * a_name,
        int a_port_count)
{
    PYFX_Descriptor *f_result =
            (PYFX_Descriptor*)malloc(sizeof(PYFX_Descriptor));

    f_result->UniqueID = a_uid;
    f_result->Name = a_name;
    f_result->Maker = "PyDAW Team";
    f_result->Copyright = "GNU GPL v3";
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

#ifdef __cplusplus
}
#endif

#endif /* PYDAW_PLUGIN_INCLUDED */
