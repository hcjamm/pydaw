/* -*- c-basic-offset: 4 -*-  vi:set ts=8 sts=4 sw=4: */
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

#define _BSD_SOURCE    1
#define _SVID_SOURCE   1
#define _ISOC99_SOURCE 1

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "pydaw_files.h"
#include "../../include/pydaw3/pydaw_plugin.h"
#include <alsa/asoundlib.h>
#include <alsa/seq.h>
#include <portaudio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <dlfcn.h>
#include <unistd.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <signal.h>
#include <dirent.h>
#include <time.h>
#include <libgen.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <lo/lo.h>

#include "main.h"

#include "message_buffer.h"

#include "synth.c"

static snd_seq_t *alsaClient;

#define SAMPLE_RATE (44100)
#define PA_SAMPLE_TYPE paFloat32
#define FRAMES_PER_BUFFER (512)

typedef float SAMPLE;

//static d3h_dll_t     *dlls;

static d3h_plugin_t  *plugins;
static int            plugin_count = 0;

static float sample_rate;

static d3h_instance_t instances[D3H_MAX_INSTANCES];
static int            instance_count = 0;

static PYFX_Handle    *instanceHandles;
static snd_seq_event_t **instanceEventBuffers;
static int    *instanceEventCounts;

static int insTotal, outsTotal;
static float **pluginInputBuffers, **pluginOutputBuffers;

static int controlInsTotal, controlOutsTotal;
static float *pluginControlIns, *pluginControlOuts;
static d3h_instance_t **pluginControlInInstances;          /* maps global control in # to instance */
static unsigned long *pluginControlInPortNumbers;          /* maps global control in # to instance LADSPA port # */
static int *pluginPortUpdated;                             /* indexed by global control in # */

static char osc_path_tmp[1024];

static char *projectDirectory;

static char * clientName;

lo_server_thread serverThread;

static sigset_t _signals;

int exiting = 0;
static int verbose = 0;
const char *myName = "pydaw";

#define EVENT_BUFFER_SIZE 1024
static snd_seq_event_t midiEventBuffer[EVENT_BUFFER_SIZE]; /* ring buffer */
static int midiEventReadIndex = 0, midiEventWriteIndex = 0;

static pthread_mutex_t midiEventBufferMutex = PTHREAD_MUTEX_INITIALIZER;

void osc_error(int num, const char *m, const char *path);

int osc_message_handler(const char *path, const char *types, lo_arg **argv, int
		      argc, void *data, void *user_data) ;
int osc_debug_handler(const char *path, const char *types, lo_arg **argv, int
		      argc, void *data, void *user_data) ;

void start_gui()
{
    if (fork() == 0) 
    {
        printf("start_gui with osc path %s\n", osc_path_tmp);
        execlp("/usr/lib/pydaw3/pydaw/PYDAW_qt", "/usr/lib/pydaw3/pydaw/PYDAW_qt", osc_path_tmp, "pydaw.so", "pydaw3", "pydaw", (char*)NULL);
        perror("exec failed");
        exit(1);  //TODO:  should be getting rid of this???
    }
}

void signalHandler(int sig)
{
    fprintf(stderr, "%s: signal %d caught, trying to clean up and exit\n",
	    myName, sig);
    exiting = 1;
}

void midi_callback()
{
    snd_seq_event_t *ev = 0;
    struct timeval tv;

    pthread_mutex_lock(&midiEventBufferMutex);

    do {
	if (snd_seq_event_input(alsaClient, &ev) > 0) {

	    if (midiEventReadIndex == midiEventWriteIndex + 1) {
		fprintf(stderr, "%s: Warning: MIDI event buffer overflow! ignoring incoming event\n", myName);
		continue;
	    }

	    midiEventBuffer[midiEventWriteIndex] = *ev;

	    ev = &midiEventBuffer[midiEventWriteIndex];

	    /* At this point we change the event timestamp so that its
	       real-time field contains the actual time at which it was
	       received and processed (i.e. now).  Then in the audio
	       callback we use that to calculate frame offset. */

	    gettimeofday(&tv, NULL);
	    ev->time.time.tv_sec = tv.tv_sec;
	    ev->time.time.tv_nsec = tv.tv_usec * 1000L;

	    if (ev->type == SND_SEQ_EVENT_NOTEON && ev->data.note.velocity == 0) {
		ev->type =  SND_SEQ_EVENT_NOTEOFF;
	    }

	    /* We don't need to handle EVENT_NOTE here, because ALSA
	       won't ever deliver them on the sequencer queue -- it
	       unbundles them into NOTE_ON and NOTE_OFF when they're
	       dispatched.  We would only need worry about them when
	       retrieving MIDI events from some other source. */

	    midiEventWriteIndex = (midiEventWriteIndex + 1) % EVENT_BUFFER_SIZE;
	}
	
    } while (snd_seq_event_input_pending(alsaClient, 0) > 0);

    pthread_mutex_unlock(&midiEventBufferMutex);
}

