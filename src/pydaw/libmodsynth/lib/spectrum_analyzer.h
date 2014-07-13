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

#ifndef SPECTRUM_ANALYZER_H
#define	SPECTRUM_ANALYZER_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <complex.h>
#include <fftw3.h>



typedef struct {
    float * buffer;
    int buf_pos;
    fftw_complex *output;
    fftw_plan plan;
    int height, width;
    int samples_count;
    int samples_count_div2;
    double *samples;
    char * str_buf;
    char str_tmp[128];
} t_spa_spectrum_analyzer;


t_spa_spectrum_analyzer * g_spa_spectrum_analyzer_get(int a_sample_count)
{
    t_spa_spectrum_analyzer * f_result =
            (t_spa_spectrum_analyzer*)malloc(sizeof(t_spa_spectrum_analyzer));
    int f_i;

    if(posix_memalign((void**)&f_result->buffer, 16,
            sizeof(float) * a_sample_count) != 0)
    {
        return 0;
    }

    f_result->buf_pos = 0;
    f_result->samples_count = a_sample_count;
    f_result->samples_count_div2 = a_sample_count / 2;

    f_result->samples = fftw_alloc_real(a_sample_count);
    f_result->output = fftw_alloc_complex(a_sample_count);

    while(f_i < f_result->samples_count)
    {
        f_result->samples[f_i] = 0.0f;
        f_result->output[f_i] = 0.0f;
        f_i++;
    }

    f_result->str_buf = (char*)malloc(sizeof(char) * 15 * a_sample_count);
    f_result->str_buf[0] = '\0';

    f_result->plan = fftw_plan_dft_r2c_1d(f_result->samples_count,
        f_result->samples, f_result->output, 0);

    return f_result;
}

static void g_spa_free(t_spa_spectrum_analyzer *a_spa)
{
    fftw_destroy_plan(a_spa->plan);
    fftw_free(a_spa->output);
    fftw_free(a_spa->samples);
}

void v_spa_compute_fft(t_spa_spectrum_analyzer *a_spa)
{
    int f_i = 1;

    fftw_execute(a_spa->plan);

    sprintf(a_spa->str_buf, "%f", cabs(a_spa->output[0]));

    while(f_i < a_spa->samples_count_div2)
    {
        sprintf(a_spa->str_tmp, "|%f", cabs(a_spa->output[f_i]));
        strcat(a_spa->str_buf, a_spa->str_tmp);
        f_i++;
    }
}

/* void v_spa_run(struct t_spa_spectrum_analyzer *a_spa,
 * float * a_buf0, float * a_buf1, int a_count)
 *
 * Check if a_spa->str_buf[0] == '\0', if not, send a configure message
 * and then set spa->str_buf[0] = '\0'
 */
void v_spa_run(t_spa_spectrum_analyzer *a_spa,
        float * a_buf0, float * a_buf1, int a_count)
{
    int f_i = 0;

    while(f_i < a_count)
    {
        a_spa->samples[a_spa->buf_pos] = (a_buf0[f_i] + a_buf1[f_i]) * 0.5f;
        a_spa->buf_pos++;
        f_i++;

        if(a_spa->buf_pos >= a_spa->samples_count)
        {
            a_spa->buf_pos = 0;
            v_spa_compute_fft(a_spa);
        }
    }
}

#ifdef	__cplusplus
}
#endif

#endif	/* SPECTRUM_ANALYZER_H */

