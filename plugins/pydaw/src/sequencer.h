/* 
 * File:   sequencer.h
 * Author: JeffH
 *
 * The ALSA sequencer parts of PyDAW.  Currently just a fork of miniArp.c by
 * Dr. Matthias Nagorni, part of his ALSA programming tutorial
 * 
 */

#ifndef SEQUENCER_H
#define	SEQUENCER_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <alsa/asoundlib.h>

#define TICKS_PER_QUARTER 128
#define MAX_SEQ_LEN        64

int queue_id, port_in_id, port_out_id, transpose, bpm0, bpm, tempo, swing, sequence[3][MAX_SEQ_LEN], seq_len;
snd_seq_t *seq_handle;
char seq_filename[1024];
snd_seq_tick_time_t tick;

snd_seq_t *open_seq() {

  snd_seq_t *seq_handle;

  if (snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_DUPLEX, 0) < 0) {
    fprintf(stderr, "Error opening ALSA sequencer.\n");
    exit(1);
  }
  snd_seq_set_client_name(seq_handle, "miniArp");
  if ((port_out_id = snd_seq_create_simple_port(seq_handle, "miniArp",
            SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ,
            SND_SEQ_PORT_TYPE_APPLICATION)) < 0) {
    fprintf(stderr, "Error creating sequencer port.\n");
  }
  if ((port_in_id = snd_seq_create_simple_port(seq_handle, "miniArp",
            SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
            SND_SEQ_PORT_TYPE_APPLICATION)) < 0) {
    fprintf(stderr, "Error creating sequencer port.\n");
    exit(1);
  }
  return(seq_handle);
}

void set_tempo() {

  snd_seq_queue_tempo_t *queue_tempo;

  snd_seq_queue_tempo_malloc(&queue_tempo);
  tempo = (int)(6e7 / ((double)bpm * (double)TICKS_PER_QUARTER) * (double)TICKS_PER_QUARTER);
  snd_seq_queue_tempo_set_tempo(queue_tempo, tempo);
  snd_seq_queue_tempo_set_ppq(queue_tempo, TICKS_PER_QUARTER);
  snd_seq_set_queue_tempo(seq_handle, queue_id, queue_tempo);
  snd_seq_queue_tempo_free(queue_tempo);
}

snd_seq_tick_time_t get_tick() {

  snd_seq_queue_status_t *status;
  snd_seq_tick_time_t current_tick;
  
  snd_seq_queue_status_malloc(&status);
  snd_seq_get_queue_status(seq_handle, queue_id, status);
  current_tick = snd_seq_queue_status_get_tick_time(status);
  snd_seq_queue_status_free(status);
  return(current_tick);
}

void init_queue() {

  queue_id = snd_seq_alloc_queue(seq_handle);
  snd_seq_set_client_pool_output(seq_handle, (seq_len<<1) + 4);
}

void clear_queue() {

  snd_seq_remove_events_t *remove_ev;

  snd_seq_remove_events_malloc(&remove_ev);
  snd_seq_remove_events_set_queue(remove_ev, queue_id);
  snd_seq_remove_events_set_condition(remove_ev, SND_SEQ_REMOVE_OUTPUT | SND_SEQ_REMOVE_IGNORE_OFF);
  snd_seq_remove_events(seq_handle, remove_ev);
  snd_seq_remove_events_free(remove_ev);
}

void arpeggio() {

  snd_seq_event_t ev;
  int l1;
  double dt;
 
  for (l1 = 0; l1 < seq_len; l1++) {
    dt = (l1 % 2 == 0) ? (double)swing / 16384.0 : -(double)swing / 16384.0;
    snd_seq_ev_clear(&ev);
    snd_seq_ev_set_note(&ev, 0, sequence[2][l1] + transpose, 127, sequence[1][l1]);
    snd_seq_ev_schedule_tick(&ev, queue_id,  0, tick);
    snd_seq_ev_set_source(&ev, port_out_id);
    snd_seq_ev_set_subs(&ev);
    snd_seq_event_output_direct(seq_handle, &ev);
    tick += (int)((double)sequence[0][l1] * (1.0 + dt));
  }
  snd_seq_ev_clear(&ev);
  ev.type = SND_SEQ_EVENT_ECHO; 
  snd_seq_ev_schedule_tick(&ev, queue_id,  0, tick);
  snd_seq_ev_set_dest(&ev, snd_seq_client_id(seq_handle), port_in_id);
  snd_seq_event_output_direct(seq_handle, &ev);
}

