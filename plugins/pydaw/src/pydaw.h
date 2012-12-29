/* 
 * File:   pydaw.h
 * Author: jeffh
 *
 * Types and functions specific the the IO and file system specs of PyDAW 
 */


#ifndef PYDAW_H
#define	PYDAW_H

#ifdef	__cplusplus
extern "C" {
#endif
  
//Required for sched.h
#define __USE_GNU
    
//Uncomment this to constantly inspect the heap for known signs of corruption and throw a SIGABRT upon detection.
//#define PYDAW_MEMCHECK
    
//Print detailed information about the MIDI data being sent from PyDAW, or alternately pass in from 'make' with -DPYDAW_PRINT_DEBUG_INFO
//#define PYDAW_PRINT_DEBUG_INFO
    
#define PYDAW_CONFIGURE_KEY_SS "ss"
#define PYDAW_CONFIGURE_KEY_OS "os"
#define PYDAW_CONFIGURE_KEY_SI "si"
#define PYDAW_CONFIGURE_KEY_DI "di"
#define PYDAW_CONFIGURE_KEY_SR "sr"
#define PYDAW_CONFIGURE_KEY_DR "dr"
#define PYDAW_CONFIGURE_KEY_RR "rr"
#define PYDAW_CONFIGURE_KEY_RI "ri"
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
#define PYDAW_CONFIGURE_KEY_SAVE_TRACKS "st"
#define PYDAW_CONFIGURE_KEY_REC_ARM_TRACK "tr"
#define PYDAW_CONFIGURE_KEY_SHOW_FX_UI "fx"
#define PYDAW_CONFIGURE_KEY_TRACK_NAME "tn"
    
#define PYDAW_CONFIGURE_KEY_PREVIEW_SAMPLE "preview"
#define PYDAW_CONFIGURE_KEY_OFFLINE_RENDER "or"
    
#define PYDAW_CONFIGURE_KEY_SET_TRACK_BUS "bs"

#define PYDAW_CONFIGURE_KEY_AUDIO_LOAD "al"
#define PYDAW_CONFIGURE_KEY_AUDIO_SAVE "as"
#define PYDAW_CONFIGURE_KEY_AUDIO_SHOW_FX "af"
#define PYDAW_CONFIGURE_KEY_AUDIO_INPUT_SAVE "ai"
    
#define PYDAW_LOOP_MODE_OFF 0
#define PYDAW_LOOP_MODE_BAR 1
#define PYDAW_LOOP_MODE_REGION 2
    
#define PYDAW_PLAYBACK_MODE_OFF 0
#define PYDAW_PLAYBACK_MODE_PLAY 1
#define PYDAW_PLAYBACK_MODE_REC 2
    
//arbitrary, I may change these 3 after evaluating memory use vs. probable item count in a real project
#define PYDAW_MAX_ITEM_COUNT 5000
#define PYDAW_MAX_REGION_COUNT 300
#define PYDAW_MAX_EVENTS_PER_ITEM_COUNT 128
    
#define PYDAW_BUS_TRACK_COUNT 5
#define PYDAW_AUDIO_INPUT_TRACK_COUNT 5
#define PYDAW_AUDIO_TRACK_COUNT 8
#define PYDAW_MIDI_TRACK_COUNT 16
    
#define PYDAW_MAX_EVENT_BUFFER_SIZE 512  //This could probably be made smaller
#define PYDAW_MAX_REGION_SIZE 16
#define PYDAW_MIDI_NOTE_COUNT 128
#define PYDAW_MIDI_RECORD_BUFFER_LENGTH 600 //(PYDAW_MAX_REGION_COUNT * PYDAW_REGION_SIZE)  //recording buffer for MIDI, in bars
    
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ladspa.h>
#include "pydaw_files.h"
#include "pydaw_plugin.h"
#include <sys/stat.h>
#include <sched.h>
#include <unistd.h>
#include <sndfile.h>
#include <time.h>
#include "../../libmodsynth/lib/amp.h"
    
typedef struct st_pynote
{
    int note;
    int velocity;
    float start;
    float length;    
}t_pynote;

typedef struct st_pycc
{
    int cc_num;
    int cc_val;
    float start;
}t_pycc;

typedef struct st_pypitchbend
{
    int val;
    float start;
} t_pypitchbend;

typedef struct st_pyitem
{
    char * name;
    t_pynote * notes[PYDAW_MAX_EVENTS_PER_ITEM_COUNT];
    int note_count;    
    t_pycc * ccs[PYDAW_MAX_EVENTS_PER_ITEM_COUNT];
    int cc_count;
    t_pypitchbend * pitchbends[PYDAW_MAX_EVENTS_PER_ITEM_COUNT];
    int pitchbend_count;
    int note_index[PYDAW_MAX_EVENTS_PER_ITEM_COUNT];
    int cc_index[PYDAW_MAX_EVENTS_PER_ITEM_COUNT];
    int uid;
}t_pyitem;

typedef struct st_pyregion
{
    int item_indexes[PYDAW_MIDI_TRACK_COUNT][PYDAW_MAX_REGION_SIZE];  //Refers to the index of items in the master item pool 
    char * name;
    /*This flag is set to 1 if created during recording, signifying that it requires a default name to be created for it*/
    int not_yet_saved;
    
    int region_length_bars;    //0 to use pydaw_data default
    int region_length_beats;    //0 to use pydaw_data default
    int bar_length;  //0 to use pydaw_data default
    int alternate_tempo;  //0 or 1, used as a boolean
    float tempo;
}t_pyregion;

typedef struct st_pysong
{
    t_pyregion * regions[PYDAW_MAX_REGION_COUNT];
    int default_bar_length;
}t_pysong;

typedef struct st_pytrack
{    
    float volume;
    float volume_linear;
    int solo;
    int mute;
    int rec;
    int plugin_index;  //-1 == bus track, 0 == off, 1 == Euphoria, 2 == Ray-V
    int bus_num;
    snd_seq_event_t * event_buffer;
    int current_period_event_index;
    t_pydaw_plugin * instrument;
    t_pydaw_plugin * effect;
    pthread_mutex_t mutex;
    char name[64];
    int track_num;  //Track number for the pool of tracks it is in, for example: 0-15 for midi tracks
}t_pytrack;

typedef struct
{
    int track_number;    
}t_pydaw_work_queue_item;

typedef struct st_pydaw_data
{
    long sample_count;   //set from the Jack buffer size every time the main loop is called..
    float tempo;
    pthread_mutex_t main_mutex;
    t_pysong * pysong;
    t_pytrack * track_pool[PYDAW_MIDI_TRACK_COUNT];
    t_pytrack * bus_pool[PYDAW_BUS_TRACK_COUNT];
    t_pytrack * audio_track_pool[PYDAW_AUDIO_TRACK_COUNT];
    int track_current_item_note_event_indexes[PYDAW_MIDI_TRACK_COUNT];
    int track_current_item_cc_event_indexes[PYDAW_MIDI_TRACK_COUNT];
    int track_current_item_pitchbend_event_indexes[PYDAW_MIDI_TRACK_COUNT];
    int playback_mode;  //0 == Stop, 1 == Play, 2 == Rec
    int loop_mode;  //0 == Off, 1 == Bar, 2 == Region
    //char * project_name;
    char * project_folder;
    char * instruments_folder;
    char * item_folder;
    char * region_folder;    
    double playback_cursor; //only refers to the fractional position within the current bar.
    double playback_inc;  //the increment per-period to iterate through 1 bar, as determined by sample rate and tempo
    int current_region; //the current region
    int current_bar; //the current bar(0 to 7), within the current region    
    //int samples_per_bar;
    float sample_rate;
    long current_sample;  //The sample number of the exact point in the song, 0 == bar0/region0, 44100 == 1 second in at 44.1khz.
    long note_offs[PYDAW_MIDI_TRACK_COUNT][PYDAW_MIDI_NOTE_COUNT];  //When a note_on event is fired, a sample number of when to release it is stored here
    lo_server_thread serverThread;
    
    char * osc_url;
    
    float samples_per_beat;  //The number of samples per beat, for calculating length
    
    t_pyitem * item_pool[PYDAW_MAX_ITEM_COUNT];
        
    int item_count;
    int is_soloed;
    /*Records which beat since recording started, combined with start forms the exact time*/
    int recorded_notes_beat_tracker[PYDAW_MIDI_NOTE_COUNT];
    /*This refers to item index in the item pool*/
    int recorded_notes_item_tracker[PYDAW_MIDI_NOTE_COUNT];
    /*Counts the number of beats elapsed since record was pressed.*/
    int recorded_note_current_beat;
    /*The position of the note_on in the bar it was first pressed, in beats*/
    double recorded_notes_start_tracker[PYDAW_MIDI_NOTE_COUNT];
    int recorded_notes_velocity_tracker[PYDAW_MIDI_NOTE_COUNT];
    /*Boolean for whether the current bar has been added to the item pool*/
    int recording_in_current_bar;
    /*Then index of the item currently being recorded to*/
    int recording_current_item_pool_index;
    /*Used for suffixing file names when recording...  TODO:  A better system, like the GUI sending a 'track0-blah..' name for more uniqueness*/
    int record_name_index_items;
    int record_name_index_regions;
    /*item_pool_index of the first item recorded during the current record session*/
    int recording_first_item;
    t_amp * amp_ptr;
    
    pthread_mutex_t quit_mutex;  //must be acquired to free memory, to protect saving on exit...
    
    pthread_mutex_t track_cond_mutex;  //For signaling to process the instruments
    pthread_cond_t track_cond;   //For broadcasting to the threads that it's time to process the tracks
    pthread_mutex_t * track_block_mutexes;  //For preventing the main thread from continuing until the workers finish
    pthread_t * track_worker_threads;
    t_pydaw_work_queue_item ** track_work_queues;
    int * track_work_queue_counts;
    int track_worker_thread_count;
    int * track_thread_quit_notifier;
    int * track_thread_is_finished;
    
    pthread_mutex_t offline_mutex;  //used to prevent the main loop from processing during certain events...
    int is_offline_rendering;  //Used to artificially inject sleep into the loop to prevent a race condition
    
    int default_region_length_bars;
    int default_region_length_beats;
    int default_bar_length;
    
}t_pydaw_data;

typedef struct 
{
    t_pydaw_data * pydaw_data;
    int thread_num;
}t_pydaw_thread_args;

void g_pysong_get(t_pydaw_data*);
t_pytrack * g_pytrack_get(int);
t_pyregion * g_pyregion_get(t_pydaw_data* a_pydaw, const char*);
void g_pyitem_get(t_pydaw_data* a_pydaw, const char * a_name);
t_pycc * g_pycc_get(char a_cc_num, char a_cc_val, float a_start);
t_pypitchbend * g_pypitchbend_get(float a_start, int a_value);
t_pynote * g_pynote_get(char a_note, char a_vel, float a_start, float a_length);
t_pydaw_data * g_pydaw_data_get(float);
int i_get_region_index_from_name(t_pydaw_data * a_pydaw_data, const char * a_name);
void v_open_project(t_pydaw_data*, const char*);
void v_set_tempo(t_pydaw_data*,float);
void v_set_loop_mode(t_pydaw_data * a_pydaw_data, int a_mode);
void v_set_playback_cursor(t_pydaw_data * a_pydaw_data, int a_region, int a_bar);
void v_pydaw_parse_configure_message(t_pydaw_data*, const char*, const char*);
int i_pydaw_get_item_index_from_name(t_pydaw_data * a_pydaw_data, const char* a_name);
void v_set_plugin_index(t_pydaw_data * a_pydaw_data, t_pytrack * a_track, int a_index);
void v_pydaw_assert_memory_integrity(t_pydaw_data* a_pydaw_data);
int i_get_song_index_from_region_name(t_pydaw_data* a_pydaw_data, const char * a_region_name);
void v_save_pysong_to_disk(t_pydaw_data * a_pydaw_data);
void v_save_pyitem_to_disk(t_pydaw_data * a_pydaw_data, int a_index);
void v_save_pyregion_to_disk(t_pydaw_data * a_pydaw_data, int a_region_num);
void v_pydaw_save_plugin(t_pydaw_data * a_pydaw_data, t_pytrack * a_track, int a_is_fx);
void v_pydaw_open_plugin(t_pydaw_data * a_pydaw_data, t_pytrack * a_track, int a_is_fx);
int g_pyitem_get_new(t_pydaw_data* a_pydaw_data);
t_pyregion * g_pyregion_get_new(t_pydaw_data* a_pydaw_data);
void v_pydaw_set_track_volume(t_pydaw_data * a_pydaw_data,  t_pytrack * a_track, float a_vol);
inline void v_pydaw_update_ports(t_pydaw_plugin * a_plugin);
void * v_pydaw_worker_thread(void*);
void v_pydaw_init_worker_threads(t_pydaw_data*);
void v_open_default_project(t_pydaw_data * a_data);
inline void v_pydaw_process_external_midi(t_pydaw_data * pydaw_data, unsigned long sample_count, snd_seq_event_t *events, unsigned long event_count);
inline void v_pydaw_run_main_loop(t_pydaw_data * pydaw_data, unsigned long sample_count, 
        snd_seq_event_t *events, unsigned long event_count, long f_next_current_sample, LADSPA_Data *output0, LADSPA_Data *output1);
void v_pydaw_offline_render(t_pydaw_data * a_pydaw_data, int a_start_region, int a_start_bar, int a_end_region, 
        int a_end_bar, char * a_file_out);
inline void v_pydaw_schedule_work(t_pydaw_data * a_pydaw_data);
void v_pydaw_process_plugins_single_threaded(t_pydaw_data * a_pydaw_data);
void v_pydaw_print_benchmark(char * a_message, clock_t a_start);
void v_pydaw_init_busses(t_pydaw_data * a_pydaw_data);
/*End declarations.  Begin implementations.*/

void v_pydaw_init_busses(t_pydaw_data * a_pydaw_data)
{
    int f_i = 0;
    
    while(f_i < PYDAW_BUS_TRACK_COUNT)
    {
        v_set_plugin_index(a_pydaw_data, a_pydaw_data->track_pool[f_i], -1);
        f_i++;
    }
}
/* Create a clock_t with clock() when beginning some work, and use this function to print the completion time*/
void v_pydaw_print_benchmark(char * a_message, clock_t a_start)
{
    printf ( "\n\nCompleted %s in %f seconds\n", a_message, ( (double)clock() - a_start ) / CLOCKS_PER_SEC );
}

void v_pydaw_init_worker_threads(t_pydaw_data * a_pydaw_data)
{
    int f_i = 0;
    pthread_mutex_init(&a_pydaw_data->track_cond_mutex, NULL);
    pthread_cond_init(&a_pydaw_data->track_cond, NULL);
    a_pydaw_data->track_worker_thread_count = sysconf( _SC_NPROCESSORS_ONLN ) - 1;    
    if((a_pydaw_data->track_worker_thread_count) > 4)
    {
        a_pydaw_data->track_worker_thread_count = 4;
    }
    else if((a_pydaw_data->track_worker_thread_count) <= 0)
    {
        a_pydaw_data->track_worker_thread_count = 1;
    }
    
    a_pydaw_data->track_block_mutexes = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t) * (a_pydaw_data->track_worker_thread_count));
    a_pydaw_data->track_worker_threads = (pthread_t*)malloc(sizeof(pthread_t) * (a_pydaw_data->track_worker_thread_count));
    a_pydaw_data->track_work_queues = (t_pydaw_work_queue_item**)malloc(sizeof(t_pydaw_work_queue_item*) * (a_pydaw_data->track_worker_thread_count));
    a_pydaw_data->track_work_queue_counts = (int*)malloc(sizeof(int) * (a_pydaw_data->track_worker_thread_count));
    a_pydaw_data->track_thread_quit_notifier = (int*)malloc(sizeof(int) * (a_pydaw_data->track_worker_thread_count));
    a_pydaw_data->track_thread_is_finished = (int*)malloc(sizeof(int) * (a_pydaw_data->track_worker_thread_count));

    while(f_i < (a_pydaw_data->track_worker_thread_count))
    {        
        pthread_mutex_init(&a_pydaw_data->track_block_mutexes[f_i], NULL);
        a_pydaw_data->track_work_queues[f_i] = (t_pydaw_work_queue_item*)malloc(sizeof(t_pydaw_work_queue_item) * 32);  //Max 32 work items per thread...
        a_pydaw_data->track_work_queue_counts[f_i] = 0;
        a_pydaw_data->track_thread_is_finished[f_i] = 0;
        a_pydaw_data->track_thread_quit_notifier[f_i] = 0;
        t_pydaw_thread_args * f_args = (t_pydaw_thread_args*)malloc(sizeof(t_pydaw_thread_args));
        f_args->pydaw_data = a_pydaw_data;
        f_args->thread_num = f_i;     
        
        pthread_attr_t threadAttr;
        struct sched_param param;
        param.__sched_priority = 90;
        pthread_attr_init(&threadAttr);
        pthread_attr_setschedparam(&threadAttr, &param);
        pthread_attr_setstacksize(&threadAttr, 16777216); //8388608); 
        pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_DETACHED);
        pthread_create(&a_pydaw_data->track_worker_threads[f_i], &threadAttr, v_pydaw_worker_thread, (void*)f_args);
        pthread_attr_destroy(&threadAttr);
        
        f_i++;
    }
}

