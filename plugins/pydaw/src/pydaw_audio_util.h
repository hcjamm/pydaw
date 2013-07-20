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

#ifndef PYDAW_AUDIO_UTIL_H
#define	PYDAW_AUDIO_UTIL_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <sndfile.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include "../../libmodsynth/lib/interpolate-cubic.h"
#include "../../libmodsynth/lib/pitch_core.h"
    
    
/*Convert a wav file to 32bit float and normalize, for use with Paulstretch*/
void v_pydaw_convert_wav_to_32_bit_float(char * a_file_in, char * a_file_out)
{    
    SF_INFO info;
    SNDFILE *file;    
    float *tmpFrames;        
    float f_max = 0.0;
            
    info.format = 0;
    file = sf_open(a_file_in, SFM_READ, &info);

    if (!file) 
    {
	const char *filename = strrchr(a_file_in, '/');
	if (filename) ++filename;
	else filename = a_file_in;
        
	if (!file) 
        {
	    assert(0);
	}
    }
    
    if (info.frames > 100000000)
    {
	//TODO:  Something, anything....
    }

    //!!! complain also if more than 2 channels
    
    tmpFrames = (float *)malloc(info.frames * info.channels * sizeof(float));
    sf_readf_float(file, tmpFrames, info.frames);
    
    SF_INFO f_sf_info;
    f_sf_info.channels = info.channels;
    f_sf_info.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;
    f_sf_info.samplerate = info.samplerate;
    sf_close(file);
    
    int f_i = 0;
    
    while(f_i < info.frames * info.channels)
    {
        if(tmpFrames[f_i] > 0.0f && tmpFrames[f_i] > f_max)
        {
            f_max = tmpFrames[f_i];
        }
        else if(tmpFrames[f_i] < 0.0f && (tmpFrames[f_i] * -1.0f) > f_max)
        {
            f_max = tmpFrames[f_i] * -1.0f;
        }        
        f_i++;
    }
    
    printf("\n\nf_max = %f\n\n\n", f_max);
    if(f_max > 0.99f)
    {
        float f_normalize = 0.99f / f_max;
        printf("\n\nNormalizing, f_normalizing = %f\n\n\n", f_normalize);
        f_i = 0;
        while(f_i < (info.frames * info.channels))
        {
            tmpFrames[f_i] = f_normalize * tmpFrames[f_i];
            f_i++;
        }
    }
           
    SNDFILE * f_sndfile = sf_open(a_file_out, SFM_WRITE, &f_sf_info);
    sf_writef_float(f_sndfile, tmpFrames, info.frames);    
    sf_close(f_sndfile);
        
    char f_tmp_finished[256];    
    sprintf(f_tmp_finished, "%s.finished", a_file_out);    
    FILE * f_finished = fopen(f_tmp_finished, "w");    
    fclose(f_finished);  
    
    free(tmpFrames);
}



/*For time(affecting pitch) time stretching...  Since this is done
 offline anyways, it's not super optimized... */
void v_pydaw_rate_envelope(char * a_file_in, char * a_file_out, float a_start_rate, float a_end_rate)
{    
    SF_INFO info;
    SNDFILE *file;    
    float *tmpFrames;        
                
    info.format = 0;
    file = sf_open(a_file_in, SFM_READ, &info);

    if (!file) 
    {
	const char *filename = strrchr(a_file_in, '/');
	if (filename) ++filename;
	else filename = a_file_in;
        
	if (!file) 
        {
	    assert(0);
	}
    }
    
    if (info.frames > 100000000)
    {
	//TODO:  Something, anything....
    }

    //!!! complain also if more than 2 channels
    
    tmpFrames = (float *)malloc(info.frames * info.channels * sizeof(float));
    sf_readf_float(file, tmpFrames, info.frames);
    
    SF_INFO f_sf_info;
    f_sf_info.channels = info.channels;
    f_sf_info.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;
    f_sf_info.samplerate = info.samplerate;
    sf_close(file);
    
    t_cubic_interpolater * f_cubic = g_cubic_get();
        
    float f_sample_pos = 0.0;
                
    long f_size = 0;
    long f_block_size = 5000;
    
    float * f_output = (float*)malloc(sizeof(float) * (f_block_size * 2)); //f_sample_count_long);
        
    float * f_buffer0 = 0;
    float * f_buffer1 = 0;
    int f_i = 0;
    
    if(info.channels == 1)
    {
        f_buffer0 = tmpFrames;
    }
    else if(info.channels == 2)
    {
        f_buffer0 = (float*)malloc(sizeof(float) * info.frames);
        f_buffer1 = (float*)malloc(sizeof(float) * info.frames);
        
        int f_i2 = 0;
        //De-interleave...
        while(f_i < (info.frames * 2))
        {
            f_buffer0[f_i2] = tmpFrames[f_i];
            f_i++;
            f_buffer1[f_i2] = tmpFrames[f_i];
            f_i++;
            f_i2++;
        }
    }
    else
    {
        printf("\n\nDamn, I really didn't anticipate people loading wav files with");
        printf("more than 2 channels, I'll consider fixing it when somebody complains, barfing for now...\n\n");
        assert(0);
    }
    
    SNDFILE * f_sndfile = sf_open(a_file_out, SFM_WRITE, &f_sf_info);
    
    while(((int)f_sample_pos) < info.frames)
    {
        f_size = 0;
        
        while(f_size < f_block_size)
        {            
            if(info.channels == 1)
            {
                f_output[f_size] = f_cubic_interpolate_ptr_wrap(f_buffer0, info.frames, f_sample_pos, f_cubic);
                f_size++;
            }
            else if(info.channels == 2)
            {
                f_output[f_size] = f_cubic_interpolate_ptr_wrap(f_buffer0, info.frames, f_sample_pos, f_cubic);
                f_size++;
                f_output[f_size] = f_cubic_interpolate_ptr_wrap(f_buffer1, info.frames, f_sample_pos, f_cubic);
                f_size++;                
            }         
            
            double f_rate = (((double)(a_end_rate - a_start_rate)) * 
                (f_sample_pos / ((double)(info.frames)))) + a_start_rate;
            
            f_sample_pos += f_rate;
            
            if((int)f_sample_pos >= info.frames)
            {
                break;
            }
        }
        
        if(info.channels == 1)
        {
            sf_writef_float(f_sndfile, f_output, f_size);
        }
        else if(info.channels == 2)
        {
            sf_writef_float(f_sndfile, f_output, f_size / 2);
        }
    }
        
    sf_close(f_sndfile);
        
    char f_tmp_finished[256];    
    sprintf(f_tmp_finished, "%s.finished", a_file_out);    
    FILE * f_finished = fopen(f_tmp_finished, "w");    
    fclose(f_finished); 
    free(f_output);
    free(f_buffer0);    
    if(f_buffer1)
    {
        free(f_buffer1);
    }
    if(info.channels > 1)
    {
        free(tmpFrames);
    }
}



