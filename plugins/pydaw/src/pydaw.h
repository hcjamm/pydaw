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
  
//Uncomment this to constantly inspect the heap for corruption and throw a SIGABRT upon detection.
//#define PYDAW_MEMCHECK
    
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
#define PYDAW_CONFIGURE_KEY_CHANGE_INSTRUMENT "ci"
#define PYDAW_CONFIGURE_KEY_SHOW_PLUGIN_UI "su"
#define PYDAW_CONFIGURE_KEY_SAVE_TRACKS "st"
#define PYDAW_CONFIGURE_KEY_REC_ARM_TRACK "tr"
#define PYDAW_CONFIGURE_KEY_SHOW_FX_UI "fx"

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
#define PYDAW_MAX_TRACK_COUNT 16
#define PYDAW_MAX_EVENT_BUFFER_SIZE 512  //This could probably be made smaller
#define PYDAW_REGION_SIZE 8
#define PYDAW_MIDI_NOTE_COUNT 128
#define PYDAW_MIDI_RECORD_BUFFER_LENGTH (PYDAW_MAX_REGION_COUNT * PYDAW_REGION_SIZE)  //recording buffer for MIDI, in bars
    
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ladspa.h>
#include "pydaw_files.h"
#include "pydaw_plugin.h"
    
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
}t_pyitem;

typedef struct st_pyregion
{
    int item_indexes[PYDAW_MAX_TRACK_COUNT][PYDAW_REGION_SIZE];  //Refers to the index of items in the master item pool 
    char * name;
    /*This flag is set to 1 if created during recording, signifying that it requires a default name to be created for it*/
    int not_yet_saved;
}t_pyregion;

typedef struct st_pysong
{
    int region_count;
    t_pyregion * regions[PYDAW_MAX_REGION_COUNT];
    char region_names[PYDAW_MAX_REGION_COUNT][LMS_TINY_STRING];
}t_pysong;

typedef struct st_pytrack
{    
    float volume;
    int solo;
    int mute;
    int rec;
    int plugin_index;
    snd_seq_event_t * event_buffer;
    int current_period_event_index;
    t_pydaw_plugin * instrument;
    t_pydaw_plugin * effect;
}t_pytrack;

typedef struct st_pydaw_data
{
    int is_initialized;
    float tempo;
    pthread_mutex_t mutex;
    t_pysong * pysong;
    t_pytrack * track_pool[PYDAW_MAX_TRACK_COUNT];
    int track_current_item_note_event_indexes[PYDAW_MAX_TRACK_COUNT];
    int track_current_item_cc_event_indexes[PYDAW_MAX_TRACK_COUNT];
    int track_current_item_pitchbend_event_indexes[PYDAW_MAX_TRACK_COUNT];
    int playback_mode;  //0 == Stop, 1 == Play, 2 == Rec
    int loop_mode;  //0 == Off, 1 == Bar, 2 == Region
    char * project_name;
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
    long note_offs[PYDAW_MAX_TRACK_COUNT][PYDAW_MIDI_NOTE_COUNT];  //When a note_on event is fired, a sample number of when to release it is stored here
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
}t_pydaw_data;

