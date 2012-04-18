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

#ifdef	__cplusplus
extern "C" {
#endif
    
#define MIDI_NOTES  128  //Maximum MIDI note.  You probably don't want to change this

typedef enum 
{
    off = 0,    
    running
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
    f_result->n_state = off;
    
    return *f_result;
}

typedef struct st_voc_voices
{
    int note2voice[MIDI_NOTES];
    t_voc_single_voice * voices;
    int count;
}t_voc_voices;

t_voc_voices * g_voc_get_voices(int);

t_voc_voices * g_voc_get_voices(int a_count)
{
    t_voc_voices * f_result = (t_voc_voices*)malloc(sizeof(t_voc_voices));
    
    f_result->count = a_count;
    
    f_result->voices = (t_voc_single_voice*)malloc(sizeof(t_voc_single_voice*) * a_count);
    
    int f_i = 0;
    
    while(f_i < a_count)
    {
        f_result->voices[f_i] = g_voc_get_single_voice(f_i);   
        f_i++;
    }
    
    f_i = 0;
    
    while(f_i < MIDI_NOTES)
    {
        f_result->note2voice[f_i] = 0;
        f_i++;
    }
    
    return f_result;
}

/* Original comment:  find the voice that is least relevant (low note priority)*/
/*libmodsynth comment:  *data is an array of voices in an LMS struct,
 iterate through them for a free voice, or if one cannot be found,
 pick the highest voice*/
int i_pick_voice(t_voc_voices *data, int a_current_note)
{
    unsigned int f_i;
    int highest_note = 0;
    int highest_note_voice = 0;
    
    /*Look for the voice being played by the current note.
     It's more musical to kill the same note than to let it play twice,
     guitars, pianos, etc... work that way.  It also helps to prevent hung notes*/    
    for (f_i=0; f_i<data->count; f_i++) {
	if (data->voices[f_i].note == a_current_note) {
#ifdef LMS_DEBUG_VOICES            
            printf("pick_voice found current_note:  %i\n", f_i);
#endif
            /*TODO:  Re-add this part*/
            //v_adsr_set_fast_release(data[f_i].p_voice->adsr_amp);
            break;	    
	}
    }
    
    /* Look for an inactive voice */
    for (f_i=0; f_i<data->count; f_i++) {
	if (data->voices[f_i].n_state == off) {
#ifdef LMS_DEBUG_VOICES            
            printf("pick_voice found inactive voice:  %i\n", f_i);
#endif
            data->note2voice[a_current_note] = f_i;
            data->voices[f_i].note = a_current_note;
            
	    return f_i;
	}
    }

    /* otherwise find for the highest note and replace that */
    for (f_i=0; f_i<data->count; f_i++) {
	if (data->voices[f_i].note > highest_note) {
	    highest_note = data->voices[f_i].note;
	    highest_note_voice = f_i;
	}
    }

#ifdef LMS_DEBUG_VOICES            
            printf("pick_voice found highest voice:  %i\n", highest_note_voice);
#endif
    data->note2voice[a_current_note] = highest_note_voice;
    data->voices[highest_note_voice].note = a_current_note;
            
    return highest_note_voice;        
}


#ifdef	__cplusplus
}
#endif

#endif	/* VOICE_H */

