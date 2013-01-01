/* 
 * File:   pydaw_audio_tracks.h
 * Author: JeffH
 *
 * Created on December 29, 2012, 12:03 AM
 */

#ifndef PYDAW_AUDIO_TRACKS_H
#define	PYDAW_AUDIO_TRACKS_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include <sndfile.h>

#define PYDAW_MAX_AUDIO_ITEM_COUNT 32
    
typedef struct 
{
    float * samples[2];
    int length;    
} t_pydaw_audio_item;

typedef struct 
{
    t_pydaw_audio_item * items;
} t_pydaw_audio_items;



t_pydaw_audio_item * g_pydaw_audio_item_get();
t_pydaw_audio_item * g_pydaw_audio_items_get();

t_pydaw_audio_item * g_pydaw_audio_item_get()
{
    t_pydaw_audio_item * f_result;
    
    if(posix_memalign((void**)&f_result, 16, sizeof(t_pydaw_audio_item)) != 0)
    {     
        return 0;
    }
    
    f_result->length = 0;
    f_result->samples[0] = 0;
    f_result->samples[1] = 0;
    
    return f_result;
}

t_pydaw_audio_item * g_pydaw_audio_items_get()
{
    t_pydaw_audio_items * f_result;
    
    if(posix_memalign((void**)&f_result, 16, sizeof(t_pydaw_audio_items)) != 0)
    {     
        return 0;
    }
    
    int f_i = 0;
    
    while(f_i < PYDAW_MAX_AUDIO_ITEM_COUNT)
    {
        f_result->items[f_i] = g_pydaw_audio_item_get();
        f_i++;
    }
    
    return f_result;
}

#ifdef	__cplusplus
}
#endif

#endif	/* PYDAW_AUDIO_TRACKS_H */

