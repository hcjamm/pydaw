/* 
 * File:   pydaw_audio_inputs.h
 * Author: JeffH
 *
 * Created on January 5, 2013, 10:48 PM
 */

#ifndef PYDAW_AUDIO_INPUTS_H
#define	PYDAW_AUDIO_INPUTS_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <sndfile.h>
#include <pthread.h>

//8 megabyte buffers per audio input...  2 (interleaved) buffers per input
#define PYDAW_AUDIO_INPUT_REC_BUFFER_SIZE 8388608
    
typedef struct
{
    int rec;
    int stereo_mode;  //tentatively: 0 == stereo, 1 == mono-L-only...  TODO:  a mono-mixed mode also???
    int output_track;
    SF_INFO sf_info;    
    SNDFILE * sndfile;
    float rec_buffers[2][PYDAW_AUDIO_INPUT_REC_BUFFER_SIZE];
    int current_buffer;
    int flush_last_buffer_pending;
}pydaw_audio_input;

pydaw_audio_input * g_pydaw_audio_input_get();

pydaw_audio_input * g_pydaw_audio_input_get()
{
    pydaw_audio_input * f_result;
    
    if(posix_memalign((void**)(&f_result), 16, (sizeof(pydaw_audio_input))) != 0)
    {
        printf("Call to posix_memalign failed for g_pydaw_audio_input_get\n");
        return 0;
    }
            
    f_result->rec = 0;
    f_result->output_track = 0;
    
    return f_result;
}

void v_pydaw_audio_input_record_init(pydaw_audio_input * a_audio_input, char * a_file_out)
{
    if(a_audio_input->rec)
    {
        a_audio_input->sndfile = sf_open(a_file_out, SFM_WRITE, &a_audio_input->sf_info);
    }    
}


#ifdef	__cplusplus
}
#endif

#endif	/* PYDAW_AUDIO_INPUTS_H */

