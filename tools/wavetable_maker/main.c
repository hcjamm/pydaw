/* 
 * File:   main.c
 * Author: Jeff Hubbard
 * 
 * Generate wavetables and print as C arrays.
 * 
 * You may be interested to know that this is how it was done back in the 
 * early days of digital hardware(and probably still today), and that this 
 * methodology will produce far better results than ie:  sampling ye olden 
 * vintage hardware and trimming the samples by hand.
 *
 * Created on March 12, 2012, 7:46 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sndfile.h>
#include <string.h>
#include "../../plugins/libmodsynth/modules/oscillator/osc_simple.h"
#include "../../plugins/libmodsynth/modules/oscillator/noise.h"
#include "../../plugins/libmodsynth/modules/filter/svf.h"
#include "../../plugins/libmodsynth/lib/pitch_core.h"

#define FLOATS_PER_LINE 12
#define WT_HZ 40.0f
#define WT_FRAMES_PER_CYCLE 1200
#define WT_SR 48000.0f

void print_to_c_array(float * a_buffer, int a_count, char * a_name)
{
    //normalize  TODO:  DC offset...
    int f_i = 0;
    
    float f_highest = 0.0f;
    float f_offset = 0.0f;
    
    while(f_i < a_count)
    {
        float f_val = fabs(a_buffer[f_i]);
        if(f_val > f_highest)
        {
            f_highest = f_val;
        }
        f_i++;
    }
    
    f_i = 0;
    
    float f_normalize = 0.99f/f_highest;
    
    while(f_i < a_count)
    {
        a_buffer[f_i] = a_buffer[f_i] * f_normalize;
        f_i++;
    }   
    
    printf("#define %s_count %i\n\nfloat %s_array[%s_count] = {\n", a_name, a_count, a_name, a_name);
    
    f_i = 0;
    
    while(f_i < a_count)
    {
        int f_i2 = 0;
        while(f_i < a_count && f_i2 < FLOATS_PER_LINE)
        {
            printf("%ff, ", a_buffer[f_i]);
            f_i++;
            f_i2++;
        }
        printf("\n");        
    }
    printf("};\n\n\n");
    
    /*Now output to a sndfile so it can be analyzed in a wave editor*/
        
    SF_INFO f_sf_info;
    f_sf_info.channels = 1;
    f_sf_info.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;
    f_sf_info.samplerate = (int)(WT_SR);
            
    char f_file_out[512];
    sprintf(f_file_out, "%s.wav", a_name);
    
    SNDFILE * f_sndfile = sf_open(f_file_out, SFM_WRITE, &f_sf_info);
    sf_writef_float(f_sndfile, a_buffer, a_count);
    sf_close(f_sndfile);    
}


