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
#include "../../plugins/libmodsynth/modules/filter/peak_eq.h"
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
            
    float * tmp = (float*)malloc(sizeof(float) * 10000);
    
    t_white_noise * f_noise = g_get_white_noise(WT_SR);
    
    t_pkq_peak_eq * f_eq1 = g_pkq_get(WT_SR);
    t_pkq_peak_eq * f_eq2 = g_pkq_get(WT_SR);
    
    t_osc_simple_unison * f_osc = g_osc_get_osc_simple_unison(WT_SR);
    v_osc_set_uni_voice_count(f_osc, 1);
    v_osc_set_simple_osc_unison_type(f_osc, 0);
    f_osc->voice_inc[0] =  WT_HZ * f_osc->sr_recip;
    
    t_state_variable_filter * f_svf = g_svf_get(WT_SR);
    float f_note_pitch = f_pit_hz_to_midi_note(WT_HZ);
    float f_filter_cutoff = f_note_pitch + 24.0f;
    t_pit_pitch_core * f_pitch_core = g_pit_get();
    float f_converted_fast_hz = f_pit_midi_note_to_hz_fast(f_note_pitch, f_pitch_core);
    v_svf_set_cutoff_base(f_svf, f_filter_cutoff);
    v_svf_set_res(f_svf, -3.0f);
    v_svf_set_cutoff(f_svf);
    
    t_state_variable_filter * f_svf_lp = g_svf_get(WT_SR);
    v_svf_set_cutoff_base(f_svf_lp, f_pit_hz_to_midi_note(9000));
    v_svf_set_res(f_svf_lp, -9.0f);
    v_svf_set_cutoff(f_svf_lp);
    
    
    int f_i = 0;
            
    /*Raw saw wave, mostly as a reference waveform*/
    
    f_osc->osc_cores[0]->output = 0.0f;
    
    while(f_i < WT_FRAMES_PER_CYCLE)
    {        
        tmp[f_i] = f_osc_run_unison_osc(f_osc);
        f_i++;
    }
    
    print_to_c_array(tmp, WT_FRAMES_PER_CYCLE, "plain_saw");    
    
    f_wav_count++;
    
    /*Supersaw-style HP'd saw wave*/
    
    f_i = 0;
    
    while(f_i < 1000000)
    {
        //f_osc_run_unison_osc(f_osc);
        v_svf_run_2_pole_hp(f_svf, f_osc_run_unison_osc(f_osc));
        f_i++;
    }
        
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
        
    f_wav_count++;
    
    
    /*A Unison saw done as a single cycle*/    
        
    v_svf_set_cutoff_base(f_svf, f_filter_cutoff);
    v_svf_set_res(f_svf, -3.0f);
    v_svf_set_cutoff(f_svf);
    v_osc_set_uni_voice_count(f_osc, 7);
    v_osc_set_unison_pitch(f_osc, 0.5f, f_note_pitch);
    
    f_i = 0;
    
    while(f_i < 1000000)
    {
        //f_osc_run_unison_osc(f_osc);
        v_svf_run_2_pole_hp(f_svf, f_osc_run_unison_osc(f_osc) + f_run_pink_noise(f_noise));
        f_i++;
    }
        
    f_i = 0;
    
    v_pkq_calc_coeffs(f_eq1, 90.0f, 6.0f, 6.0f);
    //Reset the phase by running it directly through the filter, so as not to disrupt the filter's state
    if(f_osc->osc_cores[0]->output >= 0.5f)
    {
        while(f_osc->osc_cores[0]->output > 0.5f)
        {
            v_pkq_run(f_eq1, 
                v_svf_run_2_pole_hp(f_svf, f_osc_run_unison_osc(f_osc)),
                0.0f);
            
        }
        
        while(f_osc->osc_cores[0]->output < 0.5f)
        {
            v_pkq_run(f_eq1, 
                v_svf_run_2_pole_hp(f_svf, f_osc_run_unison_osc(f_osc)),
                0.0f);
        }
    }
    else
    {
        while(f_osc->osc_cores[0]->output < 0.5f)
        {
            v_pkq_run(f_eq1, 
                v_svf_run_2_pole_hp(f_svf, f_osc_run_unison_osc(f_osc)),
                0.0f);
        }
    }
    
    f_i = 0;
    
    while(f_i < WT_FRAMES_PER_CYCLE)
    {        
        float f_current_sample = f_osc_run_unison_osc(f_osc) + (f_run_pink_noise(f_noise) * 0.25f);        
        v_pkq_run(f_eq1, 
                v_svf_run_2_pole_hp(f_svf, f_current_sample),
                0.0f);
        tmp[f_i] = f_eq1->output0;
                
        f_i++;
    }
        
    print_to_c_array(tmp, WT_FRAMES_PER_CYCLE, "viralsaw");  
        
    f_wav_count++;
    
    
    /*A softer saw suitable for pads that aren't meant to be very bright*/
    
    v_svf_set_cutoff_base(f_svf, f_filter_cutoff);
    v_svf_set_res(f_svf, -6.0f);
    v_svf_set_cutoff(f_svf);
    v_osc_set_uni_voice_count(f_osc, 1);
    v_osc_set_unison_pitch(f_osc, 0.5f, f_note_pitch);
    
    f_i = 0;
    
    while(f_i < 1000000)
    {
        //f_osc_run_unison_osc(f_osc);
        v_svf_run_2_pole_hp(f_svf, f_osc_run_unison_osc(f_osc) + f_run_pink_noise(f_noise));
        f_i++;
    }
        
    f_i = 0;
    
    v_pkq_calc_coeffs(f_eq1, f_pit_hz_to_midi_note(3600.0f), 6.0f, -6.0f);
    //Reset the phase by running it directly through the filter, so as not to disrupt the filter's state
    if(f_osc->osc_cores[0]->output >= 0.5f)
    {
        while(f_osc->osc_cores[0]->output > 0.5f)
        {
            v_pkq_run(f_eq1, 
                v_svf_run_2_pole_hp(f_svf, f_osc_run_unison_osc(f_osc)),
                0.0f);
            v_svf_run_4_pole_lp(f_svf_lp, f_eq1->output0);            
        }
        
        while(f_osc->osc_cores[0]->output < 0.5f)
        {
            v_pkq_run(f_eq1, 
                v_svf_run_2_pole_hp(f_svf, f_osc_run_unison_osc(f_osc)),
                0.0f);
            v_svf_run_4_pole_lp(f_svf_lp, f_eq1->output0);
        }
    }
    else
    {
        while(f_osc->osc_cores[0]->output < 0.5f)
        {
            v_pkq_run(f_eq1, 
                v_svf_run_2_pole_hp(f_svf, f_osc_run_unison_osc(f_osc)),
                0.0f);
            v_svf_run_4_pole_lp(f_svf_lp, f_eq1->output0);
        }
    }
    
    f_i = 0;
    
    while(f_i < WT_FRAMES_PER_CYCLE)
    {        
        float f_current_sample = f_osc_run_unison_osc(f_osc) + (f_run_pink_noise(f_noise) * 0.25f);        
        v_pkq_run(f_eq1, 
                v_svf_run_2_pole_hp(f_svf, f_current_sample),
                0.0f);
        tmp[f_i] = v_svf_run_4_pole_lp(f_svf_lp, f_eq1->output0);
                
        f_i++;
    }
        
    print_to_c_array(tmp, WT_FRAMES_PER_CYCLE, "soft_saw");  
    
    f_wav_count++;
    
    
    
    /*An aggressive and fuzzy sounding square*/
    
    v_svf_set_cutoff_base(f_svf, f_filter_cutoff);
    v_svf_set_res(f_svf, -6.0f);
    v_svf_set_cutoff(f_svf);
    v_osc_set_uni_voice_count(f_osc, 1);
    v_osc_set_unison_pitch(f_osc, 0.5f, f_note_pitch);
    v_osc_set_simple_osc_unison_type(f_osc, 1);
    
    f_i = 0;
    
    while(f_i < 1000000)
    {
        //f_osc_run_unison_osc(f_osc);
        v_svf_run_2_pole_hp(f_svf, f_osc_run_unison_osc(f_osc) + f_run_white_noise(f_noise));
        f_i++;
    }
        
    f_i = 0;
    
    v_pkq_calc_coeffs(f_eq1, f_pit_hz_to_midi_note(600.0f), 6.0f, -6.0f);
    v_pkq_calc_coeffs(f_eq2, f_pit_hz_to_midi_note(7000.0f), 6.0f, 6.0f);
    //Reset the phase by running it directly through the filter, so as not to disrupt the filter's state
    if(f_osc->osc_cores[0]->output >= 0.5f)
    {
        while(f_osc->osc_cores[0]->output > 0.5f)
        {
            v_pkq_run(f_eq1, 
                v_svf_run_2_pole_hp(f_svf, f_osc_run_unison_osc(f_osc) + (f_run_white_noise(f_noise) * 0.25f)),
                0.0f);
            v_pkq_run(f_eq2, 
                f_eq1->output0,
                0.0f);
            v_svf_run_4_pole_lp(f_svf_lp, f_eq2->output0); 
        }
        
        while(f_osc->osc_cores[0]->output < 0.5f)
        {            
            v_pkq_run(f_eq1, 
                v_svf_run_2_pole_hp(f_svf, f_osc_run_unison_osc(f_osc) + (f_run_white_noise(f_noise) * 0.25f)),
                0.0f);
            v_pkq_run(f_eq2, 
                f_eq1->output0,
                0.0f);
            v_svf_run_4_pole_lp(f_svf_lp, f_eq2->output0);
        }
    }
    else
    {
        while(f_osc->osc_cores[0]->output < 0.5f)
        {            
            v_pkq_run(f_eq1, 
                v_svf_run_2_pole_hp(f_svf, f_osc_run_unison_osc(f_osc) + (f_run_white_noise(f_noise) * 0.25f)),
                0.0f);
            v_pkq_run(f_eq2, 
                f_eq1->output0,
                0.0f);
            v_svf_run_4_pole_lp(f_svf_lp, f_eq2->output0);
        }
    }
    
    f_i = 0;
    
    while(f_i < WT_FRAMES_PER_CYCLE)
    {        
        float f_current_sample = f_osc_run_unison_osc(f_osc) + (f_run_white_noise(f_noise) * 0.25f);        
        v_pkq_run(f_eq1, 
                v_svf_run_2_pole_hp(f_svf, f_current_sample),
                0.0f);
        v_pkq_run(f_eq2, 
                f_eq1->output0,
                0.0f);
        tmp[f_i] = f_eq2->output0; //v_svf_run_4_pole_lp(f_svf_lp, f_eq2->output0);
                
        f_i++;
    }
        
    print_to_c_array(tmp, WT_FRAMES_PER_CYCLE, "evil_square");  
    
    f_wav_count++;
    
    
    /*A soft square*/
    
    v_svf_set_cutoff_base(f_svf, f_filter_cutoff);
    v_svf_set_res(f_svf, -6.0f);
    v_svf_set_cutoff(f_svf);
    v_osc_set_uni_voice_count(f_osc, 1);
    v_osc_set_unison_pitch(f_osc, 0.5f, f_note_pitch);
    v_osc_set_simple_osc_unison_type(f_osc, 1);
    
    f_i = 0;
    
    while(f_i < 1000000)
    {
        //f_osc_run_unison_osc(f_osc);
        v_svf_run_2_pole_hp(f_svf, f_osc_run_unison_osc(f_osc) + f_run_white_noise(f_noise));
        f_i++;
    }
        
    f_i = 0;
    
    v_pkq_calc_coeffs(f_eq1, f_pit_hz_to_midi_note(1200.0f), 6.0f, 6.0f);
    v_pkq_calc_coeffs(f_eq2, f_pit_hz_to_midi_note(3600.0f), 6.0f, -6.0f);
    //Reset the phase by running it directly through the filter, so as not to disrupt the filter's state
    if(f_osc->osc_cores[0]->output >= 0.5f)
    {
        while(f_osc->osc_cores[0]->output > 0.5f)
        {
            v_pkq_run(f_eq1, 
                v_svf_run_2_pole_hp(f_svf, f_osc_run_unison_osc(f_osc)),
                0.0f);
            v_pkq_run(f_eq2, 
                f_eq1->output0,
                0.0f);
            v_svf_run_4_pole_lp(f_svf_lp, f_eq2->output0); 
        }
        
        while(f_osc->osc_cores[0]->output < 0.5f)
        {            
            v_pkq_run(f_eq1, 
                v_svf_run_2_pole_hp(f_svf, f_osc_run_unison_osc(f_osc)),
                0.0f);
            v_pkq_run(f_eq2, 
                f_eq1->output0,
                0.0f);
            v_svf_run_4_pole_lp(f_svf_lp, f_eq2->output0);
        }
    }
    else
    {
        while(f_osc->osc_cores[0]->output < 0.5f)
        {            
            v_pkq_run(f_eq1, 
                v_svf_run_2_pole_hp(f_svf, f_osc_run_unison_osc(f_osc)),
                0.0f);
            v_pkq_run(f_eq2, 
                f_eq1->output0,
                0.0f);
            v_svf_run_4_pole_lp(f_svf_lp, f_eq2->output0);
        }
    }
    
    f_i = 0;
    
    while(f_i < WT_FRAMES_PER_CYCLE)
    {        
        float f_current_sample = f_osc_run_unison_osc(f_osc) + (f_run_white_noise(f_noise) * 0.25f);        
        v_pkq_run(f_eq1, 
                v_svf_run_2_pole_hp(f_svf, f_current_sample),
                0.0f);
        v_pkq_run(f_eq2, 
                f_eq1->output0,
                0.0f);
        tmp[f_i] = v_svf_run_4_pole_lp(f_svf_lp, f_eq2->output0);
                
        f_i++;
    }
        
    print_to_c_array(tmp, WT_FRAMES_PER_CYCLE, "soft_square");  
    
    f_wav_count++;
    
    
    
    /*A pink noise glitch*/
    
    v_svf_set_cutoff_base(f_svf, f_note_pitch);
    v_svf_set_res(f_svf, -6.0f);
    v_svf_set_cutoff(f_svf);
    v_osc_set_uni_voice_count(f_osc, 1);
    v_osc_set_unison_pitch(f_osc, 0.5f, f_note_pitch);
    v_osc_set_simple_osc_unison_type(f_osc, 1);
    
    f_i = 0;
    
    while(f_i < 1000000)
    {
        //f_osc_run_unison_osc(f_osc);
        v_svf_run_2_pole_hp(f_svf, f_run_pink_noise(f_noise));
        f_i++;
    }
    
    f_i = 0;
    
    while(f_i < WT_FRAMES_PER_CYCLE)
    {   
        tmp[f_i] = v_svf_run_2_pole_hp(f_svf, f_run_pink_noise(f_noise));                
        f_i++;
    }
        
    print_to_c_array(tmp, WT_FRAMES_PER_CYCLE, "pink_glitch");  
    
    f_wav_count++;
        
    
    /*A white noise glitch*/
    
    v_svf_set_cutoff_base(f_svf, f_note_pitch);
    v_svf_set_res(f_svf, -6.0f);
    v_svf_set_cutoff(f_svf);
    v_osc_set_uni_voice_count(f_osc, 1);
    v_osc_set_unison_pitch(f_osc, 0.5f, f_note_pitch);
    v_osc_set_simple_osc_unison_type(f_osc, 1);
    
    f_i = 0;
    
    while(f_i < 1000000)
    {
        //f_osc_run_unison_osc(f_osc);
        v_svf_run_2_pole_hp(f_svf, f_run_white_noise(f_noise));
        f_i++;
    }
        
    f_i = 0;
    
    while(f_i < WT_FRAMES_PER_CYCLE)
    {   
        tmp[f_i] = v_svf_run_2_pole_hp(f_svf, f_run_white_noise(f_noise));                
        f_i++;
    }
        
    print_to_c_array(tmp, WT_FRAMES_PER_CYCLE, "white_glitch");  
    
    f_wav_count++;
    
    /*End waveforms*/
    
    printf("\n\n#define WT_TOTAL_WAVETABLE_COUNT %i\n\n", f_wav_count);
    
    return 0; //(EXIT_SUCCESS);
}

