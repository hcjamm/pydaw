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
#include "../../plugins/libmodsynth/modules/oscillator/osc_simple.h"
#include "../../plugins/libmodsynth/modules/oscillator/noise.h"
#include "../../plugins/libmodsynth/modules/filter/svf.h"

#define FLOATS_PER_LINE 12

void print_to_c_array(float * a_buffer, int a_count, char * a_name)
{
    printf("#define %s_count %i\n\nfloat %s_array[%s_count] = {\n", a_name, a_count, a_name, a_name);
    
    int f_i = 0;
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
    printf("};");
}

int main(int argc, char** argv) 
{
    float * tmp = (float*)malloc(sizeof(float) * 10000);
    
    /*Supersaw*/
    
    print_to_c_array(tmp, 20, "test");
    
        
    return 0; //(EXIT_SUCCESS);
}

