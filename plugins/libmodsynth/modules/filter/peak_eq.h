/* 
 * File:   peak_eq.h
 * Author: jeffh
 *
 * Created on July 15, 2012, 9:46 PM
 */

#ifndef PEAK_EQ_H
#define	PEAK_EQ_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include "../../lib/pitch_core.h"
#include "../../lib/amp.h"
#include "../../constants.h"
#include "../../lib/denormal.h"
#include <math.h>
    
typedef struct st_pkq_peak_eq
{
    float BW, dB, pitch, exp_value, exp_db, d, B, d_times_B,
            w, w2, wQ, y2_0, y2_1, FIR_out_0, FIR_out_1, w2p1, coeff0, coeff1, coeff2, pi_div_sr;
    float input0, input1, in0_m1, in0_m2, in1_m1, in1_m2;
    float output0, output1, out0_m1, out0_m2, out1_m1, out1_m2;
    float warp_input, warp_input_squared, warp_input_tripled, warp_outstream0, warp_outstream1, warp_output;
    
    float coeff1_x_out_m1_0, coeff2_x_out_m2_0, iir_output0, coeff1_x_out_m1_1, coeff2_x_out_m2_1, iir_output1;
    
    t_amp * amp_ptr;
    t_pit_pitch_core * pitch_core;
}t_pkq_peak_eq;

t_pkq_peak_eq * g_pkq_get(float);

inline void v_pkq_calc_coeffs(t_pkq_peak_eq*,float,float,float);
inline void v_pkq_run(t_pkq_peak_eq*,float,float);

/* inline void v_pkq_calc_coeffs(
 * t_pkq_peak_eq *a_pkq,
 * float a_pitch, 
 * float a_bw, 
 * float a_db)
 */
inline void v_pkq_calc_coeffs(t_pkq_peak_eq *__restrict a_pkq,float a_pitch, float a_bw, float a_db)
{
    if((a_db != (a_pkq->dB)) || (a_bw != (a_pkq->BW)))
    {
        a_pkq->BW = a_bw;
        a_pkq->dB = a_db;
        a_pkq->exp_value = exp((a_pkq->BW)*log(1.421f));
        a_pkq->exp_db = exp((a_pkq->dB)*log(1.061f));
        
        a_pkq->d = (((a_pkq->exp_value) * (a_pkq->exp_value)) - 1.0f)/((a_pkq->exp_value) * (a_pkq->exp_db));

        a_pkq->B = ((a_pkq->exp_db) * (a_pkq->exp_db)) - 1.0f;

        a_pkq->d_times_B = (a_pkq->d) * (a_pkq->B);
    }
    
    a_pkq->warp_input = f_pit_midi_note_to_hz_fast(a_pitch, a_pkq->pitch_core) * (a_pkq->pi_div_sr);
    a_pkq->warp_input_squared = (a_pkq->warp_input)*(a_pkq->warp_input);
    a_pkq->warp_input_tripled = (a_pkq->warp_input_squared)*(a_pkq->warp_input);
    a_pkq->warp_outstream0 = (a_pkq->warp_input_squared) * (a_pkq->warp_input_tripled) * 0.133333f;
    a_pkq->warp_outstream1 = (a_pkq->warp_input_tripled) * 0.333333f;
    a_pkq->w = (a_pkq->warp_outstream0) + (a_pkq->warp_outstream1) + (a_pkq->warp_input);
    
    
    a_pkq->w2 = (a_pkq->w) * (a_pkq->w);
    a_pkq->wQ = (a_pkq->w) * (a_pkq->d);
    
    
    a_pkq->w2p1 = (a_pkq->w2) + 1.0f;
    a_pkq->coeff0 = 1.0f/((a_pkq->w2p1) + (a_pkq->wQ));
    a_pkq->coeff1   = ((a_pkq->w2) - 1.0f) * 2.0f;
    a_pkq->coeff2   = (a_pkq->w2p1) - (a_pkq->wQ);
}

