/* -*- c-basic-offset: 4 -*- */

/* pydaw_plugin.h

   PyDAW Draft Plugin Format, pre-1.0
   Copyright (c) 2013 Jeff Hubbard
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1 of
   the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   Lesser General Public License for more details.   
*/

#ifndef PYDAW_PLUGIN_HEADER_INCLUDED
#define PYDAW_PLUGIN_HEADER_INCLUDED

#include <alsa/seq_event.h>

#define LADSPA_VERSION "1.1"
#define LADSPA_VERSION_MAJOR 1
#define LADSPA_VERSION_MINOR 1

#define DSSI_VERSION "1.0"
#define DSSI_VERSION_MAJOR 1
#define DSSI_VERSION_MINOR 0

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/

/* Fundamental data type passed in and out of plugin. This data type
   is used to communicate audio samples and control values. It is
   assumed that the plugin will work sensibly given any numeric input
   value although it may have a preferred range (see hints below). 

   For audio it is generally assumed that 1.0f is the `0dB' reference
   amplitude and is a `normal' signal level. */

typedef float LADSPA_Data;

/*****************************************************************************/

/* Special Plugin Properties: 
 
   Optional features of the plugin type are encapsulated in the
   LADSPA_Properties type. This is assembled by ORing individual
   properties together. */

typedef int LADSPA_Properties;

/* Property LADSPA_PROPERTY_REALTIME indicates that the plugin has a
   real-time dependency (e.g. listens to a MIDI device) and so its
   output must not be cached or subject to significant latency. */
#define LADSPA_PROPERTY_REALTIME        0x1

/* Property LADSPA_PROPERTY_INPLACE_BROKEN indicates that the plugin
   may cease to work correctly if the host elects to use the same data
   location for both input and output (see connect_port()). This
   should be avoided as enabling this flag makes it impossible for
   hosts to use the plugin to process audio `in-place.' */
#define LADSPA_PROPERTY_INPLACE_BROKEN  0x2

/* Property LADSPA_PROPERTY_HARD_RT_CAPABLE indicates that the plugin
   is capable of running not only in a conventional host but also in a
   `hard real-time' environment. To qualify for this the plugin must
   satisfy all of the following:

   (1) The plugin must not use malloc(), free() or other heap memory
   management within its run() or run_adding() functions. All new
   memory used in run() must be managed via the stack. These
   restrictions only apply to the run() function.

   (2) The plugin will not attempt to make use of any library
   functions with the exceptions of functions in the ANSI standard C
   and C maths libraries, which the host is expected to provide.

   (3) The plugin will not access files, devices, pipes, sockets, IPC
   or any other mechanism that might result in process or thread
   blocking.
      
   (4) The plugin will take an amount of time to execute a run() or
   run_adding() call approximately of form (A+B*SampleCount) where A
   and B depend on the machine and host in use. This amount of time
   may not depend on input signals or plugin state. The host is left
   the responsibility to perform timings to estimate upper bounds for
   A and B. */
#define LADSPA_PROPERTY_HARD_RT_CAPABLE 0x4

#define LADSPA_IS_REALTIME(x)        ((x) & LADSPA_PROPERTY_REALTIME)
#define LADSPA_IS_INPLACE_BROKEN(x)  ((x) & LADSPA_PROPERTY_INPLACE_BROKEN)
#define LADSPA_IS_HARD_RT_CAPABLE(x) ((x) & LADSPA_PROPERTY_HARD_RT_CAPABLE)

/*****************************************************************************/

/* Plugin Ports: 

   Plugins have `ports' that are inputs or outputs for audio or
   data. Ports can communicate arrays of LADSPA_Data (for audio
   inputs/outputs) or single LADSPA_Data values (for control
   input/outputs). This information is encapsulated in the
   LADSPA_PortDescriptor type which is assembled by ORing individual
   properties together.

   Note that a port must be an input or an output port but not both
   and that a port must be a control or audio port but not both. */

typedef int LADSPA_PortDescriptor;

/* Property LADSPA_PORT_INPUT indicates that the port is an input. */
#define LADSPA_PORT_INPUT   0x1

