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

float _remove_denormal(float);

/*Prevent recursive modules like filters and feedback delays from consuming too much CPU*/
float _remove_denormal(float _input)
{
    if((_input < .0001) && (_input > -.0001))
        return 0;
    else
        return _input;
    
}



#ifdef	__cplusplus
}
#endif

#endif	/* DENORMAL_H */