inline void v_pkq_run(t_pkq_peak_eq *__restrict a_pkq,float a_in0, float a_in1)
{
    a_pkq->in0_m2 = (a_pkq->in0_m1);
    a_pkq->in0_m1 = a_in0;
    
    a_pkq->y2_0 = a_pkq->in0_m2;
    a_pkq->FIR_out_0 = (a_in0 - (a_pkq->y2_0)) * (a_pkq->w);
    
    a_pkq->in1_m2 = (a_pkq->in1_m1);
    a_pkq->in1_m1 = a_in1;
    
    a_pkq->y2_1 = a_pkq->in1_m2;
    a_pkq->FIR_out_1 = (a_in1 - (a_pkq->y2_1)) * (a_pkq->w);
    
    a_pkq->coeff1_x_out_m1_0 = (a_pkq->FIR_out_0) - ((a_pkq->coeff1) * (a_pkq->out0_m1));
    a_pkq->coeff2_x_out_m2_0 = (a_pkq->coeff1_x_out_m1_0) - ((a_pkq->coeff2) * (a_pkq->out0_m2));
    a_pkq->iir_output0 = f_remove_denormal((a_pkq->coeff2_x_out_m2_0) * (a_pkq->coeff0));
    a_pkq->output0 = ((a_pkq->d_times_B) * (a_pkq->iir_output0)) + a_in0;
    
    a_pkq->coeff1_x_out_m1_1 = (a_pkq->FIR_out_1) - ((a_pkq->coeff1) * (a_pkq->out1_m1));
    a_pkq->coeff2_x_out_m2_1 = (a_pkq->coeff1_x_out_m1_1) - ((a_pkq->coeff2) * (a_pkq->out1_m2));
    a_pkq->iir_output1 = f_remove_denormal((a_pkq->coeff2_x_out_m2_1) * (a_pkq->coeff0));
    a_pkq->output1 = ((a_pkq->d_times_B) * (a_pkq->iir_output1)) + a_in1;
    
    
    a_pkq->out0_m2 = (a_pkq->out0_m1);
    a_pkq->out0_m1 = (a_pkq->iir_output0);
    
    a_pkq->out1_m2 = (a_pkq->out1_m1);
    a_pkq->out1_m1 = (a_pkq->iir_output1);
}

t_pkq_peak_eq * g_pkq_get(float a_sample_rate)
{
    t_pkq_peak_eq * f_result = (t_pkq_peak_eq*)malloc(sizeof(t_pkq_peak_eq));
    
    f_result->B = 0.0f;
    f_result->FIR_out_0 = 0.0f;
    f_result->FIR_out_1 = 0.0f;
    f_result->BW = 0.0f;
    f_result->coeff1 = 0.0f;
    f_result->coeff2 = 0.0f;
    f_result->amp_ptr = g_amp_get();
    f_result->d = 0.0f;
    f_result->dB = 0.0f;
    f_result->d_times_B = 0.0f;
    f_result->exp_db = 0.0f;
    f_result->exp_value = 0.0f;
    f_result->in0_m1 = 0.0f;
    f_result->in0_m2 = 0.0f;
    f_result->in1_m1 = 0.0f;
    f_result->in1_m2 = 0.0f;
    f_result->input0 = 0.0f;
    f_result->input1 = 0.0f;
    f_result->out0_m1 = 0.0f;
    f_result->out0_m2 = 0.0f;
    f_result->out1_m1 = 0.0f;
    f_result->out1_m2 = 0.0f;
    f_result->output0 = 0.0f;
    f_result->output1 = 0.0f;
    f_result->pi_div_sr = PI/a_sample_rate;
    f_result->pitch = 66.0f;
    f_result->pitch_core = g_pit_get();
    f_result->coeff0 = 0.0f;
    f_result->warp_input = 0.0f;
    f_result->warp_input_squared = 0.0f;
    f_result->warp_input_tripled = 0.0f;
    f_result->warp_output = 0.0f;
    f_result->warp_outstream0 = 0.0f;
    f_result->warp_outstream1 = 0.0f;
    f_result->w = 0.0f;
    f_result->w2 = 0.0f;
    f_result->w2p1 = 0.0f;
    f_result->wQ = 0.0f;
    f_result->y2_0 = 0.0f;
    f_result->y2_1 = 0.0f;
    
    return f_result;
}

#ifdef	__cplusplus
}
#endif

#endif	/* PEAK_EQ_H */

