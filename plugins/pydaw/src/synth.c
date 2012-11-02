/* -*- c-basic-offset: 4 -*-  vi:set ts=8 sts=4 sw=4: */

/* synth.c

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <stdint.h>

#include <math.h>
#include <stdio.h>

#include "dssi.h"
#include "ladspa.h"

#include "libmodsynth.h"
#include "../../libmodsynth/lib/amp.h"
#include "pydaw.h"
#include "osc_handlers.h"
#include "synth.h"
#include "meta.h"

#include <unistd.h>
#include <alsa/asoundlib.h>

static LADSPA_Descriptor *LMSLDescriptor = NULL;
static DSSI_Descriptor *LMSDDescriptor = NULL;

static t_pydaw_data * pydaw_data;

int pydaw_osc_debug_handler(const char *path, const char *types, lo_arg **argv,
                      int argc, void *data, void *user_data)
{
    int i;

    printf("PyDAW: got unhandled OSC message:\npath: <%s>\n", path);
    for (i=0; i<argc; i++) 
    {
        printf("PyDAW: arg %d '%c' ", i, types[i]);
        lo_arg_pp(types[i], argv[i]);
        printf("\n");
    }
    
    return 1;
}

int pydaw_osc_message_handler(const char *path, const char *types, lo_arg **argv,
                        int argc, void *data, void *user_data)
{
    int i;
    t_pydaw_plugin *instance;
    const char *method;
    unsigned int flen = 0;
    lo_message message;
    lo_address source;
    int send_to_ui = 0;
    char tmp[6];
    
    //printf("\npydaw_osc_message_handler: %s\n\n", path);
    
    if (strncmp(path, "/dssi/", 6))
    {
        printf("\nstrncmp(path, \"/dssi/\", 6)\n\n");
        return pydaw_osc_debug_handler(path, types, argv, argc, data, user_data);
    }
      
    flen = 2;
    
    
    for (i = 0; i < PYDAW_MAX_TRACK_COUNT; i++) 
    {
        sprintf(tmp, "%i", i);
	flen = strlen(tmp);
        if (!strncmp(path + 20, tmp, flen)
	    && *(path + 20 + flen) == '/') //avoid matching prefix only
        { 
            //printf("instance==%i\n", i);
            instance = pydaw_data->track_pool[i]->instrument; //&instances[i];
            break;
        }
        
        sprintf(tmp, "%i-fx", i);
	flen = strlen(tmp);
        if (!strncmp(path + 20, tmp, flen)
	    && *(path + 20 + flen) == '/') //avoid matching prefix only
        { 
            //printf("instance==%i\n", i);
            instance = pydaw_data->track_pool[i]->effect; //&instances[i];
            break;
        }
    }
    
    
    if (!instance)
    {
        printf("\n!instance\n");
        return pydaw_osc_debug_handler(path, types, argv, argc, data, user_data);
    }
    
    method = path + 20 + flen;
    if (*method != '/' || *(method + 1) == 0)
    {
        printf("\n(*method != '/' || *(method + 1) == 0)\n\n%s\n\n", method);
        return pydaw_osc_debug_handler(path, types, argv, argc, data, user_data);
    }
    
    method++;

    message = (lo_message)data;
    source = lo_message_get_source(message);

    if (instance->uiSource && instance->uiTarget) 
    {
	if (strcmp(lo_address_get_hostname(source),lo_address_get_hostname(instance->uiSource)) 
                ||
	    strcmp(lo_address_get_port(source), lo_address_get_port(instance->uiSource))) 
        {
	    /* This didn't come from our known UI for this plugin, so send an update to that as well */
	    send_to_ui = 1;
	}
    }
    
    if (!strcmp(method, "configure") && argc == 2 && !strcmp(types, "ss")) 
    {
	if (send_to_ui) 
        {
	    lo_send(instance->uiTarget, instance->ui_osc_configure_path, "ss",
		    &argv[0]->s, &argv[1]->s);
	}

        return pydaw_osc_configure_handler(instance, argv);
    } 
    else if (!strcmp(method, "control") && argc == 2 && !strcmp(types, "if")) 
    {
	if (send_to_ui) 
        {
	    lo_send(instance->uiTarget, instance->ui_osc_control_path, "if",
		    argv[0]->i, argv[1]->f);
	}

        return pydaw_osc_control_handler(instance, argv);

    } 
    /*else if (!strcmp(method, "midi") && argc == 1 && !strcmp(types, "m")) 
    {
        return pydaw_osc_midi_handler(instance, argv);
    } */
    else if (!strcmp(method, "update") && argc == 1 && !strcmp(types, "s")) 
    {
        return pydaw_osc_update_handler(instance, argv, source);
    }
    else if (!strcmp(method, "exiting") && argc == 0) 
    {
        return pydaw_osc_exiting_handler(instance, argv);
    }
        
    return pydaw_osc_debug_handler(path, types, argv, argc, data, user_data);
}