void setControl(d3h_instance_t *instance, long controlIn, snd_seq_event_t *event)
{
    float value = (float)event->data.control.value;

    pluginControlIns[controlIn] = value;
    pluginPortUpdated[controlIn] = 1;
}

static int portaudioCallback( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData )
{
    int i;
    int outCount; 
    //int inCount;
    d3h_instance_t *instance;
    struct timeval tv, evtv, diff;
    long framediff;
    
    float *out = (float*)outputBuffer;
    
    (void) inputBuffer; //Prevent unused variable warning.

    gettimeofday(&tv, NULL);

    /* Not especially pretty or efficient */

    for (i = 0; i < instance_count; i++) {
        instanceEventCounts[i] = 0;
    }

    for ( ; midiEventReadIndex != midiEventWriteIndex;
         midiEventReadIndex = (midiEventReadIndex + 1) % EVENT_BUFFER_SIZE) {

	snd_seq_event_t *ev = &midiEventBuffer[midiEventReadIndex];

        if (!snd_seq_ev_is_channel_type(ev)) {
            /* discard non-channel oriented messages */
            continue;
        }

        instance = &instances[0];
        
        i = 0; //instance->number;

        /* Stop processing incoming MIDI if an instance's event buffer is
         * full. */
	if (instanceEventCounts[i] == EVENT_BUFFER_SIZE)
            break;

	/* Each event has a real-time timestamp indicating when it was
	 * received (set by midi_callback).  We need to calculate the
	 * difference between then and the start of the audio callback
	 * (held in tv), and use that to assign a frame offset, to
	 * avoid jitter.  We should stop processing when we reach any
	 * event received after the start of the audio callback. */

	evtv.tv_sec = ev->time.time.tv_sec;
	evtv.tv_usec = ev->time.time.tv_nsec / 1000;

	if (evtv.tv_sec > tv.tv_sec ||
	    (evtv.tv_sec == tv.tv_sec &&
	     evtv.tv_usec > tv.tv_usec)) {
	    break;
	}

	diff.tv_sec = tv.tv_sec - evtv.tv_sec;
	if (tv.tv_usec < evtv.tv_usec) {
	    --diff.tv_sec;
	    diff.tv_usec = tv.tv_usec + 1000000 - evtv.tv_usec;
	} else {
	    diff.tv_usec = tv.tv_usec - evtv.tv_usec;
	}

	framediff =
	    diff.tv_sec * sample_rate +
	    ((diff.tv_usec / 1000) * sample_rate) / 1000 +
	    ((diff.tv_usec - 1000 * (diff.tv_usec / 1000)) * sample_rate) / 1000000;

	if (framediff >= framesPerBuffer) framediff = framesPerBuffer - 1;
	else if (framediff < 0) framediff = 0;

	ev->time.tick = framesPerBuffer - framediff - 1;

	if (ev->type == SND_SEQ_EVENT_CONTROLLER) {
	    
	    int controller = ev->data.control.param;

	    if (controller == 0) 
            { // bank select MSB                
		instance->pendingBankMSB = ev->data.control.value;
                
	    } else if (controller == 32) 
            { // bank select LSB
		instance->pendingBankLSB = ev->data.control.value;

	    } else if (controller > 0 && controller < MIDI_CONTROLLER_COUNT) 
            {
		long controlIn = instance->controllerMap[controller];
		if (controlIn >= 0) 
                {
                    /* controller is mapped to LADSPA port, update the port */
		    setControl(instance, controlIn, ev);

		} else {

                    /* controller is not mapped, so pass the event through to plugin */
                    instanceEventBuffers[i][instanceEventCounts[i]] = *ev;
                    instanceEventCounts[i]++;
                }
	    }
	} 
        else 
        {
            instanceEventBuffers[i][instanceEventCounts[i]] = *ev;
            instanceEventCounts[i]++;
	}
    }

    i = 0;
    outCount = 0;
    outCount += instances[i].plugin->outs;

    if (instances[i].plugin->descriptor->run_synth) 
    {
        instances[i].plugin->descriptor->run_synth(instanceHandles[i],  framesPerBuffer, instanceEventBuffers[i],
                                                   instanceEventCounts[i]);
    } 
    else if (instances[i].plugin->descriptor->PYFX_Plugin->run) 
    {
        instances[i].plugin->descriptor->PYFX_Plugin->run(instanceHandles[i], framesPerBuffer);
    }
        
    for( i=0; i < framesPerBuffer; i++ )    
    {
        *out++ = pluginOutputBuffers[0][i];  // left
        *out++ = pluginOutputBuffers[1][i];  // right        
    }

    return paContinue;
}

