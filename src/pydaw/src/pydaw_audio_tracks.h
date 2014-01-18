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

#ifndef PYDAW_AUDIO_TRACKS_H
#define	PYDAW_AUDIO_TRACKS_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <sndfile.h>
#include "../libmodsynth/lib/amp.h"
#include "../libmodsynth/lib/pitch_core.h"
#include "../libmodsynth/lib/interpolate-cubic.h"
//Imported only for t_int_frac_read_head... TODO:  Fork that into it's own file...
#include "../libmodsynth/lib/interpolate-sinc.h"
#include "../libmodsynth/modules/modulation/adsr.h"
#include "../include/pydaw_plugin.h"
#include "pydaw_files.h"
#include "pydaw.h"

#define PYDAW_MAX_AUDIO_ITEM_COUNT 256
#define PYDAW_MAX_WAV_POOL_ITEM_COUNT 500

#define PYDAW_AUDIO_ITEM_PADDING 64
#define PYDAW_AUDIO_ITEM_PADDING_DIV2 32
#define PYDAW_AUDIO_ITEM_PADDING_DIV2_FLOAT 32.0f

t_wav_pool_item * g_wav_pool_item_get(int a_uid, const char *a_path, float a_sr)
{
    t_wav_pool_item *f_result;

    if(posix_memalign((void**)&f_result, 16, (sizeof(t_wav_pool_item))) != 0)
    {
        return 0;
    }

    f_result->uid = a_uid;
    f_result->is_loaded = 0;
    f_result->host_sr = a_sr;

    sprintf(f_result->path, "%s", a_path);

    return f_result;
}

int i_wav_pool_item_load(t_wav_pool_item *a_wav_pool_item)
{
    SF_INFO info;
    SNDFILE *file;
    size_t samples = 0;
    float *tmpFrames, *tmpSamples[2];

    info.format = 0;
    file = sf_open(a_wav_pool_item->path, SFM_READ, &info);

    if (!file) {

	const char *filename = strrchr(a_wav_pool_item->path, '/');
	if (filename) ++filename;
	else filename = a_wav_pool_item->path;

	if (!file) {
            printf("error: unable to load sample file '%s'", a_wav_pool_item->path);
	    return 0;
	}
    }

    samples = info.frames;

    tmpFrames = (float *)malloc(info.frames * info.channels * sizeof(float));
    sf_readf_float(file, tmpFrames, info.frames);
    sf_close(file);

    if ((int)(info.samplerate) != (int)(a_wav_pool_item->host_sr))
    {
	double ratio = (double)(info.samplerate)/(double)(a_wav_pool_item->host_sr);
        a_wav_pool_item->ratio_orig = (float)ratio;
    }
    else
    {
        a_wav_pool_item->ratio_orig = 1.0f;
    }

    int f_adjusted_channel_count = 1;
    if(info.channels >= 2)
    {
        f_adjusted_channel_count = 2;
    }

    int f_actual_array_size = (samples + PYDAW_AUDIO_ITEM_PADDING);

    if(posix_memalign((void**)(&(tmpSamples[0])), 16, ((f_actual_array_size) * sizeof(float))) != 0)
    {
        printf("Call to posix_memalign failed for tmpSamples[0]\n");
        return 0;
    }
    if(f_adjusted_channel_count > 1)
    {
        if(posix_memalign((void**)(&(tmpSamples[1])), 16, ((f_actual_array_size) * sizeof(float))) != 0)
        {
            printf("Call to posix_memalign failed for tmpSamples[1]\n");
            return 0;
        }
    }

    int f_i, j;

    //For performing a 5ms fadeout of the sample, for preventing clicks
    float f_fade_out_dec = (1.0f/(float)(info.samplerate))/(0.005);
    int f_fade_out_start = (samples + PYDAW_AUDIO_ITEM_PADDING_DIV2) - ((int)(0.005f * ((float)(info.samplerate))));
    float f_fade_out_envelope = 1.0f;
    float f_temp_sample = 0.0f;

    for(f_i = 0; f_i < f_actual_array_size; f_i++)
    {
        if((f_i > PYDAW_AUDIO_ITEM_PADDING_DIV2) && (f_i < (samples + PYDAW_AUDIO_ITEM_PADDING_DIV2))) // + Sampler_Sample_Padding)))
        {
            if(f_i >= f_fade_out_start)
            {
                if(f_fade_out_envelope <= 0.0f)
                {
                    f_fade_out_dec = 0.0f;
                }

                f_fade_out_envelope -= f_fade_out_dec;
            }

	    for (j = 0; j < f_adjusted_channel_count; ++j)
            {
                f_temp_sample = (tmpFrames[(f_i - PYDAW_AUDIO_ITEM_PADDING_DIV2) * info.channels + j]);

                if(f_i >= f_fade_out_start)
                {
                    tmpSamples[j][f_i] = f_temp_sample * f_fade_out_envelope;
                }
                else
                {
                    tmpSamples[j][f_i] = f_temp_sample;
                }
            }
        }
        else
        {
            tmpSamples[0][f_i] = 0.0f;
            if(f_adjusted_channel_count > 1)
            {
                tmpSamples[1][f_i] = 0.0f;
            }
        }
    }

    free(tmpFrames);

    a_wav_pool_item->samples[0] = tmpSamples[0];

    if(f_adjusted_channel_count > 1)
    {
        a_wav_pool_item->samples[1] = tmpSamples[1];
    }
    else
    {
        a_wav_pool_item->samples[1] = 0;
    }

    a_wav_pool_item->length = (samples + PYDAW_AUDIO_ITEM_PADDING_DIV2 - 20);  //-20 to ensure we don't read past the end of the array

    a_wav_pool_item->sample_rate = info.samplerate;

    a_wav_pool_item->channels = f_adjusted_channel_count;

    a_wav_pool_item->is_loaded = 1;

    return 1;
}

