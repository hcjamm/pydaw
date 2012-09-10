/* 
 * File:   lms_alsa.h
 * Author: Jeff Hubbard
 *
 * Functions and Types for handling ALSA events, mostly in the context of a MIDI sequencer
 * 
 * Created on September 9, 2012, 11:50 AM
 */

#ifndef LMS_ALSA_H
#define	LMS_ALSA_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <alsa/asoundlib.h>

#define LMS_MAX_ALSA_MIDI_PORTS_COUNT 16
    
typedef struct st_alsa_note_event
{    
    unsigned char a_note_number;
    unsigned char a_velocity;
    snd_seq_event_type_t a_event_type;
    unsigned int a_timestamp;  //TODO:  This should be retroactively determined by the sequencer, change to a relative musical notation
    
}t_alsa_note_event;

t_alsa_note_event * g_alsa_get_note_ev(/*TODO*/)
{
    t_alsa_note_event * f_result;
    
    //TODO
    
    return f_result;
}
    
typedef struct st_alsa_sequencer
{
    snd_seq_t *seq_handle;
    int out_port;
}t_alsa_sequencer;


t_alsa_sequencer * midi_open()
{
    t_alsa_sequencer * a_seq;
    
    snd_seq_open(&a_seq->seq_handle, "default", SND_SEQ_OPEN_OUTPUT, 0);
    
    snd_seq_set_client_name(a_seq->seq_handle, "PyDAW");
    
    a_seq->out_port = snd_seq_create_simple_port(a_seq->seq_handle, "out",
                      SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ,
                      SND_SEQ_PORT_TYPE_APPLICATION);    
    
    return a_seq;
}

/* void write_note_event(
 * int a_port_number, 
 * unsigned char a_note_number, 
 * unsigned char a_velocity, 
 * snd_seq_event_type_t a_event_type, //Expected:  SND_SEQ_EVENT_NOTEON or SND_SEQ_EVENT_NOTEOFF
 * unsigned int a_timestamp)
 * */
void write_note_event(t_alsa_sequencer* a_seq, int a_port_number)
{    
    snd_seq_event_t *ev;
    ev->type = a_event_type;
    ev->time->tick = a_timestamp;
    ev->data->note->note = a_note_number;
    ev->data->note->velocity = a_velocity;
    ev->dest->port = a_seq->out_port;//[a_port_number]
    
    write_midi_event(ev);
}

void write_controller_event(t_alsa_sequencer* a_seq)
{
    
}

static void write_midi_event(t_alsa_sequencer* a_seq, snd_seq_event_t *ev)
{   
    snd_seq_event_output_direct(a_seq->seq_handle, ev);
    //snd_seq_event_output_buffer(seq_handle, ev);
    //snd_seq_event_output
    
}

/*These are the obvious candidates for creating a connection(shitty ALSA documentation FTW!!!):
 
snd_seq_connect_to() 
 
typedef struct snd_seq_addr {
	unsigned char client;	
	unsigned char port;	
} snd_seq_addr_t;


typedef struct snd_seq_connect {
	snd_seq_addr_t sender;	
	snd_seq_addr_t dest;	
} snd_seq_connect_t;
 */

#ifdef	__cplusplus
}
#endif

#endif	/* LMS_ALSA_H */

