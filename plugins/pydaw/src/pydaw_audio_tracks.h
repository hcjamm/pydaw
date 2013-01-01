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

#define PYDAW_MAX_AUDIO_ITEM_COUNT 32
#define PYDAW_AUDIO_ITEM_PADDING 64
#define PYDAW_AUDIO_ITEM_PADDING_DIV2 32
    
typedef struct 
{
    char path[512];
    float * samples[2];
    int channels;
    int length;
    float ratio;
    int uid;
    int start_region;
    int start_bar;
    float start_beat;
    int end_mode;  //0 == full sample length, 1 == musical end time
    int end_region;
    int end_bar;
    float end_beat;
    int timestretch_mode;  //tentatively: 0 == none, 1 == pitch, 2 == time+pitch
    float pitch_shift;
} t_pydaw_audio_item;

typedef struct 
{
    t_pydaw_audio_item * items[PYDAW_MAX_AUDIO_ITEM_COUNT];
    int sample_rate;
    t_cubic_interpolater * cubic_interpolator;
} t_pydaw_audio_items;



t_pydaw_audio_item * g_pydaw_audio_item_get();
t_pydaw_audio_items * g_pydaw_audio_items_get(int);

t_pydaw_audio_item * g_pydaw_audio_item_get()
{
    t_pydaw_audio_item * f_result;
    
    if(posix_memalign((void**)&f_result, 16, sizeof(t_pydaw_audio_item)) != 0)
    {     
        return 0;
    }
    
    f_result->path[0] = '\0';
    f_result->length = 0;
    f_result->samples[0] = 0;
    f_result->samples[1] = 0;
    
    return f_result;
}

t_pydaw_audio_items * g_pydaw_audio_items_get(int a_sample_rate)
{
    t_pydaw_audio_items * f_result;
    
    if(posix_memalign((void**)&f_result, 16, sizeof(t_pydaw_audio_items)) != 0)
    {     
        return 0;
    }
    
    f_result->sample_rate = a_sample_rate;
    int f_i = 0;
    
    while(f_i < PYDAW_MAX_AUDIO_ITEM_COUNT)
    {
        f_result->items[f_i] = g_pydaw_audio_item_get();
        f_i++;
    }
    
    return f_result;
}

void v_audio_items_load(t_pydaw_audio_items *plugin_data, const char *path, int a_index)
{   
    /*Add that index to the list of loaded samples to iterate though when playing, if not already added*/
            
    SF_INFO info;
    SNDFILE *file;
    size_t samples = 0;
    float *tmpFrames, *tmpSamples[2], *tmpOld[2];
        
    tmpOld[0] = 0;
    tmpOld[1] = 0;
    
    info.format = 0;
    file = sf_open(path, SFM_READ, &info);

    if (!file) {

	const char *filename = strrchr(path, '/');
	if (filename) ++filename;
	else filename = path;
        
	if (!file) {
            printf("error: unable to load sample file '%s'", path);
	    return;
	}
    }
        
    samples = info.frames;

    tmpFrames = (float *)malloc(info.frames * info.channels * sizeof(float));
    sf_readf_float(file, tmpFrames, info.frames);
    sf_close(file);

    if ((int)(info.samplerate) != (int)(plugin_data->sample_rate)) 
    {	
	double ratio = (double)(info.samplerate)/(double)(plugin_data->sample_rate);
	
        plugin_data->items[(a_index)]->ratio = (float)ratio;
    }
    else
    {
        plugin_data->items[(a_index)]->ratio = 1.0f;
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
    
    if(plugin_data->items[a_index]->samples[0])
    {
        tmpOld[0] = plugin_data->items[a_index]->samples[0];
    }
    
    if(plugin_data->items[a_index]->samples[1])
    {
        tmpOld[1] = plugin_data->items[a_index]->samples[1];
    }
    
    plugin_data->items[a_index]->samples[0] = tmpSamples[0];
    
    if(f_adjusted_channel_count > 1)
    {
        plugin_data->items[a_index]->samples[1] = tmpSamples[1];
    }
    else
    {
        plugin_data->items[a_index]->samples[1] = 0;
    }
    
    plugin_data->items[a_index]->length = (samples + PYDAW_AUDIO_ITEM_PADDING_DIV2 - 20);  //-20 to ensure we don't read past the end of the array
    
    plugin_data->items[a_index]->channels = f_adjusted_channel_count;
    
    sprintf(plugin_data->items[a_index]->path, "%s", path);
        
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

void v_audio_items_sample_clear(t_pydaw_audio_items *plugin_data, int a_index)
{
    plugin_data->items[a_index]->path[0] = '\0';
        
    float *tmpOld[2];    
    
    //pthread_mutex_lock(&plugin_data->mutex);

    tmpOld[0] = plugin_data->items[a_index]->samples[0];
    tmpOld[1] = plugin_data->items[a_index]->samples[1];
    plugin_data->items[a_index]->samples[0] = 0;
    plugin_data->items[a_index]->samples[1] = 0;
    plugin_data->items[a_index]->length = 0;

    //pthread_mutex_unlock(&plugin_data->mutex);
    
    if (tmpOld[0]) free(tmpOld[0]);
    if (tmpOld[1]) free(tmpOld[1]);    
}

/*Load/Reload samples from file...*/
void v_audio_items_load_all(t_pydaw_audio_items * a_pydaw_audio_items, const char * a_file)
{
    //TODO
}


#ifdef	__cplusplus
}
#endif

#endif	/* PYDAW_AUDIO_TRACKS_H */