#ifndef RTLD_LOCAL
#define RTLD_LOCAL  (0)
#endif

static int instance_sort_cmp(const void *a, const void *b)
{
    d3h_instance_t *ia = (d3h_instance_t *)a;
    d3h_instance_t *ib = (d3h_instance_t *)b;

    if (ia->plugin->number != ib->plugin->number) {
        return ia->plugin->number - ib->plugin->number;
    } else {
        return ia->channel - ib->channel;
    }
}

int main(int argc, char **argv)
{
    int portid;
    int npfd;
    struct pollfd *pfd;

    d3h_dll_t *dll;
    d3h_plugin_t *plugin;
    d3h_instance_t *instance;
    
    char *tmp;
    char *url;
    int i, reps, j;
    int in, out, controlIn, controlOut;
    clientName = "PyDAWv3";    
        
    setsid();
    sigemptyset (&_signals);
    sigaddset(&_signals, SIGHUP);
    sigaddset(&_signals, SIGINT);
    sigaddset(&_signals, SIGQUIT);
    sigaddset(&_signals, SIGPIPE);
    sigaddset(&_signals, SIGTERM);
    sigaddset(&_signals, SIGUSR1);
    sigaddset(&_signals, SIGUSR2);
    pthread_sigmask(SIG_BLOCK, &_signals, 0);

    insTotal = outsTotal = controlInsTotal = controlOutsTotal = 0;

    plugin = (d3h_plugin_t *)calloc(1, sizeof(d3h_plugin_t));
    plugin->number = plugin_count;
    plugin->label = "pydaw";
    dll = (d3h_dll_t *)calloc(1, sizeof(d3h_dll_t));
    dll->name = "pydaw";
    dll->directory = "/usr/lib/pydaw3";
    dll->descfn = (PYINST_Descriptor_Function)PYINST_descriptor; 
    j = 0;
    
    plugin->descriptor = PYINST_descriptor(0);
    
    plugin->dll = dll;

    /* Count number of i/o buffers and ports required */
    plugin->ins = 0;
    plugin->outs = 0;
    plugin->controlIns = 0;
    plugin->controlOuts = 0;

    for (j = 0; j < plugin->descriptor->PYFX_Plugin->PortCount; j++) 
    {
        PYFX_PortDescriptor pod = plugin->descriptor->PYFX_Plugin->PortDescriptors[j];

        if (PYFX_IS_PORT_AUDIO(pod)) 
        {
            if (PYFX_IS_PORT_INPUT(pod)) ++plugin->ins;
            else if (PYFX_IS_PORT_OUTPUT(pod)) ++plugin->outs;
        }
        else if (PYFX_IS_PORT_CONTROL(pod)) 
        {
            if (PYFX_IS_PORT_INPUT(pod)) ++plugin->controlIns;
            else if (PYFX_IS_PORT_OUTPUT(pod)) ++plugin->controlOuts;
        }
    }

    /* finish up new plugin */
    plugin->instances = 0;
    plugin->next = plugins;
    plugins = plugin;
    plugin_count++;

    reps = 1;
    /* set up instances */
    
    instance = &instances[instance_count];

    instance->plugin = plugin;
    instance->channel = instance_count;
    instance->inactive = 1;
    tmp = (char *)malloc(strlen(plugin->dll->name) +
                         strlen(plugin->label) + 9);
    instance->friendly_name = tmp;
    strcpy(tmp, plugin->dll->name);
    if (strlen(tmp) > 3 &&
        !strcasecmp(tmp + strlen(tmp) - 3, ".so")) {
        tmp = tmp + strlen(tmp) - 3;
    } else {
        tmp = tmp + strlen(tmp);
    }
    sprintf(tmp, "/%s/chan%02d", plugin->label, instance->channel);
    instance->pluginProgramCount = 0;    
    instance->currentBank = 0;
    instance->currentProgram = 0;
    instance->pendingBankLSB = -1;
    instance->pendingBankMSB = -1;
    instance->pendingProgramChange = -1;
    instance->uiTarget = NULL;
    instance->uiSource = NULL;
    instance->ui_initial_show_sent = 0;
    instance->uiNeedsProgramUpdate = 0;
    instance->ui_osc_control_path = NULL;
    instance->ui_osc_program_path = NULL;
    instance->ui_osc_quit_path = NULL;
    instance->ui_osc_rate_path = NULL;
    instance->ui_osc_show_path = NULL;

    insTotal += plugin->ins;
    outsTotal += plugin->outs;
    controlInsTotal += plugin->controlIns;
    controlOutsTotal += plugin->controlOuts;

    plugin->instances++;
    instance_count++;

    reps = 1;


    /* sort array of instances to group them by plugin */
    if (instance_count > 1) {
        qsort(instances, instance_count, sizeof(d3h_instance_t), instance_sort_cmp);
    }
    
    pluginInputBuffers = (float **)malloc(insTotal * sizeof(float *));
    pluginControlIns = (float *)calloc(controlInsTotal, sizeof(float));
    pluginControlInInstances =
        (d3h_instance_t **)malloc(controlInsTotal * sizeof(d3h_instance_t *));
    pluginControlInPortNumbers =
        (unsigned long *)malloc(controlInsTotal * sizeof(unsigned long));
    pluginPortUpdated = (int *)malloc(controlInsTotal * sizeof(int));

    pluginOutputBuffers = (float **)malloc(outsTotal * sizeof(float *));
    pluginControlOuts = (float *)calloc(controlOutsTotal, sizeof(float));

    instanceHandles = (PYFX_Handle *)malloc(instance_count *
                                              sizeof(PYFX_Handle));
    instanceEventBuffers = (snd_seq_event_t **)malloc(instance_count *
                                                      sizeof(snd_seq_event_t *));
    instanceEventCounts = (int*)malloc(instance_count * sizeof(int));

    for (i = 0; i < instance_count; i++) {
        instanceEventBuffers[i] = (snd_seq_event_t *)malloc(EVENT_BUFFER_SIZE *
                                                            sizeof(snd_seq_event_t));
        instances[i].pluginPortControlInNumbers =
            (int *)malloc(instances[i].plugin->descriptor->PYFX_Plugin->PortCount *
                          sizeof(int));
    }
        
    int f_frame_count = 8192; //FRAMES_PER_BUFFER;
    sample_rate = 44100.0f;
    
        /*Start Portaudio*/
    PaStreamParameters inputParameters, outputParameters;
    PaStream *stream;
    PaError err;
    
    err = Pa_Initialize();
    //if( err != paNoError ) goto error;

    inputParameters.device = Pa_GetDefaultInputDevice(); /* default input device */
    
    char f_device_file_path[2048];
    char * f_home = getenv("HOME");
    if(!strcmp(f_home, "/home/ubuntu") && i_pydaw_file_exists("/media/pydaw_data"))
    {
        sprintf(f_device_file_path, "/media/pydaw_data/pydaw3/device.txt");
    }
    else
    {
        sprintf(f_device_file_path, "%s/pydaw3/device.txt", f_home);
    }
    
    char * f_show_dialog_cmd = "python2.7 /usr/lib/pydaw3/pydaw/python/libpydaw/pydaw_portaudio.py";
    char f_cmd_buffer[10000];
    f_cmd_buffer[0] = '\0';
    char f_device_name[1024];
    f_device_name[0] = '\0';
    
    f_frame_count = FRAMES_PER_BUFFER;
    
    while(1)
    {
        if(i_pydaw_file_exists(f_device_file_path))
        {
            printf("device.txt exists\n");
            t_2d_char_array * f_current_string = g_get_2d_array_from_file(f_device_file_path, LMS_LARGE_STRING); 
            
            while(1)
            {            
                char * f_key_char = c_iterate_2d_char_array(f_current_string);
                if(f_current_string->eof)
                {
                    break;
                }                
                if(!strcmp(f_key_char, "") || f_current_string->eol)
                {
                    continue;
                }
                
                char * f_value_char = c_iterate_2d_char_array_to_next_line(f_current_string);
                
                if(!strcmp(f_key_char, "name"))
                {
                    sprintf(f_device_name, "%s", f_value_char);
                    printf("device name: %s\n", f_device_name);
                }
                else if(!strcmp(f_key_char, "bufferSize"))
                {                    
                    f_frame_count = atoi(f_value_char);
                    printf("bufferSize: %i\n", f_frame_count);
                }
                else if(!strcmp(f_key_char, "sampleRate"))
                {                    
                    sample_rate = atof(f_value_char);
                    printf("sampleRate: %i\n", f_frame_count);
                }
                else
                {
                    printf("Unknown key|value pair: %s|%s\n", f_key_char, f_value_char);
                }
            }
        }
        else
        {
            printf("device.txt does not exist\n");
            f_device_name[0] = '\0';
            system(f_show_dialog_cmd);
            if(i_pydaw_file_exists(f_device_file_path))
            {
                continue;  //If not, just go with the default device
            }
        }
        
        if (inputParameters.device == paNoDevice)
        {
          sprintf(f_cmd_buffer, "%s \"%s\"", f_show_dialog_cmd, "Error: No default input device.\n");
          system(f_cmd_buffer);
          continue;
        }
        inputParameters.channelCount = 0;
        inputParameters.sampleFormat = PA_SAMPLE_TYPE;
        inputParameters.suggestedLatency = Pa_GetDeviceInfo( inputParameters.device )->defaultLowInputLatency;
        inputParameters.hostApiSpecificStreamInfo = NULL;

        if (outputParameters.device == paNoDevice) 
        {
          sprintf(f_cmd_buffer, "%s \"%s\"", f_show_dialog_cmd, "Error: No default output device.\n");
          system(f_cmd_buffer);
          continue;
        }
        
        outputParameters.channelCount = 2; /* stereo output */
        outputParameters.sampleFormat = PA_SAMPLE_TYPE;
        outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
        outputParameters.hostApiSpecificStreamInfo = NULL;
        
        if(f_device_name[0] == '\0')
        {
            outputParameters.device = Pa_GetDefaultOutputDevice();
        }
        else
        {
            int f_i = 0;
            while(f_i < Pa_GetDeviceCount())
            {
                const PaDeviceInfo * f_padevice = Pa_GetDeviceInfo(f_i);  //I guess that this is not supposed to be free'd?
                if(!strcmp(f_padevice->name, f_device_name))
                {
                    outputParameters.device = f_i;
                    break;
                }
                f_i++;
            }
        }

        err = Pa_OpenStream(
                  &stream,
                  0, //&inputParameters,
                  &outputParameters,
                  44100.0f, //SAMPLE_RATE,
                  f_frame_count, //FRAMES_PER_BUFFER,
                  0, /* paClipOff, */ /* we won't output out of range samples so don't bother clipping them */
                  portaudioCallback,
                  NULL );
        if( err != paNoError )
        {
            sprintf(f_cmd_buffer, "%s \"%s\"", f_show_dialog_cmd, "Error: Unknown error while opening device.");
            system(f_cmd_buffer);
            continue;
        }

        err = Pa_StartStream( stream );
        if( err != paNoError )
        {
            sprintf(f_cmd_buffer, "%s \"%s\"", f_show_dialog_cmd, "Error: Unknown error while starting device.  Please re-configure your device and try starting PyDAW again.");
            system(f_cmd_buffer);
            continue;
        }
        
        break;
    }

    in = 0;
    out = 0;
    reps = 0;
    for (i = 0; i < instance_count; i++) {
	if (i > 0 &&
	    !strcmp(instances[i  ].plugin->descriptor->PYFX_Plugin->Name,
		    instances[i-1].plugin->descriptor->PYFX_Plugin->Name)) {
	    ++reps;
	} else if (i < instance_count - 1 &&
		   !strcmp(instances[i  ].plugin->descriptor->PYFX_Plugin->Name,
			   instances[i+1].plugin->descriptor->PYFX_Plugin->Name)) {
	    reps = 1;
	} else {
	    reps = 0;
	}
	for (j = 0; j < instances[i].plugin->ins; ++j) 
        {
            //Port naming code was here
            
            if(posix_memalign((void**)(&pluginInputBuffers[in]), 16, (sizeof(float) * f_frame_count)) != 0)
            {
                return 0;
            }
            
            int f_i = 0;
            while(f_i < f_frame_count)
            {
                pluginInputBuffers[in][f_i] = 0.0f;
                f_i++;
            }	    
	    ++in;
	}
	for (j = 0; j < instances[i].plugin->outs; ++j) 
        {
	    //Port naming code was here            
	                
            if(posix_memalign((void**)(&pluginOutputBuffers[out]), 16, (sizeof(float) * f_frame_count)) != 0)
            {
                return 0;
            }
            
            int f_i = 0;
            while(f_i < f_frame_count)
            {
                pluginOutputBuffers[out][f_i] = 0.0f;
                f_i++;
            }
            
	    ++out;
	}
    }
    
    /* Instantiate plugins */

    for (i = 0; i < instance_count; i++) {
        plugin = instances[i].plugin;
        instanceHandles[i] = plugin->descriptor->PYFX_Plugin->instantiate
            (plugin->descriptor->PYFX_Plugin, sample_rate);
        if (!instanceHandles[i]) {
            fprintf(stderr, "\n%s: Error: Failed to instantiate instance %d!, plugin \"%s\"\n",
                    myName, i, plugin->label);
            return 1;
        }
	if (projectDirectory && plugin->descriptor->configure) {
	    char *rv =plugin->descriptor->configure(instanceHandles[i],
						    PYINST_PROJECT_DIRECTORY_KEY,
						    projectDirectory);
	    if (rv) {
		fprintf(stderr, "%s: Warning: plugin doesn't like project directory: \"%s\"\n", myName, rv);
	    }
	}
    }

    /* Create OSC thread */

    serverThread = lo_server_thread_new(NULL, osc_error);
    snprintf((char *)osc_path_tmp, 31, "/dssi");
    tmp = lo_server_thread_get_url(serverThread);
    url = (char *)malloc(strlen(tmp) + strlen(osc_path_tmp));
    sprintf(url, "%s%s", tmp, osc_path_tmp + 1);
    
    printf("Registering %s\n", url);
    
    free(tmp);

    lo_server_thread_add_method(serverThread, NULL, NULL, osc_message_handler, NULL);
    lo_server_thread_start(serverThread);

    /* Connect and activate plugins */

    for (in = 0; in < controlInsTotal; in++) {
        pluginPortUpdated[in] = 0;
    }

    in = out = controlIn = controlOut = 0;

    for (i = 0; i < instance_count; i++) {   /* i is instance number */
        instance = &instances[i];

        instance->firstControlIn = controlIn;
        for (j = 0; j < MIDI_CONTROLLER_COUNT; j++) {
            instance->controllerMap[j] = -1;
        }

        plugin = instance->plugin;
        for (j = 0; j < plugin->descriptor->PYFX_Plugin->PortCount; j++) {  /* j is LADSPA port number */

            PYFX_PortDescriptor pod =
                plugin->descriptor->PYFX_Plugin->PortDescriptors[j];

            instance->pluginPortControlInNumbers[j] = -1;

            if (PYFX_IS_PORT_AUDIO(pod)) {

                if (PYFX_IS_PORT_INPUT(pod)) {
                    plugin->descriptor->PYFX_Plugin->connect_port
                        (instanceHandles[i], j, pluginInputBuffers[in++]);

                } else if (PYFX_IS_PORT_OUTPUT(pod)) {
                    plugin->descriptor->PYFX_Plugin->connect_port
                        (instanceHandles[i], j, pluginOutputBuffers[out++]);
                }

            } else if (PYFX_IS_PORT_CONTROL(pod)) {

                if (PYFX_IS_PORT_INPUT(pod)) {

                    pluginControlInInstances[controlIn] = instance;
                    pluginControlInPortNumbers[controlIn] = j;
                    instance->pluginPortControlInNumbers[j] = controlIn;

                    plugin->descriptor->PYFX_Plugin->connect_port
                        (instanceHandles[i], j, &pluginControlIns[controlIn++]);

                } else if (PYFX_IS_PORT_OUTPUT(pod)) {
                    plugin->descriptor->PYFX_Plugin->connect_port
                        (instanceHandles[i], j, &pluginControlOuts[controlOut++]);
                }
            }
        }  /* 'for (j...'  LADSPA port number */

        if (plugin->descriptor->PYFX_Plugin->activate) {
            plugin->descriptor->PYFX_Plugin->activate(instanceHandles[i]);
        }
	instance->inactive = 0;
    } /* 'for (i...' instance number */

    assert(in == insTotal);
    assert(out == outsTotal);
    assert(controlIn == controlInsTotal);
    assert(controlOut == controlOutsTotal);

    /* Create ALSA MIDI port */

    if (snd_seq_open(&alsaClient, "hw", SND_SEQ_OPEN_DUPLEX, 0) < 0) {
	fprintf(stderr, "\n%s: Error: Failed to open ALSA sequencer interface\n",
		myName);
	return 1;
    }

    snd_seq_set_client_name(alsaClient, clientName);

    if ((portid = snd_seq_create_simple_port
	 (alsaClient, clientName,
	  SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE, SND_SEQ_PORT_TYPE_APPLICATION)) < 0) {
	fprintf(stderr, "\n%s: Error: Failed to create ALSA sequencer port\n",
		myName);
	return 1;
    }

    npfd = snd_seq_poll_descriptors_count(alsaClient, POLLIN);
    pfd = (struct pollfd *)alloca(npfd * sizeof(struct pollfd));
    snd_seq_poll_descriptors(alsaClient, pfd, npfd, POLLIN);

    mb_init("host: ");
    
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGHUP, signalHandler);
    signal(SIGQUIT, signalHandler);
    pthread_sigmask(SIG_UNBLOCK, &_signals, 0);
    

    /*Start the UI*/
    snprintf(osc_path_tmp, 1024, "%s/%s", url, "pydaw"); //instances[i].friendly_name);
    
    printf("\n%s: OSC URL is:\n%s\n\n", myName, osc_path_tmp);
    
    start_gui();

    MB_MESSAGE("Ready\n");

    exiting = 0;

    while (!exiting)
    {
	if (poll(pfd, npfd, 100) > 0)
        {
	    midi_callback();
	}
        
        //Pa_Sleep(2000);
    }

    err = Pa_CloseStream( stream );
    //if( err != paNoError ) goto error;    
    Pa_Terminate();

    /* cleanup plugins */
    for (i = 0; i < instance_count; i++) {
        instance = &instances[i];

        if (instance->uiTarget) {
            lo_send(instance->uiTarget, instance->ui_osc_quit_path, "");
            lo_address_free(instance->uiTarget);
            instance->uiTarget = NULL;
        }

        if (instance->uiSource) {
            lo_address_free(instance->uiSource);
            instance->uiSource = NULL;
        }

        if (instance->plugin->descriptor->PYFX_Plugin->deactivate) {
            instance->plugin->descriptor->PYFX_Plugin->deactivate
		(instanceHandles[i]);
	}

        if (instance->plugin->descriptor->PYFX_Plugin->cleanup) {
            instance->plugin->descriptor->PYFX_Plugin->cleanup
		(instanceHandles[i]);
	}
    }

    v_pydaw_destructor();
    
    sleep(1);
    sigemptyset (&_signals);
    sigaddset(&_signals, SIGHUP);
    pthread_sigmask(SIG_BLOCK, &_signals, 0);
    kill(0, SIGHUP);
   
    printf("PyDAW main() returning\n");
    return 0;
}

