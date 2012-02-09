/* 
 * File:   amp.h

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

 */

#ifndef AMP_H
#define	AMP_H

#include <math.h>
#include "interpolate-linear.h"

#ifdef	__cplusplus
extern "C" {
#endif

inline float f_db_to_linear(float);
inline float f_linear_to_db(float);    
inline float f_db_to_linear_fast(float);

inline float f_db_to_linear(float a_db)
{
    float f_result = pow ( 10.0, (0.05 * a_db) );
    return f_result;
}

inline float f_linear_to_db(float a_linear)
{
    float f_result = 20.0 * log10 ( a_linear );
    return f_result;
}


/*Arrays*/

#define arr_amp_db2a_count 137

//-100 to +36
float arr_amp_db2a [arr_amp_db2a_count] = {
0.000010,
0.000011,
0.000013,
0.000014,
0.000016,
0.000018,
0.000020,
0.000022,
0.000025,
0.000028,
0.000032,
0.000035,
0.000040,
0.000045,
0.000050,
0.000056,
0.000063,
0.000071,
0.000079,
0.000089,
0.000100,
0.000112,
0.000126,
0.000141,
0.000158,
0.000178,
0.000200,
0.000224,
0.000251,
0.000282,
0.000316,
0.000355,
0.000398,
0.000447,
0.000501,
0.000562,
0.000631,
0.000708,
0.000794,
0.000891,
0.001000,
0.001122,
0.001259,
0.001413,
0.001585,
0.001778,
0.001995,
0.002239,
0.002512,
0.002818,
0.003162,
0.003548,
0.003981,
0.004467,
0.005012,
0.005623,
0.006310,
0.007079,
0.007943,
0.008913,
0.010000,
0.011220,
0.012589,
0.014125,
0.015849,
0.017783,
0.019953,
0.022387,
0.025119,
0.028184,
0.031623,
0.035481,
0.039811,
0.044668,
0.050119,
0.056234,
0.063096,
0.070795,
0.079433,
0.089125,
0.100000,
0.112202,
0.125893,
0.141254,
0.158489,
0.177828,
0.199526,
0.223872,
0.251189,
0.281838,
0.316228,
0.354813,
0.398107,
0.446684,
0.501187,
0.562341,
0.630957,
0.707946,
0.794328,
0.891251,
1.000000,
1.122018,
1.258925,
1.412538,
1.584893,
1.778279,
1.995262,
2.238721,
2.511886,
2.818383,
3.162278,
3.548134,
3.981072,
4.466836,
5.011872,
5.623413,
6.309574,
7.079458,
7.943282,
8.912509,
10.000000,
11.220184,
12.589254,
14.125376,
15.848932,
17.782795,
19.952623,
22.387211,
25.118864,
28.183830,
31.622776,
35.481339,
39.810719,
44.668358,
50.118725,
56.234131,
63.095734};


inline float f_db_to_linear_fast(float a_db)
{
    if(a_db > arr_amp_db2a_count)
        a_db = arr_amp_db2a_count;
    
    if(a_db < 0)
        a_db = 0;
    
    return f_linear_interpolate_arr(arr_amp_db2a, a_db + 100.0f);
}

#ifdef	__cplusplus
}
#endif

#endif	/* AMP_H */

