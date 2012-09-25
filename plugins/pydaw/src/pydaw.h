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
    
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include "pydaw_files.h"
    
typedef struct st_pynote
{
    //TODO:  Include the native ALSA types to pass, to avoid having to free them later?
    char note;
    char velocity;
    float start;
    float length;    
}t_pynote;

t_pynote * g_pynote_get();

typedef struct st_pycc
{
    int cc_num;
    int cc_val;
    float start;
}t_pycc;

t_pycc * g_pycc_get();

typedef struct st_pyitem
{
    char * name;
    t_pynote * notes;
    int note_count;
    int max_notes;
    t_pycc * ccs;
    int cc_count;
    int max_ccs;
    int resize_factor;
}t_pyitem;

t_pyitem * g_pyitem_get();


typedef struct st_pyregion
{
    t_pyitem ** items;
    int row_count;
    int column_count;
}t_pyregion;

t_pyregion * g_pyregion_get();

typedef struct st_pysong
{
    t_pyregion * regions;
    int region_count;
    int max_regions;
}t_pysong;

t_pysong * g_pysong_get();


t_pysong * g_pysong_get()
{
    t_pysong * f_result = (t_pysong*)malloc(sizeof(t_pysong));
    
    f_result->region_count = 0;
    f_result->max_regions = 100;
    f_result->regions = (t_pyregion*)malloc(sizeof(t_pyregion) * (f_result->max_regions));
    
    return f_result;
}

t_pyregion * g_pyregion_get()
{
    t_pyregion * f_result = (t_pyregion*)malloc(sizeof(t_pyregion));
    
    //TODO:  Make these not hard-coded
    f_result->column_count = 8;
    f_result->row_count = 16;
    f_result->items = malloc(sizeof(t_pyitem*) * (f_result->row_count));

    int i;
    for (i = 0; i < (f_result->row_count); i++)
    {
      f_result->items[i] = malloc(sizeof(t_pyitem) * (f_result->column_count));
    }
            
    return f_result;
}

t_pyitem * g_pyitem_get(char * a_name)
{
    t_pyitem * f_result = (t_pyitem*)malloc(sizeof(t_pyitem));
    
    f_result->name = a_name;
    f_result->cc_count = 0;
    f_result->note_count = 0;
    f_result->resize_factor = 128;
    f_result->max_ccs = (f_result->resize_factor);
    f_result->max_notes = (f_result->resize_factor);
    f_result->ccs = (t_pycc*)malloc(sizeof(t_pycc));
    f_result->notes = (t_pynote*)malloc(sizeof(t_pynote));
    
    return f_result;
}


typedef struct st_pydaw_data
{
    int loop_mode;
    float tempo;
    pthread_mutex_t mutex;
    t_pysong * pysong;
    t_pyitem * item_pool;
    int item_count;
    int max_items;
    int resize_factor;
}t_pydaw_data;

t_pydaw_data * g_pydaw_data_get();

t_pydaw_data * g_pydaw_data_get()
{
    t_pydaw_data * f_result = (t_pydaw_data*)malloc(sizeof(t_pydaw_data));
    
    //f_result->mutex = PTHREAD_MUTEX_INITIALIZER;
    f_result->tempo = 140.0f;
    f_result->pysong = g_pysong_get();
    f_result->resize_factor = 512;
    f_result->item_count = 0;
    f_result->max_items = (f_result->resize_factor);
    f_result->item_pool = (t_pyitem*)malloc(sizeof(t_pyitem) * (f_result->max_items));
        
    return f_result;
}

void v_pydaw_parse_configure_message(t_pydaw_data*, char*, char*);

void v_pydaw_parse_configure_message(t_pydaw_data* a_pydaw, char* a_key, char* a_value)
{
    //TODO:  Move the obvious most commonly used ones to the top of the stack...
    if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_SS))  //Save Song
    {
        
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_NS)) //New Song
    {
        
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_SS)) //Update Song
    {
        
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_OS)) //Open Song
    {
        
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_SI)) //Save Item
    {
        
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_DI)) //Delete Item
    {
        
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_SR)) //Save region
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
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_PLAY)) //Begin playback
    {
        
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_REC)) //Begin recording
    {
        
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_STOP)) //Stop playback or recording
    {
        
    }
    else if(!strcmp(a_key, PYDAW_CONFIGURE_KEY_LOOP)) //Set loop mode
    {
        
    }
    
    else
    {
        printf("Unknown configure message key: %s, value %s", a_key, a_value);
    }
        
    

}

#ifdef	__cplusplus
}
#endif

#endif	/* PYDAW_H */