static void v_pydaw_run(LADSPA_Handle instance, unsigned long sample_count,
		  snd_seq_event_t * events, unsigned long EventCount);


__attribute__ ((visibility("default")))
const LADSPA_Descriptor *ladspa_descriptor(unsigned long index)
{
    switch (index) {
    case 0:
	return LMSLDescriptor;
    default:
	return NULL;
    }
}

__attribute__ ((visibility("default")))
const DSSI_Descriptor *dssi_descriptor(unsigned long index)
{
    switch (index) {
    case 0:
	return LMSDDescriptor;
    default:
	return NULL;
    }
}

static void v_pydaw_cleanup(LADSPA_Handle instance)
{
    free(instance);
}

static void v_pydaw_connect_port(LADSPA_Handle instance, unsigned long port, LADSPA_Data * data)
{
    t_pydaw_engine *plugin;

    plugin = (t_pydaw_engine *) instance;
        
    if((port >= PYDAW_INPUT_MIN) && (port < PYDAW_INPUT_MAX))
    {
        plugin->input_arr[(port - PYDAW_INPUT_MIN)] = data;
    }
    else
    {        
        switch (port) 
        {     
            case PYDAW_OUTPUT0: plugin->output0 = data; break;
            case PYDAW_OUTPUT1: plugin->output1 = data; break;        
        }
    }    
}

static LADSPA_Handle g_pydaw_instantiate(const LADSPA_Descriptor * descriptor, unsigned long s_rate)
{
    t_pydaw_engine *plugin_data = (t_pydaw_engine *) malloc(sizeof(t_pydaw_engine));
    pydaw_data = g_pydaw_data_get(s_rate);
    
    plugin_data->fs = s_rate;
        
    return (LADSPA_Handle) plugin_data;
}

static void v_pydaw_activate(LADSPA_Handle instance)
{
    t_pydaw_engine *plugin_data = (t_pydaw_engine *) instance;        
    plugin_data->mono_modules = v_mono_init((plugin_data->fs));  
    v_pydaw_activate_osc_thread(pydaw_data, pydaw_osc_message_handler);
}

/*
static void runLMSWrapper(LADSPA_Handle instance, unsigned long sample_count)
{
    run_lms_pydaw(instance, sample_count, NULL, 0);
}
*/