void osc_error(int num, const char *msg, const char *path)
{
    fprintf(stderr, "%s: liblo server error %d in path %s: %s\n",
	    myName, num, path, msg);
}

int osc_control_handler(d3h_instance_t *instance, lo_arg **argv)
{
    int port = argv[0]->i;
    PYFX_Data value = argv[1]->f;

    if (port < 0 || port > instance->plugin->descriptor->PYFX_Plugin->PortCount) {
	fprintf(stderr, "%s: OSC: %s port number (%d) is out of range\n",
                myName, instance->friendly_name, port);
	return 0;
    }
    if (instance->pluginPortControlInNumbers[port] == -1) {
	fprintf(stderr, "%s: OSC: %s port %d is not a control in\n",
                myName, instance->friendly_name, port);
	return 0;
    }
    pluginControlIns[instance->pluginPortControlInNumbers[port]] = value;
    if (verbose) {
	printf("%s: OSC: %s port %d = %f\n",
	       myName, instance->friendly_name, port, value);
    }
    
    return 0;
}

int osc_configure_handler(d3h_instance_t *instance, lo_arg **argv)
{
    const char *key = (const char *)&argv[0]->s;
    const char *value = (const char *)&argv[1]->s;
    char *message;

    /* This is pretty much the simplest legal implementation of
     * configure in a DSSI host. */

    /* The host has the option to remember the set of (key,value)
     * pairs associated with a particular instance, so that if it
     * wants to restore the "same" instance on another occasion it can
     * just call configure() on it for each of those pairs and so
     * restore state without any input from a GUI.  Any real-world GUI
     * host will probably want to do that.  This host doesn't have any
     * concept of restoring an instance from one run to the next, so
     * we don't bother remembering these at all. */

    if (instance->plugin->descriptor->configure) {

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
	
	while (n <= m) {

	    message = instances[n].plugin->descriptor->configure
		(instanceHandles[n], key, value);
	    if (message) {
		printf("%s: on configure '%s' '%s', plugin '%s' returned error '%s'\n",
		       myName, key, value, instance->friendly_name, message);
		free(message);
	    }

	    // also call back on UIs for plugins other than the one
	    // that requested this:
	    if (n != instance->number && instances[n].uiTarget) {
		lo_send(instances[n].uiTarget,
			instances[n].ui_osc_configure_path, "ss", key, value);
	    }
	    
	    ++n;
	}	    
    }

    return 0;
}