void v_wav_pool_item_free(t_wav_pool_item *a_wav_pool_item)
{
    a_wav_pool_item->path[0] = '\0';

    float *tmpOld[2];

    tmpOld[0] = a_wav_pool_item->samples[0];
    tmpOld[1] = a_wav_pool_item->samples[1];
    a_wav_pool_item->samples[0] = 0;
    a_wav_pool_item->samples[1] = 0;
    a_wav_pool_item->length = 0;

    if (tmpOld[0]) free(tmpOld[0]);
    if (tmpOld[1]) free(tmpOld[1]);
    free(a_wav_pool_item);
}

typedef struct
{
    t_wav_pool_item * wav_pool_item;  //pointer assigned when playing
    int wav_pool_uid;
    float ratio;
    int uid;
    int start_bar;
    float start_beat;
    float adjusted_start_beat;
    int timestretch_mode;  //tentatively: 0 == none, 1 == pitch, 2 == time+pitch
    float pitch_shift;
    float sample_start;
    float sample_end;
    int sample_start_offset;
    float sample_start_offset_float;
    int sample_end_offset;
    //The audio track whose Modulex instance to write the samples to
    int audio_track_output;
    t_int_frac_read_head * sample_read_head;
    t_adsr * adsr;
    int index;
    float vol;
    float vol_linear;

    float timestretch_amt;
    float sample_fade_in;
    float sample_fade_out;
    int sample_fade_in_end;
    int sample_fade_out_start;
    float sample_fade_in_divisor;
    float sample_fade_out_divisor;
    float fade_vol;

    t_amp * amp_ptr;
    t_pit_pitch_core * pitch_core_ptr;
    t_pit_ratio * pitch_ratio_ptr;

    float pitch_shift_end;
    float timestretch_amt_end;
    int is_reversed;
    float fadein_vol;
    float fadeout_vol;
    int paif_automation_uid;  //placeholder for future functionality
} t_pydaw_audio_item __attribute__((aligned(16)));

typedef struct
{
    float sample_rate;
    int count;
    t_wav_pool_item * items[PYDAW_MAX_WAV_POOL_ITEM_COUNT];
    char samples_folder[2048];  //This must be set when opening a project
}t_wav_pool;

