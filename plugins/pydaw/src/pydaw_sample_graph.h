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
//#include <sys/stat.h>

#define PYDAW_SAMPLE_GRAPH_MAX_SIZE 2097152

void v_pydaw_generate_sample_graph(char * a_file_in, char * a_file_out);

void v_pydaw_generate_sample_graph(char * a_file_in, char * a_file_out)
{
    char * f_result = (char*)malloc(sizeof(char) * PYDAW_SAMPLE_GRAPH_MAX_SIZE);
        
    SF_INFO info;
    SNDFILE *file;
    size_t samples = 0;
    float *tmpFrames, *tmpSamples[2];
    
    info.format = 0;
    file = sf_open(a_file_in, SFM_READ, &info);

    if (!file) 
    {
        printf("error: unable to load sample file '%s'", a_file_in);
        return;
    }
        
    samples = info.frames;

    tmpFrames = (float *)malloc(info.frames * info.channels * sizeof(float));
    sf_readf_float(file, tmpFrames, info.frames);
    sf_close(file);
           
    int f_adjusted_channel_count = 1;
    if(info.channels >= 2)    
    {
        f_adjusted_channel_count = 2;
    }

    int f_actual_array_size = (samples);

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
    
    float f_temp_sample = 0.0f;
        
    for(f_i = 0; f_i < f_actual_array_size; f_i++)
    {       
        for (j = 0; j < f_adjusted_channel_count; ++j) 
        {
            f_temp_sample = (tmpFrames[f_i * info.channels + j]);
            tmpSamples[j][f_i] = f_temp_sample;            
        }

    }
        
    free(tmpFrames);
    
    
    FILE * f = fopen(a_file_out,"wb");
        
    if(f)
    {
        fprintf(f, f_result);
        fflush(f);
        fclose(f);
    }
    else
    {
        printf("cc_map.h:  Cannot open %s for writing, path is either invalid or you do not have the rights to open it.\n", a_file_out);
    }
}

#ifdef	__cplusplus
}
#endif

#endif	/* PYDAW_SAMPLE_GRAPH_H */

