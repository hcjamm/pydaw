/* 
 * File:   denormal.h
 * Author: vm-user
 *
 * Created on January 11, 2012, 8:10 PM
 */

#ifndef DENORMAL_H
#define	DENORMAL_H

#ifdef	__cplusplus
extern "C" {
#endif

float f_remove_denormal(float);

/*Prevent recursive modules like filters and feedback delays from consuming too much CPU*/
float f_remove_denormal(float a_input)
{
    if((a_input < .0001) && (a_input > -.0001))
        return 0;
    else
        return a_input;
    
}



#ifdef	__cplusplus
}
#endif

#endif	/* DENORMAL_H */

