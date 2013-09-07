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

#ifndef STRINGS_H
#define	STRINGS_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#define LMS_STRCPY_LIMIT 2000

void lms_strcpy(char*,const char*);

/* void lms_strcpy(char* a_dest, const char* a_src)
 * 
 * The string must be NULL terminated, otherwise bad things will happen.
 * 
 * The LMS_STRCPY_LIMIT macro prevents an infinite loop, but you will still SEGFAULT 
 */
void lms_strcpy(char* a_dest, const char* a_src)
{
    int f_i;
    
    for(f_i = 0; f_i < LMS_STRCPY_LIMIT; f_i++)
    {
        char f_value = a_src[f_i];
        a_dest[f_i] = f_value;
        if((f_value) == '\0')
        {
            break;
        }
    }
}


#ifdef	__cplusplus
}
#endif

#endif	/* STRINGS_H */