void g_pysong_get(t_pydaw_data*, const char*);
t_pytrack * g_pytrack_get();
t_pyregion * g_pyregion_get(t_pydaw_data* a_pydaw, const char*);
void g_pyitem_get(t_pydaw_data* a_pydaw, const char * a_name);
t_pycc * g_pycc_get(char a_cc_num, char a_cc_val, float a_start);
t_pypitchbend * g_pypitchbend_get(float a_start, int a_value);
t_pynote * g_pynote_get(char a_note, char a_vel, float a_start, float a_length);
t_pydaw_data * g_pydaw_data_get(float);
int i_get_region_index_from_name(t_pydaw_data * a_pydaw_data, const char * a_name);
void v_open_project(t_pydaw_data*, char*, char*);
void v_set_tempo(t_pydaw_data*,float);
void v_set_playback_cursor(t_pydaw_data * a_pydaw_data, int a_region, int a_bar);
void v_pydaw_parse_configure_message(t_pydaw_data*, const char*, const char*);
int i_pydaw_get_item_index_from_name(t_pydaw_data * a_pydaw_data, const char* a_name);
void v_set_plugin_index(t_pydaw_data * a_pydaw_data, int a_track_num, int a_index);
void v_pydaw_assert_memory_integrity(t_pydaw_data* a_pydaw_data);
int i_get_song_index_from_region_name(t_pydaw_data* a_pydaw_data, const char * a_region_name);
void v_save_pysong_to_disk(t_pydaw_data * a_pydaw_data);
void v_save_pyitem_to_disk(t_pydaw_data * a_pydaw_data, int a_index);
void v_save_pyregion_to_disk(t_pydaw_data * a_pydaw_data, int a_region_num);
void v_pydaw_save_plugin(t_pydaw_data * a_pydaw_data, int a_track_num, int a_is_fx);
void v_pydaw_open_plugin(t_pydaw_data * a_pydaw_data, int a_track_num, int a_is_fx);
int g_pyitem_get_new(t_pydaw_data* a_pydaw_data);
t_pyregion * g_pyregion_get_new(t_pydaw_data* a_pydaw_data);

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

t_pypitchbend * g_pypitchbend_get(float a_start, int a_value)
{
    t_pypitchbend * f_result = (t_pypitchbend*)malloc(sizeof(t_pypitchbend));
    
    f_result->start = a_start;
    f_result->val = a_value;
    
    return f_result;
}

void g_pysong_get(t_pydaw_data* a_pydaw, const char * a_name)
{    
    if(a_pydaw->pysong)
    {
        free(a_pydaw->pysong);
    }
    
    a_pydaw->pysong = (t_pysong*)malloc(sizeof(t_pysong));
    
    a_pydaw->pysong->region_count = 0;
    
    char f_full_path[2048];
    sprintf(f_full_path, "%s%s.pysong", a_pydaw->project_folder, a_name);
            
    t_2d_char_array * f_current_string = g_get_2d_array_from_file(f_full_path, LMS_LARGE_STRING);    
    
    int f_i = 0;
    
    while(f_i < PYDAW_MAX_REGION_COUNT)
    {
        a_pydaw->pysong->regions[f_i] = 0;
        sprintf(a_pydaw->pysong->region_names[f_i], "__no__region__");
        f_i++;
    }
#ifdef PYDAW_MEMCHECK
    v_pydaw_assert_memory_integrity(a_pydaw);
#endif
    f_i = 0;
    
    while(f_i < PYDAW_MAX_REGION_COUNT)
    {            
        char * f_pos_char = c_iterate_2d_char_array(f_current_string);
        if(f_current_string->eof)
        {
            break;
        }
        int f_pos = atoi(f_pos_char);        
        char * f_region_char = c_iterate_2d_char_array(f_current_string);
        a_pydaw->pysong->regions[f_pos] = g_pyregion_get(a_pydaw, f_region_char);
        strcpy(a_pydaw->pysong->region_names[f_pos], f_region_char);
        free(f_pos_char);
        free(f_region_char);
        f_i++;
    }

    g_free_2d_char_array(f_current_string);
    
#ifdef PYDAW_MEMCHECK
    v_pydaw_assert_memory_integrity(a_pydaw);
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
        if(!strcmp(a_region_name, a_pydaw_data->pysong->region_names[f_i]))
        {
            return f_i;
        }
        f_i++;
    }
    
    return -1;
}

