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

#ifndef OSCILLATOR_H
#define	OSCILLATOR_H

#include "../group_box.h"
#include "../knob_regular.h"
#include "../lms_combobox.h"
#include "../lms_control.h"
#include <QStringList>

class LMS_oscillator_widget
{
public:    
    LMS_oscillator_widget(LMS_style_info * a_style, QWidget * a_parent, QString a_title, int a_pitch_port, 
            int a_fine_port, int a_vol_port, int a_type_port, QStringList a_osc_types)
    {
        lms_groupbox = new LMS_group_box(a_parent, a_title, a_style);
         
        lms_pitch_knob =  new LMS_knob_regular(QString("Pitch"), -36, 36, 1, 0, QString("0"), lms_groupbox->lms_groupbox, a_style, lms_kc_integer, a_pitch_port);    
        lms_groupbox->lms_add_h(lms_pitch_knob);    
        lms_fine_knob = new LMS_knob_regular(QString("Fine"), -100, 100, 1, 0, QString("0"), lms_groupbox->lms_groupbox, a_style, lms_kc_decimal, a_fine_port);
        lms_groupbox->lms_add_h(lms_fine_knob);
        lms_vol_knob = new LMS_knob_regular(QString("Vol"), -60, 0, 1, 0, QString("0"), lms_groupbox->lms_groupbox, a_style, lms_kc_integer, a_vol_port);
        lms_groupbox->lms_add_h(lms_vol_knob);
        lms_osc_type_box = new LMS_combobox(QString("Type"), lms_groupbox->lms_groupbox, a_osc_types, a_type_port, a_style);        
        lms_osc_type_box->lms_combobox->setMinimumWidth(114);
        lms_groupbox->lms_add_h(lms_osc_type_box);
    }
    
    LMS_group_box * lms_groupbox;    
    LMS_knob_regular * lms_pitch_knob;    
    LMS_knob_regular * lms_fine_knob;    
    LMS_knob_regular * lms_vol_knob;    
    LMS_combobox * lms_osc_type_box;
};

#endif	/* OSCILLATOR_H */

