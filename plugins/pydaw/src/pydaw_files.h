/* 
 * File:   pydaw_files.h
 * Author: JeffH
 *
 * Functions for opening pydaw files and reading/parsing their string content 
 */

#ifndef PYDAW_FILES_H
#define	PYDAW_FILES_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

/*Standard string sizes.  When in doubt, pick a really big one, it's better to 
 * waste memory than to SEGFAULT...*/
#define LMS_LARGE_STRING 262144 //65536
#define LMS_MEDIUM_STRING 65536 //8192
#define LMS_SMALL_STRING 8192 //512
#define LMS_TINY_STRING 256 //32
    
#define LMS_TERMINATING_CHAR '\\'
    
#include <stdio.h>
#include <time.h>
    
/*
void pydaw_write_log(char * a_string)
{
    assert(a_string);
    char buff[LMS_LARGE_STRING];
    time_t now = time (0);
    strftime (buff, 100, "%Y-%m-%d %H:%M:%S.000", localtime (&now));

    strcat(buff, " - ");
    strcat(buff, a_string);
    
    FILE* pFile = fopen("pydaw-engine.log", "a");
    fprintf(pFile, "%s\n",buff);
    fclose(pFile);
}
*/
void get_string_from_file(const char * a_file, int a_size, char * a_buf)
{
    //char log_buff[200];
    //sprintf(log_buff, "get_string_from_file: a_file: \"%s\" a_size: %i \n", a_file, a_size);
    //pydaw_write_log(log_buff);    
    FILE * f_file;	
    f_file = fopen(a_file, "r");
    assert(f_file);
    fread(a_buf, sizeof(char), sizeof(char) * a_size, f_file);	
    fclose(f_file);
}

typedef struct st_1d_char_array
{
    char ** array;
    int x_count;
}t_1d_char_array;

typedef struct st_2d_char_array
{
    char * array;  //allocate a continuous chunk of memory, otherwise we would need to free possibly thousands of pointers
    int current_index;
    int current_row;
    int current_column;
    int eof;
}t_2d_char_array;

void g_free_1d_char_array(t_1d_char_array * a_array)
{
    int f_i = 0;
    
    while(f_i < (a_array->x_count))
    {
        free(a_array->array[f_i]);
        f_i++;
    }
    
    free(a_array->array);    
    free(a_array);
}

void g_free_2d_char_array(t_2d_char_array * a_array)
{    
    free(a_array->array);    
    free(a_array);
}

/* A specialized split function.  Column count and string size will always be known in advance
 for all of the use cases in PyDAW*/
t_1d_char_array * c_split_str(const char * a_input, char a_delim, int a_column_count, int a_string_size)
{
    int f_i = 0;
    int f_current_string_index = 0;
    int f_current_column = 0;
    
    t_1d_char_array * f_result = (t_1d_char_array*)malloc(sizeof(t_1d_char_array));
    f_result->array = (char**)malloc(sizeof(char*) * a_column_count);
    f_result->x_count = a_column_count;
    
    while(f_i < a_column_count)
    {
        f_result->array[f_i] = (char*)malloc(sizeof(char) * a_string_size);
        f_i++;
    }
    
    f_i = 0;
    
    while(1)
    {
        if(a_input[f_i] == a_delim)
        {
            f_result->array[f_current_column][f_current_string_index] = '\0';
            f_current_column++;
            f_current_string_index = 0;
            assert(f_current_column < a_column_count);                        
        }
        else if((a_input[f_i] == '\n') || (a_input[f_i] == '\0'))
        {
            f_result->array[f_current_column][f_current_string_index] = '\0';
            break;
        }
        else
        {
            f_result->array[f_current_column][f_current_string_index] = a_input[f_i];
            f_current_string_index++;
        }
        
        f_i++;
    }
    
    return f_result;
}

/* Return a 2d array of strings from a file delimited by "|" and "\n" individual fields are 
 * limited to being the size of LMS_TINY_STRING */
