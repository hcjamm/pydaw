/* 
 * File:   pydaw.h
 * Author: jeffh
 *
 * Types and functions specific the the IO and file system specs of PyDAW 
 */

/* Current TODO:
 * 
 * A type + functions that track notes currently on, so that "stop" configure message can send all_note_off events...  
 * Mutex functionality similar to how Euphoria sample loading works
 * A track type to encapsulate various track info...
 * Use the format of midi_callback() from jack-dssi-host.c to synthesize and write events to the MIDI out ports
 * 
 */

#ifndef PYDAW_H
#define	PYDAW_H

#ifdef	__cplusplus
extern "C" {
#endif
    
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
#define PYDAW_CONFIGURE_KEY_TSIG "tsig"
#define PYDAW_CONFIGURE_KEY_VOL "vol"
#define PYDAW_CONFIGURE_KEY_SOLO "solo"
#define PYDAW_CONFIGURE_KEY_MUTE "mute"

#define PYDAW_LOOP_MODE_OFF 0
#define PYDAW_LOOP_MODE_BAR 1
#define PYDAW_LOOP_MODE_REGION 2
    
//arbitrary, I may change these 3 after evaluating memory use vs. probable item count in a real project
#define PYDAW_MAX_ITEM_COUNT 5000
#define PYDAW_MAX_REGION_COUNT 300
#define PYDAW_MAX_EVENTS_PER_ITEM_COUNT 128
#define PYDAW_MAX_TRACK_COUNT 16
#define PYDAW_REGION_SIZE 8
    
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include "pydaw_files.h"
    
typedef struct st_pynote
{
    //TODO:  It may be more efficient to process as Int?
    char note;
    char velocity;
    float start;
    float length;    
}t_pynote;

typedef struct st_pycc
{
    char cc_num;
    char cc_val;
    float start;
}t_pycc;

typedef struct st_pyitem
{
    char * name;
    t_pynote * notes[PYDAW_MAX_EVENTS_PER_ITEM_COUNT];
    int note_count;    
    t_pycc * ccs[PYDAW_MAX_EVENTS_PER_ITEM_COUNT];
    int cc_count;
    int resize_factor;
    int note_index[PYDAW_MAX_EVENTS_PER_ITEM_COUNT];
    int cc_index[PYDAW_MAX_EVENTS_PER_ITEM_COUNT];
}t_pyitem;

typedef struct st_pyregion
{
    int items[PYDAW_MAX_TRACK_COUNT][PYDAW_REGION_SIZE];  //Refers to the index of items in the master item pool
    int row_count;
    int column_count;
    char * name;
}t_pyregion;

typedef struct st_pysong
{
    int region_count;
    int region_index[PYDAW_MAX_REGION_COUNT];
    int max_regions;
}t_pysong;

typedef struct st_pytrack
{
    float volume;
    int solo;
    int mute;    
}t_pytrack;

typedef struct st_pydaw_data
{
    float tempo;
    pthread_mutex_t mutex;
    t_pysong * pysong;
    t_pyitem * item_pool[PYDAW_MAX_ITEM_COUNT];
    t_pyregion * region_pool[PYDAW_MAX_REGION_COUNT];
    t_pytrack * track_pool[PYDAW_MAX_TRACK_COUNT];
    int item_count;
    int region_count;    
    int playback_mode;  //0 == Stop, 1 == Play, 2 == Rec
    int loop_mode;  //0 == Off, 1 == Bar, 2 == Region
    char * project_name;
    char * project_folder;
    char * item_folder;
    char * region_folder;    
    float playback_cursor; //only refers to the fractional position within the current bar.    
    float playback_cursor_last; //only refers to the fractional position within the current bar.    
    float playback_inc;  //the increment per-period to iterate through 1 bar, as determined by sample rate and tempo
    int current_region; //the current region
    int current_bar; //the current bar(0 to 7), within the current region
    int current_bar_start;  //The current bar start in samples
    int current_bar_end;  //The current bar end in samples
    int period_size;  //The size of the soundcards sample buffer, ie:  512 samples, 256 samples, etc...
    int samples_per_bar;
    float sample_rate;
    int current_sample;  //The sample number of the exact point in the song, 0 == bar0/region0, 44100 == 1 second in at 44.1khz.  Reset to zero on beginning playback    
    snd_seq_t *seq_handle;    
    
    int queue_id, port_in_id[PYDAW_MAX_TRACK_COUNT], port_out_id[PYDAW_MAX_TRACK_COUNT];    
    snd_seq_tick_time_t tick[PYDAW_MAX_TRACK_COUNT];
    
}t_pydaw_data;

void g_pysong_get(t_pydaw_data*, const char*);
t_pytrack * g_pytrack_get();
void g_pyregion_get(t_pydaw_data* a_pydaw, const char*);
void g_pyitem_get(t_pydaw_data* a_pydaw, const char * a_name);
t_pycc * g_pycc_get(char a_cc_num, char a_cc_val, float a_start);
t_pynote * g_pynote_get(char a_note, char a_vel, float a_start, float a_length);
t_pydaw_data * g_pydaw_data_get(float);
int i_get_item_index_from_name(t_pydaw_data * a_pydaw_data, const char * a_name);
int i_get_region_index_from_name(t_pydaw_data * a_pydaw_data, const char * a_name);
void v_open_project(t_pydaw_data*, char*, char*);
void v_set_tempo(t_pydaw_data*,float);
void v_set_playback_cursor(t_pydaw_data * a_pydaw_data, int a_region, int a_bar);

/*End declarations.  Begin implementations.*/

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

void g_pysong_get(t_pydaw_data* a_pydaw, const char * a_name)
{
    printf("\ng_pysong_get\n");
    t_pysong * f_result = (t_pysong*)malloc(sizeof(t_pysong));
    
    f_result->region_count = 0;
    f_result->max_regions = PYDAW_MAX_REGION_COUNT;
    
    char * f_full_path = (char*)malloc(sizeof(char) * 256);
    strcpy(f_full_path, a_pydaw->project_folder);
    strcat(f_full_path, a_name);
    strcat(f_full_path, ".pysong");
        
    t_2d_char_array * f_current_string = g_get_2d_array_from_file(f_full_path, LMS_LARGE_STRING);    
    free(f_full_path);
    
    int f_i = 0;
    
    while(f_i < 128)
    {            
        char * f_pos_char = c_iterate_2d_char_array(f_current_string);
        if(f_current_string->eof)
        {
            break;
        }
        int f_pos = atoi(f_pos_char);        
        char * f_region_char = c_iterate_2d_char_array(f_current_string);
        int f_region_index = i_get_region_index_from_name(a_pydaw, f_region_char);
        f_result->region_index[f_pos] = f_region_index;
        free(f_pos_char);
        free(f_region_char);
        f_i++;
    }

    g_free_2d_char_array(f_current_string);
    
    if(a_pydaw->pysong)
    {
        free(a_pydaw->pysong);
    }
    
    a_pydaw->pysong = f_result;
}

void g_pyregion_get(t_pydaw_data* a_pydaw, const char * a_name)
{
    printf("\ng_pyregion_get: a_name: \"%s\"\n", a_name);
    t_pyregion * f_result = (t_pyregion*)malloc(sizeof(t_pyregion));    
    
    f_result->name = (char*)malloc(sizeof(char) * 64);
    strcpy(f_result->name, a_name);
    
    int f_i = 0; 
    int f_i2 = 0;
    
    while(f_i < PYDAW_MAX_TRACK_COUNT)
    {
        while(f_i2 < PYDAW_REGION_SIZE)
        {
            f_result->items[f_i][f_i2] = -1;
            f_i2++;
        }
        f_i++;
    }
    
    char * f_full_path = (char*)malloc(sizeof(char) * 256);
    strcpy(f_full_path, a_pydaw->region_folder);
    strcat(f_full_path, a_name);
    strcat(f_full_path, ".pyreg");
    
    t_2d_char_array * f_current_string = g_get_2d_array_from_file(f_full_path, LMS_LARGE_STRING);
    
    free(f_full_path);

    f_i = 0;

    while(f_i < 128)
    {            
        char * f_x_char = c_iterate_2d_char_array(f_current_string);
        if(f_current_string->eof)
        {
            break;
        }
        int f_x = atoi(f_x_char);
        free(f_x_char);
        char * f_y_char = c_iterate_2d_char_array(f_current_string);
        int f_y = atoi(f_y_char);
        free(f_y_char);
        char * f_item_name = c_iterate_2d_char_array(f_current_string);            
        f_result->items[f_x][f_y] = i_get_item_index_from_name(a_pydaw, f_item_name);            
        free(f_item_name);            

        f_i++;
    }

    g_free_2d_char_array(f_current_string);
    
    int f_current_index = i_get_region_index_from_name(a_pydaw, a_name);
    
    if(f_current_index == -1)
    {
        a_pydaw->region_pool[(a_pydaw->region_count)] = f_result;
        a_pydaw->region_count = (a_pydaw->region_count) + 1;
    }
    else
    {
        free(a_pydaw->region_pool[f_current_index]);
        a_pydaw->region_pool[f_current_index] = f_result;
    }
}

void g_pyitem_get(t_pydaw_data* a_pydaw, const char * a_name)
{
    printf("g_pyitem_get: a_name: \"%s\"\n", a_name);
    t_pyitem * f_result = (t_pyitem*)malloc(sizeof(t_pyitem));
    
    f_result->name = (char*)malloc(sizeof(char) * 64);
    strcpy(f_result->name, a_name);
    f_result->cc_count = 0;
    f_result->note_count = 0;
    
    int f_existing_item_index = i_get_item_index_from_name(a_pydaw, a_name);
    
    char * f_full_path = (char*)malloc(sizeof(char) * 256);
    strcpy(f_full_path, a_pydaw->item_folder);
    strcat(f_full_path, a_name);
    strcat(f_full_path, ".pyitem");
    
    t_2d_char_array * f_current_string = g_get_2d_array_from_file(f_full_path, LMS_LARGE_STRING);
    
    free(f_full_path);

    int f_i = 0;

    while(f_i < 256)
    {   
        char * f_type = c_iterate_2d_char_array(f_current_string);
        
        if(f_current_string->eof)
        {
            break;
        }
        
        char * f_list_pos = c_iterate_2d_char_array(f_current_string);
        
        char * f_start = c_iterate_2d_char_array(f_current_string);
        
        if(!strcmp(f_type, "n"))
        {
            char * f_length = c_iterate_2d_char_array(f_current_string);
            char * f_note_text = c_iterate_2d_char_array(f_current_string);
            free(f_note_text);  //This one is ignored
            char * f_note = c_iterate_2d_char_array(f_current_string);
            char * f_vel = c_iterate_2d_char_array(f_current_string);
                        
            f_result->notes[(f_result->note_count)] = g_pynote_get(atoi(f_note), atoi(f_vel), atof(f_start), atof(f_length));
            f_result->note_count = (f_result->note_count) + 1;
            
            free(f_length);
            free(f_note);
            free(f_vel);
        }
        else if(!strcmp(f_type, "c"))
        {
            char * f_cc_num = c_iterate_2d_char_array(f_current_string);
            char * f_cc_val = c_iterate_2d_char_array(f_current_string);
            
            f_result->ccs[(f_result->cc_count)] = g_pycc_get(atoi(f_cc_num), atoi(f_cc_val), atof(f_start));
            f_result->cc_count = (f_result->cc_count) + 1;
            
            free(f_cc_num);
            free(f_cc_val);
        }
        else
        {
            printf("Invalid event type %s\n", f_type);
        }
        free(f_list_pos);
        free(f_start);
        free(f_type);
        f_i++;
    }

    g_free_2d_char_array(f_current_string);
        
    if(f_existing_item_index == -1)
    {
        a_pydaw->item_pool[(a_pydaw->item_count)] = f_result;
        a_pydaw->item_count = (a_pydaw->item_count) + 1;
    }
    else
    {
        free(a_pydaw->item_pool[f_existing_item_index]);
        a_pydaw->item_pool[f_existing_item_index] = f_result;
    }    
}

t_pytrack * g_pytrack_get()
{
    t_pytrack * f_result = (t_pytrack*)malloc(sizeof(t_pytrack));
    
    f_result->mute = 0;
    f_result->solo = 0;
    f_result->volume = 0.0f;
    
    return f_result;
}

t_pydaw_data * g_pydaw_data_get(float a_sample_rate)
{
    t_pydaw_data * f_result = (t_pydaw_data*)malloc(sizeof(t_pydaw_data));
    
    //f_result->mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_init(&f_result->mutex, NULL);
    f_result->sample_rate = a_sample_rate;
    f_result->item_count = 0;
    f_result->region_count = 0;
    f_result->current_sample = 0;
    f_result->loop_mode = 0;    
    f_result->period_size = 512;  //Arbitrary value to avoid a SEGFAULT early on.  TODO:  Find a way to get this from ALSA
    f_result->item_folder = (char*)malloc(sizeof(char) * 256);
    f_result->project_folder = (char*)malloc(sizeof(char) * 256);
    f_result->region_folder = (char*)malloc(sizeof(char) * 256);
    f_result->project_name = (char*)malloc(sizeof(char) * 256);
    
    if (snd_seq_open(&f_result->seq_handle, "default", SND_SEQ_OPEN_OUTPUT, 0) < 0) 
    {
      fprintf(stderr, "Error opening ALSA sequencer.\n");
      exit(1);
    }
    
    int f_i = 0;
    
    while(f_i < PYDAW_MAX_TRACK_COUNT)
    {
        f_result->track_pool[f_i] = g_pytrack_get();
        
        /*ALSA stuff*/

        snd_seq_set_client_name(f_result->seq_handle, "PyDAW");
        
        char f_char_arr[12];
        sprintf(f_char_arr, "pydaw-%i", f_i);
        
        if ((port_out_id = snd_seq_create_simple_port(f_result->seq_handle, f_char_arr,
                  SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ,
                  SND_SEQ_PORT_TYPE_APPLICATION)) < 0) {
          fprintf(stderr, "Error creating sequencer port.\n");
        }
        
        /*End ALSA stuff*/
        
        f_i++;
    }
            
    return f_result;
}

//Functions adapted from miniArp.c
void g_pydaw_alsa_start(t_pydaw_data* a_pydaw_data);
void v_pydaw_clear_queue(t_pydaw_data * a_pydaw_data);
void g_pydaw_alsa_stop(t_pydaw_data* a_pydaw_data);
snd_seq_tick_time_t g_pydaw_data_get_tick();
void v_pydaw_init_queue(t_pydaw_data* a_pydaw_data);
void v_pydaw_schedule_item(t_pydaw_data * a_pydaw_data, int a_item_number, int a_track_number);
void v_pydaw_clear_queue(t_pydaw_data * a_pydaw_data);

void g_pydaw_alsa_start(t_pydaw_data* a_pydaw_data)
{
    snd_seq_start_queue(a_pydaw_data->seq_handle, queue_id, NULL);
    snd_seq_drain_output(a_pydaw_data->seq_handle);
    //npfd = snd_seq_poll_descriptors_count(seq_handle, POLLIN);
    //pfd = (struct pollfd *)alloca(npfd * sizeof(struct pollfd));
    //snd_seq_poll_descriptors(seq_handle, pfd, npfd, POLLIN);
}

void g_pydaw_alsa_stop(t_pydaw_data* a_pydaw_data)
{
    v_pydaw_clear_queue(a_pydaw_data);
    sleep(2);
    snd_seq_stop_queue(a_pydaw_data->seq_handle, a_pydaw_data->queue_id, NULL);
    snd_seq_free_queue(a_pydaw_data->seq_handle, a_pydaw_data->queue_id);
}

snd_seq_tick_time_t g_pydaw_data_get_tick(t_pydaw_data * a_pydaw_data)
{
  snd_seq_queue_status_t *status;
  snd_seq_tick_time_t current_tick;
  
  snd_seq_queue_status_malloc(&status);
  snd_seq_get_queue_status(a_pydaw_data->seq_handle, a_pydaw_data->queue_id, status);
  current_tick = snd_seq_queue_status_get_tick_time(status);
  snd_seq_queue_status_free(status);
  return(current_tick);
}

void v_pydaw_init_queue(t_pydaw_data* a_pydaw_data)
{
  a_pydaw_data->queue_id = snd_seq_alloc_queue(a_pydaw_data->seq_handle);
  snd_seq_set_client_pool_output(a_pydaw_data->seq_handle, (seq_len<<1) + 4);
} 
 
void v_pydaw_schedule_item(t_pydaw_data * a_pydaw_data, int a_item_number, int a_track_number)
{
  snd_seq_event_t ev;
  int l1;
  double dt;
 
  for (l1 = 0; l1 < (a_pydaw_data->item_pool[a_item_number]->note_count); l1++) {
    dt = (l1 % 2 == 0) ? (double)swing / 16384.0 : -(double)swing / 16384.0;
    snd_seq_ev_clear(&ev);
    snd_seq_ev_set_note(&ev, 0, sequence[2][l1] + transpose, 127, sequence[1][l1]);
    snd_seq_ev_schedule_tick(&ev, a_pydaw_data->queue_id,  0, a_pydaw_data->tick[a_track_number]);
    snd_seq_ev_set_source(&ev, a_pydaw_data->port_out_id[a_track_number]);
    snd_seq_ev_set_subs(&ev);
    snd_seq_event_output_direct(a_pydaw_data->seq_handle, &ev);
    a_pydaw_data->tick[a_track_number] += (int)((double)sequence[0][l1] * (1.0 + dt));
  }
  
  //snd_seq_ev_set_controller
  
  
  //The echo event causes it to loop, commenting out for now
  //snd_seq_ev_clear(&ev);
  //ev.type = SND_SEQ_EVENT_ECHO; 
  //snd_seq_ev_schedule_tick(&ev, a_pydaw_data->queue_id,  0, a_pydaw_data->tick[a_track_number]);
  //snd_seq_ev_set_dest(&ev, snd_seq_client_id(a_pydaw_data->seq_handle), port_in_id);
  //snd_seq_event_output_direct(a_pydaw_data->seq_handle, &ev);
}

void v_pydaw_clear_queue(t_pydaw_data * a_pydaw_data)
{
    snd_seq_remove_events_t *remove_ev;
    snd_seq_remove_events_malloc(&remove_ev);
    snd_seq_remove_events_set_queue(remove_ev, a_pydaw_data->queue_id);
    snd_seq_remove_events_set_condition(remove_ev, SND_SEQ_REMOVE_OUTPUT | SND_SEQ_REMOVE_IGNORE_OFF);
    snd_seq_remove_events(a_pydaw_data->seq_handle, remove_ev);
    snd_seq_remove_events_free(remove_ev);
}

//This will eventually get a real indexing algorithm.  Although generally it shouldn't have noticeably
//bad performance on a modern CPU because it's only called when the user does something in the UI.
int i_get_item_index_from_name(t_pydaw_data * a_pydaw_data, const char * a_name)
{
    printf("i_get_item_index_from_name: a_name: \"%s\"\n", a_name);
    int f_i = 0;
    
    while(f_i < a_pydaw_data->item_count)
    {
        //printf("Comparing %s\n", (a_pydaw_data->item_pool[f_i]->name));
        if(!strcmp((a_pydaw_data->item_pool[f_i]->name), a_name))
        {
            printf("return %i for %s\n", f_i, a_name);
            return f_i;
        }
        
        f_i++;
    }
    printf("return -1 for \"%s\"\n", a_name);
    return -1;
}

//This will eventually get a real indexing algorithm.  Although generally it shouldn't have noticeably
//bad performance on a modern CPU because it's only called when the user does something in the UI.
int i_get_region_index_from_name(t_pydaw_data * a_pydaw_data, const char * a_name)
{
    int f_i = 0;
    
    while(f_i < a_pydaw_data->region_count)
    {
        if(!strcmp((a_pydaw_data->region_pool[f_i]->name), a_name))
        {
            return f_i;
        }
        
        f_i++;
    }
    
    return -1;
}

void v_open_project(t_pydaw_data* a_pydaw, char* a_project_folder, char* a_name)
{
    printf("\nv_open_project: a_project_folder: %s a_name: \"%s\"\n", a_project_folder, a_name);
    strcpy(a_pydaw->project_folder, a_project_folder);
    strcat(a_pydaw->project_folder, "/");
    strcpy(a_pydaw->item_folder, a_pydaw->project_folder);
    strcat(a_pydaw->item_folder, "items/");
    printf("\na_pydaw->item_folder == %s\n", a_pydaw->item_folder);
    strcpy(a_pydaw->region_folder, a_pydaw->project_folder);
    strcat(a_pydaw->region_folder, "regions/");
    printf("\na_pydaw->region_folder == %s\n\n", a_pydaw->region_folder);
    strcpy(a_pydaw->project_name, a_name);    
    
    int f_i = 0;
        
    t_dir_list * f_items = g_get_dir_list(a_pydaw->item_folder);
    
    printf("\npyitem file count: %i\n", f_items->dir_count);
    
    while(f_i < (f_items->dir_count))
    {
        printf("Processing item: %s\n", f_items->dir_list[f_i]);
        t_1d_char_array * f_arr = c_split_str(f_items->dir_list[f_i], '.', 2, LMS_SMALL_STRING);
        printf("v_open_project calling ");        
        g_pyitem_get(a_pydaw, f_arr->array[0]);
        g_free_1d_char_array(f_arr);
        f_i++;
    }
    
    t_dir_list * f_regions = g_get_dir_list(a_pydaw->region_folder);
    
    printf("pyregion file count: %i\n", f_regions->dir_count);
    
    while(f_i < (f_regions->dir_count))
    {
        printf("Processing region: %s\n", f_items->dir_list[f_i]);
        t_1d_char_array * f_arr = c_split_str(f_regions->dir_list[f_i], '.', 2, LMS_SMALL_STRING);
        printf("v_open_project calling ");
        g_pyregion_get(a_pydaw, f_arr->array[0]);
        g_free_1d_char_array(f_arr);
        f_i++;
    }
    printf("v_open_project calling ");
    g_pysong_get(a_pydaw, a_name);
}

/* void v_set_playback_mode(t_pydaw_data * a_pydaw_data, 
 * int a_mode, //
 * int a_region, //The region index to start playback on
 * int a_bar) //The bar index (with a_region) to start playback on
 */
void v_set_playback_mode(t_pydaw_data * a_pydaw_data, int a_mode, int a_region, int a_bar)
{
    a_pydaw_data->playback_mode = a_mode;
    
    switch(a_mode)
    {
        case 0:  //stop
            //Initiate some sort of mixer fadeout?
            break;
        case 1:  //play
            a_pydaw_data->current_sample = 0;
            v_set_playback_cursor(a_pydaw_data, a_region, a_bar);
            break;
        case 2:  //record
            a_pydaw_data->current_sample = 0;
            v_set_playback_cursor(a_pydaw_data, a_region, a_bar);
            break;
    }
}

void v_set_playback_cursor(t_pydaw_data * a_pydaw_data, int a_region, int a_bar)
{
    a_pydaw_data->current_bar = a_bar;
    a_pydaw_data->current_region = a_region;
    a_pydaw_data->playback_cursor = 0.0f;
    //TODO:  An  "all notes off" function
}

void v_set_loop_mode(t_pydaw_data * a_pydaw_data, int a_mode)
{
    a_pydaw_data->loop_mode = a_mode;
}

void v_set_tempo(t_pydaw_data * a_pydaw_data, float a_tempo)
{
    a_pydaw_data->tempo = a_tempo;
    //This assumes 4/4 timesig, once alternate timesigs are available, replace 0.25f
    a_pydaw_data->playback_inc = ((1.0f/(a_tempo * 0.25f)) * (a_pydaw_data->sample_rate) * ((float)(a_pydaw_data->period_size)));
}

void v_pydaw_parse_configure_message(t_pydaw_data*, const char*, const char*);

void v_pydaw_parse_configure_message(t_pydaw_data* a_pydaw, const char* a_key, const char* a_value)
{
    printf("v_pydaw_parse_configure_message:  key: \"%s\", value: \"%s\"\n", a_key, a_value);
    
    if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_SR)) //Save region
    {
        printf("v_pydaw_parse_configure_message calling ");
        g_pyregion_get(a_pydaw, a_value);        
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_SI)) //Save Item
    {
        printf("v_pydaw_parse_configure_message calling ");
        g_pyitem_get(a_pydaw, a_value);
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_SS))  //Save Song
    {
        printf("v_pydaw_parse_configure_message calling ");
        g_pysong_get(a_pydaw, a_value);
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_PLAY)) //Begin playback
    {
        t_1d_char_array * f_arr = c_split_str(a_value, '|', 2, LMS_SMALL_STRING);
        int f_region = atoi(f_arr->array[0]);
        int f_bar = atoi(f_arr->array[1]);
        v_set_playback_mode(a_pydaw, 1, f_region, f_bar);
        g_free_1d_char_array(f_arr);
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_REC)) //Begin recording
    {
        t_1d_char_array * f_arr = c_split_str(a_value, '|', 2, LMS_SMALL_STRING);
        int f_region = atoi(f_arr->array[0]);
        int f_bar = atoi(f_arr->array[1]);
        v_set_playback_mode(a_pydaw, 2, f_region, f_bar);
        g_free_1d_char_array(f_arr);
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_STOP)) //Stop playback or recording
    {
        v_set_playback_mode(a_pydaw, 0, -1, -1);
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_LOOP)) //Set loop mode
    {
        int f_value = atoi(a_value);
        v_set_loop_mode(a_pydaw, f_value);
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_OS)) //Open Song
    {
        t_1d_char_array * f_arr = c_split_str(a_value, '|', 2, LMS_SMALL_STRING);
        v_open_project(a_pydaw, f_arr->array[0], f_arr->array[1]);
        g_free_1d_char_array(f_arr);
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_TEMPO)) //Change tempo
    {
        v_set_tempo(a_pydaw, atof(a_value));
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
        a_pydaw->track_pool[f_track_num]->solo = f_mode;
        g_free_1d_char_array(f_val_arr);
        printf("Solo[%i] == %i\n", f_track_num, (a_pydaw->track_pool[f_track_num]->solo));
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_MUTE)) //Set track mute
    {
        t_1d_char_array * f_val_arr = c_split_str(a_value, '|', 2, LMS_TINY_STRING);
        int f_track_num = atoi(f_val_arr->array[0]);
        int f_mode = atoi(f_val_arr->array[1]);
        a_pydaw->track_pool[f_track_num]->mute = f_mode;
        g_free_1d_char_array(f_val_arr);
        printf("Mute[%i] == %i\n", f_track_num, (a_pydaw->track_pool[f_track_num]->mute));
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_VOL)) //Set track volume
    {
        t_1d_char_array * f_val_arr = c_split_str(a_value, '|', 2, LMS_TINY_STRING);
        int f_track_num = atoi(f_val_arr->array[0]);
        float f_track_vol = atof(f_val_arr->array[1]);
        a_pydaw->track_pool[f_track_num]->volume = f_track_vol;
        g_free_1d_char_array(f_val_arr);
        printf("vol[%i] == %f\n", f_track_num, (a_pydaw->track_pool[f_track_num]->volume));
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

