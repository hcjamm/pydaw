/* 
 * File:   pydaw.h
 * Author: jeffh
 *
 * Types and functions specific the the IO and file system specs of PyDAW 
 */

/* Current TODO:
 * 
 * Item specific functions for delete and instantiate from file
 * A type + functions that track notes currently on, so that "stop" configure message can send all_note_off events...
 * A type to encapsulate the global data like tempo, and to process the standard configure messages
 * A function to load files to strings, and to split them into arrays
 * Encapsulate the native ALSA types into the PyDAW types #EDIT:  maybe not, that probably has to be calculated on the fly to support tempo changes
 * Destructor functions for freeing all memory when deleting PyDAW type instances
 * Mutex functionality similar to how Euphoria sample loading works
 * Region->Items should be pointers to a pool of items, not unique items...
 * A track type to encapsulate various track info...
 * Develop a comprehensive strategy for freeing char* types, and other memory management of things that get deleted
 * Perhaps just allocate an array of 10,000 or so items for the item pool and don't allow resizing?   TODO:  Calculate how large that would be
 */

#ifndef PYDAW_H
#define	PYDAW_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#define PYDAW_CONFIGURE_KEY_SS "ss"
#define PYDAW_CONFIGURE_KEY_NS "ns"
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
#define PYDAW_CONFIGURE_KEY_TSIG "tsig"
#define PYDAW_CONFIGURE_KEY_VOL "vol"
#define PYDAW_CONFIGURE_KEY_SOLO "solo"
#define PYDAW_CONFIGURE_KEY_MUTE "mute"
    
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
#include "pydaw_files.h"
    
typedef struct st_pynote
{
    //TODO:  Include the native ALSA types to pass, to avoid having to free them later?
    //TODO TODO:  It may be more efficient to process as Int?
    char note;
    char velocity;
    float start;
    float length;    
}t_pynote;

t_pynote * g_pynote_get(char a_note, char a_vel, float a_start, float a_length);

typedef struct st_pycc
{
    char cc_num;
    char cc_val;
    float start;
}t_pycc;

t_pycc * g_pycc_get(char a_cc_num, char a_cc_val, float a_start);

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

t_pyitem * g_pyitem_get(char * a_name);

typedef struct st_pyregion
{
    int items[PYDAW_MAX_TRACK_COUNT][PYDAW_REGION_SIZE];  //Refers to the index of items in the master item pool
    int row_count;
    int column_count;    
}t_pyregion;

t_pyregion * g_pyregion_get(char * a_name);

typedef struct st_pysong
{
    int region_count;
    int region_index[PYDAW_MAX_REGION_COUNT];
    int max_regions;
}t_pysong;

t_pysong * g_pysong_get(char * a_name);

typedef struct st_pytrack
{
    float volume;
    int solo;
    int mute;    
}t_pytrack;

t_pytrack * g_pytrack_get();

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

t_pysong * g_pysong_get(char * a_name)
{
    t_pysong * f_result = (t_pysong*)malloc(sizeof(t_pysong));
    
    f_result->region_count = 0;
    f_result->max_regions = PYDAW_MAX_REGION_COUNT;
    
    return f_result;
}

t_pyregion * g_pyregion_get(char * a_name)
{
    t_pyregion * f_result = (t_pyregion*)malloc(sizeof(t_pyregion));
    
    int f_i, f_i2 = 0;
    
    while(f_i < PYDAW_MAX_TRACK_COUNT)
    {
        while(f_i2 < PYDAW_REGION_SIZE)
        {
            f_result->items[f_i][f_i2] = -1;
            f_i2++;
        }
        f_i++;
    }
    
    return f_result;
}

t_pyitem * g_pyitem_get(char * a_name)
{
    t_pyitem * f_result = (t_pyitem*)malloc(sizeof(t_pyitem));
    
    f_result->name = a_name;
    f_result->cc_count = 0;
    f_result->note_count = 0;
    
    return f_result;
}

t_pytrack * g_pytrack_get()
{
    t_pytrack * f_result = (t_pytrack*)malloc(sizeof(t_pytrack));
    
    f_result->mute = 0;
    f_result->solo = 0;
    f_result->volume = 0.0f;
    
    return f_result;
}

typedef struct st_pydaw_data
{
    float tempo;
    pthread_mutex_t mutex;
    t_pysong * pysong;
    t_pyitem * item_pool[PYDAW_MAX_ITEM_COUNT];
    t_pyregion * region_pool[PYDAW_MAX_REGION_COUNT];
    t_pytrack * track_pool[PYDAW_MAX_TRACK_COUNT];
    int item_count;
    int playback_mode;  //0 == Stop, 1 == Play, 2 == Rec
    int loop_mode;  //0 == Off, 1 == Bar, 2 == Region
}t_pydaw_data;

t_pydaw_data * g_pydaw_data_get();

t_pydaw_data * g_pydaw_data_get()
{
    t_pydaw_data * f_result = (t_pydaw_data*)malloc(sizeof(t_pydaw_data));
    
    //f_result->mutex = PTHREAD_MUTEX_INITIALIZER;
    f_result->tempo = 140.0f;
    //f_result->pysong = g_pysong_get();    
    f_result->item_count = 0;
    
    int f_i = 0;
    
    while(f_i < PYDAW_MAX_TRACK_COUNT)
    {
        f_result->track_pool[f_i] = g_pytrack_get();
        f_i++;
    }        
            
    return f_result;
}

//This will eventually get a real indexing algorithm.  Although generally it shouldn't have noticeably
//bad performance on a modern CPU because it's only called when the user does something in the UI.
int i_get_item_index_from_name(t_pydaw_data * a_pydaw_data, char * a_name)
{
    int f_i = 0;
    
    while(f_i < a_pydaw_data->item_count)
    {
        if(!strcmp((a_pydaw_data->item_pool[f_i]->name), a_name))
        {
            return f_i;
        }
        
        f_i++;
    }
    
    return -1;
}

void v_set_playback_mode(t_pydaw_data * a_pydaw_data, int a_mode)
{
    
}

void v_set_loop_mode(t_pydaw_data * a_pydaw_data, int a_mode)
{
    
}


void v_pydaw_parse_configure_message(t_pydaw_data*, const char*, const char*);

void v_pydaw_parse_configure_message(t_pydaw_data* a_pydaw, const char* a_key, const char* a_value)
{
    if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_SR)) //Save region
    {
        
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_SI)) //Save Item
    {
        
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_SS))  //Save Song
    {
        //Nothing for now, placeholder for future functionality
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_PLAY)) //Begin playback
    {
        v_set_playback_mode(a_pydaw, 1);
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_REC)) //Begin recording
    {
        v_set_playback_mode(a_pydaw, 2);
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_STOP)) //Stop playback or recording
    {
        v_set_playback_mode(a_pydaw, 0);
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_LOOP)) //Set loop mode
    {
        int f_value = atoi(a_value);
        v_set_loop_mode(a_pydaw, f_value);
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_NS)) //New Song
    {
        
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_OS)) //Open Song
    {
        
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_DI)) //Delete Item
    {
        
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_DR)) //Delete region
    {
        
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_RR)) //Rename region
    {
        
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_RI)) //Rename item
    {
        
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_SOLO)) //Set track solo
    {
        
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_MUTE)) //Set track mute
    {
        
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

