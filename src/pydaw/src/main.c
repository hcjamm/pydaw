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

#define PYDAW_CONFIGURE_KEY_SS "ss"
#define PYDAW_CONFIGURE_KEY_OS "os"
#define PYDAW_CONFIGURE_KEY_SI "si"
#define PYDAW_CONFIGURE_KEY_SR "sr"
#define PYDAW_CONFIGURE_KEY_PLAY "play"
#define PYDAW_CONFIGURE_KEY_REC "rec"
#define PYDAW_CONFIGURE_KEY_STOP "stop"
#define PYDAW_CONFIGURE_KEY_LOOP "loop"
#define PYDAW_CONFIGURE_KEY_TEMPO "tempo"
#define PYDAW_CONFIGURE_KEY_VOL "vol"
#define PYDAW_CONFIGURE_KEY_SOLO "solo"
#define PYDAW_CONFIGURE_KEY_MUTE "mute"
#define PYDAW_CONFIGURE_KEY_CHANGE_INSTRUMENT "ci"
#define PYDAW_CONFIGURE_KEY_SHOW_PLUGIN_UI "su"
#define PYDAW_CONFIGURE_KEY_REC_ARM_TRACK "tr"
#define PYDAW_CONFIGURE_KEY_SHOW_FX_UI "fx"
    
#define PYDAW_CONFIGURE_KEY_PREVIEW_SAMPLE "preview"
#define PYDAW_CONFIGURE_KEY_OFFLINE_RENDER "or"
    
#define PYDAW_CONFIGURE_KEY_SET_TRACK_BUS "bs"
#define PYDAW_CONFIGURE_KEY_AUDIO_ITEM_LOAD_ALL "ai"
#define PYDAW_CONFIGURE_KEY_CREATE_SAMPLE_GRAPH "sg"
    
#define PYDAW_CONFIGURE_KEY_UPDATE_AUDIO_INPUTS "ua"    
#define PYDAW_CONFIGURE_KEY_SET_OVERDUB_MODE "od"
#define PYDAW_CONFIGURE_KEY_LOAD_CC_MAP "cm"
    
#define PYDAW_CONFIGURE_KEY_LOAD_AB_OPEN "abo"
#define PYDAW_CONFIGURE_KEY_LOAD_AB_SET "abs"
#define PYDAW_CONFIGURE_KEY_LOAD_AB_POS "abp"
#define PYDAW_CONFIGURE_KEY_LOAD_AB_VOL "abv"
#define PYDAW_CONFIGURE_KEY_PANIC "panic"
#define PYDAW_CONFIGURE_KEY_CONV32F "conv32f"
#define PYDAW_CONFIGURE_KEY_PITCH_ENV "penv"
#define PYDAW_CONFIGURE_KEY_RATE_ENV "renv"
//Update a single control for a per-audio-item-fx
#define PYDAW_CONFIGURE_KEY_PER_AUDIO_ITEM_FX "paif"
//Reload entire region for per-audio-item-fx
#define PYDAW_CONFIGURE_KEY_PER_AUDIO_ITEM_FX_REGION "par"
#define PYDAW_CONFIGURE_KEY_UPDATE_PLUGIN_CONTROL "pc"
#define PYDAW_CONFIGURE_KEY_CONFIGURE_PLUGIN "co"
#define PYDAW_CONFIGURE_KEY_GLUE_AUDIO_ITEMS "ga"
#define PYDAW_CONFIGURE_KEY_EXIT "exit"
#define PYDAW_CONFIGURE_KEY_MIDI_LEARN "ml"

void v_pydaw_parse_configure_message(t_pydaw_data*, const char*, const char*);

static snd_seq_t *alsaClient;

#define SAMPLE_RATE (44100)
#define PA_SAMPLE_TYPE paFloat32
#define FRAMES_PER_BUFFER (512)

typedef float SAMPLE;

static float sample_rate;

static d3h_instance_t *this_instance;

static PYFX_Handle    instanceHandles;
static snd_seq_event_t *instanceEventBuffers;
static int    instanceEventCounts;

static int insTotal, outsTotal;
static float **pluginInputBuffers, **pluginOutputBuffers;

static int controlInsTotal, controlOutsTotal;

//static char * osc_path_tmp = "osc.udp://localhost:19271/dssi/pydaw";

static char * clientName;

lo_server_thread serverThread;

static sigset_t _signals;

int exiting = 0;
const char *myName = "PyDAW";

#define EVENT_BUFFER_SIZE 1024
static snd_seq_event_t midiEventBuffer[EVENT_BUFFER_SIZE]; /* ring buffer */
static int midiEventReadIndex __attribute__((aligned(16))) = 0;
static int midiEventWriteIndex __attribute__((aligned(16))) = 0;