/*For pitch(affecting time) pitch shifting...  Since this is done
 offline anyways, it's not super optimized... */
void v_pydaw_pitch_envelope(char * a_file_in, char * a_file_out, float a_start_pitch, float a_end_pitch)
{    
    SF_INFO info;
    SNDFILE *file;    
    float *tmpFrames;        
                
    info.format = 0;
    file = sf_open(a_file_in, SFM_READ, &info);

    if (!file) 
    {
	const char *filename = strrchr(a_file_in, '/');
	if (filename) ++filename;
	else filename = a_file_in;
        
	if (!file) 
        {
	    assert(0);
	}
    }
    
    if (info.frames > 100000000)
    {
	//TODO:  Something, anything....
    }

    //!!! complain also if more than 2 channels
    
    tmpFrames = (float *)malloc(info.frames * info.channels * sizeof(float));
    sf_readf_float(file, tmpFrames, info.frames);
    
    SF_INFO f_sf_info;
    f_sf_info.channels = info.channels;
    f_sf_info.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;
    f_sf_info.samplerate = info.samplerate;
    sf_close(file);
    
    t_cubic_interpolater * f_cubic = g_cubic_get();
        
    float f_sample_pos = 0.0;
                
    long f_size = 0;
    long f_block_size = 10000;
    
    float * f_output = (float*)malloc(sizeof(float) * (f_block_size * 2)); //f_sample_count_long);
        
    float * f_buffer0 = 0;
    float * f_buffer1 = 0;
    int f_i = 0;
    
    t_pit_ratio * f_pit_ratio = g_pit_ratio();
    t_pit_pitch_core * f_pit_core = g_pit_get();
    
    if(info.channels == 1)
    {
        f_buffer0 = tmpFrames;
    }
    else if(info.channels == 2)
    {
        f_buffer0 = (float*)malloc(sizeof(float) * info.frames);
        f_buffer1 = (float*)malloc(sizeof(float) * info.frames);
        
        int f_i2 = 0;
        //De-interleave...
        while(f_i < (info.frames * 2))
        {
            f_buffer0[f_i2] = tmpFrames[f_i];
            f_i++;
            f_buffer1[f_i2] = tmpFrames[f_i];
            f_i++;
            f_i2++;
        }
    }
    else
    {
        printf("\n\nDamn, I really didn't anticipate people loading wav files with");
        printf("more than 2 channels, I'll consider fixing it when somebody complains, barfing for now...\n\n");
        assert(0);
    }
    
    SNDFILE * f_sndfile = sf_open(a_file_out, SFM_WRITE, &f_sf_info);
    
    while(((int)f_sample_pos) < info.frames)
    {
        f_size = 0;
                
        //Interleave the samples...
        while(f_size < f_block_size)
        {            
            if(info.channels == 1)
            {
                f_output[f_size] = f_cubic_interpolate_ptr_wrap(f_buffer0, info.frames, f_sample_pos, f_cubic);
                f_size++;
            }
            else if(info.channels == 2)
            {
                f_output[f_size] = f_cubic_interpolate_ptr_wrap(f_buffer0, info.frames, f_sample_pos, f_cubic);
                f_size++;
                f_output[f_size] = f_cubic_interpolate_ptr_wrap(f_buffer1, info.frames, f_sample_pos, f_cubic);
                f_size++;                
            }         
            
            double f_rate = (((double)(a_end_pitch - a_start_pitch)) * 
                (f_sample_pos / ((double)(info.frames)))) + a_start_pitch;
            
            float f_inc = f_pit_midi_note_to_ratio_fast(60.0f, f_rate + 60.0f, f_pit_core, f_pit_ratio);
            
            f_sample_pos += f_inc;
            
            if((int)f_sample_pos >= info.frames)
            {
                break;
            }
        }
        
        sf_writef_float(f_sndfile, f_output, f_size);
    }
        
    sf_close(f_sndfile);
        
    char f_tmp_finished[256];    
    sprintf(f_tmp_finished, "%s.finished", a_file_out);    
    FILE * f_finished = fopen(f_tmp_finished, "w");    
    fclose(f_finished); 
    free(f_output);
    free(f_buffer0);    
    if(f_buffer1)
    {
        free(f_buffer1);
    }
    if(info.channels > 1)
    {
        free(tmpFrames);
    }
    free(f_pit_core);
    free(f_pit_ratio);
}




#ifdef	__cplusplus
}
#endif

#endif	/* PYDAW_AUDIO_UTIL_H */