/* Property LADSPA_PORT_OUTPUT indicates that the port is an output. */
#define LADSPA_PORT_OUTPUT  0x2

/* Property LADSPA_PORT_CONTROL indicates that the port is a control
   port. */
#define LADSPA_PORT_CONTROL 0x4

/* Property LADSPA_PORT_AUDIO indicates that the port is a audio
   port. */
#define LADSPA_PORT_AUDIO   0x8

#define LADSPA_IS_PORT_INPUT(x)   ((x) & LADSPA_PORT_INPUT)
#define LADSPA_IS_PORT_OUTPUT(x)  ((x) & LADSPA_PORT_OUTPUT)
#define LADSPA_IS_PORT_CONTROL(x) ((x) & LADSPA_PORT_CONTROL)
#define LADSPA_IS_PORT_AUDIO(x)   ((x) & LADSPA_PORT_AUDIO)

/*****************************************************************************/

/* Plugin Port Range Hints: 

   The host may wish to provide a representation of data entering or
   leaving a plugin (e.g. to generate a GUI automatically). To make
   this more meaningful, the plugin should provide `hints' to the host
   describing the usual values taken by the data.
   
   Note that these are only hints. The host may ignore them and the
   plugin must not assume that data supplied to it is meaningful. If
   the plugin receives invalid input data it is expected to continue
   to run without failure and, where possible, produce a sensible
   output (e.g. a high-pass filter given a negative cutoff frequency
   might switch to an all-pass mode).
    
   Hints are meaningful for all input and output ports but hints for
   input control ports are expected to be particularly useful.
   
   More hint information is encapsulated in the
   LADSPA_PortRangeHintDescriptor type which is assembled by ORing
   individual hint types together. Hints may require further
   LowerBound and UpperBound information.

   All the hint information for a particular port is aggregated in the
   LADSPA_PortRangeHint structure. */

typedef int LADSPA_PortRangeHintDescriptor;

/* Hint LADSPA_HINT_BOUNDED_BELOW indicates that the LowerBound field
   of the LADSPA_PortRangeHint should be considered meaningful. The
   value in this field should be considered the (inclusive) lower
   bound of the valid range. If LADSPA_HINT_SAMPLE_RATE is also
   specified then the value of LowerBound should be multiplied by the
   sample rate. */
#define LADSPA_HINT_BOUNDED_BELOW   0x1

/* Hint LADSPA_HINT_BOUNDED_ABOVE indicates that the UpperBound field
   of the LADSPA_PortRangeHint should be considered meaningful. The
   value in this field should be considered the (inclusive) upper
   bound of the valid range. If LADSPA_HINT_SAMPLE_RATE is also
   specified then the value of UpperBound should be multiplied by the
   sample rate. */
#define LADSPA_HINT_BOUNDED_ABOVE   0x2

/* Hint LADSPA_HINT_TOGGLED indicates that the data item should be
   considered a Boolean toggle. Data less than or equal to zero should
   be considered `off' or `false,' and data above zero should be
   considered `on' or `true.' LADSPA_HINT_TOGGLED may not be used in
   conjunction with any other hint except LADSPA_HINT_DEFAULT_0 or
   LADSPA_HINT_DEFAULT_1. */
#define LADSPA_HINT_TOGGLED         0x4

/* Hint LADSPA_HINT_SAMPLE_RATE indicates that any bounds specified
   should be interpreted as multiples of the sample rate. For
   instance, a frequency range from 0Hz to the Nyquist frequency (half
   the sample rate) could be requested by this hint in conjunction
   with LowerBound = 0 and UpperBound = 0.5. Hosts that support bounds
   at all must support this hint to retain meaning. */
#define LADSPA_HINT_SAMPLE_RATE     0x8

/* Hint LADSPA_HINT_LOGARITHMIC indicates that it is likely that the
   user will find it more intuitive to view values using a logarithmic
   scale. This is particularly useful for frequencies and gains. */
#define LADSPA_HINT_LOGARITHMIC     0x10

