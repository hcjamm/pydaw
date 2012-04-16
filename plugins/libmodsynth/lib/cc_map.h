/* 
 * File:   cc_map.h
 * Author: Jeff Hubbard
 * 
 * This file is not yet production-ready.
 * 
 * Purpose:  Allow the user to edit the CC map for each plugin
 * 
 * a_cc_map is an array of int [CC_MAX_COUNT][2]
 * 
 * [0] == LADSPA Control
 * [1] == MIDI CC number
 *
 * Created on March 6, 2012, 7:44 PM
 */

#ifndef CC_MAP_H
#define	CC_MAP_H

#include <cstring>

#ifdef	__cplusplus
extern "C" {
#endif

#define CC_MAX_COUNT 128
    
int v_ccm_get_cc(int a_cc_map[CC_MAX_COUNT][2],int a_control)
{
    int f_i = 0;
    
    while(f_i < CC_MAX_COUNT)
    {
        if(a_cc_map[f_i][0] == a_control)
        {
            return a_cc_map[f_i][1];
        }
        
        f_i++;
    }
    
    return 0;  //TODO:  DSSI_CC(NONE);
}

int v_ccm_set_cc(int a_cc_map[CC_MAX_COUNT][2], int a_control, int a_cc)
{    
    int f_i = 0;
    
    while(f_i < CC_MAX_COUNT)
    {
        if(a_cc_map[f_i][0] == a_control)
        {
            a_cc_map[f_i][1] = a_cc;
        }
        
        f_i++;
    }    
}

int i_ccm_char_arr_to_int(char * a_input)
{
    int f_result = 0;
    /*TODO:  validate that none of the digits returned -1 */
    return (i_ccm_char_arr_to_digit(a_input[0]) * 100) + (i_ccm_char_arr_to_digit(a_input[1]) * 10)
            + (i_ccm_char_arr_to_digit(a_input[2]));
}

int i_ccm_char_arr_to_digit(char a_input)
{
    switch(a_input)
    {
        case '0':
            return 0;
        case '1':
            return 1;
        case '2':
            return 2;
        case '3':
            return 3;
        case '4':
            return 4;
        case '5':
            return 5;
        case '6':
            return 6;
        case '7':
            return 7;
        case '8':
            return 8;
        case '9':
            return 9;
        default:
            return -1;       
    }    
}

void v_ccm_read_file_to_array()
{
    FILE *f = fopen("text.txt", "rb");
    fseek(f, 0, SEEK_END);
    long pos = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *bytes = malloc(pos);
    fread(bytes, pos, 1, f);
    fclose(f);

    //hexdump(bytes); // do some stuff with it
    free(bytes); // free allocated memory
}

#ifdef	__cplusplus
}
#endif

#endif	/* CC_MAP_H */