t_wav_pool * g_wav_pool_get(float a_sr)
{
    t_wav_pool * f_result = (t_wav_pool*)malloc(sizeof(t_wav_pool));

    f_result->sample_rate = a_sr;
    f_result->count = 0;

    int f_i = 0;
    while(f_i < PYDAW_MAX_WAV_POOL_ITEM_COUNT)
    {
        f_result->items[f_i] = 0;
        f_i++;
    }
    return f_result;
}

void v_wav_pool_add_item(t_wav_pool* a_wav_pool, int a_uid, char * a_file_path)
{
    char f_path[2048];
    sprintf(f_path, "%s%s", a_wav_pool->samples_folder, a_file_path);
    t_wav_pool_item * f_result = g_wav_pool_item_get(a_uid, f_path,
            a_wav_pool->sample_rate);
    a_wav_pool->items[a_wav_pool->count] = f_result;
    a_wav_pool->count++;
}

/* Load entire pool at startup/open */
void v_wav_pool_add_items(t_wav_pool* a_wav_pool, char * a_file_path)
{
    a_wav_pool->count = 0;
    t_2d_char_array * f_arr = g_get_2d_array_from_file(a_file_path,
            PYDAW_LARGE_STRING);
    while(1)
    {
        char * f_uid_str = c_iterate_2d_char_array(f_arr);
        if(f_arr->eof)
        {
            free(f_uid_str);
            break;
        }
        int f_uid = atoi(f_uid_str);
        free(f_uid_str);
        char * f_file_path = c_iterate_2d_char_array_to_next_line(f_arr);
        v_wav_pool_add_item(a_wav_pool, f_uid, f_file_path);
        free(f_file_path);
    }
}

t_wav_pool_item * g_wav_pool_get_item_by_uid(t_wav_pool* a_wav_pool, int a_uid)
{
    int f_i = 0;
    while(f_i < a_wav_pool->count)
    {
        if(a_wav_pool->items[f_i] && a_wav_pool->items[f_i]->uid == a_uid)
        {
            if(!a_wav_pool->items[f_i]->is_loaded)
            {
                if(!i_wav_pool_item_load(a_wav_pool->items[f_i]))
                {
                    return 0;
                }
            }
            return a_wav_pool->items[f_i];
        }
        f_i++;
    }
    return 0;
}

typedef struct
{
    t_pydaw_audio_item * items[PYDAW_MAX_AUDIO_ITEM_COUNT];
    int sample_rate;
    t_cubic_interpolater * cubic_interpolator;
    int uid;
} t_pydaw_audio_items;


t_pydaw_audio_item * g_pydaw_audio_item_get(float);
t_pydaw_audio_items * g_pydaw_audio_items_get(int);
void v_pydaw_audio_item_free(t_pydaw_audio_item *);
//void v_audio_items_load(t_pydaw_audio_item *a_audio_item, const char *a_path,
//      float a_sr, int a_uid);

void v_pydaw_audio_item_free(t_pydaw_audio_item* a_audio_item)
{
    //TODO:  Create a free method for these...
    //if(a_audio_item->adsr)
    //{ }
    //if(a_audio_item->sample_read_head)
    //{}
    if(!a_audio_item)
    {
        return;
    }

    if(a_audio_item)
    {
        free(a_audio_item->adsr);
        free(a_audio_item->amp_ptr);
        free(a_audio_item->pitch_core_ptr);
        free(a_audio_item->sample_read_head);
        free(a_audio_item->pitch_ratio_ptr);
        free(a_audio_item);
    }
}

t_pydaw_audio_item * g_pydaw_audio_item_get(float a_sr)
{
    t_pydaw_audio_item * f_result;

    if(posix_memalign((void**)&f_result, 16, sizeof(t_pydaw_audio_item)) != 0)
    {
        return 0;
    }

    f_result->ratio = 1.0f;
    f_result->uid = -1;
    f_result->adsr = g_adsr_get_adsr(1.0f/a_sr);
    v_adsr_set_adsr_db(f_result->adsr, 0.003f, 0.1f, 0.0f, 0.2f);
    v_adsr_retrigger(f_result->adsr);
    f_result->sample_read_head = g_ifh_get();

    f_result->amp_ptr = g_amp_get();
    f_result->pitch_core_ptr = g_pit_get();
    f_result->pitch_ratio_ptr = g_pit_ratio();
    f_result->vol = 0.0f;
    f_result->vol_linear = 1.0f;

    return f_result;
}

