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


#ifndef PYDAW_H
#define	PYDAW_H

#ifdef	__cplusplus
extern "C" {
#endif
  
//Required for sched.h
//#define __USE_GNU
    
//Uncomment this to constantly inspect the heap for known signs of corruption and throw a SIGABRT upon detection.
//#define PYDAW_MEMCHECK
    
//Print detailed information about the MIDI data being sent from PyDAW, or alternately pass in from 'make' with -DPYDAW_PRINT_DEBUG_INFO
//#define PYDAW_PRINT_DEBUG_INFO
    
//#define PYDAW_DEBUG_RECORDING
    
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
#define PYDAW_CONFIGURE_KEY_SAVE_TRACKS "st"
#define PYDAW_CONFIGURE_KEY_REC_ARM_TRACK "tr"
#define PYDAW_CONFIGURE_KEY_SHOW_FX_UI "fx"
#define PYDAW_CONFIGURE_KEY_TRACK_NAME "tn"
    
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
    
#define PYDAW_LOOP_MODE_OFF 0
#define PYDAW_LOOP_MODE_BAR 1
#define PYDAW_LOOP_MODE_REGION 2
    
#define PYDAW_PLAYBACK_MODE_OFF 0
#define PYDAW_PLAYBACK_MODE_PLAY 1
#define PYDAW_PLAYBACK_MODE_REC 2
    
//arbitrary, I may change these 3 after evaluating memory use vs. probable item count in a real project
#define PYDAW_MAX_ITEM_COUNT 5000
#define PYDAW_MAX_REGION_COUNT 300
#define PYDAW_MAX_EVENTS_PER_ITEM_COUNT 256
    
//#define PYDAW_MAX_EVENTS_PER_TRACK_COUNT 2048  //Song-level CC events
    
#define PYDAW_BUS_TRACK_COUNT 5
#define PYDAW_AUDIO_INPUT_TRACK_COUNT 5
#define PYDAW_AUDIO_TRACK_COUNT 8
#define PYDAW_MIDI_TRACK_COUNT 20
#define PYDAW_TRACK_COUNT_ALL (PYDAW_BUS_TRACK_COUNT + PYDAW_AUDIO_TRACK_COUNT + PYDAW_MIDI_TRACK_COUNT)
    
#define PYDAW_MAX_EVENT_BUFFER_SIZE 512  //This could probably be made smaller
#define PYDAW_MAX_REGION_SIZE 32
#define PYDAW_MIDI_NOTE_COUNT 128
#define PYDAW_MIDI_RECORD_BUFFER_LENGTH 600 //(PYDAW_MAX_REGION_COUNT * PYDAW_REGION_SIZE)  //recording buffer for MIDI, in bars
#define PYDAW_MAX_WORK_ITEMS_PER_THREAD 128
    
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "pydaw_files.h"
#include "pydaw_plugin_wrapper.h"
#include <sys/stat.h>
#include <sched.h>
#include <unistd.h>
#include <sndfile.h>
#include <time.h>
#include "../../libmodsynth/lib/amp.h"
#include "../../libmodsynth/modules/multifx/multifx3knob.h"
#include "pydaw_audio_tracks.h"
#include "pydaw_sample_graph.h"
#include "pydaw_audio_inputs.h"
#include "pydaw_audio_util.h"
#include <lo/lo.h>
  
typedef struct
{
    int effects_only;
    int rayv_port;
    int wayv_port;
    int modulex_port;
    int euphoria_port;    
}t_py_cc_map_item;
   
typedef struct
{
    int note;
    int velocity;
    float start;
    float length;    
}t_pynote;

typedef struct
{
    //int cc_num;
    int plugin_index;
    int port;
    float cc_val;
    float start;
}t_pycc;

typedef struct
{
    float val;
    float start;
} t_pypitchbend;

typedef struct
{
    t_pynote * notes[PYDAW_MAX_EVENTS_PER_ITEM_COUNT];
    int note_count;    
    t_pycc * ccs[PYDAW_MAX_EVENTS_PER_ITEM_COUNT];
    int cc_count;
    t_pypitchbend * pitchbends[PYDAW_MAX_EVENTS_PER_ITEM_COUNT];
    int pitchbend_count;
    //int note_index[PYDAW_MAX_EVENTS_PER_ITEM_COUNT];
    //int cc_index[PYDAW_MAX_EVENTS_PER_ITEM_COUNT];
    int uid;
}t_pyitem;

typedef struct
{
    int item_indexes[PYDAW_TRACK_COUNT_ALL][PYDAW_MAX_REGION_SIZE];  //Refers to the index of items in the master item pool     
    int uid;
    /*This flag is set to 1 if created during recording, signifying that it requires a default name to be created for it*/
    int not_yet_saved;
    
    int region_length_bars;    //0 to use pydaw_data default
    int region_length_beats;    //0 to use pydaw_data default
    int bar_length;  //0 to use pydaw_data default
    int alternate_tempo;  //0 or 1, used as a boolean
    float tempo;
}t_pyregion;

typedef struct
{
    float a_knobs[3];
    int fx_type;
    fp_mf3_run func_ptr;
    t_mf3_multi * mf3;
}t_pydaw_per_audio_item_fx_item;

typedef struct
{
    int loaded[PYDAW_MAX_AUDIO_ITEM_COUNT];
    t_pydaw_per_audio_item_fx_item * items[PYDAW_MAX_AUDIO_ITEM_COUNT][8];
}t_pydaw_per_audio_item_fx_region;

typedef struct
{
    t_pyregion * regions[PYDAW_MAX_REGION_COUNT];
    t_pydaw_audio_items * audio_items[PYDAW_MAX_REGION_COUNT];
    t_pydaw_per_audio_item_fx_region * per_audio_item_fx[PYDAW_MAX_REGION_COUNT];
    int default_bar_length;
}t_pysong;

typedef struct
{
    float volume;
    float volume_linear;
    int solo;
    int mute;    
    int plugin_index;  //-1 == bus track, 0 == off, 1 == Euphoria, 2 == Ray-V, 3 == Way-V
    int bus_num;
    snd_seq_event_t * event_buffer;
    int current_period_event_index;
    t_pydaw_plugin * instrument;
    t_pydaw_plugin * effect;
    pthread_mutex_t mutex;
    char name[64];
    int track_num;  //Track number for the pool of tracks it is in, for example: 0-15 for midi tracks
    int track_type;  //0 == MIDI/plugin-instrument, 1 == Bus, 2 == Audio
    
    int bus_count;  //Only for busses, the count of plugins writing to the buffer
    int bus_counter;  //This is reset to bus_count each cycle and the bus track processed when count reaches 0
    int bus_buffer_state; //0 = Not ready, 1 = being cleared, 2 = ready    
}t_pytrack;

typedef struct
{
    int track_number;
    int track_type;  //Valid types:  0 == MIDI, 2 == Audio  (1==Bus is not valid for this purpose)
}t_pydaw_work_queue_item;

typedef struct
{
    int sample_count;   //set from the Jack buffer size every time the main loop is called..
    float tempo;
    pthread_mutex_t main_mutex;
    t_pysong * pysong;
    t_pytrack * track_pool[PYDAW_MIDI_TRACK_COUNT];
    t_pytrack * record_armed_track;
    int record_armed_track_index_all;  //index within track_pool_all
    t_pytrack * bus_pool[PYDAW_BUS_TRACK_COUNT];
    pthread_spinlock_t bus_spinlocks[PYDAW_BUS_TRACK_COUNT];
    t_pytrack * audio_track_pool[PYDAW_AUDIO_TRACK_COUNT];
    t_pytrack * track_pool_all[PYDAW_TRACK_COUNT_ALL];  //contains a reference to all track types, in order:  MIDI, Bus, Audio
    t_pyaudio_input * audio_inputs[PYDAW_AUDIO_INPUT_TRACK_COUNT];
    int track_current_item_note_event_indexes[PYDAW_MIDI_TRACK_COUNT];
    int track_current_item_cc_event_indexes[PYDAW_TRACK_COUNT_ALL];
    int track_current_item_pitchbend_event_indexes[PYDAW_MIDI_TRACK_COUNT];
    int playback_mode;  //0 == Stop, 1 == Play, 2 == Rec
    int loop_mode;  //0 == Off, 1 == Bar, 2 == Region
    int overdub_mode;  //0 == Off, 1 == On
    //char * project_name;
    char * project_folder;
    char * instruments_folder;
    char * item_folder;
    char * region_folder;
    char * region_audio_folder;
    char * audio_folder;
    char * audio_tmp_folder;    
    char * samples_folder;
    char * samplegraph_folder;
    char * recorded_items_file;
    char * recorded_regions_file;
    char * wav_pool_file;
    char * busfx_folder;
    char * audiofx_folder;
        
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
    
    //pthread_mutex_t * track_cond_mutex;  //For signaling to process the instruments
    pthread_cond_t * track_cond;   //For broadcasting to the threads that it's time to process the tracks
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
    t_pydaw_audio_item * audio_items_running[512];
            
    //Main-loop variables, prefixed with ml_
    double ml_sample_period_inc;
    double ml_sample_period_inc_beats;
    double ml_next_playback_cursor;
    double ml_current_period_beats;
    double ml_next_period_beats;
    //New additions, to eventually replace some of the older variables
    int ml_current_region;
    int ml_current_bar;
    double ml_current_beat;
    int ml_next_region;
    int ml_next_bar;
    double ml_next_beat;
    int ml_starting_new_bar;  //1 if a new bar starts in this sample period, 0 otherwise
    int ml_is_looping;  //0 if false, or 1 if the next bar loops.  Consumers of this value should check for the ->loop_mode variable..
    
    float ** input_buffers;
    int input_buffers_active;
    pthread_mutex_t audio_inputs_mutex;
    
    pthread_t audio_recording_thread;
    int audio_recording_quit_notifier;
    int rec_region_current_uid;
    int rec_item_current_uid;
    int suppress_new_audio_items;  //used to prevent new audio items from playing while the existing are being faded out.
    t_py_cc_map_item * cc_map[PYDAW_MIDI_NOTE_COUNT];
    t_wav_pool * wav_pool;
    t_wav_pool_item * ab_wav_item;
    t_pydaw_audio_item * ab_audio_item;
    int ab_mode;  //0 == off, 1 == on
    float ab_start; //0.0f to 1.0f
    int is_ab_ing;  //Set this to a_pydaw_data->ab_mode on playback
    float ab_amp_lin;
    t_cubic_interpolater * cubic_interpolator;
    t_wav_pool_item * preview_wav_item;
    t_pydaw_audio_item * preview_audio_item;
    int preview_mode;  //0 == off, 1 == on
    float preview_start; //0.0f to 1.0f
    int is_previewing;  //Set this to a_pydaw_data->ab_mode on playback
    float preview_amp_lin;
    int preview_max_sample_count;        
    char * per_audio_item_fx_folder;
    lo_address uiTarget;
}t_pydaw_data;

typedef struct 
{
    t_pydaw_data * pydaw_data;
    int thread_num;
}t_pydaw_thread_args;

void g_pysong_get(t_pydaw_data*);
t_pytrack * g_pytrack_get(int,int);
t_pyregion * g_pyregion_get(t_pydaw_data* a_pydaw, const int);
void g_pyitem_get(t_pydaw_data*, int);
int g_pyitem_clone(t_pydaw_data * a_pydaw_data, int a_item_index);
t_pycc * g_pycc_get(int, int, float, float);
t_pypitchbend * g_pypitchbend_get(float a_start, float a_value);
t_pynote * g_pynote_get(int a_note, int a_vel, float a_start, float a_length);
t_pydaw_data * g_pydaw_data_get(float);
int i_get_region_index_from_name(t_pydaw_data *, int);
void v_open_project(t_pydaw_data*, const char*, int);
void v_set_tempo(t_pydaw_data*,float);
void v_pydaw_set_is_soloed(t_pydaw_data * a_pydaw_data);
void v_set_loop_mode(t_pydaw_data * a_pydaw_data, int a_mode);
void v_set_playback_cursor(t_pydaw_data * a_pydaw_data, int a_region, int a_bar);
void v_pydaw_parse_configure_message(t_pydaw_data*, const char*, const char*);
int i_pydaw_get_item_index_from_uid(t_pydaw_data *, int);
void v_set_plugin_index(t_pydaw_data * a_pydaw_data, t_pytrack * a_track, int a_index, int a_schedule_threads);
void v_pydaw_assert_memory_integrity(t_pydaw_data* a_pydaw_data);
int i_get_song_index_from_region_uid(t_pydaw_data*, int);
void v_save_pysong_to_disk(t_pydaw_data * a_pydaw_data);
void v_save_pyitem_to_disk(t_pydaw_data * a_pydaw_data, int a_index);
void v_save_pyregion_to_disk(t_pydaw_data * a_pydaw_data, int a_region_num);
void v_pydaw_save_plugin(t_pydaw_data * a_pydaw_data, t_pytrack * a_track, int a_is_fx, int a_type);
void v_pydaw_open_plugin(t_pydaw_data * a_pydaw_data, t_pytrack * a_track, int a_is_fx);
int g_pyitem_get_new(t_pydaw_data* a_pydaw_data);
t_pyregion * g_pyregion_get_new(t_pydaw_data* a_pydaw_data);
void v_pydaw_set_track_volume(t_pydaw_data * a_pydaw_data,  t_pytrack * a_track, float a_vol);
inline void v_pydaw_update_ports(t_pydaw_plugin * a_plugin);
void * v_pydaw_worker_thread(void*);
void v_pydaw_init_worker_threads(t_pydaw_data*);
void v_open_default_project(t_pydaw_data * a_data);
inline void v_pydaw_process_external_midi(t_pydaw_data * pydaw_data, int sample_count, snd_seq_event_t *events, int event_count);
inline void v_pydaw_run_main_loop(t_pydaw_data * pydaw_data, int sample_count, 
        snd_seq_event_t *events, int event_count, long f_next_current_sample, 
        PYFX_Data *output0, PYFX_Data *output1, PYFX_Data **a_input_buffers);
void v_pydaw_offline_render(t_pydaw_data * a_pydaw_data, int a_start_region, int a_start_bar, int a_end_region, 
        int a_end_bar, char * a_file_out);
inline void v_pydaw_schedule_work(t_pydaw_data * a_pydaw_data);
void v_pydaw_process_plugins_single_threaded(t_pydaw_data * a_pydaw_data);
void v_pydaw_print_benchmark(char * a_message, clock_t a_start);
void v_pydaw_init_busses(t_pydaw_data * a_pydaw_data);
inline int v_pydaw_audio_items_run(t_pydaw_data * a_pydaw_data, int a_sample_count, float* a_output0, 
        float* a_output1, int a_audio_track_num);
void v_pydaw_reset_audio_item_read_heads(t_pydaw_data * a_pydaw_data, int a_region, int a_bar);
void v_pydaw_update_audio_inputs(t_pydaw_data * a_pydaw_data);
void * v_pydaw_audio_recording_thread(void* a_arg);
inline float v_pydaw_count_beats(t_pydaw_data * a_pydaw_data, int a_start_region, int a_start_bar, float a_start_beat,
        int a_end_region, int a_end_bar, float a_end_beat);
t_py_cc_map_item * g_py_cc_map_item_get(int a_effects_only, int a_rayv_port, int a_wayv_port, int a_euphoria_port, int a_modulex_port);
void v_pydaw_load_cc_map(t_pydaw_data * a_pydaw_data, const char * a_name);
t_pydaw_audio_items * v_audio_items_load_all(t_pydaw_data * a_pydaw_data, int a_region_uid);

void v_pydaw_set_ab_mode(t_pydaw_data * a_pydaw_data, int a_mode);
void v_pydaw_set_ab_start(t_pydaw_data * a_pydaw_data, int a_start);
void v_pydaw_set_ab_file(t_pydaw_data * a_pydaw_data, const char * a_file);
void v_pydaw_set_ab_vol(t_pydaw_data * a_pydaw_data, float a_vol);

t_pydaw_per_audio_item_fx_region * g_paif_region_get();
void v_paif_region_free(t_pydaw_per_audio_item_fx_region*);
t_pydaw_per_audio_item_fx_region * g_paif_region_open(t_pydaw_data*, int);
t_pydaw_per_audio_item_fx_item * g_paif_item_get(t_pydaw_data *);
void v_paif_set_control(t_pydaw_data *, int, int, int, float);

void v_pysong_free(t_pysong *);

/*End declarations.  Begin implementations.*/


void v_pysong_free(t_pysong * a_pysong)
{
    int f_i = 0;
    while(f_i < PYDAW_MAX_REGION_COUNT)
    {
        if(a_pysong->audio_items[f_i])
        {
            v_pydaw_audio_items_free(a_pysong->audio_items[f_i]);
        }
        
        if(a_pysong->per_audio_item_fx[f_i])
        {
            v_paif_region_free(a_pysong->per_audio_item_fx[f_i]);
        }
        
        if(a_pysong->regions[f_i])
        {
            free(a_pysong->regions[f_i]);
        }
        
        f_i++;
    }    
}

t_pydaw_per_audio_item_fx_region * g_paif_region_get()
{
    t_pydaw_per_audio_item_fx_region * f_result = (t_pydaw_per_audio_item_fx_region*)malloc(sizeof(t_pydaw_per_audio_item_fx_region));
    
    int f_i = 0;
    
    while(f_i < PYDAW_MAX_AUDIO_ITEM_COUNT)
    {
        f_result->loaded[f_i] = 0;
        int f_i2 = 0;
        while(f_i2 < 8)
        {
            f_result->items[f_i][f_i2] = 0;
            f_i2++;
        }
        f_i++;
    }
    
    return f_result;
}

void v_paif_region_free(t_pydaw_per_audio_item_fx_region * a_paif)
{
    int f_i = 0;
    while(f_i < PYDAW_MAX_AUDIO_ITEM_COUNT)
    {
        int f_i2 = 0;
        while(f_i2 < 8)
        {
            if(a_paif->items[f_i][f_i2])
            {
                v_mf3_free(a_paif->items[f_i][f_i2]->mf3);                
                free(a_paif->items[f_i][f_i2]);
                a_paif->items[f_i][f_i2] = 0;
            }
            f_i2++;
        }
        f_i++;
    }
    free(a_paif);
}

/* t_pydaw_per_audio_item_fx_region * g_paif_region_open(t_pydaw_data * a_pydaw_data, int a_region_uid) */
t_pydaw_per_audio_item_fx_region * g_paif_region_open(t_pydaw_data * a_pydaw_data, int a_region_uid)
{
    t_pydaw_per_audio_item_fx_region * f_result = g_paif_region_get();
    
    int f_i = 0;
    char f_temp[256];    
    sprintf(f_temp, "%s%i", a_pydaw_data->per_audio_item_fx_folder, a_region_uid);
    if(i_pydaw_file_exists(f_temp))
    {
        t_2d_char_array * f_current_string = g_get_2d_array_from_file(f_temp, LMS_LARGE_STRING);                
        while(f_i < PYDAW_MAX_AUDIO_ITEM_COUNT)
        {            
            char * f_index_char = c_iterate_2d_char_array(f_current_string);
            if(f_current_string->eof)
            {
                free(f_index_char);
                break;
            }
            int f_index = atoi(f_index_char);
            free(f_index_char);
            
            f_result->loaded[f_index] = 1;
            
            int f_i2 = 0;
            
            while(f_i2 < 8)
            {
                f_result->items[f_index][f_i2] = g_paif_item_get(a_pydaw_data);
                int f_i3 = 0;
                while(f_i3 < 3)
                {
                    char * f_knob_char = c_iterate_2d_char_array(f_current_string);
                    float f_knob_val = atof(f_knob_char);
                    free(f_knob_char);
                    f_result->items[f_index][f_i2]->a_knobs[f_i3] = f_knob_val;
                    f_i3++;
                }
                char * f_type_char = c_iterate_2d_char_array(f_current_string);
                int f_type_val = atoi(f_type_char);
                free(f_type_char);
                f_result->items[f_index][f_i2]->fx_type = f_type_val;
                f_result->items[f_index][f_i2]->func_ptr = g_mf3_get_function_pointer(f_type_val);
                v_mf3_set(f_result->items[f_index][f_i2]->mf3,
                        f_result->items[f_index][f_i2]->a_knobs[0],
                        f_result->items[f_index][f_i2]->a_knobs[1],
                        f_result->items[f_index][f_i2]->a_knobs[2]);
                f_i2++;
            }            
            
            f_i++;
        }

        g_free_2d_char_array(f_current_string);        
    }
    
    return f_result;    
}

t_pydaw_per_audio_item_fx_item * g_paif_item_get(t_pydaw_data * a_pydaw_data)
{
    t_pydaw_per_audio_item_fx_item * f_result = (t_pydaw_per_audio_item_fx_item*)malloc(sizeof(t_pydaw_per_audio_item_fx_item));
    
    int f_i = 0;
    while(f_i < 3)
    {
        f_result->a_knobs[f_i] = 64.0f;
        f_i++;
    }
    f_result->fx_type = 0;
    f_result->func_ptr = v_mf3_run_off;
    f_result->mf3 = g_mf3_get(a_pydaw_data->sample_rate);
    
    return f_result;
}

void v_paif_set_control(t_pydaw_data * a_pydaw_data, int a_region_uid, int a_item_index, int a_port, float a_val)
{
    int f_effect_index = a_port / 4;
    int f_control_index = a_port % 4;
    int f_song_index = i_get_song_index_from_region_uid(a_pydaw_data, a_region_uid);
        
    if(!a_pydaw_data->pysong->per_audio_item_fx[f_song_index])
    {
        t_pydaw_per_audio_item_fx_region * f_region = g_paif_region_get();
        
        pthread_mutex_lock(&a_pydaw_data->main_mutex);
        a_pydaw_data->pysong->per_audio_item_fx[f_song_index] = f_region;
        pthread_mutex_unlock(&a_pydaw_data->main_mutex);
    }
    
    if(!a_pydaw_data->pysong->per_audio_item_fx[f_song_index]->loaded[a_item_index])
    {
        t_pydaw_per_audio_item_fx_item * f_items[8];
        int f_i = 0;
        while(f_i < 8)
        {
            f_items[f_i] = g_paif_item_get(a_pydaw_data);
            f_i++;
        }
        pthread_mutex_lock(&a_pydaw_data->main_mutex);
        f_i = 0;
        while(f_i < 8)
        {
            a_pydaw_data->pysong->per_audio_item_fx[f_song_index]->items[a_item_index][f_i] = f_items[f_i];
            f_i++;
        }
        a_pydaw_data->pysong->per_audio_item_fx[f_song_index]->loaded[a_item_index] = 1;
        pthread_mutex_unlock(&a_pydaw_data->main_mutex);
    }
    
    pthread_mutex_lock(&a_pydaw_data->main_mutex);
    
    if(f_control_index == 3)
    {
        int f_fx_index = (int)a_val;
        a_pydaw_data->pysong->per_audio_item_fx[f_song_index]->items[a_item_index][f_effect_index]->fx_type = f_fx_index;
        a_pydaw_data->pysong->per_audio_item_fx[f_song_index]->items[a_item_index][f_effect_index]->func_ptr = g_mf3_get_function_pointer(f_fx_index);
        
        v_mf3_set(a_pydaw_data->pysong->per_audio_item_fx[f_song_index]->items[a_item_index][f_effect_index]->mf3,
            a_pydaw_data->pysong->per_audio_item_fx[f_song_index]->items[a_item_index][f_effect_index]->a_knobs[0],
            a_pydaw_data->pysong->per_audio_item_fx[f_song_index]->items[a_item_index][f_effect_index]->a_knobs[1],
            a_pydaw_data->pysong->per_audio_item_fx[f_song_index]->items[a_item_index][f_effect_index]->a_knobs[2]);
    }
    else
    {
        a_pydaw_data->pysong->per_audio_item_fx[f_song_index]->items[a_item_index][f_effect_index]->a_knobs[f_control_index] = a_val;
        
        v_mf3_set(a_pydaw_data->pysong->per_audio_item_fx[f_song_index]->items[a_item_index][f_effect_index]->mf3,
            a_pydaw_data->pysong->per_audio_item_fx[f_song_index]->items[a_item_index][f_effect_index]->a_knobs[0],
            a_pydaw_data->pysong->per_audio_item_fx[f_song_index]->items[a_item_index][f_effect_index]->a_knobs[1],
            a_pydaw_data->pysong->per_audio_item_fx[f_song_index]->items[a_item_index][f_effect_index]->a_knobs[2]);
    }
        
    pthread_mutex_unlock(&a_pydaw_data->main_mutex);
    
}

void v_pydaw_init_busses(t_pydaw_data * a_pydaw_data)
{
    int f_i = 0;
    
    while(f_i < PYDAW_BUS_TRACK_COUNT)
    {
        v_set_plugin_index(a_pydaw_data, a_pydaw_data->bus_pool[f_i], -1, 0);
        f_i++;
    }
    
    f_i = 0;
    
    while(f_i < PYDAW_AUDIO_TRACK_COUNT)
    {
        v_set_plugin_index(a_pydaw_data, a_pydaw_data->audio_track_pool[f_i], -1, 0);
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
    
    a_pydaw_data->track_worker_thread_count = sysconf( _SC_NPROCESSORS_ONLN );
    
    if((a_pydaw_data->track_worker_thread_count) > 4)
    {
        a_pydaw_data->track_worker_thread_count = 4;
    }
    else if((a_pydaw_data->track_worker_thread_count) == 4)
    {
        a_pydaw_data->track_worker_thread_count = 3;
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

    //a_pydaw_data->track_cond_mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t) * (a_pydaw_data->track_worker_thread_count));
    a_pydaw_data->track_cond = (pthread_cond_t*)malloc(sizeof(pthread_cond_t) * (a_pydaw_data->track_worker_thread_count));
    
    while(f_i < (a_pydaw_data->track_worker_thread_count))
    {
        //pthread_mutex_init(&a_pydaw_data->track_cond_mutex[f_i], NULL);
        pthread_cond_init(&a_pydaw_data->track_cond[f_i], NULL);
    
        pthread_mutex_init(&a_pydaw_data->track_block_mutexes[f_i], NULL);
        a_pydaw_data->track_work_queues[f_i] = (t_pydaw_work_queue_item*)malloc(sizeof(t_pydaw_work_queue_item) * PYDAW_MAX_WORK_ITEMS_PER_THREAD);
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
    
    a_pydaw_data->audio_recording_quit_notifier = 0;
    
    /*The worker thread for flushing recorded audio from memory to disk*/
    //No longer recording in PyDAW, but keeping the code here for a future fork that will..
    /*pthread_attr_t threadAttr;
    struct sched_param param;
    param.__sched_priority = 90;
    pthread_attr_init(&threadAttr);
    pthread_attr_setschedparam(&threadAttr, &param);
    pthread_attr_setstacksize(&threadAttr, 8388608); 
    pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_DETACHED);
    pthread_create(&a_pydaw_data->audio_recording_thread, &threadAttr, v_pydaw_audio_recording_thread, (void*)a_pydaw_data);
    pthread_attr_destroy(&threadAttr);*/
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

inline void v_pydaw_fx_update_ports(t_pydaw_data * a_pydaw_data, t_pydaw_plugin * a_plugin, int a_track_type, int a_track_num, int a_is_inst)
{
    int f_i = 0;    
    while(f_i < (a_plugin->controlIns))
    {
        if (a_plugin->pluginPortUpdated[f_i]) 
        {
            int port = a_plugin->pluginControlInPortNumbers[f_i];
            float value = a_plugin->pluginControlIns[f_i];

            a_plugin->pluginPortUpdated[f_i] = 0;
            char a_value[256];
            sprintf(a_value, "%i|%i|%i|%i|%f", a_is_inst, a_track_type, a_track_num, port, value);            
            lo_send(a_pydaw_data->uiTarget, "pydaw/ui_configure", "ss", "pc", a_value);            
        }
        f_i++;
    }
}

inline void v_pydaw_set_bus_counters(t_pydaw_data * a_pydaw_data)
{    
    int f_i = 0;
    
    while(f_i < PYDAW_BUS_TRACK_COUNT)
    {
        a_pydaw_data->bus_pool[f_i]->bus_count = 0;
        f_i++;
    }
    
    f_i = 0;
    
    while(f_i < PYDAW_MIDI_TRACK_COUNT)
    {
        if((a_pydaw_data->track_pool[f_i]->plugin_index) > 0)
        {
            a_pydaw_data->bus_pool[(a_pydaw_data->track_pool[f_i]->bus_num)]->bus_count = 
                    (a_pydaw_data->bus_pool[(a_pydaw_data->track_pool[f_i]->bus_num)]->bus_count) + 1;
        }
        f_i++;
    }
    
    f_i = 0;
    
    while(f_i < PYDAW_AUDIO_TRACK_COUNT)
    {
        a_pydaw_data->bus_pool[(a_pydaw_data->audio_track_pool[f_i]->bus_num)]->bus_count = 
                (a_pydaw_data->bus_pool[(a_pydaw_data->audio_track_pool[f_i]->bus_num)]->bus_count) + 1;
        f_i++;
    }    
    
    f_i = 1;
    
    while(f_i < PYDAW_BUS_TRACK_COUNT)    
    {
        if(a_pydaw_data->bus_pool[f_i]->bus_count > 0)
        {
                a_pydaw_data->bus_pool[0]->bus_count = (a_pydaw_data->bus_pool[0]->bus_count) + 1;
        }
        f_i++;
    }
    
    f_i = 0;
    
    while(f_i < PYDAW_BUS_TRACK_COUNT)
    {
        a_pydaw_data->bus_pool[f_i]->bus_counter = (a_pydaw_data->bus_pool[f_i]->bus_count);
        //printf("bus[%i] count == %i\n", f_i, a_pydaw_data->bus_pool[f_i]->bus_count);
        f_i++;
    }
    
}

inline void v_pydaw_run_pre_effect_vol(t_pydaw_data * a_pydaw_data, t_pytrack * a_track)
{
    int f_i = 0;
    while(f_i < (a_pydaw_data->sample_count))
    {
        a_track->effect->pluginInputBuffers[0][f_i] = 
                (a_track->effect->pluginInputBuffers[0][f_i]) * (a_track->volume_linear);
        a_track->effect->pluginInputBuffers[1][f_i] = 
                (a_track->effect->pluginInputBuffers[1][f_i]) * (a_track->volume_linear);
        f_i++;
    }
}

inline void v_pydaw_sum_track_outputs(t_pydaw_data * a_pydaw_data, t_pytrack * a_track, int bus_num)
{
    pthread_spin_lock(&a_pydaw_data->bus_spinlocks[bus_num]);
    
    if((a_pydaw_data->bus_pool[(bus_num)]->bus_buffer_state) == 0)
    {
        a_pydaw_data->bus_pool[(bus_num)]->bus_buffer_state = 1;

        int f_i2 = 0;

        while(f_i2 < a_pydaw_data->sample_count)
        {
            a_pydaw_data->bus_pool[(bus_num)]->effect->pluginInputBuffers[0][f_i2] = 
                    (a_track->effect->pluginOutputBuffers[0][f_i2]);
            a_pydaw_data->bus_pool[(bus_num)]->effect->pluginInputBuffers[1][f_i2] = 
                    (a_track->effect->pluginOutputBuffers[1][f_i2]);
            f_i2++;
        }

        a_pydaw_data->bus_pool[(bus_num)]->bus_buffer_state = 2;
    }
    else if((a_pydaw_data->bus_pool[(bus_num)]->bus_buffer_state) == 2)
    {
        int f_i2 = 0;

        while(f_i2 < a_pydaw_data->sample_count)
        {
            a_pydaw_data->bus_pool[(bus_num)]->effect->pluginInputBuffers[0][f_i2] += 
                    (a_track->effect->pluginOutputBuffers[0][f_i2]);
            a_pydaw_data->bus_pool[(bus_num)]->effect->pluginInputBuffers[1][f_i2] += 
                    (a_track->effect->pluginOutputBuffers[1][f_i2]);
            f_i2++;
        }
    }

    a_pydaw_data->bus_pool[(bus_num)]->bus_counter =
            (a_pydaw_data->bus_pool[(bus_num)]->bus_counter) - 1;
    
    pthread_spin_unlock(&a_pydaw_data->bus_spinlocks[bus_num]);
}

void * v_pydaw_audio_recording_thread(void* a_arg)
{
    t_pydaw_data * a_pydaw_data = (t_pydaw_data*)(a_arg);
    char f_file_name[256];
    
    sleep(3);    
    
    while(1)
    {        
        int f_flushed_buffer = 0;
        int f_did_something = 0;
        
        if(a_pydaw_data->audio_recording_quit_notifier)
        {        
            printf("audio recording thread exiting...\n");            
            break;
        }
        
        pthread_mutex_lock(&a_pydaw_data->audio_inputs_mutex);
        
        if(a_pydaw_data->playback_mode == PYDAW_PLAYBACK_MODE_REC)
        {
            int f_i = 0;            
                        
            while(f_i < PYDAW_AUDIO_INPUT_TRACK_COUNT)
            {
                if((a_pydaw_data->audio_inputs[f_i]->rec) &&
                    (a_pydaw_data->audio_inputs[f_i]->flush_last_buffer_pending))
                {
                    f_flushed_buffer = 1;
                    printf("Flushing record buffer of %i frames\n",
                            ((a_pydaw_data->audio_inputs[f_i]->buffer_iterator[(a_pydaw_data->audio_inputs[f_i]->buffer_to_flush)]) / 2));
                    
                    sf_writef_float(a_pydaw_data->audio_inputs[f_i]->sndfile, 
                            a_pydaw_data->audio_inputs[f_i]->rec_buffers[(a_pydaw_data->audio_inputs[f_i]->buffer_to_flush)], 
                            ((a_pydaw_data->audio_inputs[f_i]->buffer_iterator[(a_pydaw_data->audio_inputs[f_i]->buffer_to_flush)]) / 2) );
                 
                    a_pydaw_data->audio_inputs[f_i]->flush_last_buffer_pending = 0;
                    a_pydaw_data->audio_inputs[f_i]->buffer_iterator[(a_pydaw_data->audio_inputs[f_i]->buffer_to_flush)] = 0;
                }
                                
                f_i++;
            }
        }
        else
        {
            int f_i = 0;
                        
            while(f_i < PYDAW_AUDIO_INPUT_TRACK_COUNT)
            {
                //I guess the main mutex keeps this concurrent, as the set_playback_mode has to grab it before
                //setting the recording_stopped flag, which means we won't wind up with half-a-buffer, even if this
                //thread uses lockless techniques while running fast-and-loose with the data...  TODO:  verify that this is safe...                
                if(a_pydaw_data->audio_inputs[f_i]->recording_stopped)
                {
                    f_did_something = 1;
                    sf_writef_float(a_pydaw_data->audio_inputs[f_i]->sndfile, 
                            a_pydaw_data->audio_inputs[f_i]->rec_buffers[(a_pydaw_data->audio_inputs[f_i]->current_buffer)], 
                            ((a_pydaw_data->audio_inputs[f_i]->buffer_iterator[(a_pydaw_data->audio_inputs[f_i]->current_buffer)]) / 2) );
                                     
                    sf_close(a_pydaw_data->audio_inputs[f_i]->sndfile);                    
                    a_pydaw_data->audio_inputs[f_i]->recording_stopped = 0;
                    a_pydaw_data->audio_inputs[f_i]->sndfile = 0;
                }
                f_i++;
            }
            
            //Re-create the sndfile if it no longer exists, that means the UI has moved it from the tmp folder...
            f_i = 0;
                        
            while(f_i < PYDAW_AUDIO_INPUT_TRACK_COUNT)
            {
                if(a_pydaw_data->audio_inputs[f_i]->rec)
                {
                    f_did_something = 1;
                                        
                    sprintf(f_file_name, "%s%i.wav", a_pydaw_data->audio_tmp_folder, f_i);
                    
                    if(!i_pydaw_file_exists(f_file_name))
                    {
                        v_pydaw_audio_input_record_set(a_pydaw_data->audio_inputs[f_i], f_file_name);
                    }
                }
                f_i++;
            }
            
        }
        
        pthread_mutex_unlock(&a_pydaw_data->audio_inputs_mutex);
        
        if(!f_flushed_buffer || !f_did_something)
        {
            usleep(10000);
        }
    }
    
    return (void*)1;
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
        pthread_cond_wait(&f_args->pydaw_data->track_cond[f_args->thread_num], &f_args->pydaw_data->track_block_mutexes[f_args->thread_num]);
                
        if(f_args->pydaw_data->track_thread_quit_notifier[f_args->thread_num])
        {            
            f_args->pydaw_data->track_thread_quit_notifier[f_args->thread_num] = 2;
            printf("worker thread %i exiting...\n", f_args->thread_num);
            break;
        }
        
        int f_i = 0;
        while(f_i < f_args->pydaw_data->track_work_queue_counts[f_args->thread_num])
        {
            t_pydaw_work_queue_item f_item = f_args->pydaw_data->track_work_queues[f_args->thread_num][f_i];
            
            if(f_item.track_type == 0)  //MIDI/plugin-instrument
            {
                if(f_args->pydaw_data->track_pool[f_item.track_number]->plugin_index >= 2)
                {
                    v_pydaw_fx_update_ports(f_args->pydaw_data, f_args->pydaw_data->track_pool[f_item.track_number]->instrument, 0,
                        f_item.track_number, 1);
                }
                else
                {
                    v_pydaw_update_ports(f_args->pydaw_data->track_pool[f_item.track_number]->instrument);
                }
                
                v_pydaw_fx_update_ports(f_args->pydaw_data, f_args->pydaw_data->track_pool[f_item.track_number]->effect, 0,
                        f_item.track_number, 0);

                v_run_plugin(f_args->pydaw_data->track_pool[f_item.track_number]->instrument, (f_args->pydaw_data->sample_count), 
                        f_args->pydaw_data->track_pool[f_item.track_number]->event_buffer, 
                        f_args->pydaw_data->track_pool[f_item.track_number]->current_period_event_index);

                v_pydaw_run_pre_effect_vol(f_args->pydaw_data, f_args->pydaw_data->track_pool[f_item.track_number]);
                
                v_run_plugin(f_args->pydaw_data->track_pool[f_item.track_number]->effect, (f_args->pydaw_data->sample_count), 
                        f_args->pydaw_data->track_pool[f_item.track_number]->event_buffer, 
                        f_args->pydaw_data->track_pool[f_item.track_number]->current_period_event_index);

                f_args->pydaw_data->track_pool[f_item.track_number]->current_period_event_index = 0;
                
                v_pydaw_sum_track_outputs(f_args->pydaw_data, f_args->pydaw_data->track_pool[f_item.track_number],
                        f_args->pydaw_data->track_pool[f_item.track_number]->bus_num);
            }
            else if(f_item.track_type == 2)  //Audio track
            {
                int f_i2 = 0;
                
                while(f_i2 < f_args->pydaw_data->sample_count)
                {
                    f_args->pydaw_data->audio_track_pool[f_item.track_number]->effect->pluginInputBuffers[0][f_i2] = 0.0f;
                    f_args->pydaw_data->audio_track_pool[f_item.track_number]->effect->pluginInputBuffers[1][f_i2] = 0.0f;
                    f_i2++;
                }
                
                v_pydaw_fx_update_ports(f_args->pydaw_data, f_args->pydaw_data->audio_track_pool[f_item.track_number]->effect, 2,
                        f_item.track_number, 0);
                
                if((!f_args->pydaw_data->audio_track_pool[f_item.track_number]->mute) &&
                    ((!f_args->pydaw_data->is_soloed) ||
                    ((f_args->pydaw_data->is_soloed) && (f_args->pydaw_data->audio_track_pool[f_item.track_number]->solo))))
                {                    
                    if(v_pydaw_audio_items_run(f_args->pydaw_data, (f_args->pydaw_data->sample_count), 
                            f_args->pydaw_data->audio_track_pool[f_item.track_number]->effect->pluginInputBuffers[0],
                            f_args->pydaw_data->audio_track_pool[f_item.track_number]->effect->pluginInputBuffers[1],
                            f_item.track_number))
                    {
                        v_pydaw_run_pre_effect_vol(f_args->pydaw_data, f_args->pydaw_data->audio_track_pool[f_item.track_number]);
                        v_run_plugin(f_args->pydaw_data->audio_track_pool[f_item.track_number]->effect, (f_args->pydaw_data->sample_count), 
                        f_args->pydaw_data->audio_track_pool[f_item.track_number]->event_buffer, 
                        f_args->pydaw_data->audio_track_pool[f_item.track_number]->current_period_event_index);
                
                        v_pydaw_sum_track_outputs(f_args->pydaw_data, f_args->pydaw_data->audio_track_pool[f_item.track_number],
                                f_args->pydaw_data->audio_track_pool[f_item.track_number]->bus_num);
                    }
                    else
                    {
                        f_args->pydaw_data->bus_pool[(f_args->pydaw_data->audio_track_pool[f_item.track_number]->bus_num)]->bus_counter = 
                                (f_args->pydaw_data->bus_pool[(f_args->pydaw_data->audio_track_pool[f_item.track_number]->bus_num)]->bus_counter) - 1;
                    }
                }
                else
                {
                    f_args->pydaw_data->bus_pool[(f_args->pydaw_data->audio_track_pool[f_item.track_number]->bus_num)]->bus_counter = 
                            (f_args->pydaw_data->bus_pool[(f_args->pydaw_data->audio_track_pool[f_item.track_number]->bus_num)]->bus_counter) - 1;
                }
            }
            else if(f_item.track_type == 1)  //Bus track
            {
                //Not necessary because it's assigned by the first plugin that writes to it...
                /*
                int f_i2 = 0;                
                while(f_i2 < f_args->pydaw_data->sample_count)
                {
                    f_args->pydaw_data->bus_pool[f_item.track_number]->effect->pluginInputBuffers[0][f_i2] = 0.0f;
                    f_args->pydaw_data->bus_pool[f_item.track_number]->effect->pluginInputBuffers[1][f_i2] = 0.0f;
                    f_i2++;
                }
                */

                if((f_args->pydaw_data->bus_pool[f_item.track_number]->bus_count) == 0)
                {
                    f_args->pydaw_data->bus_pool[0]->bus_counter = (f_args->pydaw_data->bus_pool[0]->bus_counter) - 1;
                }
                else
                {   
                    while(1)
                    {
                        pthread_spin_lock(&f_args->pydaw_data->bus_spinlocks[f_item.track_number]);
                        if((f_args->pydaw_data->bus_pool[f_item.track_number]->bus_counter) <= 0)
                        {
                            break;
                        }
                        pthread_spin_unlock(&f_args->pydaw_data->bus_spinlocks[f_item.track_number]);
                    }
                    
                    v_pydaw_fx_update_ports(f_args->pydaw_data, f_args->pydaw_data->bus_pool[f_item.track_number]->effect, 1, 
                            f_item.track_number, 0);

                    v_pydaw_run_pre_effect_vol(f_args->pydaw_data, f_args->pydaw_data->bus_pool[f_item.track_number]);
                    
                    v_run_plugin(f_args->pydaw_data->bus_pool[f_item.track_number]->effect, (f_args->pydaw_data->sample_count), 
                            f_args->pydaw_data->bus_pool[f_item.track_number]->event_buffer, 
                            f_args->pydaw_data->bus_pool[f_item.track_number]->current_period_event_index);

                    v_pydaw_sum_track_outputs(f_args->pydaw_data, f_args->pydaw_data->bus_pool[f_item.track_number],
                            0);
                }                
                
                f_args->pydaw_data->bus_pool[f_item.track_number]->bus_buffer_state = 0;
                f_args->pydaw_data->bus_pool[f_item.track_number]->bus_counter = (f_args->pydaw_data->bus_pool[f_item.track_number]->bus_count);
                pthread_spin_unlock(&f_args->pydaw_data->bus_spinlocks[f_item.track_number]);                
            }
            
            f_i++;
        }
        
        f_args->pydaw_data->track_thread_is_finished[f_args->thread_num] = 1;        
    }
    
    return (void*)1;
}

t_py_cc_map_item * g_py_cc_map_item_get(int a_effects_only, int a_rayv_port, int a_wayv_port, int a_euphoria_port, int a_modulex_port)
{
    t_py_cc_map_item * f_result = (t_py_cc_map_item*)malloc(sizeof(t_py_cc_map_item));
    f_result->effects_only = a_effects_only;
    
    if(a_effects_only)
    {
        f_result->modulex_port = a_modulex_port;
        f_result->euphoria_port = 0;
        f_result->rayv_port = 0;
        f_result->wayv_port = 0;
    }
    else
    {
        f_result->modulex_port = 0;
        f_result->euphoria_port = a_euphoria_port;
        f_result->rayv_port = a_rayv_port;
        f_result->wayv_port = a_wayv_port;
    }
    
    return f_result;
}

void v_pydaw_load_cc_map(t_pydaw_data * a_pydaw_data, const char * a_name)
{
    int f_i = 0;
    while(f_i < PYDAW_MIDI_NOTE_COUNT)
    {
        if(a_pydaw_data->cc_map[f_i])
        {
            free(a_pydaw_data->cc_map[f_i]);
            a_pydaw_data->cc_map[f_i] = 0;
        }
        f_i++;
    }
    char f_temp[256];
    char * f_home = getenv("HOME");
    sprintf(f_temp, "%s/pydaw3/cc_maps/%s", f_home, a_name);
    if(i_pydaw_file_exists(f_temp))
    {
        t_2d_char_array * f_current_string = g_get_2d_array_from_file(f_temp, LMS_LARGE_STRING);        
        f_i = 0;
        while(f_i < PYDAW_MIDI_NOTE_COUNT)
        {            
            char * f_cc_char = c_iterate_2d_char_array(f_current_string);
            if(f_current_string->eof)
            {
                free(f_cc_char);
                break;
            }
            int f_cc = atoi(f_cc_char);
            free(f_cc_char);
            char * f_effects_only_char = c_iterate_2d_char_array(f_current_string);
            int f_effects_only = atoi(f_effects_only_char);
            free(f_effects_only_char);
            char * f_rayv_port_char = c_iterate_2d_char_array(f_current_string);
            int f_rayv_port = atoi(f_rayv_port_char);
            free(f_rayv_port_char);
            char * f_wayv_port_char = c_iterate_2d_char_array(f_current_string);
            int f_wayv_port = atoi(f_wayv_port_char);
            free(f_wayv_port_char);
            char * f_euphoria_port_char = c_iterate_2d_char_array(f_current_string);
            int f_euphoria_port = atoi(f_euphoria_port_char);
            free(f_euphoria_port_char);
            char * f_modulex_port_char = c_iterate_2d_char_array(f_current_string);
            int f_modulex_port = atoi(f_modulex_port_char);
            free(f_modulex_port_char);
            a_pydaw_data->cc_map[f_cc] = g_py_cc_map_item_get(f_effects_only, f_rayv_port, f_wayv_port, f_euphoria_port, f_modulex_port);
            f_i++;
        }

        g_free_2d_char_array(f_current_string);
        
    }
}

inline void v_pydaw_process_external_midi(t_pydaw_data * a_pydaw_data, int sample_count, snd_seq_event_t *events, int event_count)
{
    if((a_pydaw_data->record_armed_track) && ((a_pydaw_data->record_armed_track->plugin_index) != 0))
    {
        int f_i2 = 0;

        if(a_pydaw_data->playback_mode == 0)
        {
            a_pydaw_data->record_armed_track->current_period_event_index = 0;
        }
        
        if(a_pydaw_data->playback_mode == PYDAW_PLAYBACK_MODE_REC)
        {
            if(!a_pydaw_data->recording_in_current_bar)
            {
                a_pydaw_data->recording_in_current_bar = 1;
                if(!a_pydaw_data->pysong->regions[(a_pydaw_data->current_region)])
                {
                    a_pydaw_data->pysong->regions[(a_pydaw_data->current_region)] = g_pyregion_get_new(a_pydaw_data);                        
                }

                if(a_pydaw_data->overdub_mode)
                {
                    int f_item_index = a_pydaw_data->pysong->regions[(a_pydaw_data->current_region)]->item_indexes[a_pydaw_data->record_armed_track_index_all][a_pydaw_data->current_bar];
                    if(f_item_index == -1)
                    {
                        a_pydaw_data->recording_current_item_pool_index = g_pyitem_get_new(a_pydaw_data);
                        a_pydaw_data->pysong->regions[(a_pydaw_data->current_region)]->item_indexes[a_pydaw_data->record_armed_track_index_all][a_pydaw_data->current_bar] =
                                a_pydaw_data->item_pool[(a_pydaw_data->recording_current_item_pool_index)]->uid;                            
                    }
                    else
                    {
                        a_pydaw_data->recording_current_item_pool_index = g_pyitem_clone(a_pydaw_data, f_item_index);
                        a_pydaw_data->pysong->regions[(a_pydaw_data->current_region)]->item_indexes[a_pydaw_data->record_armed_track_index_all][a_pydaw_data->current_bar] =
                                a_pydaw_data->item_pool[(a_pydaw_data->recording_current_item_pool_index)]->uid;                            
                    }                        
                }
                else
                {
                    a_pydaw_data->recording_current_item_pool_index = g_pyitem_get_new(a_pydaw_data);
                    a_pydaw_data->pysong->regions[(a_pydaw_data->current_region)]->item_indexes[a_pydaw_data->record_armed_track_index_all][a_pydaw_data->current_bar] =
                            a_pydaw_data->item_pool[(a_pydaw_data->recording_current_item_pool_index)]->uid;                        
                }
                if((a_pydaw_data->recording_first_item) == -1)
                {
                    a_pydaw_data->recording_first_item = (a_pydaw_data->recording_current_item_pool_index);
                }
                a_pydaw_data->pysong->regions[(a_pydaw_data->current_region)]->item_indexes[a_pydaw_data->record_armed_track_index_all][a_pydaw_data->current_bar] = (a_pydaw_data->recording_current_item_pool_index);
                a_pydaw_data->pysong->regions[(a_pydaw_data->current_region)]->not_yet_saved = 1;
            }
        }

        while(f_i2 < event_count)
        {
            if(events[f_i2].type == SND_SEQ_EVENT_NOTEON)
            {
                snd_seq_ev_note_t n = events[f_i2].data.note;
                snd_seq_ev_clear(&a_pydaw_data->record_armed_track->event_buffer[(a_pydaw_data->record_armed_track->current_period_event_index)]);
                snd_seq_ev_set_noteon(&a_pydaw_data->record_armed_track->event_buffer[(a_pydaw_data->record_armed_track->current_period_event_index)], 0,
                            n.note, n.velocity);
                a_pydaw_data->record_armed_track->event_buffer[(a_pydaw_data->record_armed_track->current_period_event_index)].time.tick = 
                        (events[f_i2].time.tick);
                a_pydaw_data->record_armed_track->current_period_event_index = (a_pydaw_data->record_armed_track->current_period_event_index) + 1;

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
                snd_seq_ev_clear(&a_pydaw_data->record_armed_track->event_buffer[(a_pydaw_data->record_armed_track->current_period_event_index)]);
                snd_seq_ev_set_noteoff(&a_pydaw_data->record_armed_track->event_buffer[(a_pydaw_data->record_armed_track->current_period_event_index)], 0, n.note, 0);
                a_pydaw_data->record_armed_track->event_buffer[(a_pydaw_data->record_armed_track->current_period_event_index)].time.tick = 
                        (events[f_i2].time.tick);

                a_pydaw_data->record_armed_track->current_period_event_index = (a_pydaw_data->record_armed_track->current_period_event_index) + 1;

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
                snd_seq_ev_clear(&a_pydaw_data->record_armed_track->event_buffer[(a_pydaw_data->record_armed_track->current_period_event_index)]);
                snd_seq_ev_set_pitchbend(&a_pydaw_data->record_armed_track->event_buffer[(a_pydaw_data->record_armed_track->current_period_event_index)], 
                        0, events[f_i2].data.control.value);
                a_pydaw_data->record_armed_track->current_period_event_index = (a_pydaw_data->record_armed_track->current_period_event_index) + 1;

                if(a_pydaw_data->playback_mode == PYDAW_PLAYBACK_MODE_REC)
                {
                    int f_index = (a_pydaw_data->recording_current_item_pool_index);
                    double f_start =
                            ((a_pydaw_data->playback_cursor) + ((((double)(events[f_i2].time.tick))/((double)sample_count)) 
                            * (a_pydaw_data->playback_inc))) * 4.0f;
                    a_pydaw_data->item_pool[f_index]->pitchbends[(a_pydaw_data->item_pool[f_index]->pitchbend_count)] =
                            g_pypitchbend_get(f_start, (float)events[f_i2].data.control.value);
                    a_pydaw_data->item_pool[f_index]->pitchbend_count = (a_pydaw_data->item_pool[f_index]->pitchbend_count) + 1;
                }
            }
            else if(events[f_i2].type == SND_SEQ_EVENT_CONTROLLER)
            {
                int controller = events[f_i2].data.control.param;
#ifdef PYDAW_PRINT_DEBUG_INFO
                printf("\n\nALSA MIDI CC event:  c.param == %i, c.value == %i\n\n", events[f_i2].data.control.param, events[f_i2].data.control.value);
#endif
                if(a_pydaw_data->cc_map[controller])
                {
                    int f_index = (a_pydaw_data->recording_current_item_pool_index);
                    double f_start =
                            ((a_pydaw_data->playback_cursor) + ((((double)(events[f_i2].time.tick))/((double)sample_count)) 
                            * (a_pydaw_data->playback_inc))) * 4.0f;
                    
                    int controlIn;
                    if(a_pydaw_data->record_armed_track->instrument)
                    {
                        int f_port; 
                        if(a_pydaw_data->record_armed_track->plugin_index == 1)  //Euphoria
                        {
                            f_port = a_pydaw_data->cc_map[controller]->euphoria_port;
                        }
                        else if(a_pydaw_data->record_armed_track->plugin_index == 2) //Ray-V
                        {
                            f_port = a_pydaw_data->cc_map[controller]->rayv_port;
                        }
                        else if(a_pydaw_data->record_armed_track->plugin_index == 3) //Way-V
                        {
                            f_port = a_pydaw_data->cc_map[controller]->wayv_port;
                        }
                        
                        controlIn = a_pydaw_data->record_armed_track->instrument->pluginPortControlInNumbers[f_port];

                        if (controlIn > 0)  //if (controlIn >= 0) 
                        {
                            /* controller is mapped to LADSPA port, update the port */
                            v_pydaw_set_control_from_cc(a_pydaw_data->record_armed_track->instrument, controlIn, &events[f_i2], 0);
                            
                            if(a_pydaw_data->playback_mode == PYDAW_PLAYBACK_MODE_REC)
                            {   
                                a_pydaw_data->item_pool[f_index]->ccs[(a_pydaw_data->item_pool[f_index]->cc_count)] =
                                        g_pycc_get(a_pydaw_data->record_armed_track->plugin_index, f_port, 
                                        (float)events[f_i2].data.control.value, f_start);
                                a_pydaw_data->item_pool[f_index]->cc_count = (a_pydaw_data->item_pool[f_index]->cc_count) + 1;
                            }
                        }
                    }
                    
                    controlIn = a_pydaw_data->record_armed_track->effect->pluginPortControlInNumbers[a_pydaw_data->cc_map[controller]->modulex_port];
                    
                    if (controlIn > 0) //if (controlIn >= 0) 
                    {
                        /* controller is mapped to LADSPA port, update the port */
                        v_pydaw_set_control_from_cc(a_pydaw_data->record_armed_track->effect, controlIn, &events[f_i2], 0);
                        
                        if(a_pydaw_data->playback_mode == PYDAW_PLAYBACK_MODE_REC)
                        {   
                            a_pydaw_data->item_pool[f_index]->ccs[(a_pydaw_data->item_pool[f_index]->cc_count)] =
                                    g_pycc_get(a_pydaw_data->record_armed_track->plugin_index, controlIn, 
                                    (float)events[f_i2].data.control.value, f_start);
                            a_pydaw_data->item_pool[f_index]->cc_count = (a_pydaw_data->item_pool[f_index]->cc_count) + 1;
                        }
                    }
                }
            }
            f_i2++;
        }
        v_pydaw_update_ports(a_pydaw_data->record_armed_track->effect);  //Ensure that the plugin GUI is updated for audio/bus tracks        
    }     
}

inline void v_pydaw_schedule_work(t_pydaw_data * a_pydaw_data)
{
    int f_i = 0;
    
    v_pydaw_set_bus_counters(a_pydaw_data);
        
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
            //printf("Schedule Euphoria instance at %i on %i\n", f_i, f_thread_index);
            
            a_pydaw_data->track_work_queues[f_thread_index][a_pydaw_data->track_work_queue_counts[f_thread_index]].track_number = f_i;
            a_pydaw_data->track_work_queues[f_thread_index][a_pydaw_data->track_work_queue_counts[f_thread_index]].track_type = 0;
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
            //printf("Schedule Way-V instance at %i on %i\n", f_i, f_thread_index);
            
            a_pydaw_data->track_work_queues[f_thread_index][a_pydaw_data->track_work_queue_counts[f_thread_index]].track_number = f_i;
            a_pydaw_data->track_work_queues[f_thread_index][a_pydaw_data->track_work_queue_counts[f_thread_index]].track_type = 0;
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
            //printf("Schedule Ray-V instance at %i on %i\n", f_i, f_thread_index);
            
            a_pydaw_data->track_work_queues[f_thread_index][a_pydaw_data->track_work_queue_counts[f_thread_index]].track_number = f_i;
            a_pydaw_data->track_work_queues[f_thread_index][a_pydaw_data->track_work_queue_counts[f_thread_index]].track_type = 0;
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
    /*Now schedule the audio tracks*/    
    while(f_i < PYDAW_AUDIO_TRACK_COUNT)
    {
        //printf("Schedule audio track instance at %i on %i\n", f_i, f_thread_index);
        
        a_pydaw_data->track_work_queues[f_thread_index][a_pydaw_data->track_work_queue_counts[f_thread_index]].track_number = f_i;
        a_pydaw_data->track_work_queues[f_thread_index][a_pydaw_data->track_work_queue_counts[f_thread_index]].track_type = 2;
        a_pydaw_data->track_work_queue_counts[f_thread_index] = (a_pydaw_data->track_work_queue_counts[f_thread_index]) + 1;
        f_thread_index++;
        
        if(f_thread_index >= a_pydaw_data->track_worker_thread_count)
        {
            f_thread_index = 0;
        }
                
        f_i++;
    }

    f_i = 1;
    /*Schedule bus tracks last, because they must be processed last*/
    while(f_i < PYDAW_BUS_TRACK_COUNT)
    {
        if(a_pydaw_data->bus_pool[f_i]->bus_count > 0)
        {
            //printf("Schedule bus track instance at %i on %i\n", f_i, f_thread_index);
            
            a_pydaw_data->track_work_queues[f_thread_index][a_pydaw_data->track_work_queue_counts[f_thread_index]].track_number = f_i;
            a_pydaw_data->track_work_queues[f_thread_index][a_pydaw_data->track_work_queue_counts[f_thread_index]].track_type = 1;
            a_pydaw_data->track_work_queue_counts[f_thread_index] = (a_pydaw_data->track_work_queue_counts[f_thread_index]) + 1;     
            f_thread_index++;

            if(f_thread_index >= a_pydaw_data->track_worker_thread_count)
            {
                f_thread_index = 0;
            }
        }

        f_i++;
    }    
}

inline void v_pydaw_run_main_loop(t_pydaw_data * a_pydaw_data, int sample_count, 
        snd_seq_event_t *events, int event_count, long f_next_current_sample, 
        PYFX_Data *output0, PYFX_Data *output1, PYFX_Data **a_input_buffers)
{
    a_pydaw_data->sample_count = sample_count;
    a_pydaw_data->input_buffers = a_input_buffers;
    
    if((a_pydaw_data->playback_mode) > 0)
    {
        a_pydaw_data->ml_sample_period_inc = ((a_pydaw_data->playback_inc) * ((double)(sample_count)));
        a_pydaw_data->ml_sample_period_inc_beats = (a_pydaw_data->ml_sample_period_inc) * 4.0f;
        a_pydaw_data->ml_next_playback_cursor = (a_pydaw_data->playback_cursor) + (a_pydaw_data->ml_sample_period_inc);
        a_pydaw_data->ml_current_period_beats = (a_pydaw_data->playback_cursor) * 4.0f;
        a_pydaw_data->ml_next_period_beats = (a_pydaw_data->ml_next_playback_cursor) * 4.0f;
        
        a_pydaw_data->ml_current_region = (a_pydaw_data->current_region);
        a_pydaw_data->ml_current_bar = (a_pydaw_data->current_bar);
        a_pydaw_data->ml_current_beat = (a_pydaw_data->ml_current_period_beats);
        
        a_pydaw_data->ml_next_bar = (a_pydaw_data->current_bar);
        a_pydaw_data->ml_next_region = (a_pydaw_data->current_region);
                        
        if((a_pydaw_data->ml_next_period_beats) > 4.0f)  //Should it be >= ???
        {
            a_pydaw_data->ml_starting_new_bar = 1;
            a_pydaw_data->ml_next_beat = (a_pydaw_data->ml_next_period_beats) - 4.0f;            
            
            if(a_pydaw_data->loop_mode != PYDAW_LOOP_MODE_BAR)
            {
                a_pydaw_data->ml_next_bar = (a_pydaw_data->current_bar) + 1;
                
                int f_region_length = 8;
                if(a_pydaw_data->pysong->regions[(a_pydaw_data->current_region)])
                {
                    f_region_length = (a_pydaw_data->pysong->regions[(a_pydaw_data->current_region)]->region_length_bars);
                }

                if(f_region_length == 0)
                {
                    f_region_length = 8;
                }

                if((a_pydaw_data->ml_next_bar) >= f_region_length)
                {
                    a_pydaw_data->ml_next_bar = 0;
                    if(a_pydaw_data->loop_mode != PYDAW_LOOP_MODE_REGION)
                    {
                        a_pydaw_data->ml_next_region = (a_pydaw_data->ml_next_region) + 1;
                    }
                    else
                    {
                        a_pydaw_data->ml_is_looping = 1;
                    }
                }                
            }
            else
            {
                a_pydaw_data->ml_is_looping = 1;
            }
        }
        else
        {
            a_pydaw_data->ml_is_looping = 0;
            a_pydaw_data->ml_starting_new_bar = 0;
            a_pydaw_data->ml_next_region = a_pydaw_data->current_region;
            a_pydaw_data->ml_next_bar = a_pydaw_data->current_bar;
            a_pydaw_data->ml_next_beat = a_pydaw_data->ml_next_period_beats;
        }
        

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
        
        //Calculate track notes for this period and send them to instruments
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
            double f_track_current_period_beats = (a_pydaw_data->ml_current_period_beats);
            double f_track_next_period_beats = (a_pydaw_data->ml_next_period_beats);
            double f_track_beats_offset = 0.0f;

            if((!a_pydaw_data->overdub_mode) && (a_pydaw_data->playback_mode == 2) && (a_pydaw_data->record_armed_track_index_all == f_i))
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
                                f_track_beats_offset = ((a_pydaw_data->ml_sample_period_inc) * 4.0f) - f_track_next_period_beats;

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
                            float f_note_start_frac = f_note_start_diff / (a_pydaw_data->ml_sample_period_inc_beats);
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
                                    f_i, f_note_start_diff, ml_sample_period_inc_beats, f_note_start_frac, f_note_sample_offset, (int)sample_count, a_pydaw_data->current_sample);
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
                            f_track_beats_offset = ((a_pydaw_data->ml_sample_period_inc) * 4.0f) - f_track_next_period_beats;

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
        while(f_i < PYDAW_TRACK_COUNT_ALL)
        {   
            /* Situations where the track is effectively muted*/
            if((a_pydaw_data->track_pool_all[f_i]->plugin_index == 0) ||
                (a_pydaw_data->track_pool_all[f_i]->mute) ||
                ((a_pydaw_data->is_soloed) && (!a_pydaw_data->track_pool_all[f_i]->solo)))
            {
                f_i++;
                continue;
            }

            int f_current_track_region = a_pydaw_data->current_region;
            int f_current_track_bar = a_pydaw_data->current_bar;
            double f_track_current_period_beats = (a_pydaw_data->ml_current_period_beats);
            double f_track_next_period_beats = (a_pydaw_data->ml_next_period_beats);
            //double f_track_beats_offset = 0.0f;

            if((!a_pydaw_data->overdub_mode) && (a_pydaw_data->playback_mode == PYDAW_PLAYBACK_MODE_REC) && (a_pydaw_data->record_armed_track_index_all == f_i))
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
                            int controller = f_current_item.ccs[(a_pydaw_data->track_current_item_cc_event_indexes[f_i])]->port;
                            if (controller > 0) //&& controller < MIDI_CONTROLLER_COUNT) 
                            {
                                int controlIn; // = f_current_item.ccs[(a_pydaw_data->track_current_item_cc_event_indexes[f_i])]->port;
                                if(a_pydaw_data->track_pool_all[f_i]->instrument && 
                                        a_pydaw_data->track_pool_all[f_i]->plugin_index == 
                                        f_current_item.ccs[(a_pydaw_data->track_current_item_cc_event_indexes[f_i])]->plugin_index)
                                {
                                    controlIn = a_pydaw_data->track_pool_all[f_i]->instrument->pluginPortControlInNumbers[controller];
                                    if (controlIn >= 0)
                                    {
                                        snd_seq_event_t f_event;
                                        f_event.data.control.value = f_current_item.ccs[(a_pydaw_data->track_current_item_cc_event_indexes[f_i])]->cc_val;
                                        v_pydaw_set_control_from_cc(a_pydaw_data->track_pool_all[f_i]->instrument, controlIn, &f_event, 0);
                                    }
                                }
                                else if(f_current_item.ccs[(a_pydaw_data->track_current_item_cc_event_indexes[f_i])]->plugin_index == -1)
                                {
                                    controlIn = a_pydaw_data->track_pool_all[f_i]->effect->pluginPortControlInNumbers[controller];
                                    if (controlIn >= 0)
                                    {
                                        snd_seq_event_t f_event;
                                        f_event.data.control.value = f_current_item.ccs[(a_pydaw_data->track_current_item_cc_event_indexes[f_i])]->cc_val;
                                        v_pydaw_set_control_from_cc(a_pydaw_data->track_pool_all[f_i]->effect, controlIn, &f_event, 0);
                                    }
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
            double f_track_current_period_beats = (a_pydaw_data->ml_current_period_beats);
            double f_track_next_period_beats = (a_pydaw_data->ml_next_period_beats);
            //double f_track_beats_offset = 0.0f;

            if((a_pydaw_data->playback_mode == 2) && (a_pydaw_data->record_armed_track_index_all == f_i))
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

        a_pydaw_data->playback_cursor = (a_pydaw_data->ml_next_playback_cursor);

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

                     
    f_i = 0;

    while(f_i < (a_pydaw_data->track_worker_thread_count))
    {
        a_pydaw_data->track_thread_is_finished[f_i] = 0;
        f_i++;
    }
    //notify the worker threads
    f_i = 0;
    while(f_i < a_pydaw_data->track_worker_thread_count)
    {
        pthread_mutex_lock(&a_pydaw_data->track_block_mutexes[f_i]);
        pthread_cond_broadcast(&a_pydaw_data->track_cond[f_i]);
        pthread_mutex_unlock(&a_pydaw_data->track_block_mutexes[f_i]);
        f_i++;
    }
    
    v_pydaw_fx_update_ports(a_pydaw_data, a_pydaw_data->bus_pool[0]->effect, 1, 0, 0);
    
    f_i = 0;
    //A ghetto pthread_join for threads that never finish...
    while(f_i < (a_pydaw_data->track_worker_thread_count))
    {        
        if(a_pydaw_data->track_thread_is_finished[f_i] == 0)
        {            
            continue;  //spin until it is finished...
        }
        
        f_i++;
    }
    
    //Run the master channels effects
    v_run_plugin(a_pydaw_data->bus_pool[0]->effect, sample_count, a_pydaw_data->bus_pool[0]->event_buffer, a_pydaw_data->bus_pool[0]->current_period_event_index);
    
    int f_i2 = 0;

    while(f_i2 < sample_count)
    {
        output0[f_i2] = (a_pydaw_data->bus_pool[0]->effect->pluginOutputBuffers[0][f_i2]) * (a_pydaw_data->bus_pool[0]->volume_linear);
        output1[f_i2] = (a_pydaw_data->bus_pool[0]->effect->pluginOutputBuffers[1][f_i2]) * (a_pydaw_data->bus_pool[0]->volume_linear);
        a_pydaw_data->bus_pool[0]->effect->pluginOutputBuffers[0][f_i2] = 0.0f;
        a_pydaw_data->bus_pool[0]->effect->pluginOutputBuffers[1][f_i2] = 0.0f;
        f_i2++;
    }
    
    a_pydaw_data->bus_pool[0]->bus_buffer_state = 0;
    a_pydaw_data->bus_pool[0]->bus_counter = (a_pydaw_data->bus_pool[0]->bus_count);
    
    if(a_pydaw_data->ml_is_looping)
    {        
        v_pydaw_reset_audio_item_read_heads(a_pydaw_data, a_pydaw_data->ml_next_region, a_pydaw_data->ml_next_bar);
    }
    
    //We run everything else as normal to prevent hung notes and other oddities, even though the buffer is overwritten...
    if(a_pydaw_data->is_ab_ing)
    {
        f_i = 0;
        while(f_i < sample_count)
        {            
            if((a_pydaw_data->ab_audio_item->sample_read_head->whole_number) >=  (a_pydaw_data->ab_wav_item->length))
            {
                output0[f_i] = 0.0f;
                output1[f_i] = 0.0f;                
            }
            else
            {
                v_adsr_run_db(a_pydaw_data->ab_audio_item->adsr);
                if(a_pydaw_data->ab_wav_item->channels == 1)
                {
                    float f_tmp_sample = f_cubic_interpolate_ptr_ifh(
                    (a_pydaw_data->ab_wav_item->samples[0]),
                    (a_pydaw_data->ab_audio_item->sample_read_head->whole_number),
                    (a_pydaw_data->ab_audio_item->sample_read_head->fraction),
                    (a_pydaw_data->cubic_interpolator)) * 
                    (a_pydaw_data->ab_audio_item->adsr->output) *
                    (a_pydaw_data->ab_amp_lin); // * 
                    //(a_pydaw_data->ab_audio_item->fade_vol);

                    output0[f_i] = f_tmp_sample;
                    output1[f_i] = f_tmp_sample;
                }
                else if(a_pydaw_data->ab_wav_item->channels > 1)
                {
                    output0[f_i] = f_cubic_interpolate_ptr_ifh(
                    (a_pydaw_data->ab_wav_item->samples[0]),
                    (a_pydaw_data->ab_audio_item->sample_read_head->whole_number),
                    (a_pydaw_data->ab_audio_item->sample_read_head->fraction),
                    (a_pydaw_data->cubic_interpolator)) * 
                    (a_pydaw_data->ab_audio_item->adsr->output) *
                    (a_pydaw_data->ab_amp_lin); // * 
                    //(a_pydaw_data->ab_audio_item->fade_vol);

                    output1[f_i] = f_cubic_interpolate_ptr_ifh(
                    (a_pydaw_data->ab_wav_item->samples[1]),
                    (a_pydaw_data->ab_audio_item->sample_read_head->whole_number),
                    (a_pydaw_data->ab_audio_item->sample_read_head->fraction),
                    (a_pydaw_data->cubic_interpolator)) * 
                    (a_pydaw_data->ab_audio_item->adsr->output) *
                    (a_pydaw_data->ab_amp_lin); // * 
                    //(a_pydaw_data->ab_audio_item->fade_vol);
                }

                v_ifh_run(a_pydaw_data->ab_audio_item->sample_read_head, a_pydaw_data->ab_audio_item->ratio);

                if(a_pydaw_data->playback_mode != PYDAW_PLAYBACK_MODE_PLAY && 
                        a_pydaw_data->ab_audio_item->adsr->stage < 3)
                {
                    v_adsr_release(a_pydaw_data->ab_audio_item->adsr);
                }
                else if(a_pydaw_data->playback_mode == PYDAW_PLAYBACK_MODE_OFF && 
                        a_pydaw_data->ab_audio_item->adsr->stage == 4)
                {
                    a_pydaw_data->is_ab_ing = 0;
                }                                
            }            
            f_i++;
        }
    }
    
    if(a_pydaw_data->is_previewing)
    {
        f_i = 0;
        while(f_i < sample_count)
        {            
            if((a_pydaw_data->preview_audio_item->sample_read_head->whole_number) >=  (a_pydaw_data->preview_wav_item->length))
            {
                a_pydaw_data->is_previewing = 0;
                break;
            }
            else
            {
                v_adsr_run_db(a_pydaw_data->preview_audio_item->adsr);
                if(a_pydaw_data->preview_wav_item->channels == 1)
                {
                    float f_tmp_sample = f_cubic_interpolate_ptr_ifh(
                    (a_pydaw_data->preview_wav_item->samples[0]),
                    (a_pydaw_data->preview_audio_item->sample_read_head->whole_number),
                    (a_pydaw_data->preview_audio_item->sample_read_head->fraction),
                    (a_pydaw_data->cubic_interpolator)) * 
                    (a_pydaw_data->preview_audio_item->adsr->output) *
                    (a_pydaw_data->preview_amp_lin); // * 
                    //(a_pydaw_data->preview_audio_item->fade_vol);

                    output0[f_i] = f_tmp_sample;
                    output1[f_i] = f_tmp_sample;
                }
                else if(a_pydaw_data->preview_wav_item->channels > 1)
                {
                    output0[f_i] = f_cubic_interpolate_ptr_ifh(
                    (a_pydaw_data->preview_wav_item->samples[0]),
                    (a_pydaw_data->preview_audio_item->sample_read_head->whole_number),
                    (a_pydaw_data->preview_audio_item->sample_read_head->fraction),
                    (a_pydaw_data->cubic_interpolator)) * 
                    (a_pydaw_data->preview_audio_item->adsr->output) *
                    (a_pydaw_data->preview_amp_lin); // * 
                    //(a_pydaw_data->preview_audio_item->fade_vol);

                    output1[f_i] = f_cubic_interpolate_ptr_ifh(
                    (a_pydaw_data->preview_wav_item->samples[1]),
                    (a_pydaw_data->preview_audio_item->sample_read_head->whole_number),
                    (a_pydaw_data->preview_audio_item->sample_read_head->fraction),
                    (a_pydaw_data->cubic_interpolator)) * 
                    (a_pydaw_data->preview_audio_item->adsr->output) *
                    (a_pydaw_data->preview_amp_lin); // * 
                    //(a_pydaw_data->preview_audio_item->fade_vol);
                }

                v_ifh_run(a_pydaw_data->preview_audio_item->sample_read_head, a_pydaw_data->preview_audio_item->ratio);

                if((a_pydaw_data->preview_audio_item->sample_read_head->whole_number) 
                        >=  (a_pydaw_data->preview_max_sample_count))
                {
                    v_adsr_release(a_pydaw_data->preview_audio_item->adsr);
                }
                else if(a_pydaw_data->preview_audio_item->adsr->stage == 4)
                {
                    a_pydaw_data->is_previewing = 0;
                    break;
                }                                
            }            
            f_i++;
        }
    }
}



inline int v_pydaw_audio_items_run(t_pydaw_data * a_pydaw_data, int a_sample_count, float* a_output0, 
        float* a_output1, int a_audio_track_num)
{    
    /*
    if((a_pydaw_data->playback_mode) == PYDAW_PLAYBACK_MODE_OFF)
    {
        return;
    }
    */
    
    if(!a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region])
    {
        return 0;  //TODO:  Check if next period has one -- TODO:  Return zero or one?
    }
    
    int f_return_value = 0;
    
    int f_i = 0;
    int f_i2 = 0;
    
    double f_adjusted_song_pos_beats = v_pydaw_count_beats(a_pydaw_data, 0, 0, 0.0f, a_pydaw_data->ml_current_region, a_pydaw_data->ml_current_bar, 
                a_pydaw_data->ml_current_beat);
    double f_adjusted_next_song_pos_beats = v_pydaw_count_beats(a_pydaw_data, 0, 0, 0.0f, a_pydaw_data->ml_next_region, a_pydaw_data->ml_next_bar, 
                a_pydaw_data->ml_next_beat);
        
    while(f_i < PYDAW_MAX_AUDIO_ITEM_COUNT)
    {
        if((a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]) == 0)
        {
            f_i++;
            continue;
        }
        
        if(a_pydaw_data->suppress_new_audio_items && 
            ((a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->adsr->stage) == 4))
        {
            f_i++;
            continue;
        }
        
        if((a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->audio_track_output) == a_audio_track_num)
        {              
            f_return_value = 1;
            
            a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->adjusted_start_beat = 
                    v_pydaw_count_beats(a_pydaw_data, 0, 0, 0.0f,
                    a_pydaw_data->current_region,
                    //a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->start_region, 
                    a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->start_bar, 
                    a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->start_beat);
            
            if((a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->adjusted_start_beat) >= 
                    f_adjusted_next_song_pos_beats)
            {
                f_i++;
                continue;
            }                        
            
            if(a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->end_mode == 1)            
            {
                a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->adjusted_end_beat = 
                        v_pydaw_count_beats(a_pydaw_data, 0, 0, 0.0f,
                        a_pydaw_data->current_region,                    
                        a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->end_bar, 
                    a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->end_beat);
                
                if((a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->adjusted_end_beat) 
                        < (f_adjusted_song_pos_beats))
                {
                    f_i++;
                    continue;
                }
            }            

            int f_adjusted_sample_count = a_sample_count;
            f_i2 = 0;            
            
            if(((a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->adjusted_start_beat) >= f_adjusted_song_pos_beats) &&
                ((a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->adjusted_start_beat) < f_adjusted_next_song_pos_beats))
            {
                double test1 = (a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->adjusted_start_beat) - f_adjusted_song_pos_beats;
                double test2 = f_adjusted_next_song_pos_beats - f_adjusted_song_pos_beats;
                double test3 = (test1 / test2) * ((double)(a_sample_count));
                f_i2 = (int)test3;
                v_ifh_retrigger(a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->sample_read_head, 
                        a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->sample_start_offset); 
                v_adsr_retrigger(a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->adsr);
            }
            
            if((a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->end_mode == 1) &&
                    ((a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->adjusted_end_beat) < f_adjusted_next_song_pos_beats))
            {
                double test1 = (a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->adjusted_start_beat) - f_adjusted_song_pos_beats;
                double test2 = f_adjusted_next_song_pos_beats - f_adjusted_song_pos_beats;
                double test3 = (test1 / test2) * ((double)(a_sample_count));
                
                f_adjusted_sample_count = (int)test3;
            }
                                    
            if(a_pydaw_data->ml_is_looping)
            {
                double test1 = f_adjusted_next_song_pos_beats - 4.0f;
                double test2 = f_adjusted_next_song_pos_beats - f_adjusted_song_pos_beats;
                double test3 = (test1 / test2) * ((double)(a_sample_count));

                f_adjusted_sample_count = (int)test3;
            }
            
            if((a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->adsr->stage) != 4)
            {
                while(1)
                {
                    while((f_i2 < f_adjusted_sample_count) && 
                    ((a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->sample_read_head->whole_number) <  (a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->sample_end_offset)))
                            //(a_pydaw_data->audio_items->items[f_i]->length)))
                    {   
                        v_pydaw_audio_item_set_fade_vol(a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]);
                        
                        if(a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->wav_pool_item->channels == 1)
                        {
                            float f_tmp_sample0 = f_cubic_interpolate_ptr_ifh(
                            (a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->wav_pool_item->samples[0]),
                            (a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->sample_read_head->whole_number),
                            (a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->sample_read_head->fraction),
                            (a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->cubic_interpolator)) * 
                            (a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->adsr->output) *
                            (a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->vol_linear) * 
                            (a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->fade_vol);

                            float f_tmp_sample1 = f_tmp_sample0;
                            
                            if(a_pydaw_data->pysong->per_audio_item_fx[(a_pydaw_data->current_region)])
                            {
                                if(a_pydaw_data->pysong->per_audio_item_fx[(a_pydaw_data->current_region)]->items[f_i])
                                {
                                    int f_i3 = 0;                                    
                                    while(f_i3 < 8)
                                    {                                        
                                        a_pydaw_data->pysong->per_audio_item_fx[(a_pydaw_data->current_region)]->items[f_i][f_i3]->func_ptr(
                                        a_pydaw_data->pysong->per_audio_item_fx[(a_pydaw_data->current_region)]->items[f_i][f_i3]->mf3,
                                        f_tmp_sample0, f_tmp_sample1);
                                        f_tmp_sample0 = 
                                                a_pydaw_data->pysong->per_audio_item_fx[(a_pydaw_data->current_region)]->items[f_i][f_i3]->mf3->output0;
                                        f_tmp_sample1 = 
                                                a_pydaw_data->pysong->per_audio_item_fx[(a_pydaw_data->current_region)]->items[f_i][f_i3]->mf3->output1;
                                        f_i3++;
                                    }
                                }
                            }
                            
                            a_output0[f_i2] += f_tmp_sample0;
                            a_output1[f_i2] += f_tmp_sample1;
                        }
                        else if(a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->wav_pool_item->channels == 2)
                        {
                            float f_tmp_sample0 = f_cubic_interpolate_ptr_ifh(
                            (a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->wav_pool_item->samples[0]),
                            (a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->sample_read_head->whole_number),
                            (a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->sample_read_head->fraction),
                            (a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->cubic_interpolator)) * 
                            (a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->adsr->output) *
                            (a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->vol_linear) * 
                            (a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->fade_vol);

                            float f_tmp_sample1 = f_cubic_interpolate_ptr_ifh(
                            (a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->wav_pool_item->samples[1]),
                            (a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->sample_read_head->whole_number),
                            (a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->sample_read_head->fraction),
                            (a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->cubic_interpolator)) * 
                            (a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->adsr->output) *
                            (a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->vol_linear) * 
                            (a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->fade_vol);
                            
                            
                            if(a_pydaw_data->pysong->per_audio_item_fx[(a_pydaw_data->current_region)])
                            {
                                if(a_pydaw_data->pysong->per_audio_item_fx[(a_pydaw_data->current_region)]->loaded[f_i])
                                {
                                    int f_i3 = 0;                                    
                                    while(f_i3 < 8)
                                    {
                                        a_pydaw_data->pysong->per_audio_item_fx[(a_pydaw_data->current_region)]->items[f_i][f_i3]->func_ptr(
                                        a_pydaw_data->pysong->per_audio_item_fx[(a_pydaw_data->current_region)]->items[f_i][f_i3]->mf3,
                                        f_tmp_sample0, f_tmp_sample1);
                                        f_tmp_sample0 = 
                                                a_pydaw_data->pysong->per_audio_item_fx[(a_pydaw_data->current_region)]->items[f_i][f_i3]->mf3->output0;
                                        f_tmp_sample1 = 
                                                a_pydaw_data->pysong->per_audio_item_fx[(a_pydaw_data->current_region)]->items[f_i][f_i3]->mf3->output1;                                        
                                        f_i3++;
                                    }
                                }
                            }
                            
                            a_output0[f_i2] += f_tmp_sample0;
                            a_output1[f_i2] += f_tmp_sample1;
                            
                        }
                        else
                        {
                            //TODO:  Catch this during load and do something then...
                            printf("Error: v_pydaw_pysong->audio_items[a_pydaw_data->current_region]_run, invalid number of channels %i\n", a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->wav_pool_item->channels);
                        }

                        v_ifh_run(a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->sample_read_head, a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->ratio);
                        
                        if(a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->sample_read_head->whole_number >=
                            a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->wav_pool_item->length - 1)
                        {
                            a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->adsr->stage = 4;
                            break;
                        }
                        
                        v_adsr_run_db(a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->adsr);

                        f_i2++;
                    }

                    if(f_adjusted_sample_count != a_sample_count)  //This would mean that there is an ADSR release or loop event in this cycle
                    {
                        if(a_pydaw_data->ml_is_looping)
                        {
                            double test1;
                            
                            if(a_pydaw_data->loop_mode == PYDAW_LOOP_MODE_BAR)
                            {
                                test1 = f_adjusted_next_song_pos_beats - 4.0f - (a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->adjusted_start_beat);                                
                            }
                            else if(a_pydaw_data->loop_mode == PYDAW_LOOP_MODE_REGION)
                            {                                
                                test1 = f_adjusted_next_song_pos_beats - 4.0f - (a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->adjusted_start_beat);
                            }
                                                                                    
                            if(test1 < 0.0f)  //meaning the audio item starts in mid-region...
                            {
                                //printf("test1 < 0.0f");
                                v_adsr_release(a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->adsr);
                            }
                            else
                            {
                                double test2 = test1 * (a_pydaw_data->samples_per_beat) * (a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->ratio);
                                v_ifh_retrigger_double(a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->sample_read_head, test2 + 
                                        a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->sample_start_offset_float); //PYDAW_AUDIO_ITEM_PADDING_DIV2_FLOAT);                                
                                //v_adsr_retrigger(a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->adsr);                                
                            }                            
                        }
                        else
                        {
                            v_adsr_release(a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->adsr);
                        }
                        
                        f_adjusted_sample_count = a_sample_count;
                    }
                    else
                    {
                        break;
                    }

                } //while(1)
            }  //if stage
        } //if this track_num
        f_i++;
    }

    return f_return_value;
}


t_pynote * g_pynote_get(int a_note, int a_vel, float a_start, float a_length)
{
    t_pynote * f_result = (t_pynote*)malloc(sizeof(t_pynote));
    
    f_result->length = a_length;
    f_result->note = a_note;
    f_result->start = a_start;
    f_result->velocity = a_vel;
    
    return f_result;
}

t_pycc * g_pycc_get(int a_plugin_index, int a_cc_num, float a_cc_val, float a_start)
{
    t_pycc * f_result = (t_pycc*)malloc(sizeof(t_pycc));
    
    f_result->plugin_index = a_plugin_index;
    f_result->port = a_cc_num;
    f_result->cc_val = a_cc_val;
    f_result->start = a_start;
    
    return f_result;
}

t_pypitchbend * g_pypitchbend_get(float a_start, float a_value)
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
        f_result->audio_items[f_i] = 0;
        f_result->per_audio_item_fx[f_i] = 0;
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
            int f_uid = atoi(f_region_char);
            f_result->regions[f_pos] = g_pyregion_get(a_pydaw_data, f_uid);
            f_result->regions[f_pos]->uid = f_uid;
            //v_pydaw_audio_items_free(a_pydaw_data->audio_items);
            f_result->audio_items[f_pos] = v_audio_items_load_all(a_pydaw_data, f_uid);
            f_result->per_audio_item_fx[f_pos] = g_paif_region_open(a_pydaw_data, f_uid);

            free(f_pos_char);
            free(f_region_char);
            f_i++;
        }

        g_free_2d_char_array(f_current_string);
    }
    
    t_pysong * f_old = a_pydaw_data->pysong;
    
    pthread_mutex_lock(&a_pydaw_data->main_mutex);
    
    a_pydaw_data->pysong = f_result;
    
    pthread_mutex_unlock(&a_pydaw_data->main_mutex);
    
    if(f_old)
    {
        v_pysong_free(f_old);
    }
        
#ifdef PYDAW_MEMCHECK
    v_pydaw_assert_memory_integrity(a_pydaw_data);
#endif
}

int i_pydaw_get_item_index_from_uid(t_pydaw_data * a_pydaw_data, int a_uid)
{
    int f_i = 0;
    
    while(f_i < a_pydaw_data->item_count)
    {
        if(a_pydaw_data->item_pool[f_i]->uid)  //Accounting for recorded items that aren't named yet
        {
            if(a_uid == a_pydaw_data->item_pool[f_i]->uid)
            {
                return f_i;
            }
        }        
        f_i++;
    }
    return -1;
}

int i_get_song_index_from_region_uid(t_pydaw_data* a_pydaw_data, int a_uid)
{
    int f_i = 0;
    
    while(f_i < PYDAW_MAX_REGION_COUNT)
    {
        if(a_pydaw_data->pysong->regions[f_i])
        {
            if(a_uid == a_pydaw_data->pysong->regions[f_i]->uid)
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
    f_result->uid = (a_pydaw_data->rec_region_current_uid);
    a_pydaw_data->rec_region_current_uid = (a_pydaw_data->rec_region_current_uid) + 1;
    
    int f_i = 0;
    int f_i2 = 0;
    
    while(f_i < PYDAW_TRACK_COUNT_ALL)
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

t_pyregion * g_pyregion_get(t_pydaw_data* a_pydaw_data, int a_uid)
{    
    t_pyregion * f_result = (t_pyregion*)malloc(sizeof(t_pyregion));    
    if (a_uid >= a_pydaw_data->rec_region_current_uid)
    {
        a_pydaw_data->rec_region_current_uid = (a_pydaw_data->rec_region_current_uid) + 1;
    }
    f_result->alternate_tempo = 0;
    f_result->tempo = 140.0f;
    f_result->region_length_bars = 0;
    f_result->region_length_beats = 0;
    f_result->bar_length = 0;
    f_result->uid = a_uid;    
    f_result->not_yet_saved = 0;
    
    int f_i = 0;
    int f_i2 = 0;
    
    while(f_i < PYDAW_TRACK_COUNT_ALL)
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
    sprintf(f_full_path, "%s%i", a_pydaw_data->region_folder, a_uid);    
    
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
        
        char * f_item_uid = c_iterate_2d_char_array(f_current_string);
        assert(f_y < PYDAW_TRACK_COUNT_ALL);
        assert(f_x < PYDAW_MAX_REGION_SIZE);
        f_result->item_indexes[f_y][f_x] = i_pydaw_get_item_index_from_uid(a_pydaw_data, atoi(f_item_uid));
        assert((f_result->item_indexes[f_y][f_x]) != -1);
        assert((f_result->item_indexes[f_y][f_x]) < a_pydaw_data->item_count);
        free(f_item_uid);        

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
            sprintf(f_temp, "%i|%i\n", f_i, a_pydaw_data->pysong->regions[f_i]->uid);
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
    
    while(1)
    {
        int f_no_changes = 1;
        
        f_i = 0;
        /*Use the Bubble Sort algorithm.  It's generally complete garbage, but in this instance, where the list is going to be
         pseudo-sorted to begin with, it's actually very efficient, likely only making 3 passes at most, 1 or 2 passes typically*/
        while(f_i < (f_pyitem->cc_count - 1))
        {        
            if((f_pyitem->ccs[f_i]->start) > (f_pyitem->ccs[f_i + 1]->start))
            {
                t_pycc * f_greater = f_pyitem->ccs[f_i];
                t_pycc * f_lesser = f_pyitem->ccs[f_i + 1];
                f_pyitem->ccs[f_i] = f_lesser;
                f_pyitem->ccs[f_i + 1] = f_greater;
                f_no_changes = 0;
            }

            f_i++;
        }
        
        if(f_no_changes)
        {
            break;
        }
    }
    
    while(1)
    {
        int f_no_changes = 1;
        
        f_i = 0;
        /*Use the Bubble Sort algorithm.  It's generally complete garbage, but in this instance, where the list is going to be
         pseudo-sorted to begin with, it's actually very efficient, likely only making 3 passes at most, 1 or 2 passes typically*/
        while(f_i < (f_pyitem->pitchbend_count - 1))
        {        
            if((f_pyitem->pitchbends[f_i]->start) > (f_pyitem->pitchbends[f_i + 1]->start))
            {
                t_pypitchbend * f_greater = f_pyitem->pitchbends[f_i];
                t_pypitchbend * f_lesser = f_pyitem->pitchbends[f_i + 1];
                f_pyitem->pitchbends[f_i] = f_lesser;
                f_pyitem->pitchbends[f_i + 1] = f_greater;
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
        sprintf(f_temp, "c|%f|%i|%i|%f\n", f_pyitem->ccs[f_i]->start, f_pyitem->ccs[f_i]->plugin_index, f_pyitem->ccs[f_i]->port, f_pyitem->ccs[f_i]->cc_val);
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
    sprintf(f_temp, "%s%i", a_pydaw_data->item_folder, f_pyitem->uid);    
    v_pydaw_write_to_file(f_temp, f_result);    
    sprintf(f_temp, "%i\n", f_pyitem->uid);
    v_pydaw_append_to_file(a_pydaw_data->recorded_items_file, f_temp);
}

/* Items must be saved before regions to prevent a SEGFAULT at the line that references item name...*/
void v_save_pyregion_to_disk(t_pydaw_data * a_pydaw_data, int a_region_num)
{    
    int f_i = 0;
    int f_i2 = 0;
    
    char * f_result = (char*)malloc(sizeof(char) * LMS_LARGE_STRING);
    strcpy(f_result, "");
    
    char f_temp[LMS_TINY_STRING];
    
    while(f_i < PYDAW_TRACK_COUNT_ALL)
    {
        f_i2 = 0;
        while(f_i2 < PYDAW_MAX_REGION_SIZE)
        {
            if(a_pydaw_data->pysong->regions[a_region_num]->item_indexes[f_i][f_i2]  != -1)
            {                
                sprintf(f_temp, "%i|%i|%i\n", f_i, f_i2, 
                        a_pydaw_data->item_pool[(a_pydaw_data->pysong->regions[a_region_num]->item_indexes[f_i][f_i2])]->uid);
                strcat(f_result, f_temp);
            }
            f_i2++;
        }
        f_i++;
    }
    
    strcat(f_result, "\\");
    sprintf(f_temp, "%s%i", a_pydaw_data->region_folder, a_pydaw_data->pysong->regions[a_region_num]->uid);            
    v_pydaw_write_to_file(f_temp, f_result);
    sprintf(f_temp, "%i\n", a_pydaw_data->pysong->regions[a_region_num]->uid);
    v_pydaw_append_to_file(a_pydaw_data->recorded_regions_file, f_temp);
}

/*Get an empty pyitem, used for recording.  Returns the item number in the item pool*/
int g_pyitem_get_new(t_pydaw_data* a_pydaw_data)
{
    t_pyitem * f_item = (t_pyitem*)malloc(sizeof(t_pyitem));    
    f_item->cc_count = 0;
    f_item->note_count = 0;
    f_item->pitchbend_count = 0;
    f_item->uid = (a_pydaw_data->rec_item_current_uid);
    a_pydaw_data->rec_item_current_uid = (a_pydaw_data->rec_item_current_uid) + 1;    
    a_pydaw_data->item_pool[(a_pydaw_data->item_count)] = f_item;
    int f_result = (a_pydaw_data->item_count);
    a_pydaw_data->item_count = (a_pydaw_data->item_count) + 1;
    return f_result;
}

int g_pyitem_clone(t_pydaw_data * a_pydaw_data, int a_item_index)
{
    int f_result = g_pyitem_get_new(a_pydaw_data);
    a_pydaw_data->item_pool[f_result]->cc_count = a_pydaw_data->item_pool[a_item_index]->cc_count;
    a_pydaw_data->item_pool[f_result]->note_count = a_pydaw_data->item_pool[a_item_index]->note_count;
    a_pydaw_data->item_pool[f_result]->pitchbend_count = a_pydaw_data->item_pool[a_item_index]->pitchbend_count;
    
    int f_i = 0;
    while(f_i < a_pydaw_data->item_pool[a_item_index]->note_count)
    {
        a_pydaw_data->item_pool[f_result]->notes[f_i] = g_pynote_get(
                a_pydaw_data->item_pool[a_item_index]->notes[f_i]->note,
                a_pydaw_data->item_pool[a_item_index]->notes[f_i]->velocity,
                a_pydaw_data->item_pool[a_item_index]->notes[f_i]->start,
                a_pydaw_data->item_pool[a_item_index]->notes[f_i]->length);
        f_i++;
    }
    f_i = 0;
    while(f_i < a_pydaw_data->item_pool[a_item_index]->cc_count)
    {
        a_pydaw_data->item_pool[f_result]->ccs[f_i] = g_pycc_get(
                a_pydaw_data->item_pool[a_item_index]->ccs[f_i]->plugin_index,
                a_pydaw_data->item_pool[a_item_index]->ccs[f_i]->port,
                a_pydaw_data->item_pool[a_item_index]->ccs[f_i]->cc_val,
                a_pydaw_data->item_pool[a_item_index]->ccs[f_i]->start);
        f_i++;
    }
    f_i = 0;
    while(f_i < a_pydaw_data->item_pool[a_item_index]->pitchbend_count)
    {
        a_pydaw_data->item_pool[f_result]->pitchbends[f_i] = g_pypitchbend_get(                
                a_pydaw_data->item_pool[a_item_index]->pitchbends[f_i]->start,
                a_pydaw_data->item_pool[a_item_index]->pitchbends[f_i]->val);
        f_i++;
    }
    
    return f_result;
}

void g_pyitem_get(t_pydaw_data* a_pydaw_data, int a_uid)
{
    t_pyitem * f_result = (t_pyitem*)malloc(sizeof(t_pyitem));
    if (a_uid >= a_pydaw_data->rec_item_current_uid)
    {
        a_pydaw_data->rec_item_current_uid = (a_pydaw_data->rec_item_current_uid) + 1;
    }
    f_result->cc_count = 0;
    f_result->note_count = 0;
    f_result->pitchbend_count = 0;
    f_result->uid = a_uid;
    
    char f_full_path[512];
    sprintf(f_full_path, "%s%i", a_pydaw_data->item_folder, a_uid);    
    
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
            char * f_cc_plugin_index = c_iterate_2d_char_array(f_current_string);
            char * f_cc_num = c_iterate_2d_char_array(f_current_string);  //this is really port number, will refactor later...
            char * f_cc_val = c_iterate_2d_char_array(f_current_string);
            
            f_result->ccs[(f_result->cc_count)] = g_pycc_get(atoi(f_cc_plugin_index), atoi(f_cc_num), atof(f_cc_val), atof(f_start));
            f_result->cc_count = (f_result->cc_count) + 1;
            
            free(f_cc_num);
            free(f_cc_val);
        }        
        else if(!strcmp(f_type, "p")) //pitchbend
        {            
            char * f_pb_val_char = c_iterate_2d_char_array(f_current_string);
            float f_pb_val = atof(f_pb_val_char) * 8192.0f;
            
            f_result->pitchbends[(f_result->pitchbend_count)] = g_pypitchbend_get(atof(f_start), f_pb_val);
            f_result->pitchbend_count = (f_result->pitchbend_count) + 1;
            
            free(f_pb_val_char);
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
    
    int f_item_index = i_pydaw_get_item_index_from_uid(a_pydaw_data, a_uid);
    
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

t_pytrack * g_pytrack_get(int a_track_num, int a_track_type)
{
    t_pytrack * f_result = (t_pytrack*)malloc(sizeof(t_pytrack));
    
    f_result->track_num = a_track_num;
    f_result->track_type = a_track_type;
    f_result->mute = 0;
    f_result->solo = 0;
    f_result->volume = 0.0f;
    f_result->volume_linear = 1.0f;
    f_result->plugin_index = 0;
    f_result->event_buffer = (snd_seq_event_t*)malloc(sizeof(snd_seq_event_t) * PYDAW_MAX_EVENT_BUFFER_SIZE);    
    f_result->bus_num = 0;
    f_result->name[0] = '\0';
            
    f_result->bus_buffer_state = 0;
    f_result->bus_count = 0;
    f_result->bus_counter = 0;
        
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
    pthread_mutex_init(&f_result->audio_inputs_mutex, NULL);
    
    
    f_result->sample_rate = a_sample_rate;
    f_result->current_sample = 0;
    f_result->current_bar = 0;
    f_result->current_region = 0;
    f_result->playback_cursor = 0.0f;
    f_result->playback_inc = 0.0f;

    f_result->overdub_mode = 0;    
    f_result->loop_mode = 0;
    f_result->item_folder = (char*)malloc(sizeof(char) * 256);
    f_result->instruments_folder = (char*)malloc(sizeof(char) * 256);
    f_result->project_folder = (char*)malloc(sizeof(char) * 256);
    f_result->region_folder = (char*)malloc(sizeof(char) * 256);
    //f_result->project_name = (char*)malloc(sizeof(char) * 256);
    
    f_result->busfx_folder = (char*)malloc(sizeof(char) * 256);
    f_result->audio_folder = (char*)malloc(sizeof(char) * 256);
    f_result->audio_tmp_folder = (char*)malloc(sizeof(char) * 256);
    f_result->audiofx_folder = (char*)malloc(sizeof(char) * 256);
    f_result->samples_folder = (char*)malloc(sizeof(char) * 256);
    f_result->samplegraph_folder = (char*)malloc(sizeof(char) * 256);
    f_result->recorded_items_file = (char*)malloc(sizeof(char) * 256);
    f_result->recorded_regions_file = (char*)malloc(sizeof(char) * 256);
    f_result->wav_pool_file = (char*)malloc(sizeof(char) * 256);
    f_result->region_audio_folder = (char*)malloc(sizeof(char) * 256);
    f_result->per_audio_item_fx_folder = (char*)malloc(sizeof(char) * 256);
    
    f_result->playback_mode = 0;
    f_result->pysong = NULL;
    f_result->item_count = 0;
    f_result->is_soloed = 0;
    f_result->recording_in_current_bar = 0;
    f_result->record_name_index_items = 0;
    f_result->record_name_index_regions = 0;
    f_result->recorded_note_current_beat = 0;
    f_result->suppress_new_audio_items = 0;
    f_result->recording_current_item_pool_index = -1;
    f_result->recording_first_item = -1;
    
    f_result->ml_current_period_beats = 0.0f;
    f_result->ml_next_period_beats = 0.0f;
    f_result->ml_next_playback_cursor = 0.0f;
    f_result->ml_sample_period_inc = 0.0f;
    f_result->ml_sample_period_inc_beats = 0.0f;
    
    f_result->ml_current_region = 0;
    f_result->ml_next_region = 0;
    f_result->ml_next_bar = 0;
    f_result->ml_next_beat = 0.0;
    f_result->ml_starting_new_bar = 0;
    f_result->ml_is_looping = 0;
    
    f_result->rec_region_current_uid = 10000000;
    f_result->rec_item_current_uid = 10000000;
    
    f_result->amp_ptr = g_amp_get();
    f_result->is_offline_rendering = 0;
    
    f_result->default_region_length_bars = 8;
    f_result->default_region_length_beats = 0;
    f_result->default_bar_length = 4;
    
    f_result->input_buffers_active = 0;
    f_result->record_armed_track = 0;
    f_result->record_armed_track_index_all = -1;
    
    f_result->wav_pool = g_wav_pool_get(a_sample_rate);
    f_result->ab_wav_item = 0;
    f_result->ab_audio_item = g_pydaw_audio_item_get(f_result->sample_rate);
    f_result->ab_mode = 0;
    f_result->ab_start = 0.0f;
    f_result->ab_amp_lin = 1.0f;
    f_result->is_ab_ing = 0;
    f_result->cubic_interpolator = g_cubic_get();
    f_result->preview_wav_item = 0;
    f_result->preview_audio_item = g_pydaw_audio_item_get(f_result->sample_rate);
    f_result->preview_mode = 0;
    f_result->preview_start = 0.0f;
    f_result->preview_amp_lin = 1.0f;
    f_result->is_previewing = 0;
    f_result->preview_max_sample_count = ((int)(a_sample_rate)) * 12;
    
    int f_i = 0;
    
    while(f_i < PYDAW_MIDI_NOTE_COUNT)
    {
        f_result->cc_map[f_i] = 0;
        f_i++;
    }
            
    v_pydaw_load_cc_map(f_result, "default");
    
    f_i = 0;
    int f_track_total = 0;
    
    while(f_i < PYDAW_AUDIO_INPUT_TRACK_COUNT)
    {
        f_result->audio_inputs[f_i] = g_pyaudio_input_get(a_sample_rate);
        f_result->audio_inputs[f_i]->input_port[0] = f_i * 2;
        f_result->audio_inputs[f_i]->input_port[1] = (f_i * 2) + 1;
        f_i++;
    }

    f_i = 0;
    
    while(f_i < PYDAW_MIDI_TRACK_COUNT)
    {
        f_result->track_pool[f_i] = g_pytrack_get(f_i, 0);        
        f_result->track_pool_all[f_track_total] = f_result->track_pool[f_i];
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
        f_track_total++;
    }     
    
    f_i = 0;
    
    while(f_i < PYDAW_BUS_TRACK_COUNT)
    {
        pthread_spin_init(&f_result->bus_spinlocks[f_i], 0);
        f_result->bus_pool[f_i] = g_pytrack_get(f_i, 1);    
        f_result->track_pool_all[f_track_total] = f_result->bus_pool[f_i];
        sprintf(f_result->bus_pool[f_i]->name, "Bus%i", f_i);        
        f_i++;
        f_track_total++;
    }
    
    sprintf(f_result->bus_pool[0]->name, "Master");
    
    f_i = 0;
    
    while(f_i < PYDAW_AUDIO_TRACK_COUNT)
    {        
        f_result->audio_track_pool[f_i] = g_pytrack_get(f_i, 2);
        f_result->track_pool_all[f_track_total] = f_result->audio_track_pool[f_i];
        f_i++;
        f_track_total++;
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

    f_result->uiTarget = lo_address_new_from_url("osc.udp://localhost:30321/");
    
    return f_result;
}

void v_open_default_project(t_pydaw_data * a_data)
{
    char * f_home = getenv("HOME");
    char f_default_project_folder[512];
    sprintf(f_default_project_folder, "%s/pydaw3/default-project", f_home);
    v_open_project(a_data, f_default_project_folder, 1);
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
        switch(a_track->track_type)
        {
            case 0:  //MIDI
                sprintf(f_file_name, "%s%i.pyfx", a_pydaw_data->instruments_folder, a_track->track_num);
                break;
            case 1:  //Bus
                sprintf(f_file_name, "%s%i.pyfx", a_pydaw_data->busfx_folder, a_track->track_num);
                break;
            case 2:  //Audio
                sprintf(f_file_name, "%s%i.pyfx", a_pydaw_data->audiofx_folder, a_track->track_num);
                break;
        }        
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
                
                assert(f_port_key < (f_instance->controlIns));
                
                f_instance->pluginControlIns[f_port_key] = f_port_value;
            }                
        }

        g_free_2d_char_array(f_2d_array);
        
        if(f_instance->euphoria_load_set)
        {
            char * message = f_instance->descriptor->configure(f_instance->PYFX_handle, "load", f_instance->euphoria_load);
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
    
    a_pydaw_data->record_armed_track = 0;
    a_pydaw_data->record_armed_track_index_all = -1;
    
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
                        
            int f_vol = atoi(f_vol_str);
            free(f_vol_str);
            assert(f_vol < 24 && f_vol > -150);
                        
            sprintf(a_pydaw_data->track_pool[f_track_index]->name, "%s", f_name_str);
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
                v_set_plugin_index(a_pydaw_data, a_pydaw_data->track_pool[f_track_index], f_plugin_index, 0);
            }
            
            a_pydaw_data->track_pool[f_track_index]->solo = f_solo;
            a_pydaw_data->track_pool[f_track_index]->mute = f_mute;
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
            
            v_set_plugin_index(a_pydaw_data, a_pydaw_data->track_pool[f_i], 0, 0);
            
            a_pydaw_data->track_pool[f_i]->solo = 0;
            a_pydaw_data->track_pool[f_i]->mute = 0;
            sprintf(a_pydaw_data->track_pool[f_i]->name, "track%i", (f_i + 1));  //TODO:  Move this to the above 'if' statement so the bus tracks can be named differently       
            v_pydaw_set_track_volume(a_pydaw_data, a_pydaw_data->track_pool[f_i], 0.0f);            
            v_pydaw_open_track(a_pydaw_data, a_pydaw_data->track_pool[f_i]);
            
            f_i++;
        }
    }
    
    sprintf(f_file_name, "%sdefault.pybus", a_pydaw_data->project_folder);
    
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
            
            char * f_vol_str = c_iterate_2d_char_array(f_2d_array);
                        
            int f_track_index = atoi(f_track_index_str);
            free(f_track_index_str);
            assert(f_track_index >= 0 && f_track_index < PYDAW_BUS_TRACK_COUNT);
                        
            int f_vol = atoi(f_vol_str);
            free(f_vol_str);
            assert(f_vol < 24 && f_vol > -150);
                                                
            v_pydaw_set_track_volume(a_pydaw_data, a_pydaw_data->bus_pool[f_track_index], f_vol);
            
            v_pydaw_open_track(a_pydaw_data, a_pydaw_data->bus_pool[f_track_index]);
        }

        g_free_2d_char_array(f_2d_array);
    }
    else   //ensure everything is closed...
    {
        int f_i = 0;
        
        while(f_i < PYDAW_BUS_TRACK_COUNT)
        {
            a_pydaw_data->bus_pool[f_i]->plugin_index = 0;  //Must set it to zero to prevent the state file from being deleted
            
            v_set_plugin_index(a_pydaw_data, a_pydaw_data->bus_pool[f_i], 0, 0);
            
            a_pydaw_data->bus_pool[f_i]->solo = 0;
            a_pydaw_data->bus_pool[f_i]->mute = 0;
            sprintf(a_pydaw_data->bus_pool[f_i]->name, "Bus%i", (f_i + 1));  //TODO:  Move this to the above 'if' statement so the bus tracks can be named differently       
            v_pydaw_set_track_volume(a_pydaw_data, a_pydaw_data->bus_pool[f_i], 0.0f);            
            v_pydaw_open_track(a_pydaw_data, a_pydaw_data->bus_pool[f_i]);
            
            f_i++;
        }
        sprintf(a_pydaw_data->bus_pool[0]->name, "Master");
    }    
    
    sprintf(f_file_name, "%sdefault.pyaudio", a_pydaw_data->project_folder);
    
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
            char * f_vol_str = c_iterate_2d_char_array(f_2d_array);
            char * f_name_str = c_iterate_2d_char_array(f_2d_array);            
            char * f_bus_num_str = c_iterate_2d_char_array(f_2d_array);
                        
            int f_track_index = atoi(f_track_index_str);
            free(f_track_index_str);
            assert(f_track_index >= 0 && f_track_index < PYDAW_AUDIO_TRACK_COUNT);
            
            int f_solo = atoi(f_solo_str);
            free(f_solo_str);
            assert(f_solo == 0 || f_solo == 1);
            
            int f_mute = atoi(f_mute_str);
            free(f_mute_str);
            assert(f_mute == 0 || f_mute == 1);
            
            int f_vol = atoi(f_vol_str);
            free(f_vol_str);
            assert(f_vol < 24 && f_vol > -150);
                        
            strcpy(a_pydaw_data->audio_track_pool[f_track_index]->name, f_name_str);
            free(f_name_str);
            
            int f_bus_num = atoi(f_bus_num_str);
            free(f_bus_num_str);
            assert(f_bus_num >= 0 && f_bus_num < PYDAW_BUS_TRACK_COUNT);
            
            a_pydaw_data->audio_track_pool[f_track_index]->solo = f_solo;
            a_pydaw_data->audio_track_pool[f_track_index]->mute = f_mute;            
            v_pydaw_set_track_volume(a_pydaw_data, a_pydaw_data->audio_track_pool[f_track_index], f_vol);
            a_pydaw_data->audio_track_pool[f_track_index]->bus_num = f_bus_num;
            
            v_pydaw_open_track(a_pydaw_data, a_pydaw_data->audio_track_pool[f_track_index]);
        }

        g_free_2d_char_array(f_2d_array);
    }
    else   //ensure everything is closed...
    {
        int f_i = 0;
        
        while(f_i < PYDAW_AUDIO_TRACK_COUNT)
        {
            //a_pydaw_data->audio_track_pool[f_i]->plugin_index = 0;  //Must set it to zero to prevent the state file from being deleted
            
            v_set_plugin_index(a_pydaw_data, a_pydaw_data->audio_track_pool[f_i], 0, 0);
            
            a_pydaw_data->audio_track_pool[f_i]->solo = 0;
            a_pydaw_data->audio_track_pool[f_i]->mute = 0;
            sprintf(a_pydaw_data->audio_track_pool[f_i]->name, "track%i", (f_i + 1));  //TODO:  Move this to the above 'if' statement so the bus tracks can be named differently       
            v_pydaw_set_track_volume(a_pydaw_data, a_pydaw_data->audio_track_pool[f_i], 0.0f);            
            v_pydaw_open_track(a_pydaw_data, a_pydaw_data->audio_track_pool[f_i]);
            
            f_i++;
        }
    }
    
#ifdef PYDAW_MEMCHECK
    v_pydaw_assert_memory_integrity(a_pydaw_data);
#endif
}

void v_open_project(t_pydaw_data* a_pydaw_data, const char* a_project_folder, int a_first_load)
{
    clock_t f_start = clock();
    
    pthread_mutex_lock(&a_pydaw_data->offline_mutex);
        
    sprintf(a_pydaw_data->project_folder, "%s/", a_project_folder);    
    sprintf(a_pydaw_data->item_folder, "%sitems/", a_pydaw_data->project_folder);
    sprintf(a_pydaw_data->region_folder, "%sregions/", a_pydaw_data->project_folder);
    sprintf(a_pydaw_data->region_audio_folder, "%sregions_audio/", a_pydaw_data->project_folder);    
    sprintf(a_pydaw_data->instruments_folder, "%sinstruments/", a_pydaw_data->project_folder);    
    sprintf(a_pydaw_data->audio_folder, "%saudio/", a_pydaw_data->project_folder);
    sprintf(a_pydaw_data->audio_tmp_folder, "%saudio/tmp/", a_pydaw_data->project_folder);
    sprintf(a_pydaw_data->audiofx_folder, "%saudiofx/", a_pydaw_data->project_folder);
    sprintf(a_pydaw_data->busfx_folder, "%sbusfx/", a_pydaw_data->project_folder);
    sprintf(a_pydaw_data->samples_folder, "%ssamples/", a_pydaw_data->project_folder);
    sprintf(a_pydaw_data->samplegraph_folder, "%ssamplegraph/", a_pydaw_data->project_folder);    
    sprintf(a_pydaw_data->recorded_items_file, "%srecorded_items", a_pydaw_data->project_folder);
    sprintf(a_pydaw_data->recorded_regions_file, "%srecorded_regions", a_pydaw_data->project_folder);    
    sprintf(a_pydaw_data->wav_pool_file, "%sdefault.pywavs", a_pydaw_data->project_folder);
    sprintf(a_pydaw_data->per_audio_item_fx_folder, "%saudio_per_item_fx/", a_pydaw_data->project_folder);
    
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
    
    if(a_first_load)
    {
        v_pydaw_init_busses(a_pydaw_data);
    }
        
    if(a_first_load && i_pydaw_file_exists(a_pydaw_data->wav_pool_file))
    {
        v_wav_pool_add_items(a_pydaw_data->wav_pool, a_pydaw_data->wav_pool_file);
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
                
        assert(f_tempo > 30.0f && f_tempo < 300.0f);        
        v_set_tempo(a_pydaw_data, f_tempo);
        g_free_2d_char_array(f_2d_array);
    }
    else  //No transport file, set default tempo
    {
        printf("No transport file found, defaulting to 140.0 BPM\n");
        v_set_tempo(a_pydaw_data, 140.0f);
    }
    
    if(S_ISDIR(f_proj_stat.st_mode) && S_ISDIR(f_item_stat.st_mode) &&
        S_ISDIR(f_reg_stat.st_mode) && S_ISDIR(f_inst_stat.st_mode) &&
        S_ISREG(f_song_file_stat.st_mode))
    {
        t_dir_list * f_item_dir_list = g_get_dir_list(a_pydaw_data->item_folder);    
        f_i = 0;

        while(f_i < f_item_dir_list->dir_count)
        {            
            g_pyitem_get(a_pydaw_data, atoi(f_item_dir_list->dir_list[f_i]));
            f_i++;
        }
    
        g_pysong_get(a_pydaw_data);
        
        if(a_first_load)
        {
            v_pydaw_open_tracks(a_pydaw_data);
        }
    }
    else
    {
        printf("Song file and project directory structure not found, not loading project.  This is to be expected if launching PyDAW for the first time\n");
        g_pysong_get(a_pydaw_data);  //Loads empty...  TODO:  Make this a separate function for getting an empty pysong or loading a file into one...        
    }
                   
    //v_pydaw_update_audio_inputs(a_pydaw_data);
    
    v_pydaw_set_is_soloed(a_pydaw_data);
        
    v_pydaw_schedule_work(a_pydaw_data);
    
#ifdef PYDAW_MEMCHECK
    v_pydaw_assert_memory_integrity(a_pydaw_data);
#endif    
    pthread_mutex_unlock(&a_pydaw_data->offline_mutex);
    
    v_pydaw_print_benchmark("v_open_project", f_start);
}

/* Moved to it's own function for re-usability, because the mutex must be held when calling*/
void v_pydaw_reset_audio_item_read_heads(t_pydaw_data * a_pydaw_data, int a_region, int a_bar)
{
    if(!a_pydaw_data->pysong->audio_items[a_region])
    {
        return;
    }
    
    int f_i = 0;

    double f_adjusted_song_pos_beats = 
        v_pydaw_count_beats(a_pydaw_data, 0, 0, 0.0f, a_region, a_bar, 0.0f);
    
    if(f_adjusted_song_pos_beats < 0.0f)  //clip at zero, lest it somehow by rounding error is -0.0000001
    {
        f_adjusted_song_pos_beats = 0.0f;
    }
        
    while(f_i < PYDAW_MAX_AUDIO_ITEM_COUNT)
    {
        if(a_pydaw_data->pysong->audio_items[a_region]->items[f_i])
        {
            a_pydaw_data->pysong->audio_items[a_region]->items[f_i]->wav_pool_item = 
                    g_wav_pool_get_item_by_uid(a_pydaw_data->wav_pool, a_pydaw_data->pysong->audio_items[a_region]->items[f_i]->uid);

            if(((a_pydaw_data->pysong->audio_items[a_region]->items[f_i]->adjusted_start_beat) <= f_adjusted_song_pos_beats) &&
                (((a_pydaw_data->pysong->audio_items[a_region]->items[f_i]->end_mode) == 0) ||
                ((a_pydaw_data->pysong->audio_items[a_region]->items[f_i]->adjusted_end_beat) > f_adjusted_song_pos_beats)))
            {            
                double test1 = f_adjusted_song_pos_beats - (a_pydaw_data->pysong->audio_items[a_region]->items[f_i]->adjusted_start_beat);
                double test2 = test1 * (a_pydaw_data->samples_per_beat) * (a_pydaw_data->pysong->audio_items[a_region]->items[f_i]->ratio);
                v_ifh_retrigger_double(a_pydaw_data->pysong->audio_items[a_region]->items[f_i]->sample_read_head, test2 + PYDAW_AUDIO_ITEM_PADDING_DIV2_FLOAT);
                v_adsr_retrigger(a_pydaw_data->pysong->audio_items[a_region]->items[f_i]->adsr);
            }
        }
        f_i++;
    }
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
            int f_i = 0;
            int f_was_recording = 0;
            if(a_pydaw_data->playback_mode == PYDAW_PLAYBACK_MODE_REC)
            {
                f_was_recording = 1;
            }
            pthread_mutex_lock(&a_pydaw_data->main_mutex);
            a_pydaw_data->suppress_new_audio_items = 1;
            //Fade out the playing audio tracks
            if(a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region])
            {
                while(f_i < PYDAW_MAX_AUDIO_ITEM_COUNT)
                {
                    if(a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i])
                    {
                        v_adsr_release(a_pydaw_data->pysong->audio_items[a_pydaw_data->current_region]->items[f_i]->adsr);
                    }                    
                    f_i++;
                }
            }
            pthread_mutex_unlock(&a_pydaw_data->main_mutex);
            f_i = 0;
            usleep(60000);  
            pthread_mutex_lock(&a_pydaw_data->main_mutex);
            a_pydaw_data->suppress_new_audio_items = 0;
            a_pydaw_data->playback_mode = a_mode;            
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
                
                f_i = 0;
                while(f_i < PYDAW_AUDIO_INPUT_TRACK_COUNT)
                {
                    if(a_pydaw_data->audio_inputs[f_i]->rec)
                    {
                        a_pydaw_data->audio_inputs[f_i]->recording_stopped = 1;                        
                    }
                    f_i++;
                }
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
            v_pydaw_reset_audio_item_read_heads(a_pydaw_data, a_region, a_bar);            
            if(a_pydaw_data->ab_wav_item)
            {
                a_pydaw_data->is_ab_ing = a_pydaw_data->ab_mode;
                if(a_pydaw_data->is_ab_ing)
                {
                    v_ifh_retrigger(a_pydaw_data->ab_audio_item->sample_read_head, 
                            a_pydaw_data->ab_audio_item->sample_start_offset); 
                    v_adsr_retrigger(a_pydaw_data->ab_audio_item->adsr);
                }
            }
            pthread_mutex_unlock(&a_pydaw_data->main_mutex);
            break;
        case 2:  //record
            if(a_pydaw_data->playback_mode == PYDAW_PLAYBACK_MODE_REC)
            {
                return;  
            }                        
            pthread_mutex_lock(&a_pydaw_data->main_mutex);
            a_pydaw_data->is_ab_ing = 0;
            a_pydaw_data->recording_first_item = -1;
            a_pydaw_data->recorded_note_current_beat = 0;
            a_pydaw_data->playback_mode = a_mode;
            a_pydaw_data->recording_in_current_bar = 0;
            a_pydaw_data->recording_current_item_pool_index = -1;
            v_set_playback_cursor(a_pydaw_data, a_region, a_bar);            
            v_pydaw_reset_audio_item_read_heads(a_pydaw_data, a_region, a_bar);
            pthread_mutex_unlock(&a_pydaw_data->main_mutex);
            break;
    }    
    