int
osc_update_handler(d3h_instance_t *instance, lo_arg **argv, lo_address source)
{
    const char *url = (char *)&argv[0]->s;
    const char *path;
    unsigned int i;
    char *host, *port;
    const char *chost, *cport;

    if (verbose) {
	printf("%s: OSC: got update request from <%s>\n", myName, url);
    }

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
    lo_send(instance->uiTarget, instance->ui_osc_rate_path, "i", lrintf(sample_rate));

    /* At this point a more substantial host might also call
     * configure() on the UI to set any state that it had remembered
     * for the plugin instance.  But we don't remember state for
     * plugin instances (see our own configure() implementation in
     * osc_configure_handler), and so we have nothing to send except
     * the optional project directory. */

    if (projectDirectory) {
	lo_send(instance->uiTarget, instance->ui_osc_configure_path, "ss",
		PYINST_PROJECT_DIRECTORY_KEY, projectDirectory);
    }

    /* Send current bank/program  (-FIX- another race...) */
    if (instance->pendingProgramChange < 0) {
        unsigned long bank = instance->currentBank;
        unsigned long program = instance->currentProgram;
        instance->uiNeedsProgramUpdate = 0;
        if (instance->uiTarget) {
            lo_send(instance->uiTarget, instance->ui_osc_program_path, "ii", bank, program);
        }
    }

    /* Send control ports */
    for (i = 0; i < instance->plugin->controlIns; i++) {
        int in = i + instance->firstControlIn;
	int port = pluginControlInPortNumbers[in];
	lo_send(instance->uiTarget, instance->ui_osc_control_path, "if", port,
                pluginControlIns[in]);
	/* Avoid overloading the GUI if there are lots and lots of ports */
	if ((i+1) % 50 == 0) usleep(50000);
    }

    /* Send 'show' */
    if (!instance->ui_initial_show_sent) {
	lo_send(instance->uiTarget, instance->ui_osc_show_path, "");
	instance->ui_initial_show_sent = 1;
    }

    return 0;
}

