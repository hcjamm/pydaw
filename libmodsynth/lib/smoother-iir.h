/* 
 * File:   smoother-iir.h
 * Author: Jeff Hubbard
 * 
 * This file provides the t_smoother_iir type, which can be used to smooth GUI controls values.
 * 
 * Smoother linear provides better fine-tuning of the smoothing, and only uses slightly more CPU power.
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

inline void v_smr_iir_run(t_smoother_iir*, float);

/* inline void v_smr_iir_run(
 * t_smoother_iir * 
 * a_smoother, float a_in)  //The input to be smoothed
 * 
 * Use t_smoother_iir->output as your new control value after running this
 */
inline void v_smr_iir_run(t_smoother_iir * a_smoother, float a_in) 
{ 
    a_smoother->output = (a_in * .01) + ((a_smoother->output) * .99f);     
}

t_smoother_iir * g_smr_iir_get_smoother();

t_smoother_iir * g_smr_iir_get_smoother()
{
    t_smoother_iir * f_result = (t_smoother_iir*)malloc(sizeof(t_smoother_iir));
    f_result->output = 0;
    return f_result;
}

#ifdef	__cplusplus
}
#endif

#endif	/* SMOOTHER_IIR_H */

