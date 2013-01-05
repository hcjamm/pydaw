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
#include "../../libmodsynth/lib/interpolate-cubic.h"
//Imported only for t_int_frac_read_head... TODO:  Fork that into it's own file...
#include "../../libmodsynth/lib/interpolate-sinc.h"
#include "../../libmodsynth/lib/voice.h"
#include "../../libmodsynth/modules/modulation/adsr.h"
#include "pydaw_files.h"
    
#define PYDAW_MAX_AUDIO_ITEM_COUNT 32
#define PYDAW_MAX_AUDIO_ITEM_POLY_VOICE_COUNT 64

#define PYDAW_AUDIO_ITEM_PADDING 64
#define PYDAW_AUDIO_ITEM_PADDING_DIV2 32
#define PYDAW_AUDIO_ITEM_PADDING_DIV2_FLOAT 32.0f
    
typedef struct 
{
    int bool_sample_loaded;
    char path[512];
    float * samples[2];
    int channels;
    int length;
    float ratio;
    int uid;
    int start_region;
    int start_bar;
    float start_beat;
    double adjusted_start_beat;
    double adjusted_end_beat;
    int end_mode;  //0 == full sample length, 1 == musical end time
    int end_region;
    int end_bar;
    float end_beat;
    int timestretch_mode;  //tentatively: 0 == none, 1 == pitch, 2 == time+pitch
    float pitch_shift;    
    float sample_start;
    float sample_end;
    int audio_track_output;  //The audio track whose Modulex instance to write the samples to    
    float item_sample_rate;
    t_int_frac_read_head * sample_read_head;
    t_adsr * adsr;
    int index;
} t_pydaw_audio_item __attribute__((aligned(16)));

typedef struct 
{
    t_pydaw_audio_item * items[PYDAW_MAX_AUDIO_ITEM_COUNT];
    int sample_rate;
    t_cubic_interpolater * cubic_interpolator;    
    t_voc_voices * voices;
} t_pydaw_audio_items;


t_pydaw_audio_item * g_pydaw_audio_item_get(float);
t_pydaw_audio_items * g_pydaw_audio_items_get(int);
void v_pydaw_audio_item_free(t_pydaw_audio_item *);
void v_audio_items_load(t_pydaw_audio_item *a_audio_item, const char *a_path, float a_sr);

void v_pydaw_audio_item_free(t_pydaw_audio_item* a_audio_item)
{
    //TODO:  Create a free method for these...  Not really a big deal though, well under a kilobyte,
    //the leakage from this would be nominal no matter what, it'd be virtually impossible to leak an entire megabyte this way
    //if(a_audio_item->adsr)
    //{ }
    //if(a_audio_item->sample_read_head)
    //{}
    
    if(a_audio_item->samples[0])
    {
        free(a_audio_item->samples[0]);
    }
    
    if(a_audio_item->samples[1])
    {
        free(a_audio_item->samples[1]);
    }
    
    if(a_audio_item)
    {
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
    
    f_result->path[0] = '\0';
    f_result->bool_sample_loaded = 0;    
    f_result->length = 0;
    f_result->samples[0] = 0;
    f_result->samples[1] = 0;    
    f_result->ratio = 1.0f;
    f_result->uid = -1;
    
    f_result->adsr = g_adsr_get_adsr(a_sr);
    v_adsr_set_adsr_db(f_result->adsr, 0.01f, 0.1f, 0.0f, 0.1f);
    v_adsr_retrigger(f_result->adsr);
    f_result->sample_read_head = g_ifh_get();
        
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
    f_result->voices = g_voc_get_voices(PYDAW_MAX_AUDIO_ITEM_POLY_VOICE_COUNT);
    f_result->cubic_interpolator = g_cubic_get();
    
    int f_i = 0;
    
    while(f_i < PYDAW_MAX_AUDIO_ITEM_COUNT)
    {
        f_result->items[f_i] = g_pydaw_audio_item_get((float)(a_sr));
        f_i++;
    }
    
    return f_result;
}

/* t_pydaw_audio_item * g_audio_item_load_single(float a_sr, t_2d_char_array * f_current_string, t_pydaw_audio_items * a_items)
 *
 *  Pass in zero for a_items to not re-use the existing item without reloading the wave.  DO pass it in if you wish to update 
 * the item without reloading.
 */
t_pydaw_audio_item * g_audio_item_load_single(float a_sr, t_2d_char_array * f_current_string, t_pydaw_audio_items * a_items)
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

    char * f_file_name_char = c_iterate_2d_char_array(f_current_string);

    if(strcmp(f_file_name_char, f_result->path))
    {
        v_audio_items_load(f_result, f_file_name_char, a_sr);
    }

    char * f_sample_start_char = c_iterate_2d_char_array(f_current_string);
    float f_sample_start = atof(f_sample_start_char) * 0.001f;
    f_result->sample_start = f_sample_start;
    free(f_sample_start_char);

    char * f_sample_end_char = c_iterate_2d_char_array(f_current_string);
    f_result->sample_end = atof(f_sample_end_char) * 0.001f;            
    free(f_sample_end_char);

    char * f_start_region_char = c_iterate_2d_char_array(f_current_string);
    f_result->start_region = atoi(f_start_region_char);            
    free(f_start_region_char);

    char * f_start_bar_char = c_iterate_2d_char_array(f_current_string);
    f_result->start_bar = atoi(f_start_bar_char);            
    free(f_start_bar_char);

    char * f_start_beat_char = c_iterate_2d_char_array(f_current_string);
    f_result->start_beat = atof(f_start_beat_char);            
    free(f_start_beat_char);

    char * f_end_mode_char = c_iterate_2d_char_array(f_current_string);
    f_result->end_mode = atoi(f_end_mode_char);
    free(f_end_mode_char);

    char * f_end_region_char = c_iterate_2d_char_array(f_current_string);
    f_result->end_region = atoi(f_end_region_char);
    free(f_end_region_char);

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

    free(f_file_name_char);

    f_result->adjusted_start_beat = 
            (((double)(f_result->start_region)) * 4.0f * 8.0f) +
            (((double)(f_result->start_bar)) * 4.0f) +
            (f_result->start_beat);

    f_result->adjusted_end_beat = 
        (((double)(f_result->end_region)) * 4.0f * 8.0f) +
        (((double)(f_result->end_bar)) * 4.0f) +
        (f_result->end_beat);
    
    return f_result;
}