int osc_exiting_handler(d3h_instance_t *instance, lo_arg **argv)
{
    int i;

    if (verbose) {
	printf("%s: OSC: got exiting notification for instance %d\n", myName,
	       instance->number);
    }

    if (instance->uiTarget) {
        lo_address_free(instance->uiTarget);
        instance->uiTarget = NULL;
    }

    if (instance->uiSource) {
        lo_address_free(instance->uiSource);
        instance->uiSource = NULL;
    }

    if (instance->plugin) {

	/*!!! No, this isn't safe -- plugins deactivated in this way
	  would still be included in a run_multiple_synths call unless
	  we re-jigged the instance array at the same time -- leave it
	  for now
	if (instance->plugin->descriptor->PYFX_Plugin->deactivate) {
            instance->plugin->descriptor->PYFX_Plugin->deactivate
		(instanceHandles[instance->number]);
	}
	*/
	/* Leave this flag though, as we need it to determine when to exit */
	instance->inactive = 1;
    }

    /* Do we have any plugins left running? */

    for (i = 0; i < instance_count; ++i) {
	if (!instances[i].inactive) return 0;
    }

    if (verbose) {
	printf("%s: That was the last remaining plugin, exiting...\n", myName);
    }
    exiting = 1;
    return 0;
}

int osc_debug_handler(const char *path, const char *types, lo_arg **argv,
                      int argc, void *data, void *user_data)
{
    int i;

    printf("%s: got unhandled OSC message:\npath: <%s>\n", myName, path);
    for (i=0; i<argc; i++) {
        printf("%s: arg %d '%c' ", myName, i, types[i]);
        lo_arg_pp(types[i], argv[i]);
        printf("\n");
    }
    printf("%s:\n", myName);

    return 1;
}