int main(int argc, char** argv) 
{
    int f_wav_count = 0;
    
    char f_tmp_mem[1024];
    char f_tmp_iter[1024];
    
    char f_type[10000] = "\n\ntypedef struct {\nfloat ** tables;\n}t_wt_wavetables;\n\n";
    strcat(f_type, "t_wt_wavetables * g_wt_wavetables_get();\n\n");
    strcat(f_type, "t_wt_wavetables * g_wt_wavetables_get()\n{\nint f_i = 0;\n");
    strcat(f_type, "t_wt_wavetables * f_result;\nif(posix_memalign((void**)&f_result, 16, (sizeof(t_wt_wavetables))) != 0){return 0;}\n");
    strcat(f_type, "if(posix_memalign((void**)&f_result->tables, 16, (sizeof(float) * WT_TOTAL_WAVETABLE_COUNT)) != 0){return 0;}\n");
    
    float * tmp = (float*)malloc(sizeof(float) * 10000);
    
    t_osc_simple_unison * f_osc = g_osc_get_osc_simple_unison(WT_SR);
    v_osc_set_uni_voice_count(f_osc, 1);
    v_osc_set_simple_osc_unison_type(f_osc, 0);
    f_osc->voice_inc[0] =  WT_HZ * f_osc->sr_recip;
    
    t_state_variable_filter * f_svf = g_svf_get(WT_SR);
    float f_note_pitch = f_pit_hz_to_midi_note(WT_HZ * 4.0f);
    t_pit_pitch_core * f_pitch_core = g_pit_get();
    float f_converted_fast_hz = f_pit_midi_note_to_hz_fast(f_note_pitch, f_pitch_core);
    v_svf_set_cutoff_base(f_svf, f_note_pitch);
    v_svf_set_res(f_svf, -0.1f);
    v_svf_set_cutoff(f_svf);
       
    int f_i = 0;
    
    while(f_i < 1000000)
    {
        //f_osc_run_unison_osc(f_osc);
        v_svf_run_2_pole_hp(f_svf, f_osc_run_unison_osc(f_osc));
        f_i++;
    }
    
    
    /*Raw saw wave, mostly as a reference waveform*/
    
    f_i = 0;
    
    f_osc->osc_cores[0]->output = 0.0f;
    
    while(f_i < WT_FRAMES_PER_CYCLE)
    {        
        tmp[f_i] = f_osc_run_unison_osc(f_osc);
        f_i++;
    }
    
    print_to_c_array(tmp, WT_FRAMES_PER_CYCLE, "plain_saw");    
    
    sprintf(f_tmp_mem, "if(posix_memalign((void**)&f_result->tables[%i], 16, (sizeof(float) * %i)) != 0){return 0;}\n", f_wav_count, WT_FRAMES_PER_CYCLE);    
    strcat(f_type, f_tmp_mem);
    sprintf(f_tmp_iter, "f_i = 0;\nwhile(f_i < %i)\n{\nf_result->tables[f_i] = %s_array[f_i]; \nf_i++;\n}", WT_FRAMES_PER_CYCLE, "plain_saw");
    strcat(f_type, f_tmp_iter);
    
    f_wav_count++;
    
    /*Supersaw-style HP'd saw wave*/
    
    f_i = 0;
    //Reset the phase by running it directly through the filter, so as not to disrupt the filter's state
    if(f_osc->osc_cores[0]->output >= 0.5f)
    {
        while(f_osc->osc_cores[0]->output > 0.5f)
        {
            v_svf_run_2_pole_hp(f_svf, f_osc_run_unison_osc(f_osc));
        }
        
        while(f_osc->osc_cores[0]->output < 0.5f)
        {
            v_svf_run_2_pole_hp(f_svf, f_osc_run_unison_osc(f_osc));
        }
    }
    else
    {
        while(f_osc->osc_cores[0]->output < 0.5f)
        {
            v_svf_run_2_pole_hp(f_svf, f_osc_run_unison_osc(f_osc));
        }
    }
    
    f_i = 0;
    
    while(f_i < WT_FRAMES_PER_CYCLE)
    {        
        tmp[f_i] = v_svf_run_2_pole_hp(f_svf, f_osc_run_unison_osc(f_osc));
        f_i++;
    }
        
    print_to_c_array(tmp, WT_FRAMES_PER_CYCLE, "superbsaw");  
    
    sprintf(f_tmp_mem, "if(posix_memalign((void**)&f_result->tables[%i], 16, (sizeof(float) * %i)) != 0){return 0;}\n", f_wav_count, WT_FRAMES_PER_CYCLE);
    strcat(f_type, f_tmp_mem);
    sprintf(f_tmp_iter, "f_i = 0;\nwhile(f_i < %i)\n{\nf_result->tables[f_i] = %s_array[f_i]; \nf_i++;\n}", WT_FRAMES_PER_CYCLE, "superbsaw");
    strcat(f_type, f_tmp_iter);
    
    f_wav_count++;
    
    strcat(f_type, "\nreturn f_result;\n}\n\n");
    
    printf("\n\n#define WT_TOTAL_WAVETABLE_COUNT %i\n\n", f_wav_count);
    
    printf("%s", f_type);
    
    return 0; //(EXIT_SUCCESS);
}

