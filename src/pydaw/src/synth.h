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

#ifndef PYDAW_SYNTH_H

#define	PYDAW_SYNTH_H

#define PYDAW_INPUT_COUNT 0

#define PYDAW_INPUT_MIN 0
#define PYDAW_INPUT_MAX  (PYDAW_INPUT_MIN + PYDAW_INPUT_COUNT)

#define PYDAW_OUTPUT0  (PYDAW_INPUT_COUNT)
#define PYDAW_OUTPUT1  (PYDAW_OUTPUT0 + 1)

#define PYDAW_FIRST_CONTROL_PORT (PYDAW_OUTPUT1 + 1)

#define PYDAW_LAST_CONTROL_PORT (PYDAW_FIRST_CONTROL_PORT)
#define PYDAW_COUNT  (PYDAW_LAST_CONTROL_PORT)

#include "../include/pydaw_plugin.h"

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct {
    PYFX_Data **input_arr;

    PYFX_Data *output0;
    PYFX_Data *output1;

    float fs;

    int i_mono_out;
    int i_buffer_clear;

} t_pydaw_engine;

#ifdef	__cplusplus
}
#endif

#endif	/* SYNTH_H */

