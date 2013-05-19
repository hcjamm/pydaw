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

#ifndef FILTER_H
#define	FILTER_H

#include "../group_box.h"
#include "../knob_regular.h"
#include "../lms_combobox.h"

class LMS_filter_widget
{
public:
    LMS_filter_widget(QWidget * a_parent, LMS_style_info * a_style, int a_cutoff_port, int a_res_port, int a_type_port, bool a_show_combobox)
    {
        lms_groupbox = new LMS_group_box(a_parent, QString("Filter"), a_style);
        
        lms_cutoff_knob = new LMS_knob_regular(QString("Cutoff"), 20, 124, 1, 96, QString("1000"), a_parent, a_style, lms_kc_pitch, a_cutoff_port);
        lms_res_knob = new LMS_knob_regular(QString("Res"), -30, 0, 1, -12, QString("-12"), a_parent, a_style, lms_kc_integer, a_res_port);
        
        if(a_show_combobox)
        {
            QStringList f_types = QStringList() << "LP 2" << "HP 2" << "BP2" << "LP 4" << "HP 4" << "BP4" << "Off";
            lms_filter_type = new LMS_combobox(QString("Type"),  a_parent, f_types, a_type_port, a_style);
        }
        
        lms_groupbox->lms_add_h(lms_cutoff_knob);
        lms_groupbox->lms_add_h(lms_res_knob);
        
        if(a_show_combobox)
                lms_groupbox->lms_add_h(lms_filter_type);        
    }
    
    LMS_group_box * lms_groupbox;
    LMS_knob_regular * lms_cutoff_knob;
    LMS_knob_regular * lms_res_knob;
    LMS_combobox * lms_filter_type;
};

#endif	/* FILTER_H */