inline void v_pydaw_update_ports(t_pydaw_plugin * a_plugin)
{
    int f_i = 0;
    while(f_i < (a_plugin->controlIns))
    {
        if (a_plugin->pluginPortUpdated[f_i]) 
        {
            int port = a_plugin->pluginControlInPortNumbers[f_i];
            float value = a_plugin->pluginControlIns[f_i];

            a_plugin->pluginPortUpdated[f_i] = 0;
            if (a_plugin->uiTarget) 
            {
                lo_send(a_plugin->uiTarget, a_plugin->ui_osc_control_path, "if", port, value);
            }
        }
        f_i++;
    }
}

/*Mostly for offline rendering, where the performance hit is acceptable, but race conditions and possible data corruption are not...*/
void v_pydaw_process_plugins_single_threaded(t_pydaw_data * a_pydaw_data)
{
    int f_i = 0;
    
    while(f_i < PYDAW_MIDI_TRACK_COUNT)
    {
        if(a_pydaw_data->track_pool[f_i]->plugin_index > 0)
        {            
            v_pydaw_update_ports(a_pydaw_data->track_pool[f_i]->instrument);
            v_pydaw_update_ports(a_pydaw_data->track_pool[f_i]->effect);
            
            v_run_plugin(a_pydaw_data->track_pool[f_i]->instrument, (a_pydaw_data->sample_count), 
                    a_pydaw_data->track_pool[f_i]->event_buffer, 
                    a_pydaw_data->track_pool[f_i]->current_period_event_index);
                        
            memcpy(a_pydaw_data->track_pool[f_i]->effect->pluginInputBuffers[0], 
                    a_pydaw_data->track_pool[f_i]->instrument->pluginOutputBuffers[0], 
                    (a_pydaw_data->sample_count) * sizeof(LADSPA_Data));
            memcpy(a_pydaw_data->track_pool[f_i]->effect->pluginInputBuffers[1], 
                    a_pydaw_data->track_pool[f_i]->instrument->pluginOutputBuffers[1], 
                    (a_pydaw_data->sample_count) * sizeof(LADSPA_Data));
            
            v_run_plugin(a_pydaw_data->track_pool[f_i]->effect, (a_pydaw_data->sample_count), 
                    a_pydaw_data->track_pool[f_i]->event_buffer, 
                    a_pydaw_data->track_pool[f_i]->current_period_event_index);
                        
            a_pydaw_data->track_pool[f_i]->current_period_event_index = 0;
            
        }
        f_i++;
    }
}

void * v_pydaw_worker_thread(void* a_arg)
{
    t_pydaw_thread_args * f_args = (t_pydaw_thread_args*)(a_arg);
    /*
    struct sched_param param;
    int policy;
    pthread_t thread_id = pthread_self();
    pthread_getschedparam(thread_id, &policy, &param);

    printf("existing policy=%1d, priority=%1d\r\n",
    policy,param.sched_priority);

    policy = SCHED_RR;
    param.sched_priority = 90;
    pthread_setschedparam(thread_id, policy, &param);
        
    cpu_set_t cpuset;    
    CPU_ZERO(&cpuset);
    CPU_SET(f_args->thread_num, &cpuset);

    //pthread_t current_thread = pthread_self();    
    //pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);
    sched_setaffinity(0, sizeof(cpu_set_t), &cpuset);
    */
    
    while(1)
    {
        pthread_cond_wait(&f_args->pydaw_data->track_cond, &f_args->pydaw_data->track_cond_mutex);
        pthread_mutex_lock(&f_args->pydaw_data->track_block_mutexes[f_args->thread_num]);
        
        if(f_args->pydaw_data->track_thread_quit_notifier[f_args->thread_num])
        {            
            printf("worker thread %i exiting...\n", f_args->thread_num);
            pthread_mutex_unlock(&f_args->pydaw_data->track_block_mutexes[f_args->thread_num]);
            break;
        }
        
        int f_i = 0;
        while(f_i < f_args->pydaw_data->track_work_queue_counts[f_args->thread_num])
        {
            t_pydaw_work_queue_item f_item = f_args->pydaw_data->track_work_queues[f_args->thread_num][f_i];
            
            v_pydaw_update_ports(f_args->pydaw_data->track_pool[f_item.track_number]->instrument);
            v_pydaw_update_ports(f_args->pydaw_data->track_pool[f_item.track_number]->effect);
            
            v_run_plugin(f_args->pydaw_data->track_pool[f_item.track_number]->instrument, (f_args->pydaw_data->sample_count), 
                    f_args->pydaw_data->track_pool[f_item.track_number]->event_buffer, 
                    f_args->pydaw_data->track_pool[f_item.track_number]->current_period_event_index);
                        
            v_run_plugin(f_args->pydaw_data->track_pool[f_item.track_number]->effect, (f_args->pydaw_data->sample_count), 
                    f_args->pydaw_data->track_pool[f_item.track_number]->event_buffer, 
                    f_args->pydaw_data->track_pool[f_item.track_number]->current_period_event_index);
                        
            f_args->pydaw_data->track_pool[f_item.track_number]->current_period_event_index = 0;
                                    
            f_i++;
        }
        
        f_args->pydaw_data->track_thread_is_finished[f_args->thread_num] = 1;
        pthread_mutex_unlock(&f_args->pydaw_data->track_block_mutexes[f_args->thread_num]);
    }
    
    return (void*)1;
}

inline void v_pydaw_process_external_midi(t_pydaw_data * a_pydaw_data, unsigned long sample_count, snd_seq_event_t *events, unsigned long event_count)
{           
    int f_i = 0;

    while(f_i < PYDAW_MIDI_TRACK_COUNT)
    {
        if(a_pydaw_data->track_pool[f_i]->rec  && ((a_pydaw_data->track_pool[f_i]->plugin_index) != 0))
        {
            int f_i2 = 0;

            if(a_pydaw_data->playback_mode == 0)
            {
                a_pydaw_data->track_pool[f_i]->current_period_event_index = 0;
            }

            while(f_i2 < event_count)
            {
                /*TODO:  Determine if any unwanted/unused SND_SEQ_EVENT types might slip in and mess this up...*/
                if(a_pydaw_data->playback_mode == PYDAW_PLAYBACK_MODE_REC)
                {
                    /*Don't overwrite the reference to the current item until an event comes through in this bar*/
                    if(!a_pydaw_data->recording_in_current_bar && ((events[f_i2].type) != SND_SEQ_EVENT_NOTEOFF))  //Don't create an empty event for note_offs
                    {
                        a_pydaw_data->recording_in_current_bar = 1;
                        if(!a_pydaw_data->pysong->regions[(a_pydaw_data->current_region)])
                        {
#ifdef PYDAW_PRINT_DEBUG_INFO
                            printf("\nRecording:  Creating new empty region at %i\n\n", (a_pydaw_data->current_region));
#endif
                            a_pydaw_data->pysong->regions[(a_pydaw_data->current_region)] = g_pyregion_get_new(a_pydaw_data);
                        }
                        a_pydaw_data->recording_current_item_pool_index = g_pyitem_get_new(a_pydaw_data);
                        if((a_pydaw_data->recording_first_item) == -1)
                        {
                            a_pydaw_data->recording_first_item = (a_pydaw_data->recording_current_item_pool_index);
                        }
                        a_pydaw_data->pysong->regions[(a_pydaw_data->current_region)]->item_indexes[f_i][a_pydaw_data->current_bar] = (a_pydaw_data->recording_current_item_pool_index);
                        a_pydaw_data->pysong->regions[(a_pydaw_data->current_region)]->not_yet_saved = 1;
                    }
                }

                if(events[f_i2].type == SND_SEQ_EVENT_NOTEON)
                {
                    snd_seq_ev_note_t n = events[f_i2].data.note;
                    snd_seq_ev_clear(&a_pydaw_data->track_pool[f_i]->event_buffer[(a_pydaw_data->track_pool[f_i]->current_period_event_index)]);
                    snd_seq_ev_set_noteon(&a_pydaw_data->track_pool[f_i]->event_buffer[(a_pydaw_data->track_pool[f_i]->current_period_event_index)], 0,
                                n.note, n.velocity);
                    a_pydaw_data->track_pool[f_i]->event_buffer[(a_pydaw_data->track_pool[f_i]->current_period_event_index)].time.tick = 
                            (events[f_i2].time.tick);
                    a_pydaw_data->track_pool[f_i]->current_period_event_index = (a_pydaw_data->track_pool[f_i]->current_period_event_index) + 1;

                    if(a_pydaw_data->playback_mode == PYDAW_PLAYBACK_MODE_REC)
                    {
                        a_pydaw_data->recorded_notes_beat_tracker[n.note] = (a_pydaw_data->recorded_note_current_beat);
                        a_pydaw_data->recorded_notes_item_tracker[n.note] = (a_pydaw_data->recording_current_item_pool_index);
                        a_pydaw_data->recorded_notes_start_tracker[n.note] = 
                                ((a_pydaw_data->playback_cursor) + ((((double)(events[f_i2].time.tick))/((double)sample_count)) 
                                * (a_pydaw_data->playback_inc))) * 4.0f;
                        a_pydaw_data->recorded_notes_velocity_tracker[n.note] = n.velocity;
#ifdef PYDAW_PRINT_DEBUG_INFO
                        printf("\nRecording:  Initiating new note_on event\n\n");
#endif
                    }
                }
                else if(events[f_i2].type == SND_SEQ_EVENT_NOTEOFF)
                {
                    snd_seq_ev_note_t n = events[f_i2].data.note;
                    snd_seq_ev_clear(&a_pydaw_data->track_pool[f_i]->event_buffer[(a_pydaw_data->track_pool[f_i]->current_period_event_index)]);
                    snd_seq_ev_set_noteoff(&a_pydaw_data->track_pool[f_i]->event_buffer[(a_pydaw_data->track_pool[f_i]->current_period_event_index)], 0, n.note, 0);
                    a_pydaw_data->track_pool[f_i]->event_buffer[(a_pydaw_data->track_pool[f_i]->current_period_event_index)].time.tick = 
                            (events[f_i2].time.tick);

                    a_pydaw_data->track_pool[f_i]->current_period_event_index = (a_pydaw_data->track_pool[f_i]->current_period_event_index) + 1;

                    if(a_pydaw_data->playback_mode == PYDAW_PLAYBACK_MODE_REC)
                    {
                        double f_length = ((double)((a_pydaw_data->recorded_note_current_beat) - (a_pydaw_data->recorded_notes_beat_tracker[n.note])))
                                +  ((((a_pydaw_data->playback_cursor) + ((((double)(events[f_i2].time.tick))/((double)sample_count)) 
                                * (a_pydaw_data->playback_inc))) * 4.0f) - (a_pydaw_data->recorded_notes_start_tracker[n.note]));
#ifdef PYDAW_PRINT_DEBUG_INFO
                        printf("\nRecording:  Writing new note_on event f_length == %lf\n\n", f_length);
#endif
                        int f_index = (a_pydaw_data->recorded_notes_item_tracker[n.note]);                                
                        a_pydaw_data->item_pool[f_index]->notes[(a_pydaw_data->item_pool[f_index]->note_count)] = 
                                g_pynote_get(n.note,
                                a_pydaw_data->recorded_notes_velocity_tracker[n.note],
                                a_pydaw_data->recorded_notes_start_tracker[n.note],
                                f_length);
                        a_pydaw_data->item_pool[f_index]->note_count = (a_pydaw_data->item_pool[f_index]->note_count)  + 1;
                        /*Reset to -1 to invalidate the events*/
                        a_pydaw_data->recorded_notes_item_tracker[n.note] = -1;
                        a_pydaw_data->recorded_notes_beat_tracker[n.note] = -1;
                        a_pydaw_data->recorded_notes_velocity_tracker[n.note] = -1;
                    }
                }
                else if(events[f_i2].type == SND_SEQ_EVENT_PITCHBEND)
                {
                    snd_seq_ev_clear(&a_pydaw_data->track_pool[f_i]->event_buffer[(a_pydaw_data->track_pool[f_i]->current_period_event_index)]);
                    snd_seq_ev_set_pitchbend(&a_pydaw_data->track_pool[f_i]->event_buffer[(a_pydaw_data->track_pool[f_i]->current_period_event_index)], 
                            0, events[f_i2].data.control.value);
                    a_pydaw_data->track_pool[f_i]->current_period_event_index = (a_pydaw_data->track_pool[f_i]->current_period_event_index) + 1;

                    if(a_pydaw_data->playback_mode == PYDAW_PLAYBACK_MODE_REC)
                    {
                        int f_index = (a_pydaw_data->recording_current_item_pool_index);
                        double f_start =
                                ((a_pydaw_data->playback_cursor) + ((((double)(events[f_i2].time.tick))/((double)sample_count)) 
                                * (a_pydaw_data->playback_inc))) * 4.0f;
                        a_pydaw_data->item_pool[f_index]->pitchbends[(a_pydaw_data->item_pool[f_index]->pitchbend_count)] =
                                g_pypitchbend_get(f_start, events[f_i2].data.control.value);
                        a_pydaw_data->item_pool[f_index]->pitchbend_count = (a_pydaw_data->item_pool[f_index]->pitchbend_count) + 1;
                    }
                }
                else if(events[f_i2].type == SND_SEQ_EVENT_CONTROLLER)
                {
                    int controller = events[f_i2].data.control.param;
#ifdef PYDAW_PRINT_DEBUG_INFO
                    printf("\n\nALSA MIDI CC event:  c.param == %i, c.value == %i\n\n", events[f_i2].data.control.param, events[f_i2].data.control.value);
#endif
                    if (controller > 0) //&& controller < MIDI_CONTROLLER_COUNT) 
                    {
                        long controlIn;
                        if(a_pydaw_data->track_pool[f_i]->instrument)
                        {
                            controlIn = a_pydaw_data->track_pool[f_i]->instrument->controllerMap[controller];
                            
                            if (controlIn >= 0) 
                            {
                                /* controller is mapped to LADSPA port, update the port */
                                v_pydaw_set_control_from_cc(a_pydaw_data->track_pool[f_i]->instrument, controlIn, &events[f_i2], 0);

                            }
                        }
                        controlIn = a_pydaw_data->track_pool[f_i]->effect->controllerMap[controller];
                        if (controlIn >= 0) 
                        {
                            /* controller is mapped to LADSPA port, update the port */
                            v_pydaw_set_control_from_cc(a_pydaw_data->track_pool[f_i]->effect, controlIn, &events[f_i2], 0);

                        }
                        //Record the CC regardless of whether it was mapped or not...
                        if(a_pydaw_data->playback_mode == PYDAW_PLAYBACK_MODE_REC)
                        {
                            int f_index = (a_pydaw_data->recording_current_item_pool_index);
                            double f_start =
                                    ((a_pydaw_data->playback_cursor) + ((((double)(events[f_i2].time.tick))/((double)sample_count)) 
                                    * (a_pydaw_data->playback_inc))) * 4.0f;
                            a_pydaw_data->item_pool[f_index]->ccs[(a_pydaw_data->item_pool[f_index]->cc_count)] =
                                    g_pycc_get(events[f_i2].data.control.param, events[f_i2].data.control.value, f_start);
                            a_pydaw_data->item_pool[f_index]->cc_count = (a_pydaw_data->item_pool[f_index]->cc_count) + 1;
                        }
                    }
                }
                f_i2++;
            }
        }
        f_i++;
    }
}

