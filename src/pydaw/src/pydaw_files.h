/*
This file is part of the PyDAW project, Copyright PyDAW Team

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef PYDAW_FILES_H
#define	PYDAW_FILES_H

#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <assert.h>

/*Standard string sizes*/
#define PYDAW_LARGE_STRING  65536 //1048576
#define PYDAW_MEDIUM_STRING  32768 //262144 //8192
#define PYDAW_SMALL_STRING  16384 //65536 //512
#define PYDAW_TINY_STRING 4096 //16384 //32

#define PYDAW_TERMINATING_CHAR '\\'

#include <stdio.h>
#include <time.h>

#ifdef	__cplusplus
extern "C" {
#endif

/*No pointers internally, call free() directly on an instance*/
typedef struct
{
    int key_len, val_len;
    char key[256];
    char value[5000];
}t_key_value_pair;

#ifdef	__cplusplus
}
#endif

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
    //sprintf(log_buff, "get_string_from_file: a_file: \"%s\" a_size: %i \n",
    //a_file, a_size);
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
    //allocate a continuous chunk of memory, otherwise we would need to
    //free possibly thousands of pointers
    char * array;
    int current_index;
    int current_row;
    int current_column;
    int eof;
    int eol;
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
    if(a_array->array)
    {
        free(a_array->array);
    }

    if(a_array)
    {
        free(a_array);
    }
}

/* A specialized split function.  Column count and string size will
 * always be known in advance
 for all of the use cases in PyDAW*/
t_1d_char_array * c_split_str(const char * a_input, char a_delim,
        int a_column_count, int a_string_size)
{
    int f_i = 0;
    int f_current_string_index = 0;
    int f_current_column = 0;

    t_1d_char_array * f_result =
            (t_1d_char_array*)malloc(sizeof(t_1d_char_array));
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
            f_result->array[f_current_column][f_current_string_index] =
                    a_input[f_i];
            f_current_string_index++;
        }

        f_i++;
    }

    return f_result;
}

/* Same as above but ignores extra delimiters in the final column */
t_1d_char_array * c_split_str_remainder(const char * a_input, char a_delim,
        int a_column_count, int a_string_size)
{
    int f_i = 0;
    int f_current_string_index = 0;
    int f_current_column = 0;

    t_1d_char_array * f_result =
            (t_1d_char_array*)malloc(sizeof(t_1d_char_array));
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
        if((f_current_column < (a_column_count - 1)) &&
                (a_input[f_i] == a_delim))
        {
            f_result->array[f_current_column][f_current_string_index] = '\0';
            f_current_column++;
            f_current_string_index = 0;
        }
        else if((a_input[f_i] == '\n') || (a_input[f_i] == '\0'))
        {
            f_result->array[f_current_column][f_current_string_index] = '\0';
            break;
        }
        else
        {
            f_result->array[f_current_column][f_current_string_index] =
                    a_input[f_i];
            f_current_string_index++;
        }

        f_i++;
    }

    return f_result;
}


/* Specialized function, split a char* on the first index of a '|' char */
t_key_value_pair * g_kvp_get(const char * a_input)
{
    t_key_value_pair * f_result =
            (t_key_value_pair*)malloc(sizeof(t_key_value_pair));
    int f_i = 0;
    f_result->key_len = 0;
    f_result->val_len = 0;
    int f_stage = 0;

    while(a_input[f_i] != '\0')
    {
        if(f_stage)
        {
            f_result->value[f_result->val_len] = a_input[f_i];
            f_result->val_len++;
        }
        else
        {
            if(a_input[f_i] == '|')
            {
                f_stage = 1;
                f_result->key[f_result->key_len] = '\0';
            }
            else
            {
                f_result->key[f_result->key_len] = a_input[f_i];
                f_result->key_len++;
            }
        }

        f_i++;
    }

    f_result->value[f_result->val_len] = '\0';

    return f_result;
}

/* You must assign something to x->array before trying to read. */
t_2d_char_array * g_get_2d_array(int a_size)
{
    t_2d_char_array * f_result =
            (t_2d_char_array*)malloc(sizeof(t_2d_char_array));
    f_result->array = (char*)malloc(sizeof(char) * a_size);
    f_result->current_index = 0;
    f_result->current_row = 0;
    f_result->current_column = 0;
    f_result->eof = 0;
    f_result->eol = 0;
    return f_result;
}

/* Return a 2d array of strings from a file delimited by
 * "|" and "\n" individual fields are
 * limited to being the size of LMS_TINY_STRING */
t_2d_char_array * g_get_2d_array_from_file(const char * a_file, int a_size)
{
    t_2d_char_array * f_result = g_get_2d_array(a_size);
    get_string_from_file(a_file, a_size, f_result->array);
    return f_result;
}