static void v_pydaw_run(LADSPA_Handle instance, unsigned long sample_count, snd_seq_event_t *events, unsigned long event_count)
{
    t_pydaw_engine *plugin_data = (t_pydaw_engine *) instance;
    /*Define our inputs*/
    
    /*define our outputs*/
    LADSPA_Data *const output0 = plugin_data->output0;    
    LADSPA_Data *const output1 = plugin_data->output1;
    
    plugin_data->i_buffer_clear = 0;
    
    /*Clear the output buffer*/
    while((plugin_data->i_buffer_clear) < sample_count)  //TODO:  Consider memset'ing???
    {
        output0[(plugin_data->i_buffer_clear)] = 0.0f;                        
        output1[(plugin_data->i_buffer_clear)] = 0.0f;     
        plugin_data->i_buffer_clear = (plugin_data->i_buffer_clear) + 1;
    }
    
    pthread_mutex_lock(&pydaw_data->mutex);
           
    long f_next_current_sample = ((pydaw_data->current_sample) + sample_count);
    
    if(pydaw_data->is_initialized)
    {
        if((pydaw_data->playback_mode) > 0)
        {
            double f_sample_period_inc = ((pydaw_data->playback_inc) * ((double)(sample_count)));
            double f_sample_period_inc_beats = f_sample_period_inc * 4.0f;
            double f_next_playback_cursor = (pydaw_data->playback_cursor) + f_sample_period_inc;        
            double f_current_period_beats = (pydaw_data->playback_cursor) * 4.0f;
            double f_next_period_beats = f_next_playback_cursor * 4.0f;

            int f_i = 0;
            
            while(f_i < PYDAW_MAX_TRACK_COUNT)
            {
                pydaw_data->track_pool[f_i]->event_index = 0;
                f_i++;
            }
            
            f_i = 0;
            
            pydaw_data->is_soloed = 0;
            
            while(f_i < PYDAW_MAX_TRACK_COUNT)
            {
                if(pydaw_data->track_pool[f_i]->solo)
                {
                    pydaw_data->is_soloed = 1;
                    break;
                }
                f_i++;
            }
            
            //Calculate track notes for this period and send them to instruments
            f_i = 0;
            while(f_i < PYDAW_MAX_TRACK_COUNT)
            {   
                /* Situations where the track is effectively muted*/
                if((pydaw_data->track_pool[f_i]->plugin_index == 0) ||
                    (pydaw_data->track_pool[f_i]->mute) ||
                    ((pydaw_data->is_soloed) && (!pydaw_data->track_pool[f_i]->solo)) )
                {
                    f_i++;
                    continue;
                }

                int f_current_track_region = pydaw_data->current_region;
                int f_current_track_bar = pydaw_data->current_bar;
                double f_track_current_period_beats = f_current_period_beats;
                double f_track_next_period_beats = f_next_period_beats;
                double f_track_beats_offset = 0.0f;

                
                if((pydaw_data->playback_mode == 2) && (pydaw_data->track_pool[f_i]->rec))
                {
                    
                }
                else
                {
                    while(1)
                    {
                        if((pydaw_data->pysong->regions[f_current_track_region]) && 
                            (pydaw_data->pysong->regions[f_current_track_region]->item_indexes[f_i][f_current_track_bar] != -1))
                        {
                            t_pyitem f_current_item = 
                                    *(pydaw_data->item_pool[(pydaw_data->pysong->regions[f_current_track_region]->item_indexes[f_i][f_current_track_bar])]);

                            if((pydaw_data->track_note_event_indexes[f_i]) >= (f_current_item.note_count))
                            {
                                if(f_track_next_period_beats >= 4.0f)
                                {
                                    f_track_current_period_beats = 0.0f;
                                    f_track_next_period_beats = f_track_next_period_beats - 4.0f;
                                    f_track_beats_offset = (f_sample_period_inc * 4.0f) - f_track_next_period_beats;

                                    pydaw_data->track_note_event_indexes[f_i] = 0;

                                    if(pydaw_data->loop_mode != PYDAW_LOOP_MODE_BAR)
                                    {
                                        f_current_track_bar++;

                                        if((pydaw_data->current_bar) >= PYDAW_REGION_SIZE)
                                        {
                                            f_current_track_bar = 0;

                                            if(pydaw_data->loop_mode != PYDAW_LOOP_MODE_REGION)
                                            {
                                                f_current_track_region++;
                                            }
                                        }
                                    }
                                    
                                    continue;
                                }
                                else
                                {
                                    break;
                                }                            
                            }

                            if(((f_current_item.notes[(pydaw_data->track_note_event_indexes[f_i])]->start) >= f_track_current_period_beats) &&
                                ((f_current_item.notes[(pydaw_data->track_note_event_indexes[f_i])]->start) < f_track_next_period_beats))
                            {
                                snd_seq_ev_clear(&pydaw_data->track_pool[f_i]->event_buffer[(pydaw_data->track_pool[f_i]->event_index)]);

                                int f_note_sample_offset = 0;
                                float f_note_start_diff = ((f_current_item.notes[(pydaw_data->track_note_event_indexes[f_i])]->start) - f_track_current_period_beats) + f_track_beats_offset;
                                float f_note_start_frac = f_note_start_diff / f_sample_period_inc_beats;
                                f_note_sample_offset =  (int)(f_note_start_frac * ((float)sample_count));                            

                                printf("\n\nSending note_on event\nf_i = %i, f_note_start_diff = %f, f_sample_period_inc_beats = %f, f_note_start_frac = %f, f_note_sample_offset = %i, sample_count = %i, pydaw_data->current_sample = %i\n\n", 
                                        f_i, f_note_start_diff, f_sample_period_inc_beats, f_note_start_frac, f_note_sample_offset, (int)sample_count, pydaw_data->current_sample);

                                snd_seq_ev_set_noteon(&pydaw_data->track_pool[f_i]->event_buffer[(pydaw_data->track_pool[f_i]->event_index)], 0,
                                        f_current_item.notes[(pydaw_data->track_note_event_indexes[f_i])]->note,
                                        f_current_item.notes[(pydaw_data->track_note_event_indexes[f_i])]->velocity);

                                pydaw_data->track_pool[f_i]->event_buffer[(pydaw_data->track_pool[f_i]->event_index)].time.tick = f_note_sample_offset;

                                pydaw_data->track_pool[f_i]->event_index = (pydaw_data->track_pool[f_i]->event_index) + 1;

                                pydaw_data->note_offs[f_i][(f_current_item.notes[(pydaw_data->track_note_event_indexes[f_i])]->note)] = (pydaw_data->current_sample) + 
                                        ((int)(f_current_item.notes[(pydaw_data->track_note_event_indexes[f_i])]->length * (pydaw_data->samples_per_beat)));

                                //This assert will need to be adjusted once playback is allowed to start from anywhere other than bar:0, region:0                        
                                //int f_calculated_sample = ((int)(((((float)(pydaw_data->current_bar))  * 4.0f) +  (f_current_item.notes[(pydaw_data->track_note_event_indexes[f_i])]->start)) * pydaw_data->samples_per_beat));
                                //int f_real_sample = (f_note_sample_offset + (pydaw_data->current_sample));
                                //assert(f_calculated_sample == f_real_sample);

                                pydaw_data->track_note_event_indexes[f_i] = (pydaw_data->track_note_event_indexes[f_i]) + 1;
                            }
                            else
                            {
                                break;
                            }
                        }
                        else
                        {
                            break;
                        }
                    }
                }

                f_i++;
            } //while notes
            
            
            //Calculate track CCs for this period and update the controller ports
            f_i = 0;
            while(f_i < PYDAW_MAX_TRACK_COUNT)
            {   
                /* Situations where the track is effectively muted*/
                if((pydaw_data->track_pool[f_i]->plugin_index == 0) ||
                    (pydaw_data->track_pool[f_i]->mute) ||
                    ((pydaw_data->is_soloed) && (!pydaw_data->track_pool[f_i]->solo)) )
                {
                    f_i++;
                    continue;
                }

                int f_current_track_region = pydaw_data->current_region;
                int f_current_track_bar = pydaw_data->current_bar;
                double f_track_current_period_beats = f_current_period_beats;
                double f_track_next_period_beats = f_next_period_beats;
                double f_track_beats_offset = 0.0f;
                
                if((pydaw_data->playback_mode == 2) && (pydaw_data->track_pool[f_i]->rec))
                {
                    
                }
                else
                {
                    while(1)
                    {
                        if((pydaw_data->pysong->regions[f_current_track_region]) && 
                            (pydaw_data->pysong->regions[f_current_track_region]->item_indexes[f_i][f_current_track_bar] != -1))
                        {
                            t_pyitem f_current_item = 
                                    *(pydaw_data->item_pool[(pydaw_data->pysong->regions[f_current_track_region]->item_indexes[f_i][f_current_track_bar])]);

                            if((pydaw_data->track_cc_event_indexes[f_i]) >= (f_current_item.cc_count))
                            {
                                if(f_track_next_period_beats >= 4.0f)
                                {
                                    f_track_current_period_beats = 0.0f;
                                    f_track_next_period_beats = f_track_next_period_beats - 4.0f;
                                    f_track_beats_offset = (f_sample_period_inc * 4.0f) - f_track_next_period_beats;

                                    //pydaw_data->track_note_event_indexes[f_i] = 0;
                                    pydaw_data->track_cc_event_indexes[f_i] = 0;

                                    if(pydaw_data->loop_mode != PYDAW_LOOP_MODE_BAR)
                                    {
                                        f_current_track_bar++;

                                        if((pydaw_data->current_bar) >= PYDAW_REGION_SIZE)
                                        {
                                            f_current_track_bar = 0;

                                            if(pydaw_data->loop_mode != PYDAW_LOOP_MODE_REGION)
                                            {
                                                f_current_track_region++;
                                            }
                                        }
                                    }
                                    
                                    continue;
                                }
                                else
                                {
                                    break;
                                }                            
                            }

                            if(((f_current_item.ccs[(pydaw_data->track_cc_event_indexes[f_i])]->start) >= f_track_current_period_beats) &&
                                ((f_current_item.ccs[(pydaw_data->track_cc_event_indexes[f_i])]->start) < f_track_next_period_beats))
                            {
                                //
                                int controller = f_current_item.ccs[(pydaw_data->track_cc_event_indexes[f_i])]->cc_num;
                                if (controller > 0) //&& controller < MIDI_CONTROLLER_COUNT) 
                                {
                                    long controlIn = pydaw_data->track_pool[f_i]->instrument->controllerMap[controller];
                                    
                                    if (controlIn > 0) //not >= like in the other CC loop, this is raw formatted CCs without that goofy bit-shifting to make it work with ALSA.
                                    {
                                        /* controller is mapped to LADSPA port, update the port */
                                        snd_seq_event_t f_event;
                                        f_event.data.control.value = f_current_item.ccs[(pydaw_data->track_cc_event_indexes[f_i])]->cc_val;
                                        //v_pydaw_set_control_from_cc(pydaw_data->track_pool[f_i]->instrument, DSSI_CC_NUMBER(controlIn), &f_event, 0);
                                        v_pydaw_set_control_from_cc(pydaw_data->track_pool[f_i]->instrument, controlIn, &f_event, 0);
                                    }
                                }

                                pydaw_data->track_cc_event_indexes[f_i] = (pydaw_data->track_cc_event_indexes[f_i]) + 1;
                            }
                            else
                            {
                                break;
                            }
                        }
                        else
                        {
                            break;
                        }
                    }
                }

                f_i++;
            } //while CCs
            

            pydaw_data->playback_cursor = f_next_playback_cursor;

            //TODO:  I think that equaling exactly 1.0f might create a weird scenario where 2 note ons are sent
            if((pydaw_data->playback_cursor) >= 1.0f)
            {
                pydaw_data->playback_cursor = (pydaw_data->playback_cursor) - 1.0f;                
                
                if(pydaw_data->loop_mode != PYDAW_LOOP_MODE_BAR)
                {
                    pydaw_data->current_bar = (pydaw_data->current_bar) + 1;

                    if((pydaw_data->current_bar) >= PYDAW_REGION_SIZE)
                    {
                        pydaw_data->current_bar = 0;

                        if(pydaw_data->loop_mode != PYDAW_LOOP_MODE_REGION)
                        {
                            pydaw_data->current_region = (pydaw_data->current_region) + 1;

                            if((pydaw_data->current_region) >= PYDAW_MAX_REGION_COUNT)
                            {
                                pydaw_data->playback_mode = 0;
                                pydaw_data->current_region = 0;
                            }
                        }
                    }
                }

                printf("pydaw_data->current_region == %i, pydaw_data->current_bar == %i\n", (pydaw_data->current_region), (pydaw_data->current_bar));                
            }
        } //If playback_mode > 0
        
        int f_i = 0;
        
        while(f_i < PYDAW_MAX_TRACK_COUNT)
        {                             
            int f_i2 = 0;
            //This is going to be painfully slow scanning all 16x128=2048 entries, but it may not really matter, so I won't prematurely optimize it now.
            while(f_i2 < PYDAW_MIDI_NOTE_COUNT)
            {
                if((pydaw_data->note_offs[f_i][f_i2]) >= (pydaw_data->current_sample) &&
                   (pydaw_data->note_offs[f_i][f_i2]) < f_next_current_sample)
                {
                    printf("\n\nSending note_off event\nf_i = %i, pydaw_data->note_offs[f_i][f_i2] = %i, pydaw_data->current_sample = %i\n\n", 
                                f_i, pydaw_data->note_offs[f_i][f_i2], pydaw_data->current_sample);
                    snd_seq_ev_clear(&pydaw_data->track_pool[f_i]->event_buffer[(pydaw_data->track_pool[f_i]->event_index)]);

                    snd_seq_ev_set_noteoff(&pydaw_data->track_pool[f_i]->event_buffer[(pydaw_data->track_pool[f_i]->event_index)], 0, f_i2, 0);
                    pydaw_data->track_pool[f_i]->event_buffer[(pydaw_data->track_pool[f_i]->event_index)].time.tick = 
                            (pydaw_data->note_offs[f_i][f_i2]) - (pydaw_data->current_sample);

                    pydaw_data->track_pool[f_i]->event_index = (pydaw_data->track_pool[f_i]->event_index) + 1;
                }
                f_i2++;
            }
            f_i++;
        }
                
        f_i = 0;
                
        while(f_i < PYDAW_MAX_TRACK_COUNT)
        {   
            if(pydaw_data->track_pool[f_i]->rec)
            {
                int f_i2 = 0;
                
                if(pydaw_data->playback_mode == 0)
                {
                    pydaw_data->track_pool[f_i]->event_index = 0;
                }
                
                while(f_i2 < event_count)
                {
                    if(events[f_i2].type == SND_SEQ_EVENT_NOTEON)
                    {
                        snd_seq_ev_note_t n = events[f_i2].data.note;
                        snd_seq_ev_clear(&pydaw_data->track_pool[f_i]->event_buffer[(pydaw_data->track_pool[f_i]->event_index)]);
                        snd_seq_ev_set_noteon(&pydaw_data->track_pool[f_i]->event_buffer[(pydaw_data->track_pool[f_i]->event_index)], 0,
                                    n.note, n.velocity);
                        pydaw_data->track_pool[f_i]->event_buffer[(pydaw_data->track_pool[f_i]->event_index)].time.tick = 
                                (events[f_i2].time.tick);
                        pydaw_data->track_pool[f_i]->event_index = (pydaw_data->track_pool[f_i]->event_index) + 1;
                    }
                    else if(events[f_i2].type == SND_SEQ_EVENT_NOTEOFF)
                    {
                        snd_seq_ev_note_t n = events[f_i2].data.note;
                        snd_seq_ev_clear(&pydaw_data->track_pool[f_i]->event_buffer[(pydaw_data->track_pool[f_i]->event_index)]);
                        snd_seq_ev_set_noteoff(&pydaw_data->track_pool[f_i]->event_buffer[(pydaw_data->track_pool[f_i]->event_index)], 0, n.note, 0);
                        pydaw_data->track_pool[f_i]->event_buffer[(pydaw_data->track_pool[f_i]->event_index)].time.tick = 
                                (events[f_i2].time.tick);

                        pydaw_data->track_pool[f_i]->event_index = (pydaw_data->track_pool[f_i]->event_index) + 1;
                    }
                    else if(events[f_i2].type == SND_SEQ_EVENT_PITCHBEND)
                    {
                        snd_seq_ev_clear(&pydaw_data->track_pool[f_i]->event_buffer[(pydaw_data->track_pool[f_i]->event_index)]);
                        snd_seq_ev_set_pitchbend(&pydaw_data->track_pool[f_i]->event_buffer[(pydaw_data->track_pool[f_i]->event_index)], 
                                0, events[f_i2].data.control.value);
                        pydaw_data->track_pool[f_i]->event_index = (pydaw_data->track_pool[f_i]->event_index) + 1;
                    }
                    else if(events[f_i2].type == SND_SEQ_EVENT_CONTROLLER)
                    {
                        int controller = events[f_i2].data.control.param;
                        if (controller > 0) //&& controller < MIDI_CONTROLLER_COUNT) 
                        {
                            long controlIn = pydaw_data->track_pool[f_i]->instrument->controllerMap[controller];
                            if (controlIn >= 0) 
                            {
                                /* controller is mapped to LADSPA port, update the port */
                                v_pydaw_set_control_from_cc(pydaw_data->track_pool[f_i]->instrument, controlIn, &events[f_i2], 0);
                            } 
                            else 
                            {
                                /* controller is not mapped, so pass the event through to plugin */
                                //instanceEventBuffers[i][instanceEventCounts[i]] = *ev;
                                //instanceEventCounts[i]++;
                                snd_seq_ev_ctrl_t c = events[f_i2].data.control;                        
                                snd_seq_ev_clear(&pydaw_data->track_pool[f_i]->event_buffer[(pydaw_data->track_pool[f_i]->event_index)]);
                                snd_seq_ev_set_controller(&pydaw_data->track_pool[f_i]->event_buffer[(pydaw_data->track_pool[f_i]->event_index)],
                                        0, c.param, c.value);
                                pydaw_data->track_pool[f_i]->event_index = (pydaw_data->track_pool[f_i]->event_index) + 1;
                            }
                        }
                    }
                    f_i2++;
                }
            }
            
            f_i++;
        }
        
        f_i = 0;

        while(f_i < PYDAW_MAX_TRACK_COUNT)
        {   
            if(pydaw_data->track_pool[f_i]->plugin_index != 0)
            {
            int f_i2 = 0;    
            while(f_i2 < (pydaw_data->track_pool[f_i]->instrument->controlIns))
            {
                if (pydaw_data->track_pool[f_i]->instrument->pluginPortUpdated[f_i2]) 
                {
                    int port = pydaw_data->track_pool[f_i]->instrument->pluginControlInPortNumbers[f_i2];
                    float value = pydaw_data->track_pool[f_i]->instrument->pluginControlIns[f_i2];
                    
                    pydaw_data->track_pool[f_i]->instrument->pluginPortUpdated[f_i2] = 0;
                    if (pydaw_data->track_pool[f_i]->instrument->uiTarget) 
                    {
                        lo_send(pydaw_data->track_pool[f_i]->instrument->uiTarget, pydaw_data->track_pool[f_i]->instrument->ui_osc_control_path, "if", port, value);
                    }
                }
                f_i2++;
            }
                
                v_run_plugin(pydaw_data->track_pool[f_i]->instrument, sample_count, 
                            pydaw_data->track_pool[f_i]->event_buffer, pydaw_data->track_pool[f_i]->event_index);
                
                memcpy(pydaw_data->track_pool[f_i]->effect->pluginInputBuffers[0], pydaw_data->track_pool[f_i]->instrument->pluginOutputBuffers[0], 
                        sample_count * sizeof(LADSPA_Data));
                memcpy(pydaw_data->track_pool[f_i]->effect->pluginInputBuffers[1], pydaw_data->track_pool[f_i]->instrument->pluginOutputBuffers[1], 
                        sample_count * sizeof(LADSPA_Data));
                
                v_run_plugin(pydaw_data->track_pool[f_i]->effect, sample_count, NULL, 0);
                            //pydaw_data->track_pool[f_i]->event_buffer, pydaw_data->track_pool[f_i]->event_index);
                
                int f_i3 = 0;

                while(f_i3 < sample_count)
                {
                    output0[f_i3] += (pydaw_data->track_pool[f_i]->effect->pluginOutputBuffers[0][f_i3]);
                    output1[f_i3] += (pydaw_data->track_pool[f_i]->effect->pluginOutputBuffers[1][f_i3]);
                    f_i3++;
                }
            }

            f_i++;
        }
                                         
    }//If is initialized
    
    /*TODO:  Run the LMS Limiter algorithm here at 0.0db, long release time, to prevent clipping*/
    
    pydaw_data->current_sample = f_next_current_sample;
    
    pthread_mutex_unlock(&pydaw_data->mutex);
    
}

