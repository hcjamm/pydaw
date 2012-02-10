/* 
 * File:   adsr.h
 * Author: bob
 *
 * Created on February 9, 2012, 8:09 PM
 */

#ifndef ADSR_H
#define	ADSR_H


#ifdef	__cplusplus
extern "C" {
#endif
    
#include <stdio.h>
#include <stdlib.h>

void adsr_attack()
{
    float recip = 1/44100;
    float i = 0;
    float inc = .02;
    float max = 2;
    
    int count = ((max - i)/inc) + 1;
    
    printf("#define arr_adsr_attack_count %i\n\n", count);
    
    printf("float arr_adsr_attack [arr_adsr_attack_count] = {\n");
    
    while(i <= max)
    {
        if(i != 0)
            printf(",\n");
        
        if(i <= 0)
        {
            printf("1");
        }
        else
        {
            printf("%f", (recip / i));
        }
        
        i = i + inc;
    }
    
    printf("};\n");
    

}

void adsr_decay()
{
    float recip = 1/44100;
    float i = 0;
    float inc = .02;
    float max = 2;
    
    int count = ((max - i)/inc) + 1;
    
    printf("#define arr_adsr_decay_count %i\n\n", count);
    
    printf("float arr_adsr_decay [arr_adsr_decay_count] = {\n");
    
    while(i <= max)
    {
        if(i != 0)
            printf(",\n");
        
        if(i <= 0)
        {
            printf(".05");
        }
        else
        {
            printf("%f", (((recip) / (i)) * -1));
        }
        
        i = i + inc;
    }
    
    printf("};\n");
        
}

void adsr_release()
{
    float recip = 1/44100;
    float i = 0;
    float inc = .02;
    float max = 2;
    
    int count = (max - i)/inc + 1;
    
    printf("#define arr_adsr_release_count %i\n\n", count);
    
    printf("float arr_adsr_release [arr_adsr_release_count] = {\n");
    
    while(i <= max)
    {
        if(i != 0)
            printf(",\n");
        
        if(i <= 0)
        {
            printf("1");  //TODO:  Is this correct?
        }
        else
        {
            printf("%f", ((recip) / i));
        }
        
        i = i + inc;
    }
    
    printf("};\n");
       
    
}



#ifdef	__cplusplus
}
#endif


#endif	/* ADSR_H */