void v_audio_items_load(t_pydaw_audio_item *a_audio_item, const char *a_path, float a_sr)
{   
    /*Add that index to the list of loaded samples to iterate though when playing, if not already added*/
            
    SF_INFO info;
    SNDFILE *file;
    size_t samples = 0;
    float *tmpFrames, *tmpSamples[2], *tmpOld[2];
        
    tmpOld[0] = 0;
    tmpOld[1] = 0;
    
    info.format = 0;
    file = sf_open(a_path, SFM_READ, &info);

    if (!file) {

	const char *filename = strrchr(a_path, '/');
	if (filename) ++filename;
	else filename = a_path;
        
	if (!file) {
            printf("error: unable to load sample file '%s'", a_path);
	    return;
	}
    }
        
    samples = info.frames;

    tmpFrames = (float *)malloc(info.frames * info.channels * sizeof(float));
    sf_readf_float(file, tmpFrames, info.frames);
    sf_close(file);

    if ((int)(info.samplerate) != (int)(a_sr)) 
    {	
	double ratio = (double)(info.samplerate)/(double)(a_sr);
	
        a_audio_item->ratio = (float)ratio;
    }
    else
    {
        a_audio_item->ratio = 1.0f;
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
        return;
    }
    if(f_adjusted_channel_count > 1)
    {
        if(posix_memalign((void**)(&(tmpSamples[1])), 16, ((f_actual_array_size) * sizeof(float))) != 0)
        {
            printf("Call to posix_memalign failed for tmpSamples[1]\n");
            return;
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
    
    //pthread_mutex_lock(&plugin_data->mutex);
    
    if(a_audio_item->samples[0])
    {
        tmpOld[0] = a_audio_item->samples[0];
    }
    
    if(a_audio_item->samples[1])
    {
        tmpOld[1] = a_audio_item->samples[1];
    }
    
    a_audio_item->samples[0] = tmpSamples[0];
    
    if(f_adjusted_channel_count > 1)
    {
        a_audio_item->samples[1] = tmpSamples[1];
    }
    else
    {
        a_audio_item->samples[1] = 0;
    }
    
    a_audio_item->length = (samples + PYDAW_AUDIO_ITEM_PADDING_DIV2 - 20);  //-20 to ensure we don't read past the end of the array
    
    a_audio_item->item_sample_rate = info.samplerate;
    
    a_audio_item->channels = f_adjusted_channel_count;
    
    sprintf(a_audio_item->path, "%s", a_path);
        
    a_audio_item->bool_sample_loaded = 1;
    //pthread_mutex_unlock(&plugin_data->mutex);
    
    if (tmpOld[0]) 
    {
        free(tmpOld[0]);
    }
    
    if (tmpOld[1]) 
    {
        free(tmpOld[1]);
    }    
}

void v_audio_items_sample_clear(t_pydaw_audio_item *a_audio_item)
{
    a_audio_item->path[0] = '\0';
    a_audio_item->bool_sample_loaded = 0;
        
    float *tmpOld[2];    

    tmpOld[0] = a_audio_item->samples[0];
    tmpOld[1] = a_audio_item->samples[1];
    a_audio_item->samples[0] = 0;
    a_audio_item->samples[1] = 0;
    a_audio_item->length = 0;

    if (tmpOld[0]) free(tmpOld[0]);
    if (tmpOld[1]) free(tmpOld[1]);    
}

/*Load/Reload samples from file...*/
void v_audio_items_load_all(t_pydaw_audio_items * a_pydaw_audio_items, char * a_file)
{
    if(i_pydaw_file_exists(a_file))
    {
        printf("v_audio_items_load_all: loading a_file: \"%s\"\n", a_file);
        int f_i = 0;

        t_2d_char_array * f_current_string = g_get_2d_array_from_file(a_file, LMS_LARGE_STRING);        
        
        while(f_i < PYDAW_MAX_AUDIO_ITEM_COUNT)
        {            
            t_pydaw_audio_item * f_new =  g_audio_item_load_single(a_pydaw_audio_items->sample_rate, f_current_string, 0);
            if(!f_new)  //EOF'd...
            {
                break;
            }
            t_pydaw_audio_item * f_old = a_pydaw_audio_items->items[f_new->index];
            a_pydaw_audio_items->items[f_new->index] = f_new;            
            v_pydaw_audio_item_free(f_old);
            
            f_i++;
        }

        g_free_2d_char_array(f_current_string);
    }
    else
    {
        printf("Error:  v_audio_items_load_all:  a_file: \"%s\" does not exist\n", a_file);
    }
}


#ifdef	__cplusplus
}
#endif

#endif	/* PYDAW_AUDIO_TRACKS_H */