char *c_pydaw_configure(LADSPA_Handle instance, const char *key, const char *value)
{
    //t_pydaw_engine *plugin_data = (t_pydaw_engine *)instance;
    v_pydaw_parse_configure_message(pydaw_data, key, value);
        
    return NULL;
}

int i_pydaw_get_controller(LADSPA_Handle instance, unsigned long port)
{    
    //t_pydaw_engine *plugin_data = (t_pydaw_engine *) instance;
    //return DSSI_CC(i_ccm_get_cc(plugin_data->midi_cc_map, port));
    return -1;     
}

#ifdef __GNUC__
__attribute__((constructor)) void v_pydaw_constructor()
#else
void _init()
#endif
{
    char **port_names;
    LADSPA_PortDescriptor *port_descriptors;
    LADSPA_PortRangeHint *port_range_hints;

    LMSLDescriptor =
	(LADSPA_Descriptor *) malloc(sizeof(LADSPA_Descriptor));
    if (LMSLDescriptor) {
        LMSLDescriptor->UniqueID = PYDAW_PLUGIN_UUID;
	LMSLDescriptor->Label = PYDAW_PLUGIN_NAME;
	LMSLDescriptor->Properties = LADSPA_PROPERTY_REALTIME;
	LMSLDescriptor->Name = PYDAW_PLUGIN_LONG_NAME;
	LMSLDescriptor->Maker = PYDAW_PLUGIN_DEV;
	LMSLDescriptor->Copyright = "GNU GPL v3";
	LMSLDescriptor->PortCount = PYDAW_COUNT;

	port_descriptors = (LADSPA_PortDescriptor *)
				calloc(LMSLDescriptor->PortCount, sizeof
						(LADSPA_PortDescriptor));
	LMSLDescriptor->PortDescriptors =
	    (const LADSPA_PortDescriptor *) port_descriptors;

	port_range_hints = (LADSPA_PortRangeHint *)
				calloc(LMSLDescriptor->PortCount, sizeof
						(LADSPA_PortRangeHint));
	LMSLDescriptor->PortRangeHints =
	    (const LADSPA_PortRangeHint *) port_range_hints;

	port_names = (char **) calloc(LMSLDescriptor->PortCount, sizeof(char *));
	LMSLDescriptor->PortNames = (const char **) port_names;

        /* Parameters for input */
        int f_i;
        
        for(f_i = PYDAW_INPUT_MIN; f_i < PYDAW_INPUT_MAX; f_i++)
        {
            port_descriptors[f_i] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
            port_names[f_i] = "Input ";  //TODO:  Give a more descriptive port name
            port_range_hints[f_i].HintDescriptor = 0;
        }
        
	/* Parameters for output */
	port_descriptors[PYDAW_OUTPUT0] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
	port_names[PYDAW_OUTPUT0] = "Output 0";
	port_range_hints[PYDAW_OUTPUT0].HintDescriptor = 0;

        port_descriptors[PYDAW_OUTPUT1] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
	port_names[PYDAW_OUTPUT1] = "Output 1";
	port_range_hints[PYDAW_OUTPUT1].HintDescriptor = 0;
               
        
	LMSLDescriptor->activate = v_pydaw_activate;
	LMSLDescriptor->cleanup = v_pydaw_cleanup;
	LMSLDescriptor->connect_port = v_pydaw_connect_port;
	LMSLDescriptor->deactivate = NULL;
	LMSLDescriptor->instantiate = g_pydaw_instantiate;
	LMSLDescriptor->run = NULL;
	LMSLDescriptor->run_adding = NULL;
	LMSLDescriptor->set_run_adding_gain = NULL;
    }

    LMSDDescriptor = (DSSI_Descriptor *) malloc(sizeof(DSSI_Descriptor));
    if (LMSDDescriptor) {
	LMSDDescriptor->DSSI_API_Version = 1;
	LMSDDescriptor->LADSPA_Plugin = LMSLDescriptor;
	LMSDDescriptor->configure = c_pydaw_configure;
	LMSDDescriptor->get_program = NULL;
	LMSDDescriptor->get_midi_controller_for_port = i_pydaw_get_controller;
	LMSDDescriptor->select_program = NULL;
	LMSDDescriptor->run_synth = v_pydaw_run;
	LMSDDescriptor->run_synth_adding = NULL;
	LMSDDescriptor->run_multiple_synths = NULL;
	LMSDDescriptor->run_multiple_synths_adding = NULL;
    }
}

#ifdef __GNUC__
__attribute__((destructor)) void v_pydaw_destructor()
#else
void _fini()
#endif
{
    v_pydaw_close_all_uis(pydaw_data);
    
    if (LMSLDescriptor) {
	free((LADSPA_PortDescriptor *) LMSLDescriptor->PortDescriptors);
	free((char **) LMSLDescriptor->PortNames);
	free((LADSPA_PortRangeHint *) LMSLDescriptor->PortRangeHints);
	free(LMSLDescriptor);
    }
    if (LMSDDescriptor) {
	free(LMSDDescriptor);
    }
}
