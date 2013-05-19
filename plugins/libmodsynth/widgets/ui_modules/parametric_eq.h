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

#ifndef PARAMETRIC_EQ_H
#define	PARAMETRIC_EQ_H

#include "../group_box.h"
#include "../knob_regular.h"

class LMS_para_eq
{
public:
    LMS_para_eq(QWidget * a_parent, LMS_style_info * a_style, int a_cutoff_port, int a_res_port, int a_boost_port, QString a_label)
    {
        lms_groupbox = new LMS_group_box(a_parent, a_label, a_style);
        
        lms_freq = new LMS_knob_regular(QString("Freq"), 20, 124, 1, 96, QString("1000"), a_parent, a_style, lms_kc_pitch, a_cutoff_port);
        lms_res = new LMS_knob_regular(QString("Res"), -24, -1, 1, -15, QString("-15"), a_parent, a_style, lms_kc_integer, a_res_port);
        lms_boost = new LMS_knob_regular(QString("Boost"), -24, 24, 1, 0, QString("0"), a_parent, a_style, lms_kc_integer, a_boost_port);
        
        lms_groupbox->lms_add_h(lms_freq);
        lms_groupbox->lms_add_v(lms_res);
        lms_groupbox->lms_add_v(lms_boost);
    }

    LMS_group_box * lms_groupbox;
    LMS_knob_regular * lms_freq;
    LMS_knob_regular * lms_res;
    LMS_knob_regular * lms_boost;
    
};

#endif	/* PARAMETRIC_EQ_H */

