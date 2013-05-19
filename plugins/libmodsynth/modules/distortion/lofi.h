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

#ifndef PYDAW_LOFI_H
#define	PYDAW_LOFI_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <math.h>
    
typedef struct 
{
    float bits, multiplier, recip;
    int val0, val1;
    float output0, output1;
} t_lfi_lofi;

t_lfi_lofi * g_lfi_lofi_get();
void v_lfi_lofi_set(t_lfi_lofi*, float);
void v_lfi_lofi_run(t_lfi_lofi*, float, float);

t_lfi_lofi * g_lfi_lofi_get()
{
    t_lfi_lofi * f_result;
    
    if(posix_memalign((void**)&f_result, 16, (sizeof(t_lfi_lofi))) != 0)
    {
        return 0;
    }
    
    f_result->output0 = 0.0f;
    f_result->output1 = 0.0f;
    f_result->bits = 163.654f;
    f_result->multiplier = -443322.0f;
    f_result->recip = 1.010101f;
    f_result->val0 = 0;
    f_result->val1 = 0;
    
    return f_result;
}

void v_lfi_lofi_set(t_lfi_lofi* a_lfi, float a_bits)
{
    if(a_lfi->bits != a_bits)
    {
        a_lfi->bits = a_bits;
        a_lfi->multiplier = pow(2.0, a_bits);
        a_lfi->recip = 1.0f / (a_lfi->multiplier);
    }
}

void v_lfi_lofi_run(t_lfi_lofi* a_lfi, float a_in0, float a_in1)
{
    a_lfi->val0 = (int)((a_lfi->multiplier) * a_in0);
    a_lfi->val1 = (int)((a_lfi->multiplier) * a_in1);
    
    a_lfi->output0 = ((float)(a_lfi->val0)) * (a_lfi->recip);
    a_lfi->output1 = ((float)(a_lfi->val1)) * (a_lfi->recip);
}


#ifdef	__cplusplus
}
#endif

#endif	/* PYDAW_LOFI_H */

