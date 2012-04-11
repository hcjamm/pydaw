/* 
 * File:   defines.h
 * Author: Jeff Hubbard
 *
 * Created on April 11, 2012, 6:50 PM
 */

#ifndef DEFINES_H
#define	DEFINES_H

#define LMS_MIXER_CHANNEL_COUNT 8
/*How many control ports for each channel, for example: 1 amp, 2 gain, 3 pan, 4 pan_law*/
#define LMS_CONTROLS_PER_CHANNEL 4

#define LMS_INPUT0  LMS_MIXER_CHANNEL_COUNT
#define LMS_INPUT1  (LMS_INPUT0 + LMS_MIXER_CHANNEL_COUNT)

#define LMS_OUTPUT0  18
#define LMS_OUTPUT1  19
/*GUI Step 11:  Add ports to the main synthesizer file that the GUI can talk to */
    
/*LMS_FIRST_CONTROL_PORT is the first port used for controls such as knobs.  All control ports must be numbered continuously,
 as they are iterated through*/
#define LMS_FIRST_CONTROL_PORT 20
/*This is the last control port*/
#define LMS_LAST_CONTROL_PORT ((LMS_FIRST_CONTROL_PORT * LMS_CONTROLS_PER_CHANNEL) + LMS_FIRST_CONTROL_PORT)
#define LMS_COUNT (LMS_LAST_CONTROL_PORT + 1)

#endif	/* DEFINES_H */