void osc_error(int num, const char *m, const char *path);

int osc_message_handler(const char *path, const char *types, lo_arg **argv, int
		      argc, void *data, void *user_data) ;
int osc_debug_handler(const char *path, const char *types, lo_arg **argv, int
		      argc, void *data, void *user_data) ;

void signalHandler(int sig)
{
    fprintf(stderr, "%s: signal %d caught, trying to clean up and exit\n", myName, sig);
    exiting = 1;
}

void midi_callback()
{
    snd_seq_event_t *ev = 0;
    struct timeval tv;

    do {
	if (snd_seq_event_input(alsaClient, &ev) > 0) 
        {
	    if (midiEventReadIndex == midiEventWriteIndex + 1) 
            {
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
    struct timeval tv, evtv, diff;
    long framediff;
    
    float *out = (float*)outputBuffer;
    
    (void) inputBuffer; //Prevent unused variable warning.

    gettimeofday(&tv, NULL);

    /* Not especially pretty or efficient */
    
    instanceEventCounts = 0;    

    for ( ; midiEventReadIndex != midiEventWriteIndex;
         midiEventReadIndex = (midiEventReadIndex + 1) % EVENT_BUFFER_SIZE) {

	snd_seq_event_t *ev = &midiEventBuffer[midiEventReadIndex];

        if (!snd_seq_ev_is_channel_type(ev)) {
            /* discard non-channel oriented messages */
            continue;
        }
                
        i = 0; //instance->number;

        /* Stop processing incoming MIDI if an instance's event buffer is
         * full. */
	if (instanceEventCounts == EVENT_BUFFER_SIZE)
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

	if (ev->type == SND_SEQ_EVENT_CONTROLLER)
        {	    
	    int controller = ev->data.control.param;

	    if (controller == 0) 
            { 
                // bank select MSB
                
	    } else if (controller == 32) 
            { 
                // bank select LSB
	    } 
            else if (controller > 0 && controller < MIDI_CONTROLLER_COUNT) 
            {
                instanceEventBuffers[instanceEventCounts] = *ev;
                instanceEventCounts++;
	    }
	} 
        else 
        {
            instanceEventBuffers[instanceEventCounts] = *ev;
            instanceEventCounts++;
	}
    }

    i = 0;
    outCount = 0;
    outCount += this_instance->plugin->outs;
    
    v_pydaw_run(instanceHandles,  framesPerBuffer, instanceEventBuffers, instanceEventCounts);    
    
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

int main(int argc, char **argv)
{
    if(argc != 2)
    {
        printf("Usage: %s [install prefix]", argv[0]);
        exit(9996);
    }
    
    v_pydaw_constructor();
    int portid;
    int npfd;
    struct pollfd *pfd;

    d3h_dll_t *dll;
    d3h_plugin_t *plugin;
        
    int i, j;
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
    plugin->label = "pydaw";
    dll = (d3h_dll_t *)calloc(1, sizeof(d3h_dll_t));
    dll->name = "pydaw";
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

    /* set up instances */
    
    this_instance = (d3h_instance_t*)malloc(sizeof(d3h_instance_t));
    this_instance->plugin = plugin;    
    this_instance->friendly_name = "pydaw";    
        
    insTotal += plugin->ins;
    outsTotal += plugin->outs;
    controlInsTotal += plugin->controlIns;
    controlOutsTotal += plugin->controlOuts;
        
    pluginInputBuffers = (float **)malloc(insTotal * sizeof(float *));            
    pluginOutputBuffers = (float **)malloc(outsTotal * sizeof(float *));
    
    instanceHandles = (PYFX_Handle *)malloc(sizeof(PYFX_Handle));
    
    instanceEventCounts = 0;
    
    instanceEventBuffers = (snd_seq_event_t *)malloc(EVENT_BUFFER_SIZE * sizeof(snd_seq_event_t));
            
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
    
    char f_show_dialog_cmd[1024];
    sprintf(f_show_dialog_cmd, "python2 \"%s/lib/pydaw3/pydaw/python/libpydaw/pydaw_portaudio.py\"", argv[1]);
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
            f_device_name[0] = '\0';
            
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
                    printf("sampleRate: %i\n", (int)sample_rate);
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
                continue;
            }
            else
            {
                printf("It appears that the user closed the audio device dialog without choosing a device, exiting.");
                exit(9998);
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
                
        int f_i = 0;
        int f_found_index = 0;
        while(f_i < Pa_GetDeviceCount())
        {
            const PaDeviceInfo * f_padevice = Pa_GetDeviceInfo(f_i);  //I guess that this is not supposed to be free'd?
            if(!strcmp(f_padevice->name, f_device_name))
            {
                outputParameters.device = f_i;
                f_found_index = 1;
                break;
            }
            f_i++;
        }

        if(!f_found_index)
        {                
            sprintf(f_cmd_buffer, "%s \"Did not find device '%s' on this system.\"", f_show_dialog_cmd, f_device_name);
            system(f_cmd_buffer);
            continue;
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
       
        break;
    }

    in = 0;
    out = 0;
    i = 0;
    
    for (j = 0; j < this_instance->plugin->ins; ++j) 
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
    for (j = 0; j < this_instance->plugin->outs; ++j) 
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

    
    /* Instantiate plugins */

    i = 0;
    plugin = this_instance->plugin;
    instanceHandles = g_pydaw_instantiate(plugin->descriptor->PYFX_Plugin, sample_rate);
    if (!instanceHandles)
    {
        fprintf(stderr, "\n%s: Error: Failed to instantiate instance %d!, plugin \"%s\"\n",
                myName, i, plugin->label);
        return 1;
    }        

    /* Create OSC thread */

    serverThread = lo_server_thread_new("19271", osc_error);    
    lo_server_thread_add_method(serverThread, NULL, NULL, osc_message_handler, NULL);
    lo_server_thread_start(serverThread);

    /* Connect and activate plugins */

    in = out = controlIn = controlOut = 0;
    i = 0;        
    
    plugin = this_instance->plugin;
    for (j = 0; j < plugin->descriptor->PYFX_Plugin->PortCount; j++) // j is LADSPA port number
    {
        PYFX_PortDescriptor pod = plugin->descriptor->PYFX_Plugin->PortDescriptors[j];

        if (PYFX_IS_PORT_AUDIO(pod)) 
        {
            if (PYFX_IS_PORT_INPUT(pod)) 
            {
                v_pydaw_connect_port(instanceHandles, j, pluginInputBuffers[in++]);
            } 
            else if (PYFX_IS_PORT_OUTPUT(pod)) 
            {
                v_pydaw_connect_port(instanceHandles, j, pluginOutputBuffers[out++]);
            }
        }
    }  /* 'for (j...'  LADSPA port number */
    
    v_pydaw_activate(instanceHandles);    

    assert(in == insTotal);
    assert(out == outsTotal);
    assert(controlIn == controlInsTotal);
    assert(controlOut == controlOutsTotal);

    /* Create ALSA MIDI port */

    if (snd_seq_open(&alsaClient, "hw", SND_SEQ_OPEN_DUPLEX, 0) < 0) 
    {
	fprintf(stderr, "\n%s: Error: Failed to open ALSA sequencer interface\n", myName);
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
    
    MB_MESSAGE("Ready\n");

    exiting = 0;
    
    err = Pa_StartStream( stream );
    if( err != paNoError )
    {
        sprintf(f_cmd_buffer, "%s \"%s\"", f_show_dialog_cmd, "Error: Unknown error while starting device.  Please re-configure your device and try starting PyDAW again.");
        system(f_cmd_buffer);        
    }
    else
    {
        while (!exiting)
        {
            if (poll(pfd, npfd, 1000) > 0)
            {
                midi_callback();
            }
        }
    }
    
    err = Pa_CloseStream( stream );    
    Pa_Terminate();

    v_pydaw_cleanup(instanceHandles);    

    v_pydaw_destructor();
        
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

int osc_configure_handler(d3h_instance_t *instance, lo_arg **argv)
{
    const char *key = (const char *)&argv[0]->s;
    const char *value = (const char *)&argv[1]->s;
    
    v_pydaw_parse_configure_message(pydaw_data, key, value);
    
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
    const char *method;
    unsigned int flen = 0;
        
    if (strncmp(path, "/dssi/", 6))
    {
        printf("if (strncmp(path, \"/dssi/\", 6))\n");
        return osc_debug_handler(path, types, argv, argc, data, user_data);
    }
        
    method = path + 6 + flen;
        
    if (!strcmp(method, "pydaw/configure") && argc == 2 && !strcmp(types, "ss")) 
    {
        return osc_configure_handler(this_instance, argv);
    }

    return osc_debug_handler(path, types, argv, argc, data, user_data);
}


void v_pydaw_parse_configure_message(t_pydaw_data* a_pydaw_data, const char* a_key, const char* a_value)
{
    printf("v_pydaw_parse_configure_message:  key: \"%s\", value: \"%s\"\n", a_key, a_value);
    if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_UPDATE_PLUGIN_CONTROL)) //Set plugin control
    {
        t_1d_char_array * f_val_arr = c_split_str(a_value, '|', 5, LMS_TINY_STRING);
        int f_is_inst = atoi(f_val_arr->array[0]);
        int f_track_type = atoi(f_val_arr->array[1]);
        int f_track_num = atoi(f_val_arr->array[2]);
        int f_port = atoi(f_val_arr->array[3]);
        float f_value = atof(f_val_arr->array[4]);
        
        t_pydaw_plugin * f_instance;
        pthread_mutex_lock(&a_pydaw_data->main_mutex);
        //pthread_mutex_lock(&a_pydaw_data->track_pool[f_track_num]->mutex);
        switch(f_track_type)
        {
            case 0:  //MIDI track
                if(f_is_inst)
                {
                    f_instance = a_pydaw_data->track_pool[f_track_num]->instrument;
                }
                else
                {
                    f_instance = a_pydaw_data->track_pool[f_track_num]->effect;
                }
                break;
            case 1:  //Bus track
                f_instance = a_pydaw_data->bus_pool[f_track_num]->effect;
                break;
            case 2:  //Audio track
                f_instance = a_pydaw_data->audio_track_pool[f_track_num]->effect;
                break;
            default:
                assert(0);
                break;
        }
        
        int f_control_in = f_instance->pluginPortControlInNumbers[f_port];        
        f_instance->pluginControlIns[f_control_in] = f_value;
        //f_instance->pluginPortUpdated[f_control_in] = 1;
        pthread_mutex_unlock(&a_pydaw_data->main_mutex);
        //pthread_mutex_unlock(&a_pydaw_data->track_pool[f_track_num]->mutex);
        g_free_1d_char_array(f_val_arr);
    }    
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_CONFIGURE_PLUGIN)) //Configure plugin
    {
        t_1d_char_array * f_val_arr = c_split_str_remainder(a_value, '|', 5, LMS_TINY_STRING);
        int f_is_inst = atoi(f_val_arr->array[0]);
        int f_track_type = atoi(f_val_arr->array[1]);
        int f_track_num = atoi(f_val_arr->array[2]);
        char * f_key = f_val_arr->array[3];
        char * f_message = f_val_arr->array[4];
                
        t_pydaw_plugin * f_instance;
        //pthread_mutex_lock(&a_pydaw_data->main_mutex);
        //pthread_mutex_lock(&a_pydaw_data->track_pool[f_track_num]->mutex);
        switch(f_track_type)
        {
            case 0:  //MIDI track
                if(f_is_inst)
                {
                    f_instance = a_pydaw_data->track_pool[f_track_num]->instrument;
                }
                else
                {
                    f_instance = a_pydaw_data->track_pool[f_track_num]->effect;
                }
                break;
            case 1:  //Bus track
                f_instance = a_pydaw_data->bus_pool[f_track_num]->effect;
                break;
            case 2:  //Audio track
                f_instance = a_pydaw_data->audio_track_pool[f_track_num]->effect;
                break;
        }        
        
        v_pydaw_plugin_configure_handler(f_instance, f_key, f_message);
        //f_instance->pluginPortUpdated[f_control_in] = 1;
        //pthread_mutex_unlock(&a_pydaw_data->main_mutex);
        //pthread_mutex_unlock(&a_pydaw_data->track_pool[f_track_num]->mutex);
        g_free_1d_char_array(f_val_arr);
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_VOL)) //Set track volume
    {
        t_1d_char_array * f_val_arr = c_split_str(a_value, '|', 3, LMS_TINY_STRING);
        int f_track_num = atoi(f_val_arr->array[0]);
        float f_track_vol = atof(f_val_arr->array[1]);
        int f_track_type = atoi(f_val_arr->array[2]);
        pthread_mutex_lock(&a_pydaw_data->track_pool[f_track_num]->mutex);
        pthread_mutex_lock(&a_pydaw_data->main_mutex);
        
        switch(f_track_type)
        {
            case 0:  //MIDI track
                v_pydaw_set_track_volume(a_pydaw_data, a_pydaw_data->track_pool[f_track_num], f_track_vol);
                break;
            case 1:  //Bus track
                v_pydaw_set_track_volume(a_pydaw_data, a_pydaw_data->bus_pool[f_track_num], f_track_vol);
                break;
            case 2:  //Audio track
                v_pydaw_set_track_volume(a_pydaw_data, a_pydaw_data->audio_track_pool[f_track_num], f_track_vol);
                break;
            case 3:  //Audio Input
                a_pydaw_data->audio_inputs[f_track_num]->vol = f_track_vol;            
                a_pydaw_data->audio_inputs[f_track_num]->vol_linear = f_db_to_linear_fast(f_track_vol, a_pydaw_data->amp_ptr);
                break;
            default:
                assert(0);
                break;
        }
        
        pthread_mutex_unlock(&a_pydaw_data->main_mutex);
        pthread_mutex_unlock(&a_pydaw_data->track_pool[f_track_num]->mutex);
        g_free_1d_char_array(f_val_arr);
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_PER_AUDIO_ITEM_FX))
    {
        t_1d_char_array * f_arr = c_split_str(a_value, '|', 4, LMS_SMALL_STRING);
        int f_region_uid = atoi(f_arr->array[0]);
        int f_item_index = atoi(f_arr->array[1]);
        int f_port_num = atoi(f_arr->array[2]);
        float f_port_val = atof(f_arr->array[3]);
        
        v_paif_set_control(a_pydaw_data, f_region_uid, f_item_index, f_port_num, f_port_val);        
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_PLAY)) //Begin playback
    {
        t_1d_char_array * f_arr = c_split_str(a_value, '|', 2, LMS_SMALL_STRING);
        int f_region = atoi(f_arr->array[0]);
        int f_bar = atoi(f_arr->array[1]);
        v_set_playback_mode(a_pydaw_data, 1, f_region, f_bar);
        g_free_1d_char_array(f_arr);
    }    
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_REC)) //Begin recording
    {
        t_1d_char_array * f_arr = c_split_str(a_value, '|', 2, LMS_SMALL_STRING);
        int f_region = atoi(f_arr->array[0]);
        int f_bar = atoi(f_arr->array[1]);
        v_set_playback_mode(a_pydaw_data, 2, f_region, f_bar);
        g_free_1d_char_array(f_arr);
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_STOP)) //Stop playback or recording
    {
        v_set_playback_mode(a_pydaw_data, 0, -1, -1);
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_SR)) //Save region
    {        
        int f_uid = atoi(a_value);
        t_pyregion * f_result = g_pyregion_get(a_pydaw_data, f_uid);
        int f_region_index = i_get_song_index_from_region_uid(a_pydaw_data, f_uid);
                
        if(f_region_index >= 0 )
        {
            t_pyregion * f_old_region = NULL;
            if(a_pydaw_data->pysong->regions[f_region_index])
            {
                f_old_region = a_pydaw_data->pysong->regions[f_region_index];                
            }
            pthread_mutex_lock(&a_pydaw_data->main_mutex);
            a_pydaw_data->pysong->regions[f_region_index] = f_result;
            pthread_mutex_unlock(&a_pydaw_data->main_mutex);  
            if(f_old_region)
            {
                free(f_old_region);
            }
        }
        else
        {
            printf("region %i is not in song, not loading...", f_uid);
        }
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_SI)) //Save Item
    {
        pthread_mutex_lock(&a_pydaw_data->main_mutex);
        g_pyitem_get(a_pydaw_data, atoi(a_value));
        pthread_mutex_unlock(&a_pydaw_data->main_mutex);
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_SS))  //Save Song
    {        
        g_pysong_get(a_pydaw_data);        
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_AUDIO_ITEM_LOAD_ALL)) //Reload the entire audio items list
    {
        int f_uid = atoi(a_value);
        //t_pydaw_audio_items * f_old;
        t_pydaw_audio_items * f_result = v_audio_items_load_all(a_pydaw_data, f_uid);        
        int f_region_index = i_get_song_index_from_region_uid(a_pydaw_data, f_uid);
        pthread_mutex_lock(&a_pydaw_data->main_mutex);        
        a_pydaw_data->pysong->audio_items[f_region_index] = f_result;        
        pthread_mutex_unlock(&a_pydaw_data->main_mutex);
        //v_pydaw_audio_items_free(f_old); //Method needs to be re-thought...
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_PER_AUDIO_ITEM_FX_REGION))
    {
        int f_uid = atoi(a_value);
        t_pydaw_per_audio_item_fx_region * f_result = g_paif_region_open(a_pydaw_data, f_uid);
        int f_region_index = i_get_song_index_from_region_uid(a_pydaw_data, f_uid);
        t_pydaw_per_audio_item_fx_region * f_old = a_pydaw_data->pysong->per_audio_item_fx[f_region_index];
        pthread_mutex_lock(&a_pydaw_data->main_mutex);        
        a_pydaw_data->pysong->per_audio_item_fx[f_region_index] = f_result;        
        pthread_mutex_unlock(&a_pydaw_data->main_mutex);
        v_paif_region_free(f_old);
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_CREATE_SAMPLE_GRAPH)) //Create a .pygraph file for each .wav...
    {
        t_key_value_pair * f_kvp = g_kvp_get(a_value);
        char f_file_name_tmp[256];
        sprintf(f_file_name_tmp, "%s%s", a_pydaw_data->samplegraph_folder, f_kvp->key);
        v_pydaw_generate_sample_graph(f_kvp->value, f_file_name_tmp);     
        printf("v_wav_pool_add_item(a_pydaw_data->wav_pool, %i, \"%s\")\n", atoi(f_kvp->key), f_kvp->value);
        v_wav_pool_add_item(a_pydaw_data->wav_pool, atoi(f_kvp->key), f_kvp->value);
        free(f_kvp);
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_LOOP)) //Set loop mode
    {
        int f_value = atoi(a_value);
        v_set_loop_mode(a_pydaw_data, f_value);
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_OS)) //Open Song
    {        
        t_key_value_pair * f_kvp = g_kvp_get(a_value);
        int f_first_open = atoi(f_kvp->key);
        v_open_project(a_pydaw_data, f_kvp->value, f_first_open);        
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_TEMPO)) //Change tempo
    {
        v_set_tempo(a_pydaw_data, atof(a_value));
        g_pysong_get(a_pydaw_data);  //To reload audio items when tempo has changed
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_SOLO)) //Set track solo
    {
        t_1d_char_array * f_val_arr = c_split_str(a_value, '|', 3, LMS_TINY_STRING);
        int f_track_num = atoi(f_val_arr->array[0]);
        int f_mode = atoi(f_val_arr->array[1]);
        assert(f_mode == 0 || f_mode == 1);
        int f_track_type = atoi(f_val_arr->array[2]);
        pthread_mutex_lock(&a_pydaw_data->main_mutex);
        pthread_mutex_lock(&a_pydaw_data->track_pool[f_track_num]->mutex);
        switch(f_track_type)
        {
            case 0:  //MIDI
                a_pydaw_data->track_pool[f_track_num]->solo = f_mode;
                a_pydaw_data->track_pool[f_track_num]->current_period_event_index = 0;
                break;
            case 2:  //Audio
                a_pydaw_data->audio_track_pool[f_track_num]->solo = f_mode;
                a_pydaw_data->audio_track_pool[f_track_num]->current_period_event_index = 0;
                break;
            default:
                assert(0);
                break;
        }        
        v_pydaw_set_is_soloed(a_pydaw_data);
        
        pthread_mutex_unlock(&a_pydaw_data->main_mutex);
        pthread_mutex_unlock(&a_pydaw_data->track_pool[f_track_num]->mutex);
        g_free_1d_char_array(f_val_arr);        
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_MUTE)) //Set track mute
    {
        t_1d_char_array * f_val_arr = c_split_str(a_value, '|', 3, LMS_TINY_STRING);
        int f_track_num = atoi(f_val_arr->array[0]);
        int f_mode = atoi(f_val_arr->array[1]);
        assert(f_mode == 0 || f_mode == 1);
        int f_track_type = atoi(f_val_arr->array[2]);
        pthread_mutex_lock(&a_pydaw_data->track_pool[f_track_num]->mutex);
        pthread_mutex_lock(&a_pydaw_data->main_mutex);
        switch(f_track_type)
        {
            case 0:  //MIDI
                a_pydaw_data->track_pool[f_track_num]->mute = f_mode;
                a_pydaw_data->track_pool[f_track_num]->current_period_event_index = 0;
                break;
            case 2:  //Audio
                a_pydaw_data->audio_track_pool[f_track_num]->mute = f_mode;
                a_pydaw_data->audio_track_pool[f_track_num]->current_period_event_index = 0;
                break;
            default:
                assert(0);
                break;
        }        
        pthread_mutex_unlock(&a_pydaw_data->main_mutex);
        pthread_mutex_unlock(&a_pydaw_data->track_pool[f_track_num]->mutex);
        g_free_1d_char_array(f_val_arr);
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_REC_ARM_TRACK)) //Set track record arm
    {
        t_1d_char_array * f_val_arr = c_split_str(a_value, '|', 3, LMS_TINY_STRING);
        int f_type = atoi(f_val_arr->array[0]);
        int f_track_num = atoi(f_val_arr->array[1]);
        int f_mode = atoi(f_val_arr->array[2]);
        assert(f_mode == 0 || f_mode == 1);
        pthread_mutex_lock(&a_pydaw_data->main_mutex);
        if(f_mode)
        {
            switch(f_type)
            {
                case 0:  //MIDI/plugin                
                    a_pydaw_data->record_armed_track = a_pydaw_data->track_pool[f_track_num];
                    a_pydaw_data->record_armed_track_index_all = f_track_num;
                    break;
                case 1: //Bus
                    a_pydaw_data->record_armed_track = a_pydaw_data->bus_pool[f_track_num];
                    a_pydaw_data->record_armed_track_index_all = f_track_num + PYDAW_MIDI_TRACK_COUNT;
                    break;
                case 2: //audio track
                    a_pydaw_data->record_armed_track = a_pydaw_data->audio_track_pool[f_track_num];
                    a_pydaw_data->record_armed_track_index_all = f_track_num + PYDAW_MIDI_TRACK_COUNT + PYDAW_BUS_TRACK_COUNT;
                    break;
            }
            a_pydaw_data->record_armed_track_index = f_track_num;
            a_pydaw_data->record_armed_track_type = f_type;
        }   
        else
        {
            a_pydaw_data->record_armed_track = 0;
            a_pydaw_data->record_armed_track_index_all = -1;
            a_pydaw_data->record_armed_track_index = -1;
            a_pydaw_data->record_armed_track_type = -1;
        }
        pthread_mutex_unlock(&a_pydaw_data->main_mutex);
        g_free_1d_char_array(f_val_arr);
    }    
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_UPDATE_AUDIO_INPUTS)) //Change the plugin
    {
        v_pydaw_update_audio_inputs(a_pydaw_data);
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_CHANGE_INSTRUMENT)) //Change the plugin
    {
        t_1d_char_array * f_val_arr = c_split_str(a_value, '|', 2, LMS_TINY_STRING);
        int f_track_num = atoi(f_val_arr->array[0]);
        int f_plugin_index = atoi(f_val_arr->array[1]);
        v_set_plugin_index(a_pydaw_data,  a_pydaw_data->track_pool[f_track_num], f_plugin_index, 1);
        g_free_1d_char_array(f_val_arr);
    }    
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_PREVIEW_SAMPLE)) //Preview a sample
    {
        v_pydaw_set_preview_file(a_pydaw_data, a_value);
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_OFFLINE_RENDER)) //Render a project to .wav file
    {
        t_1d_char_array * f_val_arr = c_split_str(a_value, '|', 7, LMS_TINY_STRING);
        int f_start_region = atoi(f_val_arr->array[0]);
        int f_start_bar = atoi(f_val_arr->array[1]);
        int f_end_region = atoi(f_val_arr->array[2]);
        int f_end_bar = atoi(f_val_arr->array[3]);
        char * f_file_out = f_val_arr->array[4];
        
        v_pydaw_offline_render(a_pydaw_data, f_start_region, f_start_bar, f_end_region, f_end_bar, f_file_out, 0);
        g_free_1d_char_array(f_val_arr);
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_SET_TRACK_BUS)) //Set the bus number for specified track
    {
        t_1d_char_array * f_val_arr = c_split_str(a_value, '|', 3, LMS_TINY_STRING);
        int f_track_num = atoi(f_val_arr->array[0]);
        int f_bus_num = atoi(f_val_arr->array[1]);
        int f_track_type = atoi(f_val_arr->array[2]);
                
        switch(f_track_type)
        {
            case 0:  //MIDI track
                pthread_mutex_lock(&a_pydaw_data->main_mutex);
                a_pydaw_data->track_pool[f_track_num]->bus_num = f_bus_num;
                v_pydaw_schedule_work(a_pydaw_data);
                //v_pydaw_set_bus_counters(a_pydaw_data);
                pthread_mutex_unlock(&a_pydaw_data->main_mutex);
                break;
            case 2:  //Audio track
                pthread_mutex_lock(&a_pydaw_data->main_mutex);
                a_pydaw_data->audio_track_pool[f_track_num]->bus_num = f_bus_num;
                v_pydaw_schedule_work(a_pydaw_data);
                //v_pydaw_set_bus_counters(a_pydaw_data);
                pthread_mutex_unlock(&a_pydaw_data->main_mutex);
                break;
            default:
                printf("PYDAW_CONFIGURE_KEY_SET_TRACK_BUS:  Invalid track type %i\n\n", f_track_type);
                assert(0);
                break;
        }
        
        g_free_1d_char_array(f_val_arr);
    }       
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_SET_OVERDUB_MODE)) //Set the bus number for specified track
    {
        int f_bool = atoi(a_value);
        assert(f_bool == 0 || f_bool == 1);
        pthread_mutex_lock(&a_pydaw_data->main_mutex);
        a_pydaw_data->overdub_mode = f_bool;
        pthread_mutex_unlock(&a_pydaw_data->main_mutex);
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_LOAD_CC_MAP))
    {
        v_pydaw_load_cc_map(a_pydaw_data, a_value);
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_LOAD_AB_OPEN))
    {
        v_pydaw_set_ab_file(a_pydaw_data, a_value);
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_LOAD_AB_POS))
    {
        int f_pos = atoi(a_value);
        v_pydaw_set_ab_start(a_pydaw_data, f_pos);
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_LOAD_AB_SET))
    {
        int f_mode = atoi(a_value);
        v_pydaw_set_ab_mode(a_pydaw_data, f_mode);
    }    
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_LOAD_AB_VOL))
    {
        float f_vol = atof(a_value);
        v_pydaw_set_ab_vol(a_pydaw_data, f_vol);
    }    
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_PANIC))
    {
        long f_sample_off = a_pydaw_data->current_sample + 12000;
        int f_i = 0;
        int f_i2 = 0;
        
        pthread_mutex_lock(&a_pydaw_data->main_mutex);
        while(f_i < PYDAW_MIDI_TRACK_COUNT)
        {
            while(f_i2 < PYDAW_MIDI_NOTE_COUNT)
            {
                a_pydaw_data->note_offs[f_i][f_i2] = f_sample_off;
                f_i2++;
            }
            f_i++;
        }
        pthread_mutex_unlock(&a_pydaw_data->main_mutex);
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_CONV32F))
    {
        t_2d_char_array * f_arr = g_get_2d_array(LMS_SMALL_STRING);
        char f_tmp_char[LMS_SMALL_STRING];
        sprintf(f_tmp_char, "%s", a_value);
        f_arr->array = f_tmp_char;
        char * f_in_file = c_iterate_2d_char_array(f_arr);
        char * f_out_file = c_iterate_2d_char_array(f_arr);
        
        v_pydaw_convert_wav_to_32_bit_float(f_in_file, f_out_file);
        
        free(f_in_file);
        free(f_out_file);
        f_arr->array = 0;
        g_free_2d_char_array(f_arr);
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_RATE_ENV))
    {
        t_2d_char_array * f_arr = g_get_2d_array(LMS_SMALL_STRING);
        char f_tmp_char[LMS_SMALL_STRING];
        sprintf(f_tmp_char, "%s", a_value);
        f_arr->array = f_tmp_char;
        char * f_in_file = c_iterate_2d_char_array(f_arr);
        char * f_out_file = c_iterate_2d_char_array(f_arr);
        char * f_start_str = c_iterate_2d_char_array(f_arr);
        char * f_end_str = c_iterate_2d_char_array(f_arr);
        float f_start = atof(f_start_str);
        float f_end = atof(f_end_str);
        
        v_pydaw_rate_envelope(f_in_file, f_out_file, f_start, f_end);
        
        free(f_in_file);
        free(f_out_file);
        free(f_start_str);
        free(f_end_str);
        
        f_arr->array = 0;
        g_free_2d_char_array(f_arr);
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_PITCH_ENV))
    {
        t_2d_char_array * f_arr = g_get_2d_array(LMS_SMALL_STRING);
        char f_tmp_char[LMS_SMALL_STRING];
        sprintf(f_tmp_char, "%s", a_value);
        f_arr->array = f_tmp_char;
        char * f_in_file = c_iterate_2d_char_array(f_arr);
        char * f_out_file = c_iterate_2d_char_array(f_arr);
        char * f_start_str = c_iterate_2d_char_array(f_arr);
        char * f_end_str = c_iterate_2d_char_array(f_arr);
        float f_start = atof(f_start_str);
        float f_end = atof(f_end_str);
        
        v_pydaw_pitch_envelope(f_in_file, f_out_file, f_start, f_end);
        
        free(f_in_file);
        free(f_out_file);
        free(f_start_str);
        free(f_end_str);
        
        f_arr->array = 0;
        g_free_2d_char_array(f_arr);
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_GLUE_AUDIO_ITEMS))
    {
        t_pydaw_line_split * f_val_arr = g_split_line('|', a_value);
        char * f_path = f_val_arr->str_arr[0];  //Don't free this
        int f_region_index = atoi(f_val_arr->str_arr[1]);
        int f_start_bar = atoi(f_val_arr->str_arr[2]);
        int f_end_bar = atoi(f_val_arr->str_arr[3]);
        int f_i = 0;
        while(f_i < PYDAW_MAX_AUDIO_ITEM_COUNT)
        {
            a_pydaw_data->audio_glue_indexes[f_i] = 0;
            f_i++;
        }
        
        f_i = 4;
        while(f_i < f_val_arr->count)
        {
            int f_index = atoi(f_val_arr->str_arr[f_i]);
            a_pydaw_data->audio_glue_indexes[f_index] = 1;
            f_i++;
        }
        
        v_pydaw_offline_render(a_pydaw_data, f_region_index, f_start_bar, f_region_index, f_end_bar, f_path, 1);
        
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_EXIT))
    {
        exiting = 1;        
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_MIDI_LEARN))
    {
        int f_is_midi_learn = atoi(a_value);
        assert(f_is_midi_learn == 0 || f_is_midi_learn == 1);
        pydaw_data->midi_learn = f_is_midi_learn;
    }    
    else
    {
        printf("Unknown configure message key: %s, value %s\n", a_key, a_value);        
    }
}