/* Hint LADSPA_HINT_INTEGER indicates that a user interface would
   probably wish to provide a stepped control taking only integer
   values. Any bounds set should be slightly wider than the actual
   integer range required to avoid floating point rounding errors. For
   instance, the integer set {0,1,2,3} might be described as [-0.1,
   3.1]. */
#define LADSPA_HINT_INTEGER         0x20

/* The various LADSPA_HINT_HAS_DEFAULT_* hints indicate a `normal'
   value for the port that is sensible as a default. For instance,
   this value is suitable for use as an initial value in a user
   interface or as a value the host might assign to a control port
   when the user has not provided one. Defaults are encoded using a
   mask so only one default may be specified for a port. Some of the
   hints make use of lower and upper bounds, in which case the
   relevant bound or bounds must be available and
   LADSPA_HINT_SAMPLE_RATE must be applied as usual. The resulting
   default must be rounded if LADSPA_HINT_INTEGER is present. Default
   values were introduced in LADSPA v1.1. */
#define LADSPA_HINT_DEFAULT_MASK    0x3C0

/* This default values indicates that no default is provided. */
#define LADSPA_HINT_DEFAULT_NONE    0x0

/* This default hint indicates that the suggested lower bound for the
   port should be used. */
#define LADSPA_HINT_DEFAULT_MINIMUM 0x40

/* This default hint indicates that a low value between the suggested
   lower and upper bounds should be chosen. For ports with
   LADSPA_HINT_LOGARITHMIC, this should be exp(log(lower) * 0.75 +
   log(upper) * 0.25). Otherwise, this should be (lower * 0.75 + upper
   * 0.25). */
#define LADSPA_HINT_DEFAULT_LOW     0x80

/* This default hint indicates that a middle value between the
   suggested lower and upper bounds should be chosen. For ports with
   LADSPA_HINT_LOGARITHMIC, this should be exp(log(lower) * 0.5 +
   log(upper) * 0.5). Otherwise, this should be (lower * 0.5 + upper *
   0.5). */
#define LADSPA_HINT_DEFAULT_MIDDLE  0xC0

/* This default hint indicates that a high value between the suggested
   lower and upper bounds should be chosen. For ports with
   LADSPA_HINT_LOGARITHMIC, this should be exp(log(lower) * 0.25 +
   log(upper) * 0.75). Otherwise, this should be (lower * 0.25 + upper
   * 0.75). */
#define LADSPA_HINT_DEFAULT_HIGH    0x100

/* This default hint indicates that the suggested upper bound for the
   port should be used. */
#define LADSPA_HINT_DEFAULT_MAXIMUM 0x140

/* This default hint indicates that the number 0 should be used. Note
   that this default may be used in conjunction with
   LADSPA_HINT_TOGGLED. */
#define LADSPA_HINT_DEFAULT_0       0x200

/* This default hint indicates that the number 1 should be used. Note
   that this default may be used in conjunction with
   LADSPA_HINT_TOGGLED. */
#define LADSPA_HINT_DEFAULT_1       0x240

/* This default hint indicates that the number 100 should be used. */
#define LADSPA_HINT_DEFAULT_100     0x280

/* This default hint indicates that the Hz frequency of `concert A'
   should be used. This will be 440 unless the host uses an unusual
   tuning convention, in which case it may be within a few Hz. */
#define LADSPA_HINT_DEFAULT_440     0x2C0

#define LADSPA_IS_HINT_BOUNDED_BELOW(x)   ((x) & LADSPA_HINT_BOUNDED_BELOW)
#define LADSPA_IS_HINT_BOUNDED_ABOVE(x)   ((x) & LADSPA_HINT_BOUNDED_ABOVE)
#define LADSPA_IS_HINT_TOGGLED(x)         ((x) & LADSPA_HINT_TOGGLED)
#define LADSPA_IS_HINT_SAMPLE_RATE(x)     ((x) & LADSPA_HINT_SAMPLE_RATE)
#define LADSPA_IS_HINT_LOGARITHMIC(x)     ((x) & LADSPA_HINT_LOGARITHMIC)
#define LADSPA_IS_HINT_INTEGER(x)         ((x) & LADSPA_HINT_INTEGER)

