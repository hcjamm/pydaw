/* 
 * File:   mixer_channel.h
 * Author: Jeff Hubbard
 * 
 * A mixer channel for a multi-channel mixer
 *
 * Created on April 11, 2012, 12:07 PM
 */

#ifndef MIXER_CHANNEL_H
#define	MIXER_CHANNEL_H

#include <QGroupBox>
#include <QVBoxLayout>
#include "../lms_slider.h"

class LMS_mixer_channel
{
public:
    LMS_mixer_channel()
    {
        
    }
    
    QGroupBox * lms_groupbox;    
    LMS_slider * lms_amp_slider;
    LMS_slider * lms_pan_slider;    
    LMS_knob_regular * lms_pan_law_knob;
    LMS_knob_regular * lms_gain_knob;
    
private:
    QVBoxLayout * lms_layout;
    QHBoxLayout * lms_pan_law_gain_layout;
    
};


#endif	/* MIXER_CHANNEL_H */