/*For getting a new empty region during recording*/
t_pyregion *  g_pyregion_get_new(t_pydaw_data* a_pydaw_data)
{
    t_pyregion * f_result = (t_pyregion*)malloc(sizeof(t_pyregion));    
    
    f_result->not_yet_saved = 1;
    f_result->name = NULL;
    
    int f_i = 0;
    int f_i2 = 0;
    
    while(f_i < PYDAW_MAX_TRACK_COUNT)
    {
        f_i2 = 0;
        while(f_i2 < PYDAW_REGION_SIZE)
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
    
    f_result->name = (char*)malloc(sizeof(char) * LMS_TINY_STRING);
    strcpy(f_result->name, a_name);
    f_result->not_yet_saved = 0;
    
    int f_i = 0;
    int f_i2 = 0;
    
    while(f_i < PYDAW_MAX_TRACK_COUNT)
    {
        f_i2 = 0;
        while(f_i2 < PYDAW_REGION_SIZE)
        {
            f_result->item_indexes[f_i][f_i2] = -1;
            f_i2++;
        }
        f_i++;
    }
    
    
    char f_full_path[LMS_TINY_STRING];
    sprintf(f_full_path, "%s%s.pyreg", a_pydaw_data->region_folder, a_name);    
    
    t_2d_char_array * f_current_string = g_get_2d_array_from_file(f_full_path, LMS_LARGE_STRING);
    
    while(f_i < 128)
    {   
        char * f_y_char = c_iterate_2d_char_array(f_current_string);
        if(f_current_string->eof)
        {
            free(f_y_char);
            break;
        }
        int f_y = atoi(f_y_char);
        free(f_y_char);
        
        char * f_x_char = c_iterate_2d_char_array(f_current_string);        
        int f_x = atoi(f_x_char);
        free(f_x_char);
        
        char * f_item_name = c_iterate_2d_char_array(f_current_string);
        assert(f_y < PYDAW_MAX_TRACK_COUNT);
        assert(f_x < PYDAW_REGION_SIZE);
        f_result->item_indexes[f_y][f_x] = i_pydaw_get_item_index_from_name(a_pydaw_data, f_item_name);
        assert((f_result->item_indexes[f_y][f_x]) != -1);
        assert((f_result->item_indexes[f_y][f_x]) < a_pydaw_data->item_count);
        //f_result->item_populated[f_y][f_x] = 1;
        //sprintf(log_buff, "f_x == %i, f_y = %i\n", f_x, f_y);
        //pydaw_write_log(log_buff);
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
    
    while(f_i < PYDAW_MAX_TRACK_COUNT)
    {
        f_i2 = 0;
        while(f_i2 < PYDAW_REGION_SIZE)
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
        printf("");
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
        else
        {
            printf("Invalid event type %s\n", f_type);
            //sprintf(log_buff, "Invalid event type %s\n", f_type);
            //pydaw_write_log(log_buff);
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
    
    //v_pydaw_assert_memory_integrity(a_pydaw_data);
}

t_pytrack * g_pytrack_get()
{
    t_pytrack * f_result = (t_pytrack*)malloc(sizeof(t_pytrack));
    
    f_result->mute = 0;
    f_result->solo = 0;
    f_result->volume = 0.0f;
    f_result->plugin_index = 0;
    f_result->event_buffer = (snd_seq_event_t*)malloc(sizeof(snd_seq_event_t) * PYDAW_MAX_EVENT_BUFFER_SIZE);
    f_result->rec = 0;
            
    int f_i = 0;
    
    while(f_i < PYDAW_MAX_EVENT_BUFFER_SIZE)    
    {
        snd_seq_ev_clear(&f_result->event_buffer[f_i]);
        f_i++;
    }
    
    f_result->instrument = NULL;
    f_result->effect = NULL;
            
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
    
    //f_result->mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_init(&f_result->mutex, NULL);
    f_result->is_initialized = 0;
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
    f_result->project_name = (char*)malloc(sizeof(char) * 256);
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
    
    int f_i = 0;
    
    while(f_i < PYDAW_MAX_TRACK_COUNT)
    {
        f_result->track_pool[f_i] = g_pytrack_get();
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
        
    //v_pydaw_assert_memory_integrity(f_result);
    
    return f_result;
}

void v_pydaw_activate_osc_thread(t_pydaw_data * a_pydaw_data, lo_method_handler osc_message_handler)
{
    lo_server_thread_add_method(a_pydaw_data->serverThread, NULL, NULL, osc_message_handler, NULL);
    lo_server_thread_start(a_pydaw_data->serverThread);
}

void v_pydaw_open_track(t_pydaw_data * a_pydaw_data, int a_track_num)
{
    v_pydaw_open_plugin(a_pydaw_data, a_track_num, 0);
    v_pydaw_open_plugin(a_pydaw_data, a_track_num, 1);
    
#ifdef PYDAW_MEMCHECK
    v_pydaw_assert_memory_integrity(a_pydaw_data);
#endif
}

void v_pydaw_open_plugin(t_pydaw_data * a_pydaw_data, int a_track_num, int a_is_fx)
{
    char f_file_name[512];

    if(a_is_fx)
    {
        sprintf(f_file_name, "%s%i.pyfx", a_pydaw_data->instruments_folder, a_track_num);
    }
    else
    {
        sprintf(f_file_name, "%s%i.pyinst", a_pydaw_data->instruments_folder, a_track_num);
    }
    
    if(i_pydaw_file_exists(f_file_name))
    {
        printf("v_pydaw_open_track:  Track exists %s , loading\n", f_file_name);

        t_2d_char_array * f_2d_array = g_get_2d_array_from_file(f_file_name, LMS_LARGE_STRING);

        t_pydaw_plugin * f_instance;
        
        if(a_is_fx)
        {
            f_instance = a_pydaw_data->track_pool[a_track_num]->effect;
        }
        else
        {
            f_instance = a_pydaw_data->track_pool[a_track_num]->instrument;
        }
        
        while(1)
        {
            char * f_key = c_iterate_2d_char_array(f_2d_array);
            
            if(f_2d_array->eof)
            {
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
                                
                char * message = f_instance->descriptor->configure(
                        f_instance->ladspa_handle, "load", f_value);
                if (message) 
                {
                    printf("v_pydaw_open_track: on configure '%s' '%s', plugin returned error '%s'\n","load", f_value, message);
                    free(message);
                }
                
                if(f_instance->uiTarget)
                {
                        lo_send(f_instance->uiTarget, 
                                f_instance->ui_osc_configure_path, "ss", "load", f_value);
                }
            }
            else
            {
                int f_port_key = atoi(f_key);
                float f_port_value = atof(f_value);
                
                //assert(f_port_key < (f_instance->controlIns));                
                //f_instance->pluginControlIns[f_port_key] = f_port_value;
                
                f_instance->pluginControlIns[f_port_key] = f_port_value;
            }                
        }

        g_free_2d_char_array(f_2d_array);
    }
}

void v_pydaw_open_tracks(t_pydaw_data * a_pydaw_data)
{
    int f_i = 0;
    
    while(f_i < PYDAW_MAX_TRACK_COUNT)
    {
        v_pydaw_open_track(a_pydaw_data, f_i);
        f_i++;
    }
    
#ifdef PYDAW_MEMCHECK
    v_pydaw_assert_memory_integrity(a_pydaw_data);
#endif
}

void v_open_project(t_pydaw_data* a_pydaw, char* a_project_folder, char* a_name)
{
    pthread_mutex_lock(&a_pydaw->mutex);
    a_pydaw->is_initialized = 1;
    pthread_mutex_unlock(&a_pydaw->mutex);
    
    //char log_buff[200];
    //sprintf(log_buff, "\nv_open_project: a_project_folder: %s a_name: \"%s\"\n", a_project_folder, a_name);
    //pydaw_write_log(log_buff);
    sprintf(a_pydaw->project_folder, "%s/", a_project_folder);    
    sprintf(a_pydaw->item_folder, "%sitems/", a_pydaw->project_folder);    
    //sprintf(log_buff, "\na_pydaw->item_folder == %s\n", a_pydaw->item_folder);
    //pydaw_write_log(log_buff);
    sprintf(a_pydaw->region_folder, "%sregions/", a_pydaw->project_folder);    
    //sprintf(log_buff, "\na_pydaw->region_folder == %s\n\n", a_pydaw->region_folder);
    //pydaw_write_log(log_buff);
    sprintf(a_pydaw->instruments_folder, "%sinstruments/", a_pydaw->project_folder);    
    //sprintf(log_buff, "\na_pydaw->instruments_folder == %s\n\n", a_pydaw->instruments_folder);
    //pydaw_write_log(log_buff);
    
    strcpy(a_pydaw->project_name, a_name);
    
    t_dir_list * f_item_dir_list = g_get_dir_list(a_pydaw->item_folder);
    
    int f_i = 0;
    
    while(f_i < f_item_dir_list->dir_count)
    {
        t_1d_char_array * f_file_name = c_split_str(f_item_dir_list->dir_list[f_i], '.', 2, LMS_SMALL_STRING);
        g_pyitem_get(a_pydaw, f_file_name->array[0]);
        g_free_1d_char_array(f_file_name);
        f_i++;
    }    
    
    g_pysong_get(a_pydaw, a_name);
        
    pthread_mutex_lock(&a_pydaw->mutex);
    a_pydaw->is_initialized = 1;
    
#ifdef PYDAW_MEMCHECK
    v_pydaw_assert_memory_integrity(a_pydaw);
#endif
    
    pthread_mutex_unlock(&a_pydaw->mutex);
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
            pthread_mutex_lock(&a_pydaw_data->mutex);
            a_pydaw_data->playback_mode = a_mode;
            pthread_mutex_unlock(&a_pydaw_data->mutex);
            if(f_was_recording)  //Things must be saved in the order of:  items|regions|song, otherwise it will SEGFAULT from not having a name yet...
            {
                int f_i;
                
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
            //Initiate some sort of mixer fadeout?
        }
            break;
        case 1:  //play
            pthread_mutex_lock(&a_pydaw_data->mutex);
            a_pydaw_data->playback_mode = a_mode;
            v_set_playback_cursor(a_pydaw_data, a_region, a_bar);
            pthread_mutex_unlock(&a_pydaw_data->mutex);
            break;
        case 2:  //record
            if(a_pydaw_data->playback_mode == PYDAW_PLAYBACK_MODE_REC)
            {
                return;  
            }            
                        
            pthread_mutex_lock(&a_pydaw_data->mutex);
            a_pydaw_data->recording_first_item = -1;
            a_pydaw_data->recorded_note_current_beat = 0;
            a_pydaw_data->playback_mode = a_mode;
            a_pydaw_data->recording_in_current_bar = 0;
            a_pydaw_data->recording_current_item_pool_index = -1;
            v_set_playback_cursor(a_pydaw_data, a_region, a_bar);
            pthread_mutex_unlock(&a_pydaw_data->mutex);
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
    
    while(f_i < PYDAW_MAX_TRACK_COUNT)
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
    pthread_mutex_lock(&a_pydaw_data->mutex);
    a_pydaw_data->loop_mode = a_mode;
    
#ifdef PYDAW_MEMCHECK
    v_pydaw_assert_memory_integrity(a_pydaw_data);
#endif
    
    pthread_mutex_unlock(&a_pydaw_data->mutex);
}

void v_set_tempo(t_pydaw_data * a_pydaw_data, float a_tempo)
{
    pthread_mutex_lock(&a_pydaw_data->mutex);
    //char log_buff[200];
    a_pydaw_data->tempo = a_tempo;
    a_pydaw_data->playback_inc = ( (1.0f/(a_pydaw_data->sample_rate)) / (60.0f/(a_tempo * 0.25f)) );
    a_pydaw_data->samples_per_beat = (a_pydaw_data->sample_rate)/(a_tempo/60.0f);

#ifdef PYDAW_MEMCHECK
    v_pydaw_assert_memory_integrity(a_pydaw_data);
#endif
    
    pthread_mutex_unlock(&a_pydaw_data->mutex);
}

void v_pydaw_save_track(t_pydaw_data * a_pydaw_data, int a_track_num)
{
    if(a_pydaw_data->track_pool[a_track_num]->plugin_index == 0)
    {
        return;  //Delete the file if exists?
    }

    v_pydaw_save_plugin(a_pydaw_data, a_track_num, 0);
    v_pydaw_save_plugin(a_pydaw_data, a_track_num, 1);
    
#ifdef PYDAW_MEMCHECK
    v_pydaw_assert_memory_integrity(a_pydaw_data);
#endif
}

void v_pydaw_save_plugin(t_pydaw_data * a_pydaw_data, int a_track_num, int a_is_fx)
{    
    char f_string[LMS_LARGE_STRING];
    f_string[0] = '\0';
    //sprintf(f_string, "");
    
    t_pydaw_plugin * f_instance;

    if(a_is_fx)
    {
        f_instance = a_pydaw_data->track_pool[a_track_num]->effect;
    }
    else
    {
        f_instance = a_pydaw_data->track_pool[a_track_num]->instrument;
    }

    if((a_is_fx == 0) && (a_pydaw_data->track_pool[a_track_num]->plugin_index == 1))
    {
        if(a_pydaw_data->track_pool[a_track_num]->instrument->euphoria_load_set)
        {
            char f_load[8192];
            char f_temp[8192];
            strcpy(f_temp, a_pydaw_data->track_pool[a_track_num]->instrument->euphoria_load);
            int f_i = 0;
            while(1)
            {
                if(f_temp[f_i] == '\0')
                {
                    break;
                }
                else if(f_temp[f_i] == '|')
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
        sprintf(f_file_name, "%s%i.pyfx", a_pydaw_data->instruments_folder, a_track_num);
    }
    else
    {
        sprintf(f_file_name, "%s%i.pyinst", a_pydaw_data->instruments_folder, a_track_num);
    }
    

    v_pydaw_write_to_file(f_file_name, f_string);
}

void v_pydaw_save_tracks(t_pydaw_data * a_pydaw_data)
{
    //TODO:  mutex lock, or engineer some other way to be safe about it
    int f_i = 0;
    
    while(f_i < PYDAW_MAX_TRACK_COUNT)
    {
        v_pydaw_save_track(a_pydaw_data, f_i);
        f_i++;
    }
    
#ifdef PYDAW_MEMCHECK
    v_pydaw_assert_memory_integrity(a_pydaw_data);
#endif
}

void v_show_plugin_ui(t_pydaw_data * a_pydaw_data, int a_track_num, int a_is_fx)
{   
    if(a_is_fx)
    {
        if(a_pydaw_data->track_pool[a_track_num]->effect->ui_visible)
        {
            return;
        }
        a_pydaw_data->track_pool[a_track_num]->effect->ui_visible = 1;
    }
    else
    {
        if(a_pydaw_data->track_pool[a_track_num]->instrument->ui_visible)
        {
            return;
        }
        a_pydaw_data->track_pool[a_track_num]->instrument->ui_visible = 1;
    }
    
    
    char * filename;
    char oscUrl[256];    
    char * dllName;
    char * label;
     
    
    if(a_is_fx)
    {
        filename = "/usr/lib/dssi/lms_modulex/LMS_MODULEX_qt";
        dllName = "lms_modulex.so";
        label = "LMS_MODULEX";            
    }
    else
    {
        switch(a_pydaw_data->track_pool[a_track_num]->plugin_index)
        {
            case 1:
                filename = "/usr/lib/dssi/euphoria/LMS_EUPHORIA_qt";
                dllName = "euphoria.so";
                label = "LMS_EUPHORIA";            
                break;
            case 2:
                filename = "/usr/lib/dssi/ray_v/LMS_RAYV_qt";
                dllName = "ray_v.so";
                label = "LMS_RAYV";            
                break;
        }

    }
    
    char track_number_string[6];
    sprintf(track_number_string, "%i", a_track_num);
    
    if(a_is_fx)
    {
        sprintf(oscUrl, "%s/%i-fx", a_pydaw_data->osc_url, a_track_num);
    }
    else
    {
        sprintf(oscUrl, "%s/%i", a_pydaw_data->osc_url, a_track_num);
    }
        
#ifdef PYDAW_MEMCHECK
    v_pydaw_assert_memory_integrity(a_pydaw_data);
#endif
    
    if (fork() == 0) 
    {
        execlp(filename, filename, oscUrl, dllName, label, track_number_string, NULL);
        perror("exec failed");
        exit(1);  //TODO:  should be getting rid of this???
    }    
}

//TODO:  A 'close all plugins' function that actually frees the plugin memory
void v_pydaw_close_all_uis(t_pydaw_data * a_pydaw_data)
{
    int f_i = 0;
    
    while(f_i < PYDAW_MAX_TRACK_COUNT)
    {
        if((a_pydaw_data->track_pool[f_i]->instrument) && a_pydaw_data->track_pool[f_i]->instrument->ui_visible)
        {
            lo_send(a_pydaw_data->track_pool[f_i]->instrument->uiTarget, 
                a_pydaw_data->track_pool[f_i]->instrument->ui_osc_configure_path, "ss", "pydaw_close_window", "");
        }
        
        if((a_pydaw_data->track_pool[f_i]->effect) && a_pydaw_data->track_pool[f_i]->effect->ui_visible)
        {
            lo_send(a_pydaw_data->track_pool[f_i]->effect->uiTarget, 
                a_pydaw_data->track_pool[f_i]->effect->ui_osc_configure_path, "ss", "pydaw_close_window", "");
        }
        
        f_i++;
    }
}

void v_set_plugin_index(t_pydaw_data * a_pydaw_data, int a_track_num, int a_index)
{       
    t_pydaw_plugin * f_result;
    t_pydaw_plugin * f_result_fx;
    
    char f_file_name[512];
    sprintf(f_file_name, "%s%i.pyinst", a_pydaw_data->instruments_folder, a_track_num);
    
    char f_file_name_fx[512];
    sprintf(f_file_name_fx, "%s%i.pyfx", a_pydaw_data->instruments_folder, a_track_num);
    
    if(a_index == 0)
    {
        pthread_mutex_lock(&a_pydaw_data->mutex);
                
        if(i_pydaw_file_exists(f_file_name))
        {
            remove(f_file_name);
        }

        if(a_pydaw_data->track_pool[a_track_num]->instrument)
        {
            v_free_pydaw_plugin(a_pydaw_data->track_pool[a_index]->instrument);
        }
        
        if(a_pydaw_data->track_pool[a_track_num]->effect)
        {
            v_free_pydaw_plugin(a_pydaw_data->track_pool[a_index]->effect);
        }        
        
        a_pydaw_data->track_pool[a_track_num]->instrument = NULL;
        a_pydaw_data->track_pool[a_track_num]->effect = NULL;
        a_pydaw_data->track_pool[a_track_num]->plugin_index = a_index;
        
#ifdef PYDAW_MEMCHECK
        v_pydaw_assert_memory_integrity(a_pydaw_data);
#endif
        pthread_mutex_unlock(&a_pydaw_data->mutex);    
    }
    else
    {
        f_result = g_pydaw_plugin_get((int)(a_pydaw_data->sample_rate), a_index);
        f_result_fx = g_pydaw_plugin_get((int)(a_pydaw_data->sample_rate), -1);
                
        /* If switching from a different plugin(but not from no plugin), delete the preset file */
        if(((a_pydaw_data->track_pool[a_track_num]->plugin_index) != 0) && i_pydaw_file_exists(f_file_name))
        {
            remove(f_file_name);
            //TODO:  the .pyfx file also?
        }
        
        pthread_mutex_lock(&a_pydaw_data->mutex);
        //TODO:  Try aliasing the pointer, and then locking the mutex only to assign it and freeing the old pointer later
        if(a_pydaw_data->track_pool[a_track_num]->instrument)
        {        
            if(a_pydaw_data->track_pool[a_track_num]->instrument->ui_visible)
            {
                lo_send(a_pydaw_data->track_pool[a_track_num]->instrument->uiTarget, 
                        a_pydaw_data->track_pool[a_track_num]->instrument->ui_osc_configure_path, "ss", "pydaw_close_window", "");
            }

            v_free_pydaw_plugin(a_pydaw_data->track_pool[a_track_num]->instrument);
        }
        
        if(a_pydaw_data->track_pool[a_track_num]->effect)
        {        
            if(a_pydaw_data->track_pool[a_track_num]->effect->ui_visible)
            {
                lo_send(a_pydaw_data->track_pool[a_track_num]->effect->uiTarget, 
                        a_pydaw_data->track_pool[a_track_num]->effect->ui_osc_configure_path, "ss", "pydaw_close_window", "");
            }

            v_free_pydaw_plugin(a_pydaw_data->track_pool[a_track_num]->effect);
        }
        
        a_pydaw_data->track_pool[a_track_num]->instrument = f_result;
        a_pydaw_data->track_pool[a_track_num]->effect = f_result_fx;
        a_pydaw_data->track_pool[a_track_num]->plugin_index = a_index;
        v_pydaw_open_track(a_pydaw_data, a_track_num);  //Opens the .inst file if exists
        
#ifdef PYDAW_MEMCHECK
        v_pydaw_assert_memory_integrity(a_pydaw_data);
#endif
        pthread_mutex_unlock(&a_pydaw_data->mutex);
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
    
    if(a_pydaw_data->pysong)
    {
        while(f_i3 < PYDAW_MAX_REGION_COUNT)
        {
            if(a_pydaw_data->pysong->regions[f_i3])
            {
                f_i = 0;
                while(f_i < PYDAW_REGION_SIZE)
                {
                    f_i2 = 0;
                    while(f_i2 < PYDAW_MAX_TRACK_COUNT)
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
        
        pthread_mutex_lock(&a_pydaw_data->mutex);
        if(a_pydaw_data->pysong->regions[f_region_index])
        {
            free(a_pydaw_data->pysong->regions[f_region_index]);
        }
        a_pydaw_data->pysong->regions[f_region_index] = f_result;
        pthread_mutex_unlock(&a_pydaw_data->mutex);        
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_SI)) //Save Item
    {
        g_pyitem_get(a_pydaw_data, a_value);
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_SS))  //Save Song
    {
        g_pysong_get(a_pydaw_data, a_value);
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
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_LOOP)) //Set loop mode
    {
        int f_value = atoi(a_value);
        v_set_loop_mode(a_pydaw_data, f_value);
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_OS)) //Open Song
    {
        t_1d_char_array * f_arr = c_split_str(a_value, '|', 2, LMS_SMALL_STRING);
        v_open_project(a_pydaw_data, f_arr->array[0], f_arr->array[1]);
        g_free_1d_char_array(f_arr);
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_TEMPO)) //Change tempo
    {
        v_set_tempo(a_pydaw_data, atof(a_value));
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_SHOW_PLUGIN_UI))
    {
        int f_track_num = atoi(a_value);
        v_show_plugin_ui(a_pydaw_data, f_track_num, 0);
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_SHOW_FX_UI))
    {
        int f_track_num = atoi(a_value);
        v_show_plugin_ui(a_pydaw_data, f_track_num, 1);
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
        a_pydaw_data->track_pool[f_track_num]->solo = f_mode;
        g_free_1d_char_array(f_val_arr);        
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_MUTE)) //Set track mute
    {
        t_1d_char_array * f_val_arr = c_split_str(a_value, '|', 2, LMS_TINY_STRING);
        int f_track_num = atoi(f_val_arr->array[0]);
        int f_mode = atoi(f_val_arr->array[1]);
        a_pydaw_data->track_pool[f_track_num]->mute = f_mode;
        g_free_1d_char_array(f_val_arr);
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_REC_ARM_TRACK)) //Set track record arm
    {
        t_1d_char_array * f_val_arr = c_split_str(a_value, '|', 2, LMS_TINY_STRING);
        int f_track_num = atoi(f_val_arr->array[0]);
        int f_mode = atoi(f_val_arr->array[1]);
        if(f_mode)
        {
            //TODO:  This will need to be removed if PyDAW ever supports multiple MIDI input devices, just a quick hack for now
            int f_i = 0;            
            while(f_i < PYDAW_MAX_TRACK_COUNT)
            {
                a_pydaw_data->track_pool[f_track_num]->rec = 0;
                f_i++;
            }
        }
        a_pydaw_data->track_pool[f_track_num]->rec = f_mode;
        g_free_1d_char_array(f_val_arr);
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_VOL)) //Set track volume
    {
        t_1d_char_array * f_val_arr = c_split_str(a_value, '|', 2, LMS_TINY_STRING);
        int f_track_num = atoi(f_val_arr->array[0]);
        float f_track_vol = atof(f_val_arr->array[1]);
        a_pydaw_data->track_pool[f_track_num]->volume = f_track_vol;
        g_free_1d_char_array(f_val_arr);
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_CHANGE_INSTRUMENT)) //Change the plugin
    {
        t_1d_char_array * f_val_arr = c_split_str(a_value, '|', 2, LMS_TINY_STRING);
        int f_track_num = atoi(f_val_arr->array[0]);
        int f_plugin_index = atof(f_val_arr->array[1]);
        
        v_set_plugin_index(a_pydaw_data, f_track_num, f_plugin_index);
        
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

