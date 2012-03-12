/* 
 * File:   osc_additive.h
 * Author: Jeff Hubbard
 * 
 * This file is still incomplete, do not attempt to use it
 *
 * Created on February 16, 2012, 6:50 PM
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
    
/*Define the maximum number of harmonics in a wave.  A given note may have fewer than this if it exceeds the Nyquist frequency*/
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