#define LADSPA_IS_HINT_HAS_DEFAULT(x)     ((x) & LADSPA_HINT_DEFAULT_MASK)
#define LADSPA_IS_HINT_DEFAULT_MINIMUM(x) (((x) & LADSPA_HINT_DEFAULT_MASK)   \
                                           == LADSPA_HINT_DEFAULT_MINIMUM)
#define LADSPA_IS_HINT_DEFAULT_LOW(x)     (((x) & LADSPA_HINT_DEFAULT_MASK)   \
                                           == LADSPA_HINT_DEFAULT_LOW)
#define LADSPA_IS_HINT_DEFAULT_MIDDLE(x)  (((x) & LADSPA_HINT_DEFAULT_MASK)   \
                                           == LADSPA_HINT_DEFAULT_MIDDLE)
#define LADSPA_IS_HINT_DEFAULT_HIGH(x)    (((x) & LADSPA_HINT_DEFAULT_MASK)   \
                                           == LADSPA_HINT_DEFAULT_HIGH)
#define LADSPA_IS_HINT_DEFAULT_MAXIMUM(x) (((x) & LADSPA_HINT_DEFAULT_MASK)   \
                                           == LADSPA_HINT_DEFAULT_MAXIMUM)
#define LADSPA_IS_HINT_DEFAULT_0(x)       (((x) & LADSPA_HINT_DEFAULT_MASK)   \
                                           == LADSPA_HINT_DEFAULT_0)
#define LADSPA_IS_HINT_DEFAULT_1(x)       (((x) & LADSPA_HINT_DEFAULT_MASK)   \
                                           == LADSPA_HINT_DEFAULT_1)
#define LADSPA_IS_HINT_DEFAULT_100(x)     (((x) & LADSPA_HINT_DEFAULT_MASK)   \
                                           == LADSPA_HINT_DEFAULT_100)
#define LADSPA_IS_HINT_DEFAULT_440(x)     (((x) & LADSPA_HINT_DEFAULT_MASK)   \
                                            == LADSPA_HINT_DEFAULT_440)

typedef struct _LADSPA_PortRangeHint {

  /* Hints about the port. */
  LADSPA_PortRangeHintDescriptor HintDescriptor;

  /* Meaningful when hint LADSPA_HINT_BOUNDED_BELOW is active. When
     LADSPA_HINT_SAMPLE_RATE is also active then this value should be
     multiplied by the relevant sample rate. */
  LADSPA_Data LowerBound;

  /* Meaningful when hint LADSPA_HINT_BOUNDED_ABOVE is active. When
     LADSPA_HINT_SAMPLE_RATE is also active then this value should be
     multiplied by the relevant sample rate. */
  LADSPA_Data UpperBound;

} LADSPA_PortRangeHint;

/*****************************************************************************/

/* Plugin Handles: 

   This plugin handle indicates a particular instance of the plugin
   concerned. It is valid to compare this to NULL (0 for C++) but
   otherwise the host should not attempt to interpret it. The plugin
   may use it to reference internal instance data. */

typedef void * LADSPA_Handle;

/*****************************************************************************/

/* Descriptor for a Type of Plugin: 

   This structure is used to describe a plugin type. It provides a
   number of functions to examine the type, instantiate it, link it to
   buffers and workspaces and to run it. */

