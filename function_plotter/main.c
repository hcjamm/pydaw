/* 
 * File:   main.c
 * Author: bob
 *
 * Created on February 7, 2012, 6:52 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include "../libmodsynth/lib/amp.h"
#include "../libmodsynth/lib/pitch_core.h"

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
    int i = 0;
    
    printf("float arr_pit_p2f [129] = {\n");
    
    while(i < 129)
    {
        if(i != 0)
            printf(",\n");
        
        printf("%f", f_pit_midi_note_to_hz(i));
        
        i++;
    }
    
    printf("};");
}

void plot_freq_to_pitch()
{
    
}

void plot_db_to_amp()
{
    int i = -100;
    
    printf("float arr_amp_db2a [137] = {\n");
    
    while(i <= 36)
    {
        if(i != -100)
            printf(",\n");
        
        printf("%f", f_db_to_linear(i));
        
        i++;
    }
    
    printf("};\n");
}

/*
 * 
 */
int main(int argc, char** argv) {

    //plot_pitch_to_freq();
    plot_db_to_amp();
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

