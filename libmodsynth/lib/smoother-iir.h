/* 
 * File:   smoother-iir.h
 * Author: bob
 *
 * Created on February 6, 2012, 7:12 PM
 */

#ifndef SMOOTHER_IIR_H
#define	SMOOTHER_IIR_H

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct s_smoother_iir
{
    float output;
}t_smoother_iir;

inline void v_smr_iir_run(float,t_smoother_iir*);

inline void v_smr_iir_run(float a_in,t_smoother_iir * a_smoother) 
{ 
    a_smoother->output = (a_in * .01) + ((a_smoother->output) * .99f);     
}

#ifdef	__cplusplus
}
#endif

#endif	/* SMOOTHER_IIR_H */

