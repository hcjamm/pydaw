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

#ifndef VOICE_H
#define	VOICE_H

#define VOICES_MAX_MIDI_NOTE_NUMBER 128

#ifdef	__cplusplus
extern "C" {
#endif

#define MIDI_NOTES  128

typedef enum
{
    note_state_off = 0,
    note_state_running,
    /*Synths should iterate voices looking for any voice note_state
     * that is set to releasing, and  trigger a release event in
     * it's amplitude envelope*/
    note_state_releasing,
    note_state_killed
} note_state;

typedef struct st_voc_voice
{
    int voice_number;
    int note;
    note_state n_state;
    long on;
    long off;
}t_voc_single_voice;

t_voc_single_voice g_voc_get_single_voice(int);

t_voc_single_voice g_voc_get_single_voice(int a_voice_number)
{
    t_voc_single_voice * f_result =
            (t_voc_single_voice*)malloc(sizeof(t_voc_single_voice));

    f_result->voice_number = a_voice_number;
    f_result->note = -1;
    f_result->n_state = note_state_off;
    f_result->on = -1;
    f_result->off = -1;
    return *f_result;
}

typedef struct st_voc_voices
{
    t_voc_single_voice * voices;
    int count;
    int iterator;
    int steal_voice_index;
}t_voc_voices;

t_voc_voices * g_voc_get_voices(int);

t_voc_voices * g_voc_get_voices(int a_count)
{
    t_voc_voices * f_result = (t_voc_voices*)malloc(sizeof(t_voc_voices));

    f_result->count = a_count;

    f_result->voices =
            (t_voc_single_voice*)malloc(sizeof(t_voc_single_voice) * a_count);

    f_result->iterator = 0;
    f_result->steal_voice_index = 0;

    int f_i = 0;

    while(f_i < a_count)
    {
        f_result->voices[f_i] = g_voc_get_single_voice(f_i);
        f_i++;
    }

    return f_result;
}

/* int i_pick_voice(
 * t_voc_voices *data,
 * int a_current_note)
 *
 */
int i_pick_voice(t_voc_voices *data, int a_current_note,
        long a_current_sample, long a_tick)
{
    data->iterator = 0;
    /* Look for a duplicate note */
    int f_note_count = 0;
    int f_last_note = -1;
    while ((data->iterator) < (data->count))
    {
	//if ((data->voices[(data->iterator)].note == a_current_note) &&
        //(data->voices[(data->iterator)].n_state == note_state_running))
        if(data->voices[(data->iterator)].note == a_current_note)
        {
            if((data->voices[(data->iterator)].n_state == note_state_releasing)
                    ||
            (data->voices[(data->iterator)].n_state == note_state_running))
            {
                data->voices[(data->iterator)].n_state = note_state_killed;
                data->voices[(data->iterator)].off = a_current_sample;
            }

            f_note_count++;
            //do not allow more than 2 voices for any note, at any time...
            if(f_note_count > 1)
            {
                if(data->steal_voice_index == 0)
                {
                    data->steal_voice_index = 1;
                    data->voices[(data->iterator)].on =
                            a_current_sample + a_tick;
                    data->voices[(data->iterator)].n_state = note_state_running;
                    //data->voices[(data->iterator)].off = -1;
                    return (data->iterator);
                }
                else
                {
                    data->steal_voice_index = 0;
                    data->voices[f_last_note].on = a_current_sample + a_tick;
                    data->voices[f_last_note].n_state = note_state_running;
                    //data->voices[f_last_note].off = -1;
                    return f_last_note;
                }
            }
            else
            {
                f_last_note = (data->iterator);
            }
        }
        data->iterator = (data->iterator) + 1;
    }

    data->iterator = 0;
    /* Look for an inactive voice */
    while ((data->iterator) < (data->count))
    {
	if (data->voices[(data->iterator)].n_state == note_state_off)
        {
            data->voices[(data->iterator)].note = a_current_note;
            data->voices[(data->iterator)].n_state = note_state_running;
            data->voices[(data->iterator)].on = a_current_sample + a_tick;

            return (data->iterator);
	}

        data->iterator = (data->iterator) + 1;
    }

    int highest_note = 0;
    int highest_note_voice = 0;

    data->iterator = 0;
    /* otherwise find for the highest note and replace that */
    while ((data->iterator) < (data->count))
    {
	if (data->voices[(data->iterator)].note > highest_note)
        {
	    highest_note = data->voices[(data->iterator)].note;
	    highest_note_voice = (data->iterator);
	}
        data->iterator = (data->iterator) + 1;
    }

    data->voices[highest_note_voice].note = a_current_note;
    data->voices[highest_note_voice].on = a_current_sample + a_tick;
    data->voices[highest_note_voice].n_state = note_state_running;
    data->voices[highest_note_voice].off = -1;

    return highest_note_voice;
}

/* void v_voc_note_off(t_voc_voices * a_voc, int a_note,
 * long a_current_sample, long a_tick)
 */
void v_voc_note_off(t_voc_voices * a_voc, int a_note,
        long a_current_sample, long a_tick)
{
    a_voc->iterator = 0;

    while((a_voc->iterator) < (a_voc->count))
    {
        if(((a_voc->voices[(a_voc->iterator)].note) == a_note) &&
           ((a_voc->voices[(a_voc->iterator)].n_state) == note_state_running))
        {
            a_voc->voices[(a_voc->iterator)].n_state = note_state_releasing;
            a_voc->voices[(a_voc->iterator)].off = a_current_sample + a_tick;
        }

        a_voc->iterator  = (a_voc->iterator) + 1;
    }
}

#ifdef	__cplusplus
}
#endif

#endif	/* VOICE_H */