#ifdef PYDAW_MEMCHECK
    v_pydaw_assert_memory_integrity(a_pydaw_data);
#endif
}


/*Load/Reload samples from file...*/
t_pydaw_audio_items * v_audio_items_load_all(t_pydaw_data * a_pydaw_data, int a_region_uid)
{
    t_pydaw_audio_items * f_result = g_pydaw_audio_items_get(a_pydaw_data->sample_rate);
    char f_file[256] = "\0";
    sprintf(f_file, "%s%i", a_pydaw_data->region_audio_folder, a_region_uid);
    
    if(i_pydaw_file_exists(f_file))
    {
        printf("v_audio_items_load_all: loading a_file: \"%s\"\n", f_file);
        int f_i = 0;

        t_2d_char_array * f_current_string = g_get_2d_array_from_file(f_file, LMS_LARGE_STRING);        
        
        while(f_i < PYDAW_MAX_AUDIO_ITEM_COUNT)
        {            
            t_pydaw_audio_item * f_new =  g_audio_item_load_single(a_pydaw_data->sample_rate, 
                    f_current_string, 0, a_pydaw_data->wav_pool, a_pydaw_data->tempo);
            if(!f_new)  //EOF'd...
            {
                break;
            }
            t_pydaw_audio_item * f_old = f_result->items[f_new->index];
            f_result->items[f_new->index] = f_new;            
            v_pydaw_audio_item_free(f_old);
            
            f_i++;
        }

        g_free_2d_char_array(f_current_string);
    }
    else
    {
        printf("Error:  v_audio_items_load_all:  a_file: \"%s\" does not exist\n", f_file);
        assert(0);
    }
    
    return f_result;
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

void v_pydaw_set_is_soloed(t_pydaw_data * a_pydaw_data)
{
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

    if(a_pydaw_data->is_soloed == 0)
    {
        f_i = 0;
        while(f_i < PYDAW_AUDIO_TRACK_COUNT)
        {
            if(a_pydaw_data->audio_track_pool[f_i]->solo)
            {
                a_pydaw_data->is_soloed = 1;
                break;
            }
            f_i++;
        }
    }
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

void v_pydaw_save_track(t_pydaw_data * a_pydaw_data, t_pytrack * a_track, int a_type)
{
    if(a_track->plugin_index == 0)
    {
        return;  //Delete the file if exists?
    }

    //if(a_track->plugin_index != -1)    
    //{
        v_pydaw_save_plugin(a_pydaw_data, a_track, 0, a_type);
    //}
    
    //Now being done through the UI
    //v_pydaw_save_plugin(a_pydaw_data, a_track, 1, a_type);
    
#ifdef PYDAW_MEMCHECK
    v_pydaw_assert_memory_integrity(a_pydaw_data);
#endif
}

void v_pydaw_save_plugin(t_pydaw_data * a_pydaw_data, t_pytrack * a_track, int a_is_fx, int a_type)
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
    /*
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
    */
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
        switch(a_type)
        {
            case 0: //MIDI track
                sprintf(f_file_name, "%s%i.pyfx", a_pydaw_data->instruments_folder, a_track->track_num);
                break;
            case 1: //Bus track
                sprintf(f_file_name, "%s%i.pyfx", a_pydaw_data->busfx_folder, a_track->track_num);
                break;
            case 2: //audio track
                sprintf(f_file_name, "%s%i.pyfx", a_pydaw_data->audiofx_folder, a_track->track_num);
                break;
            default:
                assert(0);
                break;
        }        
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
    
    /*
    int f_i = 0;
     
    while(f_i < PYDAW_MIDI_TRACK_COUNT)
    {
        if(a_pydaw_data->track_pool[f_i]->plugin_index == 1)
        {
            v_pydaw_save_track(a_pydaw_data, a_pydaw_data->track_pool[f_i], 0);
        }
        f_i++;
    }
    
    f_i = 0;
    
    while(f_i < PYDAW_BUS_TRACK_COUNT)
    {
        v_pydaw_save_track(a_pydaw_data, a_pydaw_data->bus_pool[f_i], 1);
        f_i++;
    }
    
    f_i = 0;
    
    while(f_i < PYDAW_AUDIO_TRACK_COUNT)
    {
        v_pydaw_save_track(a_pydaw_data, a_pydaw_data->audio_track_pool[f_i], 2);
        f_i++;
    }
    */
    printf("Saving tracks complete\n");
    pthread_mutex_unlock(&a_pydaw_data->quit_mutex);
    char f_tmp_file[512] = "\0";
    sprintf(f_tmp_file, "%stracks.finished", a_pydaw_data->project_folder);
    v_pydaw_write_to_file(f_tmp_file, "Finished");
    
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
        filename = "/usr/lib/pydaw3/lms_modulex/LMS_MODULEX_qt";
        dllName = "lms_modulex.so";       
    }
    else
    {
        switch(a_track->plugin_index)
        {
            case 1:
                filename = "/usr/lib/pydaw3/euphoria/LMS_EUPHORIA_qt";
                dllName = "euphoria.so";
                break;
            case 2:
                filename = "/usr/lib/pydaw3/ray_v/LMS_RAYV_qt";
                dllName = "ray_v.so";
                break;
            case 3:
                filename = "/usr/lib/pydaw3/way_v/WAY_V_qt";
                dllName = "way_v.so";
                break;
            default:
                return;
        }

    }
    
    char track_number_string[6];
    sprintf(track_number_string, "%i", a_track->track_num);
    
    char f_track_str[256];
    
    if(a_is_fx)
    {
        switch(a_track->track_type)
        {
            case 0: //MIDI
                sprintf(f_track_str, "[MIDI] %s", a_track->name);
                sprintf(oscUrl, "%s/%i-mfx", a_pydaw_data->osc_url, a_track->track_num);
                break;
            case 1: //Bus
                sprintf(f_track_str, "[Bus] %s", a_track->name);
                sprintf(oscUrl, "%s/%i-bfx", a_pydaw_data->osc_url, a_track->track_num);
                break;
            case 2: //Audio
                sprintf(f_track_str, "[Audio] %s", a_track->name);
                sprintf(oscUrl, "%s/%i-afx", a_pydaw_data->osc_url, a_track->track_num);
                break;
        }        
    }
    else
    {
        sprintf(f_track_str, "%s", a_track->name);
        sprintf(oscUrl, "%s/%i-mi", a_pydaw_data->osc_url, a_track->track_num);
    }
        
#ifdef PYDAW_MEMCHECK
    v_pydaw_assert_memory_integrity(a_pydaw_data);
#endif
    
    if (fork() == 0) 
    {
        execlp(filename, filename, oscUrl, dllName, f_track_str, track_number_string, (char*)NULL);
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
        
        f_i = 0;
        
        while(f_i < PYDAW_BUS_TRACK_COUNT)
        {
            if((a_pydaw_data->bus_pool[f_i]->effect) && a_pydaw_data->bus_pool[f_i]->effect->uiTarget)
            {
                lo_send(a_pydaw_data->bus_pool[f_i]->effect->uiTarget, 
                    a_pydaw_data->bus_pool[f_i]->effect->ui_osc_quit_path, "");
            }
            f_i++;
        }
        
        f_i = 0;
        
        while(f_i < PYDAW_AUDIO_TRACK_COUNT)
        {
            if((a_pydaw_data->audio_track_pool[f_i]->effect) && a_pydaw_data->audio_track_pool[f_i]->effect->uiTarget)
            {
                lo_send(a_pydaw_data->audio_track_pool[f_i]->effect->uiTarget, 
                    a_pydaw_data->audio_track_pool[f_i]->effect->ui_osc_quit_path, "");
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

void v_set_plugin_index(t_pydaw_data * a_pydaw_data, t_pytrack * a_track, int a_index, int a_schedule_threads)
{
    pthread_mutex_lock(&a_track->mutex);  //Prevent multiple simultaneous operations from running in parallel, without blocking the main thread
    t_pydaw_plugin * f_result;
    t_pydaw_plugin * f_result_fx;
    
    char f_file_name[512];
    sprintf(f_file_name, "%s%i.pyinst", a_pydaw_data->instruments_folder, a_track->track_num);
    
    char f_file_name_fx[512];
    sprintf(f_file_name_fx, "%s%i.pyfx", a_pydaw_data->instruments_folder, a_track->track_num);
    
    if(a_index == -1)  //bus track
    {
        f_result_fx = g_pydaw_plugin_get((int)(a_pydaw_data->sample_rate), -1);
        
        free(f_result_fx->pluginOutputBuffers[0]);
        free(f_result_fx->pluginOutputBuffers[1]);
        
        f_result_fx->pluginOutputBuffers = f_result_fx->pluginInputBuffers;
        
        f_result_fx->descriptor->PYFX_Plugin->connect_port(f_result_fx->PYFX_handle, 2, f_result_fx->pluginOutputBuffers[0]);
        f_result_fx->descriptor->PYFX_Plugin->connect_port(f_result_fx->PYFX_handle, 3, f_result_fx->pluginOutputBuffers[1]);
        
        t_pydaw_plugin * f_fx = a_track->effect;
                
        pthread_mutex_lock(&a_pydaw_data->main_mutex);        
        
        a_track->instrument = 0;
        a_track->effect = f_result_fx;
        a_track->plugin_index = a_index;
        a_track->current_period_event_index = 0;
        v_pydaw_open_track(a_pydaw_data, a_track);  //Opens the .inst file if exists
        
        if(a_schedule_threads)
        {
            v_pydaw_schedule_work(a_pydaw_data);
        }
        
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
        
        if(a_schedule_threads)
        {
            v_pydaw_schedule_work(a_pydaw_data);
        }
        
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
        free(f_result_fx->pluginOutputBuffers[0]);
        free(f_result_fx->pluginOutputBuffers[1]);
        
        f_result_fx->pluginInputBuffers = f_result->pluginOutputBuffers;
        f_result_fx->pluginOutputBuffers = f_result_fx->pluginInputBuffers;
        
        f_result_fx->descriptor->PYFX_Plugin->connect_port(f_result_fx->PYFX_handle, 0, f_result_fx->pluginInputBuffers[0]);
        f_result_fx->descriptor->PYFX_Plugin->connect_port(f_result_fx->PYFX_handle, 1, f_result_fx->pluginInputBuffers[1]);
        f_result_fx->descriptor->PYFX_Plugin->connect_port(f_result_fx->PYFX_handle, 2, f_result_fx->pluginOutputBuffers[0]);
        f_result_fx->descriptor->PYFX_Plugin->connect_port(f_result_fx->PYFX_handle, 3, f_result_fx->pluginOutputBuffers[1]);
                
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
        
        if(a_schedule_threads)
        {
            v_pydaw_schedule_work(a_pydaw_data);
        }
        
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
}

void v_pydaw_update_audio_inputs(t_pydaw_data * a_pydaw_data)
{
    char f_inputs_file[256];
    sprintf(f_inputs_file, "%sdefault.pyinput", a_pydaw_data->project_folder);
    
    if(i_pydaw_file_exists(f_inputs_file))
    {
        int f_i = 0;
        t_2d_char_array * f_2d_array = g_get_2d_array_from_file(f_inputs_file, LMS_LARGE_STRING);
        
        pthread_mutex_lock(&a_pydaw_data->audio_inputs_mutex);
        while(f_i < PYDAW_AUDIO_INPUT_TRACK_COUNT)
        {
            char * f_index_str = c_iterate_2d_char_array(f_2d_array);

            if(!strcmp(f_index_str, "\\"))
            {
                free(f_index_str);            
                break;
            }

            int f_index = atoi(f_index_str);
            free(f_index_str);
            
            char * f_rec_str = c_iterate_2d_char_array(f_2d_array);
            int f_rec = atoi(f_rec_str);
            free(f_rec_str);
            
            char * f_vol_str = c_iterate_2d_char_array(f_2d_array);
            int f_vol = atoi(f_vol_str);
            free(f_vol_str);
            
            char * f_out_str = c_iterate_2d_char_array(f_2d_array);
            int f_out = atoi(f_out_str);
            free(f_out_str);
                        
            char * f_in_str = c_iterate_2d_char_array(f_2d_array);
            free(f_in_str);  //Not used yet
            
            a_pydaw_data->audio_inputs[f_index]->rec = f_rec;
            a_pydaw_data->audio_inputs[f_index]->output_track = f_out;
                        
            a_pydaw_data->audio_inputs[f_index]->vol = f_vol;            
            a_pydaw_data->audio_inputs[f_index]->vol_linear = f_db_to_linear_fast(f_vol, a_pydaw_data->amp_ptr);
            
            char f_tmp_file_name[128];
            
            sprintf(f_tmp_file_name, "%s%i.wav", a_pydaw_data->audio_tmp_folder, f_index);
            
            v_pydaw_audio_input_record_set(a_pydaw_data->audio_inputs[f_index], f_tmp_file_name);
            
            f_i++;
        }
        pthread_mutex_unlock(&a_pydaw_data->audio_inputs_mutex);
        g_free_2d_char_array(f_2d_array);
    }
    else 
    {        
        printf("%s not found, setting default values\n", f_inputs_file);
        pthread_mutex_lock(&a_pydaw_data->audio_inputs_mutex);
        int f_i = 0;        
        while(f_i < PYDAW_AUDIO_INPUT_TRACK_COUNT)
        {
            a_pydaw_data->audio_inputs[f_i]->rec = 0;
            a_pydaw_data->audio_inputs[f_i]->output_track = 0;
                        
            a_pydaw_data->audio_inputs[f_i]->vol = 0.0f;
            a_pydaw_data->audio_inputs[f_i]->vol_linear = 1.0f;
            f_i++;
        }
        pthread_mutex_unlock(&a_pydaw_data->audio_inputs_mutex);
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

/*Count the number of beats between 2 points in time...*/
inline float v_pydaw_count_beats(t_pydaw_data * a_pydaw_data, int a_start_region, int a_start_bar, float a_start_beat, 
        int a_end_region, int a_end_bar, float a_end_beat)
{
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
    
    return ((float)(f_beat_total)) + (a_end_beat - a_start_beat);
}

void v_pydaw_offline_render(t_pydaw_data * a_pydaw_data, int a_start_region, int a_start_bar, int a_end_region, 
        int a_end_bar, char * a_file_out)
{
    pthread_mutex_lock(&a_pydaw_data->offline_mutex);
    a_pydaw_data->is_offline_rendering = 1;
    a_pydaw_data->input_buffers_active = 0;
    int f_ab_old = a_pydaw_data->ab_mode;
    a_pydaw_data->ab_mode = 0;
    
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
    
    long f_size = 0;
    long f_block_size = (a_pydaw_data->sample_count);    
    
    float * f_output = (float*)malloc(sizeof(float) * (f_block_size * 2)); //f_sample_count_long);
    
    long f_next_sample_block = 0;
    float * f_buffer0 = (float*)malloc(sizeof(float) * f_block_size);
    float * f_buffer1 = (float*)malloc(sizeof(float) * f_block_size);
    
    int f_old_loop_mode = a_pydaw_data->loop_mode;  //We must set it back afterwards, or the UI will be wrong...
    v_set_loop_mode(a_pydaw_data, PYDAW_LOOP_MODE_OFF);
    
#ifdef PYDAW_DEBUG_RECORDING
    f_i = 0;
    a_pydaw_data->input_buffers = (float**)malloc(sizeof(float*) * PYDAW_AUDIO_INPUT_TRACK_COUNT);
    
    while(f_i < (PYDAW_AUDIO_INPUT_TRACK_COUNT * 2))
    {
        a_pydaw_data->input_buffers[f_i] = (PYFX_Data*)malloc(sizeof(PYFX_Data) * 8192);        
        f_i++;
    }
    a_pydaw_data->input_buffers_active = 1;
    v_set_playback_mode(a_pydaw_data, PYDAW_PLAYBACK_MODE_REC, a_start_region, a_start_bar);
#else    
    v_set_playback_mode(a_pydaw_data, PYDAW_PLAYBACK_MODE_PLAY, a_start_region, a_start_bar);    
#endif   
    SF_INFO f_sf_info;
    f_sf_info.channels = 2;
    f_sf_info.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;
    f_sf_info.samplerate = (int)(a_pydaw_data->sample_rate);
            
    SNDFILE * f_sndfile = sf_open(a_file_out, SFM_WRITE, &f_sf_info);
    
    clock_t f_start = clock();
    
    int f_current_bar = 999;  //For printing the current region/bar
    
    while(((a_pydaw_data->current_region) < a_end_region) || ((a_pydaw_data->current_bar) < a_end_bar))
    {
        if(a_pydaw_data->current_bar != f_current_bar)
        {
            f_current_bar = a_pydaw_data->current_bar;
            printf("%i:%i\n", a_pydaw_data->current_region, a_pydaw_data->current_bar);
        }
        
        f_i = 0;
        f_size = 0;
    
        while(f_i < f_block_size)
        {
            f_buffer0[f_i] = 0.0f;
            f_buffer1[f_i] = 0.0f;
            f_i++;
        }
        
        pthread_mutex_lock(&a_pydaw_data->main_mutex);
        f_next_sample_block = (a_pydaw_data->current_sample) + f_block_size;        
#ifdef PYDAW_DEBUG_RECORDING        
        v_pydaw_run_main_loop(a_pydaw_data, f_block_size, NULL, 0, f_next_sample_block, f_buffer0, f_buffer1, a_pydaw_data->input_buffers);
#else
        v_pydaw_run_main_loop(a_pydaw_data, f_block_size, NULL, 0, f_next_sample_block, f_buffer0, f_buffer1, 0);
#endif
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
        
        sf_writef_float(f_sndfile, f_output, f_block_size);
    }
        
    v_pydaw_print_benchmark("v_pydaw_offline_render ", f_start);
    printf("f_size = %ld\nf_sample_count_long = %ld\n(f_sample_count_long - f_size) = %ld\n", 
            f_size, f_sample_count_long, (f_sample_count_long - f_size));
    
    v_set_playback_mode(a_pydaw_data, PYDAW_PLAYBACK_MODE_OFF, a_start_region, a_start_bar);
    v_set_loop_mode(a_pydaw_data, f_old_loop_mode);
    
    sf_close(f_sndfile);
    
    free(f_buffer0);
    free(f_buffer1);
    free(f_output);    
    
    char f_tmp_finished[256];
    
    sprintf(f_tmp_finished, "%s.finished", a_file_out);
    
    FILE * f_finished = fopen(f_tmp_finished, "w");    
    fclose(f_finished);
    
    a_pydaw_data->is_offline_rendering = 0;
    a_pydaw_data->ab_mode = f_ab_old;
    pthread_mutex_unlock(&a_pydaw_data->offline_mutex);
}


void v_pydaw_set_ab_mode(t_pydaw_data * a_pydaw_data, int a_mode)
{
    pthread_mutex_lock(&a_pydaw_data->main_mutex);
    
    a_pydaw_data->ab_mode = a_mode;
    
    pthread_mutex_unlock(&a_pydaw_data->main_mutex);
}

void v_pydaw_set_ab_start(t_pydaw_data * a_pydaw_data, int a_start)
{
    if(!a_pydaw_data->ab_wav_item)
    {
        return;
    }
    pthread_mutex_lock(&a_pydaw_data->main_mutex);
    a_pydaw_data->ab_audio_item->sample_start = (float)(a_start) * 0.001f;        
    a_pydaw_data->ab_audio_item->sample_start_offset = (int)((a_pydaw_data->ab_audio_item->sample_start * ((float)a_pydaw_data->ab_wav_item->length))) + PYDAW_AUDIO_ITEM_PADDING_DIV2;
    a_pydaw_data->ab_audio_item->sample_start_offset_float = (float)(a_pydaw_data->ab_audio_item->sample_start_offset);
    pthread_mutex_unlock(&a_pydaw_data->main_mutex);
}

void v_pydaw_set_ab_file(t_pydaw_data * a_pydaw_data, const char * a_file)
{
    t_wav_pool_item * f_result = g_wav_pool_item_get(0, a_file, a_pydaw_data->sample_rate);
    
    pthread_mutex_lock(&a_pydaw_data->main_mutex);
    
    t_wav_pool_item * f_old = a_pydaw_data->ab_wav_item;
    a_pydaw_data->ab_wav_item = f_result;
    
    if(!f_result)
    {
        a_pydaw_data->ab_mode = 0;
    }
    
    a_pydaw_data->ab_audio_item->ratio = a_pydaw_data->ab_wav_item->ratio_orig;
    
    pthread_mutex_unlock(&a_pydaw_data->main_mutex);
    
    if(f_old)
    {
        v_wav_pool_item_free(f_old);
    }
}


void v_pydaw_set_ab_vol(t_pydaw_data * a_pydaw_data, float a_vol)
{
    pthread_mutex_lock(&a_pydaw_data->main_mutex);
    a_pydaw_data->ab_amp_lin = f_db_to_linear_fast(a_vol, a_pydaw_data->amp_ptr);
    pthread_mutex_unlock(&a_pydaw_data->main_mutex);
}

void v_pydaw_set_preview_file(t_pydaw_data * a_pydaw_data, const char * a_file)
{
    t_wav_pool_item * f_result = g_wav_pool_item_get(0, a_file, a_pydaw_data->sample_rate);
    
    pthread_mutex_lock(&a_pydaw_data->main_mutex);
    
    t_wav_pool_item * f_old = a_pydaw_data->preview_wav_item;
    a_pydaw_data->preview_wav_item = f_result;
    
    if(!f_result)
    {
        a_pydaw_data->preview_mode = 0;
    }
    
    a_pydaw_data->preview_audio_item->ratio = a_pydaw_data->preview_wav_item->ratio_orig;
    
    a_pydaw_data->preview_mode = 1;
    
    if(a_pydaw_data->preview_wav_item)
    {
        a_pydaw_data->is_previewing = a_pydaw_data->preview_mode;
        if(a_pydaw_data->is_previewing)
        {
            v_ifh_retrigger(a_pydaw_data->preview_audio_item->sample_read_head, 
                    a_pydaw_data->preview_audio_item->sample_start_offset); 
            v_adsr_retrigger(a_pydaw_data->preview_audio_item->adsr);
        }
    }
    
    pthread_mutex_unlock(&a_pydaw_data->main_mutex);
    
    if(f_old)
    {
        v_wav_pool_item_free(f_old);
    }
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
        }
        
        int f_control_in = f_instance->pluginPortControlInNumbers[f_port];        
        f_instance->pluginControlIns[f_control_in] = f_value;
        //f_instance->pluginPortUpdated[f_control_in] = 1;
        pthread_mutex_unlock(&a_pydaw_data->main_mutex);
        //pthread_mutex_unlock(&a_pydaw_data->track_pool[f_track_num]->mutex);
        g_free_1d_char_array(f_val_arr);
    }    
    if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_CONFIGURE_PLUGIN)) //Configure plugin
    {
        t_1d_char_array * f_val_arr = c_split_str(a_value, '|', 5, LMS_TINY_STRING);
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
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_SHOW_PLUGIN_UI))
    {
        int f_track_num = atoi(a_value);
        v_show_plugin_ui(a_pydaw_data, a_pydaw_data->track_pool[f_track_num], 0);        
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_SHOW_FX_UI))
    {
        t_1d_char_array * f_val_arr = c_split_str(a_value, '|', 2, LMS_TINY_STRING);
        int f_track_num = atoi(f_val_arr->array[0]);
        int f_track_type = atoi(f_val_arr->array[1]);        
        g_free_1d_char_array(f_val_arr);
                
        switch(f_track_type)
        {
            case 0:  //MIDI track
                v_show_plugin_ui(a_pydaw_data, a_pydaw_data->track_pool[f_track_num], 1);
                break;
            case 1:  //Bus track
                v_show_plugin_ui(a_pydaw_data, a_pydaw_data->bus_pool[f_track_num], 1);
                break;
            case 2:  //Audio track
                v_show_plugin_ui(a_pydaw_data, a_pydaw_data->audio_track_pool[f_track_num], 1);
                break;            
            case 3:  //Audio Input
                //TODO
                break;
            default:
                assert(0);
                break;
        }                
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_SAVE_TRACKS))
    {
        v_pydaw_save_tracks(a_pydaw_data);
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
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_TRACK_NAME)) //Set track name
    {
        t_1d_char_array * f_val_arr = c_split_str(a_value, '|', 3, LMS_TINY_STRING);
        int f_track_num = atoi(f_val_arr->array[0]);
        int f_track_type = atoi(f_val_arr->array[2]);
        pthread_mutex_lock(&a_pydaw_data->track_pool[f_track_num]->mutex);
        switch(f_track_type)
        {
            case 0:
                strcpy(a_pydaw_data->track_pool[f_track_num]->name, f_val_arr->array[1]);
                break;
            case 2:
                strcpy(a_pydaw_data->audio_track_pool[f_track_num]->name, f_val_arr->array[1]);
                break;
            default:
                assert(0);
                break;
        }
        
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
        }   
        else
        {
            a_pydaw_data->record_armed_track = 0;
            a_pydaw_data->record_armed_track_index_all = -1;
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
        
        v_pydaw_offline_render(a_pydaw_data, f_start_region, f_start_bar, f_end_region, f_end_bar, f_file_out);
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
    else
    {
        printf("Unknown configure message key: %s, value %s\n", a_key, a_value);        
    }
}


#ifdef	__cplusplus
}
#endif

#endif	/* PYDAW_H */

