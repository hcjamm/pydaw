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

#ifndef OSC_ADDITIVE_H
#define	OSC_ADDITIVE_H

#ifdef	__cplusplus
extern "C" {
#endif


#include "../../lib/pitch_core.h"
#include "../../lib/amp.h"
#include "../../lib/osc_core.h"
#include "osc_simple.h"

/*Define the maximum number of harmonics in a wave.
 * A given note may have fewer than this if it exceeds the Nyquist frequency*/
#define OSC_ADDITIVE_MAX_HARMONICS 64

typedef struct st_osca_additive
{
    float harmonics_freq [OSC_ADDITIVE_MAX_HARMONICS];
    float harmonics_amp [OSC_ADDITIVE_MAX_HARMONICS];
    t_osc_core cores [OSC_ADDITIVE_MAX_HARMONICS];
    int retrig_iterator;
    float last_note;
}t_osca_additive;





#ifdef	__cplusplus
}
#endif

#endif	/* OSC_ADDITIVE_H */