int osc_message_handler(const char *path, const char *types, lo_arg **argv,
                        int argc, void *data, void *user_data)
{
    //int i;
    d3h_instance_t *instance = NULL;
    const char *method;
    unsigned int flen = 0;
    lo_message message;
    lo_address source;
    int send_to_ui = 0;

    if (strncmp(path, "/dssi/", 6))
    {
        printf("if (strncmp(path, \"/dssi/\", 6))\n");
        return osc_debug_handler(path, types, argv, argc, data, user_data);
    }
    
    instance = &instances[0];
    /*
    for (i = 0; i < instance_count; i++) {
	flen = strlen(instances[i].friendly_name);
        if (!strncmp(path + 6, instances[i].friendly_name, flen) &&
	    *(path + 6 + flen) == '/') {
            instance = &instances[i];
            break;
        }
    }
    if (!instance)
    {
        printf("!instance\n");
        return osc_debug_handler(path, types, argv, argc, data, user_data);
    }
    */
           
    
    method = path + 6 + flen;
    /*
    if (*method != '/' || *(method + 1) == 0)
    {
        printf("(*method != '/' || *(method + 1) == 0)\n%s\n", method);
        return osc_debug_handler(path, types, argv, argc, data, user_data);
    }
    method++;
    */
    

    message = (lo_message)data;
    source = lo_message_get_source(message);

    if (instance->uiSource && instance->uiTarget) {
	if (strcmp(lo_address_get_hostname(source),
		   lo_address_get_hostname(instance->uiSource)) ||
	    strcmp(lo_address_get_port(source),
		   lo_address_get_port(instance->uiSource))) {
	    /* This didn't come from our known UI for this plugin,
	       so send an update to that as well */
	    send_to_ui = 1;
	}
    }

    if (!strcmp(method, "pydaw/configure") && argc == 2 && !strcmp(types, "ss")) {

	if (send_to_ui) {
	    lo_send(instance->uiTarget, instance->ui_osc_configure_path, "ss",
		    &argv[0]->s, &argv[1]->s);
	}

        return osc_configure_handler(instance, argv);

    } else if (!strcmp(method, "pydaw/exiting") && argc == 0) {

        return osc_exiting_handler(instance, argv);
    }

    return osc_debug_handler(path, types, argv, argc, data, user_data);
}

