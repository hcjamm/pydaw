/* 
 * File:   cc_map.h
 * Author: Jeff Hubbard
 *
 * Created on March 6, 2012, 7:44 PM
 */

#ifndef CC_MAP_H
#define	CC_MAP_H

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct st_cc_map
{
    char** name;
    int*   key;
    int*   value;
    int    count;
}t_cc_map;


t_cc_map * g_ccm_get_cc_map(char a_name [], int a_key [], int a_value [], int a_count);
int i_ccm_get_cc(t_cc_map*, int);
int i_ccm_char_to_int(char);


int i_ccm_get_cc(t_cc_map* a_map, int a_port)
{
    int f_i = 0;
    
    while(f_i < (a_map->count))
    {
        if((a_map->key[f_i]) == a_port)
        {
            return (a_map->value[f_i]);
        }
        f_i++;
    }
    
    return -1;
}


int i_ccm_char_to_int(char a_char)
{
    switch(a_char)
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

t_cc_map * g_ccm_get_cc_map(char* a_name [], int a_key [], int a_value [], int a_count, char* a_map_file)
{
    t_cc_map * f_result = (t_cc_map*)malloc(sizeof(t_cc_map));
    
    f_result->count = a_count;
    f_result->key = (int*)malloc(sizeof(int) * a_count);
    f_result->value = (int*)malloc(sizeof(int) * a_count);
    f_result->name = (char*)malloc(sizeof(char) * a_count);
    
    int f_i = 0;
    
    while(f_i < a_count)
    {
        f_result->name[f_i] = a_name[f_i];
        f_result->key[f_i] = a_key[f_i];
        f_result->value[f_i] = a_value[f_i];
        
        f_i++;
    }
    
    return f_result;    
}

#ifdef	__cplusplus
}
#endif

#endif	/* CC_MAP_H */