typedef struct _LADSPA_Descriptor { 

  /* This numeric identifier indicates the plugin type
     uniquely. Plugin programmers may reserve ranges of IDs from a
     central body to avoid clashes. Hosts may assume that IDs are
     below 0x1000000. */
  unsigned long UniqueID;

  /* This identifier can be used as a unique, case-sensitive
     identifier for the plugin type within the plugin file. Plugin
     types should be identified by file and label rather than by index
     or plugin name, which may be changed in new plugin
     versions. Labels must not contain white-space characters. */
  const char * Label;

  /* This indicates a number of properties of the plugin. */
  LADSPA_Properties Properties;

  /* This member points to the null-terminated name of the plugin
     (e.g. "Sine Oscillator"). */
  const char * Name;

  /* This member points to the null-terminated string indicating the
     maker of the plugin. This can be an empty string but not NULL. */
  const char * Maker;

  /* This member points to the null-terminated string indicating any
     copyright applying to the plugin. If no Copyright applies the
     string "None" should be used. */
  const char * Copyright;

  /* This indicates the number of ports (input AND output) present on
     the plugin. */
  int PortCount;

  /* This member indicates an array of port descriptors. Valid indices
     vary from 0 to PortCount-1. */
  const LADSPA_PortDescriptor * PortDescriptors;

  /* This member indicates an array of null-terminated strings
     describing ports (e.g. "Frequency (Hz)"). Valid indices vary from
     0 to PortCount-1. */
  const char * const * PortNames;

  /* This member indicates an array of range hints for each port (see
     above). Valid indices vary from 0 to PortCount-1. */
  const LADSPA_PortRangeHint * PortRangeHints;

  /* This may be used by the plugin developer to pass any custom
     implementation data into an instantiate call. It must not be used
     or interpreted by the host. It is expected that most plugin
     writers will not use this facility as LADSPA_Handle should be
     used to hold instance data. */
  void * ImplementationData;

  /* This member is a function pointer that instantiates a plugin. A
     handle is returned indicating the new plugin instance. The
     instantiation function accepts a sample rate as a parameter. The
     plugin descriptor from which this instantiate function was found
     must also be passed. This function must return NULL if
     instantiation fails. 

     Note that instance initialisation should generally occur in
     activate() rather than here. */
  LADSPA_Handle (*instantiate)(const struct _LADSPA_Descriptor * Descriptor, int SampleRate);

  /* This member is a function pointer that connects a port on an
     instantiated plugin to a memory location at which a block of data
     for the port will be read/written. The data location is expected
     to be an array of LADSPA_Data for audio ports or a single
     LADSPA_Data value for control ports. Memory issues will be
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
     LADSPA_Data the plugin should pay careful attention to the block
     size passed to the run function as the block allocated may only
     just be large enough to contain the block of samples.

     Plugin writers should be aware that the host may elect to use the
     same buffer for more than one port and even use the same buffer
     for both input and output (see LADSPA_PROPERTY_INPLACE_BROKEN).
     However, overlapped buffers or use of a single buffer for both
     audio and control data may result in unexpected behaviour. */
   void (*connect_port)(LADSPA_Handle Instance, int Port, LADSPA_Data * DataLocation);

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
  void (*activate)(LADSPA_Handle Instance);

  /* This method is a function pointer that runs an instance of a
     plugin for a block. Two parameters are required: the first is a
     handle to the particular instance to be run and the second
     indicates the block size (in samples) for which the plugin
     instance may run.

     Note that if an activate() function exists then it must be called
     before run() or run_adding(). If deactivate() is called for a
     plugin instance then the plugin instance may not be reused until
     activate() has been called again.

     If the plugin has the property LADSPA_PROPERTY_HARD_RT_CAPABLE
     then there are various things that the plugin should not do
     within the run() or run_adding() functions (see above). */
  void (*run)(LADSPA_Handle Instance, int SampleCount);

  /* This method is a function pointer that runs an instance of a
     plugin for a block. This has identical behaviour to run() except
     in the way data is output from the plugin. When run() is used,
     values are written directly to the memory areas associated with
     the output ports. However when run_adding() is called, values
     must be added to the values already present in the memory
     areas. Furthermore, output values written must be scaled by the
     current gain set by set_run_adding_gain() (see below) before
     addition.

     run_adding() is optional. When it is not provided by a plugin,
     this function pointer must be set to NULL. When it is provided,
     the function set_run_adding_gain() must be provided also. */
  void (*run_adding)(LADSPA_Handle Instance, int SampleCount);

  /* This method is a function pointer that sets the output gain for
     use when run_adding() is called (see above). If this function is
     never called the gain is assumed to default to 1. Gain
     information should be retained when activate() or deactivate()
     are called.

     This function should be provided by the plugin if and only if the
     run_adding() function is provided. When it is absent this
     function pointer must be set to NULL. */
  void (*set_run_adding_gain)(LADSPA_Handle Instance,
                              LADSPA_Data   Gain);

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
  void (*deactivate)(LADSPA_Handle Instance);

  /* Once an instance of a plugin has been finished with it can be
     deleted using the following function. The instance handle passed
     ceases to be valid after this call.
  
     If activate() was called for a plugin instance then a
     corresponding call to deactivate() must be made before cleanup()
     is called. */
  void (*cleanup)(LADSPA_Handle Instance);

} LADSPA_Descriptor;

