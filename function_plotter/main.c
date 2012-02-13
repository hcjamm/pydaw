/* 
 * File:   main.c
 * Author: Jeff Hubbard
 *
 * Created on February 7, 2012, 6:52 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include "../libmodsynth/lib/amp.h"
#include "../libmodsynth/lib/pitch_core.h"
#include "adsr.h"

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
        
        printf("%f", f_db_to_linear(i));
        
        i = i + inc;
    }
    
    printf("};\n");
}

void test_db_plot()
{
    int i = -100;
    
    while(i < 36)
    {
        printf("real: %f   ", f_db_to_linear(i));
        printf("fast: %f   ", f_db_to_linear_fast(i));
        printf("\n");
        i += 3;
    }
}

void test_pitch_plot()
{
    int i = 0;
    
    while(i < 129)
    {
        printf("real: %f   ", f_pit_midi_note_to_hz(i));
        printf("fast: %f   ", f_pit_midi_note_to_hz_fast(i));
        printf("\n");
        i += 3;
    }
}

/*
 * 
 */
int main(int argc, char** argv) {

    //test_pitch_plot();
    
    //adsr_attack(2);
    
    //test_db_plot();
    plot_pitch_to_freq();
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

