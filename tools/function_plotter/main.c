/* 
 * File:   main.c
 * Author: Jeff Hubbard
 * 
 * This is for plotting functions.  The "doodling" code style should make it abundantly clear that
 * this is not a serious application for serious people, but rather a fast-and-loose developer's tool
 * for generating code that may or may not require more tweaking.
 *
 * Created on February 7, 2012, 6:52 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include "../libmodsynth/lib/amp.h"
#include "../libmodsynth/lib/pitch_core.h"
#include "../libmodsynth/modules/modulation/adsr.h"
#include <math.h>

void print_help()
{
    printf("usage:  function_plotter function_name\nA crude tool for generating C arrays from math functions that are normally very CPU intensive\n");
    printf("function names:\n" );
    printf("p2f  :  MIDI note number to linear hz\n");
    printf("f2p  :  Linear hz to MIDI note number\n");
    printf("db2a :  Decibels to linear amplitude\n");
}


void plot_pitch_to_freq()
{
    double i = 0;
    int i2 = 0;
    double inc = .05;
    double max = 126;
    
    int count = ((max - i)/inc) + 1;
    
    printf("#define arr_pit_p2f_count %i\n", count);
    printf("#define arr_pit_p2f_count_m1 %i\n\n", count - 1);
    
    printf("float arr_pit_p2f [arr_pit_p2f_count] = {\n");
    
    while(i < max)
    {
        if(i != 0)
            printf(", ");
        
        if(i2 >= 20)
        {
            printf("\n");
            i2 = 0;
        }
        
        i2++;
        
        printf("%f", f_pit_midi_note_to_hz((float)(i)));
        
        i = i + inc;
    }
    
    printf("};");
}

void plot_freq_to_pitch()
{
    
}

void plot_db_to_amp()
{    
    float i = -100;
    int i2 = 0;
    float inc = .25;
    float max = 36;
    
    int count = (max - i)/inc + 1;
    
    t_amp * amp_ptr = g_amp_get();
    
    printf("#define arr_amp_db2a_count %i\n\n", count);
    
    printf("float arr_amp_db2a [arr_amp_db2a_count] = {\n");
    
    while(i <= max)
    {
        if(i != -100)
            printf(",");
        
        if(i2 >= 6)
        {
            printf("\n");
            i2 = 0;
        }
        
        i2++;
        
        printf("%f", f_db_to_linear(i, amp_ptr));
        
        i = i + inc;
    }
    
    printf("};\n");
}

void plot_amp_to_db()
{    
    float i = .01;
    int i2 = 0;
    float inc = .01;
    float max = 4;
    
    int count = (max - i)/inc + 1;
    
    t_amp * amp_ptr = g_amp_get();
    
    printf("#define arr_amp_a2db_count %i\n\n", count);
    
    printf("float arr_amp_a2db [arr_amp_a2db_count] = {\n");
    
    while(i <= max)
    {
        if(i != 0.01)
            printf(",");
        
        if(i2 >= 6)
        {
            printf("\n");
            i2 = 0;
        }
        
        i2++;
        
        printf("%f", f_linear_to_db(i, amp_ptr));
        
        i = i + inc;
    }
    
    printf("};\n");
}

void test_db_plot()
{
    int i = -100;
    
    t_amp * amp_ptr = g_amp_get();
    
    while(i < 36)
    {
        printf("real: %f   ", f_db_to_linear(i, amp_ptr));
        printf("fast: %f   ", f_db_to_linear_fast(i, amp_ptr));
        printf("\n");
        i += 3;
    }
}

void test_pitch_plot()
{
    float i = 0;
        
    t_pit_pitch_core * core = g_pit_get();
    
    while(i < 129)
    {
        float f_real = f_pit_midi_note_to_hz(i);
        float f_fast = f_pit_midi_note_to_hz_fast(i, core);
        float f_deviation = (f_real - f_fast)/f_real;
        
        
        printf("real: %f   ", f_real);
        printf("fast: %f   ", f_fast);
        printf("deviation: %f   ", f_deviation);
        printf("\n");
        i += 5.12765;
    }
    
}

void plot_sine()
{
    double freq = 55;
    double fs = 44100;
    double PI_2 = 6.283185307;
    
    double inc = (freq/fs) * PI_2;
    
    printf("//inc == %f \n\n", (float)inc);
    
    double i = 0;
    int i2 = 0;
    
    int count = PI_2/inc;
    
    printf("#define arr_sine_count %i\n\n", count);
    
    printf("float arr_sine [arr_sine_count] = {\n");
    
    
    while (i < PI_2)
    {
        if(i != 0)
            printf(", ");

        if(i2 >= 20)
        {
            printf("\n");
            i2 = 0;
        }
        
        i2++;
    
        float result = (float)(sin(i));
        
        printf("%f", result);
        
        i += inc;
    }
    
    printf("};\n");
}

void plot_adsr_inc()
{
    t_adsr * f_adsr = g_adsr_get_adsr((1/44100));
    
    printf("#define arr_adsr_attack_count 99\n\n");
    
    printf("float arr_adsr_attack [arr_adsr_attack_count] = {\n");
    
    float i = 0.01;
    
    int i2 = 0;
    
    float inc = .01;
    
    while (i <= 1)
    {
        if(i != 0.01)
            printf(", ");

        if(i2 >= 20)
        {
            printf("\n");
            i2 = 0;
        }
        
        i2++;
    
        v_adsr_set_a_time(f_adsr, i);
        
        printf("%e", (f_adsr->a_inc));
        
        i += inc;
    }
    
    printf("};\n");
}


void plot_sqrt()
{    
    double i = 0;
    int i2 = 0;
    double inc = .01;
    double max = 4;
    
    int count = (max - i)/inc + 1;
    
    printf("#define arr_sqrt_count %i\n\n", count);
    
    printf("float arr_sqrt [arr_sqrt_count] = {\n");
    
    while(i <= max)
    {
        if(i != 0)
            printf(",");
        
        if(i2 >= 6)
        {
            printf("\n");
            i2 = 0;
        }
        
        i2++;
        
        printf("%f", sqrt(i));
        
        i = i + inc;
    }
    
    printf("};\n");
}



/*
 * 
 */
int main(int argc, char** argv) {

    plot_sqrt();
    //plot_amp_to_db();
    //test_pitch_plot();
    //plot_adsr_inc();
    //plot_sine();
    
    //adsr_attack(2);
    
    //test_db_plot();
    //plot_pitch_to_freq();
    //plot_db_to_amp();
    /*
    if(argc < 1)
    {
        printf("Error:  Argument required\n");
        print_help();
        
        return 1;
    }
    
    
        if(argv[0] == "p2f")
            plot_freq_to_pitch();            
        else if(argv[0] == "f2p")
            plot_freq_to_pitch();
        else if(argv[0] == "db2a")
            plot_db_to_amp();
        else
        {
            printf("Invalid argument\n");
            print_help();
            return 1;
        }
        */
    return (EXIT_SUCCESS);
}