t_2d_char_array * g_get_2d_array_from_file(const char * a_file, int a_size)
{    
    //char log_buff[200];
    //sprintf(log_buff, "g_get_2d_array_from_file: a_file: \"%s\" a_size: %i\n", a_file, a_size);
    //pydaw_write_log(log_buff);
    
    t_2d_char_array * f_result = (t_2d_char_array*)malloc(sizeof(t_2d_char_array));
    f_result->array = (char*)malloc(sizeof(char) * a_size);
    f_result->current_index = 0;
    f_result->current_row = 0;
    f_result->current_column = 0;
    f_result->eof = 0;
    get_string_from_file(a_file, a_size, f_result->array);
    //pydaw_write_log(f_result->array);    
    return f_result;    
}

/* Return the next string from the array*/
char * c_iterate_2d_char_array(t_2d_char_array* a_array)
{
    char * f_result = (char*)malloc(sizeof(char) * LMS_TINY_STRING);
    int f_i = 0;    
        
    while(1)
    {        
        //char a_test = a_array->array[(a_array->current_index)];
        if(a_array->array[(a_array->current_index)] == LMS_TERMINATING_CHAR)
        {
            f_result[f_i] = '\0';
            a_array->eof = 1;
            break;
        }
        else if(a_array->array[(a_array->current_index)] == '\n')
        {
            f_result[f_i] = '\0';
            a_array->current_index = (a_array->current_index) + 1;
            a_array->current_row = (a_array->current_row) + 1;
            a_array->current_column = 0;
            break;
        }
        else if(a_array->array[(a_array->current_index)] == '|')
        {   
            f_result[f_i] = '\0';
            a_array->current_index = (a_array->current_index) + 1;
            a_array->current_column = (a_array->current_column) + 1;
            //assert((a_array->current_column) < (a_array->));  //TODO:  A check for acceptable column counts
            break;
        }
        else
        {
            f_result[f_i] = a_array->array[(a_array->current_index)];            
        }
        
        a_array->current_index = (a_array->current_index) + 1;
        f_i++;
    }
    
    return f_result;
}

typedef struct st_dir_list
{
    char ** dir_list;
    int dir_count;
}t_dir_list;

t_dir_list * g_get_dir_list(char * a_dir)
{    
    t_dir_list * f_result = (t_dir_list*)malloc(sizeof(t_dir_list));
    f_result->dir_count = 0;
    
    int f_resize_factor = 256;
    int f_current_max = 256;
    
    f_result->dir_list = (char**)malloc(sizeof(char*) * f_current_max);
    
    DIR *dir;
    struct dirent *ent;
    dir = opendir (a_dir);    
    if (dir != NULL) 
    {
      while ((ent = readdir (dir)) != NULL) 
      {
          if((!strcmp(ent->d_name, ".")) || (!strcmp(ent->d_name, "..")))
          {
              continue;
          }
          
          f_result->dir_list[(f_result->dir_count)] = (char*)malloc(sizeof(char) * LMS_TINY_STRING);
          
            strcpy(f_result->dir_list[(f_result->dir_count)], ent->d_name);
          
          f_result->dir_count = (f_result->dir_count) + 1;
          
          if((f_result->dir_count) >= f_current_max)
          {
              f_current_max += f_resize_factor;
              f_result->dir_list = realloc(f_result->dir_list, sizeof(char*) * f_current_max);
          }
      }
      closedir (dir);
    } 
    else 
    {
      return 0;
    }

    return f_result;
}

void v_pydaw_write_to_file(char * a_file, char * a_string)
{    
    FILE* pFile = fopen(a_file, "w");
    assert(pFile);
    fprintf(pFile, "%s",a_string);
    fclose(pFile);
}

int i_pydaw_file_exists(char * f_file_name)
{
    struct stat sts;
    
    //TODO:  Determine if there is a better way to do this
    if ((stat(f_file_name, &sts)) == -1)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

#ifdef	__cplusplus
}
#endif

#endif	/* PYDAW_FILES_H */

