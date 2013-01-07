/* 
 * File:   pydaw_sample_graph.h
 * Author: JeffH
 * 
 * For generating the forthcoming .pygraph sample graph analysis file format...
 * 
 * Created on January 5, 2013, 11:14 PM
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
//#include <sys/stat.h>

#define PYDAW_SAMPLE_GRAPH_MAX_SIZE 2097152
#define PYDAW_SAMPLE_GRAPH_POINTS_PER_SAMPLE 300.0f
    
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

    int f_actual_array_size = (samples);
        
    int f_peak_count = (int)(((float)info.frames) / PYDAW_SAMPLE_GRAPH_POINTS_PER_SAMPLE);
    
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
        
    for(f_i = 0; f_i < f_actual_array_size; f_i++)
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
        
        f_peak_counter++;
        
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

#ifdef	__cplusplus
}
#endif

#endif	/* PYDAW_SAMPLE_GRAPH_H */

