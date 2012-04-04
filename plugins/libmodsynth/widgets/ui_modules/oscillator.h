/* 
 * File:   oscillator.h
 * Author: Jeff Hubbard
 * 
 * A modular UI element for an oscillator
 *
 * Created on April 3, 2012, 11:24 PM
 */

#ifndef OSCILLATOR_H
#define	OSCILLATOR_H

#include "../group_box.h"
#include "../knob_regular.h"
#include "../lms_combobox.h"
#include "../lms_control.h"
#include <QStringList>

class LMS_ui_oscillator
{
public:    
    LMS_ui_oscillator(LMS_style_info * a_style, QWidget * a_parent, QString a_title, int a_pitch_port, 
            int a_fine_port, int a_vol_port, QStringList a_osc_types, int a_type_port)
    {
        lms_groupbox = new LMS_group_box(a_parent, a_title, a_style);
        
    }
    
    LMS_group_box * lms_groupbox;    
    LMS_knob_regular * lms_pitch_knob;    
    LMS_knob_regular * lms_fine_knob;    
    LMS_knob_regular * lms_vol_knob;    
    LMS_combobox * lms_osc_type_box;
    
    
};

#endif	/* OSCILLATOR_H */