inline void v_pydaw_schedule_work(t_pydaw_data * a_pydaw_data)
{
    int f_i = 0;

    pthread_mutex_lock(&a_pydaw_data->main_mutex);
    
    while(f_i < (a_pydaw_data->track_worker_thread_count))
    {
        a_pydaw_data->track_work_queue_counts[f_i] = 0;            
        f_i++;
    }

    f_i = 0;
    int f_thread_index = 0;

    /*Schedule all Euphoria instances on their own core if possible because it can use more CPU than Ray-V or Way-V*/
    while(f_i < PYDAW_MIDI_TRACK_COUNT)
    {   
        if(a_pydaw_data->track_pool[f_i]->plugin_index == 1)
        {
            a_pydaw_data->track_work_queues[f_thread_index][a_pydaw_data->track_work_queue_counts[f_thread_index]].track_number = f_i;
            a_pydaw_data->track_work_queue_counts[f_thread_index] = (a_pydaw_data->track_work_queue_counts[f_thread_index]) + 1;
            f_thread_index++;
            if(f_thread_index >= a_pydaw_data->track_worker_thread_count)
            {
                f_thread_index = 0;
            }
        }
        f_i++;
    }

    f_i = 0;
    
    /*Schedule all Way-V instances on their own core if possible because it can use more CPU than Ray-V*/
    while(f_i < PYDAW_MIDI_TRACK_COUNT)
    {   
        if(a_pydaw_data->track_pool[f_i]->plugin_index == 3)
        {
            a_pydaw_data->track_work_queues[f_thread_index][a_pydaw_data->track_work_queue_counts[f_thread_index]].track_number = f_i;
            a_pydaw_data->track_work_queue_counts[f_thread_index] = (a_pydaw_data->track_work_queue_counts[f_thread_index]) + 1;
            f_thread_index++;
            if(f_thread_index >= a_pydaw_data->track_worker_thread_count)
            {
                f_thread_index = 0;
            }
        }
        f_i++;
    }
    
    f_i = 0;

    /*Now schedule all Ray-V tracks*/
    while(f_i < PYDAW_MIDI_TRACK_COUNT)
    {   
        if(a_pydaw_data->track_pool[f_i]->plugin_index == 2)
        {
            a_pydaw_data->track_work_queues[f_thread_index][a_pydaw_data->track_work_queue_counts[f_thread_index]].track_number = f_i;
            a_pydaw_data->track_work_queue_counts[f_thread_index] = (a_pydaw_data->track_work_queue_counts[f_thread_index]) + 1;
            f_thread_index++;
            if(f_thread_index >= a_pydaw_data->track_worker_thread_count)
            {
                f_thread_index = 0;
            }
        }
        f_i++;
    }

    pthread_mutex_unlock(&a_pydaw_data->main_mutex);
}

inline void v_pydaw_run_main_loop(t_pydaw_data * a_pydaw_data, unsigned long sample_count, 
        snd_seq_event_t *events, unsigned long event_count, long f_next_current_sample, LADSPA_Data *output0, LADSPA_Data *output1)
{
    a_pydaw_data->sample_count = sample_count;
    if((a_pydaw_data->playback_mode) > 0)
    {
        double f_sample_period_inc = ((a_pydaw_data->playback_inc) * ((double)(sample_count)));
        double f_sample_period_inc_beats = f_sample_period_inc * 4.0f;
        double f_next_playback_cursor = (a_pydaw_data->playback_cursor) + f_sample_period_inc;        
        double f_current_period_beats = (a_pydaw_data->playback_cursor) * 4.0f;
        double f_next_period_beats = f_next_playback_cursor * 4.0f;

        int f_region_length_bars = a_pydaw_data->default_region_length_bars;        
        //double f_bar_length = (double)(a_pydaw_data->default_bar_length);
        
        if(a_pydaw_data->pysong->regions[(a_pydaw_data->current_region)])
        {
            if(a_pydaw_data->pysong->regions[(a_pydaw_data->current_region)]->bar_length)
            {
                //f_bar_length = (double)(a_pydaw_data->pysong->regions[(a_pydaw_data->current_region)]->bar_length);
            }

            if(a_pydaw_data->pysong->regions[(a_pydaw_data->current_region)]->region_length_bars)
            {
                f_region_length_bars = a_pydaw_data->pysong->regions[(a_pydaw_data->current_region)]->region_length_bars;

                if(a_pydaw_data->pysong->regions[(a_pydaw_data->current_region)]->region_length_beats)
                {
                    f_region_length_bars++;

                    if((a_pydaw_data->current_bar) == (f_region_length_bars - 1))
                    {                
                        //f_bar_length = (double)(a_pydaw_data->pysong->regions[(a_pydaw_data->current_region)]->region_length_beats);
                    }
                }
            }
        }
        int f_i = 0;

        a_pydaw_data->is_soloed = 0;

        while(f_i < PYDAW_MIDI_TRACK_COUNT)
        {
            if(a_pydaw_data->track_pool[f_i]->solo)
            {
                a_pydaw_data->is_soloed = 1;
                break;
            }
            f_i++;
        }

        //Calculate track notes for this period and send them to instruments
        f_i = 0;
        while(f_i < PYDAW_MIDI_TRACK_COUNT)
        {   
            /* Situations where the track is effectively muted*/
            if((a_pydaw_data->track_pool[f_i]->plugin_index == 0) ||
                (a_pydaw_data->track_pool[f_i]->mute) ||
                ((a_pydaw_data->is_soloed) && (!a_pydaw_data->track_pool[f_i]->solo)) )
            {
                f_i++;
                continue;
            }

            int f_current_track_region = a_pydaw_data->current_region;
            int f_current_track_bar = a_pydaw_data->current_bar;
            double f_track_current_period_beats = f_current_period_beats;
            double f_track_next_period_beats = f_next_period_beats;
            double f_track_beats_offset = 0.0f;

            if((a_pydaw_data->playback_mode == 2) && (a_pydaw_data->track_pool[f_i]->rec))
            {

            }
            else
            {
                while(1)
                {
                    if((a_pydaw_data->pysong->regions[f_current_track_region]) && 
                        (a_pydaw_data->pysong->regions[f_current_track_region]->item_indexes[f_i][f_current_track_bar] != -1))
                    {
                        t_pyitem f_current_item = 
                                *(a_pydaw_data->item_pool[(a_pydaw_data->pysong->regions[f_current_track_region]->item_indexes[f_i][f_current_track_bar])]);

                        if((a_pydaw_data->track_current_item_note_event_indexes[f_i]) >= (f_current_item.note_count))
                        {
                            if(f_track_next_period_beats >= 4.0f)
                            {
                                f_track_current_period_beats = 0.0f;
                                f_track_next_period_beats = f_track_next_period_beats - 4.0f;
                                f_track_beats_offset = (f_sample_period_inc * 4.0f) - f_track_next_period_beats;

                                a_pydaw_data->track_current_item_note_event_indexes[f_i] = 0;

                                if(a_pydaw_data->loop_mode != PYDAW_LOOP_MODE_BAR)
                                {
                                    f_current_track_bar++;

                                    if(f_current_track_bar >= f_region_length_bars)
                                    {
                                        f_current_track_bar = 0;

                                        if(a_pydaw_data->loop_mode != PYDAW_LOOP_MODE_REGION)
                                        {
                                            f_current_track_region++;
                                        }
                                    }
                                }

                                continue;
                            }
                            else
                            {
                                break;
                            }                            
                        }

                        if(((f_current_item.notes[(a_pydaw_data->track_current_item_note_event_indexes[f_i])]->start) >= f_track_current_period_beats) &&
                            ((f_current_item.notes[(a_pydaw_data->track_current_item_note_event_indexes[f_i])]->start) < f_track_next_period_beats))
                        {
                            int f_note_sample_offset = 0;
                            float f_note_start_diff = ((f_current_item.notes[(a_pydaw_data->track_current_item_note_event_indexes[f_i])]->start) - f_track_current_period_beats) + f_track_beats_offset;
                            float f_note_start_frac = f_note_start_diff / f_sample_period_inc_beats;
                            f_note_sample_offset =  (int)(f_note_start_frac * ((float)sample_count));

                            if((a_pydaw_data->note_offs[f_i][(f_current_item.notes[(a_pydaw_data->track_current_item_note_event_indexes[f_i])]->note)]) 
                                    >= (a_pydaw_data->current_sample))                                    
                            {  //There's already a note_off scheduled ahead of this one, process it immediately to avoid hung notes                                    
                                snd_seq_ev_clear(&a_pydaw_data->track_pool[f_i]->event_buffer[(a_pydaw_data->track_pool[f_i]->current_period_event_index)]);

                                snd_seq_ev_set_noteoff(&a_pydaw_data->track_pool[f_i]->event_buffer[(a_pydaw_data->track_pool[f_i]->current_period_event_index)], 0, 
                                        (f_current_item.notes[(a_pydaw_data->track_current_item_note_event_indexes[f_i])]->note), 0);
                                a_pydaw_data->track_pool[f_i]->event_buffer[(a_pydaw_data->track_pool[f_i]->current_period_event_index)].time.tick = f_note_sample_offset;

                                a_pydaw_data->track_pool[f_i]->current_period_event_index = (a_pydaw_data->track_pool[f_i]->current_period_event_index) + 1;
                            }

                            snd_seq_ev_clear(&a_pydaw_data->track_pool[f_i]->event_buffer[(a_pydaw_data->track_pool[f_i]->current_period_event_index)]);

#ifdef PYDAW_PRINT_DEBUG_INFO
                            printf("\n\nSending note_on event\nf_i = %i, f_note_start_diff = %f, f_sample_period_inc_beats = %f, f_note_start_frac = %f, f_note_sample_offset = %i, sample_count = %i, pydaw_data->current_sample = %ld\n\n", 
                                    f_i, f_note_start_diff, f_sample_period_inc_beats, f_note_start_frac, f_note_sample_offset, (int)sample_count, a_pydaw_data->current_sample);
#endif
                            snd_seq_ev_set_noteon(&a_pydaw_data->track_pool[f_i]->event_buffer[(a_pydaw_data->track_pool[f_i]->current_period_event_index)], 0,
                                    f_current_item.notes[(a_pydaw_data->track_current_item_note_event_indexes[f_i])]->note,
                                    f_current_item.notes[(a_pydaw_data->track_current_item_note_event_indexes[f_i])]->velocity);
#ifdef PYDAW_PRINT_DEBUG_INFO                                
                            printf("\nsnd_seq_ev_set_noteon(event_buffer[%i], 0, %i, %i)\n", a_pydaw_data->track_pool[f_i]->current_period_event_index,
                                    f_current_item.notes[(a_pydaw_data->track_current_item_note_event_indexes[f_i])]->note,
                                    f_current_item.notes[(a_pydaw_data->track_current_item_note_event_indexes[f_i])]->velocity);
#endif                            

                            a_pydaw_data->track_pool[f_i]->event_buffer[(a_pydaw_data->track_pool[f_i]->current_period_event_index)].time.tick = f_note_sample_offset;

                            a_pydaw_data->track_pool[f_i]->current_period_event_index = (a_pydaw_data->track_pool[f_i]->current_period_event_index) + 1;

                            a_pydaw_data->note_offs[f_i][(f_current_item.notes[(a_pydaw_data->track_current_item_note_event_indexes[f_i])]->note)] = (a_pydaw_data->current_sample) + 
                                    ((int)(f_current_item.notes[(a_pydaw_data->track_current_item_note_event_indexes[f_i])]->length * (a_pydaw_data->samples_per_beat)));

                            //This assert will need to be adjusted once playback is allowed to start from anywhere other than bar:0, region:0                        
                            //int f_calculated_sample = ((int)(((((float)(pydaw_data->current_bar))  * 4.0f) +  (f_current_item.notes[(pydaw_data->track_note_event_indexes[f_i])]->start)) * pydaw_data->samples_per_beat));
                            //int f_real_sample = (f_note_sample_offset + (pydaw_data->current_sample));
                            //assert(f_calculated_sample == f_real_sample);

                            a_pydaw_data->track_current_item_note_event_indexes[f_i] = (a_pydaw_data->track_current_item_note_event_indexes[f_i]) + 1;
                        }
                        else
                        {
                            break;
                        }
                    }
                    else
                    {
                        if(f_track_next_period_beats >= 4.0f)
                        {
                            f_track_current_period_beats = 0.0f;
                            f_track_next_period_beats = f_track_next_period_beats - 4.0f;
                            f_track_beats_offset = (f_sample_period_inc * 4.0f) - f_track_next_period_beats;

                            a_pydaw_data->track_current_item_note_event_indexes[f_i] = 0;

                            if(a_pydaw_data->loop_mode != PYDAW_LOOP_MODE_BAR)
                            {
                                f_current_track_bar++;

                                if(f_current_track_bar >= f_region_length_bars)
                                {
                                    f_current_track_bar = 0;

                                    if(a_pydaw_data->loop_mode != PYDAW_LOOP_MODE_REGION)
                                    {
                                        f_current_track_region++;
                                    }
                                }
                            }

                            continue;
                        }
                        else
                        {
                            break;
                        }
                    }
                }
            }

            f_i++;
        } //while notes


        //Calculate track CCs for this period and update the controller ports
        f_i = 0;
        while(f_i < PYDAW_MIDI_TRACK_COUNT)
        {   
            /* Situations where the track is effectively muted*/
            if((a_pydaw_data->track_pool[f_i]->plugin_index == 0) ||
                (a_pydaw_data->track_pool[f_i]->mute) ||
                ((a_pydaw_data->is_soloed) && (!a_pydaw_data->track_pool[f_i]->solo)))
            {
                f_i++;
                continue;
            }

            int f_current_track_region = a_pydaw_data->current_region;
            int f_current_track_bar = a_pydaw_data->current_bar;
            double f_track_current_period_beats = f_current_period_beats;
            double f_track_next_period_beats = f_next_period_beats;
            //double f_track_beats_offset = 0.0f;

            if((a_pydaw_data->playback_mode == 2) && (a_pydaw_data->track_pool[f_i]->rec))
            {

            }
            else
            {
                while(1)
                {
                    if((a_pydaw_data->pysong->regions[f_current_track_region]) && 
                        (a_pydaw_data->pysong->regions[f_current_track_region]->item_indexes[f_i][f_current_track_bar] != -1))
                    {
                        t_pyitem f_current_item = 
                                *(a_pydaw_data->item_pool[(a_pydaw_data->pysong->regions[f_current_track_region]->item_indexes[f_i][f_current_track_bar])]);

                        if((a_pydaw_data->track_current_item_cc_event_indexes[f_i]) >= (f_current_item.cc_count))
                        {
                            if(f_track_next_period_beats >= 4.0f)
                            {
                                f_track_current_period_beats = 0.0f;
                                f_track_next_period_beats = f_track_next_period_beats - 4.0f;
                                //f_track_beats_offset = (f_sample_period_inc * 4.0f) - f_track_next_period_beats;

                                //pydaw_data->track_note_event_indexes[f_i] = 0;
                                a_pydaw_data->track_current_item_cc_event_indexes[f_i] = 0;

                                if(a_pydaw_data->loop_mode != PYDAW_LOOP_MODE_BAR)
                                {
                                    f_current_track_bar++;

                                    if(f_current_track_bar >= f_region_length_bars)
                                    {
                                        f_current_track_bar = 0;

                                        if(a_pydaw_data->loop_mode != PYDAW_LOOP_MODE_REGION)
                                        {
                                            f_current_track_region++;
                                        }
                                    }
                                }

                                continue;
                            }
                            else
                            {
                                break;
                            }                            
                        }

                        if(((f_current_item.ccs[(a_pydaw_data->track_current_item_cc_event_indexes[f_i])]->start) >= f_track_current_period_beats) &&
                            ((f_current_item.ccs[(a_pydaw_data->track_current_item_cc_event_indexes[f_i])]->start) < f_track_next_period_beats))
                        {
                            //
                            int controller = f_current_item.ccs[(a_pydaw_data->track_current_item_cc_event_indexes[f_i])]->cc_num;
                            if (controller > 0) //&& controller < MIDI_CONTROLLER_COUNT) 
                            {
                                long controlIn;
                                if(a_pydaw_data->track_pool[f_i]->instrument)
                                {
                                    controlIn = a_pydaw_data->track_pool[f_i]->instrument->controllerMap[controller];

                                    if (controlIn >= 0)
                                    {
                                        /* controller is mapped to LADSPA port, update the port */
                                        snd_seq_event_t f_event;
                                        f_event.data.control.value = f_current_item.ccs[(a_pydaw_data->track_current_item_cc_event_indexes[f_i])]->cc_val;
                                        v_pydaw_set_control_from_cc(a_pydaw_data->track_pool[f_i]->instrument, controlIn, &f_event, 0);
                                    }
                                }

                                controlIn = a_pydaw_data->track_pool[f_i]->effect->controllerMap[controller];

                                if (controlIn >= 0)
                                {
                                    /* controller is mapped to LADSPA port, update the port */
                                    snd_seq_event_t f_event;
                                    f_event.data.control.value = f_current_item.ccs[(a_pydaw_data->track_current_item_cc_event_indexes[f_i])]->cc_val;
                                    v_pydaw_set_control_from_cc(a_pydaw_data->track_pool[f_i]->effect, controlIn, &f_event, 0);
                                }
                            }

                            a_pydaw_data->track_current_item_cc_event_indexes[f_i] = (a_pydaw_data->track_current_item_cc_event_indexes[f_i]) + 1;
                        }
                        else
                        {
                            break;
                        }
                    }                        
                    else
                    {
                        if(f_track_next_period_beats >= 4.0f)
                        {
                            f_track_current_period_beats = 0.0f;
                            f_track_next_period_beats = f_track_next_period_beats - 4.0f;
                            //f_track_beats_offset = (f_sample_period_inc * 4.0f) - f_track_next_period_beats;

                            a_pydaw_data->track_current_item_cc_event_indexes[f_i] = 0;

                            if(a_pydaw_data->loop_mode != PYDAW_LOOP_MODE_BAR)
                            {
                                f_current_track_bar++;

                                if(f_current_track_bar >= f_region_length_bars)
                                {
                                    f_current_track_bar = 0;

                                    if(a_pydaw_data->loop_mode != PYDAW_LOOP_MODE_REGION)
                                    {
                                        f_current_track_region++;
                                    }
                                }
                            }

                            continue;
                        }
                        else
                        {
                            break;
                        }
                    }
                }
            }

            f_i++;
        } //while CCs


        //Calculate track pitchbends for this period and update the controller ports
        f_i = 0;
        while(f_i < PYDAW_MIDI_TRACK_COUNT)
        {   
            /* Situations where the track is effectively muted*/
            if((a_pydaw_data->track_pool[f_i]->plugin_index == 0) ||
                (a_pydaw_data->track_pool[f_i]->mute) ||
                ((a_pydaw_data->is_soloed) && (!a_pydaw_data->track_pool[f_i]->solo)) )
            {
                f_i++;
                continue;
            }

            int f_current_track_region = a_pydaw_data->current_region;
            int f_current_track_bar = a_pydaw_data->current_bar;
            double f_track_current_period_beats = f_current_period_beats;
            double f_track_next_period_beats = f_next_period_beats;
            //double f_track_beats_offset = 0.0f;

            if((a_pydaw_data->playback_mode == 2) && (a_pydaw_data->track_pool[f_i]->rec))
            {

            }
            else
            {
                while(1)
                {
                    if((a_pydaw_data->pysong->regions[f_current_track_region]) && 
                        (a_pydaw_data->pysong->regions[f_current_track_region]->item_indexes[f_i][f_current_track_bar] != -1))
                    {
                        t_pyitem f_current_item = 
                                *(a_pydaw_data->item_pool[(a_pydaw_data->pysong->regions[f_current_track_region]->item_indexes[f_i][f_current_track_bar])]);

                        if((a_pydaw_data->track_current_item_pitchbend_event_indexes[f_i]) >= (f_current_item.pitchbend_count))
                        {
                            if(f_track_next_period_beats >= 4.0f)
                            {
                                f_track_current_period_beats = 0.0f;
                                f_track_next_period_beats = f_track_next_period_beats - 4.0f;
                                //f_track_beats_offset = (f_sample_period_inc * 4.0f) - f_track_next_period_beats;

                                //pydaw_data->track_note_event_indexes[f_i] = 0;
                                a_pydaw_data->track_current_item_pitchbend_event_indexes[f_i] = 0;

                                if(a_pydaw_data->loop_mode != PYDAW_LOOP_MODE_BAR)
                                {
                                    f_current_track_bar++;

                                    if(f_current_track_bar >= f_region_length_bars)
                                    {
                                        f_current_track_bar = 0;

                                        if(a_pydaw_data->loop_mode != PYDAW_LOOP_MODE_REGION)
                                        {
                                            f_current_track_region++;
                                        }
                                    }
                                }

                                continue;
                            }
                            else
                            {
                                break;
                            }                            
                        }

                        if(((f_current_item.pitchbends[(a_pydaw_data->track_current_item_pitchbend_event_indexes[f_i])]->start) >= f_track_current_period_beats) &&
                            ((f_current_item.pitchbends[(a_pydaw_data->track_current_item_pitchbend_event_indexes[f_i])]->start) < f_track_next_period_beats))
                        {
                            snd_seq_ev_clear(&a_pydaw_data->track_pool[f_i]->event_buffer[(a_pydaw_data->track_pool[f_i]->current_period_event_index)]);
                            snd_seq_ev_set_pitchbend(
                                    &a_pydaw_data->track_pool[f_i]->event_buffer[(a_pydaw_data->track_pool[f_i]->current_period_event_index)],
                                    0, 
                                    f_current_item.pitchbends[(a_pydaw_data->track_current_item_pitchbend_event_indexes[f_i])]->val);

                            a_pydaw_data->track_pool[f_i]->current_period_event_index = (a_pydaw_data->track_pool[f_i]->current_period_event_index) + 1;                                
                            a_pydaw_data->track_current_item_pitchbend_event_indexes[f_i] = (a_pydaw_data->track_current_item_pitchbend_event_indexes[f_i]) + 1;
                        }
                        else
                        {
                            break;
                        }
                    }                        
                    else
                    {
                        if(f_track_next_period_beats >= 4.0f)
                        {
                            f_track_current_period_beats = 0.0f;
                            f_track_next_period_beats = f_track_next_period_beats - 4.0f;
                            //f_track_beats_offset = (f_sample_period_inc * 4.0f) - f_track_next_period_beats;

                            a_pydaw_data->track_current_item_pitchbend_event_indexes[f_i] = 0;

                            if(a_pydaw_data->loop_mode != PYDAW_LOOP_MODE_BAR)
                            {
                                f_current_track_bar++;

                                if(f_current_track_bar >= f_region_length_bars)
                                {
                                    f_current_track_bar = 0;

                                    if(a_pydaw_data->loop_mode != PYDAW_LOOP_MODE_REGION)
                                    {
                                        f_current_track_region++;
                                    }
                                }
                            }

                            continue;
                        }
                        else
                        {
                            break;
                        }
                    }
                }
            }
            f_i++;
        } //while pitchbends

        v_pydaw_process_external_midi(a_pydaw_data, sample_count, events, event_count);

        a_pydaw_data->playback_cursor = f_next_playback_cursor;

        if((a_pydaw_data->playback_cursor) >= 1.0f)
        {
            a_pydaw_data->recording_in_current_bar = 0;
            a_pydaw_data->recorded_note_current_beat = (a_pydaw_data->recorded_note_current_beat) + 4;

            a_pydaw_data->playback_cursor = (a_pydaw_data->playback_cursor) - 1.0f;                

            if(a_pydaw_data->loop_mode != PYDAW_LOOP_MODE_BAR)
            {
                a_pydaw_data->current_bar = (a_pydaw_data->current_bar) + 1;

                if((a_pydaw_data->current_bar) >= f_region_length_bars)
                {
                    a_pydaw_data->current_bar = 0;

                    if(a_pydaw_data->loop_mode != PYDAW_LOOP_MODE_REGION)
                    {
                        a_pydaw_data->current_region = (a_pydaw_data->current_region) + 1;

                        if((a_pydaw_data->current_region) >= PYDAW_MAX_REGION_COUNT)
                        {
                            a_pydaw_data->playback_mode = 0;
                            a_pydaw_data->current_region = 0;
                        }
                    }
                }
            }
#ifdef PYDAW_PRINT_DEBUG_INFO
            printf("pydaw_data->current_region == %i, pydaw_data->current_bar == %i\n", (a_pydaw_data->current_region), (a_pydaw_data->current_bar));                
#endif
        }
    } //If playback_mode > 0
    else
    {
        v_pydaw_process_external_midi(a_pydaw_data, sample_count, events, event_count);  //Process external MIDI if playback/recording are stopped...
    }        

    int f_i = 0;

    while(f_i < PYDAW_MIDI_TRACK_COUNT)
    {   
        if((a_pydaw_data->track_pool[f_i]->plugin_index) > 0)  //TODO:  When/if effects can ever accept MIDI notes, this will actually result in hung notes...
        {
            int f_i2 = 0;

            while(f_i2 < PYDAW_MIDI_NOTE_COUNT)
            {
                if((a_pydaw_data->note_offs[f_i][f_i2]) >= (a_pydaw_data->current_sample) &&
                   (a_pydaw_data->note_offs[f_i][f_i2]) < f_next_current_sample)
                {
#ifdef PYDAW_PRINT_DEBUG_INFO                    
                    printf("\n\nSending note_off event\nf_i = %i, pydaw_data->note_offs[f_i][f_i2] = %ld, pydaw_data->current_sample = %ld\n\n", 
                                f_i, a_pydaw_data->note_offs[f_i][f_i2], a_pydaw_data->current_sample);

                    assert(((a_pydaw_data->note_offs[f_i][f_i2]) >= (a_pydaw_data->current_sample)) || ((a_pydaw_data->note_offs[f_i][f_i2]) == -1));
#endif                    
                    snd_seq_ev_clear(&a_pydaw_data->track_pool[f_i]->event_buffer[(a_pydaw_data->track_pool[f_i]->current_period_event_index)]);

                    snd_seq_ev_set_noteoff(&a_pydaw_data->track_pool[f_i]->event_buffer[(a_pydaw_data->track_pool[f_i]->current_period_event_index)], 0, f_i2, 0);
                    a_pydaw_data->track_pool[f_i]->event_buffer[(a_pydaw_data->track_pool[f_i]->current_period_event_index)].time.tick = 
                            (a_pydaw_data->note_offs[f_i][f_i2]) - (a_pydaw_data->current_sample);

                    a_pydaw_data->track_pool[f_i]->current_period_event_index = (a_pydaw_data->track_pool[f_i]->current_period_event_index) + 1;

                    a_pydaw_data->note_offs[f_i][f_i2] = -1;
                }
                f_i2++;
            }
        }
        f_i++;
    }

    //if(a_pydaw_data->is_offline_rendering)
    //{
    //   v_pydaw_process_plugins_single_threaded(a_pydaw_data);
    //}
    //else
    //{                    
        f_i = 0;

        while(f_i < (a_pydaw_data->track_worker_thread_count))
        {
            a_pydaw_data->track_thread_is_finished[f_i] = 0;
            f_i++;
        }
        //notify the worker threads
        pthread_mutex_lock(&a_pydaw_data->track_cond_mutex);
        pthread_cond_broadcast(&a_pydaw_data->track_cond);
        pthread_mutex_unlock(&a_pydaw_data->track_cond_mutex);
        
        f_i = 0;
        
        while(f_i < PYDAW_BUS_TRACK_COUNT)
        {
            v_pydaw_update_ports(a_pydaw_data->bus_pool[f_i]->effect);
            
            int f_i2 = 0;
            while(f_i2 < sample_count)
            {
                a_pydaw_data->bus_pool[f_i]->effect->pluginInputBuffers[0][f_i2] = 0.0f;
                a_pydaw_data->bus_pool[f_i]->effect->pluginInputBuffers[1][f_i2] = 0.0f;
                f_i2++;
            }            
            f_i++;
        }

        f_i = 0;
        //A ghetto pthread_join for threads that never finish...
        while(f_i < (a_pydaw_data->track_worker_thread_count))
        {            
            pthread_mutex_lock(&a_pydaw_data->track_block_mutexes[f_i]);
            if(a_pydaw_data->track_thread_is_finished[f_i] == 0)
            {
                pthread_mutex_unlock(&a_pydaw_data->track_block_mutexes[f_i]);
                continue;  //Meaning we somehow beat the thread to the mutex, don't proceed, unlock the mutex and wait again for it...
            }
            pthread_mutex_unlock(&a_pydaw_data->track_block_mutexes[f_i]);
            f_i++;
        }
    //}

    
    f_i = 0;

    while(f_i < PYDAW_MIDI_TRACK_COUNT)
    {   
        if(a_pydaw_data->track_pool[f_i]->plugin_index != 0)
        {                
            int f_i2 = 0;

            while(f_i2 < sample_count)
            {
                a_pydaw_data->bus_pool[(a_pydaw_data->track_pool[f_i]->bus_num)]->effect->pluginInputBuffers[0][f_i2] += (a_pydaw_data->track_pool[f_i]->effect->pluginOutputBuffers[0][f_i2]) * (a_pydaw_data->track_pool[f_i]->volume_linear);
                a_pydaw_data->bus_pool[(a_pydaw_data->track_pool[f_i]->bus_num)]->effect->pluginInputBuffers[1][f_i2] += (a_pydaw_data->track_pool[f_i]->effect->pluginOutputBuffers[1][f_i2]) * (a_pydaw_data->track_pool[f_i]->volume_linear);
                f_i2++;
            }
        }

        f_i++;
    }
    
        
    f_i = 1;

    while(f_i < PYDAW_BUS_TRACK_COUNT)
    {        
        v_run_plugin(a_pydaw_data->bus_pool[f_i]->effect, sample_count, a_pydaw_data->bus_pool[f_i]->event_buffer, a_pydaw_data->bus_pool[f_i]->current_period_event_index);
        
        int f_i2 = 0;
        
        while(f_i2 < sample_count)
        {
            a_pydaw_data->bus_pool[0]->effect->pluginInputBuffers[0][f_i2] += (a_pydaw_data->bus_pool[f_i]->effect->pluginOutputBuffers[0][f_i2]) * (a_pydaw_data->bus_pool[f_i]->volume_linear);
            a_pydaw_data->bus_pool[0]->effect->pluginInputBuffers[1][f_i2] += (a_pydaw_data->bus_pool[f_i]->effect->pluginOutputBuffers[1][f_i2]) * (a_pydaw_data->bus_pool[f_i]->volume_linear);
            f_i2++;
        }
        
        f_i++;
    }

    v_run_plugin(a_pydaw_data->bus_pool[0]->effect, sample_count, a_pydaw_data->bus_pool[0]->event_buffer, a_pydaw_data->bus_pool[0]->current_period_event_index);
    
    int f_i2 = 0;

    while(f_i2 < sample_count)
    {
        output0[f_i2] = (a_pydaw_data->bus_pool[0]->effect->pluginOutputBuffers[0][f_i2]) * (a_pydaw_data->bus_pool[0]->volume_linear);
        output1[f_i2] = (a_pydaw_data->bus_pool[0]->effect->pluginOutputBuffers[1][f_i2]) * (a_pydaw_data->bus_pool[0]->volume_linear);
        f_i2++;
    }    
}

