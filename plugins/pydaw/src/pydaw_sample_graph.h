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

#ifndef PYDAW_SAMPLE_GRAPH_H
#define	PYDAW_SAMPLE_GRAPH_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <sndfile.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
//#include <sys/stat.h>
#include "../../libmodsynth/modules/filter/svf.h"

#define PYDAW_SAMPLE_GRAPH_MAX_SIZE 2097152
#define PYDAW_SAMPLE_GRAPH_POINTS_PER_SECOND 32.0f
    
/* void v_pydaw_generate_sample_graph(char * a_file_in, char * a_file_out);
 * The file format:
 * Tag lines:
 * meta|attr.name|value1|...
 * 
 * Valid meta attributes:
 * meta|filename|value   (value should be a full "/home/me/l337samples/cowbell.wav" file path)
 * meta|channels|value   (values should be 1 or 2, PyDAW doesn't support more than 2 channels)
 * meta|timestamp|value  (UNIX time(?))  The time that the graph was made, to compare against file modified time
 * meta|count|value      (value is the number of peaks in the file)
 * meta|length|value      (value is the length of the file in seconds)
 * 
 * Value lines:
 * p|channel|(h/l)|value(float)  (peak, channel, high/low peak, and the value: 1.0 to -1.0...
 * 
 * EOF char:  '\'
 */

//TODO:  A sample graph generator benchmark???
    
void v_pydaw_generate_sample_graph(char * a_file_in, char * a_file_out);

void v_pydaw_generate_sample_graph(char * a_file_in, char * a_file_out)
{
    char * f_result = (char*)malloc(sizeof(char) * PYDAW_SAMPLE_GRAPH_MAX_SIZE);    
    f_result[0] = '\0';
    
    sprintf(f_result, "meta|filename|%s\nmeta|timestamp|%ld\n", a_file_in, time(0));
    
    char f_temp_char[256];
    f_temp_char[0] = '\0';
        
    SF_INFO info;
    SNDFILE *sndfile;
    size_t samples = 0;
    float *tmpFrames;
    
    info.format = 0;
    sndfile = sf_open(a_file_in, SFM_READ, &info);

    if (!sndfile) 
    {
        printf("error: unable to load sample file '%s'", a_file_in);
        return;
    }
        
    samples = info.frames;

    tmpFrames = (float *)malloc(info.frames * info.channels * sizeof(float));
    sf_readf_float(sndfile, tmpFrames, info.frames);
    sf_close(sndfile);
           
    int f_adjusted_channel_count = 1;
    if(info.channels >= 2)    
    {
        f_adjusted_channel_count = 2;
    }
    
    sprintf(f_temp_char, "meta|channels|%i\n", f_adjusted_channel_count);
    strcat(f_result, f_temp_char);
    
    float f_length_seconds = ((float)info.frames)/((float)(info.samplerate));
            
    sprintf(f_temp_char, "meta|length|%f\n", f_length_seconds);
    strcat(f_result, f_temp_char);

    int f_actual_array_size = (samples);
        
    int f_peak_count = (int)(f_length_seconds * PYDAW_SAMPLE_GRAPH_POINTS_PER_SECOND);
    
    if(f_peak_count < 1)
    {
        f_peak_count = 1;
    }
    
    int f_peak_counter = 0;
    
    float f_high_peak[2], f_low_peak[2];
    
    f_high_peak[0] = -1.0f;
    f_high_peak[1] = -1.0f;
    f_low_peak[0] = 1.0f;
    f_low_peak[1] = 1.0f;
    
    int f_count = 0;
    
    int f_i, j;
    
    float f_temp_sample = 0.0f;
        
    for(f_i = 0; f_i < f_actual_array_size; f_i += 10)
    {       
        for (j = 0; j < f_adjusted_channel_count; ++j) 
        {
            f_temp_sample = (tmpFrames[f_i * info.channels + j]);
            if(f_temp_sample > f_high_peak[j])
            {
                f_high_peak[j] = f_temp_sample;
            }
            else if(f_temp_sample < f_low_peak[j])
            {
                f_low_peak[j] = f_temp_sample;
            }
        }
        
        f_peak_counter += 10;
        
        if(f_peak_counter >= f_peak_count)
        {
            f_peak_counter = 0;
            
            for (j = 0; j < f_adjusted_channel_count; ++j)
            {
                sprintf(f_temp_char, "p|%i|h|%f\np|%i|l|%f\n", j, f_high_peak[j], j, f_low_peak[j]);
                strcat(f_result, f_temp_char);
            }
                                    
            f_high_peak[0] = -1.0f;
            f_high_peak[1] = -1.0f;
            f_low_peak[0] = 1.0f;
            f_low_peak[1] = 1.0f;
            
            f_count++;
        }
    }
        
    sf_close(sndfile);
    free(tmpFrames);
    
    f_count++;
    
    sprintf(f_temp_char, "meta|count|%i\n\\", f_count);
    
    strcat(f_result, f_temp_char);
    
    FILE * f = fopen(a_file_out,"wb");
    
    if(f)
    {
        fprintf(f, "%s", f_result);
        fflush(f);
        fclose(f);
    }
    else
    {
        printf("v_pydaw_generate_sample_graph:  Cannot open '%s' for writing, path is either invalid or you do not have the rights to open it.\n", a_file_out);
    }
}

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
    
    
    t_svf2_filter * f_dc_offset = g_svf2_get((float)(info.samplerate));
    v_svf2_set_res(f_dc_offset, -24.0f);
    v_svf2_set_cutoff_base(f_dc_offset, 48.0);
    
    if(info.channels == 1)
    {
        while(f_i < info.frames * info.channels)
        {
            float f_sample0 = tmpFrames[f_i];
            v_svf2_run_2_pole_hp(f_dc_offset, f_sample0, f_sample0);
            tmpFrames[f_i] = f_dc_offset->output0;
            f_i++;
        }
    }
    else if(info.channels == 2)
    {
        while(f_i < info.frames * info.channels)
        {
            float f_sample0 = tmpFrames[f_i];
            f_i++;
            float f_sample1 = tmpFrames[f_i];
            v_svf2_run_2_pole_hp(f_dc_offset, f_sample0, f_sample1);
            tmpFrames[f_i - 1] = f_dc_offset->output0;
            tmpFrames[f_i] = f_dc_offset->output1;
            f_i++;
        }
    }
    
    v_svf2_free(f_dc_offset);        
    
    f_i = 0;
    
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

#ifdef	__cplusplus
}
#endif

#endif	/* PYDAW_SAMPLE_GRAPH_H */