/**********************************************************************/

/* Accessing a Plugin: */

/* The exact mechanism by which plugins are loaded is host-dependent,
   however all most hosts will need to know is the name of shared
   object file containing the plugin types. To allow multiple hosts to
   share plugin types, hosts may wish to check for environment
   variable LADSPA_PATH. If present, this should contain a
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

const LADSPA_Descriptor * ladspa_descriptor(int Index);

/* Datatype corresponding to the ladspa_descriptor() function. */
typedef const LADSPA_Descriptor * 
(*LADSPA_Descriptor_Function)(int Index);

/**********************************************************************/




typedef struct _DSSI_Program_Descriptor {

    /** Bank number for this program.  Note that DSSI does not support
        MIDI-style separation of bank LSB and MSB values.  There is no
        restriction on the set of available banks: the numbers do not
        need to be contiguous, there does not need to be a bank 0, etc. */
    int Bank;

    /** Program number (unique within its bank) for this program.
	There is no restriction on the set of available programs: the
	numbers do not need to be contiguous, there does not need to
	be a program 0, etc. */
    int Program;

    /** Name of the program. */
    const char * Name;

} DSSI_Program_Descriptor;


typedef struct _DSSI_Descriptor {

    /**
     * DSSI_API_Version
     *
     * This member indicates the DSSI API level used by this plugin.
     * If we're lucky, this will never be needed.  For now all plugins
     * must set it to 1.
     */
    int DSSI_API_Version;

    /**
     * LADSPA_Plugin
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
     * instantiate function, passing in this LADSPA_Descriptor
     * pointer.  The returned LADSPA_Handle is used as the argument
     * for the DSSI functions below as well as for the LADSPA ones.
     */
    const LADSPA_Descriptor *LADSPA_Plugin;

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
     *
     * Reserved and special key prefixes
     * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     * The DSSI: prefix
     * ----------------
     * Configure keys starting with DSSI: are reserved for particular
     * purposes documented in the DSSI specification.  At the moment,
     * there is one such key: DSSI:PROJECT_DIRECTORY.  A host may call
     * configure() passing this key and a directory path value.  This
     * indicates to the plugin and its UI that a directory at that
     * path exists and may be used for project-local data.  Plugins
     * may wish to use the project directory as a fallback location
     * when looking for other file data, or as a base for relative
     * paths in other configuration values.
     *
     * The GLOBAL: prefix
     * ------------------
     * Configure keys starting with GLOBAL: may be used by the plugin
     * and its UI for any purpose, but are treated specially by the
     * host.  When one of these keys is used in a configure OSC call
     * from the plugin UI, the host makes the corresponding configure
     * call (preserving the GLOBAL: prefix) not only to the target
     * plugin but also to all other plugins in the same instance
     * group, as well as their UIs.  Note that if any instance
     * returns non-NULL from configure to indicate error, the host
     * may stop there (and the set of plugins on which configure has
     * been called will thus depend on the host implementation).
     * See also the configure OSC call documentation in RFC.txt.
     */
    char *(*configure)(LADSPA_Handle Instance,
		       const char *Key,
		       const char *Value);

    #define DSSI_RESERVED_CONFIGURE_PREFIX "DSSI:"
    #define DSSI_GLOBAL_CONFIGURE_PREFIX "GLOBAL:"
    #define DSSI_PROJECT_DIRECTORY_KEY \
	DSSI_RESERVED_CONFIGURE_PREFIX "PROJECT_DIRECTORY"

    /**
     * get_program()
     *
     * This member is a function pointer that provides a description
     * of a program (named preset sound) available on this synth.  A
     * plugin that does not support programs at all should set this
     * member to NULL.
     *
     * The Index argument is an index into the plugin's list of
     * programs, not a program number as represented by the Program
     * field of the DSSI_Program_Descriptor.  (This distinction is
     * needed to support synths that use non-contiguous program or
     * bank numbers.)
     *
     * This function returns a DSSI_Program_Descriptor pointer that is
     * guaranteed to be valid only until the next call to get_program,
     * deactivate, or configure, on the same plugin instance.  This
     * function must return NULL if passed an Index argument out of
     * range, so that the host can use it to query the number of
     * programs as well as their properties.
     */
    const DSSI_Program_Descriptor *(*get_program)(LADSPA_Handle Instance, int Index);
    
    /**
     * select_program()
     *
     * This member is a function pointer that selects a new program
     * for this synth.  The program change should take effect
     * immediately at the start of the next run_synth() call.  (This
     * means that a host providing the capability of changing programs
     * between any two notes on a track must vary the block size so as
     * to place the program change at the right place.  A host that
     * wanted to avoid this would probably just instantiate a plugin
     * for each program.)
     * 
     * A plugin that does not support programs at all should set this
     * member NULL.  Plugins should ignore a select_program() call
     * with an invalid bank or program.
     *
     * A plugin is not required to select any particular default
     * program on activate(): it's the host's duty to set a program
     * explicitly.  The current program is invalidated by any call to
     * configure().
     *
     * A plugin is permitted to re-write the values of its input
     * control ports when select_program is called.  The host should
     * re-read the input control port values and update its own
     * records appropriately.  (This is the only circumstance in
     * which a DSSI plugin is allowed to modify its own input ports.)
     */
    void (*select_program)(LADSPA_Handle Instance,
			   int Bank,
			   int Program);

    /**
     * get_midi_controller_for_port()
     *
     * This member is a function pointer that returns the MIDI
     * controller number or NRPN that should be mapped to the given
     * input control port.  If the given port should not have any MIDI
     * controller mapped to it, the function should return DSSI_NONE.
     * The behaviour of this function is undefined if the given port
     * number does not correspond to an input control port.  A plugin
     * that does not want MIDI controllers mapped to ports at all may
     * set this member NULL.
     *
     * Correct values can be got using the macros DSSI_CC(num) and
     * DSSI_NRPN(num) as appropriate, and values can be combined using
     * bitwise OR: e.g. DSSI_CC(23) | DSSI_NRPN(1069) means the port
     * should respond to CC #23 and NRPN #1069.
     *
     * The host is responsible for doing proper scaling from MIDI
     * controller and NRPN value ranges to port ranges according to
     * the plugin's LADSPA port hints.  Hosts should not deliver
     * through run_synth any MIDI controller events that have already
     * been mapped to control port values.
     *
     * A plugin should not attempt to request mappings from
     * controllers 0 or 32 (MIDI Bank Select MSB and LSB).
     */
    int (*get_midi_controller_for_port)(LADSPA_Handle Instance,
					int Port);

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
     *
     * Bank and Program Events
     * ~~~~~~~~~~~~~~~~~~~~~~~
     * Hosts must map MIDI Bank Select MSB and LSB (0 and 32)
     * controllers and MIDI Program Change events onto the banks and
     * programs specified by the plugin, using the DSSI select_program
     * call.  No host should ever deliver a program change or bank
     * select controller to a plugin via run_synth.
     */
    void (*run_synth)(LADSPA_Handle    Instance,
		      int    SampleCount,
		      snd_seq_event_t *Events,
		      int    EventCount);

    /**
     * run_synth_adding()
     *
     * This member is a function pointer that runs an instance of a
     * synth for a block, adding its outputs to the values already
     * present at the output ports.  This is provided for symmetry
     * with LADSPA run_adding(), and is equally optional.  A plugin
     * that does not provide it must set this member to NULL.
     */
    void (*run_synth_adding)(LADSPA_Handle    Instance,
			     int    SampleCount,
			     snd_seq_event_t *Events,
			     int    EventCount);

    /**
     * run_multiple_synths()
     *
     * This member is a function pointer that runs multiple synth
     * instances for a block.  This is very similar to run_synth(),
     * except that Instances, Events, and EventCounts each point to
     * arrays that hold the LADSPA handles, event buffers, and
     * event counts for each of InstanceCount instances.  That is,
     * Instances points to an array of InstanceCount pointers to
     * DSSI plugin instantiations, Events points to an array of
     * pointers to each instantiation's respective event list, and
     * EventCounts points to an array containing each instantiation's
     * respective event count.
     *
     * A host using this function must guarantee that ALL active
     * instances of the plugin are represented in each call to the
     * function -- that is, a host may not call run_multiple_synths()
     * for some instances of a given plugin and then call run_synth()
     * as well for others.  'All .. instances of the plugin' means
     * every instance sharing the same LADSPA label and shared object
     * (*.so) file (rather than every instance sharing the same *.so).
     * 'Active' means any instance for which activate() has been called
     * but deactivate() has not.
     *
     * A plugin may provide this function, run_synths() (see above),
     * both, or neither (if it not in fact a synth).  A plugin that
     * does not provide this function must set this member to NULL.
     * Plugin authors implementing run_multiple_synths are strongly
     * encouraged to implement run_synth as well if at all possible,
     * to aid simplistic hosts, even where it would be less efficient
     * to use it.
     */
    void (*run_multiple_synths)(int     InstanceCount,
                                LADSPA_Handle    *Instances,
                                int     SampleCount,
                                snd_seq_event_t **Events,
                                int    *EventCounts);

    /**
     * run_multiple_synths_adding()
     *
     * This member is a function pointer that runs multiple synth
     * instances for a block, adding each synth's outputs to the
     * values already present at the output ports.  This is provided
     * for symmetry with both the DSSI run_multiple_synths() and LADSPA
     * run_adding() functions, and is equally optional.  A plugin
     * that does not provide it must set this member to NULL.
     */
    void (*run_multiple_synths_adding)(int     InstanceCount,
                                       LADSPA_Handle    *Instances,
                                       int     SampleCount,
                                       snd_seq_event_t **Events,
                                       int    *EventCounts);
} DSSI_Descriptor;