t_pynote * g_pynote_get(char a_note, char a_vel, float a_start, float a_length)
{
    t_pynote * f_result = (t_pynote*)malloc(sizeof(t_pynote));
    
    f_result->length = a_length;
    f_result->note = a_note;
    f_result->start = a_start;
    f_result->velocity = a_vel;
    
    return f_result;
}

t_pycc * g_pycc_get(char a_cc_num, char a_cc_val, float a_start)
{
    t_pycc * f_result = (t_pycc*)malloc(sizeof(t_pycc));
    
    f_result->cc_num = a_cc_num;
    f_result->cc_val = a_cc_val;
    f_result->start = a_start;
    
    return f_result;
}

t_pypitchbend * g_pypitchbend_get(float a_start, int a_value)
{
    t_pypitchbend * f_result = (t_pypitchbend*)malloc(sizeof(t_pypitchbend));
    
    f_result->start = a_start;
    f_result->val = a_value;
    
    return f_result;
}

void g_pysong_get(t_pydaw_data* a_pydaw_data)
{   
    t_pysong * f_result = (t_pysong*)malloc(sizeof(t_pysong));
        
    int f_i = 0;
    
    while(f_i < PYDAW_MAX_REGION_COUNT)
    {
        f_result->regions[f_i] = 0;        
        f_i++;
    }
#ifdef PYDAW_MEMCHECK
    v_pydaw_assert_memory_integrity(a_pydaw_data);
#endif
    
    char f_full_path[2048];
    sprintf(f_full_path, "%sdefault.pysong", a_pydaw_data->project_folder);
    
    if(i_pydaw_file_exists(f_full_path))
    {
        f_i = 0;

        t_2d_char_array * f_current_string = g_get_2d_array_from_file(f_full_path, LMS_LARGE_STRING);        

        while(f_i < PYDAW_MAX_REGION_COUNT)
        {            
            char * f_pos_char = c_iterate_2d_char_array(f_current_string);
            if(f_current_string->eof)
            {
                break;
            }
            int f_pos = atoi(f_pos_char);        
            char * f_region_char = c_iterate_2d_char_array(f_current_string);
            f_result->regions[f_pos] = g_pyregion_get(a_pydaw_data, f_region_char);
            strcpy(f_result->regions[f_pos]->name, f_region_char);
            free(f_pos_char);
            free(f_region_char);
            f_i++;
        }

        g_free_2d_char_array(f_current_string);
    }
    
    pthread_mutex_lock(&a_pydaw_data->main_mutex);
    
    if(a_pydaw_data->pysong)
    {
        free(a_pydaw_data->pysong);
    }
    
    a_pydaw_data->pysong = f_result;
    
    pthread_mutex_unlock(&a_pydaw_data->main_mutex);
    
#ifdef PYDAW_MEMCHECK
    v_pydaw_assert_memory_integrity(a_pydaw_data);
#endif
}


int i_pydaw_get_item_index_from_name(t_pydaw_data * a_pydaw_data, const char* a_name)
{
    int f_i = 0;
    
    while(f_i < a_pydaw_data->item_count)
    {
        if(a_pydaw_data->item_pool[f_i]->name)  //Accounting for recorded items that aren't named yet
        {
            if(!strcmp(a_name, a_pydaw_data->item_pool[f_i]->name))
            {
                return f_i;
            }
        }        
        f_i++;
    }    
    return -1;
}

int i_get_song_index_from_region_name(t_pydaw_data* a_pydaw_data, const char * a_region_name)
{
    int f_i = 0;
    
    while(f_i < PYDAW_MAX_REGION_COUNT)
    {
        if(a_pydaw_data->pysong->regions[f_i])
        {
            if(!strcmp(a_region_name, a_pydaw_data->pysong->regions[f_i]->name))
            {
                return f_i;
            }
        }
        f_i++;
    }
    
    return -1;
}

