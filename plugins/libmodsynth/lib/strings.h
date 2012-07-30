/* 
 * File:   strings.h
 * Author: jeffh
 * 
 * This was created because GCC is doing some poor optimizations to strcpy that are causing SEGFAULTs.
 * Eventually it will contain more string processing functions specific to LibModSynth.
 *
 * Created on July 29, 2012, 7:43 PM
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

