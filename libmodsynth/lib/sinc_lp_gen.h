/* 
 * File:   sinc_lp_gen.h
 * Author: Jeff Hubbard
 * 
 * This file is not yet production ready, and will likely be moved to function_plotter.
 *
 * Created on January 26, 2012, 6:56 PM
 */

#ifndef SINC_LP_GEN_H
#define	SINC_LP_GEN_H

#ifdef	__cplusplus
extern "C" {
#endif



#include <math.h>

// Function prototypes
void v_sinc_get_ir(float h[], const int &N, const int &WINDOW, const float &fc);
static void v_sinc_run_sinc(float sinc[], const int &N, const float &fc);
static void v_sinc_get_blackman_window(float w[], const int &N);


// Window type constants
/*
const int W_BLACKMAN = 1;
const int W_HANNING = 2;
const int W_HAMMING = 3;
*/


/*Example code to get the sinc impulse response:

#define SINC_TAPS 15
#define SINC_SAMPLES_PER_TAP 100
#define SINC_ARRAY_SIZE     SINC_TAPS * SINC_SAMPLES_PER_TAP

float f_sinc_array [SINC_ARRAY_SIZE];

wsfirLP(f_sinc_array, SINC_TAPS, .25, SINC_ARRAY_SIZE);

 
 //Now use the IR to do interpolation 
*/



// Generate lowpass filter
// 
// This is done by generating a sinc function and then windowing it
void v_sinc_get_ir(float h[],		// h[] will be written with the filter coefficients
            const int &N,		// size of the filter (number of taps)            
            const float &fc,       // cutoff frequency
            const int a_array_size) //number of samples per tap	
{
	int i;
	float *w = new float[N];		// window function
        float *sinc = new float[N];	// sinc function
    
	// 1. Generate Sinc function
	v_sinc_run_sinc(sinc, N, fc, a_array_size);
    
	// 2. Generate Window function
        v_sinc_get_blackman_window(w, N, a_array_size);
        
        
	// 3. Make lowpass filter
	for (i = 0; i < N; i++) {
		h[i] = sinc[i] * w[i];
	}

	// Delete dynamic storage
	delete []w;
	delete []sinc;

	return;
}

// Generate sinc function - used for making lowpass filter
static void v_sinc_run_sinc(float sinc[],		// sinc[] will be written with the sinc function
            const int &N,		// size (number of taps)
            const float &fc,            // cutoff frequency
            const int a_array_size,
            const double a_inc,  //sample increment
            const double a_max) //number of samples per tap)	        
{
	int i;
	const float M = N-1;
	float n;

        double f_inc_i;
	// Constants
	const float PI = 3.14159265358979323846;

	// Generate sinc delayed by (N-1)/2
	//for (i = 0; i < N; i++) {
        for (i = 0; i < a_array_size; i++) {
		if ((int)f_inc_i == M/2.0) {
			sinc[i] = 2.0 * fc;
		}
		else {
			n = f_inc_i - M/2.0;
			sinc[i] = sin(2.0*PI*fc*n) / (PI*n);
		}
                
                f_inc_i += a_inc;
                
	}        

	return;
}

// Generate window function (Blackman)
static void v_sinc_get_blackman_window(float w[],		// w[] will be written with the Blackman window
               const int &N, 	// size of the window
               const int a_array_size,
               const double a_inc,  //sample increment
               const double a_max) //number of samples per tap)	         
{
	int i;
	const float M = N-1;
	const float PI = 3.14159265358979323846;
        double f_inc_i;

        //for (i = 0; i < N; i++) {
	for (i = 0; i < a_array_size; i++) {
		//w[i] = 0.42 - (0.5 * cos(2.0*PI*(float)i/M)) + (0.08*cos(4.0*PI*(float)i/M));
                w[i] = 0.42 - (0.5 * cos(2.0*PI*(float)f_inc_i/M)) + (0.08*cos(4.0*PI*(float)f_inc_i/M));
                f_inc_i += a_inc;
	}

	return;
}



#ifdef	__cplusplus
}
#endif

#endif	/* SINC_LP_GEN_H */