/*For getting a new empty region during recording*/
t_pyregion *  g_pyregion_get_new(t_pydaw_data* a_pydaw_data)
{
    t_pyregion * f_result = (t_pyregion*)malloc(sizeof(t_pyregion));    
    
    f_result->alternate_tempo = 0;
    f_result->tempo = 140.0f;
    f_result->region_length_bars = 0;
    f_result->region_length_beats = 0;
    f_result->bar_length = 0;

    f_result->not_yet_saved = 1;
    f_result->name = NULL;
    
    int f_i = 0;
    int f_i2 = 0;
    
    while(f_i < PYDAW_MIDI_TRACK_COUNT)
    {
        f_i2 = 0;
        while(f_i2 < PYDAW_MAX_REGION_SIZE)
        {
            f_result->item_indexes[f_i][f_i2] = -1;
            f_i2++;
        }
        f_i++;
    }
    
    return f_result;
}

t_pyregion * g_pyregion_get(t_pydaw_data* a_pydaw_data, const char * a_name)
{    
    t_pyregion * f_result = (t_pyregion*)malloc(sizeof(t_pyregion));    
    
    f_result->alternate_tempo = 0;
    f_result->tempo = 140.0f;
    f_result->region_length_bars = 0;
    f_result->region_length_beats = 0;
    f_result->bar_length = 0;
    
    f_result->name = (char*)malloc(sizeof(char) * LMS_TINY_STRING);
    strcpy(f_result->name, a_name);
    f_result->not_yet_saved = 0;
    
    int f_i = 0;
    int f_i2 = 0;
    
    while(f_i < PYDAW_MIDI_TRACK_COUNT)
    {
        f_i2 = 0;
        while(f_i2 < PYDAW_MAX_REGION_SIZE)
        {
            f_result->item_indexes[f_i][f_i2] = -1;
            f_i2++;
        }
        f_i++;
    }
    
    
    char f_full_path[LMS_TINY_STRING];
    sprintf(f_full_path, "%s%s.pyreg", a_pydaw_data->region_folder, a_name);    
    
    t_2d_char_array * f_current_string = g_get_2d_array_from_file(f_full_path, LMS_LARGE_STRING);
    
    f_i = 0;
    
    while(f_i < 264)
    {   
        char * f_y_char = c_iterate_2d_char_array(f_current_string);
        if(f_current_string->eof)
        {
            free(f_y_char);
            break;
        }
        
        if(!strcmp("L", f_y_char))
        {            
            free(f_y_char);
            char * f_bars_char = c_iterate_2d_char_array(f_current_string);
            int f_bars = atoi(f_bars_char);
            f_result->region_length_bars = f_bars;
            free(f_bars_char);
            
            char * f_beats_char = c_iterate_2d_char_array(f_current_string);
            int f_beats = atoi(f_beats_char);
            f_result->region_length_beats = f_beats;
            free(f_beats_char);
            continue;
        }
        if(!strcmp("T", f_y_char))  //per-region tempo, not yet implemented
        {            
            free(f_y_char);
            char * f_tempo_char = c_iterate_2d_char_array(f_current_string);
            f_result->alternate_tempo = 1;
            f_result->tempo = atof(f_tempo_char);
            free(f_tempo_char);
            
            char * f_null_char = c_iterate_2d_char_array(f_current_string);            
            free(f_null_char);
            continue;
        }
        if(!strcmp("B", f_y_char))  //per-region bar length in beats, not yet implemented
        {            
            free(f_y_char);
            char * f_len_char = c_iterate_2d_char_array(f_current_string);            
            f_result->bar_length = atoi(f_len_char);
            free(f_len_char);
            
            char * f_null_char = c_iterate_2d_char_array(f_current_string);            
            free(f_null_char);
            continue;
        }
        
        int f_y = atoi(f_y_char);
        free(f_y_char);
        
        char * f_x_char = c_iterate_2d_char_array(f_current_string);        
        int f_x = atoi(f_x_char);
        free(f_x_char);
        
        char * f_item_name = c_iterate_2d_char_array(f_current_string);
        assert(f_y < PYDAW_MIDI_TRACK_COUNT);
        assert(f_x < PYDAW_MAX_REGION_SIZE);
        f_result->item_indexes[f_y][f_x] = i_pydaw_get_item_index_from_name(a_pydaw_data, f_item_name);
        assert((f_result->item_indexes[f_y][f_x]) != -1);
        assert((f_result->item_indexes[f_y][f_x]) < a_pydaw_data->item_count);
        free(f_item_name);            

        f_i++;
    }

    g_free_2d_char_array(f_current_string);
    
    //v_pydaw_assert_memory_integrity(a_pydaw_data);
    
    return f_result;
}

void v_save_pysong_to_disk(t_pydaw_data * a_pydaw_data)
{
    int f_i = 0;
    
    char f_temp[256];
    char f_result[LMS_LARGE_STRING];
    f_result[0] = '\0';
    
    while(f_i < PYDAW_MAX_REGION_COUNT)
    {
        if(a_pydaw_data->pysong->regions[f_i])
        {
            sprintf(f_temp, "%i|%s\n", f_i, a_pydaw_data->pysong->regions[f_i]->name);
            strcat(f_result, f_temp);
        }
        f_i++;
    }    
    
    strcat(f_result, "\\");
    
    sprintf(f_temp, "%sdefault.pysong", a_pydaw_data->project_folder);
    
    v_pydaw_write_to_file(f_temp, f_result);
}

/*Mimics the UI's Python __str__ method that creates/saves items...*/
void v_save_pyitem_to_disk(t_pydaw_data * a_pydaw_data, int a_index)
{    
    char * f_result = (char*)malloc(sizeof(char) * LMS_MEDIUM_STRING);
    f_result[0] = '\0';
    int f_i = 0;
    
    char f_temp[LMS_TINY_STRING];
    
    t_pyitem * f_pyitem = a_pydaw_data->item_pool[a_index];
    
    /*Sort the item first, otherwise they can be out of order due to note_release being the trigger for writing an event, rather than start time*/
    
    while(1)
    {
        int f_no_changes = 1;
        
        f_i = 0;
        /*Use the Bubble Sort algorithm.  It's generally complete garbage, but in this instance, where the list is going to be
         pseudo-sorted to begin with, it's actually very efficient, likely only making 3 passes at most, 1 or 2 passes typically*/
        while(f_i < (f_pyitem->note_count - 1))
        {        
            if((f_pyitem->notes[f_i]->start) > (f_pyitem->notes[f_i + 1]->start))
            {
                t_pynote * f_greater = f_pyitem->notes[f_i];
                t_pynote * f_lesser = f_pyitem->notes[f_i + 1];
                f_pyitem->notes[f_i] = f_lesser;
                f_pyitem->notes[f_i + 1] = f_greater;
                f_no_changes = 0;
            }

            f_i++;
        }
        
        if(f_no_changes)
        {
            break;
        }
    }
    
    f_i = 0;    
    while(f_i < (f_pyitem->note_count))
    {
        sprintf(f_temp, "n|%f|%f|%i|%i\n", f_pyitem->notes[f_i]->start, f_pyitem->notes[f_i]->length, 
                f_pyitem->notes[f_i]->note, f_pyitem->notes[f_i]->velocity);
        strcat(f_result, f_temp);
        f_i++;
    }
    
    f_i = 0;
    while(f_i < (f_pyitem->cc_count))
    {
        sprintf(f_temp, "c|%f|%i|%i\n", f_pyitem->ccs[f_i]->start, f_pyitem->ccs[f_i]->cc_num, f_pyitem->ccs[f_i]->cc_val);
        strcat(f_result, f_temp);
        f_i++;
    }
        
    f_i = 0;
    while(f_i < (f_pyitem->pitchbend_count))
    {
        sprintf(f_temp, "p|%f|%f\n", f_pyitem->pitchbends[f_i]->start, (((float)(f_pyitem->pitchbends[f_i]->val)) * 0.00012207f));
        strcat(f_result, f_temp);
        f_i++;
    }
    
    strcat(f_result, "\\");
    
    char f_temp2[LMS_TINY_STRING];
    //Generate a default name if necessary
    if(!f_pyitem->name)
    {
        do{
            a_pydaw_data->record_name_index_items = (a_pydaw_data->record_name_index_items) + 1;
            sprintf(f_temp2, "recorded-%i", a_pydaw_data->record_name_index_items);
        } while(i_pydaw_get_item_index_from_name(a_pydaw_data, f_temp2) != -1);
        
        f_pyitem->name = (char*)malloc(sizeof(char) * LMS_TINY_STRING);
        strcpy(f_pyitem->name, f_temp2);
    }
    sprintf(f_temp, "%s%s.pyitem", a_pydaw_data->item_folder, f_pyitem->name);
    
    v_pydaw_write_to_file(f_temp, f_result);
}

/* Items must be saved before regions to prevent a SEGFAULT at the line that references item name...*/
void v_save_pyregion_to_disk(t_pydaw_data * a_pydaw_data, int a_region_num)
{    
    int f_i = 0;
    int f_i2 = 0;
    
    char * f_result = (char*)malloc(sizeof(char) * LMS_LARGE_STRING);
    strcpy(f_result, "");
    
    char f_temp[LMS_TINY_STRING];
    
    while(f_i < PYDAW_MIDI_TRACK_COUNT)
    {
        f_i2 = 0;
        while(f_i2 < PYDAW_MAX_REGION_SIZE)
        {
            if(a_pydaw_data->pysong->regions[a_region_num]->item_indexes[f_i][f_i2]  != -1)
            {                
                sprintf(f_temp, "%i|%i|%s\n", f_i, f_i2, 
                        a_pydaw_data->item_pool[(a_pydaw_data->pysong->regions[a_region_num]->item_indexes[f_i][f_i2])]->name);
                strcat(f_result, f_temp);
            }
            f_i2++;
        }
        f_i++;
    }
    
    strcat(f_result, "\\");
    
    char f_temp2[LMS_TINY_STRING];
    //Generate a default name if necessary
    if(!a_pydaw_data->pysong->regions[a_region_num]->name)
    {
        f_i = 0;
        while(f_i < PYDAW_MAX_REGION_COUNT)
        {            
            if(f_i == a_region_num)
            {
                f_i++;
                continue;
            }
            
            sprintf(f_temp2, "recorded-%i", a_pydaw_data->record_name_index_regions);
            if(a_pydaw_data->pysong->regions[f_i] && (a_pydaw_data->pysong->regions[f_i]->name))
            {
                if(!strcmp(a_pydaw_data->pysong->regions[f_i]->name, f_temp2))
                {
                    f_i = 0;
                    a_pydaw_data->record_name_index_regions = (a_pydaw_data->record_name_index_regions) + 1;
                    continue;
                }
            }
            f_i++;
        }
        a_pydaw_data->record_name_index_regions = (a_pydaw_data->record_name_index_regions) + 1;  //spare a wasted trip around the region pool next time...
        
        a_pydaw_data->pysong->regions[a_region_num]->name = (char*)malloc(sizeof(char) * LMS_TINY_STRING);        
        strcpy(a_pydaw_data->pysong->regions[a_region_num]->name, f_temp2);
    }
    
    sprintf(f_temp, "%s%s.pyreg", a_pydaw_data->region_folder, a_pydaw_data->pysong->regions[a_region_num]->name);
    
    v_pydaw_write_to_file(f_temp, f_result);
}

/*Get an empty pyitem, used for recording.  Returns the item number in the item pool*/
int g_pyitem_get_new(t_pydaw_data* a_pydaw_data)
{
    t_pyitem * f_item = (t_pyitem*)malloc(sizeof(t_pyitem));
    f_item->name = NULL;
    f_item->cc_count = 0;
    f_item->note_count = 0;
    f_item->pitchbend_count = 0;
    
    a_pydaw_data->item_pool[(a_pydaw_data->item_count)] = f_item;
    int f_result = (a_pydaw_data->item_count);
    a_pydaw_data->item_count = (a_pydaw_data->item_count) + 1;
    return f_result;
}


void g_pyitem_get(t_pydaw_data* a_pydaw_data, const char * a_name)
{
    t_pyitem * f_result = (t_pyitem*)malloc(sizeof(t_pyitem));
    
    f_result->name = (char*)malloc(sizeof(char) * 256);
    strcpy(f_result->name, a_name);
    f_result->cc_count = 0;
    f_result->note_count = 0;
    f_result->pitchbend_count = 0;
    
    char f_full_path[512];
    strcpy(f_full_path, a_pydaw_data->item_folder);
    strcat(f_full_path, a_name);
    strcat(f_full_path, ".pyitem");
    
    t_2d_char_array * f_current_string = g_get_2d_array_from_file(f_full_path, LMS_LARGE_STRING);
    
    int f_i = 0;
    
    while(f_i < PYDAW_MAX_EVENTS_PER_ITEM_COUNT)
    {   
        char * f_type = c_iterate_2d_char_array(f_current_string);
        
        if(f_current_string->eof)
        {
            break;
        }
        
        char * f_start = c_iterate_2d_char_array(f_current_string);
                        
        if(!strcmp(f_type, "n"))  //note
        {
            char * f_length = c_iterate_2d_char_array(f_current_string);
            char * f_note = c_iterate_2d_char_array(f_current_string);
            char * f_vel = c_iterate_2d_char_array(f_current_string);
            assert((f_result->note_count) < PYDAW_MAX_EVENTS_PER_ITEM_COUNT);
            f_result->notes[(f_result->note_count)] = g_pynote_get(atoi(f_note), atoi(f_vel), atof(f_start), atof(f_length));
            f_result->note_count = (f_result->note_count) + 1;
            
            free(f_length);
            free(f_note);
            free(f_vel);
        }
        else if(!strcmp(f_type, "c")) //cc
        {
            char * f_cc_num = c_iterate_2d_char_array(f_current_string);
            char * f_cc_val = c_iterate_2d_char_array(f_current_string);
            
            f_result->ccs[(f_result->cc_count)] = g_pycc_get(atoi(f_cc_num), atoi(f_cc_val), atof(f_start));
            f_result->cc_count = (f_result->cc_count) + 1;
            
            free(f_cc_num);
            free(f_cc_val);
        }        
        else if(!strcmp(f_type, "p")) //pitchbend
        {            
            char * f_pb_val_char = c_iterate_2d_char_array(f_current_string);
            int f_pb_val = atof(f_pb_val_char) * 8192.0f;
            
            f_result->pitchbends[(f_result->pitchbend_count)] = g_pypitchbend_get(atof(f_start), f_pb_val);
            f_result->pitchbend_count = (f_result->pitchbend_count) + 1;
            
            free(f_pb_val_char);
        }
        else if(!strcmp("U", f_type))  //item UID, not yet implemented
        {            
            free(f_type);
            char * f_uid_char = c_iterate_2d_char_array(f_current_string);            
            f_result->uid = atoi(f_uid_char);
            free(f_uid_char);            
        }
        else
        {
            printf("Invalid event type %s\n", f_type);
        }
        
        free(f_start);
        free(f_type);
        f_i++;
    }

    g_free_2d_char_array(f_current_string);
    
    int f_item_index = i_pydaw_get_item_index_from_name(a_pydaw_data, a_name);
    
    if(f_item_index < 0)
    {
        a_pydaw_data->item_pool[(a_pydaw_data->item_count)] = f_result;
        a_pydaw_data->item_count = (a_pydaw_data->item_count) + 1;
    }
    else
    {
        free(a_pydaw_data->item_pool[f_item_index]);
        a_pydaw_data->item_pool[f_item_index] = f_result;        
    }
}

t_pytrack * g_pytrack_get(int a_track_num)
{
    t_pytrack * f_result = (t_pytrack*)malloc(sizeof(t_pytrack));
    
    f_result->track_num = a_track_num;
    f_result->mute = 0;
    f_result->solo = 0;
    f_result->volume = 0.0f;
    f_result->volume_linear = 1.0f;
    f_result->plugin_index = 0;
    f_result->event_buffer = (snd_seq_event_t*)malloc(sizeof(snd_seq_event_t) * PYDAW_MAX_EVENT_BUFFER_SIZE);
    f_result->rec = 0;
    f_result->bus_num = 0;
    f_result->name[0] = '\0';
            
    int f_i = 0;
    
    while(f_i < PYDAW_MAX_EVENT_BUFFER_SIZE)    
    {
        snd_seq_ev_clear(&f_result->event_buffer[f_i]);
        f_i++;
    }
    
    f_result->instrument = NULL;
    f_result->effect = NULL;
    
    f_result->current_period_event_index = 0;    
            
    pthread_mutex_init(&f_result->mutex, NULL);
    
    return f_result;
}


void pydaw_osc_error(int num, const char *msg, const char *path)
{
    fprintf(stderr, "PyDAW: liblo server error %d in path %s: %s\n",
	    num, path, msg);
}

