/* 
 * File:   voice.h
 * Author: Jeff Hubbard
 * 
 * Provides a voicing architecture for polyphonic instruments
 *
 * Created on April 18, 2012, 5:59 PM
 */

#ifndef VOICE_H
#define	VOICE_H

#define VOICES_MAX_MIDI_NOTE_NUMBER 128

#ifdef	__cplusplus
extern "C" {
#endif
    
#define MIDI_NOTES  128  //Maximum MIDI note.  You probably don't want to change this

typedef enum 
{
    /**/
    note_state_off = 0,
    /**/
    note_state_running,
    /*Synths should iterate voices looking for any voice note_state that is set to releasing, and 
     trigger a release event in it's amplitude envelope*/
    note_state_releasing
} note_state;
    
typedef struct st_voc_voice
{
    int     note;
    note_state n_state;    
}t_voc_single_voice;

t_voc_single_voice g_voc_get_single_voice(int);

t_voc_single_voice g_voc_get_single_voice(int a_voice_number)
{
    t_voc_single_voice * f_result = (t_voc_single_voice*)malloc(sizeof(t_voc_single_voice));
    
    f_result->note = a_voice_number;
    f_result->n_state = note_state_off;
    
    return *f_result;
}

typedef struct st_voc_voices
{
    t_voc_single_voice * voices;
    int count;
    int iterator;
    int note_ons[VOICES_MAX_MIDI_NOTE_NUMBER];  //will be 1 for note_on, zero for note_off
}t_voc_voices;

t_voc_voices * g_voc_get_voices(int);

t_voc_voices * g_voc_get_voices(int a_count)
{
    t_voc_voices * f_result = (t_voc_voices*)malloc(sizeof(t_voc_voices));
    
    f_result->count = a_count;
    
    f_result->voices = (t_voc_single_voice*)malloc(sizeof(t_voc_single_voice*) * a_count);
    
    f_result->iterator = 0;
    
    int f_i = 0;
    
    while(f_i < a_count)
    {
        f_result->voices[f_i] = g_voc_get_single_voice(f_i);   
        f_i++;
    }
    
    f_i = 0;
    
    while(f_i < VOICES_MAX_MIDI_NOTE_NUMBER)
    {
        f_result->note_ons[f_i] = 0;
        f_i++;
    }    
        
    return f_result;
}

/* int i_pick_voice(
 * t_voc_voices *data, 
 * int a_current_note)
 * 
 * You must check whether the function returns -1(duplicate note), if so, do not attempt to fire off a voice. 
 */
int i_pick_voice(t_voc_voices *data, int a_current_note)
{   
    /*Note is already on, discard this event*/
    if((data->note_ons[a_current_note]) == 1)
    {
        return -1;
    }
    else
    {
        data->note_ons[a_current_note] == 1;
    }
    
    data->iterator = 0;
    /* Look for an inactive voice */
    while ((data->iterator) < (data->count)) {
	if ((data->voices[(data->iterator)].note == a_current_note) &&
                (data->voices[(data->iterator)].n_state == note_state_running)) {
                /*Kill the note if already being used, this is to prevent hung
                 notes in hosts that might not handle MIDI events properly*/
                data->voices[(data->iterator)].n_state = note_state_releasing;
	}
        
        data->iterator = (data->iterator) + 1;
    }
    
    data->iterator = 0;
    /* Look for an inactive voice */
    while ((data->iterator) < (data->count)) {
	if (data->voices[(data->iterator)].n_state == note_state_off) {
        
        data->voices[(data->iterator)].note = a_current_note;
        data->voices[(data->iterator)].n_state = note_state_running;
        
        return (data->iterator);
	}
        
        data->iterator = (data->iterator) + 1;
    }
    
    int highest_note = 0;
    int highest_note_voice = 0;
    
    data->iterator = 0;
    /* otherwise find for the highest note and replace that */
    while ((data->iterator) < (data->count)) {
	if (data->voices[(data->iterator)].note > highest_note) {
	    highest_note = data->voices[(data->iterator)].note;
	    highest_note_voice = (data->iterator);
	}
        data->iterator = (data->iterator) + 1;
    }

    data->voices[highest_note_voice].note = a_current_note;
    data->voices[highest_note_voice].n_state = note_state_running;
            
    return highest_note_voice;        
}

/* int i_voc_note_off(t_voc_voices * a_voc)
 * 
 * After running this event at note_off, you should run a loop like this:
 * 
 *         plugin_data->i_iterator = 0;
        
        while(plugin_data->i_iterator < (plugin_data->voices->count))
        {
            if((plugin_data->voices->voices[(plugin_data->i_iterator)].n_state) == note_state_releasing)
            {
                v_poly_note_off(plugin_data->data[(plugin_data->i_iterator)]);
            }
            
            plugin_data->i_iterator = (plugin_data->i_iterator) + 1;
        }
 * 
 * Where v_poly_note_off is a function that releases ADSR envelopes, etc...
 */
void v_voc_note_off(t_voc_voices * a_voc, int a_note)
{
    /* It may be worthwhile to check that the note isn't already off, however, this is mostly for preventing stuck notes, 
     so I'd rather release an envelope twice than risk not releasing it by dropping an event */
    a_voc->note_ons[a_note] == 0;
    
    a_voc->iterator = 0;
    
    while((a_voc->iterator) < (a_voc->count))
    {
        if(((a_voc->voices[(a_voc->iterator)].note) == a_note) &&
           ((a_voc->voices[(a_voc->iterator)].n_state) == note_state_running))
        {
            a_voc->voices[(a_voc->iterator)].n_state = note_state_releasing;            
        }
        
        a_voc->iterator  = (a_voc->iterator) + 1;
    }    
}

#ifdef	__cplusplus
}
#endif

#endif	/* VOICE_H */