/**
 * DSSI supports a plugin discovery method similar to that of LADSPA:
 *
 * - DSSI hosts may wish to locate DSSI plugin shared object files by
 *    searching the paths contained in the DSSI_PATH and LADSPA_PATH
 *    environment variables, if they are present.  Both are expected
 *    to be colon-separated lists of directories to be searched (in
 *    order), and DSSI_PATH should be searched first if both variables
 *    are set.
 *
 * - Each shared object file containing DSSI plugins must include a
 *   function dssi_descriptor(), with the following function prototype
 *   and C-style linkage.  Hosts may enumerate the plugin types
 *   available in the shared object file by repeatedly calling
 *   this function with successive Index values (beginning from 0),
 *   until a return value of NULL indicates no more plugin types are
 *   available.  Each non-NULL return is the DSSI_Descriptor
 *   of a distinct plugin type.
 */

const DSSI_Descriptor *dssi_descriptor(int Index);
  
typedef const DSSI_Descriptor *(*DSSI_Descriptor_Function)(int Index);

/*
 * Macros to specify particular MIDI controllers in return values from
 * get_midi_controller_for_port()
 */

#define DSSI_CC_BITS			0x20000000
#define DSSI_NRPN_BITS			0x40000000

#define DSSI_NONE			-1
#define DSSI_CONTROLLER_IS_SET(n)	(DSSI_NONE != (n))

#define DSSI_CC(n)			(DSSI_CC_BITS | (n))
#define DSSI_IS_CC(n)			(DSSI_CC_BITS & (n))
#define DSSI_CC_NUMBER(n)		((n) & 0x7f)

#define DSSI_NRPN(n)			(DSSI_NRPN_BITS | ((n) << 7))
#define DSSI_IS_NRPN(n)			(DSSI_NRPN_BITS & (n))
#define DSSI_NRPN_NUMBER(n)		(((n) >> 7) & 0x3fff)

#ifdef __cplusplus
}
#endif

#endif /* PYDAW_PLUGIN_INCLUDED */