void midi_action() {

  snd_seq_event_t *ev;

  do {
    snd_seq_event_input(seq_handle, &ev);
    switch (ev->type) {
      case SND_SEQ_EVENT_ECHO:
        arpeggio(); 
        break;
      case SND_SEQ_EVENT_NOTEON:
        clear_queue();
        transpose = ev->data.note.note - 60;
        tick = get_tick();
        arpeggio();
        break;        
      case SND_SEQ_EVENT_CONTROLLER:
        if (ev->data.control.param == 1) {           
          bpm = (int)((double)bpm0 * (1.0 + (double)ev->data.control.value / 127.0));
          set_tempo();
        } 
        break;
      case SND_SEQ_EVENT_PITCHBEND:
        swing = (double)ev->data.control.value;        
        break;
    }
    snd_seq_free_event(ev);
  } while (snd_seq_event_input_pending(seq_handle, 0) > 0);
}

void parse_sequence() {

  FILE *f;
  char c;
  
  if (!(f = fopen(seq_filename, "r"))) {
    fprintf(stderr, "Couldn't open sequence file %s\n", seq_filename);
    exit(1);
  }
  seq_len = 0;
  while((c = fgetc(f))!=EOF) {
    switch (c) {
      case 'c': 
        sequence[2][seq_len] = 0; break; 
      case 'd': 
        sequence[2][seq_len] = 2; break;
      case 'e': 
        sequence[2][seq_len] = 4; break;
      case 'f': 
        sequence[2][seq_len] = 5; break;
      case 'g': 
        sequence[2][seq_len] = 7; break;
      case 'a': 
        sequence[2][seq_len] = 9; break;
      case 'h': 
        sequence[2][seq_len] = 11; break;
    }                    
    c =  fgetc(f);
    if (c == '#') {
      sequence[2][seq_len]++;
      c =  fgetc(f);
    }
    sequence[2][seq_len] += 12 * atoi(&c);
    c =  fgetc(f);
    sequence[1][seq_len] = TICKS_PER_QUARTER / atoi(&c);
    c =  fgetc(f);
    sequence[0][seq_len] = TICKS_PER_QUARTER / atoi(&c);    
    seq_len++;
  }
  fclose(f);
}

void sigterm_exit(int sig) {

  clear_queue();
  sleep(2);
  snd_seq_stop_queue(seq_handle, queue_id, NULL);
  snd_seq_free_queue(seq_handle, queue_id);
  exit(0);
}

int arp_main(int argc, char *argv[]) {

  int npfd, l1;
  struct pollfd *pfd;
    
  if (argc < 3) { 
    fprintf(stderr, "\n\nminiArp <beats per minute> <sequence file>\n");
    exit(1);
  }
  bpm0 = atoi(argv[1]);
  bpm = bpm0;
  strcpy(seq_filename, argv[2]); 
  parse_sequence();
  seq_handle = open_seq();
  init_queue();
  set_tempo();
  arpeggio();
  snd_seq_start_queue(seq_handle, queue_id, NULL);
  snd_seq_drain_output(seq_handle);
  npfd = snd_seq_poll_descriptors_count(seq_handle, POLLIN);
  pfd = (struct pollfd *)alloca(npfd * sizeof(struct pollfd));
  snd_seq_poll_descriptors(seq_handle, pfd, npfd, POLLIN);
  transpose = 0;
  swing = 0;
  tick = 0;
  signal(SIGINT, sigterm_exit);          
  signal(SIGTERM, sigterm_exit);  
  arpeggio();
  while (1) {
    if (poll(pfd, npfd, 100000) > 0) {
      for (l1 = 0; l1 < npfd; l1++) {
        if (pfd[l1].revents > 0) midi_action(); 
      }
    }  
  }
}

#ifdef	__cplusplus
}
#endif

#endif	/* SEQUENCER_H */