t_pydaw_audio_items * g_pydaw_audio_items_get(int a_sr)
{
    t_pydaw_audio_items * f_result;

    if(posix_memalign((void**)&f_result, 16, sizeof(t_pydaw_audio_items)) != 0)
    {
        return 0;
    }

    f_result->sample_rate = a_sr;
    f_result->cubic_interpolator = g_cubic_get();

    int f_i = 0;

    while(f_i < PYDAW_MAX_AUDIO_ITEM_COUNT)
    {
        f_result->items[f_i] = 0; //g_pydaw_audio_item_get((float)(a_sr));
        f_i++;
    }

    return f_result;
}

/* t_pydaw_audio_item * g_audio_item_load_single(float a_sr,
 * t_2d_char_array * f_current_string,
 * t_pydaw_audio_items * a_items)
 *
 */
t_pydaw_audio_item * g_audio_item_load_single(float a_sr,
        t_2d_char_array * f_current_string,
        t_pydaw_audio_items * a_items, t_wav_pool * a_wav_pool,
        t_wav_pool_item * a_wav_item)
{
    t_pydaw_audio_item * f_result;

    char * f_index_char = c_iterate_2d_char_array(f_current_string);

    if(f_current_string->eof)
    {
        free(f_index_char);
        return 0;
    }

    int f_index = atoi(f_index_char);
    free(f_index_char);

    if(a_items)
    {
        f_result = a_items->items[f_index];
    }
    else
    {
        f_result = g_pydaw_audio_item_get(a_sr);
    }

    f_result->index = f_index;

    char * f_uid_char = c_iterate_2d_char_array(f_current_string);
    f_result->uid = atoi(f_uid_char);
    free(f_uid_char);

    if(a_wav_item)
    {
        f_result->wav_pool_item = a_wav_item;
    }
    else
    {
        f_result->wav_pool_item =
            g_wav_pool_get_item_by_uid(a_wav_pool, f_result->uid);

        if(!f_result->wav_pool_item)
        {
            printf("####################\n\n");
            printf("ERROR:  g_audio_item_load_single failed for uid %i, "
                    "not found\n\n", f_result->uid);
            printf("####################\n\n");
            return 0;
        }
    }
    char * f_sample_start_char = c_iterate_2d_char_array(f_current_string);
    float f_sample_start = atof(f_sample_start_char) * 0.001f;

    if(f_sample_start < 0.0f)
    {
        f_sample_start = 0.0f;
    }
    else if(f_sample_start > 0.999f)
    {
        f_sample_start = 0.999f;
    }

    f_result->sample_start = f_sample_start;
    free(f_sample_start_char);

    f_result->sample_start_offset =
            (int)((f_result->sample_start *
            ((float)f_result->wav_pool_item->length))) +
            PYDAW_AUDIO_ITEM_PADDING_DIV2;
    f_result->sample_start_offset_float =
            (float)(f_result->sample_start_offset);

    char * f_sample_end_char = c_iterate_2d_char_array(f_current_string);
    float f_sample_end = atof(f_sample_end_char) * 0.001f;

    if(f_sample_end < 0.001f)
    {
        f_sample_end = 0.001f;
    }
    else if(f_sample_end > 1.0f)
    {
        f_sample_end = 1.0f;
    }

    f_result->sample_end = f_sample_end;

    free(f_sample_end_char);

    f_result->sample_end_offset = (int)((f_result->sample_end *
            ((float)f_result->wav_pool_item->length)));

    char * f_start_bar_char = c_iterate_2d_char_array(f_current_string);
    f_result->start_bar = atoi(f_start_bar_char);
    free(f_start_bar_char);

    char * f_start_beat_char = c_iterate_2d_char_array(f_current_string);
    f_result->start_beat = atof(f_start_beat_char);
    free(f_start_beat_char);

    char * f_time_stretch_mode_char = c_iterate_2d_char_array(f_current_string);
    f_result->timestretch_mode = atoi(f_time_stretch_mode_char);
    free(f_time_stretch_mode_char);

    char * f_pitch_shift_char = c_iterate_2d_char_array(f_current_string);
    f_result->pitch_shift = atof(f_pitch_shift_char);
    free(f_pitch_shift_char);

    char * f_audio_track_output_char =
    c_iterate_2d_char_array(f_current_string);
    f_result->audio_track_output = atoi(f_audio_track_output_char);
    free(f_audio_track_output_char);

    char * f_vol_char = c_iterate_2d_char_array(f_current_string);
    f_result->vol = atof(f_vol_char);
    f_result->vol_linear = f_db_to_linear_fast(f_result->vol,
            f_result->amp_ptr);
    free(f_vol_char);

    char * f_timestretch_amt_char = c_iterate_2d_char_array(f_current_string);
    f_result->timestretch_amt = atof(f_timestretch_amt_char);
    free(f_timestretch_amt_char);

    char * f_fade_in_char = c_iterate_2d_char_array(f_current_string);
    f_result->sample_fade_in = atof(f_fade_in_char) * 0.001f;
    free(f_fade_in_char);

    char * f_fade_out_char = c_iterate_2d_char_array(f_current_string);
    f_result->sample_fade_out = atof(f_fade_out_char) * 0.001f;
    free(f_fade_out_char);

    //Not used by the back-end
    char * f_lane_num = c_iterate_2d_char_array(f_current_string);
    free(f_lane_num);


    char * f_pitch_shift_end_char = c_iterate_2d_char_array(f_current_string);
    f_result->pitch_shift_end = atof(f_pitch_shift_end_char);
    free(f_pitch_shift_end_char);

    char * f_timestretch_end_char = c_iterate_2d_char_array(f_current_string);
    f_result->timestretch_amt_end = atof(f_timestretch_end_char);
    free(f_timestretch_end_char);

    char * f_reversed_char = c_iterate_2d_char_array(f_current_string);
    f_result->is_reversed = atoi(f_reversed_char);
    free(f_reversed_char);

    //Not used in the back-end
    char * f_crispness_char = c_iterate_2d_char_array(f_current_string);
    free(f_crispness_char);

    //These are multiplied by -1.0f to work correctly with
    //v_pydaw_audio_item_set_fade_vol()
    char * f_fadein_vol_char = c_iterate_2d_char_array(f_current_string);
    f_result->fadein_vol = atof(f_fadein_vol_char) * -1.0f;
    free(f_fadein_vol_char);

    char * f_fadeout_vol_char = c_iterate_2d_char_array(f_current_string);
    f_result->fadeout_vol = atof(f_fadeout_vol_char) * -1.0f;
    free(f_fadeout_vol_char);

    char * f_paif_uid_char = c_iterate_2d_char_array(f_current_string);
    f_result->paif_automation_uid = atoi(f_paif_uid_char);
    free(f_paif_uid_char);

    if(f_result->sample_start_offset < PYDAW_AUDIO_ITEM_PADDING_DIV2)
    {
        printf("f_result->sample_start_offset <= PYDAW_AUDIO_ITEM_PADDING_DIV2"
                " %i %i\n", f_result->sample_start_offset,
                PYDAW_AUDIO_ITEM_PADDING_DIV2);
        f_result->sample_start_offset = PYDAW_AUDIO_ITEM_PADDING_DIV2;
    }

    if(f_result->sample_end_offset < PYDAW_AUDIO_ITEM_PADDING_DIV2)
    {
        printf("f_result->sample_end_offset <= PYDAW_AUDIO_ITEM_PADDING_DIV2"
                " %i %i\n", f_result->sample_end_offset,
                PYDAW_AUDIO_ITEM_PADDING_DIV2);
        f_result->sample_end_offset = PYDAW_AUDIO_ITEM_PADDING_DIV2;
    }

    if(f_result->sample_start_offset > f_result->wav_pool_item->length)
    {
        printf("f_result->sample_start_offset >= "
                "f_result->wav_pool_item->length %i %i\n",
                f_result->sample_start_offset, f_result->wav_pool_item->length);
        f_result->sample_start_offset = f_result->wav_pool_item->length;
    }

    if(f_result->sample_end_offset > f_result->wav_pool_item->length)
    {
        printf("f_result->sample_end_offset >= f_result->wav_pool_item->length"
                " %i %i\n", f_result->sample_end_offset,
                f_result->wav_pool_item->length);
        f_result->sample_end_offset = f_result->wav_pool_item->length;
    }

    if(f_result->is_reversed)
    {
        int f_old_start = f_result->sample_start_offset;
        int f_old_end = f_result->sample_end_offset;
        f_result->sample_start_offset =
                f_result->wav_pool_item->length - f_old_end;
        f_result->sample_start_offset_float =
                (float)(f_result->sample_start_offset);
        f_result->sample_end_offset =
                f_result->wav_pool_item->length - f_old_start;
    }

    f_result->sample_fade_in_end =
            f_result->sample_end_offset - f_result->sample_start_offset;
    f_result->sample_fade_in_end =
            f_result->sample_start_offset +
            ((int)((float)(f_result->sample_fade_in_end) *
            f_result->sample_fade_in)) + PYDAW_AUDIO_ITEM_PADDING_DIV2;

    f_result->sample_fade_out_start =
            f_result->sample_end_offset - f_result->sample_start_offset;
    f_result->sample_fade_out_start =
            f_result->sample_start_offset +
            ((int)((float)(f_result->sample_fade_out_start) *
            f_result->sample_fade_out)) + PYDAW_AUDIO_ITEM_PADDING_DIV2;

    int f_fade_diff = (f_result->sample_fade_in_end -
        f_result->sample_start_offset - (PYDAW_AUDIO_ITEM_PADDING_DIV2));

    if(f_fade_diff != 0)
    {
        f_result->sample_fade_in_divisor = 1.0f / (float)f_fade_diff;
    }
    else
    {
        f_result->sample_fade_in_divisor = 0.0f;
    }

    f_fade_diff =
            (f_result->sample_end_offset - f_result->sample_fade_out_start);

    if(f_fade_diff != 0)
    {
        f_result->sample_fade_out_divisor = 1.0f / (float)f_fade_diff;
    }
    else
    {
        f_result->sample_fade_out_divisor = 0.0f;
    }

    f_result->adjusted_start_beat =
            ((float)f_result->start_bar * 4) + f_result->start_beat;

    if(f_result->is_reversed)
    {
        f_result->sample_fade_in_end =
                f_result->sample_end_offset - (f_result->sample_fade_in_end -
                f_result->sample_start_offset);
        f_result->sample_fade_out_start =
                f_result->sample_start_offset + (f_result->sample_end_offset -
                f_result->sample_fade_out_start);
    }

    f_result->ratio = f_result->wav_pool_item->ratio_orig;

    switch(f_result->timestretch_mode)
    {
        //case 0:  //None
        //    break;
        case 1:  //Pitch affecting time
        {
            //Otherwise, it's already been stretched offline
            if(f_result->pitch_shift == f_result->pitch_shift_end)
            {
                if((f_result->pitch_shift) >= 0.0f)
                {
                    f_result->ratio *=
                            f_pit_midi_note_to_ratio_fast(0.0f,
                                (f_result->pitch_shift),
                                f_result->pitch_core_ptr,
                                f_result->pitch_ratio_ptr);
                }
                else
                {
                    f_result->ratio *=
                            f_pit_midi_note_to_ratio_fast(
                            ((f_result->pitch_shift) * -1.0f),
                            0.0f,
                            f_result->pitch_core_ptr,
                            f_result->pitch_ratio_ptr);
                }
            }
        }
            break;
        case 2:  //Time affecting pitch
        {
            //Otherwise, it's already been stretched offline
            if(f_result->timestretch_amt == f_result->timestretch_amt_end)
            {
                f_result->ratio *= (1.0f / (f_result->timestretch_amt));
            }
        }
            break;
        /*
        //Don't have to do anything with these, they comes pre-stretched...
        case 3:  //Rubberband
        case 4:  //Rubberband (preserving formants)
        case 5:  //SBSMS
        case 6: Paulstretch
        */
    }

    f_result->adsr->stage = 4;

    return f_result;
}

