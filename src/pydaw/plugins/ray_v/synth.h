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

#ifndef RAYV_SYNTH_H
#define	RAYV_SYNTH_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "../../include/pydaw_plugin.h"
#include "libmodsynth.h"
#include "../../libmodsynth/lib/voice.h"

#define RAYV_ATTACK  2
#define RAYV_DECAY   3
#define RAYV_SUSTAIN 4
#define RAYV_RELEASE 5
#define RAYV_TIMBRE  6
#define RAYV_RES  7
#define RAYV_DIST 8
#define RAYV_FILTER_ATTACK  9
#define RAYV_FILTER_DECAY   10
#define RAYV_FILTER_SUSTAIN 11
#define RAYV_FILTER_RELEASE 12
#define RAYV_NOISE_AMP 13
#define RAYV_FILTER_ENV_AMT 14
#define RAYV_DIST_WET 15
#define RAYV_OSC1_TYPE 16
#define RAYV_OSC1_PITCH 17
#define RAYV_OSC1_TUNE 18
#define RAYV_OSC1_VOLUME 19
#define RAYV_OSC2_TYPE 20
#define RAYV_OSC2_PITCH 21
#define RAYV_OSC2_TUNE 22
#define RAYV_OSC2_VOLUME 23
#define RAYV_MASTER_VOLUME 24
#define RAYV_MASTER_UNISON_VOICES 25
#define RAYV_MASTER_UNISON_SPREAD 26
#define RAYV_MASTER_GLIDE 27
#define RAYV_MASTER_PITCHBEND_AMT 28
#define RAYV_PITCH_ENV_TIME 29
#define RAYV_PITCH_ENV_AMT 30
#define RAYV_LFO_FREQ 31
#define RAYV_LFO_TYPE 32
#define RAYV_LFO_AMP 33
#define RAYV_LFO_PITCH 34
#define RAYV_LFO_FILTER 35
#define RAYV_OSC_HARD_SYNC 36
#define RAYV_RAMP_CURVE 37
#define RAYV_FILTER_KEYTRK 38
#define RAYV_COUNT 39 /* must be 1 + highest value above CHANGE THIS IF YOU ADD OR TAKE AWAY ANYTHING*/

#define RAYV_POLYPHONY   16
#define RAYV_POLYPHONY_THRESH 12

typedef struct {
    PYFX_Data *output0;
    PYFX_Data *output1;
    PYFX_Data *tune;
    PYFX_Data *attack;
    PYFX_Data *decay;
    PYFX_Data *sustain;
    PYFX_Data *release;
    PYFX_Data *timbre;
    PYFX_Data *res;
    PYFX_Data *dist;
    PYFX_Data pitch;

    PYFX_Data *attack_f;
    PYFX_Data *decay_f;
    PYFX_Data *sustain_f;
    PYFX_Data *release_f;

    PYFX_Data *osc1pitch;
    PYFX_Data *osc1tune;
    PYFX_Data *osc1type;
    PYFX_Data *osc1vol;

    PYFX_Data *osc2pitch;
    PYFX_Data *osc2tune;
    PYFX_Data *osc2type;
    PYFX_Data *osc2vol;

    PYFX_Data *filter_env_amt;
    PYFX_Data *filter_keytrk;
    PYFX_Data *dist_wet;
    PYFX_Data *master_vol;

    PYFX_Data *noise_amp;


    PYFX_Data *master_uni_voice;
    PYFX_Data *master_uni_spread;
    PYFX_Data *master_glide;
    PYFX_Data *master_pb_amt;

    PYFX_Data *pitch_env_amt;
    PYFX_Data *pitch_env_time;

    PYFX_Data *lfo_freq;
    PYFX_Data *lfo_type;
    PYFX_Data *lfo_amp;
    PYFX_Data *lfo_pitch;
    PYFX_Data *lfo_filter;
    PYFX_Data *ramp_curve;

    PYFX_Data *sync_hard;

    t_rayv_poly_voice * data[RAYV_POLYPHONY];
    t_voc_voices * voices;
    long         sampleNo;

    float fs;
    t_rayv_mono_modules * mono_modules;

    int event_pos;
    int i_run_poly_voice;
    int i_iterator;

    float sv_pitch_bend_value;
    float sv_last_note;  //For glide

    int midi_event_types[200];
    int midi_event_ticks[200];
    float midi_event_values[200];
    int midi_event_ports[200];
    int midi_event_count;

    float * port_table;
} t_rayv;




#ifdef	__cplusplus
}
#endif

#endif	/* RAY_VSYNTH_H */

