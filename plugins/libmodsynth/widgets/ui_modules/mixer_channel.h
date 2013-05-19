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