/* Return the next string from the array*/
char * c_iterate_2d_char_array(t_2d_char_array* a_array)
{
    char * f_result = (char*)malloc(sizeof(char) * PYDAW_SMALL_STRING);
    int f_i = 0;

    while(1)
    {
        //char a_test = a_array->array[(a_array->current_index)];
        if(a_array->array[(a_array->current_index)] == PYDAW_TERMINATING_CHAR ||
            (a_array->array[(a_array->current_index)] == '\0'))
        {
            f_result[f_i] = '\0';
            a_array->eof = 1;
            a_array->eol = 1;
            break;
        }
        else if(a_array->array[(a_array->current_index)] == '\n')
        {
            f_result[f_i] = '\0';
            a_array->current_index = (a_array->current_index) + 1;
            a_array->current_row = (a_array->current_row) + 1;
            a_array->current_column = 0;
            a_array->eol = 1;
            break;
        }
        else if(a_array->array[(a_array->current_index)] == '|')
        {
            f_result[f_i] = '\0';
            a_array->current_index = (a_array->current_index) + 1;
            a_array->current_column = (a_array->current_column) + 1;
            a_array->eol = 0;
            //TODO:  A check for acceptable column counts
            //assert((a_array->current_column) < (a_array->));
            break;
        }
        else
        {
            a_array->eol = 0;
            f_result[f_i] = a_array->array[(a_array->current_index)];
        }

        a_array->current_index = (a_array->current_index) + 1;
        f_i++;
    }

    return f_result;
}

/* Return the next string from the array until a newline, ignoring any
 * delimiting '|' characters */
char * c_iterate_2d_char_array_to_next_line(t_2d_char_array* a_array)
{
    char * f_result = (char*)malloc(sizeof(char) * PYDAW_SMALL_STRING);
    int f_i = 0;

    while(1)
    {
        //char a_test = a_array->array[(a_array->current_index)];
        if(a_array->array[(a_array->current_index)] == PYDAW_TERMINATING_CHAR)
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
        else
        {
            f_result[f_i] = a_array->array[(a_array->current_index)];
        }

        a_array->current_index = (a_array->current_index) + 1;
        f_i++;
    }

    return f_result;
}

typedef struct
{
    int count;
    char ** str_arr;
}t_pydaw_line_split;

t_pydaw_line_split * g_split_line(char a_delimiter, const char * a_str)
{
    t_pydaw_line_split * f_result =
            (t_pydaw_line_split*)malloc(sizeof(t_pydaw_line_split));
    f_result->count = 1;

    int f_i = 0;
    while(1)
    {
        if(a_str[f_i] == '\0')
        {
            break;
        }
        else if(a_str[f_i] == a_delimiter)
        {
            f_result->count++;
        }
        f_i++;
    }

    f_result->str_arr = (char**)malloc(sizeof(char*) * f_result->count);

    f_i = 0;
    while(f_i < f_result->count)
    {
        f_result->str_arr[f_i] =
                (char*)malloc(sizeof(char) * PYDAW_TINY_STRING);
        f_result->str_arr[f_i][0] = '\0';
        f_i++;
    }

    f_i = 0;
    int f_i3 = 0;
    while(f_i < f_result->count)
    {
        int f_i2 = 0;
        while(1)
        {
            if(a_str[f_i3] == '\0' || a_str[f_i3] == a_delimiter)
            {
                f_result->str_arr[f_i][f_i2] = '\0';
                f_i3++;
                break;
            }
            else
            {
                f_result->str_arr[f_i][f_i2] = a_str[f_i3];
            }
            f_i2++;
            f_i3++;
        }
        f_i++;
    }

    return f_result;
}

void v_free_split_line(t_pydaw_line_split * a_split_line)
{
    int f_i = 0;
    while(f_i < a_split_line->count)
    {
        free(a_split_line->str_arr[f_i]);
        f_i++;
    }

    free(a_split_line->str_arr);
    free(a_split_line);
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

    assert(dir != NULL);
    //if (dir != NULL)
    //{
      while ((ent = readdir (dir)) != NULL)
      {
          if((!strcmp(ent->d_name, ".")) || (!strcmp(ent->d_name, "..")))
          {
              continue;
          }

          f_result->dir_list[(f_result->dir_count)] =
                  (char*)malloc(sizeof(char) * PYDAW_TINY_STRING);

            strcpy(f_result->dir_list[(f_result->dir_count)], ent->d_name);

          f_result->dir_count = (f_result->dir_count) + 1;

          if((f_result->dir_count) >= f_current_max)
          {
              f_current_max += f_resize_factor;
              f_result->dir_list =
                  (char**)realloc(f_result->dir_list,
                      sizeof(char*) * f_current_max);
          }
      }
      closedir (dir);
    /*
    }
    else
    {
      return 0;
    }
    */
    return f_result;
}

void v_pydaw_write_to_file(char * a_file, char * a_string)
{
    FILE* pFile = fopen(a_file, "w");
    assert(pFile);
    fprintf(pFile, "%s",a_string);
    fclose(pFile);

    char mode[] = "0777";
    int i = strtol(mode, 0, 8);

    if (chmod (a_file,i) < 0)
    {
        printf("Error chmod'ing file %s.\n", a_file);
    }
}

void v_pydaw_append_to_file(char * a_file, char * a_string)
{
    FILE* pFile = fopen(a_file, "a");
    assert(pFile);
    fprintf(pFile, "%s", a_string);
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


#endif	/* PYDAW_FILES_H */