t_pydaw_data * g_pydaw_data_get(float a_sample_rate)
{
    t_pydaw_data * f_result = (t_pydaw_data*)malloc(sizeof(t_pydaw_data));
    
    pthread_mutex_init(&f_result->main_mutex, NULL);
    pthread_mutex_init(&f_result->quit_mutex, NULL);
    pthread_mutex_init(&f_result->offline_mutex, NULL);    
    f_result->sample_rate = a_sample_rate;
    f_result->current_sample = 0;
    f_result->current_bar = 0;
    f_result->current_region = 0;
    f_result->playback_cursor = 0.0f;
    f_result->playback_inc = 0.0f;
    
    f_result->loop_mode = 0;
    f_result->item_folder = (char*)malloc(sizeof(char) * 256);
    f_result->instruments_folder = (char*)malloc(sizeof(char) * 256);
    f_result->project_folder = (char*)malloc(sizeof(char) * 256);
    f_result->region_folder = (char*)malloc(sizeof(char) * 256);
    //f_result->project_name = (char*)malloc(sizeof(char) * 256);
    f_result->playback_mode = 0;
    f_result->pysong = NULL;
    f_result->item_count = 0;
    f_result->is_soloed = 0;
    f_result->recording_in_current_bar = 0;
    f_result->record_name_index_items = 0;
    f_result->record_name_index_regions = 0;
    f_result->recorded_note_current_beat = 0;
    f_result->recording_current_item_pool_index = -1;
    f_result->recording_first_item = -1;
    
    f_result->amp_ptr = g_amp_get();
    f_result->is_offline_rendering = 0;
    
    f_result->default_region_length_bars = 8;
    f_result->default_region_length_beats = 0;
    f_result->default_bar_length = 4;
    
    int f_i = 0;
    
    while(f_i < PYDAW_AUDIO_TRACK_COUNT)
    {        
        f_result->audio_track_pool[f_i] = g_pytrack_get(f_i);
        v_set_plugin_index(f_result, f_result->audio_track_pool[f_i], -1);
        f_i++;
    }
    
    f_i = 0;
    
    while(f_i < PYDAW_BUS_TRACK_COUNT)
    {
        f_result->bus_pool[f_i] = g_pytrack_get(f_i);
        v_set_plugin_index(f_result, f_result->bus_pool[f_i], -1);
        f_i++;
    }
    
    f_i = 0;
    
    while(f_i < PYDAW_AUDIO_INPUT_TRACK_COUNT)
    {
        //TODO
        f_i++;
    }
    
    f_i = 0;
    
    while(f_i < PYDAW_MIDI_TRACK_COUNT)
    {
        f_result->track_pool[f_i] = g_pytrack_get(f_i);        
        f_result->track_current_item_note_event_indexes[f_i] = 0;
        f_result->track_current_item_cc_event_indexes[f_i] = 0;
        f_result->track_current_item_pitchbend_event_indexes[f_i] = 0;
        
        int f_i2 = 0;
        
        while(f_i2 < PYDAW_MIDI_NOTE_COUNT)
        {
            f_result->note_offs[f_i][f_i2] = -1;
            f_i2++;
        }
        
        f_i++;
    }
        
    f_i = 0;
    
    while(f_i < PYDAW_MIDI_NOTE_COUNT)
    {
        f_result->recorded_notes_item_tracker[f_i] = -1;
        f_result->recorded_notes_beat_tracker[f_i] = -1;
        f_result->recorded_notes_start_tracker[f_i] = 0.0f;
        f_result->recorded_notes_velocity_tracker[f_i] = -1;
        
        f_i++;
    }
       
    /* Create OSC thread */    
    char *tmp;
    
    char osc_path_tmp[1024];
    
    f_result->serverThread = lo_server_thread_new(NULL, pydaw_osc_error);
    snprintf(osc_path_tmp, 31, "/dssi/pydaw_plugins");
    tmp = lo_server_thread_get_url(f_result->serverThread);
    f_result->osc_url = (char *)malloc(strlen(tmp) + strlen(osc_path_tmp));
    sprintf(f_result->osc_url, "%s%s", tmp, osc_path_tmp + 1);    
    free(tmp);
    
    return f_result;
}

void v_open_default_project(t_pydaw_data * a_data)
{
    char * f_home = getenv("HOME");
    char f_default_project_folder[512];
    sprintf(f_default_project_folder, "%s/pydaw2/default-project", f_home);
    v_open_project(a_data, f_default_project_folder);
    //free(f_home);  //Not freeing this because it SEGFAULTS for some reason and is tiny....
}

void v_pydaw_activate_osc_thread(t_pydaw_data * a_pydaw_data, lo_method_handler osc_message_handler)
{
    lo_server_thread_add_method(a_pydaw_data->serverThread, NULL, NULL, osc_message_handler, NULL);
    lo_server_thread_start(a_pydaw_data->serverThread);
}

void v_pydaw_open_track(t_pydaw_data * a_pydaw_data, t_pytrack * a_track)
{
    //Don't open the instrument plugin if it's a bus track
    if((a_track->plugin_index) != -1)
    {
        v_pydaw_open_plugin(a_pydaw_data, a_track, 0);
    }
    
    v_pydaw_open_plugin(a_pydaw_data, a_track, 1);
    
#ifdef PYDAW_MEMCHECK
    v_pydaw_assert_memory_integrity(a_pydaw_data);
#endif
}

void v_pydaw_open_plugin(t_pydaw_data * a_pydaw_data, t_pytrack * a_track, int a_is_fx)
{
    char f_file_name[512];
    
    if(a_is_fx)
    {
        sprintf(f_file_name, "%s%i.pyfx", a_pydaw_data->instruments_folder, a_track->track_num);
    }
    else
    {
        sprintf(f_file_name, "%s%i.pyinst", a_pydaw_data->instruments_folder, a_track->track_num);
    }
        
    if(i_pydaw_file_exists(f_file_name))
    {
        printf("v_pydaw_open_track:  Track exists %s , loading\n", f_file_name);

        t_2d_char_array * f_2d_array = g_get_2d_array_from_file(f_file_name, LMS_LARGE_STRING);

        t_pydaw_plugin * f_instance;
        
        if(a_is_fx)
        {
            f_instance = a_track->effect;
        }
        else
        {
            f_instance = a_track->instrument;
        }
        
        while(1)
        {
            char * f_key = c_iterate_2d_char_array(f_2d_array);
            
            if(f_2d_array->eof)
            {
                free(f_key);
                break;
            }
            
            char * f_value = c_iterate_2d_char_array(f_2d_array);

            assert(strcmp(f_key, ""));
            assert(strcmp(f_value, ""));
            
            if(!strcmp(f_key, "load"))
            {                
                int f_i = 0;
                while(1)
                {
                    if(f_value[f_i] == '\0')
                    {
                        break;
                    }
                    else if(f_value[f_i] == '~')
                    {
                        f_value[f_i] = '|';
                    }
                    f_i++;
                }
                
                strcpy(f_instance->euphoria_load, f_value);
                f_instance->euphoria_load_set = 1;
            }
            else
            {
                int f_port_key = atoi(f_key);
                float f_port_value = atof(f_value);
                
                //assert(f_port_key < (f_instance->controlIns));
                
                f_instance->pluginControlIns[f_port_key] = f_port_value;
            }                
        }

        g_free_2d_char_array(f_2d_array);
        
        if(f_instance->euphoria_load_set)
        {
            char * message = f_instance->descriptor->configure(f_instance->ladspa_handle, "load", f_instance->euphoria_load);
            if (message) 
            {
                printf("v_pydaw_open_track: on configure '%s' '%s', plugin returned error '%s'\n","load", f_instance->euphoria_load, message);
                free(message);
            }

            if(f_instance->uiTarget)
            {
                    lo_send(f_instance->uiTarget, f_instance->ui_osc_configure_path, "ss", "load", f_instance->euphoria_load);
            }
        }
    }
}

void v_pydaw_open_tracks(t_pydaw_data * a_pydaw_data)
{
    char f_file_name[256];    
    sprintf(f_file_name, "%sdefault.pytracks", a_pydaw_data->project_folder);
    
    if(i_pydaw_file_exists(f_file_name))
    {
        printf("v_pydaw_open_tracks:  File exists %s , loading\n", f_file_name);

        t_2d_char_array * f_2d_array = g_get_2d_array_from_file(f_file_name, LMS_LARGE_STRING);

        while(1)
        {
            char * f_track_index_str = c_iterate_2d_char_array(f_2d_array);
            
            if(f_2d_array->eof)
            {
                free(f_track_index_str);
                break;
            }
            
            char * f_solo_str = c_iterate_2d_char_array(f_2d_array);
            char * f_mute_str = c_iterate_2d_char_array(f_2d_array);
            char * f_rec_str = c_iterate_2d_char_array(f_2d_array);
            char * f_vol_str = c_iterate_2d_char_array(f_2d_array);
            char * f_name_str = c_iterate_2d_char_array(f_2d_array);
            char * f_plugin_index_str = c_iterate_2d_char_array(f_2d_array);
            char * f_bus_num_str = c_iterate_2d_char_array(f_2d_array);

            int f_track_index = atoi(f_track_index_str);
            free(f_track_index_str);
            assert(f_track_index >= 0 && f_track_index < PYDAW_MIDI_TRACK_COUNT);
            
            int f_solo = atoi(f_solo_str);
            free(f_solo_str);
            assert(f_solo == 0 || f_solo == 1);
            
            int f_mute = atoi(f_mute_str);
            free(f_mute_str);
            assert(f_mute == 0 || f_mute == 1);
            
            int f_rec = atoi(f_rec_str);
            free(f_rec_str);
            assert(f_rec == 0 || f_rec == 1);
            
            int f_vol = atoi(f_vol_str);
            free(f_vol_str);
            assert(f_vol < 24 && f_vol > -150);
                        
            strcpy(a_pydaw_data->track_pool[f_track_index]->name, f_name_str);
            free(f_name_str);
            
            int f_plugin_index = atoi(f_plugin_index_str);
            free(f_plugin_index_str);
            assert(f_plugin_index >= -1 && f_plugin_index <= 3);   //TODO:  change this if adding more plugin instruments...
            
            int f_bus_num = atoi(f_bus_num_str);
            free(f_bus_num_str);
            assert(f_bus_num >= 0 && f_bus_num < PYDAW_BUS_TRACK_COUNT);
            
            if(a_pydaw_data->track_pool[f_track_index]->plugin_index != -1)
            {
                a_pydaw_data->track_pool[f_track_index]->plugin_index = 0;  //Must set it to zero to prevent the state file from being deleted
                v_set_plugin_index(a_pydaw_data, a_pydaw_data->track_pool[f_track_index], f_plugin_index);
            }
            
            a_pydaw_data->track_pool[f_track_index]->solo = f_solo;
            a_pydaw_data->track_pool[f_track_index]->mute = f_mute;
            a_pydaw_data->track_pool[f_track_index]->rec = f_rec;
            v_pydaw_set_track_volume(a_pydaw_data, a_pydaw_data->track_pool[f_track_index], f_vol);
            a_pydaw_data->track_pool[f_track_index]->bus_num = f_bus_num;
            
            v_pydaw_open_track(a_pydaw_data, a_pydaw_data->track_pool[f_track_index]);
        }

        g_free_2d_char_array(f_2d_array);
    }
    else   //ensure everything is closed...
    {
        int f_i = 0;
        
        while(f_i < PYDAW_MIDI_TRACK_COUNT)
        {
            a_pydaw_data->track_pool[f_i]->plugin_index = 0;  //Must set it to zero to prevent the state file from being deleted
            /*if(f_i < PYDAW_BUS_TRACK_COUNT)
            {
                v_set_plugin_index(a_pydaw_data, f_i, 0);
            }
            else
            {*/
                //v_set_plugin_index(a_pydaw_data, f_i, -1);
            //}
            v_set_plugin_index(a_pydaw_data, a_pydaw_data->track_pool[f_i], 0);
            
            a_pydaw_data->track_pool[f_i]->solo = 0;
            a_pydaw_data->track_pool[f_i]->mute = 0;
            a_pydaw_data->track_pool[f_i]->rec = 0;
            sprintf(a_pydaw_data->track_pool[f_i]->name, "track%i", (f_i + 1));  //TODO:  Move this to the above 'if' statement so the bus tracks can be named differently       
            v_pydaw_set_track_volume(a_pydaw_data, a_pydaw_data->track_pool[f_i], 0.0f);            
            v_pydaw_open_track(a_pydaw_data, a_pydaw_data->track_pool[f_i]);
            
            f_i++;
        }
    }
    
#ifdef PYDAW_MEMCHECK
    v_pydaw_assert_memory_integrity(a_pydaw_data);
#endif
}

void v_open_project(t_pydaw_data* a_pydaw_data, const char* a_project_folder)
{
    clock_t f_start = clock();
    
    pthread_mutex_lock(&a_pydaw_data->offline_mutex);
    
    sprintf(a_pydaw_data->project_folder, "%s/", a_project_folder);    
    sprintf(a_pydaw_data->item_folder, "%sitems/", a_pydaw_data->project_folder);
    sprintf(a_pydaw_data->region_folder, "%sregions/", a_pydaw_data->project_folder);
    sprintf(a_pydaw_data->instruments_folder, "%sinstruments/", a_pydaw_data->project_folder);    
    //strcpy(a_pydaw_data->project_name, a_name);
    
    int f_i = 0;
    
    while(f_i < a_pydaw_data->item_count)
    {
        if(a_pydaw_data->item_pool[f_i])
        {
            free(a_pydaw_data->item_pool[f_i]);
        }
        f_i++;
    }
        
    a_pydaw_data->item_count = 0;
    
    char f_song_file[512];    
    sprintf(f_song_file, "%sdefault.pysong", a_pydaw_data->project_folder);
    
    struct stat f_proj_stat;
    stat((a_pydaw_data->project_folder), &f_proj_stat);
    struct stat f_item_stat;
    stat((a_pydaw_data->item_folder), &f_item_stat);
    struct stat f_reg_stat;
    stat((a_pydaw_data->region_folder), &f_reg_stat);
    struct stat f_inst_stat;
    stat((a_pydaw_data->instruments_folder), &f_inst_stat);
    struct stat f_song_file_stat;
    stat(f_song_file, &f_song_file_stat);
    
    if(S_ISDIR(f_proj_stat.st_mode) && S_ISDIR(f_item_stat.st_mode) &&
        S_ISDIR(f_reg_stat.st_mode) && S_ISDIR(f_inst_stat.st_mode) &&
        S_ISREG(f_song_file_stat.st_mode))
    {
        t_dir_list * f_item_dir_list = g_get_dir_list(a_pydaw_data->item_folder);    
        f_i = 0;

        while(f_i < f_item_dir_list->dir_count)
        {
            t_1d_char_array * f_file_name = c_split_str(f_item_dir_list->dir_list[f_i], '.', 2, LMS_SMALL_STRING);
            g_pyitem_get(a_pydaw_data, f_file_name->array[0]);
            g_free_1d_char_array(f_file_name);
            f_i++;
        }
    
        g_pysong_get(a_pydaw_data);
        v_pydaw_open_tracks(a_pydaw_data);        
    }
    else
    {
        printf("Song file and project directory structure not found, not loading project.  This is to be expected if launching PyDAW for the first time\n");
        g_pysong_get(a_pydaw_data);  //Loads empty...  TODO:  Make this a separate function for getting an empty pysong or loading a file into one...
        v_pydaw_init_busses(a_pydaw_data);
    }
    
    char f_transport_file[256];  //TODO:  This should be moved to a separate function
    sprintf(f_transport_file, "%sdefault.pytransport", a_pydaw_data->project_folder);
    
    if(i_pydaw_file_exists(f_transport_file))
    {
        printf("v_open_project:  Found transport file, setting tempo from file\n");

        t_2d_char_array * f_2d_array = g_get_2d_array_from_file(f_transport_file, LMS_LARGE_STRING);
        char * f_tempo_str = c_iterate_2d_char_array(f_2d_array);
        float f_tempo = atof(f_tempo_str);
        free(f_tempo_str);
        char * f_midi_keybd_str = c_iterate_2d_char_array(f_2d_array);
        free(f_midi_keybd_str);
        char * f_loop_mode_str =  c_iterate_2d_char_array(f_2d_array);
        int f_loop_mode = atoi(f_loop_mode_str);
        assert(f_loop_mode >= 0 && f_loop_mode <= 2);
        v_set_loop_mode(a_pydaw_data, f_loop_mode);
        g_free_2d_char_array(f_2d_array);
        
        assert(f_tempo > 30.0f && f_tempo < 300.0f);        
        v_set_tempo(a_pydaw_data, f_tempo);
    }
    else  //No transport file, set default tempo
    {
        printf("No transport file found, defaulting to 140.0 BPM\n");
        v_set_tempo(a_pydaw_data, 140.0f);
    }
       
    
#ifdef PYDAW_MEMCHECK
    v_pydaw_assert_memory_integrity(a_pydaw_data);
#endif
    pthread_mutex_unlock(&a_pydaw_data->offline_mutex);
    
    v_pydaw_print_benchmark("v_open_project", f_start);
}