void v_pydaw_audio_item_set_fade_vol(t_pydaw_audio_item *a_audio_item)
{
    if(a_audio_item->is_reversed)
    {
        if(a_audio_item->sample_read_head->whole_number >
                a_audio_item->sample_fade_in_end &&
                a_audio_item->sample_fade_in_divisor != 0.0f)
        {
            a_audio_item->fade_vol =
                    ((float)(a_audio_item->sample_read_head->whole_number) -
                    a_audio_item->sample_fade_in_end)
                    * a_audio_item->sample_fade_in_divisor;
            a_audio_item->fade_vol =
                    ((1.0f - a_audio_item->fade_vol) * a_audio_item->fadein_vol)
                    - a_audio_item->fadein_vol;
            //a_audio_item->fade_vol = (a_audio_item->fade_vol * 40.0f) - 40.0f;
            a_audio_item->fade_vol =
                    f_db_to_linear_fast(a_audio_item->fade_vol,
                    a_audio_item->amp_ptr);
        }
        else if(a_audio_item->sample_read_head->whole_number <=
                a_audio_item->sample_fade_out_start &&
                a_audio_item->sample_fade_out_divisor != 0.0f)
        {
            a_audio_item->fade_vol =
                    ((float)(a_audio_item->sample_fade_out_start -
                    a_audio_item->sample_read_head->whole_number))
                    * a_audio_item->sample_fade_out_divisor;
            a_audio_item->fade_vol =
                ((1.0f - a_audio_item->fade_vol) * a_audio_item->fadeout_vol) -
                    a_audio_item->fadeout_vol;
            //a_audio_item->fade_vol =
            //  ((a_audio_item->fade_vol) * 40.0f) - 40.0f;
            a_audio_item->fade_vol =
                    f_db_to_linear_fast(a_audio_item->fade_vol,
                    a_audio_item->amp_ptr);
        }
        else
        {
            a_audio_item->fade_vol = 1.0f;
        }
    }
    else
    {
        if(a_audio_item->sample_read_head->whole_number <
                a_audio_item->sample_fade_in_end &&
                a_audio_item->sample_fade_in_divisor != 0.0f)
        {
            a_audio_item->fade_vol =
                    ((float)(a_audio_item->sample_fade_in_end -
                    a_audio_item->sample_read_head->whole_number))
                    * a_audio_item->sample_fade_in_divisor;
            a_audio_item->fade_vol =
                    ((1.0f - a_audio_item->fade_vol) * a_audio_item->fadein_vol)
                    - a_audio_item->fadein_vol;
            //a_audio_item->fade_vol =
            //  ((a_audio_item->fade_vol) * 40.0f) - 40.0f;
            a_audio_item->fade_vol =
                    f_db_to_linear_fast(a_audio_item->fade_vol,
                    a_audio_item->amp_ptr);
        }
        else if(a_audio_item->sample_read_head->whole_number >=
                a_audio_item->sample_fade_out_start &&
                a_audio_item->sample_fade_out_divisor != 0.0f)
        {
            a_audio_item->fade_vol =
                    ((float)(a_audio_item->sample_read_head->whole_number) -
                    a_audio_item->sample_fade_out_start)
                    * a_audio_item->sample_fade_out_divisor;
            a_audio_item->fade_vol =
                    ((1.0f - a_audio_item->fade_vol) *
                    a_audio_item->fadeout_vol) - a_audio_item->fadeout_vol;
            //a_audio_item->fade_vol = (a_audio_item->fade_vol * 40.0f) - 40.0f;
            a_audio_item->fade_vol =
                f_db_to_linear_fast(a_audio_item->fade_vol,
                    a_audio_item->amp_ptr);
        }
        else
        {
            a_audio_item->fade_vol = 1.0f;
        }
    }
}

void v_pydaw_audio_items_free(t_pydaw_audio_items *a_audio_items)
{
    int f_i = 0;
    while(f_i < PYDAW_MAX_AUDIO_ITEM_COUNT)
    {
        v_pydaw_audio_item_free(a_audio_items->items[f_i]);
        a_audio_items->items[f_i] = 0;
        f_i++;
    }

    free(a_audio_items);
}


#ifdef	__cplusplus
}
#endif

#endif	/* PYDAW_AUDIO_TRACKS_H */

