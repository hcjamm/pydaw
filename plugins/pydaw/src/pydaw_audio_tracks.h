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
#include "../../libmodsynth/lib/amp.h"
#include "../../libmodsynth/lib/pitch_core.h"
#include "../../libmodsynth/lib/interpolate-cubic.h"
//Imported only for t_int_frac_read_head... TODO:  Fork that into it's own file...
#include "../../libmodsynth/lib/interpolate-sinc.h"
#include "../../libmodsynth/modules/modulation/adsr.h"
#include "pydaw_files.h"
#include "pydaw.h"
    
#define PYDAW_MAX_AUDIO_ITEM_COUNT 32
#define PYDAW_MAX_WAV_POOL_ITEM_COUNT 500

#define PYDAW_AUDIO_ITEM_PADDING 64
#define PYDAW_AUDIO_ITEM_PADDING_DIV2 32
#define PYDAW_AUDIO_ITEM_PADDING_DIV2_FLOAT 32.0f
    
typedef struct
{
    char path[512];
    int uid;
    float * samples[2];
    float ratio_orig;
    int channels;
    int length;
    float sample_rate;    
}t_wav_pool_item;

t_wav_pool_item * g_wav_pool_item_get(int a_uid, const char *a_path, float a_sr)
{   
    t_wav_pool_item *f_result;
    
    if(posix_memalign((void**)&f_result, 16, (sizeof(t_wav_pool_item))) != 0)
    {
        return 0;
    }
    
    f_result->uid = a_uid;
                
    SF_INFO info;
    SNDFILE *file;
    size_t samples = 0;
    float *tmpFrames, *tmpSamples[2];
    
    info.format = 0;
    file = sf_open(a_path, SFM_READ, &info);

    if (!file) {

	const char *filename = strrchr(a_path, '/');
	if (filename) ++filename;
	else filename = a_path;
        
	if (!file) {
            printf("error: unable to load sample file '%s'", a_path);
	    return 0;
	}
    }
        
    samples = info.frames;

    tmpFrames = (float *)malloc(info.frames * info.channels * sizeof(float));
    sf_readf_float(file, tmpFrames, info.frames);
    sf_close(file);

    if ((int)(info.samplerate) != (int)(a_sr)) 
    {	
	double ratio = (double)(info.samplerate)/(double)(a_sr);
        f_result->ratio_orig = (float)ratio;
    }
    else
    {
        f_result->ratio_orig = 1.0f;
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
    
    f_result->samples[0] = tmpSamples[0];
    
    if(f_adjusted_channel_count > 1)
    {
        f_result->samples[1] = tmpSamples[1];
    }
    else
    {
        f_result->samples[1] = 0;
    }
    
    f_result->length = (samples + PYDAW_AUDIO_ITEM_PADDING_DIV2 - 20);  //-20 to ensure we don't read past the end of the array
    
    f_result->sample_rate = info.samplerate;
    
    f_result->channels = f_adjusted_channel_count;
    
    sprintf(f_result->path, "%s", a_path);
    
    return f_result;
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
    //int start_region;
    int start_bar;
    float start_beat;
    double adjusted_start_beat;
    double adjusted_end_beat;
    int end_mode;  //0 == full sample length, 1 == musical end time
    //int end_region;
    int end_bar;
    float end_beat;
    int timestretch_mode;  //tentatively: 0 == none, 1 == pitch, 2 == time+pitch
    float pitch_shift;    
    float sample_start;
    float sample_end;
    int sample_start_offset;
    float sample_start_offset_float;
    int sample_end_offset;
    int audio_track_output;  //The audio track whose Modulex instance to write the samples to        
    t_int_frac_read_head * sample_read_head;
    t_adsr * adsr;
    int index;
    float vol;
    float vol_linear;
    
    float timestretch_amt;
    int sample_fade_in;
    int sample_fade_out;
    
    t_amp * amp_ptr;
    t_pit_pitch_core * pitch_core_ptr;
    t_pit_ratio * pitch_ratio_ptr;
} t_pydaw_audio_item __attribute__((aligned(16)));

typedef struct
{
    float sample_rate;
    int count;
    t_wav_pool_item * items[PYDAW_MAX_WAV_POOL_ITEM_COUNT];
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
    t_wav_pool_item * f_result = g_wav_pool_item_get(a_uid, a_file_path, a_wav_pool->sample_rate);
    a_wav_pool->items[a_wav_pool->count] = f_result;
    a_wav_pool->count++;
}

/* Load entire pool at startup/open */
void v_wav_pool_add_items(t_wav_pool* a_wav_pool, char * a_file_path)
{
    a_wav_pool->count = 0;
    t_2d_char_array * f_arr = g_get_2d_array_from_file(a_file_path, LMS_LARGE_STRING);
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
        if(a_wav_pool->items[f_i]->uid == a_uid)
        {
            return a_wav_pool->items[f_i];
        }        
        f_i++;
    }
    return NULL;
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
//void v_audio_items_load(t_pydaw_audio_item *a_audio_item, const char *a_path, float a_sr, int a_uid);

void v_pydaw_audio_item_free(t_pydaw_audio_item* a_audio_item)
{
    //TODO:  Create a free method for these...  Not really a big deal though, well under a kilobyte,
    //the leakage from this would be nominal no matter what, it'd be virtually impossible to leak an entire megabyte this way
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
    f_result->adjusted_start_beat = 99999999.0f;
    
    f_result->adsr = g_adsr_get_adsr(1.0f/a_sr);
    v_adsr_set_adsr_db(f_result->adsr, 0.003f, 0.1f, 0.0f, 0.2f);
    v_adsr_retrigger(f_result->adsr);
    f_result->sample_read_head = g_ifh_get();
    
    f_result->amp_ptr = g_amp_get();
    f_result->pitch_core_ptr = g_pit_get();
    f_result->pitch_ratio_ptr = g_pit_ratio();
    f_result->vol = 0.0f;
    f_result->vol_linear = 1.0f;
    //f_result->start_region = 0;
    //f_result->end_region = 0;
        
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

/* t_pydaw_audio_item * g_audio_item_load_single(float a_sr, t_2d_char_array * f_current_string, t_pydaw_audio_items * a_items)
 *
 *  Pass in zero for a_items to not re-use the existing item without reloading the wave.  DO pass it in if you wish to update 
 * the item without reloading.
 */
t_pydaw_audio_item * g_audio_item_load_single(float a_sr, t_2d_char_array * f_current_string, 
        t_pydaw_audio_items * a_items, t_wav_pool * a_wav_pool)
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
    
    f_result->wav_pool_item = g_wav_pool_get_item_by_uid(a_wav_pool, f_result->uid);

    char * f_sample_start_char = c_iterate_2d_char_array(f_current_string);
    float f_sample_start = atof(f_sample_start_char) * 0.001f;
    f_result->sample_start = f_sample_start;
    free(f_sample_start_char);
    
    f_result->sample_start_offset = (int)((f_result->sample_start * ((float)f_result->wav_pool_item->length))) + PYDAW_AUDIO_ITEM_PADDING_DIV2;
    f_result->sample_start_offset_float = (float)(f_result->sample_start_offset);

    char * f_sample_end_char = c_iterate_2d_char_array(f_current_string);
    f_result->sample_end = atof(f_sample_end_char) * 0.001f;            
    free(f_sample_end_char);

    f_result->sample_end_offset = (int)((f_result->sample_end * ((float)f_result->wav_pool_item->length))) + PYDAW_AUDIO_ITEM_PADDING_DIV2;

    char * f_start_bar_char = c_iterate_2d_char_array(f_current_string);
    f_result->start_bar = atoi(f_start_bar_char);            
    free(f_start_bar_char);

    char * f_start_beat_char = c_iterate_2d_char_array(f_current_string);
    f_result->start_beat = atof(f_start_beat_char);            
    free(f_start_beat_char);

    char * f_end_mode_char = c_iterate_2d_char_array(f_current_string);
    f_result->end_mode = atoi(f_end_mode_char);
    free(f_end_mode_char);

    char * f_end_bar_char = c_iterate_2d_char_array(f_current_string);
    f_result->end_bar = atoi(f_end_bar_char);
    free(f_end_bar_char);

    char * f_end_beat_char = c_iterate_2d_char_array(f_current_string);
    f_result->end_beat = atof(f_end_beat_char);
    free(f_end_beat_char);

    char * f_time_stretch_mode_char = c_iterate_2d_char_array(f_current_string);
    f_result->timestretch_mode = atoi(f_time_stretch_mode_char);
    free(f_time_stretch_mode_char);

    char * f_pitch_shift_char = c_iterate_2d_char_array(f_current_string);
    f_result->pitch_shift = atof(f_pitch_shift_char);
    free(f_pitch_shift_char);

    char * f_audio_track_output_char = c_iterate_2d_char_array(f_current_string);
    f_result->audio_track_output = atoi(f_audio_track_output_char);
    free(f_audio_track_output_char);

    char * f_vol_char = c_iterate_2d_char_array(f_current_string);
    f_result->vol = atof(f_vol_char);
    f_result->vol_linear = f_db_to_linear_fast(f_result->vol, f_result->amp_ptr);
    free(f_vol_char);
    
    char * f_timestretch_amt_char = c_iterate_2d_char_array(f_current_string);
    f_result->timestretch_amt = atof(f_timestretch_amt_char);
    free(f_timestretch_amt_char);
    
    char * f_fade_in_char = c_iterate_2d_char_array(f_current_string);
    f_result->sample_fade_in = atoi(f_fade_in_char);
    free(f_fade_in_char);
        
    char * f_fade_out_char = c_iterate_2d_char_array(f_current_string);
    f_result->sample_fade_out = atoi(f_fade_out_char);
    free(f_fade_out_char);
    
    char * f_lane_num_char = c_iterate_2d_char_array(f_current_string);    
    free(f_lane_num_char);  //not used by the engine
        
    f_result->ratio = f_result->wav_pool_item->ratio_orig;  //Otherwise the pitch/time shifting gets re-applied every time...
    
    switch(f_result->timestretch_mode)
    {
        case 0:  //None
            break;
        case 1:  //Pitch affecting time
        {
            if((f_result->pitch_shift) >= 0.0f)
            {
                f_result->ratio *= f_pit_midi_note_to_ratio_fast(0.0f, (f_result->pitch_shift), 
                        f_result->pitch_core_ptr, f_result->pitch_ratio_ptr);
            }
            else
            {
                f_result->ratio *= f_pit_midi_note_to_ratio_fast(((f_result->pitch_shift) * -1.0f), 0.0f, 
                        f_result->pitch_core_ptr, f_result->pitch_ratio_ptr);
            }
        }
            break;
        case 2:  //Time affecting pitch
        {
            f_result->ratio *= (f_result->timestretch_amt);
        }
            break;
    }
    
    f_result->adsr->stage = 4;
    
    return f_result;
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