/* void v_set_playback_mode(t_pydaw_data * a_pydaw_data, 
 * int a_mode, //
 * int a_region, //The region index to start playback on
 * int a_bar) //The bar index (with a_region) to start playback on
 */
void v_set_playback_mode(t_pydaw_data * a_pydaw_data, int a_mode, int a_region, int a_bar)
{   
    switch(a_mode)
    {
        case 0: //stop
        {  
            int f_was_recording = 0;
            if(a_pydaw_data->playback_mode == PYDAW_PLAYBACK_MODE_REC)
            {
                f_was_recording = 1;
            }
            pthread_mutex_lock(&a_pydaw_data->main_mutex);
            a_pydaw_data->playback_mode = a_mode;
            int f_i = 0;
            //Send zero pitchbend messages so the plugins pitch isn't off next time playback starts
            while(f_i < PYDAW_MIDI_TRACK_COUNT)
            {
                if(a_pydaw_data->track_pool[f_i]->plugin_index > 0)
                {
                    snd_seq_ev_clear(&a_pydaw_data->track_pool[f_i]->event_buffer[(a_pydaw_data->track_pool[f_i]->current_period_event_index)]);
                    snd_seq_ev_set_pitchbend(
                            &a_pydaw_data->track_pool[f_i]->event_buffer[0], 0, 0);
                    a_pydaw_data->track_pool[f_i]->current_period_event_index = 1;                    
                }
                f_i++;
            }
            pthread_mutex_unlock(&a_pydaw_data->main_mutex);
            if(f_was_recording)  //Things must be saved in the order of:  items|regions|song, otherwise it will SEGFAULT from not having a name yet...
            {   
                if(a_pydaw_data->recording_current_item_pool_index != -1)  //Don't do it if we never recorded an item...
                {
                    f_i = (a_pydaw_data->recording_first_item);
                    while(f_i < (a_pydaw_data->item_count))
                    {
                        v_save_pyitem_to_disk(a_pydaw_data, f_i);
                        f_i++;
                    }
                }
                f_i = 0;                   
                while(f_i < PYDAW_MAX_REGION_COUNT)
                {
                    if((a_pydaw_data->pysong->regions[f_i]) && (a_pydaw_data->pysong->regions[f_i]->not_yet_saved))
                    {
                        v_save_pyregion_to_disk(a_pydaw_data, f_i);
                    }
                    f_i++;
                }
                
                v_save_pysong_to_disk(a_pydaw_data);
            }
            f_i = 0;
            long early_noteoff = (a_pydaw_data->current_sample) + 20000;
            while(f_i < PYDAW_MIDI_TRACK_COUNT)
            {
                int f_i2 = 0;
                while(f_i2 < PYDAW_MIDI_NOTE_COUNT)
                {
                    if((a_pydaw_data->note_offs[f_i][f_i2]) > early_noteoff)
                    {
                        a_pydaw_data->note_offs[f_i][f_i2] = early_noteoff;
                    }
                    f_i2++;
                }
                f_i++;
            }
            //Initiate some sort of mixer fadeout?
        }
            break;
        case 1:  //play
            pthread_mutex_lock(&a_pydaw_data->main_mutex);
            a_pydaw_data->playback_mode = a_mode;
            v_set_playback_cursor(a_pydaw_data, a_region, a_bar);
            pthread_mutex_unlock(&a_pydaw_data->main_mutex);
            break;
        case 2:  //record
            if(a_pydaw_data->playback_mode == PYDAW_PLAYBACK_MODE_REC)
            {
                return;  
            }            
                        
            pthread_mutex_lock(&a_pydaw_data->main_mutex);
            a_pydaw_data->recording_first_item = -1;
            a_pydaw_data->recorded_note_current_beat = 0;
            a_pydaw_data->playback_mode = a_mode;
            a_pydaw_data->recording_in_current_bar = 0;
            a_pydaw_data->recording_current_item_pool_index = -1;
            v_set_playback_cursor(a_pydaw_data, a_region, a_bar);
            pthread_mutex_unlock(&a_pydaw_data->main_mutex);
            break;
    }    
    
#ifdef PYDAW_MEMCHECK
    v_pydaw_assert_memory_integrity(a_pydaw_data);
#endif
}

void v_set_playback_cursor(t_pydaw_data * a_pydaw_data, int a_region, int a_bar)
{
    a_pydaw_data->current_bar = a_bar;
    a_pydaw_data->current_region = a_region;
    a_pydaw_data->playback_cursor = 0.0f;
    
    int f_i = 0;
    
    while(f_i < PYDAW_MIDI_TRACK_COUNT)
    {
        a_pydaw_data->track_current_item_cc_event_indexes[f_i] = 0;
        a_pydaw_data->track_current_item_note_event_indexes[f_i] = 0;
        a_pydaw_data->track_current_item_pitchbend_event_indexes[f_i] = 0;
        f_i++;
    }
    
#ifdef PYDAW_MEMCHECK
    v_pydaw_assert_memory_integrity(a_pydaw_data);
#endif
    //TODO:  An  "all notes off" function 
}

void v_set_loop_mode(t_pydaw_data * a_pydaw_data, int a_mode)
{
    pthread_mutex_lock(&a_pydaw_data->main_mutex);
    a_pydaw_data->loop_mode = a_mode;
    
#ifdef PYDAW_MEMCHECK
    v_pydaw_assert_memory_integrity(a_pydaw_data);
#endif
    
    pthread_mutex_unlock(&a_pydaw_data->main_mutex);
}

void v_set_tempo(t_pydaw_data * a_pydaw_data, float a_tempo)
{
    pthread_mutex_lock(&a_pydaw_data->main_mutex);
    //char log_buff[200];
    a_pydaw_data->tempo = a_tempo;
    a_pydaw_data->playback_inc = ( (1.0f/(a_pydaw_data->sample_rate)) / (60.0f/(a_tempo * 0.25f)) );
    a_pydaw_data->samples_per_beat = (a_pydaw_data->sample_rate)/(a_tempo/60.0f);

#ifdef PYDAW_MEMCHECK
    v_pydaw_assert_memory_integrity(a_pydaw_data);
#endif
    
    pthread_mutex_unlock(&a_pydaw_data->main_mutex);
}

void v_pydaw_save_track(t_pydaw_data * a_pydaw_data, t_pytrack * a_track)
{
    if(a_track->plugin_index == 0)
    {
        return;  //Delete the file if exists?
    }

    if(a_track->plugin_index != -1)
    {
        v_pydaw_save_plugin(a_pydaw_data, a_track, 0);
    }
    
    v_pydaw_save_plugin(a_pydaw_data, a_track, 1);
    
#ifdef PYDAW_MEMCHECK
    v_pydaw_assert_memory_integrity(a_pydaw_data);
#endif
}

void v_pydaw_save_plugin(t_pydaw_data * a_pydaw_data, t_pytrack * a_track, int a_is_fx)
{    
    pthread_mutex_lock(&a_track->mutex);
    char f_string[LMS_LARGE_STRING];
    f_string[0] = '\0';
    //sprintf(f_string, "");
    
    t_pydaw_plugin * f_instance;

    if(a_is_fx)
    {
        f_instance = a_track->effect;
    }
    else
    {
        f_instance = a_track->instrument;
    }

    if((a_is_fx == 0) && (a_track->plugin_index == 1))
    {
        if(a_track->instrument->euphoria_load_set)
        {
            char f_load[8192];
            char f_temp[8192];
            strcpy(f_temp, a_track->instrument->euphoria_load);
            int f_i = 0;
            while(1)
            {
                if(f_temp[f_i] == '\0')
                {
                    break;
                }
                else if((f_temp[f_i] == '|') || (f_temp[f_i] == '>'))  //'>' being the 'reload' character...
                {
                    f_temp[f_i] = '~';
                }
                f_i++;
            }
            sprintf(f_load, "load|%s\n", f_temp);
            strcat(f_string, f_load);
        }
    }

    int f_i2 = 0;

    while(f_i2 < (f_instance->controlIns))
    {
        int in = f_i2 + f_instance->firstControlIn;
	//int port = instance->pluginControlInPortNumbers[in];
	        
        char f_port_entry[64];        
        sprintf(f_port_entry, "%i|%f\n", in, //port,
        f_instance->pluginControlIns[in]
        );
        strcat(f_string, f_port_entry);
        f_i2++;
    }

    strcat(f_string, "\\");        
    char f_file_name[512];
        
    if(a_is_fx)
    {
        sprintf(f_file_name, "%s%i.pyfx", a_pydaw_data->instruments_folder, a_track->track_num);
    }
    else
    {
        sprintf(f_file_name, "%s%i.pyinst", a_pydaw_data->instruments_folder, a_track->track_num);
    }
        
    pthread_mutex_unlock(&a_track->mutex);

    v_pydaw_write_to_file(f_file_name, f_string);
}

void v_pydaw_save_tracks(t_pydaw_data * a_pydaw_data)
{
    pthread_mutex_lock(&a_pydaw_data->quit_mutex);
    int f_i = 0;
    
    while(f_i < PYDAW_MIDI_TRACK_COUNT)
    {
        v_pydaw_save_track(a_pydaw_data, a_pydaw_data->track_pool[f_i]);
        f_i++;
    }
    
    printf("Saving tracks complete\n");
    pthread_mutex_unlock(&a_pydaw_data->quit_mutex);
    
#ifdef PYDAW_MEMCHECK
    v_pydaw_assert_memory_integrity(a_pydaw_data);
#endif
}

void v_show_plugin_ui(t_pydaw_data * a_pydaw_data, t_pytrack * a_track, int a_is_fx)
{   
    pthread_mutex_lock(&a_track->mutex);
        
    if(a_track->plugin_index == 0)
    {
        pthread_mutex_unlock(&a_track->mutex);
        return;
    }
    
    if(a_is_fx)
    {        
        if(a_track->effect->uiTarget)
        {
            lo_send(a_track->effect->uiTarget, 
                    a_track->effect->ui_osc_show_path, "");
            pthread_mutex_unlock(&a_track->mutex);
            return;
        }
                
        if(a_track->effect->showing_ui)
        {
            pthread_mutex_unlock(&a_track->mutex);
            return;
        }
        else
        {
            a_track->effect->showing_ui = 1;
        }
    }
    else
    {
        if(a_track->instrument->uiTarget)
        {
            lo_send(a_track->instrument->uiTarget, 
                    a_track->instrument->ui_osc_show_path, "");
            pthread_mutex_unlock(&a_track->mutex);
            return;
        }
        
        if(a_track->instrument->showing_ui)
        {
            pthread_mutex_unlock(&a_track->mutex);
            return;
        }
        else
        {
            a_track->instrument->showing_ui = 1;
        }
    }
    
    
    pthread_mutex_unlock(&a_track->mutex);
    
    char * filename;
    char oscUrl[256];    
    char * dllName;
    //char * label;
     
    
    if(a_is_fx)
    {
        filename = "/usr/lib/pydaw2/lms_modulex/LMS_MODULEX_qt";
        dllName = "lms_modulex.so";       
    }
    else
    {
        switch(a_track->plugin_index)
        {
            case 1:
                filename = "/usr/lib/pydaw2/euphoria/LMS_EUPHORIA_qt";
                dllName = "euphoria.so";
                break;
            case 2:
                filename = "/usr/lib/pydaw2/ray_v/LMS_RAYV_qt";
                dllName = "ray_v.so";
                break;
            case 3:
                filename = "/usr/lib/pydaw2/way_v/WAY_V_qt";
                dllName = "way_v.so";
                break;
            default:
                return;
        }

    }
    
    char track_number_string[6];
    sprintf(track_number_string, "%i", a_track->track_num);
    
    if(a_is_fx)
    {
        sprintf(oscUrl, "%s/%i-fx", a_pydaw_data->osc_url, a_track->track_num);
    }
    else
    {
        sprintf(oscUrl, "%s/%i", a_pydaw_data->osc_url, a_track->track_num);
    }
        
#ifdef PYDAW_MEMCHECK
    v_pydaw_assert_memory_integrity(a_pydaw_data);
#endif
    
    if (fork() == 0) 
    {
        execlp(filename, filename, oscUrl, dllName, a_track->name, track_number_string, NULL);
        perror("exec failed");
        exit(1);  //TODO:  should be getting rid of this???
    }    
}

void v_pydaw_close_all_uis(t_pydaw_data * a_pydaw_data)
{
    int f_i = 0;
    
    if(a_pydaw_data)
    {
        while(f_i < PYDAW_MIDI_TRACK_COUNT)
        {
            if((a_pydaw_data->track_pool[f_i]->instrument) && a_pydaw_data->track_pool[f_i]->instrument->uiTarget)
            {
                lo_send(a_pydaw_data->track_pool[f_i]->instrument->uiTarget, 
                    a_pydaw_data->track_pool[f_i]->instrument->ui_osc_quit_path, "");
            }

            if((a_pydaw_data->track_pool[f_i]->effect) && a_pydaw_data->track_pool[f_i]->effect->uiTarget)
            {
                lo_send(a_pydaw_data->track_pool[f_i]->effect->uiTarget, 
                    a_pydaw_data->track_pool[f_i]->effect->ui_osc_quit_path, "");
            }

            f_i++;
        }
    }
}

void v_pydaw_set_track_volume(t_pydaw_data * a_pydaw_data, t_pytrack * a_track, float a_vol)
{
    a_track->volume = a_vol;
    a_track->volume_linear = f_db_to_linear_fast(a_vol, a_pydaw_data->amp_ptr);
}

void v_set_plugin_index(t_pydaw_data * a_pydaw_data, t_pytrack * a_track, int a_index)
{
    pthread_mutex_lock(&a_track->mutex);  //Prevent multiple simultaneus operations from running in parallel, without blocking the main thread
    t_pydaw_plugin * f_result;
    t_pydaw_plugin * f_result_fx;
    
    char f_file_name[512];
    sprintf(f_file_name, "%s%i.pyinst", a_pydaw_data->instruments_folder, a_track->track_num);
    
    char f_file_name_fx[512];
    sprintf(f_file_name_fx, "%s%i.pyfx", a_pydaw_data->instruments_folder, a_track->track_num);
    
    if(a_index == -1)  //bus track
    {
        f_result_fx = g_pydaw_plugin_get((int)(a_pydaw_data->sample_rate), -1);
        
        t_pydaw_plugin * f_fx = a_track->effect;
                
        pthread_mutex_lock(&a_pydaw_data->main_mutex);        
        
        a_track->instrument = 0;
        a_track->effect = f_result_fx;
        a_track->plugin_index = a_index;
        a_track->current_period_event_index = 0;
        v_pydaw_open_track(a_pydaw_data, a_track);  //Opens the .inst file if exists
        
#ifdef PYDAW_MEMCHECK
        v_pydaw_assert_memory_integrity(a_pydaw_data);
#endif
        pthread_mutex_unlock(&a_pydaw_data->main_mutex);
        
        if(f_fx)
        {        
            if(f_fx->uiTarget)
            {
                lo_send(f_fx->uiTarget, f_fx->ui_osc_quit_path, "");
            }
            v_free_pydaw_plugin(f_fx);
        }
    }
    else if(a_index == 0)  //empty
    {        
        t_pydaw_plugin * f_inst = a_track->instrument;
        t_pydaw_plugin * f_fx = a_track->effect;
                                        
        if(i_pydaw_file_exists(f_file_name))
        {
            remove(f_file_name);
        }
        if(i_pydaw_file_exists(f_file_name_fx))
        {
            remove(f_file_name_fx);
        }
        
        pthread_mutex_lock(&a_pydaw_data->main_mutex);
                
        a_track->instrument = NULL;
        a_track->effect = NULL;
        a_track->plugin_index = a_index;        
        a_track->current_period_event_index = 0;
        
#ifdef PYDAW_MEMCHECK
        v_pydaw_assert_memory_integrity(a_pydaw_data);
#endif
        pthread_mutex_unlock(&a_pydaw_data->main_mutex); 
        
        if(f_inst)
        {
            v_free_pydaw_plugin(f_inst);
        }
        
        if(f_fx)
        {
            v_free_pydaw_plugin(f_fx);
        }           
    }
    else
    {
        f_result = g_pydaw_plugin_get((int)(a_pydaw_data->sample_rate), a_index);
        f_result_fx = g_pydaw_plugin_get((int)(a_pydaw_data->sample_rate), -1);
        
        free(f_result_fx->pluginInputBuffers[0]);
        free(f_result_fx->pluginInputBuffers[1]);
        
        f_result_fx->pluginInputBuffers = f_result->pluginOutputBuffers;
        f_result_fx->descriptor->LADSPA_Plugin->connect_port(f_result_fx->ladspa_handle, 0, f_result_fx->pluginInputBuffers[0]);
        f_result_fx->descriptor->LADSPA_Plugin->connect_port(f_result_fx->ladspa_handle, 1, f_result_fx->pluginInputBuffers[1]);
                
        /* If switching from a different plugin(but not from no plugin), delete the preset file */
        if(((a_track->plugin_index) != 0) && i_pydaw_file_exists(f_file_name))
        {
            remove(f_file_name);
        }
        if(((a_track->plugin_index) != 0) && i_pydaw_file_exists(f_file_name_fx))
        {
            remove(f_file_name_fx);
        }
        
        t_pydaw_plugin * f_inst = a_track->instrument;
        t_pydaw_plugin * f_fx = a_track->effect;
                
        pthread_mutex_lock(&a_pydaw_data->main_mutex);        
        
        a_track->instrument = f_result;
        a_track->effect = f_result_fx;
        a_track->plugin_index = a_index;
        a_track->current_period_event_index = 0;
        v_pydaw_open_track(a_pydaw_data, a_track);  //Opens the .inst file if exists
        
#ifdef PYDAW_MEMCHECK
        v_pydaw_assert_memory_integrity(a_pydaw_data);
#endif
        pthread_mutex_unlock(&a_pydaw_data->main_mutex);
        
        if(f_inst)
        {        
            if(f_inst->uiTarget)
            {
                lo_send(f_inst->uiTarget, f_inst->ui_osc_quit_path, "");
            }
            v_free_pydaw_plugin(f_inst);
        }
        
        if(f_fx)
        {        
            if(f_fx->uiTarget)
            {
                lo_send(f_fx->uiTarget, f_fx->ui_osc_quit_path, "");
            }
            v_free_pydaw_plugin(f_fx);
        }
    }
    
    pthread_mutex_unlock(&a_track->mutex);
    
    if(a_index != -1)
    {
        v_pydaw_schedule_work(a_pydaw_data);
    }
    
}

