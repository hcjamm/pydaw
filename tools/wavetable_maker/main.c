/* 
 * File:   main.c
 * Author: Jeff Hubbard
 * 
 * Generate wavetables and print as C arrays.
 *
 * Created on March 12, 2012, 7:46 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sndfile.h>
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
    
    float f_normalize = 1.0f/f_highest;
    
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
    float * tmp = (float*)malloc(sizeof(float) * 10000);
    
    t_osc_simple_unison * f_osc = g_osc_get_osc_simple_unison(WT_SR);
    v_osc_set_uni_voice_count(f_osc, 1);
    v_osc_set_simple_osc_unison_type(f_osc, 0);
    f_osc->voice_inc[0] =  WT_HZ * f_osc->sr_recip;
    
    t_state_variable_filter * f_svf = g_svf_get(WT_SR);
    v_svf_set_cutoff_base(f_svf, f_pit_hz_to_midi_note(WT_HZ));
    v_svf_set_cutoff(f_svf);
        
    /*Supersaw-style HP'd saw wave*/
    
    int f_i = 0;
    
    while(f_i < WT_FRAMES_PER_CYCLE)
    {
        tmp[f_i] = f_osc_run_unison_osc(f_osc);
        tmp[f_i] = v_svf_run_2_pole_hp(f_svf, tmp[f_i]);        
        f_i++;
    }
    
    print_to_c_array(tmp, WT_FRAMES_PER_CYCLE, "superbsaw");
    
        
    return 0; //(EXIT_SUCCESS);
}

