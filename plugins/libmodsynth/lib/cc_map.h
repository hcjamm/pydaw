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

#ifdef	__cplusplus
extern "C" {
#endif

#define CC_MAX_COUNT 128
    
typedef struct st_ccm_midi_cc_map
{
    int cc_map[CC_MAX_COUNT][2];
}t_ccm_midi_cc_map;

t_ccm_midi_cc_map * g_ccm_get();
    
int v_ccm_get_cc(t_ccm_midi_cc_map* a_ccm, int a_control)
{
    int f_i = 0;
    
    while(f_i < CC_MAX_COUNT)
    {
        if(a_ccm->cc_map[f_i][0] == a_control)
        {
            return a_ccm->cc_map[f_i][1];
        }
        
        f_i++;
    }
    
    return -1;  //Equivalent to  DSSI_NONE;
}

int v_ccm_set_cc(t_ccm_midi_cc_map* a_ccm, int a_control, int a_cc)
{    
    int f_i = 0;
    
    while(f_i < CC_MAX_COUNT)
    {
        if(a_ccm->cc_map[f_i][0] == a_control)
        {
            a_ccm->cc_map[f_i][1] = a_cc;
        }
        
        f_i++;
    }    
}

/* int i_ccm_char_arr_to_int(char * a_input)
 * 
 * For converting text representations of Control Numbers or MIDI CC#s to int, for example
 * 065, 112, 027, etc...
 */
int i_ccm_char_arr_to_int(char * a_input)
{
    int f_result = 0;
    /*TODO:  validate that none of the digits returned -1 */
    return (i_ccm_char_arr_to_digit(a_input[0]) * 100) + (i_ccm_char_arr_to_digit(a_input[1]) * 10)
            + (i_ccm_char_arr_to_digit(a_input[2]));
}

/* int i_ccm_char_arr_to_digit(char * a_input)
 * 
 * Admittedly, the implementation is ugly, but it does work without the
 * compiler complaining about it.
 */
int i_ccm_char_arr_to_digit(char * a_input)
{        
    switch((int)(a_input))
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

char * c_ccm_int_to_char_arr(int a_input)
{
    char * f_result = "000";
    
    if(a_input >= 100)
    {
        
    }
    
    if(a_input >= 10)
    {
        
    }
    
    return f_result;
}

/* char * c_ccm_3_char_ptr(char* a_arr, int a_pos)
 * 
 * Generate a pointer to 3 chars from a larger char*
 */
char * c_ccm_3_char_ptr(char* a_arr, int a_pos)
{
    char * f_result = "000";
    
    f_result[0] = a_arr[a_pos];
    f_result[1] = a_arr[a_pos + 1];
    f_result[2] = a_arr[a_pos + 2];
    
    return f_result;
}

void v_ccm_read_file_to_array(t_ccm_midi_cc_map* a_ccm, char * a_file_name)
{
    FILE *f = fopen(a_file_name, "rb");
    fseek(f, 0, SEEK_END);
    long pos = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *bytes = malloc(pos);
    fread(bytes, pos, 1, f);
    fclose(f);

    int f_i = 0;
    
    int f_arr_pos = 0;
    
    while(f_i < pos)
    {
        if(i_ccm_char_arr_to_digit(bytes[f_i]) != -1)
        {
            a_ccm->cc_map[f_arr_pos][0] = i_ccm_char_arr_to_int(c_ccm_3_char_ptr(bytes, f_i));
            
            f_i += 3;
            
            a_ccm->cc_map[f_arr_pos][1] = i_ccm_char_arr_to_int(c_ccm_3_char_ptr(bytes, f_i));
            
            f_arr_pos++;
        }
    }
    
    free(bytes); // free allocated memory
}

#ifdef	__cplusplus
}
#endif

#endif	/* CC_MAP_H */