#ifdef PYDAW_MEMCHECK
/* Check a_pydaw_data for all known indicators of memory integrity, and throw a SIGABRT 
 if it fails.*/
void v_pydaw_assert_memory_integrity(t_pydaw_data* a_pydaw_data)
{
    int f_i = 0;
    int f_i2 = 0;
    int f_i3 = 0;
    
    while(f_i < PYDAW_MIDI_TRACK_COUNT)
    {
        if(a_pydaw_data->track_pool[f_i]->instrument)
        {
            assert((a_pydaw_data->track_pool[f_i]->instrument->euphoria_load_set == 0) ||
                    (a_pydaw_data->track_pool[f_i]->instrument->euphoria_load_set == 1));
        }
        
        if(a_pydaw_data->track_pool[f_i]->effect)
        {
            assert((a_pydaw_data->track_pool[f_i]->effect->euphoria_load_set == 0) ||
                    (a_pydaw_data->track_pool[f_i]->effect->euphoria_load_set == 1));
        }
        
        f_i++;
    }

    f_i = 0;
    
    if(a_pydaw_data->pysong)
    {
        while(f_i3 < PYDAW_MAX_REGION_COUNT)
        {
            if(a_pydaw_data->pysong->regions[f_i3])
            {
                f_i = 0;
                while(f_i < PYDAW_MAX_REGION_SIZE)
                {
                    f_i2 = 0;
                    while(f_i2 < PYDAW_MIDI_TRACK_COUNT)
                    {
                        assert((a_pydaw_data->pysong->regions[f_i3]->item_indexes[f_i2][f_i] < a_pydaw_data->item_count) && 
                                (a_pydaw_data->pysong->regions[f_i3]->item_indexes[f_i2][f_i] >= -1));
                        f_i2++;
                    }
                    f_i++;
                }
            }
            f_i3++;
        }
    }
}
#endif


void v_pydaw_offline_render(t_pydaw_data * a_pydaw_data, int a_start_region, int a_start_bar, int a_end_region, 
        int a_end_bar, char * a_file_out)
{
    pthread_mutex_lock(&a_pydaw_data->offline_mutex);
    a_pydaw_data->is_offline_rendering = 1;
    
    int f_bar_count = a_end_bar - a_start_bar;
    
    int f_i = a_start_region;
    int f_beat_total = 0;
    
    while(f_i < a_end_region)
    {
        if((a_pydaw_data->pysong->regions[f_i]) && (a_pydaw_data->pysong->regions[f_i]->region_length_bars))
        {
            f_beat_total += a_pydaw_data->pysong->regions[f_i]->region_length_bars * 4;
        }
        else
        {
            f_beat_total += (8 * 4);
        }
        f_i++;
    }
    
    f_beat_total += f_bar_count * 4;
    
    double f_sample_count = a_pydaw_data->samples_per_beat * ((double)f_beat_total);
    long f_sample_count_long = (((long)f_sample_count) * 2) + 6000000;
    
    float * f_output = (float*)malloc(sizeof(float) * f_sample_count_long);
       
    long f_size = 0;
    long f_block_size = (a_pydaw_data->sample_count);    
    long f_next_sample_block = 0;
    float * f_buffer0 = (float*)malloc(sizeof(float) * f_block_size);
    float * f_buffer1 = (float*)malloc(sizeof(float) * f_block_size);
    
    int f_old_loop_mode = a_pydaw_data->loop_mode;  //We must set it back afterwards, or the UI will be wrong...
    v_set_loop_mode(a_pydaw_data, PYDAW_LOOP_MODE_OFF);
    v_set_playback_mode(a_pydaw_data, PYDAW_PLAYBACK_MODE_PLAY, a_start_region, a_start_bar);    
        
    clock_t f_start = clock();
    
    while(((a_pydaw_data->current_region) < a_end_region) || ((a_pydaw_data->current_bar) < a_end_bar))
    {
        f_i = 0;
    
        while(f_i < f_block_size)
        {
            f_buffer0[f_i] = 0.0f;
            f_buffer1[f_i] = 0.0f;
            f_i++;
        }
        
        pthread_mutex_lock(&a_pydaw_data->main_mutex);
        f_next_sample_block = (a_pydaw_data->current_sample) + f_block_size;        
        v_pydaw_run_main_loop(a_pydaw_data, f_block_size, NULL, 0, f_next_sample_block, f_buffer0, f_buffer1);        
        a_pydaw_data->current_sample = f_next_sample_block;
        pthread_mutex_unlock(&a_pydaw_data->main_mutex);
        
        f_i = 0;        
        /*Interleave the samples...*/
        while(f_i < f_block_size)
        {
            f_output[f_size] = f_buffer0[f_i];
            f_size++;
            f_output[f_size] = f_buffer1[f_i];
            f_size++;
            f_i++;
        }
    }
        
    v_pydaw_print_benchmark("v_pydaw_offline_render (to RAM)", f_start);
    printf("f_size = %ld\nf_sample_count_long = %ld\n(f_sample_count_long - f_size) = %ld\n", 
            f_size, f_sample_count_long, (f_sample_count_long - f_size));
    
    v_set_playback_mode(a_pydaw_data, PYDAW_PLAYBACK_MODE_OFF, a_start_region, a_start_bar);
    v_set_loop_mode(a_pydaw_data, f_old_loop_mode);
    
    SF_INFO f_sf_info;
    f_sf_info.channels = 2;
    f_sf_info.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;
    f_sf_info.samplerate = (int)(a_pydaw_data->sample_rate);
            
    SNDFILE * f_sndfile = sf_open(a_file_out, SFM_WRITE, &f_sf_info);
    printf("%i\n", (int)(sf_writef_float(f_sndfile, f_output, f_size/2)));
    sf_close(f_sndfile);
    
    free(f_buffer0);
    free(f_buffer1);
    free(f_output);    
        
    a_pydaw_data->is_offline_rendering = 0;
    pthread_mutex_unlock(&a_pydaw_data->offline_mutex);
}


void v_pydaw_parse_configure_message(t_pydaw_data* a_pydaw_data, const char* a_key, const char* a_value)
{
    //char log_buff[200];
    printf("v_pydaw_parse_configure_message:  key: \"%s\", value: \"%s\"\n", a_key, a_value);
    //sprintf(log_buff, "v_pydaw_parse_configure_message:  key: \"%s\", value: \"%s\"\n", a_key, a_value);
    //pydaw_write_log(log_buff);
    
    if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_SR)) //Save region
    {        
        t_pyregion * f_result = g_pyregion_get(a_pydaw_data, a_value);
        int f_region_index = i_get_song_index_from_region_name(a_pydaw_data, a_value);
        
        pthread_mutex_lock(&a_pydaw_data->main_mutex);
        if(a_pydaw_data->pysong->regions[f_region_index])
        {
            free(a_pydaw_data->pysong->regions[f_region_index]);
            a_pydaw_data->pysong->regions[f_region_index] = NULL;
        }
        a_pydaw_data->pysong->regions[f_region_index] = f_result;
        pthread_mutex_unlock(&a_pydaw_data->main_mutex);        
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_SI)) //Save Item
    {
        pthread_mutex_lock(&a_pydaw_data->main_mutex);
        g_pyitem_get(a_pydaw_data, a_value);
        pthread_mutex_unlock(&a_pydaw_data->main_mutex);
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_SS))  //Save Song
    {        
        g_pysong_get(a_pydaw_data);        
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
    
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_AUDIO_LOAD)) //Reload the audio items list
    {
        
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_AUDIO_SAVE)) //Save audio tracks
    {
        
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_AUDIO_SHOW_FX)) //Show audio tracks effects plugin GUI
    {
        
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_AUDIO_INPUT_SAVE)) //Stop playback or recording
    {
        
    }
    
        
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_STOP)) //Stop playback or recording
    {
        v_set_playback_mode(a_pydaw_data, 0, -1, -1);
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_LOOP)) //Set loop mode
    {
        int f_value = atoi(a_value);
        v_set_loop_mode(a_pydaw_data, f_value);
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_OS)) //Open Song
    {        
        v_open_project(a_pydaw_data, a_value);        
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_TEMPO)) //Change tempo
    {
        v_set_tempo(a_pydaw_data, atof(a_value));
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_SHOW_PLUGIN_UI))
    {
        int f_track_num = atoi(a_value);
        v_show_plugin_ui(a_pydaw_data, a_pydaw_data->track_pool[f_track_num], 0);        
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_SHOW_FX_UI))
    {
        int f_track_num = atoi(a_value);        
        v_show_plugin_ui(a_pydaw_data, a_pydaw_data->track_pool[f_track_num], 1);        
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_SAVE_TRACKS))
    {
        v_pydaw_save_tracks(a_pydaw_data);
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_DI)) //Delete Item
    {
        //Probably won't be implemented anytime soon...
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_DR)) //Delete region
    {
        //Probably won't be implemented anytime soon...
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_RR)) //Rename region
    {
        //Probably won't be implemented anytime soon...
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_RI)) //Rename item
    {
        //Probably won't be implemented anytime soon...
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_SOLO)) //Set track solo
    {
        t_1d_char_array * f_val_arr = c_split_str(a_value, '|', 2, LMS_TINY_STRING);
        int f_track_num = atoi(f_val_arr->array[0]);
        int f_mode = atoi(f_val_arr->array[1]);
        assert(f_mode == 0 || f_mode == 1);
        pthread_mutex_lock(&a_pydaw_data->main_mutex);
        pthread_mutex_lock(&a_pydaw_data->track_pool[f_track_num]->mutex);
        a_pydaw_data->track_pool[f_track_num]->solo = f_mode;
        pthread_mutex_unlock(&a_pydaw_data->main_mutex);
        pthread_mutex_unlock(&a_pydaw_data->track_pool[f_track_num]->mutex);
        g_free_1d_char_array(f_val_arr);        
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_MUTE)) //Set track mute
    {
        t_1d_char_array * f_val_arr = c_split_str(a_value, '|', 2, LMS_TINY_STRING);
        int f_track_num = atoi(f_val_arr->array[0]);
        int f_mode = atoi(f_val_arr->array[1]);
        assert(f_mode == 0 || f_mode == 1);
        pthread_mutex_lock(&a_pydaw_data->track_pool[f_track_num]->mutex);
        pthread_mutex_lock(&a_pydaw_data->main_mutex);
        a_pydaw_data->track_pool[f_track_num]->mute = f_mode;
        pthread_mutex_unlock(&a_pydaw_data->main_mutex);
        pthread_mutex_unlock(&a_pydaw_data->track_pool[f_track_num]->mutex);
        g_free_1d_char_array(f_val_arr);
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_TRACK_NAME)) //Set track name
    {
        t_1d_char_array * f_val_arr = c_split_str(a_value, '|', 2, LMS_TINY_STRING);
        int f_track_num = atoi(f_val_arr->array[0]);        
        pthread_mutex_lock(&a_pydaw_data->track_pool[f_track_num]->mutex);
        strcpy(a_pydaw_data->track_pool[f_track_num]->name, f_val_arr->array[1]);
        pthread_mutex_unlock(&a_pydaw_data->track_pool[f_track_num]->mutex);
        g_free_1d_char_array(f_val_arr);
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_REC_ARM_TRACK)) //Set track record arm
    {
        t_1d_char_array * f_val_arr = c_split_str(a_value, '|', 2, LMS_TINY_STRING);
        int f_track_num = atoi(f_val_arr->array[0]);
        int f_mode = atoi(f_val_arr->array[1]);
        assert(f_mode == 0 || f_mode == 1);
        if(f_mode)
        {
            //TODO:  This will need to be removed if PyDAW ever supports multiple MIDI input devices, just a quick hack for now
            int f_i = 0;            
            while(f_i < PYDAW_MIDI_TRACK_COUNT)
            {
                a_pydaw_data->track_pool[f_track_num]->rec = 0;
                f_i++;
            }
        }
        pthread_mutex_lock(&a_pydaw_data->track_pool[f_track_num]->mutex);
        pthread_mutex_lock(&a_pydaw_data->main_mutex);
        a_pydaw_data->track_pool[f_track_num]->rec = f_mode;
        pthread_mutex_unlock(&a_pydaw_data->main_mutex);
        pthread_mutex_unlock(&a_pydaw_data->track_pool[f_track_num]->mutex);
        g_free_1d_char_array(f_val_arr);
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_VOL)) //Set track volume
    {
        t_1d_char_array * f_val_arr = c_split_str(a_value, '|', 2, LMS_TINY_STRING);
        int f_track_num = atoi(f_val_arr->array[0]);
        float f_track_vol = atof(f_val_arr->array[1]);
        pthread_mutex_lock(&a_pydaw_data->track_pool[f_track_num]->mutex);
        pthread_mutex_lock(&a_pydaw_data->main_mutex);
        v_pydaw_set_track_volume(a_pydaw_data,  a_pydaw_data->track_pool[f_track_num], f_track_vol);
        pthread_mutex_unlock(&a_pydaw_data->main_mutex);
        pthread_mutex_unlock(&a_pydaw_data->track_pool[f_track_num]->mutex);
        g_free_1d_char_array(f_val_arr);
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_CHANGE_INSTRUMENT)) //Change the plugin
    {
        t_1d_char_array * f_val_arr = c_split_str(a_value, '|', 2, LMS_TINY_STRING);
        int f_track_num = atoi(f_val_arr->array[0]);
        int f_plugin_index = atof(f_val_arr->array[1]);
        
        v_set_plugin_index(a_pydaw_data,  a_pydaw_data->track_pool[f_track_num], f_plugin_index);
        
        g_free_1d_char_array(f_val_arr);
    }    
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_PREVIEW_SAMPLE)) //Preview a sample
    {
        printf("Sample preview not yet implemented");
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_OFFLINE_RENDER)) //Render a project to .wav file
    {
        t_1d_char_array * f_val_arr = c_split_str(a_value, '|', 7, LMS_TINY_STRING);
        int f_start_region = atoi(f_val_arr->array[0]);
        int f_start_bar = atoi(f_val_arr->array[1]);
        int f_end_region = atoi(f_val_arr->array[2]);
        int f_end_bar = atoi(f_val_arr->array[3]);
        char * f_file_out = f_val_arr->array[4];
        
        v_pydaw_offline_render(a_pydaw_data, f_start_region, f_start_bar, f_end_region, f_end_bar, f_file_out);
        g_free_1d_char_array(f_val_arr);
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_SET_TRACK_BUS)) //Set the bus number for specified track
    {
        t_1d_char_array * f_val_arr = c_split_str(a_value, '|', 2, LMS_TINY_STRING);
        int f_track_num = atoi(f_val_arr->array[0]);
        int f_bus_num = atoi(f_val_arr->array[1]);
        
        pthread_mutex_lock(&a_pydaw_data->track_pool[f_track_num]->mutex);
        a_pydaw_data->track_pool[f_track_num]->bus_num = f_bus_num;
        pthread_mutex_unlock(&a_pydaw_data->track_pool[f_track_num]->mutex);
        
        g_free_1d_char_array(f_val_arr);
    }       
    else
    {
        printf("Unknown configure message key: %s, value %s\n", a_key, a_value);
    }
}


#ifdef	__cplusplus
}
#endif

#endif	/* PYDAW_H */

