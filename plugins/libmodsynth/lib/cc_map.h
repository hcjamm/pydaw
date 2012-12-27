/* 
 * File:   cc_map.h
 * Author: Jeff Hubbard
 *
 * Purpose:  Allow the user to edit the CC map for each plugin
 * 
 * DO NOT use 0 for a LADSPA control port number, 0 is meant to mean 'null' in
 * this context
 * 
 * a_cc_map is an array of int [CC_MAX_COUNT]
 * 
 * [index] == LADSPA Control Port number
 * array index == MIDI CC number
 *
 * Created on March 6, 2012, 7:44 PM
 */

#ifndef CC_MAP_H
#define	CC_MAP_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#ifdef	__cplusplus
extern "C" {
#endif

#define CC_MAX_COUNT 128
    
typedef struct st_ccm_midi_cc_map
{
    int cc_map[CC_MAX_COUNT];
    char * cc_descriptions[CC_MAX_COUNT];
}t_ccm_midi_cc_map;

t_ccm_midi_cc_map * g_ccm_get();
int i_ccm_get_cc(t_ccm_midi_cc_map* a_ccm, int a_control);
char * c_ccm_int_to_char_arr(int a_input);
void v_ccm_set_cc(t_ccm_midi_cc_map* a_ccm, int a_control, int a_cc, char * a_cc_description);
int i_ccm_char_arr_to_int(char * a_input);
int i_ccm_char_arr_to_digit(char * a_input);
char * c_ccm_control_and_cc_to_char_arr(int a_ladspa_port, int a_midi_cc);
char * c_ccm_int_to_char_arr(int a_input);
char * c_ccm_3_char_ptr(char* a_arr, int a_pos);
void v_ccm_read_file_to_array(t_ccm_midi_cc_map* a_ccm, char * a_file_name);

t_ccm_midi_cc_map * g_ccm_get()
{
    t_ccm_midi_cc_map * f_result = (t_ccm_midi_cc_map*)malloc(sizeof(t_ccm_midi_cc_map));
    
    int f_i = 0;
    
    while(f_i < CC_MAX_COUNT)
    {
        f_result->cc_map[f_i] = 0;
        f_result->cc_descriptions[f_i] = (char*)malloc(sizeof(char) * 120);
        strcpy(f_result->cc_descriptions[f_i], "empty");
        f_i++;
    }
    
    return f_result;
}


    
int i_ccm_get_cc(t_ccm_midi_cc_map* a_ccm, int a_control)
{
    int f_i = 0;
    
    while(f_i < CC_MAX_COUNT)
    {
        if(a_ccm->cc_map[f_i] == a_control)
        {
            return f_i;
        }
        
        f_i++;
    }
    
    return -1;  //Equivalent to  DSSI_NONE;
}

void v_ccm_set_cc(t_ccm_midi_cc_map* a_ccm, int a_control, int a_cc, char * a_cc_description)
{    
    a_ccm->cc_map[a_cc] = a_control;
    a_ccm->cc_descriptions[a_cc] = a_cc_description;
}

/* int i_ccm_char_arr_to_int(char * a_input)
 * 
 * For converting text representations of Control Numbers or MIDI CC#s to int, for example
 * 065, 112, 027, etc...
 */
int i_ccm_char_arr_to_int(char * a_input)
{
    /*TODO:  validate that none of the digits returned -1 */
    char f_digit1 = a_input[0];
    char f_digit2 = a_input[1];
    char f_digit3 = a_input[2];
    return (i_ccm_char_arr_to_digit(&f_digit1) * 100) + (i_ccm_char_arr_to_digit(&f_digit2) * 10)
            + (i_ccm_char_arr_to_digit(&f_digit3));
}

/* int i_ccm_char_arr_to_digit(char a_input)
 * 
 * Admittedly, the implementation is ugly, but it does work without the
 * compiler complaining about it.
 */
int i_ccm_char_arr_to_digit(char * a_input)
{        
        if(*a_input == '0')
            return 0;
        else if(*a_input ==  '1')
            return 1;
        else if(*a_input ==  '2')
            return 2;
        else if(*a_input ==  '3')
            return 3;
        else if(*a_input ==  '4')
            return 4;
        else if(*a_input ==  '5')
            return 5;
        else if(*a_input ==  '6')
            return 6;
        else if(*a_input ==  '7')
            return 7;
        else if(*a_input ==  '8')
            return 8;
        else if(*a_input ==  '9')
            return 9;
        else
            return -1;                
}

char * c_ccm_control_and_cc_to_char_arr(int a_ladspa_port, int a_midi_cc)
{
    /*TODO:  Check that the numbers aren't out of range*/
    char * f_result = (char*)malloc(sizeof(char) * 8);
    
    sprintf(f_result, "%s-%s", c_ccm_int_to_char_arr(a_midi_cc), c_ccm_int_to_char_arr(a_ladspa_port));
    
    return f_result;
}

char * c_ccm_int_to_char_arr(int a_input)
{
    /*TODO:  Check that the numbers aren't out of range*/
    char * f_result1 = (char*)malloc(sizeof(char) * 4);
    
    sprintf(f_result1, "%i", a_input);
    
    char * f_result2 = (char*)malloc(sizeof(char) * 4);
    
    if(a_input >= 100)
    {
        f_result2[0] = f_result1[0];
        f_result2[1] = f_result1[1];
        f_result2[2] = f_result1[2];
    }
    else if(a_input >= 10)
    {
        f_result2[0] = '0';
        f_result2[1] = f_result1[0];
        f_result2[2] = f_result1[1];
    }
    else
    {
        f_result2[0] = '0';
        f_result2[1] = '0';
        f_result2[2] = f_result1[0];
    }
    
    f_result2[3] = '\0';
    
    return f_result2;
}

/* char * c_ccm_3_char_ptr(char* a_arr, int a_pos)
 * 
 * Generate a pointer to 3 chars from a larger char*
 */
char * c_ccm_3_char_ptr(char* a_arr, int a_pos)
{
    char * f_result = (char*)malloc(sizeof(char) * 3);
    
    f_result[0] = a_arr[a_pos];
    a_pos++;
    f_result[1] = a_arr[a_pos];
    a_pos++;
    f_result[2] = a_arr[a_pos];
    a_pos++;
        
    return f_result;
}

/* void v_ccm_read_file_to_array(t_ccm_midi_cc_map* a_ccm, char * a_file_name)
 * 
 * Check for the existence of a_file_name in ~/pydaw2, creating the directory and/or the
 * file if needed.  If the file exists, read it into a_ccm, if not, read the default
 * values in a_ccm set by calling v_ccm_set_cc in the plugin's constructor.
 * 
 * Descriptions are not currently read back into a_ccm, as they are not used by
 * the plugin.  Those values are only kept for generating the file.
 */
void v_ccm_read_file_to_array(t_ccm_midi_cc_map* a_ccm, char * a_file_name)
{
    char * f_home = getenv("HOME");
    char * f_path = (char*)malloc(sizeof(char) * 200);
    char * f_file = (char*)malloc(sizeof(char) * 200);
    sprintf(f_path, "%s/pydaw2", f_home);
    sprintf(f_file, "%s/%s", f_path, a_file_name);
    printf("f_file == %s\n", f_file);
    
    struct stat st;
    if(stat(f_path,&st) != 0)
    {
        printf("%s does not exist, creating directory\n", f_path);
        mkdir(f_path, 0777);
    }
    
    FILE *f = fopen(f_file, "rb");
    
    if(!f)
    {                
        printf("Failed to open %s\n", f_file);      
        /*TODO:  Create the file from a_ccm*/
        f = fopen(f_file,"wb");
        
        if(f)
        {
            fprintf(f,"\"This is a MIDI CC mapping file.  The first 3 digits are the MIDI CC number,  do not edit them.  \nThe 2nd 3 digits are the LADSPA port number, you may change these from any value from 001 to 999.  \nAny additional text must be enclosed in quotation marks.\"\n\n");
            int f_i = 0;
            while(f_i < CC_MAX_COUNT)
            {
                fprintf(f,"%s \"%s\"\n",c_ccm_control_and_cc_to_char_arr(a_ccm->cc_map[f_i] , f_i), a_ccm->cc_descriptions[f_i]);
                f_i++;
            }
            fflush(f);
            fclose(f);
        }
        else
        {
            printf("cc_map.h:  Cannot open %s for writing, path is either invalid or you do not have the rights to open it.\n", f_file);
        }
        return;
    }
    
    
    fseek(f, 0, SEEK_END);
    long pos = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *bytes = (char*)malloc(pos);
    fread(bytes, pos, 1, f);
    fclose(f);

    int f_i = 0;
    
    while(f_i < CC_MAX_COUNT)
    {
        a_ccm->cc_map[f_i] = 0;  //clear the values
        f_i++;
    }
    
    f_i = 0;
    /*a value of 1 means that the iterator is within quotation marks*/
    int f_name_state = 0;
        
    while(f_i < pos)
    {
        if(bytes[f_i] == '"')
        {
            if(f_name_state == 0)
            {
                f_name_state = 1;
            }
            else
            {
                f_name_state = 0;
            }
            f_i++;
            continue;
        }
                
        char f_current_char = bytes[f_i];
        
        if((f_name_state == 0) && (i_ccm_char_arr_to_digit(&f_current_char) != -1))
        {
            a_ccm->cc_map[i_ccm_char_arr_to_int(c_ccm_3_char_ptr(bytes, f_i))] = 
            i_ccm_char_arr_to_int(c_ccm_3_char_ptr(bytes, f_i + 4));
            
            f_i += 7;
        }
        
        f_i++;
    }
    
    free(bytes); // free allocated memory
}

#ifdef	__cplusplus
}
#endif

#endif	/* CC_MAP_H */

